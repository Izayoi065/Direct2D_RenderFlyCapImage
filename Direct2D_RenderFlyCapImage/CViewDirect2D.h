#pragma once
#include "CWinBase.h"
#include "HandAnalyzer.h"

/* �O���錾 */
class C_WinBase;
class HandAnalyzer;
class CApplication;
class FlyCap2CVWrapper;

class CViewDirect2D :
	public CWinBase
{
#define Def_NumPressure 16
#define Def_NumSmpR 8//�T���v�����O�̂��߂̓��S�~�e�[�u���̌�
	/* ���̓��[�h */
	enum Hand_InputMode
	{
		Hand_IM2D,
		hand_IM3D,
		Hand_IMChar,
		Hand_IMEnd,
	};
	/* �F�����Ă���肪�E��or����̊Ǘ� */
	enum Hand_LeftRight
	{
		Hand_Left,//����
		Hand_Right,//�E��
		Hand_LREnd,
	};
	/* �w�̊֐߃C���f�b�N�X */
	enum Hand_Joint
	{
		Hand_JointRoot,//�t����
		Hand_Joint1,//���֐�
		Hand_Joint2,//���֐�
		Hand_JointTop,//�w��
		Hand_JointEnd,
	};
	/* �w�̃G�b�W�擾���̃C���f�b�N�X */
	enum En_GetEdge
	{
		GetEdge_Inc,
		GetEdge_Dec,
		GetEdge_Abs,
		GetEdge_End,
	};
	/* �w���̍\���� */
	typedef struct S_FingerInf {
		float pf4_PosX[Hand_JointEnd];	//�e�֐߂�3D�ʒu:x���W
		float pf4_PosY[Hand_JointEnd];	//�e�֐߂�3D�ʒu:y���W
		float pf4_PosZ[Hand_JointEnd];	//�e�֐߂�3D�ʒu:z���W

		bool b4_Touch;
		float f4_ForceX;	// �w�ɒ������邹��f��
		float f4_ForceY;	// �w�ɕ��s�Ȃ���f��
		float f4_ForceZ;	// �����R��
		float f4_Rotate;	// �w�̉�]�p�x(���v���)

		float f4_Length;
	}S_FINGERINF;
	/* ����̍\���� */
	typedef struct S_HandInf {
		int hand2DPosX;		// ���d�S��2D�ʒu�Fx���W
		int hand2DPosY;		// ���d�S��2D�ʒu�Fy���W
		float hand3DPosX;	// ���d�S��3D�ʒu�Fx���W
		float hand3DPosY;	// ���d�S��3D�ʒu�Fy���W
		float hand3DPosZ;	// ���d�S��3D�ʒu�Fz���W
		float handRotate;	// ��]�p�x(���v���)
		float handRadius;	// ���̔��a
		Hand_LeftRight e_LeftRight;	//�E�肩���肩
		Hand_InputMode e_InputMode;	//���̓��[�h
		S_FingerInf pFI[5];	//�w���
		bool FlagL;	// �F�����Ă���肪�E�肩���肩�̃t���O
	}S_HANDINF;
private:
	double totalTime = 0;		// �`�悳�ꂽ�t���[�����[�g�̍X�V�^�C�~���O��1�b���ɌŒ肷�邽�߂̊Ǘ��p
	BOOL g_InitD2D = false;		// 2�d�������h�~
	/* Direct2D�֘A */
	ID2D1Factory * m_pD2d1Factory;				// 
	ID2D1HwndRenderTarget * m_pRenderTarget;	// �����_�����O�̃^�[�Q�b�g
	IDWriteTextFormat* pTextFormat;				// �e�L�X�g�̏����ݒ�Ɏg�p����t�H���g �v���p�e�C�i���v���p�e�B�y�у��P�[�����
	IDWriteFactory* pDWFactory;					// �S�Ă�DirectWrite�I�u�W�F�N�g�̃��[�g�E�t�@�N�g���E�C���^�[�t�F�[�X
	ID2D1SolidColorBrush* pBrush;				// 
	ID2D1Bitmap * pBitmap;						// 
	/* Direct3D�֘A */
	ID3D11Device*					m_pDevice;				// �f�o�C�X
	ID3D11DeviceContext*			m_pImmediateContext;	// �f�o�C�X�E�R���e�L�X�g
	IDXGISwapChain*					m_pSwapChain;			// �X���b�v�E�`�F�C��
	ID3D11RenderTargetView*			m_pRenderTargetView;	// �`��^�[�Q�b�g�E�r���[
	ID3D11Texture2D*				m_pDepthStencilTexture;	// �[�x/�X�e���V��
	ID3D11DepthStencilView*			m_pDepthStencilView;	// �[�x/�X�e���V���E�r���[
	D3D11_VIEWPORT					m_Viewport;				// �r���[�|�[�g
	ID3D11Buffer*					m_pVertexBuffer;		// 
	ID3D11InputLayout*				m_pInputLayout;			// 
	ID3D11VertexShader*				m_pVertexShader;		// Vertex�V�F�[�_�[
	ID3D11PixelShader*				m_pPixelShader;			// pixel�V�F�[�_�[
	ID3D11SamplerState*				SampleLinear;			// �e�N�X�`���[�̃T���v���[

	static XMFLOAT3 m_pV3PixToVec[size*size];
	std::wstring strText = L"";					// �E�B���h�E�ɕ\������fps�p�e�L�X�g
	byte *memory;	// cv::Mat -> ID2D1Bitmap�p�̃o�b�t�@
	Hand_LeftRight HLR[2] = { Hand_LREnd,Hand_LREnd };//��̍��E���f�̕������p
	Hand_InputMode HIM[4] = { Hand_IMEnd,Hand_IMEnd,Hand_IMEnd,Hand_IMEnd };	// ���̓��[�h����p
public:
	FlyCap2CVWrapper* FlyCap;
	int NumRadius = 0;
	int handMinPalmR;	//�摜��ł̏��̍ŏ����a
	int handMaxPalmR;	//�摜��ł̏��̍ő唼�a
	float* ParamWeight;	//�~�`�e�[�u���̊e���a�ɑ΂�����d���ς̃E�F�C�g�����S�t�߂͏��̂��ߖޓx�̗D��x�������Ȃ違���ӂł͎w�̊ԂƂ������邩��ޓx�̗D��x���Ⴍ�Ȃ�
	int *m_pNumAngle;	// �~�`�e�[�u���̊e���a�ɑ΂���~���̒����i�T���v�����O�_�̐��j
	int **m_ppAngleX;	// �T���v�����O�_��X�ʒu
	int **m_ppAngleY;	// �T���v�����O�_��Y�ʒu
	int FingerWidth;	// �摜��ł̎w�̕�
	float *ppf4_Sample[Def_NumSmpR];//
	float *ppf4_SampleS[Def_NumSmpR];//
	int *ppi4_Peek[Def_NumSmpR];//
	XMFLOAT2 pV2_Radio[Def_NumPressure];
	cv::Mat renderImage01;		// �@�J��������̓��͉摜
	cv::Mat renderImage02;		// �A��w�̈�̒��o�摜
	cv::Mat renderImage03;		// �B���̒��S�ʒu�̐���摜
	cv::Mat renderImage04;		// �C��͏��̎擾�摜
	cv::Mat renderImage05;		// �D�C���v�b�g���[�h�̔��ʉ摜
	unsigned char ActiveCameraArea[size*size];	// �J�����ŎB�e���ꂽ�͈�
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
	void CalcHandCentroid(cv::InputArray inImage_, cv::OutputArray outRenderImage02_, cv::OutputArray outRenderImage03_);
	void CorrectionImageImageDistortion(unsigned char* tActiveArea, float* pf4_HandLikelihood_IN, float* p_tH_IN, float* p_tS_IN, float* p_tV_IN,
		S_HANDINF *pHandInf_t, float* p_tUV_OUT, float* pf4_HandLikelihood_OUT, float* p_tH_OUT, float* p_tS_OUT, float* p_tV_OUT, cv::InputArray inImage_, cv::OutputArray outImage_);
	int CalcHandCentroidRing(unsigned char* tActiveArea, float* pf4_HandLikelihood, cv::InputArray inImage_, cv::InputArray inUVImage_, S_HANDINF *pHandInf_t, cv::OutputArray outImage_);

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
	HRESULT	IdlingRender(double fps);
	//void DrawPix(cv::InputArray inImage_);

	/* ���z�֐� */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);	// �E�B���h�E�쐬���̃n���h��OnCreate
	virtual void OnDestroy();	// �E�B���h�E�j�����̃n���h�� Ondestroy
};

