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
 /* �C���N���[�h�t�@�C�� */
#include "CApplication.h"	// �A�v���P�[�V�����N���X
#include "CWinBase.h"		// ���E�B���h�E�N���X
#include "resource.h"		// ���\�[�X�t�@�C��


// static�����o�ϐ��̒�`.
std::map<HWND, CWinBase *> CWinBase::m_mapWindowMap;	// static�����o�ϐ�CWinBase::m_mapWindowMap�͐錾�ƕʂɂ����ɒ�`���Ȃ��Ƃ����Ȃ�.

/** @brief CWinBase�N���X�̃R���X�g���N�^
@note ���̊֐��́C���̃N���X���Ăяo���ꂽ�ۂɁC�ŏ��Ɏ��s�����
@param pApp
@sa CViewDirect2D CApplication
**/
CWinBase::CWinBase(CApplication * pApp) {
	MyOutputDebugString(L"	CWinBase::CWinBase(CApplication *) ���\�b�h���Ăяo����܂����D\n");
	// �����o�̏�����
	m_hWnd = NULL;	// m_hWnd��NULL.
	m_pApp = pApp;	// m_pApp��pApp.
	m_hBrush_BkColor = CreateSolidBrush(RGB(0, 0, 0));	// STATIC�R���g���[���̔w�i
	MASetting = { 2,13,115,255,0,160 };
	hsv_min = cv::Scalar(MASetting.TH_MIN_HUE, MASetting.TH_MIN_SATURATION, MASetting.TH_MIN_BRIGHTNESS);
	hsv_max = cv::Scalar(MASetting.TH_MAX_HUE, MASetting.TH_MAX_SATURATION, MASetting.TH_MAX_BRIGHTNESS);
}

/** @brief CWinBase�N���X�̃f�X�g���N�^
@note ���̊֐��́C���̃N���X���Ăяo���ꂽ�ۂɁC�Ō�Ɏ��s�����
@sa 
**/
CWinBase::~CWinBase() {
	MyOutputDebugString(L"	CWinBase::~CWinBase() ���\�b�h���Ăяo����܂����D\n");
}

/** @brief �t�H���_�[�����擾����
@note ���̊֐��́CSHBrowseForFolder��p���ăt�H���_�����擾����_�C�A���O���|�b�v������D
@param hWnd		�_�C�A���O�̐e�E�B���h�E�E�n���h��
@param def_dir	�t�H���_�I�����̏����f�B���N�g��
@param path		�I�������t�H���_�̃p�X
@return HRESULT�G���[�R�[�h��Ԃ�
@sa WStringToString BrowseCallbackProc
**/
HRESULT CWinBase::GetDir(HWND hWnd, TCHAR * def_dir, TCHAR * path)
{
	BROWSEINFO bInfo;
	LPITEMIDLIST pIDList;

	memset(&bInfo, 0, sizeof(bInfo));
	bInfo.hwndOwner = hWnd; // �_�C�A���O�̐e�E�C���h�E�̃n���h�� 
	bInfo.pidlRoot = NULL; // ���[�g�t�H���_���f�X�N�g�b�v�t�H���_�Ƃ��� 
	bInfo.pszDisplayName = path; //�t�H���_�����󂯎��o�b�t�@�ւ̃|�C���^ 
	bInfo.lpszTitle = TEXT("�t�H���_�̑I��"); // �c���[�r���[�̏㕔�ɕ\������镶���� 
	bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE | BIF_NEWDIALOGSTYLE; // �\�������t�H���_�̎�ނ������t���O 
	bInfo.lpfn = BrowseCallbackProc; // BrowseCallbackProc�֐��̃|�C���^ 
	bInfo.lParam = (LPARAM)def_dir;
	pIDList = SHBrowseForFolder(&bInfo);
	if (pIDList == NULL) {
		path[0] = _TEXT('\0');
		return S_OK; //�����I������Ȃ������ꍇ 
	}
	else {
		if (!SHGetPathFromIDList(pIDList, path))
			return false;//�ϊ��Ɏ��s 
		CoTaskMemFree(pIDList);// pIDList�̃��������J�� 
		return S_OK;
	}
}

