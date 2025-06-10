/*==============================================================================

   Direct3Dの初期化と管理 [direct3d.cpp]
                                                         Author : Youhei Sato
                                                         Date   : 2025/05/12
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>

#ifndef DIRECT3D_H
#define DIRECT3D_H

#include <Windows.h>

// セーフリリースマクロ
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }

bool Direct3D_Initialize(HWND hWnd);  // Direct3Dの初期化
void Direct3D_Finalize();             // Direct3Dの終了処理

void Direct3D_Clear();                // バックバッファのクリア
void Direct3D_Present();              // バックバッファの表示

//バックバッファの大きさを取得
unsigned int Direct3D_GetBackBufferWidth();//幅
unsigned int Direct3D_GetBackBufferHeight();//高さ

//デバイスの取得
ID3D11Device* Direct3D_GetDevice();

//デバイスコンテキストの取得
ID3D11DeviceContext* Direct3D_GetContext();

#endif // DIRECT3D_H
