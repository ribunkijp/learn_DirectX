/*
    main.cpp

*/


/*

WinMain（アプリの開始点）
└─ RegisterClass（ウィンドウクラスを登録）
└─ CreateWindowEx（ウィンドウ作成）
     └─ WM_CREATE メッセージが送られる（今回は処理なし）
└─ ShowWindow / UpdateWindow（ウィンドウを表示）
└─ メッセージループ開始（GetMessage → DispatchMessage）
     └─ WindowProc（各メッセージごとの処理関数）
          └─ WM_PAINT / WM_CLOSE / WM_DESTROY などの処理
*/



#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <new>
#include <d3d11.h>//ID3D11Device
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")//链接Direct3D 11 的静态链接库
#pragma comment(lib, "d3dcompiler.lib")//链接D3DCompiler API 的静态链接库
#include <DirectXMath.h>
#include <vector>
#include <DirectXTex.h>

//
struct alignas(16) GameObject {
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();  // 世界矩阵，表示物体的位置、旋转、缩放（默认单位矩阵）
    ID3D11Buffer* vertexBuffer = nullptr;  // 顶点缓冲区指针，存储物体的顶点数据（位置、颜色等）
    ID3D11Buffer* indexBuffer = nullptr;   // 索引缓冲区指针，用于顶点的索引绘制，减少顶点重复
    UINT vertexCount = 0;                   // 顶点数量，用于绘制调用时指定绘制的顶点个数
    UINT indexCount = 0;                    // 索引数量，用于绘制调用时指定绘制的索引个数
    ID3D11ShaderResourceView* textureSRV = nullptr;//用于存储此物体纹理的资源视图
};


//定义了将数据作为常量缓冲区传递给 GPU shaders 的布局
struct ConstantBuffer {
    //世界变换矩阵。这将按物体进行更新并发送到顶点着色器
    DirectX::XMMATRIX worldMatrix;  // 64 字节 (4x4 float)
    //一个数组，用于存储屏幕的宽度和高度。这在 shader 中用于将像素坐标转换为 Normalized Device Coordinates (NDC)。
    float screenSize[2]; // 屏幕宽高
    float padding[2];    // 保持16字节对齐
};
//
struct Vertex {
    DirectX::XMFLOAT3 position;  // 位置
    DirectX::XMFLOAT4 color;     // 颜色
    DirectX::XMFLOAT2 texCoord;  // 纹理坐标 (U, V)
};

// 管理 Direct3D11 渲染状态的结构体
struct StateInfo {
    // 设备对象：用于创建资源和着色器
    ID3D11Device* device = nullptr; //ID3D11Device（显卡接口）

    // 设备上下文：用于发出绘制命令和绑定资源 //D3D11DeviceContext（命令接口）
    ID3D11DeviceContext* context = nullptr;//渲染管线

    
    /*
        交换链：管理后台缓冲区与前台缓冲区的交换（用于屏幕显示）通常包含一个或多个后备缓冲区 (Back Buffers)，
        渲染时画面首先绘制到这些缓冲区，然后调用 Present() 将当前缓冲区“交换”到前台显示。
    */
    IDXGISwapChain* swapChain = nullptr;

    // 渲染目标视图：指定绘制目标缓冲区的视图
    ID3D11RenderTargetView* rtv = nullptr;

    // 输入布局：将顶点数据的结构传达给 GPU 的设置
    ID3D11InputLayout* inputLayout = nullptr;

    // 顶点着色器：对每个顶点进行处理的着色器
    ID3D11VertexShader* vertexShader = nullptr;

    // 像素着色器：用于决定每个像素颜色的着色器
    ID3D11PixelShader* pixelShader = nullptr;

    // 顶点缓冲区：存储用于绘制的顶点数据的缓冲区
    ID3D11Buffer* vertexBuffer = nullptr;

    // 存储索引数组
    ID3D11Buffer* indexBuffer = nullptr;

    //常量缓冲区 该数据传递给 shaders
    ID3D11Buffer* constantBuffer = nullptr;

    //用于纹理采样的采样器状态
    ID3D11SamplerState* samplerState = nullptr;
    
