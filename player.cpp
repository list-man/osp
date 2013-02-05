#include "stdafx.h"
#include "player.h"

extern Filter* g_filters[];

//////////////////////////////////////////////////////////////////////////
//COSPGraphBuilderPrivate.
COSPGraphBuilderPriavte::COSPGraphBuilderPriavte()
{
	OSVERSIONINFOEX os;
	memset(&os, 0, sizeof(OSVERSIONINFOEX));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	::GetVersionEx(&os);

	m_bIsVistaOrLater = os.dwMajorVersion>=6 ?  TRUE:FALSE;
}

COSPGraphBuilderPriavte::~COSPGraphBuilderPriavte()
{

}

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
	COSPGraphBuilderPriavte data;
	return DoRenderFile(pGb, aFile, aCallback, (void*)&data);
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

HRESULT COSPServiceMgr::DoAddSourceFilter(IGraphBuilder* pGb, LPCWSTR aFile, IOSPGraphBuilderCallback* aCallback, LPVOID aPrivate)
{
	if (!aFile) return E_INVALIDARG;

	URL_COMPONENTS url;
	memset(&url, 0, sizeof(URL_COMPONENTS));
	url.dwStructSize = sizeof(URL_COMPONENTS);
	url.dwSchemeLength = 1;
	url.dwUrlPathLength = 1;
	url.dwExtraInfoLength = 1;
	if (!InternetCrackUrl(aFile, 0, 0, &url))
	{
		memset(&url, 0, sizeof(URL_COMPONENTS));
		url.dwStructSize = sizeof(URL_COMPONENTS);
		url.nScheme = INTERNET_SCHEME_FILE;

		url.lpszUrlPath = (LPWSTR)aFile;
		url.dwUrlPathLength = wcslen(aFile);
	}

	HRESULT hRes = VFW_E_UNSUPPORTED_STREAM;
	CString strScheme(url.lpszScheme, url.dwSchemeLength);
	CString strExtension = CPathW(CString(url.lpszUrlPath, url.dwUrlPathLength)).GetExtension();
	strExtension.Trim(L'.');

	if (strScheme.GetLength() > 1 && strScheme.CompareNoCase(L"file") != 0)
	{

	}
	else	//local file.
	{
		std::map<wstring, std::multimap<long, long, std::greater<long> > >::iterator itr = m_extensionSource.find((LPCTSTR)strExtension);
		if (itr != m_extensionSource.end())
		{
			std::multimap<long, long, std::greater<long> >::iterator itr2 = itr->second.begin();
			for (; itr2 != itr->second.end(); itr2++)
			{
				if (SUCCEEDED(hRes = DoAddSourceFilter(pGb, aFile, m_allFilters[itr2->second], aCallback, aPrivate)))
				{
					return hRes;
				}
			}
		}

		HANDLE hFile = ::CreateFile(aFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return VFW_E_NOT_FOUND;
		}

		{
			vector<SourceFilter*>::iterator itr = m_sourceFilters.begin();
			for (; itr != m_sourceFilters.end(); itr++)
			{
				if (CheckBytes(*itr, hFile))
				{
					if (SUCCEEDED(hRes = DoAddSourceFilter(pGb, aFile, &(*itr)->filterBase, aCallback, aPrivate)))
					{
						CloseHandle(hFile);
						return hRes;
					}
				}
			}

			CloseHandle(hFile);
		}
	}

	return hRes;
}

HRESULT COSPServiceMgr::DoRenderFile(IGraphBuilder* pGb, LPCWSTR aFile, IOSPGraphBuilderCallback* aCallback, LPVOID aPrivate)
{
	COSPGraphBuilderPriavte* data = static_cast<COSPGraphBuilderPriavte*>(aPrivate);

	HRESULT hRes = DoAddSourceFilter(pGb, aFile, aCallback, aPrivate);
	if (SUCCEEDED(hRes))
	{
		if (SUCCEEDED(DoConnectFilter(pGb, data->m_sourceFilter, NULL, aCallback, aPrivate)))
		{

		}
	}
	
	return S_OK;
}

HRESULT COSPServiceMgr::DoRender(IGraphBuilder* pGb, IPin* aPinout, IOSPGraphBuilderCallback* aCallback, LPVOID aPrivate)
{
	return E_NOTIMPL;
}

HRESULT COSPServiceMgr::DoAddSourceFilter(IGraphBuilder* pGb, LPCWSTR aFile, Filter* aFilterData, IOSPGraphBuilderCallback* aCallback, LPVOID aPrivate)
{
	
	return E_NOTIMPL;
}

HRESULT COSPServiceMgr::DoConnectFilter(IGraphBuilder* pGb, IBaseFilter* aFilter, IPin* aPinin, IOSPGraphBuilderCallback* aCallback, LPVOID aPrivate)
{

	return E_NOTIMPL;
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

