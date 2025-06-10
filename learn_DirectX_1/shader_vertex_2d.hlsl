/*==============================================================================

   2D描画用頂点シェーダー [shader_vertex_2d.hlsl]
                                                          Author : Youhei Sato
                                                          Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/

// 定数バッファ
float4x4 mtx;

struct VS_IN
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output;
    output.pos = mul(float4(input.pos, 1.0f), g_WorldViewProj); // 矩阵乘法
    output.col = input.col;
    return output;
}