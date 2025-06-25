/*
    Update.h

*/
#ifndef UPDATE_H
#define UPDATE_H
#include "StateInfo.h"
#include "d3dApp.h"

struct ConstantBuffer;





// 更新动画帧索引 & UV 的函数
void UpdateAnimation(GameObject& obj, float deltaTime);

//
void UpdateAllObjects(StateInfo* pState, float deltaTime, float width, float height);

#endif 
