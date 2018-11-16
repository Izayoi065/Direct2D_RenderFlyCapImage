/* インクルードファイル */
#include "CApplication.h"	// アプリケーションクラス
#include "CWinBase.h"		// 基底ウィンドウクラス
#include "resource.h"		// リソースファイル


// staticメンバ変数の定義.
std::map<HWND, CWinBase *> CWinBase::m_mapWindowMap;	// staticメンバ変数CWinBase::m_mapWindowMapは宣言と別にここに定義しないといけない.

														/* コンストラクタ CWinBase */
CWinBase::CWinBase(CApplication * pApp) {
	MyOutputDebugString(L"	CWinBase::CWinBase(CApplication *) メソッドが呼び出されました．\n");

	// メンバの初期化
	m_hWnd = NULL;	// m_hWndをNULL.
	m_pApp = pApp;	// m_pAppをpApp.
}

/* デストラクタ ~CWinBase */
CWinBase::~CWinBase() {
	MyOutputDebugString(L"	CWinBase::~CWinBase() メソッドが呼び出されました．\n");

}


/* ウィンドウクラス登録関数 RegisterClass */
BOOL CWinBase::RegisterClass(HINSTANCE hInstance) {
	MyOutputDebugString(L"	CWinBase::RegisterClass(HINSTANCE) メソッドが呼び出されました．\n");

	/* 変数の宣言 */
	WNDCLASSEX wcex = { 0 };	// ウィンドウクラスEX wcex

	/* ウィンドウクラス"CWinBase"の設定 */
	wcex.cbSize = sizeof(WNDCLASSEX);						// 構造体のサイズ
	wcex.style = (CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);	// クラスのスタイル
	wcex.lpfnWndProc = CWinBase::StaticWindowProc;				// ウィンドウプロシージャの設定
	wcex.cbClsExtra = 0;										// 拡張サイズ・データサイズ
	wcex.cbWndExtra = 0;										// 拡張ウィンドウ・データサイズ
	wcex.hInstance = hInstance;								// インスタンス・ハンドル
	wcex.hIcon = apiLoadIcon(NULL, IDI_APPLICATION);		// アイコンのハンドル
	wcex.hIconSm = apiLoadIcon(NULL, IDI_APPLICATION);		// 小さいアイコンのハンドル
	wcex.hCursor = apiLoadCursor(NULL, IDC_ARROW);			// カーソルのハンドル
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);		// 背景ブラシのハンドル
	wcex.lpszMenuName = (LPCWSTR)IDC_DIRECT2DRENDERFLYCAPIMAGE;						// メニューバーの識別文字列
																//wcex.lpszMenuName = NULL;										// メニューバーの識別文字列
	wcex.lpszClassName = _T("CWinBase");							// ウィンドウクラス名"CWinBase"

																	/* ウィンドウクラスの登録 */
	if (!::RegisterClassEx(&wcex)) {	// WindowsAPIのRegisterClass関数で登録
		return FALSE;
	}

	// 成功ならTRUE
	return TRUE;
}