/** @brief std::wstring�^�������std::string�^������ɕϊ�����D
@note ���̊֐��́Cstd::wstring�^������(TCHAR�Ȃ�)��std::string�^������(cv::String�Ȃ�)�ւ̕ϊ�������K�p����D
@param oWString	�ϊ�������std::wstring�^�̕�����
@return	std::string�ϊ���̕�����
@sa flagCapture WideCharToMultiByte
**/
std::string CWinBase::WStringToString(std::wstring oWString) {
	// wstring �� SJIS
	int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str()
		, -1, (char *)NULL, 0, NULL, NULL);

	// �o�b�t�@�̎擾
	CHAR* cpMultiByte = new CHAR[iBufferSize];

	// wstring �� SJIS
	WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte
		, iBufferSize, NULL, NULL);

	// string�̐���
	std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);

	// �o�b�t�@�̔j��
	delete[] cpMultiByte;

	// �ϊ����ʂ�Ԃ�
	return(oRet);
}

/** @brief �^�惂�[�h�̊J�n�E��~�̃t���O��ݒ肷��D
@note ���̊֐��́C�^�惂�[�h�̊J�n�E��~�̃t���O�ł���CWinBase::flagCapture��؂�ւ���D
@param flag	�^�惂�[�h�̃t���O
@sa flagCapture
**/
void CWinBase::setFlagCapture(BOOL flag)
{
	flagCapture = flag;
}

void CWinBase::setHSVMinThreshold(cv::Scalar min)
{
	hsv_min = min;
}

void CWinBase::setHSVMaxThreshold(cv::Scalar max)
{
	hsv_max = max;
}

cv::Scalar CWinBase::getHSVMinThreshold()
{
	return hsv_min;
}

cv::Scalar CWinBase::getHSVMaxThreshold()
{
	return hsv_max;
}

/** @brief �E�B���h�E�N���X��o�^����
@note ���̊֐��́C������HINSTANCE�p�����[�^��ێ������E�B���h�E�N���X���쐬����D
���̋@�\�ł́CWNDCLASSEX�ŃE�B���h�E�N���X�̐ݒ���s���CRegisterClassEx�ɂč쐬���s���D
@param hInstance 
@return	BOOL �֐��̏������ʂ̐���
@sa callback CWinBase WNDCLASSEX RegisterClassEx
**/
BOOL CWinBase::RegisterClass(HINSTANCE hInstance) {
	MyOutputDebugString(L"	CWinBase::RegisterClass(HINSTANCE) ���\�b�h���Ăяo����܂����D\n");

	/* �ϐ��̐錾 */
	WNDCLASSEX wcex = { 0 };	// �E�B���h�E�N���XEX wcex

	/* �E�B���h�E�N���X"CWinBase"�̐ݒ� */
	wcex.cbSize = sizeof(WNDCLASSEX);						// �\���̂̃T�C�Y
	wcex.style = (CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);	// �N���X�̃X�^�C��
	wcex.lpfnWndProc = CWinBase::StaticWindowProc;				// �E�B���h�E�v���V�[�W���̐ݒ�
	wcex.cbClsExtra = 0;										// �g���T�C�Y�E�f�[�^�T�C�Y
	wcex.cbWndExtra = 0;										// �g���E�B���h�E�E�f�[�^�T�C�Y
	wcex.hInstance = hInstance;								// �C���X�^���X�E�n���h��
	wcex.hIcon = apiLoadIcon(NULL, IDI_APPLICATION);		// �A�C�R���̃n���h��
	wcex.hIconSm = apiLoadIcon(NULL, IDI_APPLICATION);		// �������A�C�R���̃n���h��
	wcex.hCursor = apiLoadCursor(NULL, IDC_ARROW);			// �J�[�\���̃n���h��
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);		// �w�i�u���V�̃n���h��
	wcex.lpszMenuName = (LPCWSTR)IDC_DIRECT2DRENDERFLYCAPIMAGE;						// ���j���[�o�[�̎��ʕ�����
	//wcex.lpszMenuName = NULL;										// ���j���[�o�[�̎��ʕ�����
	wcex.lpszClassName = _T("CWinBase");							// �E�B���h�E�N���X��"CWinBase"

	/* �E�B���h�E�N���X�̓o�^ */
	if (!::RegisterClassEx(&wcex)) {	// WindowsAPI��RegisterClass�֐��œo�^
		return FALSE;
	}

	// �����Ȃ�TRUE
	return TRUE;
}

