/*===================================================================================================

	main.cpp

																				Author：ribunki
																				Date  ：2025/6/4

----------------------------------------------------------------------------------------------------
	
===================================================================================================*/

#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


//ウィンドウ情報
static constexpr char WINDOW_CLASS[] = "GamaWindow";// メインウィンドウクラス名
static constexpr char TITLE[] = "ウィンドウ表示";//タイトルバーのテキスト

//ウィンドウプロシージャ　プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


//メイン
int APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE, //_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, 
	_In_ int nCmdShow
){
	

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
	constexpr int SCREEN_WITDH = 1600;
	constexpr int SCREEN_HEIGHT = 900;
	
	RECT window_rect{ 0, 0, SCREEN_WITDH, SCREEN_HEIGHT };

	DWORD style = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
	
	AdjustWindowRect(&window_rect, style, FALSE);

	const int WINDOW_WIDTH = window_rect.right - window_rect.left;
	const int WINDOW_HEIGHT = window_rect.bottom - window_rect.top;

	HWND hWnd = CreateWindow(
		WINDOW_CLASS, 
		TITLE, 
		//WS_OVERLAPPEDWINDOW,
		//WS_OVERLAPPEDWINDOW & ~ (WS_THICKFRAME | WS_MINIMIZE),
		style,
		CW_USEDEFAULT, 
		0, 
		CW_USEDEFAULT, 
		0, 
		nullptr, 
		nullptr, 
		hInstance, 
		nullptr
	);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//メッセージループ
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;

}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
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