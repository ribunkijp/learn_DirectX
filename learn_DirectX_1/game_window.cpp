/*===================================================================================================

	game_window.cpp

																				Author：ribunki
																				Date  ：2025/6/6

----------------------------------------------------------------------------------------------------

===================================================================================================*/

#include "game_window.h"
#include <algorithm>


//ウィンドウプロシージャ　プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


//ウィンドウ情報
static constexpr char WINDOW_CLASS[] = "GamaWindow";// メインウィンドウクラス名
static constexpr char TITLE[] = "GAME";//タイトルバーのテキスト
constexpr int SCREEN_WITDH = 2560;
constexpr int SCREEN_HEIGHT = 1600;

HWND GameWindow_Create(HINSTANCE hInstance) {
	//ウィンドウクラスの登録

	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wcex.lpszMenuName = nullptr;//メニューは作らない
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassEx(&wcex);


	//メインウィンドウの作成
	RECT window_rect{ 0, 0, SCREEN_WITDH, SCREEN_HEIGHT };

	DWORD style = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);

	AdjustWindowRect(&window_rect, style, FALSE);

	const int WINDOW_WIDTH = window_rect.right - window_rect.left;
	const int WINDOW_HEIGHT = window_rect.bottom - window_rect.top;

	//デスクトップのサイズを取得
	int desktop_width = GetSystemMetrics(SM_CXSCREEN);
	int desktop_height = GetSystemMetrics(SM_CXSCREEN);

	const int WINDOW_X = std::max((desktop_width - WINDOW_WIDTH) / 2, 0);
	const int WINDOW_Y = std::max((desktop_height - WINDOW_HEIGHT) / 2, 0);

	HWND hWnd = CreateWindow(
		WINDOW_CLASS,
		TITLE,
		//WS_OVERLAPPEDWINDOW,
		//WS_OVERLAPPEDWINDOW & ~ (WS_THICKFRAME | WS_MINIMIZE),
		style,
		WINDOW_X,
		WINDOW_Y,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	return hWnd;

};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;
	case WM_CLOSE:
		if (MessageBox(hWnd, "本当に終了してよろしいですか？", "確認", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK) {
			DestroyWindow(hWnd);
		}
		break;
	case WM_DESTROY://ウィンドウの破棄メッセージ
		PostQuitMessage(0);//WM_QUITメッセージの送信
		break;
	default:
		//メッセージ処理
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}