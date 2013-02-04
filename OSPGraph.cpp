#include "stdafx.h"
#include <wxutil.h>
#include "OSPGraph.h"

COSPBackgroundRequest::COSPBackgroundRequest()
{
	m_hasRequest = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_requestResult = S_OK;
}

COSPBackgroundRequest::~COSPBackgroundRequest()
{

}

void COSPBackgroundRequest::SetFileUrl(LPCWSTR lpcwStr)
{
	CAutoLock autoLock(&m_cs);
	m_url = lpcwStr;
}

void COSPBackgroundRequest::AppendRequest(COMMAND_ID aCmd)
{
	CAutoLock autoLock(&m_cs);
	m_cmds.push(aCmd);

	SetEvent(m_hasRequest);
}

COMMAND_ID COSPBackgroundRequest::RemoveRequest()
{
	COMMAND_ID res = 0;

	CAutoLock autoLock(&m_cs);
	if (m_cmds.size() > 0)
	{
		res = m_cmds.front();
		m_cmds.pop();
	}
	else
	{
		ResetEvent(m_hasRequest);
	}

	return res;
}

DWORD COSPBackgroundRequest::WaitForAllowingMessage(DWORD dwMilliseconds)
{
	return WaitForMultipleObjects(1, &m_hasRequest, TRUE, dwMilliseconds);
}

