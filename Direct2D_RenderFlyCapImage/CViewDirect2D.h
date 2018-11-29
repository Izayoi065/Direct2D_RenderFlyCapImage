#pragma once
#include "CWinBase.h"

/* 前方宣言 */
class C_WinBase;
class CApplication;
class FlyCap2CVWrapper;

class CViewDirect2D :
	public CWinBase
{
	/* 入力モード */
	enum Hand_InputMode
	{
		Hand_2D,
		hand_3D,
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
public:
	FlyCap2CVWrapper* FlyCap;
	int *m_pNumAngle;	// 円形テーブルの各半径に対する円周の長さ（サンプリング点の数）
	int **m_ppAngleX;	// サンプリング点のX位置
	int **m_ppAngleY;	// サンプリング点のY位置
	cv::Mat renderImage01;		// �@カメラからの入力画像
	cv::Mat renderImage02;		// �A手指領域の抽出画像
	cv::Mat renderImage03;		// �B掌の中心位置の推定画像
	cv::Mat renderImage04;		// �C解析情報の取得画像
	cv::Mat renderImage05;		// �Dインプットモードの判別画像
	unsigned char ActiveCameraArea[size*size];	// カメラで撮影された範囲
	S_HANDINF m_handInfo;
public:
	CViewDirect2D(CApplication *pApp);
	~CViewDirect2D();
	HRESULT InitDirect2D(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd);
	void ReleaseD2D();
	void copyImageToMemory(cv::InputArray image_, byte* data, int num);
	void handExtractor(cv::InputArray inImage_, cv::OutputArray outImage_);
	void CalcHandCentroid(cv::InputArray inImage_, cv::OutputArray outImage_);
	HRESULT AppIdle(cv::InputArray image_, double fps);
	HRESULT	Render(cv::InputArray image_, double fps);
	//void DrawPix(cv::InputArray inImage_);

	/* 仮想関数 */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);	// ウィンドウ作成時のハンドラOnCreate
	virtual void OnDestroy();	// ウィンドウ破棄時のハンドラ Ondestroy
};

