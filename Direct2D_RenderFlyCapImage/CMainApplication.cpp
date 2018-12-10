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
#define SAFE_DALETE(x) {if(x) {delete (x); (x) = NULL;}}	// 生成したオブジェクトを開放し，NULLを代入する

/* インクルードファイル */
#include "CMainApplication.h"	// メインアプリケーションクラス
#include "CWinBase.h"			// 基底ウィンドウクラス
#include "CViewDirect2D.h"		// DirectX2D関連の初期化クラス
#include "FlyCap2CVWrapper.h"	// OpenCV用のFlyCapture2ラッパークラス
#include "FPSCounter.h"

/* コンストラクタ CMainApplication */
CMainApplication::CMainApplication() {

}

/* デストラクタ ~CMainApplication */
CMainApplication::~CMainApplication() {

}

BITMAPINFOHEADER CMainApplication::MakeBITINFO(int nWidth, int nHeight, LPVOID lpBits)
{
	DWORD dwSizeImage;
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;

	dwSizeImage = nHeight * ((3 * nWidth + 3) / 4) * 4;

	ZeroMemory(&bmfHeader, sizeof(BITMAPFILEHEADER));
	bmfHeader.bfType = *(LPWORD)"BM";
	bmfHeader.bfSize = sizeof(BITMAPFILEHEADER)
		+ sizeof(BITMAPINFOHEADER) + dwSizeImage;

	bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER)
		+ sizeof(BITMAPINFOHEADER);

	ZeroMemory(&bmiHeader, sizeof(BITMAPINFOHEADER));
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = nWidth;
	bmiHeader.biHeight = -nHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 32;
	bmiHeader.biSizeImage -= dwSizeImage;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;
	bmiHeader.biXPelsPerMeter = 0;
	bmiHeader.biYPelsPerMeter = 0;

	return bmiHeader;
}

HBITMAP CMainApplication::CreateBackbuffer(int nWidth, int nHeight)
{
	LPVOID lp;
	BITMAPINFO bmi;
	BITMAPINFOHEADER bmiHeader;

	ZeroMemory(&bmiHeader, sizeof(BITMAPINFOHEADER));
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = nWidth;
	bmiHeader.biHeight = -nHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 32;

	bmi.bmiHeader = bmiHeader;

	return CreateDIBSection(NULL, (LPBITMAPINFO)&bmi,
		DIB_RGB_COLORS, &lp, NULL, 0);
}

void CMainApplication::ConvertToMat(HWND hwnd, cv::OutputArray outImage_)
{
	HDC hdc;
	RECT rc;
	BITMAP bm;
	HBITMAP hbmp;
	HBITMAP hbmpPrev;

	GetClientRect(hwnd, &rc);

	hdc = CreateCompatibleDC(NULL);
	hbmp = CreateBackbuffer(rc.right, rc.bottom);
	hbmpPrev = (HBITMAP)SelectObject(hdc, hbmp);

	BitBlt(hdc, 0, 0, rc.right, rc.bottom, GetWindowDC(hwnd), 0, 0, SRCCOPY);

	GetObject(hbmp, sizeof(BITMAP), &bm);


	BITMAPINFOHEADER bi = MakeBITINFO(rc.right, rc.bottom, bm.bmBits);

	cv::Mat converted_mat;
	converted_mat.create(rc.bottom, rc.right, CV_8UC4);

	GetDIBits(hdc, hbmp, 0, bm.bmHeight,
		converted_mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	SelectObject(hdc, hbmpPrev);
	DeleteObject(hbmp);
	DeleteDC(hdc);

	converted_mat.copyTo(outImage_);
}


/* インスタンス初期化関数 InitInstance */
BOOL CMainApplication::InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd) {
	// 変数の宣言
	RECT rc;	// ウィンドウサイズ rc

	// インスタンスハンドルをメンバにセット
	m_hInstance = hInstance;	// m_hInstanceにhInstanceをセット

	/* ウィンドウクラス"CWinBase"の登録 */
	if (!CWinBase::RegisterClass(m_hInstance)) {	// CWinBase::RegisterClassで登録
		// エラー処理
		MessageBox(NULL, _T("ウィンドウクラスの登録に失敗しました!"), _T("CWinBase"), MB_OK | MB_ICONHAND);	// MessageBoxで"ウィンドウクラスの登録に失敗しました!"と表示
		return -1;	// 異常終了
	}

	/* ウィンドウオブジェクトの作成 */
	//m_pWindow = new C_View(this);
	m_pWindow = new CViewDirect2D(this);

	/* ウィンドウサイズのセット */
	rc.left = 100;		// 100
	rc.top = 100;		// 100
	rc.right = 1100;		// 100 + 1000
	rc.bottom = 700;	// 100 + 600

	/* ウィンドウの作成 */
	if (!m_pWindow->Create(_T("CWinBase"), rc)) {	// Createの省略版でウィンドウ作成
													// エラー処理
		MessageBox(NULL, _T("ウィンドウ作成に失敗しました!"), _T("CWinBase"), MB_OK | MB_ICONHAND);	// MessageBoxで"ウィンドウ作成に失敗しました!"と表示
		return FALSE;	// FALSEを返す
	}

	/* ウィンドウ表示 */
	m_pWindow->ShowWindow(SW_SHOW);		// ShowWindowで表示

	// DirectX11の設定
	m_pWindow->InitDirect2D(hInstance, lpCmdLine, nShowCmd);

	// TRUEで返す
	return TRUE;
}

int CMainApplication::Run() {
	// 変数の宣言
	MSG msg;			// メッセージ情報msg
	LONG lCount = 0;	// ループ回数lCount

	FlyCap2CVWrapper FlyCap;
	FPSCounter FPS(10);

	HRESULT hr = S_OK;
	/* メッセージループ */
	while (TRUE) {
		// メッセージが来ているかを確認
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {	// PeekMessageでメッセージを確認
			if (GetMessage(&msg, NULL, 0, 0) > 0) {	// GetMessageでメッセージ取得.
				/* ウィンドウメッセージの送出 */
				DispatchMessage(&msg);	// DispatchMessageでウィンドウプロシージャに送出
				TranslateMessage(&msg);	// TranslateMessageで仮想キーメッセージを文字へ変換
			}
			else {	// WM_QUITまたはエラー
				// メッセージループを抜ける
				break;	// breakで抜ける
			}
		}
		else {	// メッセージが来ていない場合
			/* アイドル処理 */
			if (OnIdle(lCount)) {	// OnIdleにlCountを渡して, この中でアイドル処理をする
				lCount++;	// TRUEで返ってくるたびにlCountを増やす
			}
			/* レンダリング処理 */
			fps = FPS.GetFPS();
			//image = FlyCap.readImage();
			image = cv::imread("data/resources/handImage03.png");
			//cv::resize(image, image, cv::Size(), 0.5, 0.5);
			hr = this->m_pWindow->AppIdle(image, fps);
		}
	}

	/* 終了処理 */
	return ExitInstance();	// ExitInstanceの戻り値を返す
}