/** @brief ��{�I�ȃE�B���h�E�v���V�[�W��
@note ���̊֐��́C�A�v���P�[�V�����ɑ����Ă������b�Z�[�W�����������p�̃R�[���o�b�N�֐��D
���C���E�B���h�E�쐬���ɂ̓E�B���h�E�I�u�W�F�N�g CWinBase�ƃE�B���h�E�E�n���h�� hwnd���y�A�œo�^����D
���̑��̏����ɂ��ẮC�E�B���h�E�I�u�W�F�N�g���擾�ł���ꍇ�͂��̃E�B���h�E��DynamicWindowProc�ɁC
�ł��Ȃ��ꍇ�͊����DefWindowProc�ɓn���D
@param hWnd		�E�B���h�E�ւ̃n���h��
@param uMsg		���b�Z�[�W
@param wParam	�ǉ��̃��b�Z�[�W���Fw-�p�����[�^
@param lParam	�ǉ��̃��b�Z�[�W���Fl-�p�����[�^
@return	LRESULT �G���[�R�[�h��Ԃ�
@sa callback CWinBase m_mapWindowMap DynamicWindowProc DefWindowProc
**/
LRESULT CWinBase::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// �|�C���^�̐錾
	CWinBase *pWindow = NULL;		// CWinBase �I�u�W�F�N�g�|�C���^ pWindow

	/* ���b�Z�[�W�E�B���h�E���� */
	switch (uMsg) {
		/* �E�B���h�E�̍쐬���J�n���ꂽ�� */
	case WM_CREATE:
		// WM_CREATE�̃u���b�N
	{
		// �|�C���^�̏�����
		LPCREATESTRUCT lpCreateStruct = NULL;	// CREATESTRUCT�\���̂ւ̃|�C���^lpCreateStruct��NULL

		/* lParam����lpCreateStruct�����o�� */
		lpCreateStruct = (LPCREATESTRUCT)lParam;	// lParam��(LPCREATESTRUCT)�ɃL���X�g
		if (lpCreateStruct->lpCreateParams != NULL) {	// NULL�łȂ��ꍇ
			// lpCreateStruct->lpCreateParams��(CWinBase *)�ɃL���X�g��pWindow�ɓ����
			pWindow = (CWinBase *)lpCreateStruct->lpCreateParams;
			// CWinBase::m_mapWindowMap��hwnd��pWindow�̃y�A��o�^
		CWinBase::m_mapWindowMap.insert(std::pair < HWND, CWinBase *>(hwnd, pWindow));
		}
	}
	// ����̏����ֈȍ~
	break;
	/* ���� */
	default:
		// default�̃u���b�N
	{
		// hwnd�ŃE�B���h�E�I�u�W�F�N�g�|�C���^����������CpWindow�Ɋi�[
		if (CWinBase::m_mapWindowMap.find(hwnd) != CWinBase::m_mapWindowMap.end()) {	// hwnd��map�̍Ō�ɂɓo�^����Ă���ꍇ
			// pWindow��hwnd�ň�����CWinBase�I�u�W�F�N�g�|�C���^����
			pWindow = CWinBase::m_mapWindowMap[hwnd];
		}
	}
	// ����̏����ֈȍ~
	break;
	}

	/* �E�B���h�E�I�u�W�F�N�g���擾�ł��Ȃ��ꍇ */
	if (pWindow == NULL) {	// pWindow��NULL
							// DefWindowProc�ɔC����.
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	else {		// pWindow��������
		// ���̃E�B���h�E��DynamicWindowProc�ɓn��.
		return pWindow->DynamicWindowProc(hwnd, uMsg, wParam, lParam);	// pWindow->DynamiProc�ɓn��.
	}
}

