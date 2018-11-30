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
#define Def_FingerW 0.015f//指の幅
#define Def_ActiveLim 0.9f//尤度領域がこの値以内であれば手指領域と判定する
#define Def_ChainRange 1.5f//尤度のピークの最大接続距離
#define Def_MaxChain 20//指候補の最大保持数
#define Def_MAXActiveRate 0.9f	// 入力画像の撮影された範囲の有効半径
#define Hand_DomeR 0.1f			//アクリルドームの半径

/* インクルードファイル */
#include "CViewDirect2D.h"	// DirectX2D関連の初期化クラス
#include "resource.h"		// リソースファイル

/* staticメンバ変数の定義 */
XMFLOAT3 CViewDirect2D::m_pV3PixToVec[CViewDirect2D::size*CViewDirect2D::size];	// staticメンバ変数CWinBase:: m_pV3PixToVecは宣言と別にここに定義しないといけない.
float *ppi4_Peek[Def_NumSmpR];//
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
	FingerWidth = (int)(size*Def_FingerW) / (Hand_DomeR*Def_FOV);
#pragma omp parallel for
	for (int i = 0; i < sizeof(memory); i++) {
		memory[i] = 0;
	}

	int NumRadius = (size / 2)*Def_MAXActiveRate;
	int NumRadiusSq = NumRadius * NumRadius;
#pragma omp parallel for
	for (int x = 0; x<size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			float tRad = (x - (size / 2)) * (x - (size / 2)) * (y - (size / 2)) * (y - (size / 2));
			if (tRad < NumRadiusSq)
				ActiveCameraArea[y*size + x] = true;
			else
				ActiveCameraArea[y*size + x] = false;
		}
	}

	m_pNumAngle = (int*)malloc(sizeof(m_pNumAngle[0])*(int)NumRadius);
	m_ppAngleX = (int **)malloc(sizeof(m_ppAngleX[0])*(int)NumRadius);
	m_ppAngleY = (int **)malloc(sizeof(m_ppAngleY[0])*(int)NumRadius);
#pragma omp parallel for
	for (int IndexRad = 0; IndexRad < (int)NumRadius; IndexRad++)
	{
		m_pNumAngle[IndexRad] = (int)(2.0f*(IndexRad)*Def_PI);
		m_ppAngleX[IndexRad] = (int *)malloc(sizeof(m_ppAngleX[0])*m_pNumAngle[IndexRad]);
		m_ppAngleY[IndexRad] = (int *)malloc(sizeof(m_ppAngleY[0])*m_pNumAngle[IndexRad]);
		for (int IndexAngle = 0; IndexAngle < (int)m_pNumAngle[IndexRad]; IndexAngle++)
		{
			m_ppAngleX[IndexRad][IndexAngle] = (int)((IndexRad)*cosf(2.0f*Def_PI*(float)IndexAngle / (float)m_pNumAngle[IndexRad]));
			m_ppAngleY[IndexRad][IndexAngle] = (int)((IndexRad)*sinf(2.0f*Def_PI*(float)IndexAngle / (float)m_pNumAngle[IndexRad]));
		}
	}

#pragma omp parallel for
	for (int y = 0; y<size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			float tX = IndexToFloat(x, -0.5f, 1.0f / size);
			float tZ = IndexToFloat(y, -0.5f, 1.0f / size);
			float tY = atan2f(tX, tZ);
			float tP = sqrtf(tX*tX + tZ * tZ)*Def_PI;

			m_pV3PixToVec[y*size + x].x = sinf(tY)*sinf(tP);
			m_pV3PixToVec[y*size + x].y = cosf(tP);
			m_pV3PixToVec[y*size + x].z = cosf(tY)*sinf(tP);
		}
	}

	//ストレッチテーブルの作成
	{
		float f4_tBase = -sin(Def_FOV / 2.0f);
		float f4_tPitch = -f4_tBase * 2.0f / size;
		float f4_tLikelihood = cos(Def_FOV / 2.0f);

		for (int i4_tIndY = 0; i4_tIndY < size; i4_tIndY++)
		{
			float f4_tY0 = IndexToFloat(i4_tIndY, f4_tBase, f4_tPitch);
			for (int i4_tIndX = 0; i4_tIndX < size; i4_tIndX++)
			{
				float f4_tX0 = IndexToFloat(i4_tIndX, f4_tBase, f4_tPitch);
				float f4_tR0 = sqrtf(f4_tX0*f4_tX0 + f4_tY0 * f4_tY0);//補正前半径
				float f4_tA = atanf(f4_tR0 / f4_tLikelihood);//半径→角度
				float f4_tR1 = f4_tA / (Def_FOV / 2.0f);//補正後半径

				float f4_tX1 = f4_tX0 * f4_tR0 / f4_tR1;
				float f4_tY1 = f4_tY0 * f4_tR0 / f4_tR1;

				FixX[i4_tIndY*size + i4_tIndX] = FloatToIndex(f4_tX1, f4_tBase, f4_tPitch);
				FixY[i4_tIndY*size + i4_tIndX] = FloatToIndex(f4_tY1, f4_tBase, f4_tPitch);
			}
		}
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
	free(m_ppAngleY);
	free(m_ppAngleX);
	free(m_pNumAngle);
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
	::GetClientRect(this->m_hWndViewTarget, &rect);	// クライアント領域のサイズを取得

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
	D2D1_HWND_RENDER_TARGET_PROPERTIES HwndRenderTargetProperties = D2D1::HwndRenderTargetProperties(this->m_hWndViewTarget, PixelSize);
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
#pragma omp parallel for	//  cv::Mat型画像データをbyte型配列の指定位置に対応する場所に格納していく
	for (int row = 0; row < size; row++) {
		cv::Vec3b *src = image.ptr<cv::Vec3b>(row);
		for (int col = 0; col < size; col++) {
			cv::Vec3b hsv = src[col];
			int pointBGR = row * size * 5 * 4 + col * 4 + size * 4 * (num-1);
			data[pointBGR + 0] = (byte)hsv[0];
			data[pointBGR + 1] = (byte)hsv[1];
			data[pointBGR + 2] = (byte)hsv[2];
		}
	}
}

