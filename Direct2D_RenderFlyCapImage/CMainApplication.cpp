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
#define SAFE_DALETE(x) {if(x) {delete (x); (x) = NULL;}}	// ���������I�u�W�F�N�g���J�����CNULL��������

/* �C���N���[�h�t�@�C�� */
#include "CMainApplication.h"	// ���C���A�v���P�[�V�����N���X
#include "CWinBase.h"			// ���E�B���h�E�N���X
#include "CViewDirect2D.h"		// DirectX2D�֘A�̏������N���X
#include "FlyCap2CVWrapper.h"	// OpenCV�p��FlyCapture2���b�p�[�N���X
#include "FPSCounter.h"

/* �R���X�g���N�^ CMainApplication */
CMainApplication::CMainApplication() {

}

/* �f�X�g���N�^ ~CMainApplication */
CMainApplication::~CMainApplication() {

}

BITMAPINFOHEADER CMainApplication::MakeBITINFO(int nWidth, int nHeight, LPVOID lpBits)
{
	DWORD dwSizeImage;
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;

	dwSizeImage = nHeight * ((3 * nWidth + 3) / 4) * 4;

	ZeroMemory(&bmfHeader, sizeof(BITMAPFILEHEADER));
	bmfHeader.bfType = *(LPWORD)"BM";
	bmfHeader.bfSize = sizeof(BITMAPFILEHEADER)
		+ sizeof(BITMAPINFOHEADER) + dwSizeImage;

	bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER)
		+ sizeof(BITMAPINFOHEADER);

	ZeroMemory(&bmiHeader, sizeof(BITMAPINFOHEADER));
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = nWidth;
	bmiHeader.biHeight = -nHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 32;
	bmiHeader.biSizeImage -= dwSizeImage;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;
	bmiHeader.biXPelsPerMeter = 0;
	bmiHeader.biYPelsPerMeter = 0;

	return bmiHeader;
}

HBITMAP CMainApplication::CreateBackbuffer(int nWidth, int nHeight)
{
	LPVOID lp;
	BITMAPINFO bmi;
	BITMAPINFOHEADER bmiHeader;

	ZeroMemory(&bmiHeader, sizeof(BITMAPINFOHEADER));
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = nWidth;
	bmiHeader.biHeight = -nHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 32;

	bmi.bmiHeader = bmiHeader;

	return CreateDIBSection(NULL, (LPBITMAPINFO)&bmi,
		DIB_RGB_COLORS, &lp, NULL, 0);
}

