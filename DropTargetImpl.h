#pragma once

class CDropTargetImpl
{
public:
	CDropTargetImpl();
	~CDropTargetImpl();

	BOOL AddSupportFORMATETC(const FORMATETC& ftetc);
	UINT GetSupportNum();
	const FORMATETC* GetFORMATETC(UINT aIdx);
	BOOL QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect, bool bSupport=true);
protected:
	vector<FORMATETC> m_formatetcs;
};