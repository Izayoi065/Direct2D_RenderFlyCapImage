/****************************************************************************************************
 *タイトル：CViewDirect2D.cpp
 *説明　　：
 *		：
 *外部LIB ：
 *
 *著作権　：Tomoki Kondo
 *
 *変更履歴：2018.11.20 Tuesday
 *　　　：新規登録
 *
 ****************************************************************************************************/
 /* インクルードファイル */
#include "CApplication.h"	// アプリケーションクラス
#include "CWinBase.h"		// 基底ウィンドウクラス
#include "resource.h"		// リソースファイル


// staticメンバ変数の定義.
std::map<HWND, CWinBase *> CWinBase::m_mapWindowMap;	// staticメンバ変数CWinBase::m_mapWindowMapは宣言と別にここに定義しないといけない.

/** @brief CWinBaseクラスのコンストラクタ
@note この関数は，このクラスが呼び出された際に，最初に実行される
@param pApp
@sa CViewDirect2D CApplication
**/
CWinBase::CWinBase(CApplication * pApp) {
	MyOutputDebugString(L"	CWinBase::CWinBase(CApplication *) メソッドが呼び出されました．\n");
	// メンバの初期化
	m_hWnd = NULL;	// m_hWndをNULL.
	m_pApp = pApp;	// m_pAppをpApp.
	m_hBrush_BkColor = CreateSolidBrush(RGB(0, 0, 0));	// STATICコントロールの背景
	MASetting = { 2,13,115,255,0,160 };
	hsv_min = cv::Scalar(MASetting.TH_MIN_HUE, MASetting.TH_MIN_SATURATION, MASetting.TH_MIN_BRIGHTNESS);
	hsv_max = cv::Scalar(MASetting.TH_MAX_HUE, MASetting.TH_MAX_SATURATION, MASetting.TH_MAX_BRIGHTNESS);
}

/** @brief CWinBaseクラスのデストラクタ
@note この関数は，このクラスが呼び出された際に，最後に実行される
@sa 
**/
CWinBase::~CWinBase() {
	MyOutputDebugString(L"	CWinBase::~CWinBase() メソッドが呼び出されました．\n");
}

/** @brief フォルダー名を取得する
@note この関数は，SHBrowseForFolderを用いてフォルダ名を取得するダイアログをポップさせる．
@param hWnd		ダイアログの親ウィンドウ・ハンドル
@param def_dir	フォルダ選択時の初期ディレクトリ
@param path		選択したフォルダのパス
@return HRESULTエラーコードを返す
@sa WStringToString BrowseCallbackProc
**/
HRESULT CWinBase::GetDir(HWND hWnd, TCHAR * def_dir, TCHAR * path)
{
	BROWSEINFO bInfo;
	LPITEMIDLIST pIDList;

	memset(&bInfo, 0, sizeof(bInfo));
	bInfo.hwndOwner = hWnd; // ダイアログの親ウインドウのハンドル 
	bInfo.pidlRoot = NULL; // ルートフォルダをデスクトップフォルダとする 
	bInfo.pszDisplayName = path; //フォルダ名を受け取るバッファへのポインタ 
	bInfo.lpszTitle = TEXT("フォルダの選択"); // ツリービューの上部に表示される文字列 
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE | BIF_NEWDIALOGSTYLE; // 表示されるフォルダの種類を示すフラグ 
	bInfo.lpfn = BrowseCallbackProc; // BrowseCallbackProc関数のポインタ 
	bInfo.lParam = (LPARAM)def_dir;
	pIDList = SHBrowseForFolder(&bInfo);
	if (pIDList == NULL) {
		path[0] = _TEXT('\0');
		return S_OK; //何も選択されなかった場合 
	}
	else {
		if (!SHGetPathFromIDList(pIDList, path))
			return false;//変換に失敗 
		CoTaskMemFree(pIDList);// pIDListのメモリを開放 
		return S_OK;
	}
}

