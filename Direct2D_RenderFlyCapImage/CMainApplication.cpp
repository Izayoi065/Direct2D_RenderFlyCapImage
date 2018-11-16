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
	rc.right = 900;		// 100 + 800
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
			hr = this->m_pWindow->AppIdle(FlyCap.readImage(), FPS.GetFPS());
		}
	}

	/* �I������ */
	return ExitInstance();	// ExitInstance�̖߂�l��Ԃ�
}