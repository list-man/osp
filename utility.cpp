#include "stdafx.h"
#include "utility.h"

HRESULT AddGraphToROT(IGraphBuilder* pGb, LPCWSTR pszName, DWORD* lpdwRegisterCookie)
{
	HRESULT hr = E_FAIL;

	CComPtr<IRunningObjectTable> spROT;
	if (SUCCEEDED(GetRunningObjectTable(NULL, &spROT))
		&& spROT)
	{
		wchar_t szName[_MAX_PATH] = {0};
		//格式化的格式一定要严格按照MSDN的说明：!FilterGraph X pid Y
		wsprintfW(szName, L"FilterGraph %08p pid %08x %s", (DWORD_PTR)pGb, GetCurrentProcessId(), pszName);
		CComPtr<IMoniker> spMoniker;
		hr = CreateItemMoniker(L"!", szName, &spMoniker);
		if (SUCCEEDED(hr))
		{
			spROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pGb, spMoniker, lpdwRegisterCookie);
			//spMoniker.Release();
		}
	}

	return hr;
}

HRESULT RemoveGraphFromROT(DWORD dwResigterCookie)
{
	HRESULT hr = E_FAIL;

	CComPtr<IRunningObjectTable> spROT;
	if (SUCCEEDED(GetRunningObjectTable(NULL, &spROT))
		&& spROT)
	{
		hr = spROT->Revoke(dwResigterCookie);
	}

	return hr;
}

void Play(BSTR bstrUrl, HWND hWnd)
{
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		::MessageBox(NULL, L"初始化Com库失败!", L"Tips", MB_OK);
	}
	else
	{
		CComPtr<IGraphBuilder> spGraphBuilder;
		if (SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGraphBuilder), (LPVOID*)&spGraphBuilder))
			&& spGraphBuilder)
		{
			DWORD dwRegCookie = 0;
			AddGraphToROT(spGraphBuilder, L"JayGuo Filter", &dwRegCookie);
			CComPtr<IMediaControl> spMediaControl;
			CComPtr<IMediaEvent> spMediaEvent;

			BOOL bUseSpecifiedSourceFilter = FALSE;
			CComPtr<IBaseFilter> spMp4SFilter;
			// 			if (SUCCEEDED(CoCreateInstance(CLSID_MP4, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)&spMp4SFilter))
			// 				&& spMp4SFilter)
			// 			{
			// 				bUseSpecifiedSourceFilter = TRUE;
			// 				spGraphBuilder->AddFilter(spMp4SFilter, L"JayGuo Mp4Source");
			// 			}

			CComPtr<IBaseFilter> spVMR;
			if (SUCCEEDED(CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)&spVMR))
				&& spVMR)
			{
				spGraphBuilder->AddFilter(spVMR, L"VMR7 Render");
				CComQIPtr<IVMRFilterConfig> spVMRCongif = spVMR;
				if (spVMRCongif)
					spVMRCongif->SetRenderingMode(VMRMode_Windowless);

				CComQIPtr<IVMRWindowlessControl> spVMRWndLess = spVMR;
				if (spVMRWndLess)
					spVMRWndLess->SetVideoClippingWindow(hWnd);
			}


			spGraphBuilder->QueryInterface(__uuidof(IMediaControl), (LPVOID*)&spMediaControl);
			spGraphBuilder->QueryInterface(__uuidof(IMediaEvent), (LPVOID*)&spMediaEvent);

			if (spMediaControl && spMediaEvent)
			{
				if (!spMp4SFilter)
				{
					hr = spGraphBuilder->AddSourceFilter(bstrUrl, L"PPLive PPLMpeg4 Splitter", &spMp4SFilter);
				}
				else
				{
					CComQIPtr<IFileSourceFilter> spFileSource = spMp4SFilter;
					if (spFileSource)
						spFileSource->Load(bstrUrl, NULL);
				}

				if (spMp4SFilter)
				{
					CComPtr<IEnumPins> spEnumPins;
					spMp4SFilter->EnumPins(&spEnumPins);
					if (spEnumPins)
					{
						CComPtr<IPin> spPins[3];
						spEnumPins->Reset();

						ULONG nRetrived = 0;
						DWORD dwPinCnts = 0;
						while (SUCCEEDED(spEnumPins->Next(1, &spPins[dwPinCnts], &nRetrived)) && nRetrived>0)
						{
							dwPinCnts++;
						}

						if (spPins[0])
						{
							if (spPins[0])
								spGraphBuilder->Render(spPins[0]);
							if (spPins[1])
								spGraphBuilder->Render(spPins[1]);

							if (SUCCEEDED(spMediaControl->Run()))
							{
								long evCode = 0;
								spMediaEvent->WaitForCompletion(INFINITE, &evCode);
							}

							if (spPins[0])
								spPins[0].Release();
							if (spPins[1])
								spPins[1].Release();

							spEnumPins.Release();
							spMp4SFilter.Release();
						}
					}
				}
			}

			if (dwRegCookie > 0)
				RemoveGraphFromROT(dwRegCookie);

			spMediaControl.Release();
			spMediaEvent.Release();
			spGraphBuilder.Release();
		}
	}

	::CoUninitialize();
}

