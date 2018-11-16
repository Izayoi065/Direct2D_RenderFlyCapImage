#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#include "CViewDirect2D.h"

CViewDirect2D::CViewDirect2D(CApplication * pApp) : CWinBase(pApp)
{
	MyOutputDebugString(L"	CViewDirect2D(CApplication*):CWinBase(CApplication)���Ăяo����܂����I\n");
	m_pD2d1Factory = NULL;
	m_pRenderTarget = NULL;
	pTextFormat = NULL;
	pDWFactory = NULL;
	pBrush = NULL;
	pBitmap = NULL;	
}


CViewDirect2D::~CViewDirect2D()
{
	MyOutputDebugString(L"	~ViewDirectX11()���Ăяo����܂����I\n");
	ReleaseD2D();
}

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
	/* cv::Mat�`���ŉ摜���擾 */
	cv::Mat image = image_.getMat();

	/* ��ʂ̍X�V */
	hr = Render(image, fps);

	return hr;
}

HRESULT CViewDirect2D::Render(cv::InputArray image_, double fps)
{
	MyOutputDebugString(L"	Render()�����s���܂����D\n");
	HRESULT hResult = S_OK;
	/* cv::Mat�`���ŉ摜���擾 */
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

	/* �摜�f�[�^���m�ۍς݂̃�������֏������� */
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

	// �^�[�Q�b�g�T�C�Y�̎擾
	D2D1_SIZE_F oTargetSize = m_pRenderTarget->GetSize();

	// �`��J�n
	PAINTSTRUCT tPaintStruct;
	::BeginPaint(this->m_hWnd, &tPaintStruct);

	/*
	�e�L�X�g�̕`��
	*/
	{
		/* �J�n */
		m_pRenderTarget->BeginDraw();

		/* �w�i�̃N���A */
		D2D1_COLOR_F oBKColor = D2D1::ColorF(D2D1::ColorF::LightSlateGray);
		m_pRenderTarget->Clear(oBKColor);

		/* Bitmap�̕`�� */
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

/* �E�B���h�E�쐬���̃n���h�� OnCreate */
int CViewDirect2D::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	// �E�B���h�E�쐬����
	return 0;
}

/* �E�B���h�E�j�����̃n���h�� OnDestroy */
void CViewDirect2D::OnDestroy() {
	// ���b�Z�[�W���[�v�I��
	PostQuitMessage(0);	// PostQuitMessage�Ń��b�Z�[�W���[�v���I��点��
}