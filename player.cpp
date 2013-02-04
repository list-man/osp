#include "stdafx.h"
#include "player.h"

//////////////////////////////////////////////////////////////////////////
//public member functions.
CPlayer::CPlayer() : m_graphManager(NULL)
{
}

CPlayer::~CPlayer()
{

}

//////////////////////////////////////////////////////////////////////////
//message handler.
LRESULT CPlayer::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CreateGraphManager();

	return 0;
}

LRESULT CPlayer::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CPlayer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CPlayer::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);

	CRect rcClient;
	GetClientRect(&rcClient);
	dc.FillSolidRect(rcClient, WHITE_BRUSH);
	return 0;
}

LRESULT CPlayer::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//IWidget Implementations.
HWND STDMETHODCALLTYPE CPlayer::Create(HWND hParent, UINT nId)
{
	return CWindowImpl<CPlayer>::Create(hParent, NULL, NULL, WS_VISIBLE|WS_CHILD, 0, nId);
}

BOOL STDMETHODCALLTYPE CPlayer::Destroy(void)
{
	return DestroyWindow();
}

HWND STDMETHODCALLTYPE CPlayer::GetHWnd(void)
{
	if (m_hWnd) {
		return m_hWnd;
	}
	
	return NULL;
}

STDMETHODIMP CPlayer::play(BSTR aUrl)
{
	CheckPointer(aUrl, E_UNEXPECTED);

	if (m_graphManager)
		m_graphManager->RenderGraph(aUrl);

	return S_OK;
}

STDMETHODIMP CPlayer::pause(void)
{
	if (m_graphManager)
	{
		return m_graphManager->PauseGraph();
	}

	return E_UNEXPECTED;
}

STDMETHODIMP CPlayer::resume(void)
{
	if (m_graphManager)
	{
		return m_graphManager->PlayGraph();
	}

	return E_UNEXPECTED;
}

STDMETHODIMP CPlayer::stop(void)
{
	if (m_graphManager)
	{
		return m_graphManager->StopGraph();
	}

	return E_UNEXPECTED;
}

//////////////////////////////////////////////////////////////////////////
//protected member functions.
HRESULT CPlayer::CreateGraphManager()
{
	HRESULT hRes = S_OK;

	if (!m_graphManager)
	{
		CComObject<COSPGraphManager>* graphMgr = NULL;
		if (FAILED(hRes = CComObject<COSPGraphManager>::CreateInstance(&graphMgr)))
		{
			return hRes;
		}

		graphMgr->AddRef();
		m_graphManager = graphMgr;
	}

	return hRes;
}

HRESULT CPlayer::InitGraphManager()
{
	if (m_graphManager)
	{
		m_graphManager->SetVideoWindow(m_hWnd);

		return S_OK;
	}

	return E_UNEXPECTED;
}