/** @brief std::wstring型文字列をstd::string型文字列に変換する．
@note この関数は，std::wstring型文字列(TCHARなど)をstd::string型文字列(cv::Stringなど)への変換処理を適用する．
@param oWString	変換したいstd::wstring型の文字列
@return	std::string変換後の文字列
@sa flagCapture WideCharToMultiByte
**/
std::string CWinBase::WStringToString(std::wstring oWString) {
	// wstring → SJIS
	int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str()
		, -1, (char *)NULL, 0, NULL, NULL);

	// バッファの取得
	CHAR* cpMultiByte = new CHAR[iBufferSize];

	// wstring → SJIS
	WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte
		, iBufferSize, NULL, NULL);

	// stringの生成
	std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);

	// バッファの破棄
	delete[] cpMultiByte;

	// 変換結果を返す
	return(oRet);
}

/** @brief 録画モードの開始・停止のフラグを設定する．
@note この関数は，録画モードの開始・停止のフラグであるCWinBase::flagCaptureを切り替える．
@param flag	録画モードのフラグ
@sa flagCapture
**/
void CWinBase::setFlagCapture(BOOL flag)
{
	flagCapture = flag;
}

void CWinBase::setHSVMinThreshold(cv::Scalar min)
{
	hsv_min = min;
}

void CWinBase::setHSVMaxThreshold(cv::Scalar max)
{
	hsv_max = max;
}

cv::Scalar CWinBase::getHSVMinThreshold()
{
	return hsv_min;
}

cv::Scalar CWinBase::getHSVMaxThreshold()
{
	return hsv_max;
}

/** @brief ウィンドウクラスを登録する
@note この関数は，引数のHINSTANCEパラメータを保持したウィンドウクラスを作成する．
この機能では，WNDCLASSEXでウィンドウクラスの設定を行い，RegisterClassExにて作成を行う．
@param hInstance 
@return	BOOL 関数の処理結果の成否
@sa callback CWinBase WNDCLASSEX RegisterClassEx
**/
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

/** @brief 基本的なウィンドウプロシージャ
@note この関数は，アプリケーションに送られてきたメッセージを処理する専用のコールバック関数．
メインウィンドウ作成時にはウィンドウオブジェクト CWinBaseとウィンドウ・ハンドル hwndをペアで登録する．
その他の処理については，ウィンドウオブジェクトが取得できる場合はそのウィンドウのDynamicWindowProcに，
できない場合は既定のDefWindowProcに渡す．
@param hWnd		ウィンドウへのハンドル
@param uMsg		メッセージ
@param wParam	追加のメッセージ情報：w-パラメータ
@param lParam	追加のメッセージ情報：l-パラメータ
@return	LRESULT エラーコードを返す
@sa callback CWinBase m_mapWindowMap DynamicWindowProc DefWindowProc
**/
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
		CWinBase::m_mapWindowMap.insert(std::pair < HWND, CWinBase *>(hwnd, pWindow));
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

/** @brief メインウィンドウのメニューバーより，[ヘルプ(H)]->[バージョン情報(A)...]で呼び出されるダイアログの設定
@note この関数は，メインウィンドウのメニューバーにある[ヘルプ(H)]->[バージョン情報(A)...]を選択した際に呼び出されるダイアログ
についてのコールバック関数．
@param hDlg		ウィンドウへのハンドル
@param message	メッセージ
@param wParam	追加のメッセージ情報：w-パラメータ
@param lParam	追加のメッセージ情報：l-パラメータ
@return	INT_PTR 関数の処理結果の成否
@sa callback CWinBase UNREFERENCED_PARAMETER EndDialog
**/
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

