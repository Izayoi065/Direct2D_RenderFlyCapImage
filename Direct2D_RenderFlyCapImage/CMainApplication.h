#pragma once
/* インクルードファイル */
#include "CApplication.h"	// アプリケーションクラス

/* 前方宣言 */
class CApplication;

class CMainApplication :
	public CApplication
{
private:
	cv::Mat image;
public:
public:
	/* publicメンバ関数 */
	CMainApplication();				// コンストラクタ
	virtual ~CMainApplication();	// デストラクタ

	/* 仮想関数 */
	virtual BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd);	// インスタンス初期化関数 InitInstance
	virtual int Run();
};

