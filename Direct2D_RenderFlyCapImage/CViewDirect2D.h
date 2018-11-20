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
	ID2D1Factory * m_pD2d1Factory;				// 
	ID2D1HwndRenderTarget * m_pRenderTarget;	// レンダリングのターゲット
	IDWriteTextFormat* pTextFormat;				// テキストの書式設定に使用するフォント プロパテ，段落プロパティ及びロケール情報
	IDWriteFactory* pDWFactory;					// 全てのDirectWriteオブジェクトのルート・ファクトリ・インターフェース
	ID2D1SolidColorBrush* pBrush;				// 
	ID2D1Bitmap * pBitmap;						// 
	std::wstring strText = L"";					// ウィンドウに表示するfps用テキスト
	byte *memory;	// cv::Mat -> ID2D1Bitmap用のバッファ
	const unsigned size = 504U;	// カメラからの入力画像の1辺のサイズ
	double totalTime = 0;		// 描画されたフレームレートの更新タイミングを1秒毎に固定するための管理用
public:
	FlyCap2CVWrapper* FlyCap;
	cv::Mat renderImage01;		// ①カメラからの入力画像
	cv::Mat renderImage02;		// ②手指領域の抽出画像
	cv::Mat renderImage03;		// ③掌の中心位置の推定画像
	cv::Mat renderImage04;		// ④解析情報の取得画像
	cv::Mat renderImage05;		// ⑤インプットモードの判別画像
public:
	CViewDirect2D(CApplication *pApp);
	~CViewDirect2D();
	HRESULT InitDirect2D(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd);
	void ReleaseD2D();
	void copyImageToMemory(cv::InputArray image_, byte* data, int num);
	HRESULT AppIdle(cv::InputArray image_, double fps);
	HRESULT	Render(cv::InputArray image_, double fps);

	/* 仮想関数 */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);	// ウィンドウ作成時のハンドラOnCreate
	virtual void OnDestroy();	// ウィンドウ破棄時のハンドラ Ondestroy
};