/** @brief メインウィンドウのメニューバーより，[設定(O)]->[詳細な設定(S)]で呼び出されるダイアログの設定
@note この関数は，メインウィンドウのメニューバーにある[設定(O)]->[詳細な設定(S)]を選択した際に呼び出されるダイアログ
についてのコールバック関数．
@param hDlg		ウィンドウへのハンドル
@param message	メッセージ
@param wParam	追加のメッセージ情報：w-パラメータ
@param lParam	追加のメッセージ情報：l-パラメータ
@return	INT_PTR 関数の処理結果の成否
@sa callback CWinBase UNREFERENCED_PARAMETER SendDlgItemMessage EndDialog
**/
INT_PTR CWinBase::Setting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	//MyOutputDebugString(L"\n	WM_COMMAND：%dを受け取りました\n\n", message);
	TCHAR minHue[4], minSaturation[4], minBrightness[4], maxHue[4], maxSaturation[4], maxBrightness[4];
	static MyAdvancedSetting* localMASetting;
	switch (message)
	{
	case WM_INITDIALOG:
		localMASetting = (MyAdvancedSetting*)lParam;
		if (localMASetting)
		{
			MyOutputDebugString(L"\n////////////////////////////////////////////////////////////\n", message);
			SendDlgItemMessage(hDlg, IDC_SPINMINHUE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_EDITMINHUE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMINHUE, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MIN_HUE, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMAXHUE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMAXHUE, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MAX_HUE, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMINSATURATION, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMINSATURATION, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MIN_SATURATION, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMAXSATURATION, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMAXSATURATION, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MAX_SATURATION, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMINBRIGHTNESS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMINBRIGHTNESS, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MIN_BRIGHTNESS, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMAXBRIGHTNESS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMAXBRIGHTNESS, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MAX_BRIGHTNESS, 0));
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_EDITMINHUE, minHue, sizeof(minHue));
			GetDlgItemText(hDlg, IDC_EDITMANHUE, maxHue, sizeof(maxHue));
			GetDlgItemText(hDlg, IDC_EDITMINSATURATION, minSaturation, sizeof(minSaturation));
			GetDlgItemText(hDlg, IDC_EDITMAXSATURATION, maxSaturation, sizeof(maxSaturation));
			GetDlgItemText(hDlg, IDC_EDITMINBRIGHTNESS, minBrightness, sizeof(minBrightness));
			GetDlgItemText(hDlg, IDC_EDITMAXBRIGHTNESS, maxBrightness, sizeof(maxBrightness));
			if (localMASetting)
				*localMASetting = { ::_ttoi(minHue), ::_ttoi(maxHue), ::_ttoi(minSaturation), ::_ttoi(maxSaturation), ::_ttoi(minBrightness), ::_ttoi(maxBrightness) };
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/** @brief ウィンドウを作成する
@note この関数は，引数のパラメータに従ってメインウィンドウを作成する．
各種コントロール等についてもここで作成を行う．
@param lpctszClassName	作成するウィンドウのクラス名
@param lpctszWindowName	作成するウィンドウ名
@param dwStyle			ウィンドウのスタイル
@param rect				作成するウィンドウのサイズ
@param hWndParent		親ウィンドウまたはオーナーウィンドウとなるハンドルを指定
@param hMenu			メニューのハンドル
@param hInstance		インスタンスハンドル
@return	BOOL 関数の処理結果の成否
@sa CreateWindow SendMessage EnableWindow m_hWndViewTarget m_hwndSTATICPhase m_hwndTextBoxPhase m_hwndBUTTONPhase
**/
BOOL CWinBase::Create(LPCTSTR lpctszClassName, LPCTSTR lpctszWindowName, DWORD dwStyle, const RECT & rect, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance) {
	// ウィンドウを作成する. http://wisdom.sakura.ne.jp/system/winapi/win32/win7.html
	m_hWnd = CreateWindow(lpctszClassName, lpctszWindowName, dwStyle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hWndParent, hMenu, hInstance, this);	// CreateWindowで指定された引数を使ってウィンドウを作成.
	if (m_hWnd == NULL) {	// NULLなら失敗.
		// FALSEを返す.
		return FALSE;
	}
	// 画像のレンダリングするスペースを作成する．
	m_hWndViewTarget = CreateWindow(L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, 0, 0, rect.right - rect.left, 200, m_hWnd, NULL, hInstance, NULL);

	// 録画モードのグループを作成
	CreateWindow(L"BUTTON", L"録画モード", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 210, 670, 75, m_hWnd, NULL, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_GetFolderPass] = 
		CreateWindow(L"STATIC", L"保存先：", WS_CHILD | WS_VISIBLE, 15, 235, 65, 18, m_hWnd, (HMENU)CID_ST_GetFolderPass, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_FileName] = 
		CreateWindow(L"STATIC", L"ファイル名：", WS_CHILD | WS_VISIBLE, 15, 260, 96, 18, m_hWnd, (HMENU)CID_ST_FileName, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_CaptureStart] = 
		CreateWindow(L"STATIC", L"キャプチャー開始：", WS_CHILD | WS_VISIBLE, 275, 260, 145, 18, m_hWnd, (HMENU)CID_ST_CaptureStart, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_CaptureEnd] = 
		CreateWindow(L"STATIC", L"キャプチャー停止：", WS_CHILD | WS_VISIBLE, 480, 260, 145, 18, m_hWnd, (HMENU)CID_ST_CaptureEnd, hInstance, NULL);
	// テキストボックスを追加
	m_hwndTextBoxPhase[CID_TX_CapturePass] = 
		CreateWindow(L"EDIT", L"E:\\workspace", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE, 75, 235, 550, 20, m_hWnd, (HMENU)CID_TX_CapturePass, hInstance, NULL);
	m_hwndTextBoxPhase[CID_TX_FileName] = 
		CreateWindow(L"EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE, 115, 260, 150, 20, m_hWnd, (HMENU)CID_TX_FileName, hInstance, NULL);
	// ボタンを追加
	m_hwndBUTTONPhase[CID_BT_GetFolderPass] = 
		CreateWindow(L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_ICON, 625, 235, 30, 20, m_hWnd, (HMENU)CID_BT_GetFolderPass, hInstance, NULL);
	SendMessage(m_hwndBUTTONPhase[CID_BT_GetFolderPass], BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 40, 40, LR_DEFAULTCOLOR));
	EnableWindow(m_hwndBUTTONPhase[CID_BT_GetFolderPass], TRUE);
	m_hwndBUTTONPhase[CID_BT_CaptureStart] = 
		CreateWindow(L"BUTTON", L"Start", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 420, 260, 50, 20, m_hWnd, (HMENU)CID_BT_CaptureStart, hInstance, NULL);
	EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureStart], TRUE);
	m_hwndBUTTONPhase[CID_BT_CaptureEnd] = 
		CreateWindow(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 625, 260, 50, 20, m_hWnd, (HMENU)CID_BT_CaptureEnd, hInstance, NULL);
	EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureEnd], FALSE);

	// ログのグループを作成
	CreateWindow(L"BUTTON", L"Console", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 690, 210, 285, 325, m_hWnd, NULL, hInstance, NULL);
	// テキストボックスを追加
	m_hwndTextBoxPhase[CID_TX_Console] =
		CreateWindow(L"EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE, 695, 235, 275, 295, m_hWnd, (HMENU)CID_TX_Console, hInstance, NULL);

	// 成功ならTRUE.
	return TRUE;
}

