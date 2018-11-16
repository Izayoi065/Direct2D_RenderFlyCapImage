#pragma once
// std::string��std::wstring�̐؂�ւ�.
#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

//------------------------------------------------
// ��`�֐�
//------------------------------------------------
#define apiLoadImage(h,id,type,cast)    ((cast)LoadImage(h,MAKEINTRESOURCE(id),type,0,0,(LR_DEFAULTSIZE | LR_SHARED)))
#define apiLoadIcon(h,id)               apiLoadImage(h,id,IMAGE_ICON,HICON)
#define apiLoadCursor(h,id)             apiLoadImage(h,id,IMAGE_CURSOR,HCURSOR)

/* �O���錾 */
class CApplication;		// �A�v���P�[�V�����N���X CApplication

class CWinBase
{
public:
	/* �����o�ϐ� */
	HWND m_hWnd;			// �E�B���h�E�n���h��m_hWnd
	CApplication * m_pApp;	// �A�v���P�[�V�����I�u�W�F�N�g�|�C���^m_pApp
	static std::map<HWND, CWinBase *> m_mapWindowMap;	// �E�B���h�E�n���h������E�B���h�E�I�u�W�F�N�g�������}�b�v.

public:
	CWinBase(CApplication *pApp);	// �R���X�g���N�^CWinBase(pApp)
	virtual ~CWinBase();			// �f�X�g���N�^~CWinBase

	static BOOL RegisterClass(HINSTANCE hInstance);	// �E�B���h�E�N���X�o�^�֐�RegisterClass.
	static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	// �ŏ��ɃV�X�e�����烁�b�Z�[�W���͂��E�B���h�E�v���V�[�W��StaticWindowProc.
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	/* ���z�֐� */
	virtual BOOL Create(LPCTSTR lpctszClassName, LPCTSTR lpctszWindowName, DWORD dwStyle,
		const RECT & rect, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);					// �E�B���h�E�쐬�֐� Create
	virtual BOOL Create(LPCTSTR lpctszWindowName, const RECT & rect);							// �E�B���h�E�쐬�֐� Create(�E�B���h�E���ƃT�C�Y�݂̂̃I�[�o�[���[�h�֐�)
	virtual BOOL ShowWindow(int nCmdShow);														// �E�B���h�E�\���֐� ShowWindow
	virtual LRESULT DynamicWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);		// StaticWindowProc����e�E�B���h�E�I�u�W�F�N�g���ƂɌĂяo�����E�B���h�E�v���V�[�W�� DynamicWindowProc


	/* �������z�֐� */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) = 0;		// �E�B���h�E�쐬���̃n���h�� OnCreate
	virtual void OnDestroy() = 0;											// �E�B���h�E�j�����̃n���h�� OnDestroy
};

