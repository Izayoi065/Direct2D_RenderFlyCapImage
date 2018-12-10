#pragma once
#include "CWinBase.h"

/* 前方宣言 */
class C_WinBase;
class CApplication;
class FlyCap2CVWrapper;

class CViewDirect2D :
	public CWinBase
{
#define Def_NumPressure 16
#define Def_NumSmpR 8//サンプリングのための同心円テーブルの個数
	/* 入力モード */
	enum Hand_InputMode
	{
		Hand_IM2D,
		hand_IM3D,
		Hand_IMChar,
		Hand_IMEnd,
	};
	/* 認識している手が右手or左手の管理 */
	enum Hand_LeftRight
	{
		Hand_Left,//左手
		Hand_Right,//右手
		Hand_LREnd,
	};
	/* 指の関節インデックス */
	enum Hand_Joint
	{
		Hand_JointRoot,//付け根
		Hand_Joint1,//第二関節
		Hand_Joint2,//第一関節
		Hand_JointTop,//指先
		Hand_JointEnd,
	};
	/* 指のエッジ取得時のインデックス */
	enum En_GetEdge
	{
		GetEdge_Inc,
		GetEdge_Dec,
		GetEdge_Abs,
		GetEdge_End,
	};
	/* 指情報の構造体 */
	typedef struct S_FingerInf {
		float pf4_PosX[Hand_JointEnd];	//各関節の3D位置:x座標
		float pf4_PosY[Hand_JointEnd];	//各関節の3D位置:y座標
		float pf4_PosZ[Hand_JointEnd];	//各関節の3D位置:z座標

		bool b4_Touch;
		float f4_ForceX;	// 指に直交するせん断力
		float f4_ForceY;	// 指に平行なせん断力
		float f4_ForceZ;	// 垂直抗力
		float f4_Rotate;	// 指の回転角度(時計回り)

		float f4_Length;
	}S_FINGERINF;
	/* 手情報の構造体 */
	typedef struct S_HandInf {
		int hand2DPosX;		// 掌重心の2D位置：x座標
		int hand2DPosY;		// 掌重心の2D位置：y座標
		float hand3DPosX;	// 掌重心の3D位置：x座標
		float hand3DPosY;	// 掌重心の3D位置：y座標
		float hand3DPosZ;	// 掌重心の3D位置：z座標
		float handRotate;	// 回転角度(時計回り)
		float handRadius;	// 掌の半径
		Hand_LeftRight e_LeftRight;	//右手か左手か
		Hand_InputMode e_InputMode;	//入力モード
		S_FingerInf pFI[5];	//指情報
		bool FlagL;	// 認識している手が右手か左手かのフラグ
	}S_HANDINF;
private:
	static const unsigned size = 504U;	// カメラからの入力画像の1辺のサイズ
	double totalTime = 0;		// 描画されたフレームレートの更新タイミングを1秒毎に固定するための管理用
	BOOL g_InitD2D = false;		// 2重初期化防止
	ID2D1Factory * m_pD2d1Factory;				// 
	ID2D1HwndRenderTarget * m_pRenderTarget;	// レンダリングのターゲット
	IDWriteTextFormat* pTextFormat;				// テキストの書式設定に使用するフォント プロパテ，段落プロパティ及びロケール情報
	IDWriteFactory* pDWFactory;					// 全てのDirectWriteオブジェクトのルート・ファクトリ・インターフェース
	ID2D1SolidColorBrush* pBrush;				// 
	ID2D1Bitmap * pBitmap;						// 
	static XMFLOAT3 m_pV3PixToVec[size*size];
	std::wstring strText = L"";					// ウィンドウに表示するfps用テキスト
	byte *memory;	// cv::Mat -> ID2D1Bitmap用のバッファ
	Hand_LeftRight HLR[2] = { Hand_LREnd,Hand_LREnd };//手の左右判断の平滑化用
	Hand_InputMode HIM[4] = { Hand_IMEnd,Hand_IMEnd,Hand_IMEnd,Hand_IMEnd };	// 入力モード判定用
public:
	FlyCap2CVWrapper* FlyCap;
	int NumRadius = 0;
	int handMinPalmR;
	int handMaxPalmR;
	float* ParamWeight;
	int *m_pNumAngle;	// 円形テーブルの各半径に対する円周の長さ（サンプリング点の数）
	int **m_ppAngleX;	// サンプリング点のX位置
	int **m_ppAngleY;	// サンプリング点のY位置
	int FingerWidth;	// 画像上での指の幅
	float *ppf4_Sample[Def_NumSmpR];//
	float *ppf4_SampleS[Def_NumSmpR];//
	int *ppi4_Peek[Def_NumSmpR];//
	XMFLOAT2 pV2_Radio[Def_NumPressure];
	cv::Mat renderImage01;		// ①カメラからの入力画像
	cv::Mat renderImage02;		// ②手指領域の抽出画像
	cv::Mat renderImage03;		// ③掌の中心位置の推定画像
	cv::Mat renderImage04;		// ④解析情報の取得画像
	cv::Mat renderImage05;		// ⑤インプットモードの判別画像
	unsigned char ActiveCameraArea[size*size];	// カメラで撮影された範囲

	float ppf4_Hue[size*size];
	float ppf4_Saturation[size*size];
	float ppf4_Value[size*size];
	S_HANDINF m_handInfo;
	float FixX[size*size];
	float FixY[size*size];
public:
	CViewDirect2D(CApplication *pApp);
	~CViewDirect2D();
	HRESULT InitDirect2D(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd);
	void ReleaseD2D();
	void copyImageToMemory(cv::InputArray image_, byte* data, int num);
	void handExtractor(cv::InputArray inImage_, cv::OutputArray outImage_);
	void CalcHandCentroid(cv::InputArray inImage_, cv::OutputArray outImage_);
	//void CorrectionImageImageDistortion(unsigned char* tActiveArea, cv::InputArray inImage_, float* p_tH_IN, float* p_tS_IN, float* p_tV_IN, S_HANDINF *pHandInf_t, cv::OutputArray outImage_, float* p_tH_OUT, float* p_tS_OUT, float* p_tV_OUT);
	int CalcHandCentroidRing(unsigned char* tActiveArea, cv::InputArray inImage_, cv::InputArray inUVImage_, S_HANDINF *pHandInf_t, cv::OutputArray outImage_);

	void AnalyzeHandInf(cv::InputArray inImage_, cv::OutputArray outRenderImage04_, cv::OutputArray outRenderImage05_);
	int detectFinger2to5(int tCenterX, int tCenterY, cv::InputArray likelihoodArea, float *p_HueImage,
		float *p_SaturationImage, float *ValueImage, S_HANDINF *pHandInf_t, cv::InputArray inImage_, cv::OutputArray outImage_);
	int FindChain(int tCenterX, int tCenterY, int Line2CircleR, int MinR, int MaxR, int NumSmpR,
		float TergetAng, float SearchAng, float *pf4_tLikelihood, float *pf4_tH_In, float *pf4_tS_In, float *pf4_tV_In,
		cv::InputArray inImage_, cv::OutputArray outImage_, XMFLOAT2 *pV2_tChainRoot, XMFLOAT2 *pV2_tChainVec, float *pf4_tChainLen);
	void LeastSquares(XMVECTOR *pV2_tPos, int lenC, XMVECTOR *pV2_tRoot, XMVECTOR *pV2_tTop);
	void Line2Circle(float tCX, float tCY, float tR, XMVECTOR *pV2_t0, XMVECTOR V2_t1);
	int FindGroup2to5(int tChainSize, XMFLOAT2 *pV2_tChainRoot, XMFLOAT2 *pV2_tChainVec, float *tChainLen, S_HANDINF *pHandInf_t);
	void EstimateFinger1Root(int* tRootX0, int* tRootY0, float* tTargetAngle0, int* tRootX1, int* tRootY1,
		float* tTargetAngle1, S_HANDINF *pHandInf_t, cv::InputArray inImage_, cv::OutputArray outImage_);
	int detectFinger1(cv::InputArray likelihoodArea, float *p_HueImage,	float *p_SaturationImage, float *ValueImage, S_HANDINF *pHandInf_t,
		int tRootX0, int tRootY0, float tTargetAngle0, int tRootX1, int tRootY1, float tTargetAngle1, cv::InputArray inImage_, cv::OutputArray outImage_);
	float FindOne(float tTargetAngle, int tChainSize, XMFLOAT2 *pV2_ChainRoot, XMFLOAT2 *pV2_tChainVec,
		float *tChainLen, XMFLOAT2 *pV2_tFingerRoot, XMFLOAT2 *pV2_tFingerVec, float *tFingerLen);
	void Hand_LeftRight_Smoothing(S_HANDINF *pHandInf_t);
	void detect_InputMode(cv::InputArray likelihoodArea, float *p_HueImage, float *p_SaturationImage, float *ValueImage,
		int tCenterX, int tCenterY, S_HANDINF *pHandInf_t, cv::InputArray inImage_, cv::OutputArray outImage_);
	void detectFingerDistance(cv::InputArray likelihoodArea, float *p_HueImage, float *p_SaturationImage, float *ValueImage,
		S_HANDINF *pHandInf_t, cv::InputArray inImage_, cv::OutputArray outImage_);
	void GetEdge(En_GetEdge e_tGetEdge, float *likelihoodArea, float *p_HueImage, float *p_SaturationImage, float *ValueImage,
		int tMin, int tMax, XMFLOAT2 *pF2_tPos, XMFLOAT2 *pF2_tVec, float *p_tMaxLikelihood, int *p_tMaxIndex);

	HRESULT AppIdle(cv::InputArray image_, double fps);
	HRESULT	Render(cv::InputArray image_, double fps);
	//void DrawPix(cv::InputArray inImage_);

	/* 仮想関数 */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);	// ウィンドウ作成時のハンドラOnCreate
	virtual void OnDestroy();	// ウィンドウ破棄時のハンドラ Ondestroy
};

