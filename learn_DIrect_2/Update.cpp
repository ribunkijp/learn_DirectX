/**********************************************************************************
    Update.cpp

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/

#include "Update.h"
#include <Windows.h>
#include <cmath>

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
        state->playerVelocityX = -speed;

    }
    else if (rightPressed && !leftPressed)
    {
        state->playerVelocityX = speed;
    }
    else {
        state->playerVelocityX = 0.0f;
    }


    if (spacePressed && state->isOnGround && !state->lastSpacePressed) {
        state->playerVelocityY = state->jumpVelocity;
        state->isOnGround = false;
        state->isJumping = true;
        state->jumpHoldTime = 0.0f;
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
    }
    



    bool landed = checkPlatformCollision(state, playerY, playerX, playerH, playerW, deltaTime, state->playerVelocityY, state->playerVelocityX);


    state->isOnGround = landed;




    //
    state->lastSpacePressed = spacePressed;

    // player 座標更新
    state->Player->SetPos(playerX, playerY);

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
    

    const float LANDING_TOLERANCE = 3.0f;

    float nextPlayerX = playerX + playerVelocityX * deltaTime;
    for (const auto& obj : state->sceneObjects) {
        float platformW = obj->GetW();
        float platformH = obj->GetH();
        float platformX = obj->GetPosX();
        float platformY = obj->GetPosY();

        bool overlapY = ((playerY + playerH) > platformY) && (playerY < (platformY + platformH));

        if (overlapY && playerVelocityX > 0 && (playerX + playerW <= platformX) && (nextPlayerX + playerW > platformX)) {
            float overlapLeft = nextPlayerX + playerW - platformX;
            float overlapRight = platformX + platformW - nextPlayerX;
            
            nextPlayerX = platformX - playerW;
            playerVelocityX = 0;
        }
        else if (overlapY && playerVelocityX < 0 &&
            (playerX >= platformX + platformW) && (nextPlayerX < platformX + platformW)) {
            
            nextPlayerX = platformX + platformW;
            playerVelocityX = 0;
        }
    }
    playerX = nextPlayerX;

    float nextPlayerY = playerY + playerVelocityY * deltaTime;
    bool landed = false;
    for (const auto& obj : state->sceneObjects) {
        float platformW = obj->GetW();
        float platformH = obj->GetH();
        float platformX = obj->GetPosX();
        float platformY = obj->GetPosY();

        bool overlapX = ((playerX + playerW) > platformX) && (playerX < (platformX + platformW));

        if (overlapX && playerVelocityY > 0 && (playerY + playerH <= platformY + LANDING_TOLERANCE) && (nextPlayerY + playerH >= platformY - LANDING_TOLERANCE)) {
            nextPlayerY = platformY - playerH;
            playerVelocityY = 0;
            landed = true;
        }
        else if (overlapX && playerVelocityY == 0 &&
            std::fabs(playerY + playerH - platformY) < LANDING_TOLERANCE) {
            playerY = platformY - playerH;
            landed = true;
        }
        else if (overlapX && playerVelocityY < 0 && (playerY >= platformY + platformH) && (nextPlayerY <= platformY + platformH)) {
            nextPlayerY = platformY + platformH;
            playerVelocityY = 0;
        }
    }
    playerY = nextPlayerY;

    if (
        (playerVelocityY > 0 && (playerY + playerH) < state->groundY + LANDING_TOLERANCE && (nextPlayerY + playerH) > state->groundY - LANDING_TOLERANCE) ||
        (playerVelocityY == 0 && std::fabs(playerY + playerH - state->groundY) < LANDING_TOLERANCE)
        ) {
        playerY = state->groundY - playerH;
        playerVelocityY = 0.0f;
        landed = true;
    }

    return landed;


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