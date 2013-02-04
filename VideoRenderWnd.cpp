#include "stdafx.h"
#include "utility.h"
#include "VideoRenderWnd.h"

unsigned int WINAPI ThreadProc(LPVOID lpParameter)
{
	if (lpParameter)
	{
		IGraphBuilder* pGb = (IGraphBuilder*)lpParameter;
		CComQIPtr<IMediaControl> spMediaControl = pGb;
		if (spMediaControl)
		{
			spMediaControl->Run();
		}

		CComQIPtr<IMediaEvent> spMediaEvent = pGb;
		if (spMediaEvent)
		{
			long evCode = 0;
			spMediaEvent->WaitForCompletion(INFINITE, &evCode);
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//public member functions.
CVideoRenderWnd::CVideoRenderWnd()
{
	m_dwRegCookie = 0;
	m_hPlayThread = NULL;
	m_eRenerType = Render_VMR7;
}

CVideoRenderWnd::~CVideoRenderWnd()
{
}

HWND CVideoRenderWnd::GetHWND()
{
	return m_hWnd;
}

//////////////////////////////////////////////////////////////////////////
//protected member functions.
BOOL CVideoRenderWnd::GetSourceFilter(IBaseFilter** ppFilter)
{
	CheckPointer(ppFilter, E_POINTER);
	*ppFilter = NULL;

	HRESULT hr = CoCreateInstance(CLSID_MP4, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)ppFilter);
	if (FAILED(hr))
		hr = CoCreateInstance(CLSID_PplMp4Reader, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)ppFilter);
	return S_OK==hr ? TRUE:FALSE;
}

BOOL CVideoRenderWnd::GetRenderFilter(RenderType rdType, IBaseFilter** ppFilter)
{
	CheckPointer(ppFilter, E_POINTER);
	*ppFilter = NULL;

	CLSID clsRender;
	memset(&clsRender, 0, sizeof(CLSID));
	switch (rdType)
	{
	case Render_VMR7:
		clsRender = CLSID_VideoMixingRenderer;
		break;
	case Render_VMR9:
		clsRender = CLSID_VideoMixingRenderer9;
		break;
	case EVR:
		clsRender = CLSID_EnhancedVideoRenderer;
		break;
	default:
		return FALSE;
	}

	HRESULT hr = CoCreateInstance(clsRender, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)ppFilter);
	return S_OK==hr ? TRUE:FALSE;
}

BOOL CVideoRenderWnd::GetAudioFilter(IBaseFilter** ppFilter)
{
	CheckPointer(ppFilter, E_POINTER);
	*ppFilter = NULL;

	HRESULT hr = CoCreateInstance(CLSID_AudioRender, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)ppFilter);
	return S_OK==hr ? TRUE:FALSE;
}

BOOL CVideoRenderWnd::Play(IGraphBuilder* pGb)
{
	CheckPointer(pGb, FALSE);

	BOOL bRet = FALSE;
	HANDLE m_hPlayThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, (void*)m_spGraphBuilder, 0, NULL);

	CRect rcClient;
	GetClientRect(&rcClient);
	AdaptViedoRect(rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);

	return NULL!=m_hPlayThread ? TRUE:FALSE;
}

HRESULT CVideoRenderWnd::SetVideoPosition(RECT& rcSrc, RECT& rcDest)
{
	HRESULT hRes = S_FALSE;

	if (EVR == m_eRenerType)
	{
		CComPtr<IMFVideoDisplayControl> spMFVDCtrl;
		if (GetVideoDisplayControl(&spMFVDCtrl) && spMFVDCtrl)
		{
			MFVideoNormalizedRect mfSrc = {0.0f, 0.0f, 1.0f, 1.0f};
			hRes = spMFVDCtrl->SetVideoPosition(&mfSrc, &CRect(rcDest.left, rcDest.top, (rcDest.left+rcDest.right)>>1, (rcDest.top+rcDest.bottom)>>1));
		}
	}
	else
	{
		CComQIPtr<IVMRWindowlessControl> spVMRWndLessCtrl = m_spRenderFilter;
		if (spVMRWndLessCtrl)
			hRes = spVMRWndLessCtrl->SetVideoPosition(&rcSrc, &rcDest);
	}

	return hRes;
}

bool CVideoRenderWnd::GetVideoDisplayControl(IMFVideoDisplayControl** pMFVDCtrl)
{
	CheckPointer(pMFVDCtrl, false);
	*pMFVDCtrl = NULL;

	CComQIPtr<IMFGetService> spMFService = m_spRenderFilter;
	if (spMFService)
	{
		spMFService->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)pMFVDCtrl);
	}

	return *pMFVDCtrl != NULL;
}

void CVideoRenderWnd::RepaintVideo(HWND hWnd, HDC hDC)
{
	if (EVR == m_eRenerType)
	{
		CComPtr<IMFVideoDisplayControl> spMFVDCtrl;
		GetVideoDisplayControl(&spMFVDCtrl);
		if (spMFVDCtrl)
			spMFVDCtrl->RepaintVideo();
	}
	else
	{
		CComQIPtr<IVMRWindowlessControl> spVMRWndless = m_spRenderFilter;
		if (spVMRWndless)
			spVMRWndless->RepaintVideo(hWnd, hDC);
	}
}

