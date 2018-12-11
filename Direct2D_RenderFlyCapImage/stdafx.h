// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//
#pragma once
/* 追加ライブラリへのリンク */
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
#    define MyOutputDebugString( str, ... ) // 空実装
#endif
#define Def_PI				3.1415f
#define Def_FOV (Def_PI*0.85f)
#define FloatToIndex(f4_tPos,f4_tBase,f4_tPitch) (((f4_tPos) - (f4_tBase))/(f4_tPitch))
#define IndexToFloat(f4_tIndex,f4_tBase,f4_tPitch) ((f4_tIndex)*(f4_tPitch) + (f4_tPitch)/2.0f + (f4_tBase))

#include "targetver.h"

#define STRICT					// 型チェックを厳密に行なう
#define WIN32_LEAN_AND_MEAN		// ヘッダーからあまり使われない関数を省く
#define WINVER        0x0600	// Windows Vista以降対応アプリを指定(なくてもよい)
#define _WIN32_WINNT  0x0600	// 同上
// Windows ヘッダー ファイル:
#include <windows.h>

// C ランタイム ヘッダー ファイル
#include <stdio.h>
#include <tchar.h>		// TCHAR型
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

// TODO: プログラムに必要な追加ヘッダーをここで参照してください
/* DirectX11関連 */
#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
//#include <DirectXMath.h>

/* OpenCV関連 */
#include "opencv2/core.hpp"
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"

/* FlyCapture2関連 */
#include <FlyCapture2.h>
#include <FlyCapture2GUI.h>