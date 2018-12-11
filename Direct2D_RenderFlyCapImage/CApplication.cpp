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

#include "CApplication.h"

/* コンストラクタ CApplication */
CApplication::CApplication() {
	MyOutputDebugString(L"	CApplication::CApplication() メソッドが呼び出されました．\n");

	/* メンバの初期化 */
	m_hInstance = NULL;	// インスタンスハンドルをNULL
	m_pWindow = NULL;	// ウィンドウオブジェクトをNULL
	FlyCap = new FlyCap2CVWrapper();
}

/* デストラクタ ~CApplication */
CApplication::~CApplication() {
	MyOutputDebugString(L"	CApplication::~CApplication() メソッドが呼び出されました．\n");
	/* ウィンドウオブジェクトが残っていたら解放 */
	SAFE_DALETE(m_pWindow);
}

/* システムの実行部分 Run */
int CApplication::Run() {
	// 変数の宣言
	MSG msg;			// メッセージ情報msg
	LONG lCount = 0;	// ループ回数lCount

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
		}
	}

	/* 終了処理 */
	return ExitInstance();	// ExitInstanceの戻り値を返す
}

/* インスタンス終了処理関数 ExitInstance */
int CApplication::ExitInstance() {
	/*インスタンス終了処理の通知 */
	MyOutputDebugString(L"	CApplication::ExitInstance() メソッドが呼び出されました．\n");

	/* ウィンドウオブジェクトが残っていたら解放 */
	SAFE_DALETE(m_pWindow);

	/* 正常終了 */
	return 0;
}

/* アイドル処理関数 OnIdle */
BOOL CApplication::OnIdle(LONG lCount) {
	// とりあえずTRUE
	return TRUE;
}

void CApplication::getInputImage(cv::OutputArray outImage_)
{
	cv::Mat image;
	image = FlyCap->readImage();
	//image = cv::imread("data/resources/handImage03.png");
	cv::resize(image, image, cv::Size(), 0.5, 0.5);

	image.copyTo(outImage_);
}
