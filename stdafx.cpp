// stdafx.cpp : source file that includes just the standard includes
// DirectShow Example.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "utility.h"
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

VOID _OutputDebug(LPCTSTR lpszErrmsg, ...)
{
	CString	strMsg;

	va_list args;
	va_start(args, lpszErrmsg);
	strMsg.FormatV(lpszErrmsg, args);
	va_end(args);

	strMsg = _T("[PPLive chctrl]") + strMsg + _T("\r\n");

	strMsg.Replace(_T("\r"),_T(" "));
	strMsg.Replace(_T("\n"),_T(" "));
	::OutputDebugString(strMsg);
}
