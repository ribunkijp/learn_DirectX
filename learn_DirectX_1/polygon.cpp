/*==============================================================================

   ポリゴン描画 [polygon.cpp]
														  Author : Youhei Sato
														  Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "debug_ostream.h"


static constexpr int NUM_VERTEX = 15; // 頂点数


static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static ID3D11InputLayout* g_pInputLayout = nullptr;// 入力レイアウト

// 参照カウンタの管理のためRelease用
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;




// 頂点構造体
struct Vertex
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT4 color;
};


void Polygon_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Polygon_Initialize() : 無効なデバイスまたはコンテキストが渡されました" << std::endl;
		return;
	}

	// デバイスとコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// D3D11_APPEND_ALIGNED_ELEMENTを指定すると、前の要素からオフセットを自動計算してくれるため便利です
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	ID3DBlob* pVSBlob = Shader_GetVertexShaderBlob(); // 仮の関数
	if (pVSBlob)
	{
		g_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
			pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),
			&g_pInputLayout);
	}
}

void Polygon_Finalize(void)
{
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Polygon_Draw(void)
{
	// シェーダー開始
	Shader_Begin();

	// 頂点バッファセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// 画面サイズ取得
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	// 直交射影行列セット
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));


	// 入力レイアウトをセット
	g_pContext->IASetInputLayout(g_pInputLayout);

	// 頂点バッファセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 頂点バッファのマッピング
	D3D11_MAPPED_SUBRESOURCE msr;
	Vertex* v = (Vertex*)msr.pData;


	// プリミティブトポロジ設定（ラインリスト）
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);


	// 頂点座標設定（画面全体のライン）
	v[0].position = { SCREEN_WIDTH * 0.1f, SCREEN_HEIGHT * 0.1F, 0.0f };v[0].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	v[1].position = { SCREEN_WIDTH * 0.3f, SCREEN_HEIGHT * 0.1f, 0.0f }; v[1].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	v[2].position = { SCREEN_WIDTH * 0.1f, SCREEN_HEIGHT * 0.3F, 0.0f }; v[2].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	v[3].position = { SCREEN_WIDTH * 0.3f, SCREEN_HEIGHT * 0.3F, 0.0f }; v[3].color = { 0.0f, 1.0f, 0.0f, 1.0f };

	v[4].position = { SCREEN_WIDTH * 0.6f, SCREEN_HEIGHT * 0.6F, 0.0f }; v[4].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	v[5].position = { SCREEN_WIDTH * 0.8f, SCREEN_HEIGHT * 0.6f, 0.0f }; v[5].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	v[6].position = { SCREEN_WIDTH * 0.6f, SCREEN_HEIGHT * 0.8F, 0.0f }; v[6].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	v[7].position = { SCREEN_WIDTH * 0.8f, SCREEN_HEIGHT * 0.8F, 0.0f }; v[7].color = { 0.0f, 1.0f, 0.0f, 1.0f };

	v[8].position = { SCREEN_WIDTH * 0.6f, SCREEN_HEIGHT * 0.3f, 0.0f }; v[8].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	v[9].position = { SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT * 0.1F, 0.0f }; v[9].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	v[10].position = { SCREEN_WIDTH * 0.8f, SCREEN_HEIGHT * 0.3F, 0.0f }; v[10].color = { 0.0f, 0.0f, 1.0f, 1.0f };

	v[11].position = { SCREEN_WIDTH * 0.2f, SCREEN_HEIGHT * 0.6f, 0.0f }; v[11].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	v[12].position = { SCREEN_WIDTH * 0.4f, SCREEN_HEIGHT * 0.6F, 0.0f }; v[12].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	v[13].position = { SCREEN_WIDTH * 0.1f, SCREEN_HEIGHT * 0.8F, 0.0f }; v[13].color = { 1.0f, 0.0f, 1.0f, 1.0f };
	v[14].position = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.8F, 0.0f }; v[14].color = { 1.0f, 1.0f, 0.0f, 1.0f };
	
	// アンマップ
	g_pContext->Unmap(g_pVertexBuffer, 0);

	//
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 描画
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_pContext->Draw(4, 0);

	g_pContext->Draw(4, 4);

	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pContext->Draw(3, 8);

	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_pContext->Draw(4, 11);
}
