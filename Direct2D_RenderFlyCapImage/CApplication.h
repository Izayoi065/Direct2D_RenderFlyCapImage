#pragma once
/* �O���錾 */
class CViewDirect2D;	// �E�B���h�E�N���X

class CApplication
{
public:
	/* public�����o�ϐ� */
	HINSTANCE m_hInstance;	// �A�v���P�[�V�����C���X�^���X�n���h��m_hInstance
	CViewDirect2D *m_pWindow;		// �E�B���h�E�N���X�|�C���^m_pWindow
	double fps;
public:
	CApplication();				// �R���X�g���N�^
	virtual ~CApplication();	// �f�X�g���N�^

	/* ���z�֐� */
	virtual int Run();	// ���b�Z�[�W���[�v�֐�Run
	virtual int ExitInstance();	// �C���X�^���X�I�������֐�ExitInstance
	virtual BOOL OnIdle(LONG lCount);	// �A�C�h�������֐�OnIdle
	virtual void getInputImage(cv::OutputArray outImage_);

	/* �������z�֐� */
	virtual BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd) = 0;		// �C���X�^���X�������֐� InitInstance
};

