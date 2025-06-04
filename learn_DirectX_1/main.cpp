/*===================================================================================================

	main.cpp

																				Author：ribunki
																				Date  ：2025/6/4

----------------------------------------------------------------------------------------------------
	
===================================================================================================*/

#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sstream>
//#include "debug_ostream.h"


static constexpr char WINDOW_CLASS[] = "GamaWindow";// メインウィンドウクラス名
static constexpr char TITLE[] = "ウィンドウ表示";//タイトルバーのテキスト

//ウィンドウプロシージャ　プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



//constexpr char FILE_NAME[] = "tekito.png";
//
int APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE, //_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, 
	_In_ int nCmdShow
){
	/*std::stringstream ss;
	ss << "テクスチャファイル:" << FILE_NAME << "が読み込めませんでした";

	int a = MessageBox(nullptr, ss.str().c_str(), "キャプション", MB_RETRYCANCEL | MB_ICONERROR | MB_DEFBUTTON2);

	if (a == IDYES) {
		MessageBox(nullptr, "OK", "OK", MB_OK);
	}
	else if (a == IDCANCEL) {
		MessageBox(nullptr, "キャンセル", "キャンセル", MB_OK);
	}*/
	
	/*hal::dout << "テストだよん\n" << "上手くいくかな？" << std::endl;

	MessageBox(NULL, "はい　or　いいえ", "選択", MB_YESNO);*/

	//ウィンドウ情報
	
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;//メニューは作らない
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassEx(&wcex);


	//メインウィンドウの作成
	HWND hWnd = CreateWindow(WINDOW_CLASS, TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;

	
	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}