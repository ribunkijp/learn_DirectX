/*
    d3dApp.cpp

*/

#include "d3dApp.h"
#include "StateInfo.h"
#include "ObjectFactory.h"




//
std::vector<GameObject> sceneObjects;



bool InitD3D(HWND hwnd, StateInfo* state, float clientWidth, float clientHeight) {


    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity(); // 先用单位矩阵，你可以设置摄像机的位置后再更新
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, clientWidth,      // left, right
        clientHeight, 0.0f,      // bottom, top ← 注意顺序！
        0.0f, 1.0f                                   // nearZ, farZ
    );


    /*
        // 初始化交换链描述结构体（DXGI_SWAP_CHAIN_DESC）
        // 交换链（Swap Chain）是 DirectX 实现双缓冲或多缓冲的机制。
        // 它包含一个或多个“后备缓冲区”（Back Buffers），渲染时画面先画到这些缓冲区，
        // 然后调用 Present() 把当前缓冲区“交换”到前台显示。

        // DXGI_SWAP_CHAIN_DESC 是用来配置交换链的结构体，
        // 包括：缓冲区数量、分辨率、格式、窗口句柄、全屏/窗口模式、抗锯齿等设置。
        //
        // 此结构体将作为参数传入 D3D11CreateDeviceAndSwapChain 函数，
        // 同时创建以下关键对象：
        // - ID3D11Device（显卡接口，资源创建用）
        // - ID3D11DeviceContext（设备上下文，发命令用）
        // - IDXGISwapChain（交换链接口，管理缓冲区显示）
        //
    */
    DXGI_SWAP_CHAIN_DESC scd = {};

    // 后备缓冲区数量（只使用1个）
    scd.BufferCount = 1;

    // 绘制画面的宽度和高度（分辨率）
    scd.BufferDesc.Width = static_cast<unsigned>(clientWidth);
    scd.BufferDesc.Height = static_cast<unsigned>(clientHeight);

    // 缓冲区的颜色格式（RGBA，每个8位的标准格式）
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // 缓冲区的用途（作为渲染目标使用）缓冲区将作为渲染目标绑定到渲染管线中
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // 输出目标的窗口句柄
    scd.OutputWindow = hwnd;

    // 多重采样（抗锯齿）设置（1：无效）
    scd.SampleDesc.Count = 1;

    // 启用窗口模式（TRUE：非全屏）
    scd.Windowed = TRUE;

    // 创建 Direct3D 设备和交换链和命令上下文
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,                        // 使用默认显卡（适配器）
        D3D_DRIVER_TYPE_HARDWARE,      // 使用硬件加速（最快，也是默认）
        nullptr,                        // 无软件渲染 DLL（因为你用的是硬件驱动）
        0,                              // 不设置 debug 标志（正式版设为 0）
        nullptr, 0,                     // 不指定功能级别（会自动选择）
        D3D11_SDK_VERSION,             // Direct3D SDK 的版本宏（必须这样写）
        &scd,                          // 你准备好的交换链配置（DXGI_SWAP_CHAIN_DESC）
        &state->swapChain,             // 输出：交换链接口指针
        &state->device,                // 输出：设备指针（GPU 设备对象）
        nullptr,                       // 不关心功能级别，传 nullptr 即可
        &state->context)))            // // 输出：设备上下文（用于提交渲染命令）
        return false; // 失败返回false


    // 获取后备缓冲区的纹理（绘制目标）
    ID3D11Texture2D* backBuffer = nullptr;
    /*
        第一个参数0 表示缓冲区索引 第一个缓冲区;
        第二个参数 uuidof(ID3D11Texture2D) 指明你想得到的接口类型是 ID3D11Texture2D，也就是2D纹理;
        第三个参数(void**)&backBuffer 是输出指针，GetBuffer 会把后备缓冲区纹理的接口指针写进这里
        需要拿到它的纹理接口，后续才能创建渲染目标视图（Render Target View）绑定到它，从而告诉GPU“这就是我们渲染的目标”。
    */
    HRESULT hr = state->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr) || backBuffer == nullptr) {
        return false; // 失败时中断初始化
    }

    // 为后备缓冲区创建渲染目标视图 通过设置 RTV，把 GPU 的“绘画目标”指向后台缓冲区。
    hr = state->device->CreateRenderTargetView(backBuffer, nullptr, &state->rtv);
    if (FAILED(hr)) {
        return false; // 这里也要检查失败
    }

    // 释放后备缓冲区（视图已经创建完成）
    backBuffer->Release();





    // !!! 新增: 创建深度/模板缓冲区 !!!
    ID3D11Texture2D* depthStencilBuffer = nullptr;
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = static_cast<unsigned>(clientWidth);
    depthBufferDesc.Height = static_cast<unsigned>(clientHeight);
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24位深度 + 8位模板
    depthBufferDesc.SampleDesc.Count = 1; // 与RTV的SampleDesc.Count一致
    depthBufferDesc.SampleDesc.Quality = 0; // 与RTV的SampleDesc.Quality一致
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 绑定为深度/模板缓冲区
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    hr = state->device->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil buffer.", L"Error", MB_OK);
        return false;
    }

    // !!! 新增: 创建深度/模板视图 !!!
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 视图格式与缓冲区格式匹配
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // 2D 纹理的视图
    dsvDesc.Texture2D.MipSlice = 0; // 只使用第一个 mipmap 级别

    hr = state->device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &state->depthStencilView);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create depth stencil view.", L"Error", MB_OK);
        if (depthStencilBuffer) depthStencilBuffer->Release(); // 创建视图失败也要释放缓冲区
        return false;
    }
    if (depthStencilBuffer) depthStencilBuffer->Release(); // 视图创建成功后，缓冲区本身可以释放，视图会持有引用




    // 将创建的渲染目标视图设置到GPU（指定绘制目标）
    state->context->OMSetRenderTargets(1, &state->rtv, state->depthStencilView);

    // 设置视口（绘制区域）
    D3D11_VIEWPORT vp = {};
    vp.Width = clientWidth;       // 视口宽度（与窗口宽度一致）
    vp.Height = clientHeight;     // 视口高度（与窗口高度一致）
    vp.MinDepth = 0.0f;                  // 最小深度（深度缓冲区最小值）
    vp.MaxDepth = 1.0f;                  // 最大深度（深度缓冲区最大值）
    state->context->RSSetViewports(1, &vp); // 设置光栅化阶段的视口

    /*
            把 .hlsl 源代码编译成 GPU 可以执行的二进制字节码（Bytecode）。
    */
    // 1. 编译顶点着色器和像素着色器
    ID3DBlob* vsBlob = nullptr;         // 顶点着色器二进制数据存储
    ID3DBlob* psBlob = nullptr;         // 像素着色器二进制数据存储

    // 编译顶点着色器（VS)
    hr = D3DCompileFromFile(
        L"shader.hlsl",                 // HLSL文件路径
        nullptr, nullptr,              // 不使用宏和包含
        "VSMain", "vs_5_0",            // 入口函数名和着色器模型
        0, 0,
        &vsBlob, nullptr               // 存储编译结果（成功时）
    );
    if (FAILED(hr)) return false;      // 失败时退出
    //编译像素着色器（PS）
    hr = D3DCompileFromFile(
        L"shader.hlsl",
        nullptr, nullptr,
        "PSMain", "ps_5_0",
        0, 0,
        &psBlob, nullptr
    );
    if (FAILED(hr)) {
        vsBlob->Release();             // 释放资源
        return false;
    }

    // 2. 创建着色器对象（转换为GPU可用的着色器）
    hr = state->device->CreateVertexShader(
        vsBlob->GetBufferPointer(),    // 二进制指针
        vsBlob->GetBufferSize(),       // 二进制大小
        nullptr,
        &state->vertexShader           // 结果存储
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


    // 3. 创建输入布局（顶点缓冲区中的数据如何映射到顶点着色器的输入)
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        // POSITION: 3个float（x, y, z）
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},

        // COLOR: 4个float（r, g, b, a）
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}  // 28是前3个float+前4个float的字节大小
    };

    hr = state->device->CreateInputLayout(
        layout,      // 布局数组
        3,           // 数组长度
        vsBlob->GetBufferPointer(), // 用来检查布局是否匹配 VS
        vsBlob->GetBufferSize(),
        &state->inputLayout         // 输出结果（用于绑定到渲染管线）
    );
    vsBlob->Release();
    psBlob->Release();
    if (FAILED(hr)) return false;


    // 常量缓冲区（Constant Buffer）
    //初始化一个描述结构体，准备告诉 GPU 我们要创建一个什么样的缓冲区。
    D3D11_BUFFER_DESC cbd = {};
    //设置这个缓冲区的“用途”DYNAMIC 表示：CPU 会频繁改数据（如每帧传入新的矩阵），GPU 会读取。
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    //将缓冲区的大小 (以字节为单位) 设置为与你的 ConstantBuffer struct 匹配
    cbd.ByteWidth = sizeof(ConstantBuffer);
    //告诉 GPU：这个缓冲区是用作常量缓冲区，也就是要绑定到着色器里
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    //表示 CPU 可以写入这个缓冲区（比如更新变换矩阵）  这和 D3D11_USAGE_DYNAMIC 是配套的。只有 DYNAMIC 才能设置这个。
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //高级功能（结构化缓冲区等）才用到这个。
    cbd.MiscFlags = 0;
    //结构化缓冲区（Structured Buffer）才用
    cbd.StructureByteStride = 0;
   


    // --- 创建纹理采样器状态 ---
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 线性过滤，适用于大部分情况
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // U 坐标超出范围时重复纹理
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // V 坐标超出范围时重复纹理
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // W 坐标超出范围时重复纹理 (对2D纹理影响不大)
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = state->device->CreateSamplerState(&sampDesc, &state->samplerState);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create sampler state.", L"Error", MB_OK);
        return false;
    }

    // --- 创建并设置透明混合状态 ---
    D3D11_BLEND_DESC blendDesc = {};
    // RenderTarget[0] 对应第一个渲染目标
    blendDesc.RenderTarget[0].BlendEnable = TRUE; // 启用混合
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 源混合因子：源像素的 alpha 值
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 目标混合因子：1 - 源像素的 alpha 值
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // 混合操作：源 + 目标
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // Alpha 源混合因子 (通常为1)
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // Alpha 目标混合因子 (通常为0)
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // Alpha 混合操作 (通常为加法)
    // D3D11_COLOR_WRITE_ENABLE_ALL 表示所有颜色通道 (RGBA) 都可以写入
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = state->device->CreateBlendState(&blendDesc, &state->blendState);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create blend state.", L"Error", MB_OK);
        return false;
    }

    // 创建用于透明物体的深度/模板状态 
    D3D11_DEPTH_STENCIL_DESC transparentDepthStencilDesc = {};
    transparentDepthStencilDesc.DepthEnable = TRUE; // 仍然启用深度测试 (与不透明物体比较)
    //// **禁用深度写入**不让透明物体“覆盖”后面的物体的 Z 值，否则会挡住后面本该看到的东西
    transparentDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
    transparentDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    transparentDepthStencilDesc.StencilEnable = FALSE;

    hr = state->device->CreateDepthStencilState(&transparentDepthStencilDesc, &state->depthStencilStateTransparent);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create transparent depth stencil state.", L"Error", MB_OK);
        return false;
    }






    float offset[2] = { 0.0f, 0.0f };
    float scale[2] = { 1.0f, 1.0f };
    GameObject bg = CreateTexture(
        state->device,
        L"assets\\bg.dds",
        0.0f, 0.0f, clientWidth, clientHeight,
        false, 1,
        offset, scale,
        1, 1, 8.0f,
        view, projection);
    sceneObjects.push_back(bg);
    GameObject mario = CreateTexture(
        state->device, 
        L"assets\\mario.dds", 
        200.0f, 100.0f, 500.0f, 400.0f, 
        false, 
        1, 
        offset, 
        scale, 
        1, 
        1, 
        8.0f,
        view, projection);
    sceneObjects.push_back(mario);
    GameObject peach = CreateTexture(
        state->device, 
        L"assets\\peach.dds", 
        600.0f, 200.0f, 1020.0f, 500.0f, 
        false, 
        1, 
        offset, 
        scale, 
        1, 
        1, 
        8.0f,
        view, projection);
    sceneObjects.push_back(peach);

    scale[0] = 1.0f / 8.0f;
    GameObject runningman000 = CreateTexture(
        state->device, 
        L"assets\\runningman000.dds", 
        300.0f, 600.0f, 600.0f, 900.0f, 
        true, 
        8, 
        offset, 
        scale, 
        8, 
        1, 
        8.0f,
        view, projection);
    sceneObjects.push_back(runningman000);


   scale[0] = 1.0f / 10.0f;
    scale[1] = 1.0f / 2.0f;
    GameObject runningman003 = CreateTexture(
        state->device, 
        L"assets\\runningman003.dds", 
        700.0f, 600.0f, 1000.0f, 900.0f, 
        true, 
        10, 
        offset, 
        scale, 
        5, 
        2, 
        8.0f,
        view, projection);
    sceneObjects.push_back(runningman003);
   



    return true; // 成功时返回true
}





