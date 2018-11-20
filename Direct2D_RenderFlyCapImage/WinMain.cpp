/****************************************************************************************************
 *タイトル：Win32API アプリケーションにおけるメインメソッド
 *説明　　：システムのエントリポイント
 *		：https://www.ptgrey.com/flycapture-sdk
 *外部LIB ：FlyCapture2	2.10.3.169，
 *			OpenCV		3.4.0
 *
 *著作権　：Tomoki Kondo
 *
 *変更履歴：2018.11.16 Fryday
 *　　　：新規登録
 *
 ****************************************************************************************************/
 /* 定義関数 */
#define SAFE_DALETE(x) {if(x) {delete (x); (x) = NULL;}}	// 生成したオブジェクトを開放し，NULLを代入する

/* インクルードファイル */
#include "CMainApplication.h"	// メインアプリケーションクラス

int WINAPI _tWinMain(
	HINSTANCE hInstance,		// 現在のインスタンス・ハンドル
	HINSTANCE hPrevInstance,	// 以前のインスタンス・ハンドル
	LPTSTR lpCmdLine,			// コマンドラインの文字列
	int nCmdShow)				// ウィンドウの表示状態
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 変数・オブジェクトの宣言
	CApplication *pApp;	// CApplicationオブジェクトポインタpApp
	int iRet;	// Runの戻り値:iRet

	// メインアプリケーションオブジェクトの生成
	pApp = new CMainApplication();	// CMainApplicationオブジェクトを作成

	// インスタンス初期化
	if (!pApp->InitInstance(hInstance, lpCmdLine, nCmdShow)) {	// InitInstanceで初期化
		/* FALSEなら終了処理 */
		pApp->ExitInstance();	// ExitInstanceで終了処理
		delete pApp;			// pAppを解放
		return -1;				// 異常終了なので-1を返す
	}

	// メッセージループ
	iRet = pApp->Run();	// Runでメッセージループ, 終了したら戻り値をiRetに

	// アプリケーションオブジェクトの開放
	SAFE_DALETE(pApp);

	// プログラムの終了
	return 0;	// 正常終了の0を返す
}