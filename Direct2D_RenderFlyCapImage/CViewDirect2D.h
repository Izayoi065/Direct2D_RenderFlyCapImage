#pragma once
#include "CWinBase.h"

/* 前方宣言 */
class C_WinBase;
class CApplication;
class FlyCap2CVWrapper;

class CViewDirect2D :
	public CWinBase
{
private:
	BOOL g_InitD2D = false;		// 2重初期化防止
	ID2D1Factory * m_pD2d1Factory;					// 
	ID2D1HwndRenderTarget * m_pRenderTarget;		// レンダリングのターゲット
	IDWriteTextFormat* pTextFormat;					// テキストの書式設定に使用するフォント プロパテ，段落プロパティ及びロケール情報
	IDWriteFactory* pDWFactory;						// 全てのDirectWriteオブジェクトのルート・ファクトリ・インターフェース
	ID2D1SolidColorBrush* pBrush;
	ID2D1Bitmap * pBitmap;
	std::wstring strText = L"";
	byte *memory;	// cv::Mat -> ID2D1Bitmap用のバッファ
	const unsigned size = 504U;	// Bitmapのサイズ
	double totalTime = 0;
public:
	FlyCap2CVWrapper* FlyCap;
	cv::Mat renderImage01;
	cv::Mat renderImage02;
	cv::Mat renderImage03;
	cv::Mat renderImage04;
	cv::Mat renderImage05;
public:
	CViewDirect2D(CApplication *pApp);
	~CViewDirect2D();
	HRESULT InitDirect2D(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd);
	void ReleaseD2D();
	void copyImageToMemory(cv::InputArray image_, byte& memory, int num);
	HRESULT AppIdle(cv::InputArray image_, double fps);
	HRESULT	Render(cv::InputArray image_, double fps);

	/* 仮想関数 */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);	// ウィンドウ作成時のハンドラOnCreate
	virtual void OnDestroy();	// ウィンドウ破棄時のハンドラ Ondestroy
};

