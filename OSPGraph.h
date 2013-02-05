#pragma once
#include "stdafx.h"
#include "inner_h.h"
#include "player_h.h"
#include <queue>

typedef long	COMMAND_ID;

class COSPGraph;

class COSPBackgroundRequest
{
public:
	COSPBackgroundRequest();
	~COSPBackgroundRequest();

	void SetFileUrl(LPCWSTR lpcwStr);
	void AppendRequest(COMMAND_ID aCmd);
	COMMAND_ID RemoveRequest();
	DWORD WaitForAllowingMessage(DWORD dwMilliseconds);
	HRESULT ProcessRequest(COSPGraph* pGraph);
protected:
	wstring	m_url;
	queue<COMMAND_ID>	m_cmds;
	CCritSec	m_cs;

	HANDLE		m_hasRequest;
	HRESULT		m_requestResult;

	CComPtr<IGraphBuilder>	m_spGraphBuilder;
};

class ATL_NO_VTABLE COSPGraph : 
				public CComObjectRootEx<CComMultiThreadModel>,
				public CComCoClass<COSPGraph>,
				public IOSPGraph,
				public IOSPGraphBuilderCallback
{
public:
	COSPGraph();
	~COSPGraph();

	HRESULT FinalConstruct();
	void FinalRelease();

	friend COSPBackgroundRequest;
	static unsigned int WINAPI BackgroundHelper(LPVOID aParam);

	BEGIN_COM_MAP(COSPGraph)
		COM_INTERFACE_ENTRY(IOSPGraph)
		COM_INTERFACE_ENTRY(IOSPGraphBuilderCallback)
	END_COM_MAP()

public:
	HRESULT CreateFilterGraph(IGraphBuilder** ppGb);
	HRESULT CloseFilterGraph();
protected:
	void DoNotifyGraphStateChange(OSPGraphState gs, BOOL bNotify=TRUE);
	HRESULT DoRender(IGraphBuilder* pGb, LPCWSTR aUrl);
	HRESULT InternelPlay();
	HRESULT InternelPause();
	HRESULT InternelStop();
	HRESULT InternelClose();

//IOSPGraph.
public:
	STDMETHOD(Init)(IOSPGraphEventHandler* aEventHandler, IOSPServiceMgr* aServiceMre);
	STDMETHOD(Render)(BSTR aUrl);
	STDMETHOD(get_Url)(BSTR *ppUrl);
	STDMETHOD(Play)(void);
	STDMETHOD(Pause)(void);
	STDMETHOD(Stop)(void);
	STDMETHOD(GetCurrentPos)(long *aPos);
	STDMETHOD(GetDuration)(long *aPos);
//IOSPGraphBuilderCallback.
public:
	STDMETHOD(ShouldOperationContinue)(void);
public:
	COSPBackgroundRequest	m_request;
protected:
	wstring		m_url;
	CCritSec	m_cs;
	BOOL		m_bAborted;
	CComAutoCriticalSection	m_csAddFilter;

	CComPtr<IFileSourceFilter>	m_sourceFilter;
	CComPtr<IOSPServiceMgr>	m_spServiceMgr;
	CComPtr<IOSPGraphEventHandler>	m_spGraphEventHander;
};
