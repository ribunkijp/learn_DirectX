/*
    Update.cpp

*/

#include "Update.h"

#include "BufferUtils.h"



// 静态局部变量：保存动画状态
static float animationTimer = 0.0f;//累计时间
static int frameIndex = 0;//动画帧的编号




void UpdateAnimation(ConstantBuffer* cb, float deltaTime, int totalFrames) {

    animationTimer += deltaTime; //deltaTime 每一帧与上一帧之间的耗时（单位是秒）

    if (animationTimer >= (1.0f / 8.0f)) { // 每秒8帧
        frameIndex = (frameIndex + 1) % totalFrames;
        animationTimer = 0.0f;
    }

    float frameWidth = 1.0f / totalFrames;
    cb->texOffset[0] = frameIndex * frameWidth;
    cb->texOffset[1] = 0.0f;
    cb->texScale[0] = frameWidth;
    cb->texScale[1] = 1.0f;
}