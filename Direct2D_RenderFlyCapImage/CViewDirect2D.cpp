/****************************************************************************************************
 *�^�C�g���FCViewDirect2D.cpp
 *�����@�@�F
 *		�F
 *�O��LIB �F
 *
 *���쌠�@�FTomoki Kondo
 *
 *�ύX�����F2018.11.20 Tuesday
 *�@�@�@�F�V�K�o�^
 *
 ****************************************************************************************************/
/* ��`�֐� */
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define Def_FingerW 0.015f//�w�̕�
#define Def_ActiveLim 0.9f//�ޓx�̈悪���̒l�ȓ��ł���Ύ�w�̈�Ɣ��肷��
#define Def_ChainRange 1.5f//�ޓx�̃s�[�N�̍ő�ڑ�����
#define Def_MaxChain 20//�w���̍ő�ێ���
#define Def_MAXActiveRate 0.9f	// ���͉摜�̎B�e���ꂽ�͈̗͂L�����a
#define Hand_DomeR 0.1f			//�A�N�����h�[���̔��a

/* �C���N���[�h�t�@�C�� */
#include "CViewDirect2D.h"	// DirectX2D�֘A�̏������N���X
#include "resource.h"		// ���\�[�X�t�@�C��

/* static�����o�ϐ��̒�` */
XMFLOAT3 CViewDirect2D::m_pV3PixToVec[CViewDirect2D::size*CViewDirect2D::size];	// static�����o�ϐ�CWinBase:: m_pV3PixToVec�͐錾�ƕʂɂ����ɒ�`���Ȃ��Ƃ����Ȃ�.
float *ppi4_Peek[Def_NumSmpR];//
/** @brief CViewDirect2D�N���X�̃R���X�g���N�^
@note ���̊֐��́C���̃N���X���Ăяo���ꂽ�ۂɁC�ŏ��Ɏ��s�����
@param pApp	CApplication�N���X�̃I�u�W�F�N�g
@sa CApplication CWinBase
**/
CViewDirect2D::CViewDirect2D(CApplication * pApp) : CWinBase(pApp)
{
	MyOutputDebugString(L"	CViewDirect2D(CApplication*):CWinBase(CApplication)���Ăяo����܂����I\n");
	m_pD2d1Factory = NULL;
	m_pRenderTarget = NULL;
	pTextFormat = NULL;
	pDWFactory = NULL;
	pBrush = NULL;
	pBitmap = NULL;	
	memory = new byte[size * size * 4 * 5];	// �v�f���F��*����(pixel)*4(byte/pixel)��5�u���b�N
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

	//�X�g���b�`�e�[�u���̍쐬
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
				float f4_tR0 = sqrtf(f4_tX0*f4_tX0 + f4_tY0 * f4_tY0);//�␳�O���a
				float f4_tA = atanf(f4_tR0 / f4_tLikelihood);//���a���p�x
				float f4_tR1 = f4_tA / (Def_FOV / 2.0f);//�␳�㔼�a

				float f4_tX1 = f4_tX0 * f4_tR0 / f4_tR1;
				float f4_tY1 = f4_tY0 * f4_tR0 / f4_tR1;

				FixX[i4_tIndY*size + i4_tIndX] = FloatToIndex(f4_tX1, f4_tBase, f4_tPitch);
				FixY[i4_tIndY*size + i4_tIndX] = FloatToIndex(f4_tY1, f4_tBase, f4_tPitch);
			}
		}
	}
}

/** @brief CViewDirect2D�N���X�̃f�X�g���N�^
@note ���̊֐��́C���̃N���X���j�������ۂɁC�Ō�Ɏ��s�����
@sa ReleaseD2D
**/
CViewDirect2D::~CViewDirect2D()
{
	MyOutputDebugString(L"	~ViewDirectX11()���Ăяo����܂����I\n");
	delete[] memory;
	free(m_ppAngleY);
	free(m_ppAngleX);
	free(m_pNumAngle);
	ReleaseD2D();
}

