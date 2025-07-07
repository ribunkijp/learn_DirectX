/**********************************************************************************
    d3dApp.cpp

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/

#include "d3dApp.h"
#include "StateInfo.h"
#include "GameObject.h"




bool InitD3D(HWND hwnd, StateInfo* state, float clientWidth, float clientHeight) {

    /*
        元のモデル座標（モデル中心が原点）
             ↓ worldMatrix（平行移動・拡大縮小）
        ワールド座標（左上が原点）
             ↓ viewMatrix（単位行列・カメラなし）
        ビュー座標
             ↓ projectionMatrix（正射投影）
        NDC座標（X/Y は [-1, 1]、左上は -1,+1 に）
             ↓ グラフィックスパイプラインによる自動マッピング
        画面ピクセル位置
    */
    // カメラ位置と向きを設定
    state->view = DirectX::XMMatrixIdentity(); // まずは単位行列、カメラ位置を設定したら更新できる
    // 3D/2D ワールドを画面にマッピング
    state->projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, state->logicalWidth,      // left から right：X軸は左から右へ
        state->logicalHeight, 0.0f,     // bottom から top：Y軸は上から下へ
        0.0f, 1.0f              // near から far：Z軸は手前から奥へ
    );


    /*
        // スワップチェーン記述構造体（DXGI_SWAP_CHAIN_DESC）を初期化
        // スワップチェーン（Swap Chain）は DirectX のダブルバッファ/マルチバッファ機構です。
        // 1つ以上の「バックバッファ」を持ち、描画時にまずそこへ描画し、
        // Present() を呼ぶことで現在のバッファを前面へ「交換」して表示します。

        // DXGI_SWAP_CHAIN_DESC はスワップチェーンの設定用構造体です。
        // バッファ数、解像度、フォーマット、ウィンドウハンドル、フルスクリーン/ウィンドウ、アンチエイリアス等を設定。
        //
        // この構造体は D3D11CreateDeviceAndSwapChain 関数に渡され、
        // 以下の主要オブジェクトが同時に作成されます：
        // - ID3D11Device（GPUデバイス、リソース作成用）
        // - ID3D11DeviceContext（デバイスコンテキスト、コマンド送信用）
        // - IDXGISwapChain（スワップチェーン、バッファ管理）
        //
    */
    DXGI_SWAP_CHAIN_DESC scd = {};

    // バックバッファ数（1つのみ使用）
    scd.BufferCount = 1;

    // 描画画面の幅・高さ（解像度）
    scd.BufferDesc.Width = static_cast<unsigned>(clientWidth);
    scd.BufferDesc.Height = static_cast<unsigned>(clientHeight);

    // バッファのカラーフォーマット（RGBA・各8bit 標準フォーマット）
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // バッファの用途（レンダーターゲットとして利用）
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // 出力先ウィンドウハンドル
    scd.OutputWindow = hwnd;

    // マルチサンプリング（アンチエイリアス）（1: 無効）
    scd.SampleDesc.Count = 1;

    // ウィンドウモードを有効に（TRUE: ウィンドウ表示）
    scd.Windowed = TRUE;

    // Direct3D デバイス・スワップチェーン・コマンドコンテキスト作成
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,                        // デフォルトGPU（アダプタ）を使用
        D3D_DRIVER_TYPE_HARDWARE,      // ハードウェアアクセラレーション（最速・デフォルト）
        nullptr,                        // ソフトウェアレンダDLLはなし（ハードのみ）
        0,                              // デバッグフラグなし（リリース時は0）
        nullptr, 0,                     // 機能レベル未指定（自動選択）
        D3D11_SDK_VERSION,             // Direct3D SDK バージョン
        &scd,                          // 準備済みスワップチェーン設定（DXGI_SWAP_CHAIN_DESC）
        &state->swapChain,             // 出力：スワップチェーン
        &state->device,                // 出力：デバイス
        nullptr,                       // 機能レベル不要
        &state->context)))            // 出力：コマンドコンテキスト
        return false; // 失敗時はfalse返却


    // バックバッファテクスチャ（描画先）の取得
    ID3D11Texture2D* backBuffer = nullptr;
    /*
        1つ目の引数0はバッファ番号・最初のバッファ;
        2つ目 uuidof(ID3D11Texture2D)で求めるインターフェース型を指定;
        3つ目 (void**)&backBuffer は出力用、GetBufferがここに取得先を書き込む
        バックバッファのテクスチャを取ることで、後でレンダーターゲットビュー作成やGPUへの描画先指示が可能となる。
    */
    HRESULT hr = state->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr) || backBuffer == nullptr) {
        return false; // 失敗時は初期化中断
    }

    // バックバッファ用レンダーターゲットビューの作成。RTVでGPUの描画先をバックバッファに指定
    hr = state->device->CreateRenderTargetView(backBuffer, nullptr, &state->rtv);
    if (FAILED(hr)) {
        return false; // 失敗もチェック
    }

    // バックバッファ解放（ビュー作成済みなのでOK）
    backBuffer->Release();



    // !!! 新規: 深度/ステンシルバッファ作成 !!!
    ID3D11Texture2D* depthStencilBuffer = nullptr;
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = static_cast<unsigned>(clientWidth);
    depthBufferDesc.Height = static_cast<unsigned>(clientHeight);
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24bit深度+8bitステンシル
    depthBufferDesc.SampleDesc.Count = 1; // RTVのSampleDesc.Countと同じ
    depthBufferDesc.SampleDesc.Quality = 0; // RTVのSampleDesc.Qualityと同じ
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 深度/ステンシルバッファ用
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    hr = state->device->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil buffer.", L"Error", MB_OK);
        return false;
    }

    // !!! 新規: 深度/ステンシルビュー作成 !!!
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = state->device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &state->depthStencilView);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil view.", L"Error", MB_OK);
        if (depthStencilBuffer) depthStencilBuffer->Release();
        return false;
    }
    if (depthStencilBuffer) depthStencilBuffer->Release(); // ビュー作成後は本体解放でOK


    // 作成したレンダーターゲットビューをGPUに設定（描画先の指定）
    state->context->OMSetRenderTargets(1, &state->rtv, state->depthStencilView);

    // ビューポート（描画範囲）の設定
    D3D11_VIEWPORT vp = {};
    vp.Width = clientWidth;       // ビューポート幅（ウィンドウ幅と一致）
    vp.Height = clientHeight;     // ビューポート高さ（ウィンドウ高と一致）
    vp.MinDepth = 0.0f;           // 最小深度
    vp.MaxDepth = 1.0f;           // 最大深度
    state->context->RSSetViewports(1, &vp); // ラスタライズ段階のビューポート指定

    /*
        .hlsl ソースをGPU用のバイトコードにコンパイル
    */
    // 1. 頂点シェーダ・ピクセルシェーダのコンパイル
    ID3DBlob* vsBlob = nullptr;         // 頂点シェーダバイナリ格納
    ID3DBlob* psBlob = nullptr;         // ピクセルシェーダバイナリ格納

    // 頂点シェーダ（VS）コンパイル
    hr = D3DCompileFromFile(
        L"shader.hlsl",                 // HLSLファイルパス
        nullptr, nullptr,
        "VSMain", "vs_5_0",
        0, 0,
        &vsBlob, nullptr
    );
    if (FAILED(hr)) return false;
    // ピクセルシェーダ（PS）コンパイル
    hr = D3DCompileFromFile(
        L"shader.hlsl",
        nullptr, nullptr,
        "PSMain", "ps_5_0",
        0, 0,
        &psBlob, nullptr
    );
    if (FAILED(hr)) {
        vsBlob->Release();
        return false;
    }

    // 2. シェーダオブジェクト作成（GPUで使える形式に変換）
    hr = state->device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &state->vertexShader
    );
    if (FAILED(hr)) {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    hr = state->device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &state->pixelShader
    );
    if (FAILED(hr)) {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    // 3. 入力レイアウト作成（頂点バッファのデータがVS入力にどう割当てられるかを定義）
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        // POSITION: float3（x, y, z）
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},

        // COLOR: float4（r, g, b, a）
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}  // 28はfloat3+float4のバイト数
    };

    hr = state->device->CreateInputLayout(
        layout,
        3,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &state->inputLayout
    );
    vsBlob->Release();
    psBlob->Release();
    if (FAILED(hr)) return false;

    // 定数バッファ（Constant Buffer）
    // 記述構造体を初期化し、GPUにどんなバッファを作るか伝える
    D3D11_BUFFER_DESC cbd = {};
    // 用途はDYNAMIC：CPUが毎フレーム値を更新（例：行列）、GPUが読み取る
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    // サイズはConstantBuffer構造体と同じ
    cbd.ByteWidth = sizeof(ConstantBuffer);
    // このバッファは定数バッファとして使う
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    // CPU書き込み許可（D3D11_USAGE_DYNAMICとセットで必要）
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // 高度な用途は0
    cbd.MiscFlags = 0;
    cbd.StructureByteStride = 0;

    // --- テクスチャサンプラーステート作成 ---
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 線形フィルタ・一般用途向き
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // U座標範囲外でリピート
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // V座標範囲外でリピート
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // W座標範囲外でリピート（2Dはあまり影響なし）
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = state->device->CreateSamplerState(&sampDesc, &state->samplerState);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create sampler state.", L"Error", MB_OK);
        return false;
    }

    // --- 透明ブレンドステート作成 ---
    D3D11_BLEND_DESC blendDesc = {};
    // RenderTarget[0] は最初のレンダーターゲット
    blendDesc.RenderTarget[0].BlendEnable = TRUE; // ブレンド有効
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // ソースのα値
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 1-ソースのα値
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // ソース+デスティネーション
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // α成分（通常1）
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // α成分（通常0）
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // α加算
    // D3D11_COLOR_WRITE_ENABLE_ALL は全色成分(RGBA)書込可
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = state->device->CreateBlendState(&blendDesc, &state->blendState);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create blend state.", L"Error", MB_OK);
        return false;
    }

    // 透明物体用の深度/ステンシルステート作成
    D3D11_DEPTH_STENCIL_DESC transparentDepthStencilDesc = {};
    transparentDepthStencilDesc.DepthEnable = TRUE; // 深度テストは有効（不透明物体との比較）
    //// **深度書き込み無効化** 透明物体が奥の物体のZ値を「塗りつぶす」のを防ぐ
    transparentDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    transparentDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    transparentDepthStencilDesc.StencilEnable = FALSE;

    hr = state->device->CreateDepthStencilState(&transparentDepthStencilDesc, &state->depthStencilStateTransparent);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create transparent depth stencil state.", L"Error", MB_OK);
        return false;
    }


    auto ground = std::make_unique<GameObject>();
    ground->SetPos(0.0f, 962.0f);
    ground->Load(
        state->device,
        L"assets\\kaidan.dds",
        4000.0f, 50.0f,
        false,
        1,
        1,
        1,
        0.0f
    );
    state->sceneObjects.push_back(std::move(ground));

    auto kaidan_01 = std::make_unique<GameObject>();
    kaidan_01->SetPos(550.0f, 700.0f);
    kaidan_01->Load(
        state->device,
        L"assets\\kaidan.dds",
        280.0f, 80.0f,
        false,
        1,
        1,
        1,
        0.0f
    );
    state->sceneObjects.push_back(std::move(kaidan_01));

    auto kaidan_02 = std::make_unique<GameObject>();
    kaidan_02->SetPos(900.0f, 480.0f);
    kaidan_02->Load(
        state->device,
        L"assets\\kaidan.dds",
        280.0f, 80.0f,
        false,
        1,
        1,
        1,
        0.0f
    );
    state->sceneObjects.push_back(std::move(kaidan_02));


    auto kaidan_03 = std::make_unique<GameObject>();
    kaidan_03->SetPos(1300.0f, 450.0f);
    kaidan_03->Load(
        state->device,
        L"assets\\kaidan.dds",
        280.0f, 80.0f,
        false,
        1,
        1,
        1,
        0.0f
    );
    state->sceneObjects.push_back(std::move(kaidan_03));


    auto kaidan_04 = std::make_unique<GameObject>();
    kaidan_04->SetPos(1600.0f, 200.0f);
    kaidan_04->Load(
        state->device,
        L"assets\\kaidan.dds",
        280.0f, 80.0f,
        false,
        1,
        1,
        1,
        0.0f
    );
    state->sceneObjects.push_back(std::move(kaidan_04));


    auto kaidan_05 = std::make_unique<GameObject>();
    kaidan_05->SetPos(450.0f, 300.0f);
    kaidan_05->Load(
        state->device,
        L"assets\\kaidan.dds",
        280.0f, 80.0f,
        false,
        1,
        1,
        1,
        0.0f
    );
    state->sceneObjects.push_back(std::move(kaidan_05));

    auto kaidan_06 = std::make_unique<GameObject>();
    kaidan_06->SetPos(150.0f, 100.0f);
    kaidan_06->Load(
        state->device,
        L"assets\\kaidan.dds",
        280.0f, 80.0f,
        false,
        1,
        1,
        1,
        0.0f
    );
    state->sceneObjects.push_back(std::move(kaidan_06));


 /*   state->Player = std::make_unique<GameObject>();
    state->Player->SetSpeed(200.0f);
    state->Player->SetPos(200.0f, 762.0f);
    state->Player->Load(
        state->device,
        L"assets\\robot_run.dds",
        200.0f, 200.0f,
        true,
        10,
        9,
        1,
        24.0f
    );*/

    state->Player = std::make_unique<GameObject>();
    state->Player->SetSpeed(200.0f);
    state->Player->SetPos(200.0f, 843.0f);
    state->Player->Load(
        state->device,
        L"assets\\robot_walk.dds",
        70.0f, 119.0f,
        false,
        0,
        0,
        0,
        24.0f
    );
   




   /* auto kodomo_run = std::make_unique<GameObject>();
    kodomo_run->Load(
        state->device,
        L"assets\\kodomo_run.dds",
        400.0f, 600.0f, 560.0f, 780.0f,
        true,
        12,
        6,
        2,
        24.0f
   );
    state->sceneObjects.push_back(std::move(kodomo_run));*/

    return true; // 成功時はtrue
}


