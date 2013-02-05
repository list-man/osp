// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WINVER		0x0400
#define _WIN32_IE	0x0500
#define _RICHEDIT_VER	0x0100
#define _ATL_OLEDB_CONFORMANCE_TESTS

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _WTL_NO_CSTRING

#include "targetver.h"

#include <atlbase.h>
using namespace ATL;
#include <atlcom.h>
#include <atlctl.h>

#include <atlwin.h>
#include <atltypes.h>
#include <ATLComTime.h>
#include <atltypes.h>
#include <atlfile.h>
#include <atlcoll.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlgdi.h>
#include <atlctrlx.h>
#include <atlframe.h>
#include <atlpath.h>

//DirectShow headers.
#include <DShow.h>
#include <uuids.h>
#include <evr.h>

#include <mtype.h>
//#include <DSUtil.h>
//#include <Streams.h>
//#include <Mtype.h>

#include <Uxtheme.h>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <xstring>
#include <GdiPlus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
using namespace std;

#include <WinInet.h>
#pragma comment(lib, "Wininet.lib")

#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Winmm.lib")

#ifdef DEBUG
#pragma comment(lib, "strmbasd.lib")
#else
#pragma comment(lib, "strmbase.lib")
#endif

#include <wxdebug.h>
#include <wxutil.h>
#include "resource.h"
#include "player_error.h"

#define MY_DEBUG	_OutputDebug
VOID	_OutputDebug(LPCTSTR lpszErrmsg, ...);

// TODO: reference additional headers your program requires here
