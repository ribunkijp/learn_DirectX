/*===================================================================================================

	main.cpp

																				Author：ribunki
																				Date  ：2025/6/6

----------------------------------------------------------------------------------------------------
	
===================================================================================================*/

#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "game_window.h"
#include "direct3d.h"



//メイン
int APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE, //_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, 
	_In_ int nCmdShow
){
	

	HWND hWnd = GameWindow_Create(hInstance);
	
	Direct3D_Initialize(hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//メッセージループ
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		Direct3D_Clear();
		Direct3D_Present();
	}

	return (int)msg.wParam;

}