HRESULT COSPBackgroundRequest::ProcessRequest(COSPGraph* pGraph)
{
	COMMAND_ID cmd = RemoveRequest();
	switch (cmd)
	{
	case RID_GRAPHCREATION:
		pGraph->CreateFilterGraph(&m_spGraphBuilder);
		break;
	case RID_GRAPHDESTRUCTION:
		break;
	case RID_ASYNCOPEN:
		{
			pGraph->DoNotifyGraphStateChange(sgsOpening);
			m_requestResult = pGraph->DoRender(m_spGraphBuilder, m_url.c_str());
			if (SUCCEEDED(m_requestResult))
			{
				pGraph->DoNotifyGraphStateChange(sgsOpen);
			}
		}
		break;
	case RID_CLOSE:
		{
			pGraph->DoNotifyGraphStateChange(sgsClosing);
			m_requestResult = m_requestResult = pGraph->InternelClose();
			if (SUCCEEDED(m_requestResult))
			{
				pGraph->DoNotifyGraphStateChange(sgsClose);
			}
		}
		break;
	case RID_PLAY:
		{
			pGraph->DoNotifyGraphStateChange(sgsRunning);
			m_requestResult = pGraph->InternelPlay();
			if (SUCCEEDED(m_requestResult))
			{
				pGraph->DoNotifyGraphStateChange(sgsRun);
			}
		}
		break;
	case RID_PAUSE:
		{
			pGraph->DoNotifyGraphStateChange(sgsPausing);
			m_requestResult = pGraph->InternelPause();
			if (SUCCEEDED(m_requestResult))
			{
				pGraph->DoNotifyGraphStateChange(sgsPause);
			}
		}
		break;
	case RID_STOP:
		{
			pGraph->DoNotifyGraphStateChange(sgsStopping);
			m_requestResult = pGraph->InternelStop();
			if (SUCCEEDED(m_requestResult))
			{
				pGraph->DoNotifyGraphStateChange(sgsStop);
			}
		}
		break;
	default:
		break;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//Class COSPGraph.
COSPGraph::COSPGraph()
{

}

COSPGraph::~COSPGraph()
{

}

HRESULT COSPGraph::FinalConstruct()
{
	return S_OK;
}

void COSPGraph::FinalRelease()
{

}

//////////////////////////////////////////////////////////////////////////
//public member functions.
HRESULT COSPGraph::CreateFilterGraph(IGraphBuilder** ppGb)
{
	HRESULT hRes = S_OK;
	if (FAILED(CoCreateInstance(CLSID_FilterGraphNoThread, NULL, CLSCTX_INPROC_SERVER, __uuidof(IFilterGraph), (void**)ppGb)))
	{
		hRes = OSP_E_CREATE_FILTERGRAPH;
	}

	return hRes;
}

//////////////////////////////////////////////////////////////////////////
//protected member functions.
void COSPGraph::DoNotifyGraphStateChange(OSPGraphState gs, BOOL bNotify/*=TRUE*/)
{

}

HRESULT COSPGraph::DoRender(IFilterGraph* pGb, LPCWSTR aUrl)
{
	CheckPointer(aUrl, E_UNEXPECTED);

	HRESULT hRes = S_OK;

	{
		CAutoLock autoLock(&m_cs);
		m_url = aUrl;
	}

	CString strUrl = aUrl;
	int idx = strUrl.Find(L":\\");
	if (-1 != idx)
		idx = strUrl.Find(L":/");

	if (-1 != idx)
	{
		CString strPrefix = strUrl.Left(idx);
		if (strPrefix.GetLength()==1 && strPrefix.GetAt(0)>=L'A' && strPrefix.GetAt(0)<=L'z')
		{
			return DoRenderLocal(pGb, aUrl);
		}
	}
	
	return DoRenderRemote(pGb, aUrl);
}

HRESULT COSPGraph::DoRenderLocal(IFilterGraph* pGb, LPCWSTR aUrl)
{

	return S_OK;
}

HRESULT COSPGraph::DoRenderRemote(IFilterGraph* pGb, LPCWSTR aUrl)
{
	return E_NOTIMPL;
}

HRESULT COSPGraph::InternelPlay()
{
	return E_NOTIMPL;
}

HRESULT COSPGraph::InternelPause()
{
	return E_NOTIMPL;
}

HRESULT COSPGraph::InternelStop()
{
	return E_NOTIMPL;
}

HRESULT COSPGraph::InternelClose()
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
//public member functions.
unsigned int WINAPI COSPGraph::BackgroundHelper(LPVOID aParam)
{
	COSPGraph* pGraph = static_cast<COSPGraph*>(aParam);
	COSPBackgroundRequest* backgroundRequest = &pGraph->m_request;
	while (true)
	{
		DWORD dwResult = backgroundRequest->WaitForAllowingMessage(500);
		if (WAIT_OBJECT_0 == dwResult)
		{
			backgroundRequest->ProcessRequest(pGraph);
		}
		else
		{

		}

		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.hwnd == NULL)// && msg.message == WM_RESET_DEVICE)
			{
// 					CComQIPtr<ISubPicAllocatorPresenter> spCAP = pThis->m_spVideoRenderer;
// 					if (spCAP)
// 					{
// 						spCAP->ResetDevice();
// 					}
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return 0;
}

STDMETHODIMP COSPGraph::Init(void)
{
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, BackgroundHelper, (LPVOID)this, 0, NULL);
	CloseHandle(hThread);

	return S_OK;
}

STDMETHODIMP COSPGraph::Render(BSTR aUrl)
{
	{
		CAutoLock autoLock(&m_cs);
		m_url = aUrl;
	}
	
	m_request.SetFileUrl(aUrl);
	m_request.AppendRequest(RID_ASYNCOPEN);

	return S_OK;
}

STDMETHODIMP COSPGraph::get_Url(BSTR *ppUrl)
{
	CheckPointer(ppUrl, E_POINTER);

	CAutoLock autoLock(&m_cs);
	*ppUrl = ::SysAllocString(m_url.c_str());

	return S_OK;
}

STDMETHODIMP COSPGraph::Play(void)
{
	m_request.AppendRequest(RID_PLAY);

	return S_OK;
}

STDMETHODIMP COSPGraph::Pause(void)
{
	m_request.AppendRequest(RID_PAUSE);

	return S_OK;
}

STDMETHODIMP COSPGraph::Stop(void)
{
	m_request.AppendRequest(RID_STOP);

	return S_OK;
}

STDMETHODIMP COSPGraph::GetCurrentPos(long *aPos)
{
	return E_NOTIMPL;
}

STDMETHODIMP COSPGraph::GetDuration(long *aPos)
{
	return E_NOTIMPL;
}