    // 用于透明混合
    ID3D11BlendState* blendState = nullptr;
    // 用于透明物体的深度状态
    ID3D11DepthStencilState* depthStencilStateTransparent = nullptr; 
    //深度/模板缓冲区视图 的接口指针  GPU 用来访问和操作深度/模板缓冲区的接口
    ID3D11DepthStencilView* depthStencilView = nullptr;

};

inline StateInfo* GetAppState(HWND hwnd);

bool InitD3D(HWND hwnd, StateInfo* state);

ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device, Vertex* vertices, unsigned vertices_count);

ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device);

void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX& worldMatrix, float screenWidth, float screenHeight);

std::vector<GameObject> sceneObjects;

HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** srv);

void CleanupD3D(StateInfo* state);

// 窗口过程函数
LRESULT CALLBACK WindowProc(
    HWND hwnd,//窗口句柄 一个具体窗口的“身份证”
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);

// 获取屏幕的宽度和高度（单位：像素）
unsigned screenWidth = (unsigned)GetSystemMetrics(SM_CXSCREEN);
unsigned screenHeight = (unsigned)GetSystemMetrics(SM_CYSCREEN);

// 程序入口
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance, //实例句柄 应用本身的“身份证”代表当前运行的应用程序实例,即你这个程序在内存中的一个副本
    _In_opt_ HINSTANCE hPrevInstance,//Windows 95 时代遗留参数，总是为 NULL
    _In_ PWSTR pCmdLine,//命令行参数字符串（不包含程序本身的路径）
    _In_ int nCmdShow //窗口显示的方式
) {
    (void)hPrevInstance; // 虽然不使用，但为了消除警告而显式引用
    (void)pCmdLine;      // 同上

    // 自定义的字符串
    const wchar_t CLASS_NAME[] = L"WIndow_1";

    //wc代表你要注册的窗口类 
    WNDCLASS wc = { };
    //设置窗口消息处理函数
    wc.lpfnWndProc = WindowProc;
    //// 设置窗口所属的应用实例
    wc.hInstance = hInstance;
    //// 这里设置窗口类名(自定义的字符串)
    wc.lpszClassName = CLASS_NAME;

    // 注册窗口类
    RegisterClass(&wc);

    //pState 指针将保存所有 Direct3D rendering state
    StateInfo* pState = new StateInfo();  // 用 new 进行初始化

    if (pState == NULL)
    {
        return 0;
    }

    // 创建窗口
    HWND hwnd = CreateWindowEx(
        0,                              // 可选窗口样式
        CLASS_NAME,                     // 窗口类名
        L"Window_1",                    // 窗口标题（L 表示 UTF-16 字符串）
        WS_OVERLAPPEDWINDOW,            // 窗口样式

        // 位置和大小
        0,
        0,
        (int)screenWidth,
        (int)screenHeight,

        NULL,       // 父窗口
        NULL,       // 菜单
        hInstance,  // 实例句柄
        pState      // 附加的应用数据 ← 这是 lpParam，可在 WM_CREATE 中获取
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // 初始化 Direct3D11
    if (!InitD3D(hwnd, pState)) {
        MessageBox(hwnd, L"初期化に失敗しました", L"エラー", MB_OK);
        return 0;
    }

    // 消息循环
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        //将虚拟键消息转换为字符消息。这对于键盘输入很重要。
        TranslateMessage(&msg);
        // 将消息分派到与目标窗口 (msg.hwnd) 关联的窗口过程 (WindowProc)。
        DispatchMessage(&msg);
    }

    return 0;
}

