/****************************************************************************************************
 *�^�C�g���FWin32API �A�v���P�[�V�����ɂ����郁�C�����\�b�h
 *�����@�@�F�V�X�e���̃G���g���|�C���g
 *		�Fhttps://www.ptgrey.com/flycapture-sdk
 *�O��LIB �FFlyCapture2	2.10.3.169�C
 *			OpenCV		3.4.0
 *
 *���쌠�@�FTomoki Kondo
 *
 *�ύX�����F2018.11.16 Fryday
 *�@�@�@�F�V�K�o�^
 *
 ****************************************************************************************************/
 /* ��`�֐� */
#define SAFE_DALETE(x) {if(x) {delete (x); (x) = NULL;}}	// ���������I�u�W�F�N�g���J�����CNULL��������

/* �C���N���[�h�t�@�C�� */
#include "CMainApplication.h"	// ���C���A�v���P�[�V�����N���X

int WINAPI _tWinMain(
	HINSTANCE hInstance,		// ���݂̃C���X�^���X�E�n���h��
	HINSTANCE hPrevInstance,	// �ȑO�̃C���X�^���X�E�n���h��
	LPTSTR lpCmdLine,			// �R�}���h���C���̕�����
	int nCmdShow)				// �E�B���h�E�̕\�����
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// �ϐ��E�I�u�W�F�N�g�̐錾
	CApplication *pApp;	// CApplication�I�u�W�F�N�g�|�C���^pApp
	int iRet;	// Run�̖߂�l:iRet

	// ���C���A�v���P�[�V�����I�u�W�F�N�g�̐���
	pApp = new CMainApplication();	// CMainApplication�I�u�W�F�N�g���쐬

	// �C���X�^���X������
	if (!pApp->InitInstance(hInstance, lpCmdLine, nCmdShow)) {	// InitInstance�ŏ�����
		/* FALSE�Ȃ�I������ */
		pApp->ExitInstance();	// ExitInstance�ŏI������
		delete pApp;			// pApp�����
		return -1;				// �ُ�I���Ȃ̂�-1��Ԃ�
	}

	// ���b�Z�[�W���[�v
	iRet = pApp->Run();	// Run�Ń��b�Z�[�W���[�v, �I��������߂�l��iRet��

	// �A�v���P�[�V�����I�u�W�F�N�g�̊J��
	SAFE_DALETE(pApp);

	// �v���O�����̏I��
	return 0;	// ����I����0��Ԃ�
}