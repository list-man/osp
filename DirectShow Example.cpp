// DirectShow Example.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "utility.h"
#include "MessageLoopEx.h"
#include "mainWnd.h"
#include <ShObjIdl.h>
#include <ShlGuid.h>

//#define CLSID_MP4
CComModule _module;

//int _tmain(int argc, _TCHAR* argv[])
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	::OleInitialize(NULL);

	ULONG_PTR token;
	GdiplusStartupInput input;
	GdiplusStartupOutput output;
	GdiplusStartup(&token, &input, &output);

	CComObject<CMainWnd>* mainFrame = NULL;
	CComObject<CMainWnd>::CreateInstance(&mainFrame);

	int scrWidth = GetSystemMetrics(SM_CXSCREEN);
	int scrHeight = GetSystemMetrics(SM_CYSCREEN);
	mainFrame->Create(NULL, CRect(CPoint((scrWidth-500)>>1, (scrHeight-400)>>1), CSize(500, 400)), L"ddd", WS_VISIBLE|WS_CLIPSIBLINGS|WS_OVERLAPPEDWINDOW);
	mainFrame->AddRef();
	CMessageLoopEx msgloop;
	msgloop.Run();
	mainFrame->Release();
	GdiplusShutdown(token);
	::OleUninitialize();
	return 0;
}