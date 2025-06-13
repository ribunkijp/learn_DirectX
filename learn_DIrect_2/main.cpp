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
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")




struct ConstantBuffer {
    float screenSize[2]; // 屏幕宽高
    float padding[2];    // 保持16字节对齐
};


// 管理 Direct3D11 渲染状态的结构体
struct StateInfo {
    // 设备对象：用于创建资源和着色器
    ID3D11Device* device = nullptr; //ID3D11Device（显卡接口）

    // 设备上下文：用于发出绘制命令和绑定资源 //D3D11DeviceContext（命令接口）
    ID3D11DeviceContext* context = nullptr;

    // 交换链：管理后台缓冲区与前台缓冲区的交换（用于屏幕显示）
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

    //
    ID3D11Buffer* constantBuffer = nullptr;

};

inline StateInfo* GetAppState(HWND hwnd);

bool InitD3D(HWND hwnd, StateInfo* state);

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
    _In_ HINSTANCE hInstance, //实例句柄 应用本身的“身份证”
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
    //
    wc.hInstance = hInstance;
    //// 这里设置窗口类名(自定义的字符串)
    wc.lpszClassName = CLASS_NAME;

    // 注册窗口类
    RegisterClass(&wc);

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
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// 窗口过程
LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam, // 无符号整数类型
    LPARAM lParam  // 有符号整数类型
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
            GetClientRect(hwnd, &rect);
            float width = static_cast<float>(rect.right - rect.left);
            float height = static_cast<float>(rect.bottom - rect.top);
            // Map 常量缓冲区，更新 screenSize
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            if (SUCCEEDED(pState->context->Map(pState->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                ConstantBuffer* pCb = (ConstantBuffer*)mapped.pData;
                pCb->screenSize[0] = width;
                pCb->screenSize[1] = height;
                pState->context->Unmap(pState->constantBuffer, 0);
            }

            // 绑定常量缓冲区给顶点着色器
            pState->context->VSSetConstantBuffers(0, 1, &pState->constantBuffer);



            // 清除背景色
            float clearColor[4] = { 0.0f, 0.3f, 0.0f, 1.0f };
            pState->context->ClearRenderTargetView(pState->rtv, clearColor);

            // 设置输入布局和着色器
            /*
                告诉GPU顶点数据长啥样，怎么读；

                告诉GPU顶点数据如何被顶点着色器处理；

                告诉GPU像素怎么被像素着色器处理。
            */
            //设置输入布局（Input Layout）
            pState->context->IASetInputLayout(pState->inputLayout);
            //绑定顶点着色器（Vertex Shader）。
            pState->context->VSSetShader(pState->vertexShader, nullptr, 0);
            //绑定像素着色器（Pixel Shader）
            pState->context->PSSetShader(pState->pixelShader, nullptr, 0);

            // 绑定顶点缓冲区
            UINT stride = sizeof(float) * 7; // 3个坐标 + 4个颜色
            //offset（偏移量）：从顶点缓冲区开始处偏移多少字节读取数据，这里是0，表示从缓冲区头开始。
            UINT offset = 0;
            pState->context->IASetVertexBuffers(0, 1, &pState->vertexBuffer, &stride, &offset);

            // 作用是告诉 GPU 如何把顶点组织成图元来绘制。设置图元类型为三角形列表:D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
            pState->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);




            // 绘制3个顶点
            pState->context->Draw(3, 0);

            // 交换前后台缓冲区
            pState->swapChain->Present(1, 0);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



inline StateInfo* GetAppState(HWND hwnd)
{
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

    // 缓冲区的用途（作为渲染目标使用）
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

    // 将创建的渲染目标视图设置到GPU（指定绘制目标）
    state->context->OMSetRenderTargets(1, &state->rtv, nullptr);

    // 设置视口（绘制区域）
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)screenWidth;       // 视口宽度（与窗口宽度一致）
    vp.Height = (FLOAT)screenHeight;     // 视口高度（与窗口高度一致）
    vp.MinDepth = 0.0f;                  // 最小深度（深度缓冲区最小值）
    vp.MaxDepth = 1.0f;                  // 最大深度（深度缓冲区最大值）
    state->context->RSSetViewports(1, &vp); // 设置光栅化阶段的视口


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


    // 3. 创建输入布局（指定顶点缓冲区格式）
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        // POSITION: 3个float（x, y, z）
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},

        // COLOR: 4个float（r, g, b, a）
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = state->device->CreateInputLayout(
        layout, 2,                          // 输入布局描述数组 + 数量
        vsBlob->GetBufferPointer(),        // 编译后的顶点着色器代码（二进制）
        vsBlob->GetBufferSize(),
        &state->inputLayout                // 创建出的输入布局对象
    );
    vsBlob->Release();
    psBlob->Release();
    if (FAILED(hr)) return false;


    // 常量缓冲区（Constant Buffer）
    //初始化一个描述结构体，准备告诉 GPU 我们要创建一个什么样的缓冲区。
    D3D11_BUFFER_DESC cbd = {};
    //设置这个缓冲区的“用途”DYNAMIC 表示：CPU 会频繁改数据（如每帧传入新的矩阵），GPU 会读取。
    cbd.Usage = D3D11_USAGE_DYNAMIC;  
    //缓冲区的大小（以字节为单位）
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




    // 4. 创建顶点缓冲区（向GPU传递三角形顶点数据）
    struct Vertex {
        float pos[3];    // 位置（x, y, z）
        float color[4];  // 颜色（r, g, b, a）
    };

    Vertex vertices[] = {
     { {  500.0f,  100.0f, 0.0f }, {1, 0, 0, 1} },   // 顶点：红色
     { {  700.0f, 600.0f, 0.0f }, {0, 1, 0, 1} },   // 右下：绿色
     { {  300.0f, 600.0f, 0.0f }, {0, 0, 1, 1} }  // 左下：蓝色
    };

    // 缓冲区描述设置
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;           // 常规使用
    bd.ByteWidth = sizeof(vertices);          // 缓冲区大小（字节）
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // 作为顶点缓冲区使用

    // 初始数据设置
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;              // 顶点数据指针

    // 创建缓冲区
    hr = state->device->CreateBuffer(&bd, &initData, &state->vertexBuffer);
    if (FAILED(hr)) return false;

    return true; // 成功时返回true
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