/* staticウィンドウプロシージャ StaticWindowProc */
LRESULT CWinBase::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// ポインタの宣言
	CWinBase *pWindow = NULL;		// CWinBase オブジェクトポインタ pWindow

									/* メッセージウィンドウ処理 */
	switch (uMsg) {
		/* ウィンドウの作成が開始された時 */
	case WM_CREATE:
		// WM_CREATEのブロック
	{
		// ポインタの初期化
		LPCREATESTRUCT lpCreateStruct = NULL;	// CREATESTRUCT構造体へのポインタlpCreateStructをNULL

												/* lParamからlpCreateStructを取り出す */
		lpCreateStruct = (LPCREATESTRUCT)lParam;	// lParamを(LPCREATESTRUCT)にキャスト
		if (lpCreateStruct->lpCreateParams != NULL) {	// NULLでない場合
			// lpCreateStruct->lpCreateParamsは(CWinBase *)にキャストしpWindowに入れる
			pWindow = (CWinBase *)lpCreateStruct->lpCreateParams;
			// CWinBase::m_mapWindowMapにhwndとpWindowのペアを登録
		CWinBase:m_mapWindowMap.insert(std::pair < HWND, CWinBase *>(hwnd, pWindow));
		}
	}
	// 既定の処理へ以降
	break;
	/* 既定 */
	default:
		// defaultのブロック
	{
		// hwndでウィンドウオブジェクトポインタが引けたら，pWindowに格納
		if (CWinBase::m_mapWindowMap.find(hwnd) != CWinBase::m_mapWindowMap.end()) {	// hwndがmapの最後にに登録されている場合
			// pWindowにhwndで引けるCWinBaseオブジェクトポインタを代入
			pWindow = CWinBase::m_mapWindowMap[hwnd];
		}
	}
	// 既定の処理へ以降
	break;
	}

	/* ウィンドウオブジェクトが取得できない場合 */
	if (pWindow == NULL) {	// pWindowがNULL
							// DefWindowProcに任せる.
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	else {		// pWindowがあった
		// そのウィンドウのDynamicWindowProcに渡す.
		return pWindow->DynamicWindowProc(hwnd, uMsg, wParam, lParam);	// pWindow->DynamiProcに渡す.
	}
}
// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CWinBase::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/* ウィンドウ作成関数 */
BOOL CWinBase::Create(LPCTSTR lpctszClassName, LPCTSTR lpctszWindowName, DWORD dwStyle, const RECT & rect, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance) {
	// ウィンドウを作成する.
	m_hWnd = CreateWindow(lpctszClassName, lpctszWindowName, dwStyle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hWndParent, hMenu, hInstance, this);	// CreateWindowで指定された引数を使ってウィンドウを作成.
	if (m_hWnd == NULL) {	// NULLなら失敗.
							// FALSEを返す.
		return FALSE;
	}

	// 成功ならTRUE.
	return TRUE;
}

/* ウィンドウ作成関数 Create（ウィンドウ名とサイズのみのオーバーロード関数） */
BOOL CWinBase::Create(LPCTSTR lpctszWindowName, const RECT & rect) {

	// アプリケーションオブジェクトポインタのチェック
	if (m_pApp == NULL) {
		return FALSE;
	}

	// デフォルトのウィンドウ作成
	return CWinBase::Create(_T("CWinBase"), lpctszWindowName, WS_OVERLAPPEDWINDOW, rect, NULL, NULL, m_pApp->m_hInstance);	// フルバージョンのCreateでウィンドウ作成.
}

/* ウィンドウ表示関数 ShowWindow */
BOOL CWinBase::ShowWindow(int nCmdShow) {
	MyOutputDebugString(L"	CWinBase::ShowWindow(int) メソッドが呼び出されました．\n");

	/* ウィンドウ表示 */
	return ::ShowWindow(m_hWnd, nCmdShow);	// WindowsAPIのShowWindowでm_hWndを表示.
}

/* ダイナミックウィンドウプロシージャ DynamicWindowProc */
LRESULT CWinBase::DynamicWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
		/* ウィンドウの作成が開始された時 */
	case WM_CREATE:
		// WM_CREATEブロック
	{
		// OnCreateに任せる.
		return OnCreate(hwnd, (LPCREATESTRUCT)lParam);	// hwndとlParamをOnCreateに渡してあとは任せる
	}
	// 既定の処理
	break;
	/* ウィンドウが破棄された時 */
	case WM_DESTROY:
		// WM_DESTROYブロック
	{
		// OnDestroyに任せる.
		OnDestroy();	// OnDestroyを呼ぶ
	}
	// 既定の処理
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(this->m_pApp->m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hwnd);
			break;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	// 既定の処理
	break;
	/* その他 */
	default:
	{

	}
	// 既定の処理
	break;
	}

	// DefWindowProcに任せる.
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
