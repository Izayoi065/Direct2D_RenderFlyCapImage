#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#include "CViewDirect2D.h"

CViewDirect2D::CViewDirect2D(CApplication * pApp) : CWinBase(pApp)
{
	MyOutputDebugString(L"	CViewDirect2D(CApplication*):CWinBase(CApplication)が呼び出されました！\n");
	m_pD2d1Factory = NULL;
	m_pRenderTarget = NULL;
	pTextFormat = NULL;
	pDWFactory = NULL;
	pBrush = NULL;
	pBitmap = NULL;	
}


CViewDirect2D::~CViewDirect2D()
{
	MyOutputDebugString(L"	~ViewDirectX11()が呼び出されました！\n");
	ReleaseD2D();
}

HRESULT CViewDirect2D::InitDirect2D(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	CRect rect;
	::GetClientRect(this->m_hWnd, &rect);	// クライアント領域のサイズを取得

	HRESULT hResult = S_OK;

	/*
	ID2D1Factoryの生成
	*/
	hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pD2d1Factory);
	if (FAILED(hResult)) {
		// エラー
		MyOutputDebugString(L"	ID2D1Factoryの生成に失敗しました．");
	}

	/*
	ID2D1HwndRenderTargetの生成
	*/
	D2D1_SIZE_U PixelSize = { rect.Width(), rect.Height() };
	D2D1_RENDER_TARGET_PROPERTIES RenderTargetProperties = D2D1::RenderTargetProperties();
	D2D1_HWND_RENDER_TARGET_PROPERTIES HwndRenderTargetProperties = D2D1::HwndRenderTargetProperties(this->m_hWnd, PixelSize);
	HwndRenderTargetProperties.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY;	// 垂直同期しない	http://www.wisdomsoft.jp/444.html
	hResult = m_pD2d1Factory->CreateHwndRenderTarget(
		RenderTargetProperties,
		HwndRenderTargetProperties,
		&m_pRenderTarget);
	if (FAILED(hResult)) {
		// エラー
		MyOutputDebugString(L"	ID2D1HwndRenderTargetの生成に失敗しました．");
	}

	/* ID2D1Bitmapの生成 */
	m_pRenderTarget->CreateBitmap(
		D2D1::SizeU(size*5, size),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&pBitmap);
	
	/* IDWriteFactoryの生成 */
	hResult = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWFactory));
	if (FAILED(hResult)) {
		MyOutputDebugString(L"	DWriteCreateFactoryは失敗しました！\n");
	}

	/* ブラシの生成 */
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White)
		, &pBrush
	);

	g_InitD2D = true;

	return hResult;
}

void CViewDirect2D::ReleaseD2D()
{
	SAFE_RELEASE(pBitmap);
	SAFE_RELEASE(pBrush);
	SAFE_RELEASE(pDWFactory);
	SAFE_RELEASE(pTextFormat);
	SAFE_RELEASE(m_pRenderTarget);
	SAFE_RELEASE(m_pD2d1Factory);
}

HRESULT CViewDirect2D::AppIdle(cv::InputArray image_, double fps)
{
	HRESULT hr = S_OK;
	/* cv::Mat形式で画像を取得 */
	cv::Mat image = image_.getMat();

	/* 画面の更新 */
	hr = Render(image, fps);

	return hr;
}

