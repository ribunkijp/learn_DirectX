/*
    Update.cpp

*/

#include "Update.h"

#include "BufferUtils.h"

#include <Windows.h>



void UpdateAnimation(GameObject& obj, float deltaTime, int columns, int rows) {

    obj.animationTimer += deltaTime; //deltaTime 每一帧与上一帧之间的耗时（单位是秒）

    if (obj.animationTimer >= (1.0f / obj.fps)) { // 每秒8帧
        obj.frameIndex = (obj.frameIndex + 1) % obj.totalFrames;
        obj.animationTimer = 0.0f;

    }

    // 计算单帧宽高（单位: 纹理UV坐标 0~1）
    float frameWidth = 1.0f / obj.columns;
    float frameHeight = 1.0f / obj.rows;

    // 计算帧的行列位置
    int col = obj.frameIndex % obj.columns;
    int row = obj.frameIndex / obj.columns;

    obj.constantBufferData.texOffset[0] = col * frameWidth;
    obj.constantBufferData.texOffset[1] = row * frameHeight;
    obj.constantBufferData.texScale[0] = frameWidth;
    obj.constantBufferData.texScale[1] = frameHeight;
}

void UpdateAllObjects(StateInfo* pState, float deltaTime, float width, float height) {
    for (auto& obj : sceneObjects)
    {   
        // 1. 更新动画数据（帧切换）
        if(obj.isAnimated)  UpdateAnimation(obj, deltaTime, obj.columns, obj.rows);


        // 3. 上传到 GPU
        UpdateConstantBuffer(pState->context, obj.constantBuffer, obj.constantBufferData);
    }
}