/** @brief 手指領域の抽出処理を実行する．
@note この関数は，入力画像から手指領域を抽出処理を適用する．出力では，抽出された手指領域の二値化画像が出力される．
@param image_		 カメラからの入力画像
@param outImage_	抽出された手指領域の二値化画像
@sa	Render
**/
void CViewDirect2D::handExtractor(cv::InputArray inImage_, cv::OutputArray outImage_)
{
	cv::Mat inImage = inImage_.getMat();
	cv::Mat hsvImage, hsv_mask, dstImage;
	cv::cvtColor(inImage, hsvImage, CV_BGR2HSV);
	cv::inRange(hsvImage, hsv_min, hsv_max, hsv_mask);
	cv::cvtColor(hsv_mask, dstImage, CV_GRAY2BGR);
#pragma omp parallel for	 // HSVの各チャンネルをfloat型(0～1)で取得
	for (int row = 0; row < size; row++) {
		cv::Vec3b *src = hsvImage.ptr<cv::Vec3b>(row);
		for (int col = 0; col < size; col++) {
			cv::Vec3b hsv = src[col];
			int pointBGR = row * size + col;
			ppf4_Hue[pointBGR] = (byte)hsv[0] / 180;
			ppf4_Saturation[pointBGR] = (byte)hsv[1] / 255;
			ppf4_Value[pointBGR] = (byte)hsv[2] / 255;
		}
	}
	dstImage.copyTo(outImage_);
}

/** @brief 手指領域の重心を推定する
@note この関数は，手指領域の2値化画像(8UC3)から重心を算出し，ドーム上の3D位置を推定する．
この機能では，inImage_で取得した手指領域の二値化画像の重心を算出し，魚眼レンズやアクリルドームによる歪みを補正する．
また，先の重心算出では肌色抽出の精度等により正確な値でない可能性があるため，遺伝的アルゴリズムにより正確な値を求め，再び歪み補正を行う．
@param inImage_		手指領域の2値化画像(8UC3)
@param outImage_	手指領域の重心推定を行った画像
@sa cv::moments m_handInfo
**/
void CViewDirect2D::CalcHandCentroid(cv::InputArray inImage_, cv::OutputArray outImage_)
{
	cv::Mat inImage = inImage_.getMat();
	cv::Mat single_chImage, dstImage;
	inImage.copyTo(dstImage);
	cv::cvtColor(inImage, single_chImage, CV_BGR2GRAY);

	cv::Moments mu = cv::moments(single_chImage, false);
	cv::Point2f mc = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
	m_handInfo.hand2DPosX = (int)mc.x;
	m_handInfo.hand2DPosY = (int)mc.y;

	m_handInfo.hand3DPosX = m_pV3PixToVec[m_handInfo.hand2DPosY*size + m_handInfo.hand2DPosX].x;
	m_handInfo.hand3DPosY = m_pV3PixToVec[m_handInfo.hand2DPosY*size + m_handInfo.hand2DPosX].y;
	m_handInfo.hand3DPosZ = m_pV3PixToVec[m_handInfo.hand2DPosY*size + m_handInfo.hand2DPosX].z;

	/* 画像のゆがみ補正 */
	/* 手指領域の重心位置の再計算 */
	/* 画像のゆがみ補正 */

	cv::circle(dstImage, cv::Point(m_handInfo.hand2DPosX, m_handInfo.hand2DPosY), 5, cv::Scalar(0, 255, 0), -1, CV_AA);

	dstImage.copyTo(outImage_);
}

