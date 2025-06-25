/*
    Update.cpp

*/

#include "Update.h"

#include "BufferUtils.h"

#include <Windows.h>



void UpdateAnimation(GameObject& obj, float deltaTime) {

    obj.animationTimer += deltaTime; //deltaTime 每一帧与上一帧之间的耗时（单位是秒）

    if (obj.animationTimer >= (1.0f / obj.fps)) { // 每秒8帧
        obj.frameIndex = (obj.frameIndex + 1) % obj.totalFrames;
        obj.animationTimer = 0.0f;

    }

    // 计算单帧宽高（单位: 纹理UV坐标 0~1）
    float frameWidth = 1.0f / static_cast<float>(obj.columns);
    float frameHeight = 1.0f / static_cast<float>(obj.rows);

    // 计算帧的行列位置
    int col = obj.frameIndex % obj.columns;
    int row = obj.frameIndex / obj.columns;

    obj.texOffset[0] = static_cast<float>(col) * frameWidth;
    obj.texOffset[1] = static_cast<float>(row) * frameHeight;
    obj.texScale[0] = frameWidth;
    obj.texScale[1] = frameHeight;
}

void UpdateAllObjects(StateInfo* pState, float deltaTime, float width, float height) {
    for (auto& obj : sceneObjects)
    {   
        // 1. 更新动画数据（帧切换）
        if(obj.isAnimated)  UpdateAnimation(obj, deltaTime);


        // 3. 上传到 GPU
        UpdateConstantBuffer(pState->context, obj.constantBuffer, obj.modelMatrix, obj.texOffset, obj.texScale, pState);
    }
}