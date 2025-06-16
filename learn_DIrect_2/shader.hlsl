/*
    shader.hlsl

*/

cbuffer ConstantBuffer : register(b0)
{
    matrix worldMatrix; // 4x4 矩阵
    float2 screenSize;
    float4x4 worldMatrices[100]; // 预留100个世界矩阵（每个对应一个图形）实例化才需要目前没用
    float4 colors[100]; // 对应颜色数组 实例化才需要目前没用
    int objectCount;
};
//顶点着色器输入结构 VS_INPUT
struct VS_INPUT
{
    float3 pos : POSITION; // スクリーン座標（ピクセル）
    float4 col : COLOR;
};
//像素着色器输入结构 PS_INPUT
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};
//顶点着色器主函数 VSMain
PS_INPUT VSMain(VS_INPUT input)
{
    //把 3D 的 pos 转成 4D float4 是为了乘以 4x4 矩阵
    PS_INPUT output;

    float4 localPos = float4(input.pos, 1.0f);

    // 用 worldMatrix 对顶点坐标进行平移、旋转、缩放变换。
    float4 worldPos = mul(localPos, worldMatrix);

    // 再把 worldPos 从像素坐标转换为 NDC 坐标
    float x = worldPos.x / screenSize.x * 2.0f - 1.0f;
    float y = 1.0f - worldPos.y / screenSize.y * 2.0f;

    output.pos = float4(x, y, worldPos.z, 1.0f);
    output.col = input.col;
    //
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    //直接输出从顶点着色器传过来的颜色, 每个像素的颜色就是顶点插值得来的颜色。
    return input.col;
}