#pragma once

#include "player_h.h"
#include "common_h.h"
#include "OSPGraphManager.h"
#include "filters.h"
#include "inner_h.h"

class ATL_NO_VTABLE COSPServiceMgr :
			public CComObjectRootEx<CComMultiThreadModel>,
			public CComCoClass<COSPServiceMgr>,
			public IOSPServiceMgr
{
public:
	COSPServiceMgr();
	~COSPServiceMgr();

	BEGIN_COM_MAP(COSPServiceMgr)
		COM_INTERFACE_ENTRY(IOSPServiceMgr)
	END_COM_MAP()

//IOSPServiceMgr
public:
	STDMETHOD(RenderUrl)(IGraphBuilder *pGb, LPCWSTR aFile, IOSPGraphBuilderCallback *aCallback);
	STDMETHOD(RenderFilter)(IGraphBuilder *pGb, IBaseFilter *aFilter, IOSPGraphBuilderCallback *aCallback);
	STDMETHOD(RenderPin)(IGraphBuilder *pGb, IPin *aPintout, IOSPGraphBuilderCallback *aCallback);
public:
	void InitFilters();
protected:
	vector<SourceFilter*>	m_sourceFilters;
	vector<Filter*>			m_allFilters;

	std::map<wstring, std::map<wstring, long> >	m_protocalSource;
	std::map<wstring, std::multimap<long, long, std::greater<long> > >	m_extensionSource;

	std::multimap<long, long, std::greater<long> >	m_videoTransform;
	std::multimap<long, long, std::greater<long> >	m_audioTransform;
	std::multimap<long, long, std::greater<long> >	m_otherTransform;
};

class CPlayer :
			public CComObjectRootEx<CComMultiThreadModel>,
			public CComCoClass<CPlayer>,
			public CWindowImpl<CPlayer>,
			public IDispatchImpl<IPlayer, &_ATL_IIDOF(IPlayer), &LIBID_DirectShowExampleLib>,
			public IWidget
{
public:
	CPlayer();
	~CPlayer();
	HRESULT FinalConstruct();

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	DECLARE_NO_REGISTRY()

	BEGIN_COM_MAP(CPlayer)
		COM_INTERFACE_ENTRY(IWidget)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IPlayer)
	END_COM_MAP()

	BEGIN_MSG_MAP_EX(CPlayer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

//message handles.
protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

//interface Implementations.
//IWidget.
public:
	HWND STDMETHODCALLTYPE Create(HWND hParent, UINT nId);
	BOOL STDMETHODCALLTYPE Destroy(void);
	HWND STDMETHODCALLTYPE GetHWnd(void);

//IPlayer.
public:
	HRESULT STDMETHODCALLTYPE play(BSTR aUrl);
	HRESULT STDMETHODCALLTYPE pause(void);
	HRESULT STDMETHODCALLTYPE resume(void);
	HRESULT STDMETHODCALLTYPE stop(void);

protected:
	HRESULT CreateGraphManager();
	HRESULT CreateGraphService();
	HRESULT InitGraphManager();
protected:
	COSPGraphManager*	m_graphManager;
	CComPtr<IOSPServiceMgr>	m_spServiceMgr;
};