void CMainApplication::ConvertToMat(HWND hwnd, cv::OutputArray outImage_)
{
	HDC hdc;
	RECT rc;
	BITMAP bm;
	HBITMAP hbmp;
	HBITMAP hbmpPrev;

	GetClientRect(hwnd, &rc);

	hdc = CreateCompatibleDC(NULL);
	hbmp = CreateBackbuffer(rc.right, rc.bottom);
	hbmpPrev = (HBITMAP)SelectObject(hdc, hbmp);

	BitBlt(hdc, 0, 0, rc.right, rc.bottom, GetWindowDC(hwnd), 0, 0, SRCCOPY);

	GetObject(hbmp, sizeof(BITMAP), &bm);


	BITMAPINFOHEADER bi = MakeBITINFO(rc.right, rc.bottom, bm.bmBits);

	cv::Mat converted_mat;
	converted_mat.create(rc.bottom, rc.right, CV_8UC4);

	GetDIBits(hdc, hbmp, 0, bm.bmHeight,
		converted_mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	SelectObject(hdc, hbmpPrev);
	DeleteObject(hbmp);
	DeleteDC(hdc);

	converted_mat.copyTo(outImage_);
}


/* �C���X�^���X�������֐� InitInstance */
BOOL CMainApplication::InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd) {
	// �ϐ��̐錾
	RECT rc;	// �E�B���h�E�T�C�Y rc

	// �C���X�^���X�n���h���������o�ɃZ�b�g
	m_hInstance = hInstance;	// m_hInstance��hInstance���Z�b�g

	/* �E�B���h�E�N���X"CWinBase"�̓o�^ */
	if (!CWinBase::RegisterClass(m_hInstance)) {	// CWinBase::RegisterClass�œo�^
		// �G���[����
		MessageBox(NULL, _T("�E�B���h�E�N���X�̓o�^�Ɏ��s���܂���!"), _T("CWinBase"), MB_OK | MB_ICONHAND);	// MessageBox��"�E�B���h�E�N���X�̓o�^�Ɏ��s���܂���!"�ƕ\��
		return -1;	// �ُ�I��
	}

	/* �E�B���h�E�I�u�W�F�N�g�̍쐬 */
	//m_pWindow = new C_View(this);
	m_pWindow = new CViewDirect2D(this);

	/* �E�B���h�E�T�C�Y�̃Z�b�g */
	rc.left = 100;		// 100
	rc.top = 100;		// 100
	rc.right = 1100;		// 100 + 1000
	rc.bottom = 700;	// 100 + 600

	/* �E�B���h�E�̍쐬 */
	if (!m_pWindow->Create(_T("CWinBase"), rc)) {	// Create�̏ȗ��łŃE�B���h�E�쐬
													// �G���[����
		MessageBox(NULL, _T("�E�B���h�E�쐬�Ɏ��s���܂���!"), _T("CWinBase"), MB_OK | MB_ICONHAND);	// MessageBox��"�E�B���h�E�쐬�Ɏ��s���܂���!"�ƕ\��
		return FALSE;	// FALSE��Ԃ�
	}

	/* �E�B���h�E�\�� */
	m_pWindow->ShowWindow(SW_SHOW);		// ShowWindow�ŕ\��

	// DirectX11�̐ݒ�
	m_pWindow->InitDirect2D(hInstance, lpCmdLine, nShowCmd);

	// TRUE�ŕԂ�
	return TRUE;
}

int CMainApplication::Run() {
	// �ϐ��̐錾
	MSG msg;			// ���b�Z�[�W���msg
	LONG lCount = 0;	// ���[�v��lCount

	FlyCap2CVWrapper FlyCap;
	FPSCounter FPS(10);

	HRESULT hr = S_OK;
	/* ���b�Z�[�W���[�v */
	while (TRUE) {
		// ���b�Z�[�W�����Ă��邩���m�F
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {	// PeekMessage�Ń��b�Z�[�W���m�F
			if (GetMessage(&msg, NULL, 0, 0) > 0) {	// GetMessage�Ń��b�Z�[�W�擾.
				/* �E�B���h�E���b�Z�[�W�̑��o */
				DispatchMessage(&msg);	// DispatchMessage�ŃE�B���h�E�v���V�[�W���ɑ��o
				TranslateMessage(&msg);	// TranslateMessage�ŉ��z�L�[���b�Z�[�W�𕶎��֕ϊ�
			}
			else {	// WM_QUIT�܂��̓G���[
				// ���b�Z�[�W���[�v�𔲂���
				break;	// break�Ŕ�����
			}
		}
		else {	// ���b�Z�[�W�����Ă��Ȃ��ꍇ
			/* �A�C�h������ */
			if (OnIdle(lCount)) {	// OnIdle��lCount��n����, ���̒��ŃA�C�h������������
				lCount++;	// TRUE�ŕԂ��Ă��邽�т�lCount�𑝂₷
			}
			/* �����_�����O���� */
			fps = FPS.GetFPS();
			//image = FlyCap.readImage();
			image = cv::imread("data/resources/handImage03.png");
			//cv::resize(image, image, cv::Size(), 0.5, 0.5);
			hr = this->m_pWindow->AppIdle(image, fps);
		}
	}

	/* �I������ */
	return ExitInstance();	// ExitInstance�̖߂�l��Ԃ�
}