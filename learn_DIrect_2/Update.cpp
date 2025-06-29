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
    float speed = 0.0f;
    if (leftPressed) {
        speed = -state->playerSpeed;
    }
    else if (rightPressed) {
        speed = state->playerSpeed;
    }
    state->playerX += speed * deltaTime;
    if (state->playerX < 0) state->playerX = 0;
    // 右端の判定を追加することも可能：
    // if (state->playerX > mapWidth - playerWidth) state->playerX = mapWidth - playerWidth;
}
