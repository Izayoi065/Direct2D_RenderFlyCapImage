#pragma once
/* 前方宣言 */
class CViewDirect2D;	// ウィンドウクラス

class CApplication
{
public:
	/* publicメンバ変数 */
	HINSTANCE m_hInstance;	// アプリケーションインスタンスハンドルm_hInstance
	CViewDirect2D *m_pWindow;		// ウィンドウクラスポインタm_pWindow
	double fps;
public:
	CApplication();				// コンストラクタ
	virtual ~CApplication();	// デストラクタ

	/* 仮想関数 */
	virtual int Run();	// メッセージループ関数Run
	virtual int ExitInstance();	// インスタンス終了処理関数ExitInstance
	virtual BOOL OnIdle(LONG lCount);	// アイドル処理関数OnIdle
	virtual void getInputImage(cv::OutputArray outImage_);

	/* 純粋仮想関数 */
	virtual BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd) = 0;		// インスタンス初期化関数 InitInstance
};