// 窗口过程
LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam, // 无符号整数类型
    LPARAM lParam  // 有符号整数或指针
) {
    // 保存 StateInfo 结构体的指针（结构体 = 数据的集合）
    StateInfo* pState = nullptr;

    if (uMsg == WM_CREATE)
    {
        // 从 CreateWindowEx 中传入的 pState 取出并保存在窗口中
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pState = reinterpret_cast<StateInfo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pState);
    }
    else
    {
        pState = GetAppState(hwnd);
    }

    switch (uMsg) {
        case WM_CLOSE:
            if (MessageBox(hwnd, L"really quit？", L"cancel", MB_OKCANCEL) == IDOK) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            if (pState) { // 确保 pState 是有效的指针
                CleanupD3D(pState);
            }
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            (void)hdc;

            pState = GetAppState(hwnd);


            /*
                这段代码是为了实现动态更新GPU端的常量缓冲区，使着色器能够获取当前窗口大小，
                保证渲染效果能正确适配窗口尺寸变化。
            */
            // 取得最新窗口大小（防止窗口大小变化不更新）
            RECT rect;
            //检索窗口客户区 (可绘制区域，不包括边框和标题栏) 的尺寸
            GetClientRect(hwnd, &rect);
            float width = static_cast<float>(rect.right - rect.left);
            float height = static_cast<float>(rect.bottom - rect.top);
            
        
            


            // 绑定渲染目标视图和深度/模板视图
            pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);


            // 清除背景色
            float clearColor[4] = { 0.0f, 0.3f, 0.0f, 1.0f };
            pState->context->ClearRenderTargetView(pState->rtv, clearColor);

            //清除深度和模板缓冲区。1.0f是深度的默认最远值。
            pState->context->ClearDepthStencilView(pState->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

            // 绑定渲染目标
            pState->context->OMSetRenderTargets(1, &pState->rtv, pState->depthStencilView);

            // 设置输入布局和着色器
            /*
                告诉GPU顶点数据长啥样，怎么读；

                告诉GPU顶点数据如何被顶点着色器处理；

                告诉GPU像素怎么被像素着色器处理。
            */
            //设置输入布局（Input Layout）
            pState->context->IASetInputLayout(pState->inputLayout);
            //绑定顶点着色器（Vertex Shader）到管线
            pState->context->VSSetShader(pState->vertexShader, nullptr, 0);
            //绑定像素着色器（Pixel Shader）到管线
            pState->context->PSSetShader(pState->pixelShader, nullptr, 0);
            //
            pState->context->VSSetConstantBuffers(0, 1, &pState->constantBuffer);
            // 作用是告诉 GPU 如何把顶点组织成图元来绘制。设置图元类型为三角形列表:D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
            pState->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // --- 绑定混合状态 ---
           // 第二个参数是一个常数数组，用于那些在混合描述中设置为 D3D11_BLEND_BLEND_FACTOR 或 D3D11_BLEND_INV_BLEND_FACTOR 的混合模式。
           // 如果你的混合因子是基于源 Alpha 的，这里通常用 nullptr 或全为1的数组。
           // 第三个参数是样本遮罩，通常为 0xffffffff。
            pState->context->OMSetBlendState(pState->blendState, nullptr, 0xffffffff);

            //设置用于透明的深度模板状态！这是关键！
            // 第二个参数(StencilRef)在这里不重要，设为0即可。
            pState->context->OMSetDepthStencilState(pState->depthStencilStateTransparent, 0);


            
            // 更新顶点缓冲区步幅 (stride)
            // 确保这里的 stride 与你的 Vertex 结构体大小一致
            UINT stride = sizeof(Vertex);
            //offset（偏移量）：从顶点缓冲区开始处偏移多少字节读取数据，这里是0，表示从缓冲区头开始。
            UINT offset = 0;

           

            for (auto& obj : sceneObjects)
            {
                ID3D11Buffer* vertexBuffers[] = { obj.vertexBuffer };
                // 设置顶点缓冲区
                pState->context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
                // 设置索引缓冲区 指定每个索引是一个 32 位无符号整数
                pState->context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);


                // --- 绑定纹理和采样器到像素着色器 ---
                // PSSetShaderResources(起始槽位, 视图数量, SRV数组指针)
                // t0 寄存器对应起始槽位 0
                pState->context->PSSetShaderResources(0, 1, &obj.textureSRV);
                // PSSetSamplers(起始槽位, 采样器数量, 采样器数组指针)
                // s0 寄存器对应起始槽位 0
                pState->context->PSSetSamplers(0, 1, &pState->samplerState);
                // --- 绑定结束 ---
                
                
                // 设置常量缓冲区，传入 obj.worldMatrix
                UpdateConstantBuffer(pState->context, pState->constantBuffer, obj.worldMatrix, width, height);

                // 绘制调用
                pState->context->DrawIndexed(obj.indexCount, 0, 0);
            }



            // 将后备缓冲区 (已完成渲染的缓冲区) 与前台缓冲区 (当前显示在屏幕上的缓冲区) 交换
            pState->swapChain->Present(1, 0);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    //switch 语句中未明确处理的任何消息，此行调用默认窗口过程
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