/** @brief 手指情報の解析を行う
@note この関数は，取得した画像から手指情報の解析を行い，各指の関節の3D位置や入力モードの判定等を行う．
@param inImage_		手指領域の2値化画像(8UC3)
@param outImage_	手指領域の重心推定を行った画像
@sa cv::moments m_handInfo
**/
void CViewDirect2D::AnalyzeHandInf(cv::InputArray inImage_, cv::OutputArray outImage_)
{
	cv::Mat inImage = inImage_.getMat();
	cv::Mat dstImage;

	/* 第2指～第5指の検出 */
	detectFinger2to5(size / 2, size / 2, renderImage02, ppf4_Hue, ppf4_Saturation, ppf4_Value, &m_handInfo, inImage, dstImage);
	/* 第1指付け根の推定 */
	/* 第1指の検出 */
	/* 入力モードの判定 */
	/* 手指の距離検出 */
	/* 各ポイントでの圧力検出 */


	dstImage.copyTo(outImage_);
}

int CViewDirect2D::detectFinger2to5(int tCenterX, int tCenterY, cv::InputArray likelihoodArea, 
	float * p_HueImage, float * p_SaturationImage, float * ValueImage, S_HANDINF * pHandInf_t,cv::InputArray inImage_, cv::OutputArray outImage_)
{
	cv::Mat inImage = inImage_.getMat();
	cv::Mat binImage, dstImage;
	cv::cvtColor(inImage, binImage, CV_BGR2GRAY);
	XMFLOAT2 pV2_ChainRoot[Def_MaxChain];
	XMFLOAT2 pV2_ChainVec[Def_MaxChain];
	float p_ChainLen[Def_MaxChain];
	int paramR = (int)((size*pHandInf_t->handRotate) / (Hand_DomeR*Def_FOV));

	int SampleGap = 8;
	int MinR = paramR + SampleGap;
	int MaxR = paramR * 3.5f;
	if (((size / 2)*Def_MAXActiveRate) <= MaxR)
		MaxR = (size / 2)*Def_MAXActiveRate - 1;

	float tlikelihoodArea[size*size];
#pragma omp parallel for	 // HSVの各チャンネルをfloat型(0～1)で取得
	for (int row = 0; row < size; row++) {
		cv::Vec3b *src = binImage.ptr<cv::Vec3b>(row);
		for (int col = 0; col < size; col++) {
			cv::Vec3b bin = src[col];
			int pointBGR = row * size + col;
			tlikelihoodArea[pointBGR] = (byte)bin[0] / 255;
		}
	}

	int tNumFinger = FindChain(tCenterX, tCenterY, paramR, MinR, MaxR, Def_NumSmpR, 0.5f, 0.5f, tlikelihoodArea, 
		p_HueImage, p_SaturationImage, ValueImage, inImage, dstImage, pV2_ChainRoot, pV2_ChainVec, p_ChainLen);

	if (tNumFinger <= 3)
	{
		return tNumFinger;
	}

	tNumFinger = FindGroup2to5(tNumFinger, pV2_ChainRoot, pV2_ChainVec, p_ChainLen, pHandInf_t);

	dstImage.copyTo(outImage_);

	return tNumFinger;
}

