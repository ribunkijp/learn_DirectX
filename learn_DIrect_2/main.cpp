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

//
struct GameObject {
    ID3D11Buffer* vertexBuffer = nullptr;  // 顶点缓冲区指针，存储物体的顶点数据（位置、颜色等）
    ID3D11Buffer* indexBuffer = nullptr;   // 索引缓冲区指针，用于顶点的索引绘制，减少顶点重复
    UINT vertexCount = 0;                   // 顶点数量，用于绘制调用时指定绘制的顶点个数
    UINT indexCount = 0;                    // 索引数量，用于绘制调用时指定绘制的索引个数
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();  // 世界矩阵，表示物体的位置、旋转、缩放（默认单位矩阵）
};


//定义了将数据作为常量缓冲区传递给 GPU shaders 的布局
struct ConstantBuffer {
    //世界变换矩阵。这将按物体进行更新并发送到顶点着色器
    DirectX::XMMATRIX worldMatrix;  // 64 字节 (4x4 float)
    //一个数组，用于存储屏幕的宽度和高度。这在 shader 中用于将像素坐标转换为 Normalized Device Coordinates (NDC)。
    float screenSize[2]; // 屏幕宽高
    float padding[2];    // 保持16字节对齐
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

};

inline StateInfo* GetAppState(HWND hwnd);

bool InitD3D(HWND hwnd, StateInfo* state);

ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device);

ID3D11Buffer* CreateQuadIndexBuffer(ID3D11Device* device);

void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX& worldMatrix);