// Direct3D リソース解放関数
void CleanupD3D(StateInfo* state) {

    if (!state) return;

    state->Player.reset();

    // 全リソースは使用前に解放しておくこと
    state->sceneObjects.clear(); // GameObjectデストラクタが呼ばれリソース解放


    // StateInfoが持つD3Dリソースを全て解放
    if (state->samplerState) { // サンプラーステート解放
        state->samplerState->Release();
        state->samplerState = nullptr;
    }
    if (state->pixelShader) { // ピクセルシェーダ解放
        state->pixelShader->Release();
        state->pixelShader = nullptr;
    }
    if (state->vertexShader) { // 頂点シェーダ解放
        state->vertexShader->Release();
        state->vertexShader = nullptr;
    }
    if (state->inputLayout) { // 入力レイアウト解放
        state->inputLayout->Release();
        state->inputLayout = nullptr;
    }
    if (state->rtv) { // レンダーターゲットビュー解放
        state->rtv->Release();
        state->rtv = nullptr;
    }
    if (state->blendState) { // ブレンドステート解放
        state->blendState->Release();
        state->blendState = nullptr;
    }


    // デバイスコンテキスト・スワップチェーン解放前に、全操作完了を確認
    // 通常はデバイス解放前にまずコンテキストを解放し、未完了の操作がないことを確認
    if (state->context) {
        state->context->Release();
        state->context = nullptr;
    }

    if (state->swapChain) {
        state->swapChain->Release();
        state->swapChain = nullptr;
    }

    // 最後にデバイス本体の解放
    // デバッグ時は未解放のCOMインターフェースがないか確認
    if (state->device) {

        // 【重要修正】デバッグコードは device有効のときのみ
#ifdef _DEBUG
        ID3D11Debug* debug = nullptr;
        if (SUCCEEDED(state->device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug)))
        {
            debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
            debug->Release();
        }
#endif
        // そしてデバイス本体の解放
        state->device->Release();
        state->device = nullptr;
    }

}


