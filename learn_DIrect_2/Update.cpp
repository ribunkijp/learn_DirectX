/**********************************************************************************
    Update.cpp

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/

#include "Update.h"

void UpdateAllObjects(StateInfo* pState, float deltaTime) {

    pState->Player->Update(deltaTime);
    for (auto& obj : pState->sceneObjects)
    {
        // 1. アニメーションデータの更新（フレーム切り替え）
        obj->Update(deltaTime);
    }
}

void UpdatePlayer(StateInfo* state, float deltaTime, bool leftPressed, bool rightPressed, bool topPressed, bool bottomPressed) {
    if (!state || !state->Player) return;

    float playerX = state->Player->GetPosX();
    float playerY = state->Player->GetPosY();

    float speed = state->Player->GetSpeed();

    if (leftPressed)  playerX -= speed * deltaTime;
    if (rightPressed) playerX += speed * deltaTime;
    if (topPressed)  playerY -= speed * deltaTime;
    if (bottomPressed) playerY += speed * deltaTime;

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

    float deadZoneH = state->logicalHeight * 0.4;
    float deadZoneTop = state->cameraY + halfH - deadZoneH * 0.5;
    float deadZoneBottom = state->cameraY + halfH + deadZoneH * 0.5;

    float playerCenterY = playerY + playerH * 0.5f;
    if (playerCenterY < deadZoneTop) {
        state->cameraY -= (deadZoneTop - playerCenterY);
    }
    else if (playerCenterY > deadZoneBottom) {
        state->cameraY += (playerCenterY - deadZoneBottom);
    }

    state->cameraX = playerX + playerW * 0.5f - halfW;
    if (state->cameraX < 0.0f) state->cameraX = 0.0f;
    if (state->cameraY < 0.0f) state->cameraY = 0.0f;




    state->view = DirectX::XMMatrixTranslation(-state->cameraX, -state->cameraY, 0.0f);
}
