#pragma once

HRESULT AddGraphToROT(IGraphBuilder* pGb, LPCWSTR pszName, DWORD* lpdwRegisterCookie);
HRESULT RemoveGraphFromROT(DWORD dwResigterCookie);
void Play(IGraphBuilder* pGb, BSTR bstrUrl, HWND hWnd);
void Play(BSTR bstrUrl, HWND hWnd);

HRESULT GetPinDirection(IPin* pPin, PIN_DIRECTION* pDir);
bool IsPinConnected(IPin* pPin);
HRESULT GetUnConnectedPin(IBaseFilter* pFilter, GUID& majorType, PIN_DIRECTION adir, IPin** ppPin);