int CViewDirect2D::FindChain(int tCenterX, int tCenterY, int Line2CircleR, int MinR, int MaxR, int NumSmpR,
	float TergetAng, float SearchAng, float * pf4_tLikelihood, float * pf4_tH_In, float * pf4_tS_In, float * pf4_tV_In,
	cv::InputArray inImage_, cv::OutputArray outImage_, XMFLOAT2 * pV2_tChainRoot, XMFLOAT2 * pV2_tChainVec, float * pf4_tChainLen)
{
	cv::Mat inImage = inImage_.getMat();
	int pi4_NumChain[Def_NumSmpR];//
	int pi4_IndBaseR[Def_NumSmpR];//掌の色勾配テーブル
	if (size / 2 < MaxR)MaxR = size / 2;
	int i4_tSampleGap = (MaxR - MinR) / NumSmpR;//同心円最大半径から最小半径のそれぞれの間隔


	//テーブルの作成
	for (int i4_tIndSmpR = 0; i4_tIndSmpR < NumSmpR; i4_tIndSmpR++)
	{
		pi4_IndBaseR[i4_tIndSmpR] = i4_tIndSmpR * i4_tSampleGap + MinR;//[0]で最小半径の色勾配

		float f4_tBegAng = TergetAng - SearchAng;
		if (f4_tBegAng < 0.0f)f4_tBegAng += 1.0f;
		int i4_tBegAng = (int)(m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] * f4_tBegAng);
		int i4_tNumAng = (int)(m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] * SearchAng * 2);

		for (int i4_tIndAng = 0; i4_tIndAng < i4_tBegAng; i4_tIndAng++)//初期化
		{
			ppf4_Sample[i4_tIndSmpR][i4_tIndAng] = 0.0f;
		}

		for (int i4_tIndAng0 = i4_tBegAng; i4_tIndAng0 < i4_tBegAng + i4_tNumAng; i4_tIndAng0++)
		{
			int i4_tIndAng1 = i4_tIndAng0;
			if (m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] <= i4_tIndAng1)i4_tIndAng1 -= m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]];

			int i4_tX = tCenterX + m_ppAngleX[pi4_IndBaseR[i4_tIndSmpR]][i4_tIndAng1];
			int i4_tY = tCenterY + m_ppAngleY[pi4_IndBaseR[i4_tIndSmpR]][i4_tIndAng1];

			if (0 < i4_tX && i4_tX < size && 0 < i4_tY && i4_tY < size)
			{
				ppf4_Sample[i4_tIndSmpR][i4_tIndAng1] = pf4_tLikelihood[i4_tY*size + i4_tX] + pf4_tV_In[i4_tY*size + i4_tX];

				if (Def_ActiveLim < ppf4_Sample[i4_tIndSmpR][i4_tIndAng1])
				{
					cv::circle(inImage, cv::Point(i4_tX, i4_tY), 5, cv::Scalar(0, 0, 255), -1, CV_AA);
				}
				else
				{
					cv::circle(inImage, cv::Point(i4_tX, i4_tY), 5, cv::Scalar(0, 255, 0), -1, CV_AA);
					ppf4_Sample[i4_tIndSmpR][i4_tIndAng1] = 0.0f;
				}
			}
		}

		for (int i4_tIndAng = i4_tBegAng + i4_tNumAng; i4_tIndAng < m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]]; i4_tIndAng++)
		{
			ppf4_Sample[i4_tIndSmpR][i4_tIndAng] = 0.0f;
		}
	}

	//平滑化
	for (int i4_tIndSmpR = 0; i4_tIndSmpR < NumSmpR; i4_tIndSmpR++)
	{
		for (int i4_tIndAng = 0; i4_tIndAng < m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]]; i4_tIndAng++)
		{
			ppf4_SampleS[i4_tIndSmpR][i4_tIndAng] = 0;

			for (int i4_tIndS = -FingerWidth / 2; i4_tIndS <= FingerWidth / 2; i4_tIndS++)
			{
				int i4_tIndBase = i4_tIndAng + i4_tIndS;
				if (i4_tIndBase < 0)i4_tIndBase += m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]];
				if (m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] <= i4_tIndBase)i4_tIndBase -= m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]];

				ppf4_SampleS[i4_tIndSmpR][i4_tIndAng] += ppf4_Sample[i4_tIndSmpR][i4_tIndBase];
			}

			ppf4_SampleS[i4_tIndSmpR][i4_tIndAng] /= FingerWidth * 2 + 1;
		}
	}

	//ピークを求める
	{
		int i4_tPrev;
		int i4_tNext;
		float f4_tPrevG;
		float f4_tNextG;

		int ppi4_tPeek[Def_NumSmpR][Def_MaxChain];

		for (int i4_tIndSmpR = 0; i4_tIndSmpR < NumSmpR; i4_tIndSmpR++)
		{
			pi4_NumChain[i4_tIndSmpR] = 0;

			for (int i4_tIndAng = 0; i4_tIndAng < m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]]; i4_tIndAng++)
			{
				i4_tPrev = i4_tIndAng - 1;
				if (i4_tPrev < 0)i4_tPrev += m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]];
				i4_tNext = i4_tIndAng + 1;
				if (m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] <= i4_tNext)i4_tNext -= m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]];

				f4_tPrevG = ppf4_SampleS[i4_tIndSmpR][i4_tIndAng] - ppf4_SampleS[i4_tIndSmpR][i4_tPrev];
				f4_tNextG = ppf4_SampleS[i4_tIndSmpR][i4_tNext] - ppf4_SampleS[i4_tIndSmpR][i4_tIndAng];

				if (0.0f < f4_tPrevG && f4_tNextG < 0.0f)//Top
				{
					ppi4_Peek[i4_tIndSmpR][pi4_NumChain[i4_tIndSmpR]] = i4_tIndAng;

					cv::circle(inImage, 
						cv::Point(tCenterX + m_ppAngleX[pi4_IndBaseR[i4_tIndSmpR]][ppi4_Peek[i4_tIndSmpR][pi4_NumChain[i4_tIndSmpR]]], 
							tCenterY + m_ppAngleY[pi4_IndBaseR[i4_tIndSmpR]][ppi4_Peek[i4_tIndSmpR][pi4_NumChain[i4_tIndSmpR]]]), 
						10, cv::Scalar(255, 0, 0), -1, CV_AA);

					pi4_NumChain[i4_tIndSmpR]++;

					if (pi4_NumChain[i4_tIndSmpR] == Def_MaxChain)return 0;
				}
				else if (0.0f < f4_tPrevG && f4_tNextG == 0.0f)//Beg
				{
					int i4_tIndAng0 = i4_tIndAng + 1;
					int i4_tPeekLen = 0;
					int i4_tPrev0;
					int i4_tNext0;
					float f4_tPrevG0;
					float f4_tNextG0;

					while (ppf4_SampleS[i4_tIndSmpR][i4_tIndAng] == ppf4_SampleS[i4_tIndSmpR][i4_tIndAng0])
					{
						i4_tIndAng0++;
					}

					i4_tPrev0 = i4_tIndAng0 - 1;
					if (i4_tPrev0 < i4_tPrev0)i4_tPrev0 += m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]];
					f4_tPrevG0 = ppf4_SampleS[i4_tIndSmpR][i4_tIndAng0] - ppf4_SampleS[i4_tIndSmpR][i4_tPrev0];

					if (f4_tPrevG0 < 0.0f)
					{
						ppi4_Peek[i4_tIndSmpR][pi4_NumChain[i4_tIndSmpR]] = (i4_tIndAng + i4_tIndAng0) / 2;
						if (m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] <= ppi4_Peek[i4_tIndSmpR][pi4_NumChain[i4_tIndSmpR]])
							m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] -= m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]];

						//DrawPix(pu4_tRGB_Out,
						//	i4_tCenterX + ppi4_AngleX[pi4_IndBaseR[i4_tIndSmpR]][ppi4_Peek[i4_tIndSmpR][pi4_NumChain[i4_tIndSmpR]]],
						//	i4_tCenterY + ppi4_AngleY[pi4_IndBaseR[i4_tIndSmpR]][ppi4_Peek[i4_tIndSmpR][pi4_NumChain[i4_tIndSmpR]]],0x00,0x00,0x7f,2);

						pi4_NumChain[i4_tIndSmpR]++;

						if (pi4_NumChain[i4_tIndSmpR] == Def_MaxChain)return 0;
					}

					i4_tIndAng = i4_tIndAng0;
				}
				else if (0.0f == f4_tPrevG && f4_tNextG < 0.0f)//End
				{
				}
				else//None
				{
				}
			}
		}
	}

	//ピークの接続
	std::list<std::pair<int, std::list<int> > > List_tChain;
	{
		int i4_tChainRange = (int)(Def_ChainRange*i4_tSampleGap);
		int i4_tBaseRange = (int)(NumSmpR*0.25f);

		for (int i4_tIndP = 0; i4_tIndP < pi4_NumChain[i4_tBaseRange]; i4_tIndP++)
		{
			int i4_tIndPeek = ppi4_Peek[i4_tBaseRange][i4_tIndP];
			List_tChain.push_back(std::pair<int, std::list<int> >(i4_tBaseRange, std::list<int>(1, i4_tIndPeek)));

			int i4_tX = tCenterX + m_ppAngleX[pi4_IndBaseR[i4_tBaseRange]][i4_tIndPeek];
			int i4_tY = tCenterY + m_ppAngleY[pi4_IndBaseR[i4_tBaseRange]][i4_tIndPeek];
			cv::circle(inImage, cv::Point(i4_tX, i4_tY), 5, cv::Scalar(255, 255, 0), -1, CV_AA);
		}

		for (int i4_tIndSmpR = i4_tBaseRange; 0 < i4_tIndSmpR; i4_tIndSmpR--)
		{
			int i4_tIndBaseR0 = pi4_IndBaseR[i4_tIndSmpR];
			int i4_tIndBaseR1 = pi4_IndBaseR[i4_tIndSmpR - 1];
			for (std::list<std::pair<int, std::list<int> > >::iterator ListI_tChain = List_tChain.begin(); ListI_tChain != List_tChain.end(); ListI_tChain++)
			{
				if (ListI_tChain->first == i4_tIndSmpR)
				{
					int i4_tX0 = tCenterX + m_ppAngleX[i4_tIndBaseR0][ListI_tChain->second.front()];
					int i4_tY0 = tCenterY + m_ppAngleY[i4_tIndBaseR0][ListI_tChain->second.front()];
					int i4_tMinLenSq = i4_tChainRange * i4_tChainRange;
					int i4_tMinInd = -1;

					for (int i4_tIndP = 0; i4_tIndP < pi4_NumChain[i4_tIndSmpR - 1]; i4_tIndP++)
					{
						int i4_tX1 = tCenterX + m_ppAngleX[i4_tIndBaseR1][ppi4_Peek[i4_tIndSmpR - 1][i4_tIndP]];
						int i4_tY1 = tCenterY + m_ppAngleY[i4_tIndBaseR1][ppi4_Peek[i4_tIndSmpR - 1][i4_tIndP]];

						int i4_tLenSq = (i4_tX1 - i4_tX0)*(i4_tX1 - i4_tX0) + (i4_tY1 - i4_tY0)*(i4_tY1 - i4_tY0);
						if (i4_tLenSq < i4_tMinLenSq)
						{
							i4_tMinLenSq = i4_tLenSq;
							i4_tMinInd = ppi4_Peek[i4_tIndSmpR - 1][i4_tIndP];
						}
					}

					if (i4_tMinInd != -1)
					{
						ListI_tChain->second.push_front(i4_tMinInd);
						ListI_tChain->first = i4_tIndSmpR - 1;
					}
				}
			}
		}

		for (int i4_tIndSmpR = i4_tBaseRange; i4_tIndSmpR < NumSmpR - 1; i4_tIndSmpR++)
		{
			int i4_tIndBaseR0 = pi4_IndBaseR[i4_tIndSmpR];
			int i4_tIndBaseR1 = pi4_IndBaseR[i4_tIndSmpR + 1];
			for (std::list<std::pair<int, std::list<int> > >::iterator ListI_tChain = List_tChain.begin(); ListI_tChain != List_tChain.end(); ListI_tChain++)
			{
				if (ListI_tChain->first + ListI_tChain->second.size() == i4_tIndSmpR + 1)
				{
					int i4_tX0 = tCenterX + m_ppAngleX[i4_tIndBaseR0][ListI_tChain->second.back()];
					int i4_tY0 = tCenterY + m_ppAngleY[i4_tIndBaseR0][ListI_tChain->second.back()];
					int i4_tMinLenSq = i4_tChainRange * i4_tChainRange;
					int i4_tMinInd = -1;

					for (int i4_tIndP = 0; i4_tIndP < pi4_NumChain[i4_tIndSmpR + 1]; i4_tIndP++)
					{
						int i4_tX1 = tCenterX + m_ppAngleX[i4_tIndBaseR1][ppi4_Peek[i4_tIndSmpR + 1][i4_tIndP]];
						int i4_tY1 = tCenterY + m_ppAngleY[i4_tIndBaseR1][ppi4_Peek[i4_tIndSmpR + 1][i4_tIndP]];

						int i4_tLenSq = (i4_tX1 - i4_tX0)*(i4_tX1 - i4_tX0) + (i4_tY1 - i4_tY0)*(i4_tY1 - i4_tY0);
						if (i4_tLenSq < i4_tMinLenSq)
						{
							i4_tMinLenSq = i4_tLenSq;
							i4_tMinInd = ppi4_Peek[i4_tIndSmpR + 1][i4_tIndP];
						}
					}

					if (i4_tMinInd != -1)
					{
						ListI_tChain->second.push_back(i4_tMinInd);
					}
				}
			}
		}

		int i4_tIndChain = 0;

		for (std::list<std::pair<int, std::list<int> > >::iterator ListI_tChain = List_tChain.begin(); ListI_tChain != List_tChain.end();)
		{
			if (ListI_tChain->second.size() < (int)(0.5f*(Def_NumSmpR / 2 - 1)) || 1 < ListI_tChain->first)
			{
				ListI_tChain = List_tChain.erase(ListI_tChain);
				continue;
			}
			else
			{
				XMFLOAT2 V2_tPos = XMFLOAT2((float)(tCenterX + m_ppAngleX[pi4_IndBaseR[ListI_tChain->first]][ListI_tChain->second.front()]),
					(float)(tCenterY + m_ppAngleY[pi4_IndBaseR[ListI_tChain->first]][ListI_tChain->second.front()]));
				XMFLOAT2 pV2_tPos[Def_NumSmpR];

				std::list<int>::iterator ListI_tChain0 = ListI_tChain->second.begin();
				for (int i4_tIndL = 0; i4_tIndL < ListI_tChain->second.size(); i4_tIndL++, ListI_tChain0++)
				{
					pV2_tPos[i4_tIndL] = XMFLOAT2(
						(float)(tCenterX + m_ppAngleX[pi4_IndBaseR[ListI_tChain->first + i4_tIndL]][*ListI_tChain0]),
						(float)(tCenterY + m_ppAngleY[pi4_IndBaseR[ListI_tChain->first + i4_tIndL]][*ListI_tChain0]));
					XMVECTOR vecA = XMLoadFloat2(&pV2_tPos[i4_tIndL]);
					XMVECTOR vecB = XMLoadFloat2(&V2_tPos);
					vecA -= vecB;
					XMStoreFloat2(&pV2_tPos[i4_tIndL], vecA);
				}

				//基準点を最小二乗法で求める
				XMVECTOR vec_tPos = XMLoadFloat2(pV2_tPos);
				XMVECTOR vec_ChainRoot = XMLoadFloat2(&pV2_tChainRoot[i4_tIndChain]);
				XMVECTOR vec_ChainVec = XMLoadFloat2(&pV2_tChainVec[i4_tIndChain]);
				LeastSquares(&vec_tPos, ListI_tChain->second.size(), &vec_ChainRoot, &vec_ChainVec);
				vec_ChainRoot += vec_tPos;
				vec_ChainVec += vec_tPos;

				//基準円とチェインの交点
				Line2Circle(tCenterX, tCenterY, (float)Line2CircleR, &vec_ChainRoot, vec_ChainVec);
				XMFLOAT2 tempChain, tempRoot;
				XMStoreFloat2(&tempChain, vec_ChainRoot);
				XMStoreFloat2(&tempRoot, vec_ChainVec);
				//DrawLine(pu4_tRGB_Out, (I4)pV2_tChainRoot[i4_tIndChain].x, (I4)pV2_tChainRoot[i4_tIndChain].y, (I4)pV2_tChainVec[i4_tIndChain].x, (I4)pV2_tChainVec[i4_tIndChain].y, 0x00, 0x00, 0xff);
				cv::circle(inImage, cv::Point((int)tempChain.x, (int)tempChain.y), 5, cv::Scalar(255, 255, 255), -1, CV_AA);
				cv::circle(inImage, cv::Point((int)tempRoot.x, (int)tempRoot.y), 5, cv::Scalar(255, 255, 255), -1, CV_AA);

				vec_ChainVec -= vec_ChainRoot;//rootからvecへのベクトル（場所は関係なくなる）
				XMFLOAT2 tempLen;
				XMVECTOR Vec = XMVector2Length(vec_ChainVec);
				XMStoreFloat2(&tempLen, Vec);
				pV2_tChainRoot[i4_tIndChain] = tempLen;//rootからvecまでの長さ
				vec_ChainVec /= Vec;
				XMStoreFloat2(&pV2_tChainVec[i4_tIndChain], vec_ChainVec);//場所は関係なくなるがlenの方向をおぼえるため（これにレングスをのばすと青い線の方向ができる）

//takamori指先探索変更
				//不要な処理
				//F4 f4_tMaxLikelihood;
				//I4 i4_tMaxInd;
				//GetEdge(GetEdge_Inc, pf4_tLikelihood, pf4_tH_In, pf4_tS_In, pf4_tV_In, 0, i4_tSampleGap, &(pV2_tChainRoot[i4_tIndChain] + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain]), &pV2_tChainVec[i4_tIndChain], &f4_tMaxLikelihood, &i4_tMaxInd);
				//pf4_tChainLen[i4_tIndChain] += (F4)i4_tMaxInd;

				float f4_sample;
				for (int i4 = 0; i4 < i4_tSampleGap * 3; i4++) {
					f4_sample = pf4_tLikelihood[(int)(pV2_tChainRoot[i4_tIndChain].y + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].y)*size 
						+ (int)(pV2_tChainRoot[i4_tIndChain].x + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].x)] 
						+ pf4_tV_In[(int)(pV2_tChainRoot[i4_tIndChain].y + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].y)*size 
						+ (int)(pV2_tChainRoot[i4_tIndChain].x + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].x)];
					if (Def_ActiveLim < f4_sample) {
						cv::circle(inImage, cv::Point((int)pV2_tChainRoot[i4_tIndChain].x + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].x,
							(int)pV2_tChainRoot[i4_tIndChain].y + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].y),
							5, cv::Scalar(255, 255, 255), -1, CV_AA);
					}
					else {
						break;
					}
					pf4_tChainLen[i4_tIndChain] += 1.0;
				}

				//指先の描画色変更
				cv::circle(inImage, cv::Point((int)pV2_tChainRoot[i4_tIndChain].x + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].x,
					(int)pV2_tChainRoot[i4_tIndChain].y + pf4_tChainLen[i4_tIndChain] * pV2_tChainVec[i4_tIndChain].y),
					5, cv::Scalar(255, 0, 255), -1, CV_AA);

				ListI_tChain++;
				i4_tIndChain++;
			}
		}
	}

	return 0;
}

