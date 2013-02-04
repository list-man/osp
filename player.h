#pragma once

#include "player_h.h"
#include "common_h.h"
#include "OSPGraphManager.h"

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

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

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
	HRESULT InitGraphManager();
protected:
	COSPGraphManager*	m_graphManager;
};