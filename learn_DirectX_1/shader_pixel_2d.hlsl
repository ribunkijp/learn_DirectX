/*==============================================================================

   2D描画用ピクセルシェーダー [shader_pixel_2d.hlsl]
                                                          Author : Youhei Sato
                                                          Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
// 定数バッファ
cbuffer cb : register(b0)
{
    float4x4 mtx;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

VS_OUT VS_Main(float4 posL : POSITION, float4 color : COLOR)
{
    VS_OUT output;
    output.pos = mul(mtx, posL); // 正しい順番！
    output.col = color;
    return output;
}