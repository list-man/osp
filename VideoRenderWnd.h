#pragma once
#include "DirectShowExample_h.h"
#include "utility.h"

class CVideoRenderWnd : 
				public CComObjectRootEx<CComMultiThreadModel>,
				public CComCoClass<CVideoRenderWnd>,
				public CWindowImpl<CVideoRenderWnd>,
				public IDispatchImpl<IPlayerControl, &_ATL_IIDOF(IPlayerControl), &LIBID_DirectShowExampleLib>,
				public IDispatchImpl<IVideoRender, &_ATL_IIDOF(IVideoRender), &LIBID_DirectShowExampleLib>
{
public:
	CVideoRenderWnd();
	~CVideoRenderWnd();
	HWND GetHWND();

	enum RenderType
	{
		Render_VMR7,
		Render_VMR9,
		EVR
	}m_eRenerType;

	HRESULT FinalConstruct()
	{
		::CoInitializeEx(NULL, COINIT_MULTITHREADED);
		return S_OK;
	}

	void FinalRelease()
	{
		if (m_dwRegCookie > 0)
			RemoveGraphFromROT(m_dwRegCookie);

		::CoUninitialize();
	}

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	DECLARE_NO_REGISTRY()
	DECLARE_CLASSFACTORY_SINGLETON(CVideoRenderWnd)

	BEGIN_COM_MAP(CVideoRenderWnd)
		COM_INTERFACE_ENTRY(IVideoRender)
		COM_INTERFACE_ENTRY2(IDispatch, IVideoRender)
		COM_INTERFACE_ENTRY(IPlayerControl)
	END_COM_MAP()

public:
	BEGIN_MSG_MAP(CVideoRenderWnd)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
	END_MSG_MAP()

	LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//Implementations of IPlayerControl.
public:
	STDMETHOD(Play)(BSTR bstrUrl);
	STDMETHOD(Pause)();
	STDMETHOD(Stop)();
//Implementations of IVideoRender.
public:
	STDMETHOD(CreatePlayer)(/*[in]*/IMainWnd* pParent, /*[in]*/long aLeft, /*[in]*/long aTop, /*[in]*/long aWidth, /*[in]*/long aHeight);
	STDMETHOD(DestroyPlayer)();
protected:
	BOOL GetSourceFilter(IBaseFilter** ppFilter);
	BOOL GetRenderFilter(RenderType rdType, IBaseFilter** ppFilter);
	BOOL GetAudioFilter(IBaseFilter** ppFilter);
	BOOL Play(IGraphBuilder* pGb);
	BOOL AdaptViedoRect(int cx, int cy);
	HRESULT SetVideoPosition(RECT& rcSrc, RECT& rcDest);
	HRESULT GetNativeVideoSize(SIZE* pszVideo, SIZE* pszARVideo);
	bool GetVideoDisplayControl(IMFVideoDisplayControl** pMFVDCtrl);
	void RepaintVideo(HWND hWnd, HDC hDC);
protected:
	CComPtr<IGraphBuilder>	m_spGraphBuilder;
	CComPtr<IBaseFilter>	m_spRenderFilter;
	CComPtr<IBaseFilter>	m_spSourceFilter;
	CComPtr<IBaseFilter>	m_spAudioRndFilter;
	CComPtr<IBaseFilter>	m_spTransformFilter;
	HANDLE					m_hPlayThread;
	DWORD					m_dwRegCookie;
	CComPtr<IMainWnd>		m_spMainWnd;
	wstring					m_url;
	
};