/** @brief Direct2D�֘A�̏��������s��
@note ���̊֐��́C���̃N���X�Ŏg�p�����Direct2D�̃f�o�C�X�E�C���^�[�t�F�[�X��������������D
��{�I�ɂ́C���̃N���X�̃I�u�W�F�N�g�𐶐���������Ɏg�p����D
@param hInstance	
@param lpCmdLine	
@param nShowCmd		
@return HRESULT�G���[�R�[�h��Ԃ�
@sa D2D1CreateFactory CreateHwndRenderTarget CreateBitmap DWriteCreateFactory CreateSolidColorBrush
**/
HRESULT CViewDirect2D::InitDirect2D(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	CRect rect;
	::GetClientRect(this->m_hWndViewTarget, &rect);	// �N���C�A���g�̈�̃T�C�Y���擾

	HRESULT hResult = S_OK;

	/*
	ID2D1Factory�̐���
	*/
	hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pD2d1Factory);
	if (FAILED(hResult)) {
		// �G���[
		MyOutputDebugString(L"	ID2D1Factory�̐����Ɏ��s���܂����D");
	}

	/*
	ID2D1HwndRenderTarget�̐���
	*/
	D2D1_SIZE_U PixelSize = { rect.Width(), rect.Height() };
	D2D1_RENDER_TARGET_PROPERTIES RenderTargetProperties = D2D1::RenderTargetProperties();
	D2D1_HWND_RENDER_TARGET_PROPERTIES HwndRenderTargetProperties = D2D1::HwndRenderTargetProperties(this->m_hWndViewTarget, PixelSize);
	HwndRenderTargetProperties.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY;	// �����������Ȃ�	http://www.wisdomsoft.jp/444.html
	hResult = m_pD2d1Factory->CreateHwndRenderTarget(
		RenderTargetProperties,
		HwndRenderTargetProperties,
		&m_pRenderTarget);
	if (FAILED(hResult)) {
		// �G���[
		MyOutputDebugString(L"	ID2D1HwndRenderTarget�̐����Ɏ��s���܂����D");
	}

	/* ID2D1Bitmap�̐��� */
	m_pRenderTarget->CreateBitmap(
		D2D1::SizeU(size*5, size),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&pBitmap);
	
	/* IDWriteFactory�̐��� */
	hResult = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWFactory));
	if (FAILED(hResult)) {
		MyOutputDebugString(L"	DWriteCreateFactory�͎��s���܂����I\n");
	}

	/* �u���V�̐��� */
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White)
		, &pBrush
	);

	g_InitD2D = true;

	return hResult;
}

