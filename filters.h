#pragma once

enum FilterLoadType
{
	FLT_FILE = 0x01,
	FLT_REG = 0x02, 
	FLT_DMO = 0x04,
	FLT_INTERNAL = 0x08,
	FLT_MONIKER = 0x10,
};

enum FilterType
{
	FT_SRC =	0x00000001,
	FT_SPLT =	0x00000002,
	FT_VDEC =	0x00000004,
	FT_ADEC =	0x00000008,
	FT_VEFFECT =	0x00000010,
	FT_AEFFECT =	0x00000020,
	FT_VRENDER =	0x00000040,
	FT_ARENDER =	0x00000080,
	FT_OTHER =		0x00000100,
};

typedef struct tagFilter
{
	BOOL	bStatic;
	const CLSID*	rclsid;
	LPCWSTR	pszName;
	DWORD	dwMerit;
	long	loadType;
	long	filterType;
	UINT	nMediaTypes;
	const REGPINTYPES *lpMediaType;
	LPCWSTR	pszFilePath;
}Filter;

typedef struct tagDMOFilter
{
	tagFilter	filterBase;
	const CLSID*	catid;
	const CLSID* wrapperid;
	LPCWSTR		pszWrapperPath;
}DMOFilter;

typedef struct tagSourceFilter
{
	tagFilter	filterBase;
	LPCWSTR		pszProtocol;
	LPCWSTR		pszExtension;
	LPCTSTR		pszCheckByte;
}SourceFilter;

Filter*	g_filters[];

