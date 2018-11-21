#pragma once
/* �C���N���[�h�t�@�C�� */
#include "CApplication.h"	// �A�v���P�[�V�����N���X

/* �O���錾 */
class CApplication;

class CMainApplication :
	public CApplication
{
private:
	cv::Mat image;
public:
public:
	/* public�����o�֐� */
	CMainApplication();				// �R���X�g���N�^
	virtual ~CMainApplication();	// �f�X�g���N�^
	BITMAPINFOHEADER MakeBITINFO(int nWidth, int nHeight, LPVOID lpBits);
	HBITMAP CreateBackbuffer(int nWidth, int nHeight);
	void ConvertToMat(HWND hwnd, cv::OutputArray outImage_);

	/* ���z�֐� */
	virtual BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd);	// �C���X�^���X�������֐� InitInstance
	virtual int Run();
};