/** @brief ���̃N���X�Ŏg�p����Direct2D�֘A�̃f�o�C�X�E�C���^�[�t�F�[�X�����S�ɔj������
@note ���̊֐��́C���̃N���X�Ŏg�p����Direct2D�֘A�̃f�o�C�X�E�C���^�[�t�F�[�X�����S�ɔj������
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

/** @brief �E�B���h�E�̔C�ӂ̈ʒu�ɉ摜�f�[�^����������ɔz�u����D
@note ���̊֐��́CID2DBitmap::CopyFromMemory���g�p����O�����Ƃ��āC�����_�����O�Ώۂ̉摜����������ɔz�u����D
���̋@�\�ł́CDXGI_FORMAT_B8G8R8A8_UNORM�ō쐬���ꂽID2DBitmap�ɑΉ������f�[�^�`���Ń�������ɔz�u���邽�߂Ɏg�p�����D
�܂��C����image_��OpenCV��cv::Mat�^�摜�ł���C�f�[�^�`����CV_8UC3�Ƃ���(8bit3�`�����l��)�D
@param image_	��������ɔz�u����摜
@param data		�摜�f�[�^�̔z�u��ƂȂ郁�����̃|�C���^
@param num		�z�u����ꏊ(num=1,2,3,4,5)
@sa	ID2DBitmap::CopyFromMemory InputArray 
**/
void CViewDirect2D::copyImageToMemory(cv::InputArray image_, byte* data, int num)
{
	cv::Mat image = image_.getMat();
#pragma omp parallel for	//  cv::Mat�^�摜�f�[�^��byte�^�z��̎w��ʒu�ɑΉ�����ꏊ�Ɋi�[���Ă���
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

/** @brief ��w�̈�̒��o���������s����D
@note ���̊֐��́C���͉摜�����w�̈�𒊏o������K�p����D�o�͂ł́C���o���ꂽ��w�̈�̓�l���摜���o�͂����D
@param image_		 �J��������̓��͉摜
@param outImage_	���o���ꂽ��w�̈�̓�l���摜
@sa	Render
**/
void CViewDirect2D::handExtractor(cv::InputArray inImage_, cv::OutputArray outImage_)
{
	cv::Mat inImage = inImage_.getMat();
	cv::Mat hsvImage, hsv_mask, dstImage;
	cv::cvtColor(inImage, hsvImage, CV_BGR2HSV);
	cv::inRange(hsvImage, hsv_min, hsv_max, hsv_mask);
	cv::cvtColor(hsv_mask, dstImage, CV_GRAY2BGR);
#pragma omp parallel for	 // HSV�̊e�`�����l����float�^(0�`1)�Ŏ擾
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

/** @brief ��w�̈�̏d�S�𐄒肷��
@note ���̊֐��́C��w�̈��2�l���摜(8UC3)����d�S���Z�o���C�h�[�����3D�ʒu�𐄒肷��D
���̋@�\�ł́CinImage_�Ŏ擾������w�̈�̓�l���摜�̏d�S���Z�o���C���჌���Y��A�N�����h�[���ɂ��c�݂�␳����D
�܂��C��̏d�S�Z�o�ł͔��F���o�̐��x���ɂ�萳�m�Ȓl�łȂ��\�������邽�߁C��`�I�A���S���Y���ɂ�萳�m�Ȓl�����߁C�Ăјc�ݕ␳���s���D
@param inImage_		��w�̈��2�l���摜(8UC3)
@param outImage_	��w�̈�̏d�S������s�����摜
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

	/* �摜�̂䂪�ݕ␳ */
	/* ��w�̈�̏d�S�ʒu�̍Čv�Z */
	/* �摜�̂䂪�ݕ␳ */

	cv::circle(dstImage, cv::Point(m_handInfo.hand2DPosX, m_handInfo.hand2DPosY), 5, cv::Scalar(0, 255, 0), -1, CV_AA);

	dstImage.copyTo(outImage_);
}

/** @brief ��w���̉�͂��s��
@note ���̊֐��́C�擾�����摜�����w���̉�͂��s���C�e�w�̊֐߂�3D�ʒu����̓��[�h�̔��蓙���s���D
@param inImage_		��w�̈��2�l���摜(8UC3)
@param outImage_	��w�̈�̏d�S������s�����摜
@sa cv::moments m_handInfo
**/
void CViewDirect2D::AnalyzeHandInf(cv::InputArray inImage_, cv::OutputArray outImage_)
{
	cv::Mat inImage = inImage_.getMat();
	cv::Mat dstImage;

	/* ��2�w�`��5�w�̌��o */
	detectFinger2to5(size / 2, size / 2, renderImage02, ppf4_Hue, ppf4_Saturation, ppf4_Value, &m_handInfo, inImage, dstImage);
	/* ��1�w�t�����̐��� */
	/* ��1�w�̌��o */
	/* ���̓��[�h�̔��� */
	/* ��w�̋������o */
	/* �e�|�C���g�ł̈��͌��o */


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
#pragma omp parallel for	 // HSV�̊e�`�����l����float�^(0�`1)�Ŏ擾
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
	int pi4_IndBaseR[Def_NumSmpR];//���̐F���z�e�[�u��
	if (size / 2 < MaxR)MaxR = size / 2;
	int i4_tSampleGap = (MaxR - MinR) / NumSmpR;//���S�~�ő唼�a����ŏ����a�̂��ꂼ��̊Ԋu


	//�e�[�u���̍쐬
	for (int i4_tIndSmpR = 0; i4_tIndSmpR < NumSmpR; i4_tIndSmpR++)
	{
		pi4_IndBaseR[i4_tIndSmpR] = i4_tIndSmpR * i4_tSampleGap + MinR;//[0]�ōŏ����a�̐F���z

		float f4_tBegAng = TergetAng - SearchAng;
		if (f4_tBegAng < 0.0f)f4_tBegAng += 1.0f;
		int i4_tBegAng = (int)(m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] * f4_tBegAng);
		int i4_tNumAng = (int)(m_pNumAngle[pi4_IndBaseR[i4_tIndSmpR]] * SearchAng * 2);

		for (int i4_tIndAng = 0; i4_tIndAng < i4_tBegAng; i4_tIndAng++)//������
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

	//������
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

	//�s�[�N�����߂�
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

	//�s�[�N�̐ڑ�
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

				//��_���ŏ����@�ŋ��߂�
				XMVECTOR vec_tPos = XMLoadFloat2(pV2_tPos);
				XMVECTOR vec_ChainRoot = XMLoadFloat2(&pV2_tChainRoot[i4_tIndChain]);
				XMVECTOR vec_ChainVec = XMLoadFloat2(&pV2_tChainVec[i4_tIndChain]);
				LeastSquares(&vec_tPos, ListI_tChain->second.size(), &vec_ChainRoot, &vec_ChainVec);
				vec_ChainRoot += vec_tPos;
				vec_ChainVec += vec_tPos;

				//��~�ƃ`�F�C���̌�_
				Line2Circle(tCenterX, tCenterY, (float)Line2CircleR, &vec_ChainRoot, vec_ChainVec);
				XMFLOAT2 tempChain, tempRoot;
				XMStoreFloat2(&tempChain, vec_ChainRoot);
				XMStoreFloat2(&tempRoot, vec_ChainVec);
				//DrawLine(pu4_tRGB_Out, (I4)pV2_tChainRoot[i4_tIndChain].x, (I4)pV2_tChainRoot[i4_tIndChain].y, (I4)pV2_tChainVec[i4_tIndChain].x, (I4)pV2_tChainVec[i4_tIndChain].y, 0x00, 0x00, 0xff);
				cv::circle(inImage, cv::Point((int)tempChain.x, (int)tempChain.y), 5, cv::Scalar(255, 255, 255), -1, CV_AA);
				cv::circle(inImage, cv::Point((int)tempRoot.x, (int)tempRoot.y), 5, cv::Scalar(255, 255, 255), -1, CV_AA);

				vec_ChainVec -= vec_ChainRoot;//root����vec�ւ̃x�N�g���i�ꏊ�͊֌W�Ȃ��Ȃ�j
				XMFLOAT2 tempLen;
				XMVECTOR Vec = XMVector2Length(vec_ChainVec);
				XMStoreFloat2(&tempLen, Vec);
				pV2_tChainRoot[i4_tIndChain] = tempLen;//root����vec�܂ł̒���
				vec_ChainVec /= Vec;
				XMStoreFloat2(&pV2_tChainVec[i4_tIndChain], vec_ChainVec);//�ꏊ�͊֌W�Ȃ��Ȃ邪len�̕��������ڂ��邽�߁i����Ƀ����O�X���̂΂��Ɛ����̕������ł���j

//takamori�w��T���ύX
				//�s�v�ȏ���
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

				//�w��̕`��F�ύX
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

/** @brief �����_�����O�֘A�̃A�C�h���������s���D
@note ���̊֐��́CKHAKI�̃����_�����O�Ɋւ���A�C�h���������s���D
@param image_	�J��������̓��͉摜
@param fps		�V�X�e�������ۂɉғ����Ă���t���[�����[�g
@return HRESULT�G���[�R�[�h��Ԃ�
@sa	Render
**/
HRESULT CViewDirect2D::AppIdle(cv::InputArray image_, double fps)
{
	HRESULT hr = S_OK;
	/* cv::Mat�`���ŉ摜���擾 */
	cv::Mat image = image_.getMat();

	// �L���v�`�����[�h�Ɉڍs���Ă���̂ł����...
	if (flagCapture) {
		MyOutputDebugString(L"�EflagCapture = true\n");
		if (writer.isOpened()) {
			MyOutputDebugString(L"�Ewriter.isOpened() = true\n");
			writer << image;
		}
	}

	/* ��ʂ̍X�V */
	hr = Render(image, fps);

	return hr;
}

/** @brief �E�B���h�E�ւ̃����_�����O�����s����D
@note ���̊֐��́CKHAKI�̃��C���E�B���h�E�֊e�摜�̃����_�����O������K�p����D
���̋@�\�ł́CDirect2D��p�����S�Ẵ����_�����O���s����D
@param image_	�J��������̓��͉摜
@param fps		�V�X�e�������ۂɉғ����Ă���t���[�����[�g
@return HRESULT�G���[�R�[�h��Ԃ�
@sa copyImageToMemory m_pRenderTarget
**/
HRESULT CViewDirect2D::Render(cv::InputArray image_, double fps)
{
	MyOutputDebugString(L"	Render()�����s���܂����D\n");
	HRESULT hResult = S_OK;
	/* �@cv::Mat�`���ŉ摜���擾 */
	renderImage01 = image_.getMat();	// �J��������̓��͉摜
	
	/* �A��w�̈�݂̂𒊏o���� */
	handExtractor(renderImage01, renderImage02);
	/* �B���̒��S�ʒu�𐄒肷�� */
	CalcHandCentroid(renderImage02, renderImage03);
	MyOutputDebugString(L"m_handInfo.hand2DPosX:%d\n", m_handInfo.hand2DPosX);
	MyOutputDebugString(L"m_handInfo.hand2DPosY:%d\n", m_handInfo.hand2DPosY);
	MyOutputDebugString(L"m_handInfo.hand3DPosX:%f\n", m_handInfo.hand3DPosX);
	MyOutputDebugString(L"m_handInfo.hand3DPosY:%f\n", m_handInfo.hand3DPosY);
	MyOutputDebugString(L"m_handInfo.hand3DPosZ:%f\n", m_handInfo.hand3DPosZ);
	/* �C��w�̏�Ԃ���͂��� */
	//AnalyzeHandInf
	/* �D�C���v�b�g���[�h�̃��f�����쐬 */

	/* �摜�f�[�^���m�ۍς݂̃�������֏������� */
	copyImageToMemory(renderImage01, this->memory, 1);	// �@�J��������̓��͉摜����������ɔz�u
	copyImageToMemory(renderImage02, this->memory, 2);	// �A��w�̈�̒��o�摜����������ɔz�u
	copyImageToMemory(renderImage03, this->memory, 3);	// �B���̒��S�ʒu�̐���摜����������ɔz�u
	copyImageToMemory(renderImage01, this->memory, 4);	// �C��͏��̎擾�摜����������ɔz�u
	copyImageToMemory(renderImage01, this->memory, 5);	// �D�C���v�b�g���[�h�̔��ʉ摜����������ɔz�u

	// �^�[�Q�b�g�T�C�Y�̎擾
	D2D1_SIZE_F oTargetSize = m_pRenderTarget->GetSize();

	// �`��J�n
	PAINTSTRUCT tPaintStruct;
	::BeginPaint(this->m_hWndViewTarget, &tPaintStruct);

	/*
	�����_�����O����
	*/
	{
		/* �J�n */
		m_pRenderTarget->BeginDraw();

		/* �w�i�̃N���A */
		D2D1_COLOR_F oBKColor = D2D1::ColorF(D2D1::ColorF::LightSlateGray);
		m_pRenderTarget->Clear(oBKColor);

		/* Bitmap�̕`�� */
		{
			pBitmap->CopyFromMemory(NULL, memory, size * 5 * 4);
			m_pRenderTarget->DrawBitmap(
				pBitmap, //the bitmap to draw [a portion of],
				D2D1::RectF(0.0f, 0.0f, 200*5, 200), //destination rectangle,
				1.0f, //alpha blending multiplier,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, //interpolation mode,
				D2D1::RectF(0.0f, 0.0f, size*5, size)); //source rectangle
		}

		/* �e�L�X�g�t�H�[�}�b�g�̐��� */
		{
			pDWFactory->CreateTextFormat(
				L"Meiryo"						// �t�H���g��
				, NULL							// �t�H���g�R���N�V�����̎w��
				, DWRITE_FONT_WEIGHT_NORMAL		// �e�L�X�g�I�u�W�F�N�g�̃t�H���g�̑���
				, DWRITE_FONT_STYLE_NORMAL		// �e�L�X�g�I�u�W�F�N�g�̃t�H���g�X�^�C��
				, DWRITE_FONT_STRETCH_NORMAL	// �e�L�X�g�I�u�W�F�N�g�̃t�H���g�L�k
				, 16							// �t�H���g�T�C�Y(DIP�P��)
				, L""							// ���P�[�������܂ޕ����z��
				, &pTextFormat					// �V�����쐬���ꂽ�e�L�X�g�`���I�u�W�F�N�g�ւ̃|�C���^�A�h���X���i�[
			);
		}

		/* �o�ߎ��Ԃ�1�b�����ł���Ε`�悷��fps�̃e�L�X�g���X�V���Ȃ� */
		if (totalTime < 1.0f) {
			totalTime += (1 / fps);
		}
		else {
			strText = L"���݂̎��s���x�F" + std::to_wstring((int)fps) + L"fps";
			totalTime = 0.0;
		}

		/* �e�L�X�g�̕`�� https://msdn.microsoft.com/en-us/library/Dd371919(v=VS.85).aspx */
		if (NULL != pBrush && NULL != pTextFormat) {
			m_pRenderTarget->DrawText(
				strText.c_str()		// ������
				, strText.size()    // ������
				, pTextFormat		// �`�悷��e�L�X�g�̏����ݒ�̏ڍ�
				, &D2D1::RectF(0, 0, oTargetSize.width, oTargetSize.height) // �e�L�X�g���`�悳���̈�̃T�C�Y�ƈʒu
				, pBrush			// �e�L�X�g���y�C���g���邽�߂Ɏg�p�����u���V
				, D2D1_DRAW_TEXT_OPTIONS_NONE	// �e�L�X�g���s�N�Z�����E�ɃX�i�b�v���邩�ǂ����A�e�L�X�g�����C�A�E�g��`�ɃN���b�v���邩�ǂ����������l
			);
		}

		/* �I�� */
		m_pRenderTarget->EndDraw();
	}

	// �`��I��
	::EndPaint(this->m_hWndViewTarget, &tPaintStruct);

	return hResult;
}

/** @brief �E�B���h�E�쐬���̃n���h��
@note ���̊֐��́C�E�B���h�E���쐬�����ۂɃn���h���Ƃ��ČĂяo�����D
@return �E�B���h�E�쐬�ɐ��������t���O
**/
int CViewDirect2D::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	// �E�B���h�E�쐬����
	return 0;
}

/** @brief �v���O�������I�������邽�߂ɁC���b�Z�[�W���[�v���I��������D
@note ���̊֐��́C�V�X�e�����I��������ۂɌĂяo����C���b�Z�[�W���[�v���I��������D
@sa	PostQuitMessage
**/
void CViewDirect2D::OnDestroy() {
	// ���b�Z�[�W���[�v�I��
	PostQuitMessage(0);	// PostQuitMessage�Ń��b�Z�[�W���[�v���I��点��
}