void CViewDirect2D::LeastSquares(XMVECTOR * pV2_tPos, int lenC, XMVECTOR * pV2_tRoot, XMVECTOR * pV2_tTop)
{
	float pf4_tX1[Def_NumSmpR];
	float pf4_tY1[Def_NumSmpR];
	XMVECTOR V2_ProjX = pV2_tPos[lenC - 1] - pV2_tPos[0];
	V2_ProjX = XMVector2Normalize(V2_ProjX);
	XMFLOAT2 data_ProjX, data_ProjY;
	XMStoreFloat2(&data_ProjX, V2_ProjX);
	data_ProjY = XMFLOAT2(data_ProjX.y, -data_ProjX.x);
	XMVECTOR V2_ProjY = XMLoadFloat2(&data_ProjY);


	XMFLOAT2 tempX, tempY;
	XMVECTOR VecIndC;
	for (int i4_IndC = 0; i4_IndC < lenC; i4_IndC++)
	{
		VecIndC = XMLoadFloat2(&XMFLOAT2(i4_IndC, i4_IndC));
		XMStoreFloat2(&tempX, XMVector2Dot(*pV2_tPos + VecIndC, V2_ProjX));
		pf4_tX1[i4_IndC] = tempX.x;
		XMStoreFloat2(&tempY, XMVector2Dot(*pV2_tPos + VecIndC, V2_ProjY));
		pf4_tY1[i4_IndC] = tempY.y;
	}


	float a = 0, b = 0;
	float sum_xy = 0, sum_x = 0, sum_y = 0, sum_x2 = 0;
	for (int i = 0; i < lenC; i++)
	{
		sum_xy += pf4_tX1[i] * pf4_tY1[i];
		sum_x += pf4_tX1[i];
		sum_y += pf4_tY1[i];
		sum_x2 += pow(pf4_tX1[i], 2);
	}
	a = (lenC * sum_xy - sum_x * sum_y) / (lenC * sum_x2 - pow(sum_x, 2));
	b = (sum_x2 * sum_y - sum_xy * sum_x) / (lenC * sum_x2 - pow(sum_x, 2));

	float f4_tXR1 = 0;
	float f4_tYR1 = a * f4_tXR1 + b;
	float f4_tXT1 = pf4_tX1[lenC - 1];
	float f4_tYT1 = a * f4_tXT1 + b;

	*pV2_tRoot = f4_tXR1 * V2_ProjX + f4_tYR1 * V2_ProjY;
	*pV2_tTop = f4_tXT1 * V2_ProjX + f4_tYT1 * V2_ProjY;
}

