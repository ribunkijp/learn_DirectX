/**********************************************************************************
    Update.cpp

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/

#include "Update.h"
#include <cmath>
#include "GameObject.h"

float SmoothDamp(
    float current,
    float target,
    float& velocity,
    float smoothTime,
    float deltaTime
);

void UpdateAllObjects(StateInfo* pState, float deltaTime) {

    pState->Player->Update(deltaTime);
    for (auto& obj : pState->sceneObjects)
    {
        // 1. アニメーションデータの更新（フレーム切り替え）
        obj->Update(deltaTime);
    }
}

void UpdatePlayer(StateInfo* state, float deltaTime, bool leftPressed, bool rightPressed, bool spacePressed) {
    if (!state || !state->Player) return;


    float playerX = state->Player->GetPosX();
    float playerY = state->Player->GetPosY();
    float playerW = state->Player->GetW();
    float playerH = state->Player->GetH();

    float speed = state->Player->GetSpeed();

    if (leftPressed && !rightPressed )
    {
        if ( state->isOnGround && (state->Player->state != AnimationState::Walk || state->Player->direction != Direction::Left)) {
            state->Player->SetFrameIndex(0);
            state->Player->ResetAnimationTimer();
            state->Player->SetAnimationData(WalkLeft);
            state->Player->state = AnimationState::Walk;
            state->Player->direction = Direction::Left;
            state->Player->isAnimated = true;
        }
        state->playerVelocityX = -speed;
    }
    else if (rightPressed && !leftPressed)
    {
        if (state->isOnGround && (state->Player->state != AnimationState::Walk || state->Player->direction != Direction::Right)) {
            state->Player->SetFrameIndex(0);
            state->Player->ResetAnimationTimer();
            state->Player->SetAnimationData(WalkRight);
            state->Player->state = AnimationState::Walk;
            state->Player->direction = Direction::Right;
            state->Player->isAnimated = true;
        }
        state->playerVelocityX = speed;
    }
    else {
        state->playerVelocityX = 0.0f;

        if (state->isOnGround && state->Player->state != AnimationState::Idle) {
            state->Player->isAnimated = false;
            state->Player->SetFrameIndex(0);
            state->Player->ResetAnimationTimer();
            if (state->Player->direction == Direction::Left)
                state->Player->SetAnimationData(IdleLeft);
            else 
                state->Player->SetAnimationData(IdleRight);
            
            state->Player->state = AnimationState::Idle;
        }
    }


    //
    if (spacePressed && state->isOnGround && !state->lastSpacePressed) {
        state->playerVelocityY = state->jumpVelocity;
        state->isOnGround = false;
        state->isJumping = true;
        state->jumpHoldTime = 0.0f;

        state->Player->isAnimated = false;
        state->Player->SetFrameIndex(0);
        state->Player->ResetAnimationTimer();
        if (state->Player->direction == Direction::Left)
            state->Player->SetAnimationData(JumpLeft);
        else
            state->Player->SetAnimationData(JumpRight);

        state->Player->state = AnimationState::Jump;
    }
    if (spacePressed && state->isJumping && state->jumpHoldTime < state->maxJumpHoldTime) {
        state->playerVelocityY = state->jumpVelocity;
        state->jumpHoldTime += deltaTime;
    }
    else {
        state->isJumping = false;
    }


    if (!state->isOnGround) {

        if (!spacePressed && state->playerVelocityY < 0) {
            state->playerVelocityY += state->gravity * 2.5f * deltaTime;
        }
        else if (!state->isJumping) {
            state->playerVelocityY += state->gravity * deltaTime;
        }

        if (leftPressed && !rightPressed && state->Player->direction != Direction::Left) {
            state->Player->direction = Direction::Left;
        }
        else if (rightPressed && !leftPressed && state->Player->direction != Direction::Right) {
            state->Player->direction = Direction::Right;
        }
    }
    //
    if (!state->isOnGround && state->wasOnGround) {
        if (state->Player->state != AnimationState::Jump) {
            state->Player->isAnimated = false;
            state->Player->SetFrameIndex(0);
            state->Player->ResetAnimationTimer();
            if (state->Player->direction == Direction::Left)
                state->Player->SetAnimationData(JumpLeft);
            else
                state->Player->SetAnimationData(JumpRight);

            state->Player->state = AnimationState::Jump;
        }
    }
    //
    if (!state->wasOnGround && state->isOnGround) {
        state->Player->isAnimated = false;
        state->Player->SetFrameIndex(0);
        state->Player->ResetAnimationTimer();
        if (state->Player->direction == Direction::Left)
            state->Player->SetAnimationData(IdleLeft);
        else
            state->Player->SetAnimationData(IdleRight);

        state->Player->state = AnimationState::Idle;
    }
    



    bool onPlatform = checkPlatformCollision(state, playerY, playerX, playerH, playerW, deltaTime, state->playerVelocityY, state->playerVelocityX);


    state->isOnGround = onPlatform;




    //
    state->lastSpacePressed = spacePressed;

    // player 座標更新
    state->Player->SetPos(playerX, playerY);


    state->wasOnGround = state->isOnGround;

}

void UpdateCamera(StateInfo* state, float deltaTime) {
    float playerX = state->Player->GetPosX();
    float playerY = state->Player->GetPosY();
    float playerW = state->Player->GetW();
    float playerH = state->Player->GetH();

    float halfW = state->logicalWidth * 0.5f;
    float halfH = state->logicalHeight * 0.5f;

    float PlayerCenterX = playerX + playerW * 0.5;
    float PlayerCenterY = playerY + playerH * 0.5;

   

    

    // 
    if (playerY < halfH) {
        state->targetCameraY = playerY - halfH;
    }
 

    // 
    state->cameraY = SmoothDamp(
        state->cameraY,
        state->targetCameraY,
        state->cameraVelocityY,
        0.25f,
        deltaTime
    );
    
    state->targetCameraX =  playerX + playerW * 0.5f - halfW;
    state->cameraX = SmoothDamp(
        state->cameraX,
        state->targetCameraX,
        state->cameraVelocityX,
        0.25f,
        deltaTime
    );


    if (state->cameraX < 0.0f) state->cameraX = 0.0f;
    if (state->cameraY > 0.0f) state->cameraY = 0.0f;

   

    state->view = DirectX::XMMatrixTranslation(-state->cameraX, -state->cameraY, 0.0f);
}

bool checkPlatformCollision(StateInfo* state, float& playerY, float& playerX, float playerH, float playerW, float deltaTime, float& playerVelocityY, float& playerVelocityX) {
    

    const int subSteps = 4;
    float subDelta = deltaTime / static_cast<float>(subSteps);

    const float LANDING_TOLERANCE = 2.0f;//トレランス 許容誤差
    bool onPlatform = false;

    //
    const float hitboxOffsetX = 26.0f;
    const float hitboxW = 42.0f;

    //
    for (int step = 0; step < subSteps; step++) {
        // x
        float nextPlayerX = playerX + playerVelocityX * subDelta;
        float pX = playerX + hitboxOffsetX;
        float pW = hitboxW;
        float next_pX = nextPlayerX + hitboxOffsetX;
        //
        float nextPlayerY = playerY + playerVelocityY * subDelta;

        for (const auto& obj : state->sceneObjects) {
            float platformW = obj->GetW();
            float platformH = obj->GetH();
            float platformX = obj->GetPosX();
            float platformY = obj->GetPosY();

            // 右へ
            float yMin = (playerY < nextPlayerY) ? playerY : nextPlayerY;
            float yMax = ((playerY + playerH) > (nextPlayerY + playerH)) ? (playerY + playerH) : (nextPlayerY + playerH);
            bool overlapY = (yMax > platformY) && (yMin < (platformY + platformH));
            float right0 = pX + pW;
            float right1 = next_pX + pW;
            bool crossXRight = (right0 <= platformX && right1 > platformX) || (right1 <= platformX && right0 > platformX);
            if (overlapY && playerVelocityX > 0 && crossXRight) {
                nextPlayerX = platformX - pW - hitboxOffsetX;
                playerVelocityX = 0;
                break;
            }
            //左へ
            float platformRight = platformX + platformW;
            bool crossXLeft = (pX >= platformRight && next_pX < platformRight) || (next_pX >= platformRight && pX < platformRight);
            if (overlapY && playerVelocityX < 0 && crossXLeft) {
                nextPlayerX = platformRight - hitboxOffsetX;
                playerVelocityX = 0;
                break;
            }
        }
        playerX = nextPlayerX;
        //y
        pX = playerX + hitboxOffsetX;
        next_pX = pX + playerVelocityX * subDelta + hitboxOffsetX;
        for (const auto& obj : state->sceneObjects) {
            float platformW = obj->GetW();
            float platformH = obj->GetH();
            float platformX = obj->GetPosX();
            float platformY = obj->GetPosY();
            
            
            float xMin = pX < next_pX ? pX : next_pX;
            float xMax = ((pX + pW) > (next_pX + pW)) ? (pX + pW) : (next_pX + pW);
            float platformRight = platformX + platformW;

            //下へ
            float foot0 = playerY + playerH;
            float foot1 = nextPlayerY + playerH;

            bool overlapX = (xMax > platformX) && (xMin < platformRight);
            bool crossY = (foot0 <= platformY && foot1 > platformY) || (foot1 <= platformY && foot0 > platformY);
            if (overlapX && crossY && playerVelocityY > 0) {
                nextPlayerY = platformY - playerH;
                playerVelocityY = 0;
                onPlatform = true;
                break;
            }
            //上へ
            float platformBottom = platformY + platformH;
            bool crossYTop = (playerY >= platformBottom && nextPlayerY < platformBottom) || (nextPlayerY >= platformBottom && playerY < platformBottom);
            if (overlapX && crossYTop && playerVelocityY < 0) {
                nextPlayerY = platformY + platformH;
                playerVelocityY = 0;
                break;
            }
            //
            if (overlapX && playerVelocityY == 0 &&
                ((playerY + playerH - platformY) < LANDING_TOLERANCE && (playerY + playerH - platformY) > -LANDING_TOLERANCE)) {
                playerY = platformY - playerH;
                onPlatform = true;
                break;
            }

        }
        //ground
        if ( playerVelocityY > 0 && (playerY + playerH <= state->groundY) && (nextPlayerY + playerH >= state->groundY) ) {
            playerY = state->groundY - playerH;
            playerVelocityY = 0.0f;
            onPlatform = true;
        }

        playerY = nextPlayerY;

    }

    //
    if (playerVelocityY == 0 && std::fabs(playerY + playerH - state->groundY) < LANDING_TOLERANCE) {
        playerY = state->groundY - playerH;
        onPlatform = true;
    }

    
    return onPlatform;
}



float SmoothDamp(
    float current,       
    float target,       
    float& velocity,     
    float smoothTime,    
    float deltaTime      
) {
    float omega = 2.0f / smoothTime;
    float x = omega * deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

    float change = current - target;
    float temp = (velocity + omega * change) * deltaTime;
    velocity = (velocity - omega * temp) * exp;

    return target + (change + temp) * exp;
}