/** @brief ���C���E�B���h�E�̃��j���[�o�[���C[�w���v(H)]->[�o�[�W�������(A)...]�ŌĂяo�����_�C�A���O�̐ݒ�
@note ���̊֐��́C���C���E�B���h�E�̃��j���[�o�[�ɂ���[�w���v(H)]->[�o�[�W�������(A)...]��I�������ۂɌĂяo�����_�C�A���O
�ɂ��ẴR�[���o�b�N�֐��D
@param hDlg		�E�B���h�E�ւ̃n���h��
@param message	���b�Z�[�W
@param wParam	�ǉ��̃��b�Z�[�W���Fw-�p�����[�^
@param lParam	�ǉ��̃��b�Z�[�W���Fl-�p�����[�^
@return	INT_PTR �֐��̏������ʂ̐���
@sa callback CWinBase UNREFERENCED_PARAMETER EndDialog
**/
INT_PTR CWinBase::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/** @brief ���C���E�B���h�E�̃��j���[�o�[���C[�ݒ�(O)]->[�ڍׂȐݒ�(S)]�ŌĂяo�����_�C�A���O�̐ݒ�
@note ���̊֐��́C���C���E�B���h�E�̃��j���[�o�[�ɂ���[�ݒ�(O)]->[�ڍׂȐݒ�(S)]��I�������ۂɌĂяo�����_�C�A���O
�ɂ��ẴR�[���o�b�N�֐��D
@param hDlg		�E�B���h�E�ւ̃n���h��
@param message	���b�Z�[�W
@param wParam	�ǉ��̃��b�Z�[�W���Fw-�p�����[�^
@param lParam	�ǉ��̃��b�Z�[�W���Fl-�p�����[�^
@return	INT_PTR �֐��̏������ʂ̐���
@sa callback CWinBase UNREFERENCED_PARAMETER SendDlgItemMessage EndDialog
**/
INT_PTR CWinBase::Setting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	//MyOutputDebugString(L"\n	WM_COMMAND�F%d���󂯎��܂���\n\n", message);
	TCHAR minHue[4], minSaturation[4], minBrightness[4], maxHue[4], maxSaturation[4], maxBrightness[4];
	static MyAdvancedSetting* localMASetting;
	switch (message)
	{
	case WM_INITDIALOG:
		localMASetting = (MyAdvancedSetting*)lParam;
		if (localMASetting)
		{
			MyOutputDebugString(L"\n////////////////////////////////////////////////////////////\n", message);
			SendDlgItemMessage(hDlg, IDC_SPINMINHUE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_EDITMINHUE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMINHUE, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MIN_HUE, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMAXHUE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMAXHUE, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MAX_HUE, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMINSATURATION, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMINSATURATION, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MIN_SATURATION, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMAXSATURATION, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMAXSATURATION, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MAX_SATURATION, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMINBRIGHTNESS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMINBRIGHTNESS, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MIN_BRIGHTNESS, 0));
			SendDlgItemMessage(hDlg, IDC_SPINMAXBRIGHTNESS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SPINMAXBRIGHTNESS, UDM_SETPOS, 0, (LPARAM)MAKELONG(localMASetting->TH_MAX_BRIGHTNESS, 0));
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_EDITMINHUE, minHue, sizeof(minHue));
			GetDlgItemText(hDlg, IDC_EDITMANHUE, maxHue, sizeof(maxHue));
			GetDlgItemText(hDlg, IDC_EDITMINSATURATION, minSaturation, sizeof(minSaturation));
			GetDlgItemText(hDlg, IDC_EDITMAXSATURATION, maxSaturation, sizeof(maxSaturation));
			GetDlgItemText(hDlg, IDC_EDITMINBRIGHTNESS, minBrightness, sizeof(minBrightness));
			GetDlgItemText(hDlg, IDC_EDITMAXBRIGHTNESS, maxBrightness, sizeof(maxBrightness));
			if (localMASetting)
				*localMASetting = { ::_ttoi(minHue), ::_ttoi(maxHue), ::_ttoi(minSaturation), ::_ttoi(maxSaturation), ::_ttoi(minBrightness), ::_ttoi(maxBrightness) };
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/** @brief �E�B���h�E���쐬����
@note ���̊֐��́C�����̃p�����[�^�ɏ]���ă��C���E�B���h�E���쐬����D
�e��R���g���[�����ɂ��Ă������ō쐬���s���D
@param lpctszClassName	�쐬����E�B���h�E�̃N���X��
@param lpctszWindowName	�쐬����E�B���h�E��
@param dwStyle			�E�B���h�E�̃X�^�C��
@param rect				�쐬����E�B���h�E�̃T�C�Y
@param hWndParent		�e�E�B���h�E�܂��̓I�[�i�[�E�B���h�E�ƂȂ�n���h�����w��
@param hMenu			���j���[�̃n���h��
@param hInstance		�C���X�^���X�n���h��
@return	BOOL �֐��̏������ʂ̐���
@sa CreateWindow SendMessage EnableWindow m_hWndViewTarget m_hwndSTATICPhase m_hwndTextBoxPhase m_hwndBUTTONPhase
**/
BOOL CWinBase::Create(LPCTSTR lpctszClassName, LPCTSTR lpctszWindowName, DWORD dwStyle, const RECT & rect, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance) {
	// �E�B���h�E���쐬����. http://wisdom.sakura.ne.jp/system/winapi/win32/win7.html
	m_hWnd = CreateWindow(lpctszClassName, lpctszWindowName, dwStyle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hWndParent, hMenu, hInstance, this);	// CreateWindow�Ŏw�肳�ꂽ�������g���ăE�B���h�E���쐬.
	if (m_hWnd == NULL) {	// NULL�Ȃ玸�s.
		// FALSE��Ԃ�.
		return FALSE;
	}
	// �摜�̃����_�����O����X�y�[�X���쐬����D
	m_hWndViewTarget = CreateWindow(L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP, 0, 0, rect.right - rect.left, 200, m_hWnd, NULL, hInstance, NULL);

	// �^�惂�[�h�̃O���[�v���쐬
	CreateWindow(L"BUTTON", L"�^�惂�[�h", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 210, 670, 75, m_hWnd, NULL, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_GetFolderPass] = 
		CreateWindow(L"STATIC", L"�ۑ���F", WS_CHILD | WS_VISIBLE, 15, 235, 65, 18, m_hWnd, (HMENU)CID_ST_GetFolderPass, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_FileName] = 
		CreateWindow(L"STATIC", L"�t�@�C�����F", WS_CHILD | WS_VISIBLE, 15, 260, 96, 18, m_hWnd, (HMENU)CID_ST_FileName, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_CaptureStart] = 
		CreateWindow(L"STATIC", L"�L���v�`���[�J�n�F", WS_CHILD | WS_VISIBLE, 275, 260, 145, 18, m_hWnd, (HMENU)CID_ST_CaptureStart, hInstance, NULL);
	m_hwndSTATICPhase[CID_ST_CaptureEnd] = 
		CreateWindow(L"STATIC", L"�L���v�`���[��~�F", WS_CHILD | WS_VISIBLE, 480, 260, 145, 18, m_hWnd, (HMENU)CID_ST_CaptureEnd, hInstance, NULL);
	// �e�L�X�g�{�b�N�X��ǉ�
	m_hwndTextBoxPhase[CID_TX_CapturePass] = 
		CreateWindow(L"EDIT", L"E:\\workspace", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE, 75, 235, 550, 20, m_hWnd, (HMENU)CID_TX_CapturePass, hInstance, NULL);
	m_hwndTextBoxPhase[CID_TX_FileName] = 
		CreateWindow(L"EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE, 115, 260, 150, 20, m_hWnd, (HMENU)CID_TX_FileName, hInstance, NULL);
	// �{�^����ǉ�
	m_hwndBUTTONPhase[CID_BT_GetFolderPass] = 
		CreateWindow(L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_ICON, 625, 235, 30, 20, m_hWnd, (HMENU)CID_BT_GetFolderPass, hInstance, NULL);
	SendMessage(m_hwndBUTTONPhase[CID_BT_GetFolderPass], BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 40, 40, LR_DEFAULTCOLOR));
	EnableWindow(m_hwndBUTTONPhase[CID_BT_GetFolderPass], TRUE);
	m_hwndBUTTONPhase[CID_BT_CaptureStart] = 
		CreateWindow(L"BUTTON", L"Start", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 420, 260, 50, 20, m_hWnd, (HMENU)CID_BT_CaptureStart, hInstance, NULL);
	EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureStart], TRUE);
	m_hwndBUTTONPhase[CID_BT_CaptureEnd] = 
		CreateWindow(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 625, 260, 50, 20, m_hWnd, (HMENU)CID_BT_CaptureEnd, hInstance, NULL);
	EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureEnd], FALSE);

	// ���O�̃O���[�v���쐬
	CreateWindow(L"BUTTON", L"Console", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 690, 210, 285, 325, m_hWnd, NULL, hInstance, NULL);
	// �e�L�X�g�{�b�N�X��ǉ�
	m_hwndTextBoxPhase[CID_TX_Console] =
		CreateWindow(L"EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE, 695, 235, 275, 295, m_hWnd, (HMENU)CID_TX_Console, hInstance, NULL);

	// �����Ȃ�TRUE.
	return TRUE;
}

