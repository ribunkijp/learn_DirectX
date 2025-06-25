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