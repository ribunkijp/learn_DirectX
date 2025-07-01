/**********************************************************************************
    Update.cpp

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/

#include "Update.h"

void UpdateAllObjects(StateInfo* pState, float deltaTime) {
    for (auto& obj : pState->sceneObjects)
    {
        // 1. アニメーションデータの更新（フレーム切り替え）
        obj->Update(deltaTime);

        // 3. GPUへのアップロード
        obj->UpdateConstantBuffer(pState->context, pState->view, pState->projection);
    }
}

void UpdatePlayer(StateInfo* state, float deltaTime, bool leftPressed, bool rightPressed) {
    if (!state || !state->playerPtr) return;

    float playerX = state->playerPtr->GetPlayerPosX();
    float playerY = state->playerPtr->GetPlayerPosY();

    float speed = state->playerPtr->GetSpeed();

    if (leftPressed)  playerX -= speed * deltaTime;
    if (rightPressed) playerX += speed * deltaTime;

    // player 座標更新
    state->playerPtr->SetPlayerPos(playerX, playerY);
}

void UpdateCamera(StateInfo* state) {
    float playerX = state->playerPtr->GetPlayerPosX();
    float playerY = state->playerPtr->GetPlayerPosY();
    float playerW = state->playerPtr->GetPlayerW();
    float playerH = state->playerPtr->GetPlayerH();

    float halfW = state->logicalWidth * 0.5f;
    float halfH = state->logicalHeight * 0.5f;

    float cameraX = playerX + playerW * 0.5f - halfW;
    float cameraY = playerY + playerH * 0.5f - halfH;
    if (cameraX < 0.0f) cameraX = 0.0f;
    if (cameraY < 0.0f) cameraY = 0.0f;




    state->view = DirectX::XMMatrixTranslation(-cameraX, -cameraY, 0.0f);
}