void Play(IGraphBuilder* pGb, BSTR bstrUrl, HWND hWnd)
{
	HRESULT hr = E_FAIL;
	CComPtr<IGraphBuilder> spGraphBuilder = pGb;

	DWORD dwRegCookie = 0;
	AddGraphToROT(spGraphBuilder, L"JayGuo Filter", &dwRegCookie);
	CComPtr<IMediaControl> spMediaControl;
	CComPtr<IMediaEvent> spMediaEvent;

	BOOL bUseSpecifiedSourceFilter = FALSE;
	CComPtr<IBaseFilter> spMp4SFilter;
	// 			if (SUCCEEDED(CoCreateInstance(CLSID_MP4, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)&spMp4SFilter))
	// 				&& spMp4SFilter)
	// 			{
	// 				bUseSpecifiedSourceFilter = TRUE;
	// 				spGraphBuilder->AddFilter(spMp4SFilter, L"JayGuo Mp4Source");
	// 			}

	CComPtr<IBaseFilter> spVMR;
	if (SUCCEEDED(CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBaseFilter), (LPVOID*)&spVMR))
		&& spVMR)
	{
		spGraphBuilder->AddFilter(spVMR, L"VMR7 Render");
		CComQIPtr<IVMRFilterConfig> spVMRCongif = spVMR;
		if (spVMRCongif)
			spVMRCongif->SetRenderingMode(VMRMode_Windowless);

		CComQIPtr<IVMRWindowlessControl> spVMRWndLess = spVMR;
		if (spVMRWndLess)
			spVMRWndLess->SetVideoClippingWindow(hWnd);
	}

	spGraphBuilder->QueryInterface(__uuidof(IMediaControl), (LPVOID*)&spMediaControl);
	spGraphBuilder->QueryInterface(__uuidof(IMediaEvent), (LPVOID*)&spMediaEvent);

	if (spMediaControl && spMediaEvent)
	{
		if (!spMp4SFilter)
		{
			hr = spGraphBuilder->AddSourceFilter(bstrUrl, L"PPLive PPLMpeg4 Splitter", &spMp4SFilter);
		}
		else
		{
			CComQIPtr<IFileSourceFilter> spFileSource = spMp4SFilter;
			if (spFileSource)
				spFileSource->Load(bstrUrl, NULL);
		}

		if (spMp4SFilter)
		{
			CComPtr<IEnumPins> spEnumPins;
			spMp4SFilter->EnumPins(&spEnumPins);
			if (spEnumPins)
			{
				CComPtr<IPin> spPins[3];
				spEnumPins->Reset();

				ULONG nRetrived = 0;
				DWORD dwPinCnts = 0;
				while (SUCCEEDED(spEnumPins->Next(1, &spPins[dwPinCnts], &nRetrived)) && nRetrived>0)
				{
					dwPinCnts++;
				}

				if (spPins[0])
				{
					if (spPins[0])
						spGraphBuilder->Render(spPins[0]);
					if (spPins[1])
						spGraphBuilder->Render(spPins[1]);

					if (SUCCEEDED(spMediaControl->Run()))
					{
						long evCode = 0;
						spMediaEvent->WaitForCompletion(INFINITE, &evCode);
					}

					if (spPins[0])
						spPins[0].Release();
					if (spPins[1])
						spPins[1].Release();

					spEnumPins.Release();
					spMp4SFilter.Release();
				}
			}
		}
	}

	if (dwRegCookie > 0)
		RemoveGraphFromROT(dwRegCookie);

	spMediaControl.Release();
	spMediaEvent.Release();
	spGraphBuilder.Release();
}

HRESULT GetPinDirection(IPin* pPin, PIN_DIRECTION* pDir)
{
	CheckPointer(pPin, E_INVALIDARG);

	if (SUCCEEDED(pPin->QueryDirection(pDir)))
	{
		return S_OK;
	}

	return E_FAIL;
}

bool IsPinConnected(IPin* pPin)
{
	CComPtr<IPin> spPinTo;
	return SUCCEEDED(pPin->ConnectedTo(&spPinTo)) ? true:false;
}

HRESULT GetUnConnectedPin(IBaseFilter* pFilter, GUID& majorType, PIN_DIRECTION adir, IPin** ppPin)
{
	CheckPointer(pFilter, E_INVALIDARG);
	CheckPointer(ppPin, E_INVALIDARG);
	*ppPin = NULL;

	HRESULT hRes = S_FALSE;
	CComPtr<IEnumPins> spEnumPins;
	pFilter->EnumPins(&spEnumPins);
	if (spEnumPins)
	{
		CComPtr<IPin> spPin;
		while (SUCCEEDED(spEnumPins->Next(1, &spPin, NULL)))
		{
			PIN_DIRECTION dir;
			if (!IsPinConnected(spPin)
				&& SUCCEEDED(GetPinDirection(spPin, &dir)) && dir==adir)
			{
				*ppPin = spPin.Detach();
				hRes = S_OK;
				break;
			}

			spPin.Release();
		}
	}

	return hRes;
}
