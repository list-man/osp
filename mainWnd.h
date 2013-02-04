#pragma once

#include "stdafx.h"
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include "DropTargetImpl.h"
#include "player.h"

class CMainWnd : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public CComCoClass<CMainWnd>,
		public CWindowImpl<CMainWnd>,
		public IDropTarget
{
public:
	CMainWnd();
	~CMainWnd();

	BEGIN_COM_MAP(CMainWnd)
		COM_INTERFACE_ENTRY(IDropTarget)
	END_COM_MAP()

	BEGIN_MSG_MAP(CMainWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

//IDropTarget.
public:
	STDMETHOD(DragEnter)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	STDMETHOD(DragLeave)(void);
	STDMETHOD(Drop)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

//message handler.
public:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
	void InitDropTarget();
	void UnInitDropTarget();
	HRESULT GetSPlayer(IPlayer** ppv);
	void ResizePlayer();
protected:
	CComPtr<IPlayer>	m_splayer;
	CDropTargetImpl		m_dropImpl;
	CComPtr<IDropTargetHelper>	m_spDropTargetHelper;
	bool				m_bAllowDrop;
};