std::vector<GameObject> sceneObjects;

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
            
           /*
            pState->context->Map(...): 这是一个关键的 Direct3D call。它将 GPU resource (在此例中为 pState->constantBuffer) 
            映射到 CPU-accessible memory。这允许 CPU 直接写入 GPU 将读取的 buffer 中的数据
           */
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            if (SUCCEEDED(pState->context->Map(
                pState->constantBuffer,  // 要写入的缓冲区
                0,                        // 子资源索引（通常为0）
                //写入模式动态 buffers 的常见映射标志。它指示 CPU 将覆盖整个 buffer，并且 GPU 可以丢弃先前的内容。
                // 这对于频繁更新很高效
                D3D11_MAP_WRITE_DISCARD,
                0,                        // 保留，设为0
                &mapped                  // 输出映射信息
            ))) {
                ConstantBuffer* pCb = (ConstantBuffer*)mapped.pData;//把 mapped.pData 转换成你定义的结构体 ConstantBuffer* 指针
                //当前窗口尺寸写入到映射的 constant buffer 内的 screenSize 数组中
                pCb->screenSize[0] = width;
                pCb->screenSize[1] = height;
                //取消映射 buffer，使其再次可供 GPU 使用。写入后取消映射至关重要，以确保 GPU 可以访问更新的数据。
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
            //绑定顶点着色器（Vertex Shader）到管线
            pState->context->VSSetShader(pState->vertexShader, nullptr, 0);
            //绑定像素着色器（Pixel Shader）到管线
            pState->context->PSSetShader(pState->pixelShader, nullptr, 0);

            // 绑定顶点缓冲区
            UINT stride = sizeof(float) * 7; // 3个坐标 + 4个颜色
            //offset（偏移量）：从顶点缓冲区开始处偏移多少字节读取数据，这里是0，表示从缓冲区头开始。
            UINT offset = 0;

            // 作用是告诉 GPU 如何把顶点组织成图元来绘制。设置图元类型为三角形列表:D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
            pState->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            for (auto& obj : sceneObjects)
            {
                ID3D11Buffer* vertexBuffers[] = { obj.vertexBuffer };
                // 设置顶点缓冲区
                pState->context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
                // 设置索引缓冲区 指定每个索引是一个 32 位无符号整数
                pState->context->IASetIndexBuffer(obj.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
                // 设置常量缓冲区，传入 obj.worldMatrix
                UpdateConstantBuffer(pState->context, pState->constantBuffer, obj.worldMatrix);

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

    // 将创建的渲染目标视图设置到GPU（指定绘制目标）
    state->context->OMSetRenderTargets(1, &state->rtv, nullptr);

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
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = state->device->CreateInputLayout(
        layout,      // 布局数组
        2,           // 数组长度
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


    //const UINT quadVertexCount = 4;  // 4个顶点
    const UINT quadIndexCount = 6;   // 6个索引（三角形2个，每个3个顶点

    GameObject quad1;
    // 假设你有一个函数用来创建2D矩形顶点缓冲区
    quad1.vertexBuffer = CreateQuadVertexBuffer(state->device);
    // 创建索引缓冲区
    quad1.indexBuffer = CreateQuadIndexBuffer(state->device);
    quad1.indexCount = quadIndexCount;  // 6，两个三角形的索引数量
    // worldMatrix 2D平移矩阵，Z轴一般为0
    quad1.worldMatrix = DirectX::XMMatrixTranslation(100.0f, 0.0f, 0.0f);


    GameObject quad2;
    // 假设你有一个函数用来创建2D矩形顶点缓冲区
    quad2.vertexBuffer = CreateQuadVertexBuffer(state->device);
    // 创建索引缓冲区
    quad2.indexBuffer = CreateQuadIndexBuffer(state->device);
    quad2.indexCount = quadIndexCount;  // 6，两个三角形的索引数量
    // worldMatrix 2D平移矩阵，Z轴一般为0
    quad2.worldMatrix = DirectX::XMMatrixTranslation(800.0f, 300.0f, 0.0f);
    

    sceneObjects.push_back(quad1);
    sceneObjects.push_back(quad2);



    return true; // 成功时返回true
}

ID3D11Buffer* CreateQuadVertexBuffer(ID3D11Device* device) {
    struct Vertex {
        DirectX::XMFLOAT3 position;  // 位置
        DirectX::XMFLOAT4 color;     // 颜色
    };
    Vertex vertices[] = {
        { { 200.0f, 100.0f, 0.0f }, {1, 0, 0, 1} },   // 左上：红色
        { { 500.0f, 100.0f, 0.0f }, {0, 1, 0, 1} },   // 右上：绿色
        { { 500.0f, 600.0f, 0.0f }, {0, 0, 1, 1} },   // 右下：蓝色
        { { 200.0f, 600.0f, 0.0f }, {1, 1, 0, 1} }    // 左下：黄色（或者你想要的颜色）
    };

    D3D11_BUFFER_DESC bd = {};// Direct3D 11 用来描述缓冲区属性的结构体
    bd.Usage = D3D11_USAGE_DEFAULT;              // 指示缓冲区将由 GPU 读取和写入。创建后不允许 CPU 直接访问。
    bd.ByteWidth = sizeof(vertices);             // 缓冲区大小 = 顶点总大小
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;     // 用作顶点缓冲区

    D3D11_SUBRESOURCE_DATA initData = {};//D3D11_SUBRESOURCE_DATA 是用来初始化缓冲区数据的结构体。
    initData.pSysMem = vertices;// 提供用于在 GPU 上填充缓冲区的初始数据。

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
void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer, DirectX::XMMATRIX& worldMatrix)
{
    // 先转置矩阵（DirectX一般用行主序，HLSL一般列主序）
    DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(worldMatrix);

    // 准备常量缓冲区数据结构（这里假设 ConstantBuffer 定义了相应成员）
    ConstantBuffer cb = {};
    cb.worldMatrix = transposed;              // 设置变换矩阵
    cb.screenSize[0] = (FLOAT)screenWidth;    // 屏幕宽度
    cb.screenSize[1] = (FLOAT)screenHeight;   // 屏幕高度
    cb.padding[0] = 0.0f;                      // 补齐数据，保证结构对齐
    cb.padding[1] = 0.0f;


    //将 GPU 的 constantBuffer 映射到 CPU-accessible memory。
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        // 失败处理
        return;
    }
    // 将数据从 CPU 端的 cb struct 复制到映射的 GPU memory
    memcpy(mappedResource.pData, &cb, sizeof(ConstantBuffer));
    //取消映射 buffer，使更新后的数据可供 GPU 使用。
    context->Unmap(constantBuffer, 0);
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