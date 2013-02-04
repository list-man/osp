#include <stdafx.h>
#include "DropTargetImpl.h"

CDropTargetImpl::CDropTargetImpl()
{

}

CDropTargetImpl::~CDropTargetImpl()
{

}

BOOL CDropTargetImpl::AddSupportFORMATETC(const FORMATETC& ftetc)
{
	m_formatetcs.push_back(ftetc);
	return TRUE;
}

UINT CDropTargetImpl::GetSupportNum()
{
	return m_formatetcs.size();
}

const FORMATETC* CDropTargetImpl::GetFORMATETC(UINT aIdx)
{
	if (aIdx < m_formatetcs.size())
		return &m_formatetcs[aIdx];

	return NULL;
}

BOOL CDropTargetImpl::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect, bool bSupport/*=true*/)
{
	if (!pdwEffect)	return FALSE;

	DWORD dwOKEffects = *pdwEffect;

	if (!bSupport)
	{
		*pdwEffect = DROPEFFECT_SCROLL;
		return FALSE;
	}
	//CTRL+SHIFT  -- DROPEFFECT_LINK
	//CTRL        -- DROPEFFECT_COPY
	//SHIFT       -- DROPEFFECT_MOVE
	//no modifier -- DROPEFFECT_MOVE or whatever is allowed by src
	*pdwEffect = (grfKeyState & MK_CONTROL) ?
		( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ):
		( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 );
	if(*pdwEffect == 0)
	{
		// No modifier keys used by user while dragging. 
		if (DROPEFFECT_COPY & dwOKEffects)
			*pdwEffect = DROPEFFECT_COPY;
		else if (DROPEFFECT_MOVE & dwOKEffects)
			*pdwEffect = DROPEFFECT_MOVE; 
		else if (DROPEFFECT_LINK & dwOKEffects)
			*pdwEffect = DROPEFFECT_LINK; 
		else 
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
	} 
	else
	{
		// Check if the drag source application allows the drop effect desired by user.
		// The drag source specifies this in DoDragDrop
		if(!(*pdwEffect & dwOKEffects))
			*pdwEffect = DROPEFFECT_NONE;
	}  

	return (DROPEFFECT_NONE == *pdwEffect)?FALSE:TRUE;
}