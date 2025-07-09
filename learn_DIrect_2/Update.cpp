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
    for (int step = 0; step < subSteps; step++) {
        //x
        float nextPlayerX = playerX + playerVelocityX * subDelta;
        for (const auto& obj : state->sceneObjects) {
            float platformW = obj->GetW();
            float platformH = obj->GetH();
            float platformX = obj->GetPosX();
            float platformY = obj->GetPosY();

            bool overlapY = ((playerY + playerH) > platformY) && (playerY < (platformY + platformH));

            if (overlapY && playerVelocityX > 0 && (playerX + playerW <= platformX) && (nextPlayerX + playerW > platformX)) {
                nextPlayerX = platformX - playerW;
                playerVelocityX = 0;
            }
            else if (overlapY && playerVelocityX < 0 && (playerX >= platformX + platformW) && (nextPlayerX < platformX + platformW)) {

                nextPlayerX = platformX + platformW;
                playerVelocityX = 0;
            }
        }
        playerX = nextPlayerX;
        //y
        float nextPlayerY = playerY + playerVelocityY * subDelta;
        float footY = playerY + playerH;
        float leftFootX = playerX + playerW * 0.2f;
        float rightFootX = playerX + playerW * 0.8f;
        for (const auto& obj : state->sceneObjects) {
            float platformW = obj->GetW();
            float platformH = obj->GetH();
            float platformX = obj->GetPosX();
            float platformY = obj->GetPosY();

            bool overlapX = ((playerX + playerW) > platformX) && (playerX < (platformX + platformW));

            if (overlapX && playerVelocityY > 0 && (playerY + playerH <= platformY) && (nextPlayerY + playerH >= platformY)) {

                /*if (
                    (footY >= platformY && footY <= platformY) &&
                    ((leftFootX >= platformX && leftFootX <= platformX + platformW) || (rightFootX >= platformX && rightFootX <= platformX + platformW))
                    ) {*/
                    nextPlayerY = platformY - playerH;
                    playerVelocityY = 0;
                    onPlatform = true;
                //}
            }
            else if (overlapX && playerVelocityY < 0 && (playerY >= platformY + platformH) && (nextPlayerY <= platformY + platformH)) {
                nextPlayerY = platformY + platformH;
                playerVelocityY = 0;
            }
            else if (overlapX && playerVelocityY == 0 && std::fabs(playerY + playerH - platformY) < LANDING_TOLERANCE) {
                playerY = platformY - playerH;
                onPlatform = true;
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