#include "stdafx.h"
#include "player.h"

extern Filter* g_filters[];

//////////////////////////////////////////////////////////////////////////
//COSPServiceMgr
COSPServiceMgr::COSPServiceMgr()
{
	InitFilters();
}

COSPServiceMgr::~COSPServiceMgr()
{

}

//////////////////////////////////////////////////////////////////////////
// IOSPServiceMgr
STDMETHODIMP COSPServiceMgr::RenderUrl(IGraphBuilder *pGb, LPCWSTR aFile, IOSPGraphBuilderCallback *aCallback)
{
	return E_NOTIMPL;
}

STDMETHODIMP COSPServiceMgr::RenderFilter(IGraphBuilder *pGb, IBaseFilter *aFilter, IOSPGraphBuilderCallback *aCallback)
{
	return E_NOTIMPL;
}

STDMETHODIMP COSPServiceMgr::RenderPin(IGraphBuilder *pGb, IPin *aPintout, IOSPGraphBuilderCallback *aCallback)
{
	return E_NOTIMPL;
}

void COSPServiceMgr::InitFilters()
{
	Filter** filters = g_filters;

	while (*filters)
	{
		Filter* flt = *filters;
		m_allFilters.push_back(flt);

		int index = m_allFilters.size();

		if (FT_SRC == flt->filterType)
		{
			SourceFilter* srcFilter = (SourceFilter*)flt;
			m_sourceFilters.push_back(srcFilter);
			
			if (srcFilter->pszProtocol)
			{
				CAtlList<CString> ls;
				Explode(CString(srcFilter->pszProtocol), ls, L',');
				while (ls.GetCount() >= 2)
				{
					CString protocal = ls.RemoveHead();
					CString extension = ls.RemoveHead();

					std::map<wstring, long>& extSource = m_protocalSource[(LPCTSTR)protocal];
					extSource[(LPCTSTR)extension] = index;
				}
			}
			else if (srcFilter->pszExtension)
			{
				CAtlList<CString> ls;
				Explode(CString(srcFilter->pszExtension), ls, L',');
				while (ls.GetCount() >= 2)
				{
					CString extension = ls.RemoveHead();

					std::multimap<long, long, std::greater<long> >& source = m_extensionSource[(LPCTSTR)extension];
					source.insert(std::make_pair(srcFilter->filterBase.dwMerit, index));
				}
			}
		}
		else if (FT_ADEC == flt->filterType)
		{
			m_audioTransform.insert(std::make_pair(flt->dwMerit, index));
		}
		else if (FT_VDEC == flt->filterType)
		{
			m_videoTransform.insert(std::make_pair(flt->dwMerit, index));
		}
		else if (FT_OTHER == flt->filterType || FT_SPLT == flt->filterType)
		{
			m_otherTransform.insert(std::make_pair(flt->dwMerit, index));
		}

		filters++;
	}
}

//////////////////////////////////////////////////////////////////////////
//public member functions.
CPlayer::CPlayer() : m_graphManager(NULL)
{
	CreateGraphService();
}

CPlayer::~CPlayer()
{

}

HRESULT CPlayer::FinalConstruct()
{
	CreateGraphService();
	return S_OK;
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

HRESULT CPlayer::CreateGraphService()
{
	if (!m_spServiceMgr) {
		return COSPServiceMgr::CreateInstance(&m_spServiceMgr);
	}

	return S_OK;
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

