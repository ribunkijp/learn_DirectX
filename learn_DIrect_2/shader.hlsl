// shader.hlsl
cbuffer ConstantBuffer : register(b0)
{
    matrix worldMatrix; // 4x4 矩阵
    float2 screenSize;
    float4x4 worldMatrices[100]; // 预留100个世界矩阵（每个对应一个图形）
    float4 colors[100]; // 对应颜色数组
    int objectCount;
};

struct VS_INPUT
{
    float3 pos : POSITION; // スクリーン座標（ピクセル）
    float4 col : COLOR;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;

    float4 localPos = float4(input.pos, 1.0f);

    // 应用 worldMatrix（用于偏移位置）
    float4 worldPos = mul(localPos, worldMatrix);

    // 再把 worldPos 从像素坐标转换为 NDC 坐标
    float x = worldPos.x / screenSize.x * 2.0f - 1.0f;
    float y = 1.0f - worldPos.y / screenSize.y * 2.0f;

    output.pos = float4(x, y, worldPos.z, 1.0f);
    output.col = input.col;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.col;
}