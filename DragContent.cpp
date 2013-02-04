#include "stdafx.h"
#include "DragContent.h"
#include "mainWnd.h"
#include <ShlObj.h>

CDragContent::CDragContent()
{
	CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
					__uuidof(IDragSourceHelper), (void**)&m_spDragSourceHelper.p);
}

CDragContent::~CDragContent()
{
	int iSize = m_vecStgMdPtr.size();
	for (int i=0; i<iSize; i++)
	{
		if (m_vecStgMdPtr[i].bNeedRelease)
		{
			ReleaseStgMedium(m_vecStgMdPtr[i].pMedium);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//protected member functions.
BOOL CDragContent::CopyMedium(LPSTGMEDIUM pDest, const LPSTGMEDIUM pSrc, const LPFORMATETC pFmtSrc)
{
	CheckPointer(pDest, FALSE);
	CheckPointer(pSrc, FALSE);
	CheckPointer(pFmtSrc, FALSE);

	BOOL bRet = TRUE;
	switch (pSrc->tymed)
	{
	case TYMED_HGLOBAL:
		pDest->hGlobal = (HGLOBAL)OleDuplicateData(pSrc->hGlobal, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_FILE:
		pDest->lpszFileName = (LPOLESTR)OleDuplicateData(pSrc->lpszFileName, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_GDI:
		pDest->hBitmap = (HBITMAP)OleDuplicateData(pSrc->hBitmap, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_MFPICT:
		pDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData(pSrc->hMetaFilePict, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_ENHMF:
		pDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData(pSrc->hEnhMetaFile, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_ISTREAM:
		pDest->pstg = pSrc->pstg;
		if (pDest->pstg)
			pDest->pstg->AddRef();
		break;
	case TYMED_ISTORAGE:
		pDest->pstg = pSrc->pstg;
		if (pDest->pstg)
			pDest->pstg->AddRef();
		break;
	case TYMED_NULL:
		bRet = FALSE;
		break;
	default:
		bRet = FALSE;
		break;
	}

	pDest->tymed = pSrc->tymed;
	pDest->pUnkForRelease = NULL;
	if(pSrc->pUnkForRelease != NULL)
	{
		pDest->pUnkForRelease = pSrc->pUnkForRelease;
		pSrc->pUnkForRelease->AddRef();
	}

	return bRet;
}

CBitmap* CDragContent::GetBitmapFromFile(LPCWSTR lpszPath)
{
	CheckPointer(lpszPath, NULL);

	if (PathFileExists(lpszPath))
	{
		if (!m_bitmap.IsNull())
			m_bitmap.DeleteObject();

		Bitmap bitmap(lpszPath);
		if (bitmap.GetLastStatus() == Ok)
		{
			HBITMAP hBmp;
			bitmap.GetHBITMAP(Color(192, 233, 211), &hBmp);
			m_bitmap.Attach(hBmp);
			return &m_bitmap;
		}
	}

	return NULL;
}

BOOL CDragContent::CreateQuickLink(IStream** pStream, const LPCHANNEL_INFO lpChInfo)
{
	char pContent[] = "pptv 每个人的网络电视 chId=123456";
	HGLOBAL hMem = GlobalAlloc(GHND, sizeof(pContent));
	void* ptr= GlobalLock(hMem);
	if (!ptr)
	{
		GlobalFree(hMem);
		return FALSE;
	}
	memcpy(ptr, pContent, sizeof(pContent));
	GlobalUnlock(hMem);
	CreateStreamOnHGlobal(hMem, TRUE, pStream);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//Implementations of IDragContent.
STDMETHODIMP CDragContent::DragToGenShortcut(/*[in]*/BSTR aImgPath, /*[in]*/long x, /*[in]*/long y, /*[in]*/long cx, /*[in]*/long cy)
{
	if (SUCCEEDED(AddData_DragPicture(aImgPath, x, y, cx, cy)) && m_spDragSourceHelper)
	{
		DWORD dwEffect = 0;
		return ::DoDragDrop(this, static_cast<IDropSource*>(this), DROPEFFECT_MOVE|DROPEFFECT_COPY|DROPEFFECT_LINK, &dwEffect);
	}
	else
		return S_FALSE;
}

STDMETHODIMP CDragContent::GenShortcut(/*[in]*/BSTR aImgPath, /*[in]*/BSTR aSavePath)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDragContent::SetArguments(/*[in]*/long aCatalogId, /*[in]*/long aChannelId, /*[in]*/BSTR aName, /*[in]*/long aHjId, /*[in]*/BSTR aHjName, /*[in]*/long aChType,/*[in]*/BSTR aUser, /*[in, defaultvalue(0)]*/BSTR aDesc, /*[in, defaultvalue(0)]*/long aReserved0, /*[in, defaultvalue(0)]*/long aReserved1, /*[in, defaultvalue(0)]*/long aReserved2, /*[in, defaultvalue(0)]*/BSTR aReserved3, /*[in, defaultvalue(0)]*/BSTR aReserved4, /*[in, defaultvalue(0)]*/BSTR aReserved5)
{
	m_chInfo.catalogId = aCatalogId;
	m_chInfo.channelId = aChannelId;
	m_chInfo.hjId = aHjId;
	m_chInfo.bsName = aName;
	m_chInfo.bsHjName = aHjName;
	m_chInfo.bsDesc = aDesc;
	m_chInfo.bsUser = aUser;
	m_chInfo.isHj = aHjId > 0;

	CString strPreferredName;
	if (m_chInfo.isHj && m_chInfo.bsHjName.Length() > 0)
		strPreferredName = m_chInfo.bsHjName;
	else
		strPreferredName = m_chInfo.bsName;

	FORMATETC fmtetc = { RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM medium = { TYMED_HGLOBAL };

	FILEGROUPDESCRIPTORW fgd;
	memset(&fgd, 0, sizeof(FILEGROUPDESCRIPTORW));
	fgd.cItems = 1;
	fgd.fgd[0].dwFlags = FD_LINKUI|FD_FILESIZE;
	fgd.fgd[0].nFileSizeLow = 0;
	fgd.fgd[0].nFileSizeHigh = 0;
	wcscpy_s(fgd.fgd[0].cFileName, strPreferredName);

	HGLOBAL hFgd = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(FILEGROUPDESCRIPTORW));
	if (NULL == hFgd)
		return S_FALSE;

	LPFILEGROUPDESCRIPTORW lpFgd = (LPFILEGROUPDESCRIPTORW)GlobalLock(hFgd);
	if (NULL == lpFgd)
	{
		GlobalFree(hFgd);
		return S_FALSE;
	}

	memcpy(lpFgd, &fgd, sizeof(FILEGROUPDESCRIPTORW));
	GlobalUnlock(hFgd);
	medium.hGlobal = hFgd;

	if (FAILED(SetData(&fmtetc, &medium, TRUE)))
	{
		GlobalFree(hFgd);
		return S_FALSE;
	}
	
	return S_OK;
}

//x, y stand for the offsets of horizon and vertical. If they all equal to zero, then x and y will be half of the cx and cy or half of width and height of the Bitmap.
//cx, cy stand for the width and height of the bitmap.If one of them equals to zero, then this function will use the actual width and height of the bitmap.
STDMETHODIMP CDragContent::AddData_DragPicture(/*[in]*/BSTR bsFileName, /*[in]*/long x, /*[in]*/long y, /*[in]*/long cx, /*[in]*/long cy)
{
	//Create the actual file content, and add it to the container.
	CComPtr<IStream> spStream;
	if (CreateQuickLink(&spStream, &m_chInfo))
	{
		FORMATETC fmtetc = { RegisterClipboardFormat(CFSTR_FILECONTENTS), NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM };
		STGMEDIUM medium = { TYMED_ISTREAM };
		medium.pstm = spStream.Detach();
		SetData (&fmtetc, &medium, TRUE);
	}

	SHDRAGIMAGE sgDragImg;

	CBitmap* pBmp = GetBitmapFromFile(bsFileName);
	if (m_spDragSourceHelper && pBmp)
	{
		CLIPFORMAT ff = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
		FORMATETC fmtetc = {ff, NULL, (DWORD)DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		STGMEDIUM medium = { TYMED_HGLOBAL };

		//CF_TEXT need ANSI text.
		fmtetc.cfFormat = CF_TEXT;
		fmtetc.tymed = TYMED_HGLOBAL;
		medium.tymed = TYMED_HGLOBAL;
		char* p = "http://www.pptv.com";
		HGLOBAL h = GlobalAlloc(GHND, (strlen(p)+1)*sizeof(char));
		LPVOID p2 = GlobalLock(h);
		memcpy(p2, p, (strlen(p)+1)*sizeof(char));
		GlobalUnlock(h);
		medium.hGlobal = h;
		SetData(&fmtetc, &medium, TRUE);

		if (cx<=0 || cy<=0)
		{
			BITMAP bmpInfo;
			memset(&bmpInfo, 0, sizeof(BITMAP));
			pBmp->GetBitmap(&bmpInfo);

			sgDragImg.sizeDragImage.cx = bmpInfo.bmWidth;
			sgDragImg.sizeDragImage.cy = bmpInfo.bmHeight;
		}
		else
		{
			sgDragImg.sizeDragImage.cx = cx;
			sgDragImg.sizeDragImage.cy = cy;
		}

		if (0==x && 0==y)
		{
			sgDragImg.ptOffset.x = sgDragImg.sizeDragImage.cx>>1;
			sgDragImg.ptOffset.y = sgDragImg.sizeDragImage.cy>>1;
		}
		else
		{
			sgDragImg.ptOffset.x = x;
			sgDragImg.ptOffset.y = y;
		}

		sgDragImg.hbmpDragImage = (HBITMAP)pBmp->m_hBitmap;
		sgDragImg.crColorKey = GetSysColor(COLOR_WINDOW);
		return m_spDragSourceHelper->InitializeFromBitmap(&sgDragImg, static_cast<IDataObject*>(this));
	}

	return S_FALSE;
}

//////////////////////////////////////////////////////////////////////////
////Implementations of IDataObject.
STDMETHODIMP CDragContent::GetData(/* [unique][in] */ FORMATETC *pformatetcIn, /* [out] */ STGMEDIUM *pmedium)
{
	MY_DEBUG(_T("CDragContent::GetData format=%d"), pformatetcIn->cfFormat);

	CheckPointer(pformatetcIn, E_POINTER);
	CheckPointer(pmedium, E_POINTER);
	//原来是想着在放开的时候DropTarget会调用GetData来根据cfFormat来取得相应数据，而这个时候再生成数据然后赋值给STDMEDIUM即可，
	//实际上更好的方法是：GetData只提供通用的根据cfFormat来返回STDMEDIUM的功能，而STDMEDIUM是在这之前通过SetData加入容器中的.
	ATLASSERT(m_vecFmtPtr.size() == m_vecStgMdPtr.size());
	if (m_vecStgMdPtr.size() != m_vecStgMdPtr.size())
		return E_FAIL;

	BOOL bCopySucceed = FALSE;
	INT iSize = m_vecFmtPtr.size();
	for (int i=0; i<iSize; i++)
	{
		if (pformatetcIn->cfFormat==m_vecFmtPtr[i]->cfFormat
			&& pformatetcIn->dwAspect==m_vecFmtPtr[i]->dwAspect
			&& pformatetcIn->tymed&m_vecFmtPtr[i]->tymed)
		{
			bCopySucceed = CopyMedium(pmedium, m_vecStgMdPtr[i].pMedium, pformatetcIn);
			if (bCopySucceed)
				break;
		}
	}

	return bCopySucceed ? S_OK:DV_E_FORMATETC;
}

STDMETHODIMP CDragContent::GetDataHere(/* [unique][in] */ FORMATETC *pformatetc, /* [out][in] */ STGMEDIUM *pmedium)
{
	ATLTRACE(L"IDataObject::GetDataHere");
	return E_NOTIMPL;
}

STDMETHODIMP CDragContent::QueryGetData(/* [unique][in] */ __RPC__in_opt FORMATETC *pformatetc)
{
	MY_DEBUG(_T("CDragContent::QueryGetData cfFormat=%d"), pformatetc->cfFormat);
	CheckPointer(pformatetc, E_POINTER);

	ATLASSERT(m_vecFmtPtr.size() == m_vecStgMdPtr.size());
	if (m_vecStgMdPtr.size() != m_vecStgMdPtr.size())
		return E_FAIL;

	HRESULT hr = S_OK;
	INT iSize = m_vecFmtPtr.size();
	for (int i=0; i<iSize; i++)
	{
		if (pformatetc->tymed & m_vecFmtPtr[i]->tymed)
		{
			if (pformatetc->cfFormat == m_vecFmtPtr[i]->cfFormat)
			{
				MY_DEBUG(_T("CDragContent::Get it!"));
				return S_OK;
			}
			else
				hr = DV_E_CLIPFORMAT;
		}
		else
			hr = DV_E_TYMED;
	}

	return hr;
}

STDMETHODIMP CDragContent::GetCanonicalFormatEtc(/* [unique][in] */ __RPC__in_opt FORMATETC *pformatectIn, /* [out] */ __RPC__out FORMATETC *pformatetcOut)
{
	ATLTRACE(L"IDataObject::GetCanonicalFormatEtc");
	if (pformatetcOut == NULL)
		return E_INVALIDARG;

	return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP CDragContent::SetData(/* [unique][in] */ FORMATETC *pformatetc, /* [unique][in] */ STGMEDIUM *pmedium, /* [in] */ BOOL fRelease)
{
	MY_DEBUG(_T("CDragContent::SetData"));

	CheckPointer(pformatetc, E_POINTER);
	CheckPointer(pmedium, E_POINTER);

	LPFORMATETC pNewFmtetc = new FORMATETC();
	LPSTGMEDIUM pNewStgMedium = new STGMEDIUM();

	if (NULL==pNewFmtetc || NULL==pNewStgMedium)
		return E_OUTOFMEMORY;

	memset(pNewFmtetc, 0, sizeof(FORMATETC));
	memset(pNewStgMedium, 0, sizeof(STGMEDIUM));

	*pNewFmtetc = *pformatetc;
	m_vecFmtPtr.push_back(pNewFmtetc);

	if (fRelease)
		*pNewStgMedium = *pmedium;
	else	//copy the medium for future using.otherwise you can only use it during this call.
		CopyMedium(pNewStgMedium, pmedium, pformatetc);

	CUSTOMSTDMEDIUM cMedium;
	cMedium.pMedium = pNewStgMedium;
	cMedium.bNeedRelease = true;	//Whether fRelease is true or false, we should always release it medium.because we copy it.
	m_vecStgMdPtr.push_back(cMedium);

	return S_OK;
}

STDMETHODIMP CDragContent::EnumFormatEtc(/* [in] */ DWORD dwDirection, /* [out] */ __RPC__deref_out_opt IEnumFORMATETC **ppenumFormatEtc)
{
	ATLTRACE(L"IDataObject::EnumFormatEtc");

	if (DATADIR_GET == dwDirection)
	{
		CheckPointer(ppenumFormatEtc, E_POINTER);

		if (!m_spEnumFormatEtc)
		{
			CComObject<COleEnumFormatEtc>* pEnumMoniker = NULL;
			CComObject<COleEnumFormatEtc>::CreateInstance(&pEnumMoniker);
			if (pEnumMoniker)
			{
				pEnumMoniker->QueryInterface(__uuidof(IEnumFORMATETC), (LPVOID*)&(m_spEnumFormatEtc.p));
				pEnumMoniker->AddFORMATETC(&m_vecFmtPtr);
			}
		}

		if (m_spEnumFormatEtc)
		{
			m_spEnumFormatEtc.CopyTo(ppenumFormatEtc);
			return S_OK;
		}
		else
			return E_OUTOFMEMORY;
	}
	else if (DATADIR_SET == dwDirection)
	{

	}

	return S_FALSE;
}

STDMETHODIMP CDragContent::DAdvise(/* [in] */ __RPC__in FORMATETC *pformatetc, /* [in] */ DWORD advf, /* [unique][in] */ __RPC__in_opt IAdviseSink *pAdvSink, /* [out] */ __RPC__out DWORD *pdwConnection)
{
	ATLTRACE(L"IDataObject::DAdvise");

	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDragContent::DUnadvise(/* [in] */ DWORD dwConnection)
{
	ATLTRACE(L"IDataObject::DUnadvise");

	return E_NOTIMPL;
}

STDMETHODIMP CDragContent::EnumDAdvise(/* [out] */ __RPC__deref_out_opt IEnumSTATDATA **ppenumAdvise)
{
	ATLTRACE(L"IDataObject::EnumDAdvise");

	return OLE_E_ADVISENOTSUPPORTED;
}

//////////////////////////////////////////////////////////////////////////
//Implementations of IDropSource.
STDMETHODIMP CDragContent::QueryContinueDrag(/* [annotation][in] */__in  BOOL fEscapePressed, /* [annotation][in] */__in  DWORD grfKeyState)
{
	if (fEscapePressed)
		return DRAGDROP_S_CANCEL;
	else
	{
		if (grfKeyState & (MK_LBUTTON|MK_RBUTTON))
			return S_OK;
		else
			return DRAGDROP_S_DROP;
	}
}

STDMETHODIMP CDragContent::GiveFeedback(/* [annotation][in] */ __in  DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

//Class COleEnumFormatEtc.
//////////////////////////////////////////////////////////////////////////
//Implementations of Class COleEnumFormatEtc.
COleEnumFormatEtc::COleEnumFormatEtc() : m_nCurIndex(0)
{

}

COleEnumFormatEtc::~COleEnumFormatEtc()
{

}

//////////////////////////////////////////////////////////////////////////
//public member functions.
BOOL COleEnumFormatEtc::AddFORMATETC(const std::vector<LPFORMATETC>* lpvec)
{
	if (lpvec)
		m_vecFormatEtcPtr = *lpvec;

	return TRUE;
}

BOOL COleEnumFormatEtc::SetCurIndex(UINT nIndex)
{
	ATLASSERT(nIndex>=0 && nIndex<m_vecFormatEtcPtr.size());
	if (nIndex>=0 && nIndex<m_vecFormatEtcPtr.size())
	{
		m_nCurIndex = nIndex;
		return TRUE;
	}

	return FALSE;
}

STDMETHODIMP COleEnumFormatEtc::Next(/* [in] */ ULONG celt, /* [annotation] */__out_ecount_part(celt,*pceltFetched)  FORMATETC *rgelt, /* [annotation] */ __out_opt  ULONG *pceltFetched)
{
	CheckPointer(rgelt, E_POINTER);
	if (celt<=0 || celt>=m_vecFormatEtcPtr.size() || m_nCurIndex>=m_vecFormatEtcPtr.size())
		return S_FALSE;	//Don't return E_INVALIDARG.

	DWORD dwCnts = 0;
	DWORD dwTotalSize = m_vecFormatEtcPtr.size();
	while (dwCnts<celt && m_nCurIndex<dwTotalSize)
	{
		*rgelt++ = *m_vecFormatEtcPtr[m_nCurIndex++];
		dwCnts++;
	}

	if (pceltFetched)
		*pceltFetched = dwCnts;

	MY_DEBUG(_T("CDragContent::Next ret=%s"), 0==dwCnts ? _T("S_FALSE"):_T("S_OK"));
	return 0==dwCnts ? S_FALSE:S_OK;
}

STDMETHODIMP COleEnumFormatEtc::Skip(/* [in] */ ULONG celt)
{
	if (m_nCurIndex+(int)celt >= m_vecFormatEtcPtr.size())
		return S_FALSE;

	m_nCurIndex += celt;
	return S_OK;
}

STDMETHODIMP COleEnumFormatEtc::Reset( void)
{
	m_nCurIndex = 0;
	return S_OK;
}

STDMETHODIMP COleEnumFormatEtc::Clone(/* [out] */ __RPC__deref_out_opt IEnumFORMATETC **ppenum)
{
	CheckPointer(ppenum, E_POINTER);

	COleEnumFormatEtc* pNewEnumFormatEtc = NULL;
	CComObject<COleEnumFormatEtc>* pObj = NULL;
	CComObject<COleEnumFormatEtc>::CreateInstance(&pObj);
	if (pObj)
	{
		pObj->AddRef();
		pNewEnumFormatEtc = static_cast<COleEnumFormatEtc*>(pObj);
		pNewEnumFormatEtc->AddFORMATETC(&m_vecFormatEtcPtr);
		pNewEnumFormatEtc->SetCurIndex(m_nCurIndex);
	}
	else
		return E_OUTOFMEMORY;

	*ppenum = pNewEnumFormatEtc;
	(**ppenum).AddRef();
	return S_OK;
}

//Class CIStream.
//////////////////////////////////////////////////////////////////////////
//Implementations of Class CIStream.
CIStream::CIStream()
{
	
}

CIStream::~CIStream()
{

}

//////////////////////////////////////////////////////////////////////////
//Implementations of IStream.
STDMETHODIMP CIStream::Read(/* [annotation] */__out_bcount_part(cb, *pcbRead)  void *pv, /* [in] */ ULONG cb, /* [annotation] */__out_opt  ULONG *pcbRead)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::Write(/* [annotation] */__in_bcount(cb)  const void *pv, /* [in] */ ULONG cb, /* [annotation] */__out_opt  ULONG *pcbWritten)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::Seek(/* [in] */ LARGE_INTEGER dlibMove, /* [in] */ DWORD dwOrigin, /* [annotation] */__out_opt  ULARGE_INTEGER *plibNewPosition)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::SetSize(/* [in] */ ULARGE_INTEGER libNewSize)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::CopyTo(/* [unique][in] */ IStream *pstm, /* [in] */ ULARGE_INTEGER cb, /* [annotation] */__out_opt  ULARGE_INTEGER *pcbRead, /* [annotation] */ __out_opt  ULARGE_INTEGER *pcbWritten)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::Commit(/* [in] */ DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::Revert( void)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::LockRegion(/* [in] */ ULARGE_INTEGER libOffset, /* [in] */ ULARGE_INTEGER cb, /* [in] */ DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::UnlockRegion(/* [in] */ ULARGE_INTEGER libOffset, /* [in] */ ULARGE_INTEGER cb, /* [in] */ DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::Stat(/* [out] */ __RPC__out STATSTG *pstatstg, /* [in] */ DWORD grfStatFlag)
{
	return E_NOTIMPL;
}

STDMETHODIMP CIStream::Clone(/* [out] */ __RPC__deref_out_opt IStream **ppstm)
{
	return E_NOTIMPL;
}
