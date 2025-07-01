/**********************************************************************************
    StateInfo.h

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/

#ifndef STATEINFO_H
#define STATEINFO_H

#include <d3d11.h>//ID3D11Device
#include <DirectXMath.h>
#include <vector>
#include "GameObject.h"
#include <memory>

// Direct3D11のレンダリング状態を管理する構造体
struct StateInfo {
    // デバイスオブジェクト：リソースやシェーダーの作成に使う
    ID3D11Device* device = nullptr; //ID3D11Device（グラフィックカードインターフェース）

    // デバイスコンテキスト：描画コマンドやリソースのバインドに使用 //D3D11DeviceContext（コマンドインターフェース）
    ID3D11DeviceContext* context = nullptr;//レンダリングパイプライン


    /*
        スワップチェーン：バックバッファとフロントバッファの交換を管理（画面表示用）
        通常は1つ以上のバックバッファ（Back Buffers）を持つ。
        描画時はまずこれらのバッファにレンダリングし、Present() を呼ぶことで現在のバッファをフロントに“交換”して表示する。
    */
    IDXGISwapChain* swapChain = nullptr;

    // レンダーターゲットビュー：描画対象バッファのビューを指定
    ID3D11RenderTargetView* rtv = nullptr;

    // 入力レイアウト：頂点データの構造をGPUに伝える設定
    ID3D11InputLayout* inputLayout = nullptr;

    // 頂点シェーダー：各頂点の処理を行うシェーダー
    ID3D11VertexShader* vertexShader = nullptr;

    // ピクセルシェーダー：各ピクセルの色を決定するシェーダー
    ID3D11PixelShader* pixelShader = nullptr;

    // テクスチャサンプリング用のサンプラーステート
    ID3D11SamplerState* samplerState = nullptr;

    // 透過ブレンド用
    ID3D11BlendState* blendState = nullptr;
    // 透過物体用の深度ステンシル状態
    ID3D11DepthStencilState* depthStencilStateTransparent = nullptr;
    // 深度/ステンシルバッファビューのインターフェースポインタ。GPUが深度/ステンシルバッファへアクセス・操作するためのもの
    ID3D11DepthStencilView* depthStencilView = nullptr;
    //
    ID3D11RenderTargetView* renderTargetView = nullptr;
    //
    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX projection = DirectX::XMMatrixIdentity();
    //
    float logicalWidth = 1888.0f;
    float logicalHeight = 1062.0f;
    //
    std::vector<std::unique_ptr<GameObject>> sceneObjects;
    // カメラ
    float cameraX = 0.0f;
    float cameraY = 0.0f;
    // プレイヤー
    float playerX = 200.0f;  // キャラクターの初期ワールド座標
    float playerY = 900.0f;
    float playerSpeed = 300.0f; // 移動速度、ピクセル/秒

    // 背景
    std::unique_ptr<GameObject> bg;
    float bgOffsetX = 0.0f;

    //
    GameObject* playerPtr = nullptr;
};

#endif
