/*
    Update.h

*/
#ifndef UPDATE_H
#define UPDATE_H

struct ConstantBuffer;





// 更新动画帧索引 & UV 的函数
void UpdateAnimation(ConstantBuffer* cb, float deltaTime, int totalFrames);



#endif 