HRESULT CVideoRenderWnd::GetNativeVideoSize(SIZE* pszVideo, SIZE* pszARVideo)
{
	CheckPointer(pszVideo, E_INVALIDARG);
	CheckPointer(pszARVideo, E_INVALIDARG);

	HRESULT hRes = S_FALSE;
	if (EVR == m_eRenerType)
	{
		CComPtr<IMFVideoDisplayControl> spMFVDCtrl;
		if (GetVideoDisplayControl(&spMFVDCtrl)
			&& spMFVDCtrl)
		{
			hRes = spMFVDCtrl->GetNativeVideoSize(pszVideo, pszARVideo);
		}
	}
	else
	{
		CComQIPtr<IVMRWindowlessControl> spVMRWndless = m_spRenderFilter;
		if (spVMRWndless)
			hRes = spVMRWndless->GetNativeVideoSize(&pszVideo->cx, &pszVideo->cy, &pszARVideo->cx, &pszARVideo->cy);
	}

	return hRes;
}

BOOL CVideoRenderWnd::AdaptViedoRect(int cx, int cy)
{
	if (m_spRenderFilter)
	{
		RECT rcSrc, rcDest;
		// Set the source rectangle.
		SIZE szVideo, szARVideo;
		GetNativeVideoSize(&szVideo, &szARVideo);
		SetRect(&rcSrc, 0, 0, szVideo.cx, szVideo.cy);

		SetRect(&rcDest, 0, 0, cx, cy);

		// Set the video position.
		SetVideoPosition(rcSrc, rcDest);
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//message handlers.
LRESULT CVideoRenderWnd::OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DestroyWindow();
	PostQuitMessage(0);
	return 0;
}

LRESULT CVideoRenderWnd::OnLButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CVideoRenderWnd::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	CPaintDC paintDC(m_hWnd);
	LONGLONG tm;
	CComQIPtr<IMediaControl> spMediaControl = m_spGraphBuilder;
	CComQIPtr<IMediaSeeking> spMediaSeeking = m_spGraphBuilder;
	if (spMediaSeeking)
	{
		spMediaSeeking->GetDuration(&tm);
	}

	OAFilterState state = State_Running;
	if (spMediaControl)
	{
		spMediaControl->GetState(0, &state);
	}

	if (m_spRenderFilter && (State_Running==state || State_Paused==state))
	{
		RepaintVideo(m_hWnd, paintDC.m_ps.hdc);
	}

	return 0;
}

LRESULT CVideoRenderWnd::OnEraseBkgnd(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return TRUE;
}

LRESULT CVideoRenderWnd::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	WORD height = HIWORD(lParam);
	WORD width = LOWORD(lParam);
	AdaptViedoRect(width, height);

	return 0;
}

