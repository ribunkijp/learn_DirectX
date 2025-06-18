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
#include "StateInfo.h"
#include "d3dApp.h"
#include "Render.h"



inline StateInfo* GetAppState(HWND hwnd);


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
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // 没有消息时的逻辑，例如渲染一帧
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