HRESULT CViewDirect2D::Render(cv::InputArray image_, double fps)
{
	MyOutputDebugString(L"	Render()を実行しました．\n");
	HRESULT hResult = S_OK;
	/* cv::Mat形式で画像を取得 */
	cv::Mat orgImage = image_.getMat();
	cv::Mat dstImage01 = image_.getMat();
	cv::Mat dstImage02 = image_.getMat();
	cv::Mat dstImage03 = image_.getMat();
	cv::Mat dstImage04 = image_.getMat();

	cv::hconcat(orgImage, dstImage01);
	cv::hconcat(dstImage01, dstImage02);
	cv::hconcat(dstImage02, dstImage03);
	cv::hconcat(dstImage03, dstImage04);
	byte *memory;
	memory = new byte[size * 2 * size * 4];

	/* 画像データを確保済みのメモリ上へ書き込み */
#pragma omp parallel for
	for (int row = 0; row < dstImage04.rows; row++) {
		cv::Vec3b *src = dstImage04.ptr<cv::Vec3b>(row);
		for (int col = 0; col < dstImage04.cols; col++) {
			cv::Vec3b bgr = src[col];
			int pointBGR = col * 4 + row * dstImage04.rows * 4;
			memory[pointBGR + 0] = bgr[0];
			memory[pointBGR + 1] = bgr[1];
			memory[pointBGR + 2] = bgr[2];
		}
	}

	// ターゲットサイズの取得
	D2D1_SIZE_F oTargetSize = m_pRenderTarget->GetSize();

	// 描画開始
	PAINTSTRUCT tPaintStruct;
	::BeginPaint(this->m_hWnd, &tPaintStruct);

	/*
	テキストの描画
	*/
	{
		/* 開始 */
		m_pRenderTarget->BeginDraw();

		/* 背景のクリア */
		D2D1_COLOR_F oBKColor = D2D1::ColorF(D2D1::ColorF::LightSlateGray);
		m_pRenderTarget->Clear(oBKColor);

		/* Bitmapの描画 */
		{
			pBitmap->CopyFromMemory(NULL, memory, size * 4);
			delete[] memory;
			m_pRenderTarget->DrawBitmap(
				pBitmap, //the bitmap to draw [a portion of],
				D2D1::RectF(0.0f, 0.0f, 200*5, 200), //destination rectangle,
				1.0f, //alpha blending multiplier,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, //interpolation mode,
				D2D1::RectF(0.0f, 0.0f, size*5, size)); //source rectangle
		}

		/* テキストフォーマットの生成 */
		{
			pDWFactory->CreateTextFormat(
				L"Meiryo"						// フォント名
				, NULL							// フォントコレクションの指定
				, DWRITE_FONT_WEIGHT_NORMAL		// テキストオブジェクトのフォントの太さ
				, DWRITE_FONT_STYLE_NORMAL		// テキストオブジェクトのフォントスタイル
				, DWRITE_FONT_STRETCH_NORMAL	// テキストオブジェクトのフォント伸縮
				, 16							// フォントサイズ(DIP単位)
				, L""							// ロケール名を含む文字配列
				, &pTextFormat					// 新しく作成されたテキスト形式オブジェクトへのポインタアドレスを格納
			);
		}

		/* 経過時間が1秒未満であれば描画するfpsのテキストを更新しない */
		if (totalTime < 1.0f) {
			totalTime += (1 / fps);
		}
		else {
			strText = L"現在の実行速度：" + std::to_wstring((int)fps) + L"fps";
			totalTime = 0.0;
		}

		/* テキストの描画 https://msdn.microsoft.com/en-us/library/Dd371919(v=VS.85).aspx */
		if (NULL != pBrush && NULL != pTextFormat) {
			m_pRenderTarget->DrawText(
				strText.c_str()		// 文字列
				, strText.size()    // 文字数
				, pTextFormat		// 描画するテキストの書式設定の詳細
				, &D2D1::RectF(0, 0, oTargetSize.width, oTargetSize.height) // テキストが描画される領域のサイズと位置
				, pBrush			// テキストをペイントするために使用されるブラシ
				, D2D1_DRAW_TEXT_OPTIONS_NONE	// テキストをピクセル境界にスナップするかどうか、テキストをレイアウト矩形にクリップするかどうかを示す値
			);
		}

		/* 終了 */
		m_pRenderTarget->EndDraw();
	}

	// 描画終了
	::EndPaint(this->m_hWnd, &tPaintStruct);

	return hResult;
}

/* ウィンドウ作成時のハンドラ OnCreate */
int CViewDirect2D::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	// ウィンドウ作成成功
	return 0;
}

/* ウィンドウ破棄時のハンドラ OnDestroy */
void CViewDirect2D::OnDestroy() {
	// メッセージループ終了
	PostQuitMessage(0);	// PostQuitMessageでメッセージループを終わらせる
}