// ウィンドウサイズ変更時の処理
void OnResize(HWND hwnd, StateInfo* state, UINT width, UINT height)
{
    if (!state || !state->swapChain || !state->device || !state->context) return;

    // 既存リソースを解放
    if (state->rtv) { state->rtv->Release(); state->rtv = nullptr; }
    if (state->depthStencilView) { state->depthStencilView->Release(); state->depthStencilView = nullptr; }

    // スワップチェーンのバッファをリサイズ
    state->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    // 新しいバックバッファ取得
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = state->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (SUCCEEDED(hr) && backBuffer) {
        state->device->CreateRenderTargetView(backBuffer, nullptr, &state->rtv);
        backBuffer->Release();
    }
    else {
        MessageBox(hwnd, L"Failed to get back buffer during resize.", L"Error", MB_OK);
        return;
    }

    // 新しい深度バッファ作成
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = width;
    depthBufferDesc.Height = height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthStencilBuffer = nullptr;
    hr = state->device->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer);

    // 深度ビュー作成
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthBufferDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    if (SUCCEEDED(hr) && depthStencilBuffer) {
        state->device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &state->depthStencilView);
        depthStencilBuffer->Release();
    }
    else {
        MessageBox(hwnd, L"Failed to create depth stencil buffer during resize.", L"Error", MB_OK);
        return;
    }

    // 新しいレンダーターゲットを再バインド
    state->context->OMSetRenderTargets(1, &state->rtv, state->depthStencilView);

    //    ClearRenderTargetView でウィンドウ全体をクリア
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    state->context->RSSetViewports(1, &vp);
    // この方法が推奨


    state->projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, state->logicalWidth,
        state->logicalHeight, 0.0f,
        0.0f, 1.0f);

}
