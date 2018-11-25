#pragma once
// std::stringとstd::wstringの切り替え.
#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

/* 定義関数 */
#define apiLoadImage(h,id,type,cast)    ((cast)LoadImage(h,MAKEINTRESOURCE(id),type,0,0,(LR_DEFAULTSIZE | LR_SHARED)))
#define apiLoadIcon(h,id)               apiLoadImage(h,id,IMAGE_ICON,HICON)
#define apiLoadCursor(h,id)             apiLoadImage(h,id,IMAGE_CURSOR,HCURSOR)

/* 前方宣言 */
class CApplication;		// アプリケーションクラス CApplication

class CWinBase
{
private:
	/* ボタン管理用のID */
	enum ID_BUTTON
	{
		CID_BT_GetFolderPass = 10000,
		CID_BT_CaptureStart,
		CID_BT_CaptureEnd,
		CID_BT_END,
	};
	/* STATICコントロール管理用のID */
	enum ID_STATIC
	{
		CID_ST_GetFolderPass = 11000,
		CID_ST_FileName,
		CID_ST_CaptureStart,
		CID_ST_CaptureEnd,
		CID_ST_END,
	};
	/* テキストボックス管理用のID */
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
	HWND m_hwndTextBoxPhase[CID_TX_END];	// テキストボックス管理用の識別子
	HWND m_hwndSTATICPhase[CID_ST_END];		// STATICコントロール管理用の識別子
	HWND m_hwndBUTTONPhase[CID_BT_END];		// ボタン管理用の識別子
	MYADVANCEDSETTING MASetting;
	HBRUSH      m_hBrush_BkColor;   //背景の色
public:
	/* メンバ変数 */
	HWND m_hWnd;			// ウィンドウハンドルm_hWnd
	HWND m_hWndViewTarget;	// 各種画像をレンダリングするスペース
	CApplication * m_pApp;	// アプリケーションオブジェクトポインタm_pApp
	cv::VideoWriter writer; //ビデオに書き込む変数
	BOOL flagCapture = false;
	cv::Scalar hsv_min = cv::Scalar(0, 0, 120);
	cv::Scalar hsv_max = cv::Scalar(180, 255, 150);
	static std::map<HWND, CWinBase *> m_mapWindowMap;	// ウィンドウハンドルからウィンドウオブジェクトを引くマップ.

public:
	CWinBase(CApplication *pApp);	// コンストラクタCWinBase(pApp)
	virtual ~CWinBase();			// デストラクタ~CWinBase
	HRESULT GetDir(HWND hWnd, TCHAR* def_dir, TCHAR* path);
	std::string WStringToString(std::wstring oWString);
	void setFlagCapture(BOOL flag);
	void setHSVMinThreshold(cv::Scalar min);
	void setHSVMaxThreshold(cv::Scalar max);
	cv::Scalar getHSVMinThreshold();
	cv::Scalar getHSVMaxThreshold();

	static BOOL RegisterClass(HINSTANCE hInstance);	// ウィンドウクラス登録関数RegisterClass.
	static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	// 最初にシステムからメッセージが届くウィンドウプロシージャStaticWindowProc.
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK Setting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	/* 仮想関数 */
	virtual BOOL Create(LPCTSTR lpctszClassName, LPCTSTR lpctszWindowName, DWORD dwStyle,
		const RECT & rect, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);					// ウィンドウ作成関数 Create
	virtual BOOL Create(LPCTSTR lpctszWindowName, const RECT & rect);							// ウィンドウ作成関数 Create(ウィンドウ名とサイズのみのオーバーロード関数)
	virtual BOOL ShowWindow(int nCmdShow);														// ウィンドウ表示関数 ShowWindow
	virtual LRESULT DynamicWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);		// StaticWindowProcから各ウィンドウオブジェクトごとに呼び出されるウィンドウプロシージャ DynamicWindowProc


	/* 純粋仮想関数 */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) = 0;		// ウィンドウ作成時のハンドラ OnCreate
	virtual void OnDestroy() = 0;											// ウィンドウ破棄時のハンドラ OnDestroy
};

