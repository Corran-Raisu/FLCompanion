// This file is Copyright (c) Olivier Marcoux, and licensed for free use within the FLCompanion project
// You must request the author's permission to use it outside this project: http://wiz0u.free.fr/perso/contact.php
#include "stdafx.h"

extern "C" 
{
	int (STDAPICALLTYPE *StrCmpLogical) (LPCTSTR psz1, LPCTSTR psz2);
}

int STDAPICALLTYPE StrCmpLogicalT(LPCTSTR psz1, LPCTSTR psz2)
{
	int result;
	do
	{
		if (_istdigit(*psz1) && _istdigit(*psz2))
		{
			unsigned long val1 = _tcstoul(psz1, (LPTSTR*) &psz1, 10);
			unsigned long val2 = _tcstoul(psz2, (LPTSTR*) &psz2, 10);
			if (val1 < val2)
				return -1;
			else if (val1 > val2)
				return 1;
			else
				result = 0;
		}
		else
		{
			result = _tcsncicoll(psz1, psz2, 1);
			psz1 = _tcsinc(psz1);
			psz2 = _tcsinc(psz2);
		}
	} while (result == 0);
	return result;
}

int CALLBACK ListCtrl_AutoSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LPARAM* params = (LPARAM*) lParamSort;
	TCHAR text1[1024];
	TCHAR text2[1024];
	int iSubItem = params[1];
	if (iSubItem < 0) iSubItem = -iSubItem;
	iSubItem--;

	ListView_GetItemText((HWND) params[0], (int) lParam1, iSubItem, text1, 1024);
	ListView_GetItemText((HWND) params[0], (int) lParam2, iSubItem, text2, 1024);

	return (params[1] < 0 ? -1 : 1)*StrCmpLogical(text1, text2);
}

#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif

void ListView_HeaderSortReset(HWND listWnd)
{
	int iItem = (int) RemoveProp(listWnd, _T("AutoSort"));
	if (iItem == 0) return;
	if (iItem < 0) iItem = -iItem;
	iItem--;
	HWND hdrCtrl = ListView_GetHeader(listWnd);
	HDITEM hdItem;
	hdItem.mask = HDI_FORMAT;
	Header_GetItem(hdrCtrl, iItem, &hdItem);
	hdItem.fmt = hdItem.fmt & ~(HDF_SORTDOWN|HDF_SORTUP);
	Header_SetItem(hdrCtrl, iItem, &hdItem);
}

void ListView_HeaderSort(HWND listWnd, int column, BOOL descending) 
{
#ifdef UNICODE
	(FARPROC&) StrCmpLogical = GetProcAddress(GetModuleHandle(_T("SHLWAPI.DLL")), "StrCmpLogicalW");
	if (StrCmpLogical == NULL) // si on est pas sous XP, on retombe sur le cas standard
#endif
		StrCmpLogical = StrCmpLogicalT;

	LPARAM params[2] = { 
		(LPARAM) listWnd,
		(LPARAM) descending ? -column-1 : column+1 
	};
	ListView_SortItemsEx(listWnd, ListCtrl_AutoSortFunc, params);
	ListView_HeaderSortReset(listWnd);
	HWND hwndHD = ListView_GetHeader(listWnd);
	HDITEM hdItem;
	hdItem.mask = HDI_FORMAT;
	Header_GetItem(hwndHD, column, &hdItem);
	hdItem.fmt = hdItem.fmt | ((params[1] < 0) ? HDF_SORTDOWN : HDF_SORTUP);
	Header_SetItem(hwndHD, column, &hdItem);
	SetProp(listWnd, _T("AutoSort"), (HANDLE) params[1]);
}

void ListView_HeaderSort(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	HWND listWnd = GetParent(pNMHDR->hwndFrom);
	ListView_HeaderSort(listWnd, phdn->iItem, ((int) GetProp(listWnd, _T("AutoSort")) == phdn->iItem+1));
	*pResult = 0;
}

