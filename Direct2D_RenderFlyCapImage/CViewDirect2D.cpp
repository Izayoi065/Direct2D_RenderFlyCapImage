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

 /* �C���N���[�h�t�@�C�� */
#include "CViewDirect2D.h"

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
	for (int i = 0;i < sizeof(memory);i++) {
		memory[i] = 0;
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
	::GetClientRect(this->m_hWnd, &rect);	// �N���C�A���g�̈�̃T�C�Y���擾

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
	D2D1_HWND_RENDER_TARGET_PROPERTIES HwndRenderTargetProperties = D2D1::HwndRenderTargetProperties(this->m_hWnd, PixelSize);
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
	cv::threshold(renderImage01, renderImage02, 100, 255, cv::THRESH_BINARY);

	/* �A��w�̈�݂̂𒊏o���� */
	/* �B���̒��S�ʒu�𐄒肷�� */
	/* �C��w�̏�Ԃ���͂��� */
	/* �D�C���v�b�g���[�h�̃��f�����쐬 */

	/* �摜�f�[�^���m�ۍς݂̃�������֏������� */
	copyImageToMemory(renderImage01, this->memory, 1);	// �@�J��������̓��͉摜����������ɔz�u
	copyImageToMemory(renderImage02, this->memory, 2);	// �A��w�̈�̒��o�摜����������ɔz�u
	copyImageToMemory(renderImage01, this->memory, 3);	// �B���̒��S�ʒu�̐���摜����������ɔz�u
	copyImageToMemory(renderImage01, this->memory, 4);	// �C��͏��̎擾�摜����������ɔz�u
	copyImageToMemory(renderImage01, this->memory, 5);	// �D�C���v�b�g���[�h�̔��ʉ摜����������ɔz�u

	// �^�[�Q�b�g�T�C�Y�̎擾
	D2D1_SIZE_F oTargetSize = m_pRenderTarget->GetSize();

	// �`��J�n
	PAINTSTRUCT tPaintStruct;
	::BeginPaint(this->m_hWnd, &tPaintStruct);

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
	::EndPaint(this->m_hWnd, &tPaintStruct);

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