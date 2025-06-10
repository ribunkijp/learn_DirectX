/*==============================================================================

   2D描画用頂点シェーダー [shader_vertex_2d.hlsl]
                                                          Author : Youhei Sato
                                                          Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/

// 定数バッファ (Constant Buffer)
// C++側から送られてくる行列を格納します
cbuffer ConstantBuffer : register(b0)
{
    float4x4 mtx;
}

// C++の頂点構造体と対応する入力構造体
struct VS_IN
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

// ピクセルシェーダーへ渡す出力構造体
struct VS_OUT
{
    float4 pos : SV_POSITION; // SV_POSITION は必須のセマンティクス
    float4 col : COLOR;
};


VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output;

    // 未定義の g_WorldViewProj の代わりに、cbufferで定義した mtx を使う
    // また、C++のDirectXMathは行主序行列なので、mul(vector, matrix) の順で計算する
    output.pos = mul(float4(input.pos, 1.0f), mtx);
    
    // 頂点カラーをそのままピクセルシェーダーへ渡す
    output.col = input.col;
    
    return output;
}