inline StateInfo* GetAppState(HWND hwnd)
{
    //从指定窗口（hwnd）中取出你之前用 SetWindowLongPtr 存进去的自定义指针或数据。
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    StateInfo* pState = reinterpret_cast<StateInfo*>(ptr);
    return pState;
}


bool InitD3D(HWND hwnd, StateInfo* state) {
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
    scd.BufferDesc.Width = screenWidth;
    scd.BufferDesc.Height = screenHeight;

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
        nullptr,                        // 适配器（nullptr＝默认）
        D3D_DRIVER_TYPE_HARDWARE,      // 使用硬件
        nullptr, 0,                    // 无软件设置
        nullptr, 0,                    // 无功能级别设置（默认）
        D3D11_SDK_VERSION,             // SDK版本
        &scd,                         // 交换链设置
        &state->swapChain,            // 接收交换链
        &state->device,               // 接收设备
        nullptr,                     // 不需要实际功能级别，传nullptr
        &state->context)))            // 接收设备上下文
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

    // 为后备缓冲区创建渲染目标视图
    hr = state->device->CreateRenderTargetView(backBuffer, nullptr, &state->rtv);
    if (FAILED(hr)) {
        return false; // 这里也要检查失败
    }

    // 释放后备缓冲区（视图已经创建完成）
    backBuffer->Release();




    // !!! 新增: 创建深度/模板缓冲区 !!!
    ID3D11Texture2D* depthStencilBuffer = nullptr;
    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    depthBufferDesc.Width = screenWidth;
    depthBufferDesc.Height = screenHeight;
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
    vp.Width = (FLOAT)screenWidth;       // 视口宽度（与窗口宽度一致）
    vp.Height = (FLOAT)screenHeight;     // 视口高度（与窗口高度一致）
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
    //创建常量缓冲区（Constant Buffer）在渲染过程中将数据（比如变换矩阵）从 CPU 传给 GPU 的着色器。
    hr = state->device->CreateBuffer(&cbd, nullptr, &state->constantBuffer);
    if (FAILED(hr)) return false;


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

    // !!! 新增: 创建用于透明物体的深度/模板状态 !!! (你刚才问的第一段代码，这里是它的正确位置)
    // 注意：这个状态是禁用深度写入的
    D3D11_DEPTH_STENCIL_DESC transparentDepthStencilDesc = {};
    transparentDepthStencilDesc.DepthEnable = TRUE; // 仍然启用深度测试 (与不透明物体比较)
    transparentDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // **禁用深度写入**
    transparentDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    transparentDepthStencilDesc.StencilEnable = FALSE;

    hr = state->device->CreateDepthStencilState(&transparentDepthStencilDesc, &state->depthStencilStateTransparent);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to create transparent depth stencil state.", L"Error", MB_OK);
        return false;
    }



    //const UINT quadVertexCount = 4;  // 4个顶点
    const UINT quadIndexCount = 6;   // 6个索引（三角形2个，每个3个顶点

    GameObject bg;

    RECT clientRect = {};
    GetClientRect(hwnd, &clientRect);

    float width = static_cast<float>(clientRect.right - clientRect.left);
    float height = static_cast<float>(clientRect.bottom - clientRect.top);

    Vertex bg_vertices[] = {
        { { 0.0f, 0.0f, 0.0f }, {1, 0, 0, 1}, {0.0f, 0.0f} }, // 左上角
        { { width, 0.0f, 0.0f }, {0, 1, 0, 1}, {1.0f, 0.0f} }, // 右上角
        { { width, height, 0.0f }, {0, 0, 1, 1}, {1.0f, 1.0f} }, // 右下角
        { { 0.0f, height, 0.0f }, {1, 1, 0, 1}, {0.0f, 1.0f} }  // 左下角
    };

    // 假设你有一个函数用来创建2D矩形顶点缓冲区
    bg.vertexBuffer = CreateQuadVertexBuffer(state->device, bg_vertices, 4);
    // 创建索引缓冲区
    bg.indexBuffer = CreateQuadIndexBuffer(state->device);
    bg.indexCount = quadIndexCount;  // 6，两个三角形的索引数量
    // worldMatrix 2D平移矩阵，Z轴一般为0
    bg.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);


    // 加载纹理文件，请确保 'bg.dds' 存在于你的可执行文件同级目录
    hr = LoadTextureAndCreateSRV(state->device, L"assets\\bg.dds", &bg.textureSRV);
    if (FAILED(hr)) {

        MessageBox(hwnd, L"Failed to load texture1.dds. Please check if the file exists and is a valid DDS.", L"Error", MB_OK);
        return false;
    }



    GameObject quad1;

    Vertex quad1_vertices[] = {
        { { 200.0f, 100.0f, 0.0f }, {1, 0, 0, 1}, {0.0f, 0.0f} }, // 左上角
        { { 500.0f, 100.0f, 0.0f }, {0, 1, 0, 1}, {1.0f, 0.0f} }, // 右上角
        { { 500.0f, 400.0f, 0.0f }, {0, 0, 1, 1}, {1.0f, 1.0f} }, // 右下角
        { { 200.0f, 400.0f, 0.0f }, {1, 1, 0, 1}, {0.0f, 1.0f} }  // 左下角
    };

    // 假设你有一个函数用来创建2D矩形顶点缓冲区
    quad1.vertexBuffer = CreateQuadVertexBuffer(state->device, quad1_vertices, 4);
    // 创建索引缓冲区
    quad1.indexBuffer = CreateQuadIndexBuffer(state->device);
    quad1.indexCount = quadIndexCount;  // 6，两个三角形的索引数量
    // worldMatrix 2D平移矩阵，Z轴一般为0
    quad1.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    // 加载纹理文件，请确保 'texture1.dds' 存在于你的可执行文件同级目录
    hr = LoadTextureAndCreateSRV(state->device, L"assets\\mario.dds", &quad1.textureSRV);
    if (FAILED(hr)) {

        MessageBox(hwnd, L"Failed to load texture1.dds. Please check if the file exists and is a valid DDS.", L"Error", MB_OK);
        return false;
    }


    GameObject quad2;
    Vertex quad2_vertices[] = {
       { { 600.0f, 200.0f, 0.0f }, {0, 0, 0, 1}, {0.0f, 0.0f} },
       { { 1020.0f, 200.0f, 0.0f }, {1, 1, 0, 1}, {1.0f, 0.0f} },
       { { 1020.0f, 500.0f, 0.0f }, {0, 0, 1, 1}, {1.0f, 1.0f} },
       { { 600.0f, 500.0f, 0.0f }, {0, 1, 1, 1}, {0.0f, 1.0f} }
    };
    // 假设你有一个函数用来创建2D矩形顶点缓冲区
    quad2.vertexBuffer = CreateQuadVertexBuffer(state->device, quad2_vertices, 4);
    // 创建索引缓冲区
    quad2.indexBuffer = CreateQuadIndexBuffer(state->device);
    quad2.indexCount = quadIndexCount;  // 6，两个三角形的索引数量
    // worldMatrix 2D平移矩阵，Z轴一般为0
    quad2.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    // 加载纹理文件，请确保 'texture1.dds' 存在于你的可执行文件同级目录
    hr = LoadTextureAndCreateSRV(state->device, L"assets\\peach.dds", &quad2.textureSRV);
    if (FAILED(hr)) {
        MessageBox(hwnd, L"Failed to load texture1.dds. Please check if the file exists and is a valid DDS.", L"Error", MB_OK);
        return false;
    }
    
    sceneObjects.push_back(bg);
    sceneObjects.push_back(quad1);
    sceneObjects.push_back(quad2);



    return true; // 成功时返回true
}

ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device, Vertex* vertices, unsigned vertices_count) {
  
    

    D3D11_BUFFER_DESC bd = {};// Direct3D 11 用来描述缓冲区属性的结构体
    bd.Usage = D3D11_USAGE_DEFAULT;              // 指示缓冲区将由 GPU 读取和写入。创建后不允许 CPU 直接访问。
    bd.ByteWidth = sizeof(Vertex) * vertices_count;             // 缓冲区大小 = 顶点总大小
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;     // 用作顶点缓冲区

    D3D11_SUBRESOURCE_DATA initData = {};//D3D11_SUBRESOURCE_DATA 是用来初始化缓冲区数据的结构体。
    initData.pSysMem = vertices;// 提供用于在 GPU 上填充缓冲区的初始数据指针

    //定义一个指向 Direct3D 11 顶点缓冲区的指针，初始值设为 nullptr（空指针）
    ID3D11Buffer* vertexBuffer = nullptr;
    //创建一个 GPU 上的缓冲区。
    HRESULT hr = device->CreateBuffer(&bd, &initData, &vertexBuffer);
    if (FAILED(hr)) {
        // 错误处理
        return nullptr;
    }
    return vertexBuffer;
}
ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device)
{
    // 定义矩形的索引数组，共6个索引，绘制两个三角形组成一个矩形
    // 三角形1顶点索引: 0, 1, 2
    // 三角形2顶点索引: 0, 2, 3
    UINT indices[6] = { 0, 1, 2, 0, 2, 3 };

    // 初始化缓冲区描述结构体
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;                // 默认使用方式，GPU可读写，CPU不可访问
    bd.ByteWidth = sizeof(UINT) * 6;               // 缓冲区大小，6个索引，每个索引是UINT类型
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;        // 绑定标志，表示该缓冲区是索引缓冲区

    // 定义初始化数据结构，告诉D3D缓冲区初始化时使用哪个内存的数据
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;                     // 指向索引数组的指针

    // 创建索引缓冲区指针，初始值为空
    ID3D11Buffer* indexBuffer = nullptr;

    // 调用设备接口，创建索引缓冲区
    HRESULT hr = device->CreateBuffer(&bd, &initData, &indexBuffer);
    if (FAILED(hr))
    {
        // 创建失败，返回nullptr，可以在调用处检测失败并处理
        return nullptr;
    }

    // 创建成功，返回索引缓冲区指针
    return indexBuffer;
}

