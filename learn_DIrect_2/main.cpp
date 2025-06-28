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
#include <ShellScalingAPI.h>
#pragma comment(lib, "Shcore.lib")

#include "StateInfo.h"
#include "d3dApp.h"
#include "Timer.h"
#include "Update.h"
#include "Render.h"



void GetScaledWindowSizeAndPosition(int logicalWidth, int logicalHeight,
    int& outW, int& outH, int& outLeft, int& outTop, DWORD C_WND_STYLE);

inline StateInfo* GetAppState(HWND hwnd);

void UpdateViewport(ID3D11DeviceContext* context, HWND hwnd);




// 窗口过程函数
LRESULT CALLBACK WindowProc(
    HWND hwnd,//窗口句柄 一个具体窗口的“身份证”
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);



// 程序入口
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance, //实例句柄 应用本身的“身份证”代表当前运行的应用程序实例,即你这个程序在内存中的一个副本
    _In_opt_ HINSTANCE hPrevInstance,//Windows 95 时代遗留参数，总是为 NULL
    _In_ PWSTR pCmdLine,//命令行参数字符串（不包含程序本身的路径）
    _In_ int nCmdShow //窗口显示的方式
) {
    // 设置 DPI 感知
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    //dpi检测
    //PROCESS_DPI_AWARENESS awareness;
    //HRESULT hr = GetProcessDpiAwareness(NULL, &awareness); // NULL 代表当前进程

    //if (SUCCEEDED(hr))
    //{
    //    wchar_t buffer[256];
    //    // PROCESS_DPI_AWARENESS 的枚举值也是 0, 1, 2，含义相同
    //    swprintf_s(buffer, L"DPI Awareness Level:\n\n0 = Unaware\n1 = System Aware\n2 = Per-Monitor Aware\n\nYour Level: %d", awareness);
    //    MessageBox(NULL, buffer, L"DPI Check", MB_OK);
    //}


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

    const DWORD C_WND_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    //
    int winW = 0, winH = 0, winL = 0, winT = 0;
    GetScaledWindowSizeAndPosition(
        static_cast<int>(pState->logicalWidth),
        static_cast<int>(pState->logicalHeight),
        winW, winH, winL, winT, C_WND_STYLE);
    

    // 
    HWND hwnd = CreateWindowEx(
        0,                              // 可选窗口样式
        CLASS_NAME,                     // 窗口类名
        L"Window_1",                    // 窗口标题（L 表示 UTF-16 字符串）
        C_WND_STYLE,            // 窗口样式

        // 位置和大小
        CW_USEDEFAULT, // <-- 使用默认 X 位置
        CW_USEDEFAULT, // <-- 使用默认 Y 位置
        winW, 
        winH,

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


    //RECT rect;
    //GetClientRect(hwnd, &rect);
    //float clientWidth = static_cast<float>(rect.right - rect.left);
    //float clientHeight = static_cast<float>(rect.bottom - rect.top);

    //// 初始化 Direct3D11
    //if (!InitD3D(hwnd, pState, clientWidth, clientHeight)) {
    //    MessageBox(hwnd, L"初期化に失敗しました", L"エラー", MB_OK);
    //    return 0;
    //}
   
    Timer timer;       // 计时器对象
    timer.Reset();     // 程序启动时调用一次

    // 消息循环
    MSG msg = {};
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return 0;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (pState->d3dInitialized) {

            timer.Tick();    // 每帧调用
            float deltaTime = timer.GetDeltaTime();  // 每帧耗时

            // 更新所有对象的动画和常量缓冲区
            UpdateAllObjects(pState, deltaTime);

            //更新视口
            UpdateViewport(pState->context, hwnd);

            // 如果没有消息，进行每一帧渲染
            RenderFrame(hwnd, pState);
        }
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
                delete pState;
                pState = nullptr;
            }
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
        {
            //
            pState = GetAppState(hwnd);
            if (!pState)
                return 0;

            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            if (width == 0 || height == 0)
            {
                // 窗口被最小化了，我们不处理，直接返回
                return 0;
            }

            if (!pState->d3dInitialized) {
                if (InitD3D(hwnd, pState, static_cast<float>(width), static_cast<float>(height)))
                {
                    // 初始化成功，设置标志位
                    pState->d3dInitialized = true;
                }
                else
                {
                    // 初始化失败，报告错误并销毁窗口
                    MessageBox(hwnd, L"D3D 初始化失败!", L"错误", MB_OK);
                    DestroyWindow(hwnd);
                    return 0;
                }
            }
            else
            {
                OnResize(hwnd, pState, width, height);
            }

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

//
void UpdateViewport(ID3D11DeviceContext* context, HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    context->RSSetViewports(1, &vp);
}


void GetScaledWindowSizeAndPosition(int logicalWidth, int logicalHeight,
    int& outW, int& outH, int& outLeft, int& outTop, DWORD C_WND_STYLE)
{
    // --- 获取主显示器的 DPI ---
    HMONITOR monitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
    UINT dpiX = 96, dpiY = 96;
    GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

    // --- 根据DPI计算客户端需要的物理像素大小 ---
    float dpiScale = dpiX / 96.0f;
    int scaledClientW = static_cast<int>(logicalWidth * dpiScale);
    int scaledClientH = static_cast<int>(logicalHeight * dpiScale);

    // --- 根据客户端大小和窗口样式，计算整个窗口需要的物理像素大小 ---
    RECT rect = { 0, 0, scaledClientW, scaledClientH };
    AdjustWindowRectExForDpi(&rect, C_WND_STYLE, FALSE, 0, dpiX);
    outW = rect.right - rect.left;
    outH = rect.bottom - rect.top;

    // --- 【关键修正】获取主显示器在虚拟桌面中的真实工作区矩形 ---
    MONITORINFO mi = { sizeof(mi) };
    if (GetMonitorInfo(monitor, &mi))
    {
        // 从 MONITORINFO 中获取显示器的宽度和高度
        int monitorW = mi.rcMonitor.right - mi.rcMonitor.left;
        int monitorH = mi.rcMonitor.bottom - mi.rcMonitor.top;

        // 将窗口居中到这个显示器的矩形区域内
        outLeft = mi.rcMonitor.left + (monitorW - outW) / 2;
        outTop = mi.rcMonitor.top + (monitorH - outH) / 2;
    }
    else
    {
        // 如果 GetMonitorInfo 失败，回退到旧的方法
        int screenW = GetSystemMetricsForDpi(SM_CXSCREEN, dpiX);
        int screenH = GetSystemMetricsForDpi(SM_CYSCREEN, dpiX);
        outLeft = (screenW - outW) / 2;
        outTop = (screenH - outH) / 2;
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