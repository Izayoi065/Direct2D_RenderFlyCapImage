#pragma once
#include "CWinBase.h"

/* �O���錾 */
class C_WinBase;
class CApplication;
class FlyCap2CVWrapper;

class CViewDirect2D :
	public CWinBase
{
private:
	BOOL g_InitD2D = false;		// 2�d�������h�~
	ID2D1Factory * m_pD2d1Factory;					// 
	ID2D1HwndRenderTarget * m_pRenderTarget;		// �����_�����O�̃^�[�Q�b�g
	IDWriteTextFormat* pTextFormat;					// �e�L�X�g�̏����ݒ�Ɏg�p����t�H���g �v���p�e�C�i���v���p�e�B�y�у��P�[�����
	IDWriteFactory* pDWFactory;						// �S�Ă�DirectWrite�I�u�W�F�N�g�̃��[�g�E�t�@�N�g���E�C���^�[�t�F�[�X
	ID2D1SolidColorBrush* pBrush;
	ID2D1Bitmap * pBitmap;
	std::wstring strText = L"";
	const unsigned size = 504U;	// Bitmap�̃T�C�Y
	double totalTime = 0;
public:
	FlyCap2CVWrapper* FlyCap;
public:
	CViewDirect2D(CApplication *pApp);
	~CViewDirect2D();
	HRESULT InitDirect2D(HINSTANCE hInstance, LPTSTR lpCmdLine, int nShowCmd);
	void ReleaseD2D();
	HRESULT AppIdle(cv::InputArray image_, double fps);
	HRESULT	Render(cv::InputArray image_, double fps);

	/* ���z�֐� */
	virtual int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);	// �E�B���h�E�쐬���̃n���h��OnCreate
	virtual void OnDestroy();	// �E�B���h�E�j�����̃n���h�� Ondestroy
};

