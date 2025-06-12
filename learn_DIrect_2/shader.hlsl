// 顶点着色器的输入结构体
// 从顶点缓冲区传入的每个顶点数据，包括位置（POSITION）和颜色（COLOR）
struct VS_INPUT
{
    float3 pos : POSITION; // 3D 位置坐标（x, y, z）
    float4 col : COLOR; // 颜色值（r, g, b, a）
};

// 像素着色器的输入结构体
// 来自顶点着色器的输出，传递到像素着色器进行插值处理
struct PS_INPUT
{
    float4 pos : SV_POSITION; // 变换后的顶点位置，系统值语义
    float4 col : COLOR; // 插值后的颜色值
};

// 顶点着色器的主函数
// 作用：将 VS_INPUT 结构转换为 PS_INPUT 结构，送往像素着色器
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos, 1.0f); // 将 float3 位置扩展为 float4，齐次坐标（用于后续变换）
    output.col = input.col; // 保留颜色信息，直接传递
    return output; // 返回给图元装配阶段
}

// 像素着色器的主函数
// 作用：根据插值后的颜色输出最终像素颜色
float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.col; // 输出颜色，直接作为最终像素颜色返回
}