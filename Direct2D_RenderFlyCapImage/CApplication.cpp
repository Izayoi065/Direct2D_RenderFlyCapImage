#define SAFE_DALETE(x) {if(x) {delete (x); (x) = NULL;}}	// ���������I�u�W�F�N�g���J�����CNULL��������

#include "CApplication.h"

/* �R���X�g���N�^ CApplication */
CApplication::CApplication() {
	MyOutputDebugString(L"	CApplication::CApplication() ���\�b�h���Ăяo����܂����D\n");

	/* �����o�̏����� */
	m_hInstance = NULL;	// �C���X�^���X�n���h����NULL
	m_pWindow = NULL;	// �E�B���h�E�I�u�W�F�N�g��NULL
}

/* �f�X�g���N�^ ~CApplication */
CApplication::~CApplication() {
	MyOutputDebugString(L"	CApplication::~CApplication() ���\�b�h���Ăяo����܂����D\n");
	/* �E�B���h�E�I�u�W�F�N�g���c���Ă������� */
	SAFE_DALETE(m_pWindow);
}

/* �V�X�e���̎��s���� Run */
int CApplication::Run() {
	// �ϐ��̐錾
	MSG msg;			// ���b�Z�[�W���msg
	LONG lCount = 0;	// ���[�v��lCount

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
		}
	}

	/* �I������ */
	return ExitInstance();	// ExitInstance�̖߂�l��Ԃ�
}

/* �C���X�^���X�I�������֐� ExitInstance */
int CApplication::ExitInstance() {
	/*�C���X�^���X�I�������̒ʒm */
	MyOutputDebugString(L"	CApplication::ExitInstance() ���\�b�h���Ăяo����܂����D\n");

	/* �E�B���h�E�I�u�W�F�N�g���c���Ă������� */
	SAFE_DALETE(m_pWindow);

	/* ����I�� */
	return 0;
}

/* �A�C�h�������֐� OnIdle */
BOOL CApplication::OnIdle(LONG lCount) {
	// �Ƃ肠����TRUE
	return TRUE;
}