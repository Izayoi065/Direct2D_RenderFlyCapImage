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
	rc.right = 900;		// 100 + 800
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
			hr = this->m_pWindow->AppIdle(FlyCap.readImage(), FPS.GetFPS());
		}
	}

	/* 終了処理 */
	return ExitInstance();	// ExitInstanceの戻り値を返す
}