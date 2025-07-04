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

    if (leftPressed)  playerX -= speed * deltaTime;
    if (rightPressed) playerX += speed * deltaTime;



  


    bool landed = checkPlatformCollision(state, playerY, playerX, playerH, playerW, deltaTime, state->playerVelocityY);

    
    state->isOnGround = landed;
    
       
    


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
    

    playerY += state->playerVelocityY * deltaTime;






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

bool checkPlatformCollision(StateInfo* state, float& playerY, float& playerX, float playerH, float playerW, float deltaTime, float& playerVelocityY) {
    float nextPlayerY = playerY + playerVelocityY * deltaTime;
    float playerBottom = nextPlayerY + playerH;
    float playerTop = nextPlayerY;



    for (const auto& obj : state->sceneObjects) {
        float platformW = obj->GetW();
        float platformX = obj->GetPosX();
        float platformY = obj->GetPosY();

        bool aligned = playerX + playerW * 0.8f  > platformX && playerX + playerW * 0.2f < platformX + platformW;

        if (aligned && ((playerBottom < platformY + 4.0f && nextPlayerY + playerH >= platformY && playerVelocityY > 0) || (playerVelocityY == 0 && std::fabs((playerY + playerH) - platformY) < 0.1f))) {
            playerY = platformY - playerH;
            playerVelocityY = 0.0f;
            return true;
        }

        
    }

    if (
        (playerVelocityY > 0 && (playerY + playerH) < state->groundY && (nextPlayerY + playerH) > state->groundY) ||
        (playerVelocityY == 0 && (playerY + playerH) >= state->groundY)
        ) {
        playerY = state->groundY - playerH;
        playerVelocityY = 0.0f;
        return true;
    }

    return false;


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