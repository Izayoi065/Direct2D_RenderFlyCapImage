// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//
#pragma once
/* �ǉ����C�u�����ւ̃����N */
#ifdef _DEBUG
#define DX_EXT "d.lib"
#define CV_EXT "d.lib"
#define FC2_EXT ".lib"
#else
#define DX_EXT ".lib"
#define CV_EXT ".lib"
#define FC2_EXT ".lib"
#endif
#pragma comment( lib, "winmm.lib")
#pragma comment( lib, "d3d11.lib")
#pragma comment( lib, "d2d1.lib")
#pragma comment( lib, "dwrite.lib" )
#pragma comment( lib, "opencv_world340" CV_EXT)
#pragma comment( lib, "FlyCapture2" FC2_EXT)
#pragma comment( lib, "FlyCapture2GUI" FC2_EXT)

#ifdef _DEBUG
#   define MyOutputDebugString( str, ... ) \
	{ \
        TCHAR c[256]; \
        _stprintf_s( c, str, __VA_ARGS__ ); \
        OutputDebugString( c ); \
      }
#else
#    define MyOutputDebugString( str, ... ) // �����
#endif
#define Def_PI				3.1415f
#define Def_FOV (Def_PI*0.85f)
#define FloatToIndex(f4_tPos,f4_tBase,f4_tPitch) (((f4_tPos) - (f4_tBase))/(f4_tPitch))
#define IndexToFloat(f4_tIndex,f4_tBase,f4_tPitch) ((f4_tIndex)*(f4_tPitch) + (f4_tPitch)/2.0f + (f4_tBase))

#include "targetver.h"

#define STRICT					// �^�`�F�b�N�������ɍs�Ȃ�
#define WIN32_LEAN_AND_MEAN		// �w�b�_�[���炠�܂�g���Ȃ��֐����Ȃ�
#define WINVER        0x0600	// Windows Vista�ȍ~�Ή��A�v�����w��(�Ȃ��Ă��悢)
#define _WIN32_WINNT  0x0600	// ����
// Windows �w�b�_�[ �t�@�C��:
#include <windows.h>

// C �����^�C�� �w�b�_�[ �t�@�C��
#include <stdio.h>
#include <tchar.h>		// TCHAR�^
#include <string>		// std::string
#include <map>			// std::map
#include "atltypes.h"	// CRect
#include <atlstr.h> 
#include <shlobj.h>
#include <xnamath.h>
#include <chrono>
#include <iostream>
#include <sstream>
#include <mmsystem.h>

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă�������
/* DirectX11�֘A */
#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
//#include <DirectXMath.h>

/* OpenCV�֘A */
#include "opencv2/core.hpp"
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"

/* FlyCapture2�֘A */
#include <FlyCapture2.h>
#include <FlyCapture2GUI.h>