/** @brief �E�B���h�E���쐬����
@note ���̊֐��́C�E�B���h�E���쐬����O�i�K�Ƃ��āC�A�v���P�[�V�����I�u�W�F�N�g�̃|�C���^����������Ă��邩���m�F����D
��������Ă����ꍇ�C�E�B���h�E�쐬�̏��������s����D
@param lpctszWindowName	�쐬����E�B���h�E��
@param rect				�쐬����E�B���h�E�̃T�C�Y
@return	BOOL �֐��̏������ʂ̐���
@sa Create
**/
BOOL CWinBase::Create(LPCTSTR lpctszWindowName, const RECT & rect) {

	// �A�v���P�[�V�����I�u�W�F�N�g�|�C���^�̃`�F�b�N
	if (m_pApp == NULL) {
		return FALSE;
	}

	// �f�t�H���g�̃E�B���h�E�쐬
	return CWinBase::Create(_T("CWinBase"), lpctszWindowName, WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME, rect, NULL, NULL, m_pApp->m_hInstance);	// �t���o�[�W������Create�ŃE�B���h�E�쐬.
}

/** @brief �{�v���O�����̃��C���v���O�����̕\����Ԃ�ύX����D
@note ���̊֐��́C�{�v���O�����̃��C���v���O�����̕\����Ԃ�ύX����D
@param nCmdShow	�E�B���h�E�̕\����Ԃ�\���萔(SW_SHOW�ŕ\��)
@return	BOOL �֐��̏������ʂ̐���
@sa ShowWindow
**/
BOOL CWinBase::ShowWindow(int nCmdShow) {
	MyOutputDebugString(L"	CWinBase::ShowWindow(int) ���\�b�h���Ăяo����܂����D\n");

	/* �E�B���h�E�\�� */
	return ::ShowWindow(m_hWnd, nCmdShow);	// WindowsAPI��ShowWindow��m_hWnd��\��.
}