void CViewDirect2D::Line2Circle(float tCX, float tCY, float tR, XMVECTOR * pV2_t0, XMVECTOR V2_t1)
{
	XMVECTOR V2_tAC = XMLoadFloat2(&XMFLOAT2(tCX, tCY)) - V2_t1;
	XMVECTOR V2_tAB = *pV2_t0 - V2_t1;
	XMVECTOR V2_t = XMVector2Dot(V2_tAB, V2_tAC) / XMVector2LengthSq(V2_tAB);
	XMVECTOR V2_tQ = V2_t * V2_tAB + V2_t1;

	XMVECTOR V2_data = XMVector2Length(XMLoadFloat2(&XMFLOAT2(tCX, tCY)) - V2_tQ);
	XMFLOAT2 data;
	XMStoreFloat2(&data, V2_data);
	float f4_d = data.x;
	if (f4_d < tR) {
		XMVECTOR V2_tNAB = XMVector2Normalize(V2_tAB);
		float f4_s = sqrtf(tR*tR - f4_d * f4_d);
		*pV2_t0 = V2_tQ - f4_s*V2_tNAB;
	}
	else {
		*pV2_t0 = V2_tQ;
	}

}

int CViewDirect2D::FindGroup2to5(int tChainSize, XMFLOAT2 * pV2_tChainRoot, XMFLOAT2 * pV2_tChainVec, float * tChainLen, S_HANDINF * pHandInf_t)
{
	return 0;
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

	// キャプチャモードに移行しているのであれば...
	if (flagCapture) {
		MyOutputDebugString(L"・flagCapture = true\n");
		if (writer.isOpened()) {
			MyOutputDebugString(L"・writer.isOpened() = true\n");
			writer << image;
		}
	}

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
	
	/* ②手指領域のみを抽出する */
	handExtractor(renderImage01, renderImage02);
	/* ③掌の中心位置を推定する */
	CalcHandCentroid(renderImage02, renderImage03);
	MyOutputDebugString(L"m_handInfo.hand2DPosX:%d\n", m_handInfo.hand2DPosX);
	MyOutputDebugString(L"m_handInfo.hand2DPosY:%d\n", m_handInfo.hand2DPosY);
	MyOutputDebugString(L"m_handInfo.hand3DPosX:%f\n", m_handInfo.hand3DPosX);
	MyOutputDebugString(L"m_handInfo.hand3DPosY:%f\n", m_handInfo.hand3DPosY);
	MyOutputDebugString(L"m_handInfo.hand3DPosZ:%f\n", m_handInfo.hand3DPosZ);
	/* ④手指の状態を解析する */
	//AnalyzeHandInf
	/* ⑤インプットモードのモデルを作成 */

	/* 画像データを確保済みのメモリ上へ書き込み */
	copyImageToMemory(renderImage01, this->memory, 1);	// ①カメラからの入力画像をメモリ上に配置
	copyImageToMemory(renderImage02, this->memory, 2);	// ②手指領域の抽出画像をメモリ上に配置
	copyImageToMemory(renderImage03, this->memory, 3);	// ③掌の中心位置の推定画像をメモリ上に配置
	copyImageToMemory(renderImage01, this->memory, 4);	// ④解析情報の取得画像をメモリ上に配置
	copyImageToMemory(renderImage01, this->memory, 5);	// ⑤インプットモードの判別画像をメモリ上に配置

	// ターゲットサイズの取得
	D2D1_SIZE_F oTargetSize = m_pRenderTarget->GetSize();

	// 描画開始
	PAINTSTRUCT tPaintStruct;
	::BeginPaint(this->m_hWndViewTarget, &tPaintStruct);

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
	::EndPaint(this->m_hWndViewTarget, &tPaintStruct);

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
