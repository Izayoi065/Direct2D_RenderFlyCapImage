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
/* 定義関数 */
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

 /* インクルードファイル */
#include "CViewDirect2D.h"

/** @brief CViewDirect2Dクラスのコンストラクタ
@note この関数は，このクラスが呼び出された際に，最初に実行される
@param pApp	CApplicationクラスのオブジェクト
@sa CApplication CWinBase
**/
CViewDirect2D::CViewDirect2D(CApplication * pApp) : CWinBase(pApp)
{
	MyOutputDebugString(L"	CViewDirect2D(CApplication*):CWinBase(CApplication)が呼び出されました！\n");
	m_pD2d1Factory = NULL;
	m_pRenderTarget = NULL;
	pTextFormat = NULL;
	pDWFactory = NULL;
	pBrush = NULL;
	pBitmap = NULL;	
	memory = new byte[size * size * 4 * 5];	// 要素数：幅*高さ(pixel)*4(byte/pixel)が5ブロック
	for (int i = 0;i < sizeof(memory);i++) {
		memory[i] = 0;
	}
}

/** @brief CViewDirect2Dクラスのデストラクタ
@note この関数は，このクラスが破棄される際に，最後に実行される
@sa ReleaseD2D
**/
CViewDirect2D::~CViewDirect2D()
{
	MyOutputDebugString(L"	~ViewDirectX11()が呼び出されました！\n");
	delete[] memory;
	ReleaseD2D();
}

/** @brief Direct2D関連の初期化を行う
@note この関数は，このクラスで使用されるDirect2Dのデバイス・インターフェース等を初期化する．
基本的には，このクラスのオブジェクトを生成した直後に使用する．
@param hInstance	
@param lpCmdLine	
@param nShowCmd		
@return HRESULTエラーコードを返す
@sa D2D1CreateFactory CreateHwndRenderTarget CreateBitmap DWriteCreateFactory CreateSolidColorBrush
**/
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

/** @brief このクラスで使用したDirect2D関連のデバイス・インターフェースを安全に破棄する
@note この関数は，このクラスで使用したDirect2D関連のデバイス・インターフェースを安全に破棄する
@sa SAFE_RELEASE
**/
void CViewDirect2D::ReleaseD2D()
{
	SAFE_RELEASE(pBitmap);
	SAFE_RELEASE(pBrush);
	SAFE_RELEASE(pDWFactory);
	SAFE_RELEASE(pTextFormat);
	SAFE_RELEASE(m_pRenderTarget);
	SAFE_RELEASE(m_pD2d1Factory);
}

/** @brief ウィンドウの任意の位置に画像データをメモリ上に配置する．
@note この関数は，ID2DBitmap::CopyFromMemoryを使用する前準備として，レンダリング対象の画像をメモリ上に配置する．
この機能では，DXGI_FORMAT_B8G8R8A8_UNORMで作成されたID2DBitmapに対応したデータ形式でメモリ上に配置するために使用される．
また，引数image_はOpenCVのcv::Mat型画像であり，データ形式はCV_8UC3とする(8bit3チャンネル)．
@param image_	メモリ上に配置する画像
@param data		画像データの配置先となるメモリのポインタ
@param num		配置する場所(num=1,2,3,4,5)
@sa	ID2DBitmap::CopyFromMemory InputArray 
**/
void CViewDirect2D::copyImageToMemory(cv::InputArray image_, byte* data, int num)
{
	cv::Mat image = image_.getMat();
#pragma omp parallel for
	for (int row = 0; row < size; row++) {
		cv::Vec3b *src = image.ptr<cv::Vec3b>(row);
		for (int col = 0; col < size; col++) {
			cv::Vec3b bgr = src[col];
			int pointBGR = row * size * 5 * 4 + col * 4 + size * 4 * (num-1);
			data[pointBGR + 0] = (byte)bgr[0];
			data[pointBGR + 1] = (byte)bgr[1];
			data[pointBGR + 2] = (byte)bgr[2];
		}
	}
}

/** @brief レンダリング関連のアイドル処理を行う．
@note この関数は，KHAKIのレンダリングに関するアイドル処理を行う．
@param image_	カメラからの入力画像
@param fps		システムが実際に稼働しているフレームレート
@return HRESULTエラーコードを返す
@sa	Render
**/
HRESULT CViewDirect2D::AppIdle(cv::InputArray image_, double fps)
{
	HRESULT hr = S_OK;
	/* cv::Mat形式で画像を取得 */
	cv::Mat image = image_.getMat();

	/* 画面の更新 */
	hr = Render(image, fps);

	return hr;
}

/** @brief ウィンドウへのレンダリングを実行する．
@note この関数は，KHAKIのメインウィンドウへ各画像のレンダリング処理を適用する．
この機能では，Direct2Dを用いた全てのレンダリングが行われる．
@param image_	カメラからの入力画像
@param fps		システムが実際に稼働しているフレームレート
@return HRESULTエラーコードを返す
@sa copyImageToMemory m_pRenderTarget
**/
HRESULT CViewDirect2D::Render(cv::InputArray image_, double fps)
{
	MyOutputDebugString(L"	Render()を実行しました．\n");
	HRESULT hResult = S_OK;
	/* ①cv::Mat形式で画像を取得 */
	renderImage01 = image_.getMat();	// カメラからの入力画像
	cv::threshold(renderImage01, renderImage02, 100, 255, cv::THRESH_BINARY);

	/* ②手指領域のみを抽出する */
	/* ③掌の中心位置を推定する */
	/* ④手指の状態を解析する */
	/* ⑤インプットモードのモデルを作成 */

	/* 画像データを確保済みのメモリ上へ書き込み */
	copyImageToMemory(renderImage01, this->memory, 1);	// ①カメラからの入力画像をメモリ上に配置
	copyImageToMemory(renderImage02, this->memory, 2);	// ②手指領域の抽出画像をメモリ上に配置
	copyImageToMemory(renderImage01, this->memory, 3);	// ③掌の中心位置の推定画像をメモリ上に配置
	copyImageToMemory(renderImage01, this->memory, 4);	// ④解析情報の取得画像をメモリ上に配置
	copyImageToMemory(renderImage01, this->memory, 5);	// ⑤インプットモードの判別画像をメモリ上に配置

	// ターゲットサイズの取得
	D2D1_SIZE_F oTargetSize = m_pRenderTarget->GetSize();

	// 描画開始
	PAINTSTRUCT tPaintStruct;
	::BeginPaint(this->m_hWnd, &tPaintStruct);

	/*
	レンダリング処理
	*/
	{
		/* 開始 */
		m_pRenderTarget->BeginDraw();

		/* 背景のクリア */
		D2D1_COLOR_F oBKColor = D2D1::ColorF(D2D1::ColorF::LightSlateGray);
		m_pRenderTarget->Clear(oBKColor);

		/* Bitmapの描画 */
		{
			pBitmap->CopyFromMemory(NULL, memory, size * 5 * 4);
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

/** @brief ウィンドウ作成時のハンドラ
@note この関数は，ウィンドウを作成した際にハンドラとして呼び出される．
@return ウィンドウ作成に成功したフラグ
**/
int CViewDirect2D::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	// ウィンドウ作成成功
	return 0;
}

/** @brief プログラムを終了させるために，メッセージループを終了させる．
@note この関数は，システムを終了させる際に呼び出され，メッセージループを終了させる．
@sa	PostQuitMessage
**/
void CViewDirect2D::OnDestroy() {
	// メッセージループ終了
	PostQuitMessage(0);	// PostQuitMessageでメッセージループを終わらせる
}