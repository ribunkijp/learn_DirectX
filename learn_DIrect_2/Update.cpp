/*
    Update.cpp

*/

#include "Update.h"




void UpdateAllObjects(StateInfo* pState, float deltaTime) {
    for (auto& obj : pState->sceneObjects)
    {   
        // 1. 更新动画数据（帧切换）
        obj->Update(deltaTime);


        // 3. 上传到 GPU
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
    // 可以加右边界判定：if (state->playerX > mapWidth - playerWidth) state->playerX = mapWidth - playerWidth;
}