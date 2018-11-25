#pragma once
// std::string��std::wstring�̐؂�ւ�.
#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

/* ��`�֐� */
#define apiLoadImage(h,id,type,cast)    ((cast)LoadImage(h,MAKEINTRESOURCE(id),type,0,0,(LR_DEFAULTSIZE | LR_SHARED)))
#define apiLoadIcon(h,id)               apiLoadImage(h,id,IMAGE_ICON,HICON)
#define apiLoadCursor(h,id)             apiLoadImage(h,id,IMAGE_CURSOR,HCURSOR)

/* �O���錾 */
class CApplication;		// �A�v���P�[�V�����N���X CApplication

class CWinBase
{
private:
	/* �{�^���Ǘ��p��ID */
	enum ID_BUTTON
	{
		CID_BT_GetFolderPass = 10000,
		CID_BT_CaptureStart,
		CID_BT_CaptureEnd,
		CID_BT_END,
	};
	/* STATIC�R���g���[���Ǘ��p��ID */
	enum ID_STATIC
	{
		CID_ST_GetFolderPass = 11000,
		CID_ST_FileName,
		CID_ST_CaptureStart,
		CID_ST_CaptureEnd,
		CID_ST_END,
	};
	/* �e�L�X�g�{�b�N�X�Ǘ��p��ID */
	enum ID_EDIT
	{
		CID_TX_CapturePass = 12000,
		CID_TX_FileName,
		CID_TX_Console,
		CID_TX_END,
	};
	typedef struct MyAdvancedSetting {
		int TH_MIN_HUE;
		int TH_MAX_HUE;
		int TH_MIN_SATURATION;
		int TH_MAX_SATURATION;
		int TH_MIN_BRIGHTNESS;
		int TH_MAX_BRIGHTNESS;
	}MYADVANCEDSETTING;
	HWND m_hwndTextBoxPhase[CID_TX_END];	// �e�L�X�g�{�b�N�X�Ǘ��p�̎��ʎq
	HWND m_hwndSTATICPhase[CID_ST_END];		// STATIC�R���g���[���Ǘ��p�̎��ʎq
	HWND m_hwndBUTTONPhase[CID_BT_END];		// �{�^���Ǘ��p�̎��ʎq
	MYADVANCEDSETTING MASetting;
	HBRUSH      m_hBrush_BkColor;   //�w�i�̐F
public:
	/* �����o�ϐ� */
	HWND m_hWnd;			// �E�B���h�E�n���h��m_hWnd
	HWND m_hWndViewTarget;	// �e��摜�������_�����O����X�y�[�X
	CApplication * m_pApp;	// �A�v���P�[�V�����I�u�W�F�N�g�|�C���^m_pApp
	cv::VideoWriter writer; //�r�f�I�ɏ������ޕϐ�
	BOOL flagCapture = false;
	cv::Scalar hsv_min = cv::Scalar(0, 0, 120);
	cv::Scalar hsv_max = cv::Scalar(180, 255, 150);
	static std::map<HWND, CWinBase *> m_mapWindowMap;	// �E�B���h�E�n���h������E�B���h�E�I�u�W�F�N�g�������}�b�v.

public:
	CWinBase(CApplication *pApp);	// �R���X�g���N�^CWinBase(pApp)
	virtual ~CWinBase();			// �f�X�g���N�^~CWinBase
	HRESULT GetDir(HWND hWnd, TCHAR* def_dir, TCHAR* path);
	std::string WStringToString(std::wstring oWString);
	void setFlagCapture(BOOL flag);
	void setHSVMinThreshold(cv::Scalar min);
	void setHSVMaxThreshold(cv::Scalar max);
	cv::Scalar getHSVMinThreshold();
	cv::Scalar getHSVMaxThreshold();

	static BOOL RegisterClass(HINSTANCE hInstance);	// �E�B���h�E�N���X�o�^�֐�RegisterClass.
	static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	// �ŏ��ɃV�X�e�����烁�b�Z�[�W���͂��E�B���h�E�v���V�[�W��StaticWindowProc.
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK Setting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

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