// 更新常量缓冲区的函数
void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX& worldMatrix, float screen_width, float screen_height)
{
    // 先转置矩阵（DirectX一般用行主序，HLSL一般列主序）
    DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(worldMatrix);



    //将 GPU 的 constantBuffer 映射到 CPU-accessible memory。
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        // 失败处理
        return;
    }
    //// 将数据从 CPU 端的 cb struct 复制到映射的 GPU memory
    //memcpy(mappedResource.pData, &cb, sizeof(ConstantBuffer));
    ////取消映射 buffer，使更新后的数据可供 GPU 使用。

    // 将数据拷贝到映射后的内存中
        ConstantBuffer* pCb = (ConstantBuffer*)mappedResource.pData;
        pCb->worldMatrix = transposed;           // 对象的变换矩阵
        pCb->screenSize[0] = screen_width;        // 当前窗口宽度
        pCb->screenSize[1] = screen_height;       // 当前窗口高度
    context->Unmap(constantBuffer, 0);
}

HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** srv) {
    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratchImage;
    HRESULT hr;

    // 尝试从 DDS 文件加载纹理
    hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, &metadata, scratchImage);

    if (FAILED(hr)) {
        // 如果加载失败，通常是文件不存在或格式不正确
        // 可以在这里添加 MessageBox 或日志输出，便于调试
        // MessageBox(nullptr, L"Failed to load DDS texture.", filename, MB_OK);

        return hr;
    }

    // 从加载的图片数据创建 Shader Resource View
    // 这个函数会自动处理 mipmaps 和纹理格式转换
    hr = DirectX::CreateShaderResourceView(device, scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, srv);
    return hr;
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
    if (state->constantBuffer) { // 释放常量缓冲区
        state->constantBuffer->Release();
        state->constantBuffer = nullptr;
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

    // 最后释放 StateInfo 结构体本身（因为是用 new 分配的）
    if (state) {
        delete state;
        state = nullptr;
    }
}
/*
+------------------+        +-----------------+        +------------------+
|   顶点缓冲区     | -----> | 顶点着色器 VS   | -----> | 光栅化（生成像素） |
| (Vertex Buffer)  |        |（处理位置等）   |        +------------------+
+------------------+                                   ↓
           ↑
           ❘
+------------------+        +-----------------+        ↓
| 常量缓冲区       | -----> | 像素着色器 PS   | <------+
| (ConstantBuffer) |        |（上色计算）     |
+------------------+        +-----------------+

                     最终绘制到
                     ↓

           +--------------------------+
           | 渲染目标视图 (RTV)        |
           | (后备缓冲区的“视图”)      |
           +--------------------------+
                        ↓
               Present（交换显示）

           +--------------------------+
           |   显示器/窗口            |
           +--------------------------+


*/