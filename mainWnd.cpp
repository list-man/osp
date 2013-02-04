#include "stdafx.h"
#include <ShlObj.h>
#include "mainWnd.h"
#include "resource.h"

CMainWnd::CMainWnd()
{
	m_bAllowDrop = false;
	GetSPlayer(NULL);
}

CMainWnd::~CMainWnd()
{

}

//////////////////////////////////////////////////////////////////////////
//message handlers.
LRESULT CMainWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	InitDropTarget();

	if (m_splayer) {
		CComQIPtr<IWidget> spWidget = m_splayer;
		if (spWidget) {
			spWidget->Create(m_hWnd, WID_PLAYER);
		}

		ResizePlayer();
	}

	return 0;
}

LRESULT CMainWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DestroyWindow();
	return 0;
}

LRESULT CMainWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UnInitDropTarget();
	PostQuitMessage(0);
	return 0;
}

LRESULT CMainWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ResizePlayer();
	return 0;
}

LRESULT CMainWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//Implementations of IDropTarget.
STDMETHODIMP CMainWnd::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if (!m_spDropTargetHelper)
	{
		CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, __uuidof(IDropTargetHelper), (LPVOID*)&m_spDropTargetHelper);
	}

	if (m_spDropTargetHelper)
		m_spDropTargetHelper->DragEnter(m_hWnd, pDataObj, (LPPOINT)&pt, *pdwEffect);

	UINT nSize = m_dropImpl.GetSupportNum();
	for (UINT i=0; i<nSize; i++)
	{
		const FORMATETC* pfmt = m_dropImpl.GetFORMATETC(i);
		if (pfmt)
		{
			if (SUCCEEDED(pDataObj->QueryGetData(const_cast<FORMATETC*>(m_dropImpl.GetFORMATETC(i)))))
			{
				m_bAllowDrop = true;
				break;
			}
		}
	}

	m_dropImpl.QueryDrop(grfKeyState, pdwEffect, m_bAllowDrop);

	return S_OK;
}

STDMETHODIMP CMainWnd::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if(m_spDropTargetHelper)
		m_spDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);

	m_dropImpl.QueryDrop(grfKeyState, pdwEffect, m_bAllowDrop);

	return S_OK;
}

STDMETHODIMP CMainWnd::DragLeave(void)
{
	if (m_spDropTargetHelper)
		m_spDropTargetHelper->DragLeave();

	m_bAllowDrop = false;

	return S_OK;
}

STDMETHODIMP CMainWnd::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if(m_spDropTargetHelper)
		m_spDropTargetHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);

	if (pDataObj && m_bAllowDrop)
	{
		FORMATETC fmtetc = { RegisterClipboardFormat(CFSTR_FILENAMEW), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		STGMEDIUM medium = { TYMED_HGLOBAL };
		if (S_OK == pDataObj->GetData(&fmtetc, &medium))
		{
			LPVOID pBytes = GlobalLock(medium.hGlobal);
			wstring fileName = (wchar_t*)pBytes;
			GlobalUnlock(medium.hGlobal);

			if (PathFileExists(fileName.c_str()))
			{
				CComPtr<IPlayer> player;
				if (SUCCEEDED(GetSPlayer(&player)) && player)
				{
					player->play(CComBSTR(fileName.c_str()));
				}
			}
		}
	}

	m_bAllowDrop = false;
	m_dropImpl.QueryDrop(grfKeyState, pdwEffect, m_bAllowDrop);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//protected member functions.
void CMainWnd::InitDropTarget()
{
	RegisterDragDrop(m_hWnd, static_cast<IDropTarget*>(this));

	FORMATETC fmtetcA = { RegisterClipboardFormat(CFSTR_FILENAMEA), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	FORMATETC fmtetcW = { RegisterClipboardFormat(CFSTR_FILENAMEW), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	m_dropImpl.AddSupportFORMATETC(fmtetcA);
	m_dropImpl.AddSupportFORMATETC(fmtetcW);
}

void CMainWnd::UnInitDropTarget()
{
	RevokeDragDrop(m_hWnd);
}

HRESULT CMainWnd::GetSPlayer(IPlayer** ppv)
{
	if (!m_splayer) {
		CPlayer::CreateInstance(&m_splayer);
	}

	if (m_splayer && ppv) {
		return m_splayer.CopyTo(ppv);
	}

	return E_FAIL;
}

void CMainWnd::ResizePlayer()
{
	CComQIPtr<IWidget> spWidget = m_splayer;
	if (spWidget) {
		CRect rcClient;
		GetClientRect(&rcClient);

		HWND hWnd = spWidget->GetHWnd();
		if (hWnd) {
			::MoveWindow(hWnd, 0, 0, rcClient.Width(), rcClient.Height()-50, TRUE);
		}
	}
}
