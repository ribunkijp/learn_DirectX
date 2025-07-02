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

    float PlayerX = state->Player->GetPosX();
    float PlayerY = state->Player->GetPosY();
    float PlayerH = state->Player->GetH();

    float speed = state->Player->GetSpeed();

    if (leftPressed)  PlayerX -= speed * deltaTime;
    if (rightPressed) PlayerX += speed * deltaTime;
    if (topPressed)  PlayerY -= speed * deltaTime;
    if (bottomPressed) PlayerY += speed * deltaTime;
    if (spacePressed && state->isOnGround) {
        state->playerVelocityY = state->jumpVelocity;
        state->isOnGround = false;
    }
    state->playerVelocityY += state->gravity * deltaTime;

    PlayerY += state->playerVelocityY * deltaTime;

    if ((PlayerY + PlayerH) > state->groundY) {
        PlayerY = state->groundY - PlayerH;
        state->playerVelocityY = 0.0;
        state->isOnGround = true;
    }

    // player 座標更新
    state->Player->SetPos(PlayerX, PlayerY);

}

void UpdateCamera(StateInfo* state) {
    float PlayerX = state->Player->GetPosX();
    float PlayerY = state->Player->GetPosY();
    float PlayerW = state->Player->GetW();
    float PlayerH = state->Player->GetH();

    float halfW = state->logicalWidth * 0.5f;
    float halfH = state->logicalHeight * 0.5f;


   
    
    
    
    state->cameraX = PlayerX + PlayerW * 0.5f - halfW;
    state->cameraY = PlayerY + PlayerH * 0.5 - halfH;
    //state->cameraY = 0.0f;

    /*char buf[128];
    sprintf_s(buf, "cameraX: %.2f, cameraY: %.2f\n", state->cameraX, state->cameraY);
    OutputDebugStringA(buf);*/

    if (state->cameraX < 0.0f) state->cameraX = 0.0f;
    if (state->cameraY < 0.0f) state->cameraY = 0.0f;
    


   /* char buf[256];
    sprintf_s(buf, "targetCameraY: %.2f, deadZoneTop: %.2f, deadZoneBottom: %.2f, PlayerCenterY: %.2f\n",
        targetCameraY, deadZoneTop, deadZoneBottom, PlayerCenterY);
    OutputDebugStringA(buf)*/;



    state->view = DirectX::XMMatrixTranslation(-state->cameraX, -state->cameraY, 0.0f);
}