/** @brief ウィンドウを作成する
@note この関数は，ウィンドウを作成する前段階として，アプリケーションオブジェクトのポインタが生成されているかを確認する．
生成されていた場合，ウィンドウ作成の処理を実行する．
@param lpctszWindowName	作成するウィンドウ名
@param rect				作成するウィンドウのサイズ
@return	BOOL 関数の処理結果の成否
@sa Create
**/
BOOL CWinBase::Create(LPCTSTR lpctszWindowName, const RECT & rect) {

	// アプリケーションオブジェクトポインタのチェック
	if (m_pApp == NULL) {
		return FALSE;
	}

	// デフォルトのウィンドウ作成
	return CWinBase::Create(_T("CWinBase"), lpctszWindowName, WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME, rect, NULL, NULL, m_pApp->m_hInstance);	// フルバージョンのCreateでウィンドウ作成.
}

/** @brief 本プログラムのメインプログラムの表示状態を変更する．
@note この関数は，本プログラムのメインプログラムの表示状態を変更する．
@param nCmdShow	ウィンドウの表示状態を表す定数(SW_SHOWで表示)
@return	BOOL 関数の処理結果の成否
@sa ShowWindow
**/
BOOL CWinBase::ShowWindow(int nCmdShow) {
	MyOutputDebugString(L"	CWinBase::ShowWindow(int) メソッドが呼び出されました．\n");

	/* ウィンドウ表示 */
	return ::ShowWindow(m_hWnd, nCmdShow);	// WindowsAPIのShowWindowでm_hWndを表示.
}

