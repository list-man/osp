#include "stdafx.h"
#include "OSPGraphManager.h"

COSPGraphManager::COSPGraphManager()
{
	m_hVRWnd = NULL;
}

COSPGraphManager::~COSPGraphManager()
{

}

HRESULT COSPGraphManager::FinalConstruct()
{
	return S_OK;
}

void COSPGraphManager::FinalRelease()
{

}

STDMETHODIMP COSPGraphManager::NotifyEvent(ULONG_PTR punkGraph, long aEventCode, LONG_PTR wEventParam, LONG_PTR lEventParam)
{
	return E_NOTIMPL;
}

STDMETHODIMP COSPGraphManager::NotifyError(ULONG_PTR punkGraph, long aErrCode, long param)
{
	return E_NOTIMPL;
}

STDMETHODIMP COSPGraphManager::NotifyGraphStateChange(ULONG_PTR punkGraph, long aState)
{
	return E_NOTIMPL;
}

STDMETHODIMP COSPGraphManager::NotifyStreamEnd(LONG_PTR punkGraph)
{
	return E_NOTIMPL;
}

STDMETHODIMP COSPGraphManager::NotifyVolumnChange(LONG_PTR punkGraph, long aVolumn, BOOL aMute)
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
//public member functions.
void COSPGraphManager::SetVideoWindow(HWND hWnd)
{
	m_hVRWnd = hWnd;
}

void COSPGraphManager::SetOSPServiceMgr(IOSPServiceMgr* aServiceMgr)
{
	if (m_spServiceMgr) {
		m_spServiceMgr.Release();
	}

	if (aServiceMgr) {
		aServiceMgr->QueryInterface(__uuidof(IOSPServiceMgr), (void**)&m_spServiceMgr);
	}
}

HRESULT COSPGraphManager::RenderGraph(LPCWSTR aUrl)
{
	if (!m_ospGraph)
	{
		COSPGraph::CreateInstance(&m_ospGraph);
		m_ospGraph->Init(static_cast<IOSPGraphEventHandler*>(this), m_spServiceMgr);
	}

	if (m_ospGraph)
	{
		m_ospGraph->Render(CComBSTR(aUrl));
	}

	return S_OK;
}

HRESULT COSPGraphManager::PlayGraph()
{
	CComPtr<IOSPGraph> spOSPGraph;
	if (SUCCEEDED(GetActiveGraph(&spOSPGraph)) && spOSPGraph)
	{
		return spOSPGraph->Play();
	}

	return E_UNEXPECTED;
}

HRESULT COSPGraphManager::PauseGraph()
{
	CComPtr<IOSPGraph> spOSPGraph;
	if (SUCCEEDED(GetActiveGraph(&spOSPGraph)) && spOSPGraph)
	{
		return spOSPGraph->Pause();
	}

	return E_UNEXPECTED;
}

HRESULT COSPGraphManager::StopGraph()
{
	CComPtr<IOSPGraph> spOSPGraph;
	if (SUCCEEDED(GetActiveGraph(&spOSPGraph)) && spOSPGraph)
	{
		return spOSPGraph->Stop();
	}

	return E_UNEXPECTED;
}

//////////////////////////////////////////////////////////////////////////
//protected member functions.
HRESULT COSPGraphManager::GetActiveGraph(IOSPGraph** ppG)
{
	if (m_ospGraph)
	{
		return m_ospGraph.CopyTo(ppG);
	}

	return E_NOINTERFACE;
}

HRESULT COSPGraphManager::InitOSPGraph()
{
	if (!m_ospGraph)
	{
		COSPGraph::CreateInstance(&m_ospGraph);
	}

	if (m_ospGraph)
	{
		
	}

	return S_OK;
}