LRESULT CVideoRenderWnd::OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	if (m_spRenderFilter)
	{
		CComQIPtr<IVMRWindowlessControl> spVMRWndless = m_spRenderFilter;
		if (spVMRWndless)
		{
			spVMRWndless->DisplayModeChanged();
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//Implementations of IPlayerControl.
STDMETHODIMP CVideoRenderWnd::Play(BSTR bstrUrl)
{
	if (!m_spGraphBuilder)
	{
		if (FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,  __uuidof(IGraphBuilder), (LPVOID*)&m_spGraphBuilder))
			|| !m_spGraphBuilder)
			return E_FAIL;

		AddGraphToROT(m_spGraphBuilder, L"JayGuo Filter", &m_dwRegCookie);

		if (GetSourceFilter(&m_spSourceFilter) && m_spSourceFilter)
			m_spGraphBuilder->AddFilter(m_spSourceFilter, L"JayGuo Mp4 SourceFilter");
// 		else
// 			m_spGraphBuilder->AddSourceFilter(bstrUrl, L"PPLive PPLMpeg4 Splitter", &m_spSourceFilter);

		if (GetRenderFilter(EVR, &m_spRenderFilter) && m_spRenderFilter)
		{
			m_eRenerType = EVR;

			m_spGraphBuilder->AddFilter(m_spRenderFilter, L"JayGuo VMR RenderFilter");
			CComQIPtr<IMFGetService> spMFService = m_spRenderFilter;
			if (spMFService)
			{
				CComQIPtr<IMFVideoDisplayControl> spMFVDCtrl;
				spMFService->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)&spMFVDCtrl);
				if (spMFVDCtrl)
					spMFVDCtrl->SetVideoWindow(m_hWnd);
			}
		}

		if (GetAudioFilter(&m_spAudioRndFilter) && m_spAudioRndFilter)
		{
			m_spGraphBuilder->AddFilter(m_spAudioRndFilter, L"JayGuo Mp4 AudioRenderFilter");
		}
	}
	
	if (bstrUrl && m_hWnd)
	{
		CComQIPtr<IFileSourceFilter> spFileSourFilter = m_spSourceFilter;
		if (spFileSourFilter)
			spFileSourFilter->Load(bstrUrl, NULL);

		CComPtr<IEnumPins> spEnumPins;
		m_spSourceFilter->EnumPins(&spEnumPins);
		if (spEnumPins)
		{
/*			CComPtr<IPin> spPin[5];
			DWORD dwPinIndex = 0;
			ULONG nRetrived = 0;
			while (dwPinIndex<5 && SUCCEEDED(spEnumPins->Next(1, &spPin[dwPinIndex], &nRetrived)) && nRetrived>0)
			{
				dwPinIndex++;
			}

			if (spPin[0])
				m_spGraphBuilder->Render(spPin[0]);
			if (spPin[1])
				m_spGraphBuilder->Render(spPin[1]);*/

			bool bVideoConnected = false;
			bool bAudioConnected = false;

			do 
			{
				CComPtr<IPin> spPinOut;
				CComPtr<IPin> spPinIn;
				if (SUCCEEDED(spEnumPins->Next(1, &spPinOut, NULL))
					&& spPinOut)
				{
					PIN_DIRECTION dir;
					if (!IsPinConnected(spPinOut) && SUCCEEDED(GetPinDirection(spPinOut, &dir)) && PINDIR_OUTPUT==dir)
					{
						CComPtr<IEnumMediaTypes> spEnumMediaTypes;
						spPinOut->EnumMediaTypes(&spEnumMediaTypes);
						if (spEnumMediaTypes)
						{
							do 
							{
								AM_MEDIA_TYPE* pMt = NULL;
								HRESULT hr;
								hr = spEnumMediaTypes->Next(1, &pMt, NULL);
								if (S_OK == hr)
								{
									if (MEDIATYPE_Video == pMt->majortype)
									{
										if (m_spRenderFilter)
										{
											CComPtr<IEnumPins> spEnumPinsIn;
											m_spRenderFilter->EnumPins(&spEnumPinsIn);
											if (spEnumPinsIn)
											{
												while (SUCCEEDED(spEnumPinsIn->Next(1, &spPinIn, NULL)))
												{
													PIN_DIRECTION dir;
													if (!IsPinConnected(spPinIn)
														&& SUCCEEDED(GetPinDirection(spPinIn, &dir)) && PINDIR_INPUT==dir)
													{
														bVideoConnected = true;
														goto End;
													}

													spPinIn.Release();
												}
											}
										}
									}
									else if (MEDIATYPE_Audio == pMt->majortype)
									{
										if (m_spAudioRndFilter)
										{
											CComPtr<IEnumPins> spEnumPinsIn;
											m_spAudioRndFilter->EnumPins(&spEnumPinsIn);
											if (spEnumPinsIn)
											{
												while (SUCCEEDED(spEnumPinsIn->Next(1, &spPinIn, NULL)))
												{
													PIN_DIRECTION dir;
													if (!IsPinConnected(spPinIn)
														&& SUCCEEDED(GetPinDirection(spPinIn, &dir)) && PINDIR_INPUT==dir)
													{
														bAudioConnected = true;
														goto End;
													}

													spPinIn.Release();
												}
											}
										}
									}
								}
								else if (VFW_E_ENUM_OUT_OF_SYNC == hr)
								{
									spEnumMediaTypes->Reset();
								}
								else if (S_FALSE == hr)
								{
									break;
								}
							} while (true);
						}
					}

End:				if (spPinIn && spPinOut)
					{
						m_spGraphBuilder->Connect(spPinOut, spPinIn);
					}
				}
				else
					break;

			} while (!bVideoConnected || !bAudioConnected);
		}

		Play(m_spGraphBuilder);
		Invalidate();
	}
	
	return S_OK;
}

STDMETHODIMP CVideoRenderWnd::Pause()
{
	if (m_spGraphBuilder)
	{
		CComQIPtr<IMediaControl> spMediaControl = m_spGraphBuilder;
		if (spMediaControl)
			spMediaControl->Pause();
	}

	return S_OK;
}

STDMETHODIMP CVideoRenderWnd::Stop()
{
	if (m_spGraphBuilder)
	{
		CComQIPtr<IMediaControl> spMediaControl = m_spGraphBuilder;
		if (spMediaControl)
		{
			spMediaControl->Stop();
			Invalidate();
		}
	}

	return E_NOTIMPL;
}

STDMETHODIMP CVideoRenderWnd::CreatePlayer(/*[in]*/IMainWnd* pParent, /*[in]*/long aLeft, /*[in]*/long aTop, /*[in]*/long aWidth, /*[in]*/long aHeight)
{
	CheckPointer(pParent, E_POINTER);

	HWND hWnd = pParent->GetHWND();
	if (hWnd)
		Create(hWnd, CRect(aLeft, aTop, aWidth, aHeight), L"", WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE, 0);

	return NULL!=m_hWnd ? S_OK:S_FALSE;
}

STDMETHODIMP CVideoRenderWnd::DestroyPlayer()
{
	DestroyWindow();

	if (m_spMainWnd)
		m_spMainWnd.Release();

	return S_OK;
}