// 释放 Direct3D 资源的函数
void CleanupD3D(StateInfo* state) {
    // 确保所有资源在使用前已被释放（防止野指针操作）
    // 通常按照创建的逆序释放是安全的，但关键是全部释放

    // 1. 释放场景中的 GameObject 相关的资源
    // 遍历 GameObject 列表，释放每个物体持有的 D3D 资源
    for (auto& obj : sceneObjects) {
        if (obj.vertexBuffer) {
            obj.vertexBuffer->Release();
            obj.vertexBuffer = nullptr; // 释放后置为nullptr是个好习惯
        }
        if (obj.indexBuffer) {
            obj.indexBuffer->Release();
            obj.indexBuffer = nullptr;
        }
        if (obj.textureSRV) { // 释放纹理资源视图
            obj.textureSRV->Release();
            obj.textureSRV = nullptr;
        }
    }
    sceneObjects.clear(); // 清空存储 GameObject 的 vector

    // 2. 释放 StateInfo 中全局持有的 D3D 资源
    if (state->samplerState) { // 释放采样器状态
        state->samplerState->Release();
        state->samplerState = nullptr;
    }
    if (state->pixelShader) { // 释放像素着色器
        state->pixelShader->Release();
        state->pixelShader = nullptr;
    }
    if (state->vertexShader) { // 释放顶点着色器
        state->vertexShader->Release();
        state->vertexShader = nullptr;
    }
    if (state->inputLayout) { // 释放输入布局
        state->inputLayout->Release();
        state->inputLayout = nullptr;
    }
    if (state->rtv) { // 释放渲染目标视图
        state->rtv->Release();
        state->rtv = nullptr;
    }
    if (state->blendState) { // 释放混合状态
        state->blendState->Release();
        state->blendState = nullptr;
    }


    // 释放设备上下文和交换链之前，确保所有挂起的操作都已完成
    // 通常在释放设备之前，应该先释放上下文，并确保没有其他操作正在进行
    if (state->context) {
        state->context->Release();
        state->context = nullptr;
    }

    if (state->swapChain) {
        state->swapChain->Release();
        state->swapChain = nullptr;
    }

    // 最后释放设备对象
    // 在调试模式下，可以检查是否有未释放的 COM 接口
#ifdef _DEBUG
    ID3D11Debug* debug = nullptr;
    if (SUCCEEDED(state->device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug))) {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        debug->Release();
    }