/** @brief ウィンドウオブジェクト用のウィンドウプロシージャ
@note この関数は，ウィンドウオブジェクト専用のウィンドウプロシージャであり，StaticWindowProcによって呼び出される．
メインウィンドウに関するコントロールのイベントについてはここで記述する．
@
m hWnd		ウィンドウへのハンドル
@param uMsg		メッセージ
@param wParam	追加のメッセージ情報：w-パラメータ
@param lParam	追加のメッセージ情報：l-パラメータ
@return	LRESULT エラーコードを返す
@sa Create
**/
LRESULT CWinBase::DynamicWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
		/* ウィンドウの作成が開始された時 */
	case WM_CREATE:
		// WM_CREATEブロック
	{
		// OnCreateに任せる.
		return OnCreate(hwnd, (LPCREATESTRUCT)lParam);	// hwndとlParamをOnCreateに渡してあとは任せる
		// 既定の処理
		break; 
	}
	/* ウィンドウが破棄された時 */
	case WM_DESTROY:
		// WM_DESTROYブロック
	{
		// OnDestroyに任せる.
		OnDestroy();	// OnDestroyを呼ぶ
		// 既定の処理
		break;
	}
	case WM_CTLCOLORSTATIC:
	{
		HDC hDC = (HDC)wParam;
		HWND hCtrl = (HWND)lParam;

		if (hCtrl == GetDlgItem(m_hWnd, CID_ST_GetFolderPass)) // スタティックウィンドウのID
		{
			SetBkMode(hDC, OPAQUE);             // 背景を塗りつぶし
			SetTextColor(hDC, RGB(0, 0, 0));    // テキストの色
			SetBkColor(hDC, RGB(255, 255, 255));   // テキストが書かれている部分のテキストの背景の色
			return (LRESULT)m_hBrush_BkColor;   // テキストが書かれていない部分の背景の色
		}
		else if (hCtrl == GetDlgItem(m_hWnd, CID_ST_FileName)) // スタティックウィンドウのID
		{
			SetBkMode(hDC, OPAQUE);             // 背景を塗りつぶし
			SetTextColor(hDC, RGB(0, 0, 0));    // テキストの色
			SetBkColor(hDC, RGB(255, 255, 255));   // テキストが書かれている部分のテキストの背景の色
			return (LRESULT)m_hBrush_BkColor;   // テキストが書かれていない部分の背景の色
		}
		else if (hCtrl == GetDlgItem(m_hWnd, CID_ST_CaptureStart)) // スタティックウィンドウのID
		{
			SetBkMode(hDC, OPAQUE);             // 背景を塗りつぶし
			SetTextColor(hDC, RGB(0, 0, 0));    // テキストの色
			SetBkColor(hDC, RGB(255, 255, 255));   // テキストが書かれている部分のテキストの背景の色
			return (LRESULT)m_hBrush_BkColor;   // テキストが書かれていない部分の背景の色
		}
		else if (hCtrl == GetDlgItem(m_hWnd, CID_ST_CaptureEnd)) // スタティックウィンドウのID
		{
			SetBkMode(hDC, OPAQUE);             // 背景を塗りつぶし
			SetTextColor(hDC, RGB(0, 0, 0));    // テキストの色
			SetBkColor(hDC, RGB(255, 255, 255));   // テキストが書かれている部分のテキストの背景の色
			return (LRESULT)m_hBrush_BkColor;   // テキストが書かれていない部分の背景の色
		}

		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		MyOutputDebugString(L"\n	WM_COMMAND：%dを受け取りました\n\n", wmId);

		// 選択されたメニューの解析:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(this->m_pApp->m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			break;
		case IDM_SETTING:
			DialogBoxParam(this->m_pApp->m_hInstance, MAKEINTRESOURCE(IDD_SETTINGBOX), hwnd, Setting, (LPARAM)&MASetting);
			hsv_min = cv::Scalar(MASetting.TH_MIN_HUE, MASetting.TH_MIN_SATURATION, MASetting.TH_MIN_BRIGHTNESS);
			hsv_max = cv::Scalar(MASetting.TH_MAX_HUE, MASetting.TH_MAX_SATURATION, MASetting.TH_MAX_BRIGHTNESS);
			MyOutputDebugString(L"MASetting.TH_MIN_HUE:%d\n", MASetting.TH_MIN_HUE);
			MyOutputDebugString(L"MASetting.TH_MAX_HUE:%d\n", MASetting.TH_MAX_HUE);
			MyOutputDebugString(L"MASetting.TH_MIN_SATURATION:%d\n", MASetting.TH_MIN_SATURATION);
			MyOutputDebugString(L"MASetting.TH_MAX_SATURATION:%d\n", MASetting.TH_MAX_SATURATION);
			MyOutputDebugString(L"MASetting.TH_MIN_BRIGHTNESS:%d\n", MASetting.TH_MIN_BRIGHTNESS);
			MyOutputDebugString(L"MASetting.TH_MAX_BRIGHTNESS:%d\n", MASetting.TH_MAX_BRIGHTNESS);
				break;
		case IDM_EXIT:
			DestroyWindow(hwnd);
			break;
		case CID_BT_CaptureStart:
		{
			TCHAR destPass[101], fileName[101];
			GetWindowText(m_hwndTextBoxPhase[CID_TX_CapturePass], destPass, sizeof(destPass));
			GetWindowText(m_hwndTextBoxPhase[CID_TX_FileName], fileName, sizeof(fileName));
			cv::String cvFilePass = WStringToString(lstrcat(lstrcat(lstrcat(destPass, TEXT("\\")), fileName),TEXT(".avi")));
			writer.open(cvFilePass, cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), m_pApp->fps, cv::Size(504, 504), true);
			if (!writer.isOpened()) { return -1; }

			setFlagCapture(true);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureStart], FALSE);
			EnableWindow(m_hwndBUTTONPhase[CID_TX_CapturePass], FALSE);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureEnd], TRUE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_CapturePass], FALSE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_FileName], FALSE);
			break;
		}
		case CID_BT_CaptureEnd:
		{
			writer.release();

			setFlagCapture(false);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureStart], TRUE);
			EnableWindow(m_hwndBUTTONPhase[CID_TX_CapturePass], TRUE);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureEnd], FALSE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_CapturePass], TRUE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_FileName], TRUE);
			break; 
		}
		case CID_BT_GetFolderPass:
		{
			TCHAR dir[MAX_PATH];            //      選択されたフォルダー名
			TCHAR def_dir[MAX_PATH];        //      初期フォルダー
			_tcscpy_s(def_dir, sizeof(def_dir) / sizeof(TCHAR), _TEXT("C:"));
			if (GetDir(0, def_dir, dir) == TRUE) {
				MessageBox(0, dir, _TEXT("選択されたフォルダー名\n"), MB_OK);
			}
			SetWindowText(m_hwndTextBoxPhase[CID_TX_CapturePass], (LPCTSTR)dir);
			break;
		}
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		// 既定の処理
		break;
	}
	/* その他 */
	default:
	{
		// 既定の処理
		break;
	}
	}

	// DefWindowProcに任せる.
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/** @brief GetDir メソッドで設定したフォルダー名取得ダイアログのコールバック関数
@note この関数は，GetDir メソッドで設定したフォルダー名取得ダイアログ用のコールバック関数である．
@param hWnd		ウィンドウへのハンドル
@param uMsg		メッセージ
@param lParam	追加のメッセージ情報：l-パラメータ
@param lpData	フォルダ選択時の初期ディレクトリ
@return int 成功した場合は0を返す
@sa http://yamatyuu.net/computer/program/sdk/common_dialog/SHBrowseForFolder/index.html
**/
int CWinBase::BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	TCHAR dir[MAX_PATH];
	ITEMIDLIST *lpid;
	HWND hEdit;

	switch (uMsg) {
	case BFFM_INITIALIZED:  //      ダイアログボックス初期化時
		SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);     //      コモンダイアログの初期ディレクトリ
		break;
	case BFFM_VALIDATEFAILED:       //      無効なフォルダー名が入力された
		MessageBox(hWnd, (TCHAR*)lParam, _TEXT("無効なフォルダー名が入力されました"), MB_OK);
		hEdit = FindWindowEx(hWnd, NULL, _TEXT("EDIT"), NULL);     //      エディットボックスのハンドルを取得する
		SetWindowText(hEdit, _TEXT(""));
		return 1;       //      ダイアログボックスを閉じない
		break;
	case BFFM_IUNKNOWN:
		break;
	case BFFM_SELCHANGED:   //      選択フォルダーが変化した場合
		lpid = (ITEMIDLIST *)lParam;
		SHGetPathFromIDList(lpid, dir);
		hEdit = FindWindowEx(hWnd, NULL, _TEXT("EDIT"), NULL);     //      エディットボックスのハンドルを取得する
		SetWindowText(hEdit, dir);
		break;
	}
	return 0;
}