/** @brief �E�B���h�E�I�u�W�F�N�g�p�̃E�B���h�E�v���V�[�W��
@note ���̊֐��́C�E�B���h�E�I�u�W�F�N�g��p�̃E�B���h�E�v���V�[�W���ł���CStaticWindowProc�ɂ���ČĂяo�����D
���C���E�B���h�E�Ɋւ���R���g���[���̃C�x���g�ɂ��Ă͂����ŋL�q����D
@
m hWnd		�E�B���h�E�ւ̃n���h��
@param uMsg		���b�Z�[�W
@param wParam	�ǉ��̃��b�Z�[�W���Fw-�p�����[�^
@param lParam	�ǉ��̃��b�Z�[�W���Fl-�p�����[�^
@return	LRESULT �G���[�R�[�h��Ԃ�
@sa Create
**/
LRESULT CWinBase::DynamicWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
		/* �E�B���h�E�̍쐬���J�n���ꂽ�� */
	case WM_CREATE:
		// WM_CREATE�u���b�N
	{
		// OnCreate�ɔC����.
		return OnCreate(hwnd, (LPCREATESTRUCT)lParam);	// hwnd��lParam��OnCreate�ɓn���Ă��Ƃ͔C����
		// ����̏���
		break; 
	}
	/* �E�B���h�E���j�����ꂽ�� */
	case WM_DESTROY:
		// WM_DESTROY�u���b�N
	{
		// OnDestroy�ɔC����.
		OnDestroy();	// OnDestroy���Ă�
		// ����̏���
		break;
	}
	case WM_CTLCOLORSTATIC:
	{
		HDC hDC = (HDC)wParam;
		HWND hCtrl = (HWND)lParam;

		if (hCtrl == GetDlgItem(m_hWnd, CID_ST_GetFolderPass)) // �X�^�e�B�b�N�E�B���h�E��ID
		{
			SetBkMode(hDC, OPAQUE);             // �w�i��h��Ԃ�
			SetTextColor(hDC, RGB(0, 0, 0));    // �e�L�X�g�̐F
			SetBkColor(hDC, RGB(255, 255, 255));   // �e�L�X�g��������Ă��镔���̃e�L�X�g�̔w�i�̐F
			return (LRESULT)m_hBrush_BkColor;   // �e�L�X�g��������Ă��Ȃ������̔w�i�̐F
		}
		else if (hCtrl == GetDlgItem(m_hWnd, CID_ST_FileName)) // �X�^�e�B�b�N�E�B���h�E��ID
		{
			SetBkMode(hDC, OPAQUE);             // �w�i��h��Ԃ�
			SetTextColor(hDC, RGB(0, 0, 0));    // �e�L�X�g�̐F
			SetBkColor(hDC, RGB(255, 255, 255));   // �e�L�X�g��������Ă��镔���̃e�L�X�g�̔w�i�̐F
			return (LRESULT)m_hBrush_BkColor;   // �e�L�X�g��������Ă��Ȃ������̔w�i�̐F
		}
		else if (hCtrl == GetDlgItem(m_hWnd, CID_ST_CaptureStart)) // �X�^�e�B�b�N�E�B���h�E��ID
		{
			SetBkMode(hDC, OPAQUE);             // �w�i��h��Ԃ�
			SetTextColor(hDC, RGB(0, 0, 0));    // �e�L�X�g�̐F
			SetBkColor(hDC, RGB(255, 255, 255));   // �e�L�X�g��������Ă��镔���̃e�L�X�g�̔w�i�̐F
			return (LRESULT)m_hBrush_BkColor;   // �e�L�X�g��������Ă��Ȃ������̔w�i�̐F
		}
		else if (hCtrl == GetDlgItem(m_hWnd, CID_ST_CaptureEnd)) // �X�^�e�B�b�N�E�B���h�E��ID
		{
			SetBkMode(hDC, OPAQUE);             // �w�i��h��Ԃ�
			SetTextColor(hDC, RGB(0, 0, 0));    // �e�L�X�g�̐F
			SetBkColor(hDC, RGB(255, 255, 255));   // �e�L�X�g��������Ă��镔���̃e�L�X�g�̔w�i�̐F
			return (LRESULT)m_hBrush_BkColor;   // �e�L�X�g��������Ă��Ȃ������̔w�i�̐F
		}

		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		MyOutputDebugString(L"\n	WM_COMMAND�F%d���󂯎��܂���\n\n", wmId);

		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(this->m_pApp->m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			break;
		case IDM_SETTING:
			DialogBoxParam(this->m_pApp->m_hInstance, MAKEINTRESOURCE(IDD_SETTINGBOX), hwnd, Setting, (LPARAM)&MASetting);
			hsv_min = cv::Scalar(MASetting.TH_MIN_HUE, MASetting.TH_MIN_SATURATION, MASetting.TH_MIN_BRIGHTNESS);
			hsv_max = cv::Scalar(MASetting.TH_MAX_HUE, MASetting.TH_MAX_SATURATION, MASetting.TH_MAX_BRIGHTNESS);
			MyOutputDebugString(L"MASetting.TH_MIN_HUE:%d\n", MASetting.TH_MIN_HUE);
			MyOutputDebugString(L"MASetting.TH_MAX_HUE:%d\n", MASetting.TH_MAX_HUE);
			MyOutputDebugString(L"MASetting.TH_MIN_SATURATION:%d\n", MASetting.TH_MIN_SATURATION);
			MyOutputDebugString(L"MASetting.TH_MAX_SATURATION:%d\n", MASetting.TH_MAX_SATURATION);
			MyOutputDebugString(L"MASetting.TH_MIN_BRIGHTNESS:%d\n", MASetting.TH_MIN_BRIGHTNESS);
			MyOutputDebugString(L"MASetting.TH_MAX_BRIGHTNESS:%d\n", MASetting.TH_MAX_BRIGHTNESS);
				break;
		case IDM_EXIT:
			DestroyWindow(hwnd);
			break;
		case CID_BT_CaptureStart:
		{
			TCHAR destPass[101], fileName[101];
			GetWindowText(m_hwndTextBoxPhase[CID_TX_CapturePass], destPass, sizeof(destPass));
			GetWindowText(m_hwndTextBoxPhase[CID_TX_FileName], fileName, sizeof(fileName));
			cv::String cvFilePass = WStringToString(lstrcat(lstrcat(lstrcat(destPass, TEXT("\\")), fileName),TEXT(".avi")));
			writer.open(cvFilePass, cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), m_pApp->fps, cv::Size(504, 504), true);
			if (!writer.isOpened()) { return -1; }

			setFlagCapture(true);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureStart], FALSE);
			EnableWindow(m_hwndBUTTONPhase[CID_TX_CapturePass], FALSE);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureEnd], TRUE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_CapturePass], FALSE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_FileName], FALSE);
			break;
		}
		case CID_BT_CaptureEnd:
		{
			writer.release();

			setFlagCapture(false);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureStart], TRUE);
			EnableWindow(m_hwndBUTTONPhase[CID_TX_CapturePass], TRUE);
			EnableWindow(m_hwndBUTTONPhase[CID_BT_CaptureEnd], FALSE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_CapturePass], TRUE);
			EnableWindow(m_hwndTextBoxPhase[CID_TX_FileName], TRUE);
			break; 
		}
		case CID_BT_GetFolderPass:
		{
			TCHAR dir[MAX_PATH];            //      �I�����ꂽ�t�H���_�[��
			TCHAR def_dir[MAX_PATH];        //      �����t�H���_�[
			_tcscpy_s(def_dir, sizeof(def_dir) / sizeof(TCHAR), _TEXT("C:"));
			if (GetDir(0, def_dir, dir) == TRUE) {
				MessageBox(0, dir, _TEXT("�I�����ꂽ�t�H���_�[��\n"), MB_OK);
			}
			SetWindowText(m_hwndTextBoxPhase[CID_TX_CapturePass], (LPCTSTR)dir);
			break;
		}
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		// ����̏���
		break;
	}
	/* ���̑� */
	default:
	{
		// ����̏���
		break;
	}
	}

	// DefWindowProc�ɔC����.
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/** @brief GetDir ���\�b�h�Őݒ肵���t�H���_�[���擾�_�C�A���O�̃R�[���o�b�N�֐�
@note ���̊֐��́CGetDir ���\�b�h�Őݒ肵���t�H���_�[���擾�_�C�A���O�p�̃R�[���o�b�N�֐��ł���D
@param hWnd		�E�B���h�E�ւ̃n���h��
@param uMsg		���b�Z�[�W
@param lParam	�ǉ��̃��b�Z�[�W���Fl-�p�����[�^
@param lpData	�t�H���_�I�����̏����f�B���N�g��
@return int ���������ꍇ��0��Ԃ�
@sa http://yamatyuu.net/computer/program/sdk/common_dialog/SHBrowseForFolder/index.html
**/
int CWinBase::BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	TCHAR dir[MAX_PATH];
	ITEMIDLIST *lpid;
	HWND hEdit;

	switch (uMsg) {
	case BFFM_INITIALIZED:  //      �_�C�A���O�{�b�N�X��������
		SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);     //      �R�����_�C�A���O�̏����f�B���N�g��
		break;
	case BFFM_VALIDATEFAILED:       //      �����ȃt�H���_�[�������͂��ꂽ
		MessageBox(hWnd, (TCHAR*)lParam, _TEXT("�����ȃt�H���_�[�������͂���܂���"), MB_OK);
		hEdit = FindWindowEx(hWnd, NULL, _TEXT("EDIT"), NULL);     //      �G�f�B�b�g�{�b�N�X�̃n���h�����擾����
		SetWindowText(hEdit, _TEXT(""));
		return 1;       //      �_�C�A���O�{�b�N�X����Ȃ�
		break;
	case BFFM_IUNKNOWN:
		break;
	case BFFM_SELCHANGED:   //      �I���t�H���_�[���ω������ꍇ
		lpid = (ITEMIDLIST *)lParam;
		SHGetPathFromIDList(lpid, dir);
		hEdit = FindWindowEx(hWnd, NULL, _TEXT("EDIT"), NULL);     //      �G�f�B�b�g�{�b�N�X�̃n���h�����擾����
		SetWindowText(hEdit, dir);
		break;
	}
	return 0;
}