#endif

    if (state->device) {
        state->device->Release();
        state->device = nullptr;
    }

}



void OnResize(StateInfo* state, UINT width, UINT height)
{
    if (!state || !state->swapChain || !state->device || !state->context) return;

    // 释放旧的资源
    if (state->rtv) { state->rtv->Release(); state->rtv = nullptr; }
    if (state->depthStencilView) { state->depthStencilView->Release(); state->depthStencilView = nullptr; }

    // Resize 交换链 buffers
    state->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    // 获取新的 back buffer
    ID3D11Texture2D* backBuffer = nullptr;
    state->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    state->device->CreateRenderTargetView(backBuffer, nullptr, &state->rtv);
    backBuffer->Release();

    // 创建新的深度缓冲区
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
    state->device->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer);

    // 创建深度视图
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthBufferDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    state->device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &state->depthStencilView);
    depthStencilBuffer->Release();

    // 重新绑定渲染目标
    state->context->OMSetRenderTargets(1, &state->rtv, state->depthStencilView);

    // 更新视口
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    state->context->RSSetViewports(1, &vp);

    // ✅ 若你还用 projection 矩阵，也要重新计算
    for (auto& obj : sceneObjects) {
        obj.constantBufferData.projection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f, 0.0f, 1.0f);
    }
}