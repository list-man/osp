#pragma once
#include "inner_h.h"
#include "OSPGraph.h"

class COSPGraphManager :
			public CComObjectRootEx<CComMultiThreadModel>,
			public CComCoClass<COSPGraphManager, &CLSID_NULL>,
			public IDispatchImpl<IOSPGraphEventHandler, &__uuidof(IOSPGraphEventHandler)>
{
public:
	COSPGraphManager();
	~COSPGraphManager();

	HRESULT FinalConstruct();
	void FinalRelease();

	BEGIN_COM_MAP(COSPGraphManager)
		COM_INTERFACE_ENTRY(IOSPGraphEventHandler)
	END_COM_MAP()

//IOSPGraphEventHandler.
public:
	STDMETHOD(NotifyEvent)(ULONG_PTR punkGraph, long aEventCode, LONG_PTR wEventParam, LONG_PTR lEventParam);
	STDMETHOD(NotifyError)(ULONG_PTR punkGraph, long aErrCode, long param);
	STDMETHOD(NotifyGraphStateChange)(ULONG_PTR punkGraph, long aState);
	STDMETHOD(NotifyStreamEnd)(LONG_PTR punkGraph);
	STDMETHOD(NotifyVolumnChange)(LONG_PTR punkGraph, long aVolumn, BOOL aMute);

public:
	void SetVideoWindow(HWND hWnd);
	void SetOSPServiceMgr(IOSPServiceMgr* aServiceMgr);
	HRESULT RenderGraph(LPCWSTR aUrl);
	HRESULT PlayGraph();
	HRESULT PauseGraph();
	HRESULT StopGraph();
protected:
	HRESULT GetActiveGraph(IOSPGraph** ppG);
	HRESULT InitOSPGraph();
protected:
	HWND		m_hVRWnd;
	CComPtr<IOSPGraph>	m_ospGraph;
	CComPtr<IOSPServiceMgr>	m_spServiceMgr;
};