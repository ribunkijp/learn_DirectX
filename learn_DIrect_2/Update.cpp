/**********************************************************************************
    Update.cpp

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/

#include "Update.h"
#include <Windows.h>

void UpdateAllObjects(StateInfo* pState, float deltaTime) {

    pState->Player->Update(deltaTime);
    for (auto& obj : pState->sceneObjects)
    {
        // 1. アニメーションデータの更新（フレーム切り替え）
        obj->Update(deltaTime);
    }
}

void UpdatePlayer(StateInfo* state, float deltaTime, bool leftPressed, bool rightPressed, bool topPressed, bool bottomPressed, bool spacePressed) {
    if (!state || !state->Player) return;

    float playerX = state->Player->GetPosX();
    float playerY = state->Player->GetPosY();
    float playerW = state->Player->GetW();
    float playerH = state->Player->GetH();

    float speed = state->Player->GetSpeed();

    if (leftPressed)  playerX -= speed * deltaTime;
    if (rightPressed) playerX += speed * deltaTime;
    //if (topPressed)  PlayerY -= speed * deltaTime;
    //if (bottomPressed) PlayerY += speed * deltaTime;






    bool landed = checkPlatformCollision(state, playerY, playerX, playerH, playerW, deltaTime, state->playerVelocityY, state->isOnGround);

    if (landed) {
        state->isOnGround = true;
    }
    else {
        if ((playerY + playerH) > state->groundY) {
            playerY = state->groundY - playerH;
            state->playerVelocityY = 0.0;
            state->isOnGround = true;
        }
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
    

    playerY += state->playerVelocityY * deltaTime;






    //
    state->lastSpacePressed = spacePressed;

    // player 座標更新
    state->Player->SetPos(playerX, playerY);

}

void UpdateCamera(StateInfo* state) {
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
 

    // cameraY 平滑插值
    state->cameraY += (state->targetCameraY - state->cameraY) * 0.15f;
    
    state->cameraX = playerX + playerW * 0.5f - halfW;



    if (state->cameraX < 0.0f) state->cameraX = 0.0f;
    if (state->cameraY > 0.0f) state->cameraY = 0.0f;


    //// 构造输出字符串
    //char buf[256];
    //sprintf_s(buf, " targetCameraY= %.2f, cameraY = %.2f、　PlayerY = %.2f\n",
    //    state->targetCameraY, state->cameraY, playerY);

    ////// 输出到调试窗口
    //OutputDebugStringA(buf);
   

    state->view = DirectX::XMMatrixTranslation(-state->cameraX, -state->cameraY, 0.0f);
}

bool checkPlatformCollision(StateInfo* state, float& playerY, float& playerX, float playerH, float playerW, float deltaTime, float& playerVelocityY, bool& isOnGround) {
    float nextPlayerY = playerY + playerVelocityY * deltaTime;
    float playerBottom = nextPlayerY + playerH;
    float playerTop = nextPlayerY;

    for (const auto& obj : state->sceneObjects) {
        float platformW = obj->GetW();
        float platformX = obj->GetPosX();
        float platformY = obj->GetPosY();

        bool aligned = playerX + playerW * 0.5 > platformX && playerX + playerW * 0.5 < platformX + platformW;

        if (aligned && playerBottom >= platformY && playerY + playerH <= platformY + 4.0f && playerVelocityY > 0) {
            playerY = platformY - playerH;
            playerVelocityY = 0.0f;
            isOnGround = true;
            return true;
        }
    }

    return false;


}