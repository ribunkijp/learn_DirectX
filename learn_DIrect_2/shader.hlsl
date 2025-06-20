/*
    shader.hlsl

*/

cbuffer ConstantBuffer : register(b0)
{
    matrix worldMatrix; // 4x4 矩阵
    float2 screenSize;
    float2 texOffset;
    float2 texScale;
    float2 padding; // 保持16字节对齐
    
};
// 声明一个纹理对象 (Texture2D) 和一个采样器 (SamplerState)
// register(t0) 表示将纹理绑定到寄存器 t0
// register(s0) 表示将采样器绑定到寄存器 s0
Texture2D shaderTexture : register(t0);
SamplerState SamplerClamp : register(s0);

//顶点着色器输入结构 VS_INPUT
struct VS_INPUT
{
    float3 pos : POSITION; // スクリーン座標（ピクセル）
    float4 col : COLOR;
    float2 tex : TEXCOORD; // 纹理坐标输入
};

// 这个结构体是从顶点着色器传递到像素着色器的数据，其中的纹理坐标会被自动插值
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 tex : TEXCOORD; // 纹理坐标输出
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
    output.col = input.col;//将顶点颜色传递给像素着色器
    output.tex = input.tex * texScale + texOffset;
    //
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    
     // 使用采样器和插值后的纹理坐标从纹理中采样颜色
    float4 textureColor = shaderTexture.Sample(SamplerClamp, input.tex);

    // 你可以选择如何混合纹理颜色和顶点颜色
    // 只使用纹理颜色：
    return textureColor;
    //return float4(textureColor.a, textureColor.a, textureColor.a, 1.0f);

    // 纹理颜色与顶点颜色相乘（实现纹理的颜色着色）：
    // return textureColor * input.col;
    
    //直接输出从顶点着色器传过来的颜色, 每个像素的颜色就是顶点插值得来的颜色。
    //return input.col;
}