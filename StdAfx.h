// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__79D0D619_1B7C_4C0E_916A_0D590AA8690C__INCLUDED_)
#define AFX_STDAFX_H__79D0D619_1B7C_4C0E_916A_0D590AA8690C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0600
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxtempl.h>
#include <atlbase.h>
#include <comdef.h>
#include <comip.h>
#include <assert.h>
#include <map>
#include "resource.h"

// quick hacks to replace stuff dependent on Wizou's private utilities libraries
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CSortedMap : public std::map<KEY, VALUE>
{
public:
	BOOL Contains(ARG_KEY key) const { return find(key) != end(); }
	BOOL Lookup(ARG_KEY key, VALUE& rValue) const { auto it = find(key); if (it == end()) return FALSE; rValue = it->second; return TRUE; }
};
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMapEx : public CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>
{
public:
	BOOL Contains(ARG_KEY key) const { return PLookup(key) != NULL; }
	BOOL LookupPtr(ARG_KEY key, VALUE*& rValue) { CPair* p = PLookup(key); if (p) rValue = &p->value; return p != NULL; }
	POSITION GetStartPosition() const { return (POSITION) PGetFirstAssoc(); }
	VALUE& GetNextAssoc(POSITION& rNextPosition, KEY& rKey) const { CPair* p = (CPair*) rNextPosition; rNextPosition = (POSITION) PGetNextAssoc(p); rKey = p->key; return p->value; }
};
#ifdef _DEBUG
#define CHECKERR(expr) assert(expr)
#define CHECK(expr) assert(expr)
#else
#define CHECKERR(expr) (void)(expr)
#define CHECK(expr) (void)(expr)
#endif
#define OutputDebugFormat(s, ...) OutputDebugString(s)
inline CString IntToString(long value)
{
	TCHAR buffer[24];
	_ltot(value, buffer, 10);
	return buffer;
}
inline CString DoubleToString(double value)
{
	TCHAR buffer[32];
	_sntprintf(buffer, _countof(buffer), _T("%.30g"), value);
	return buffer;
}
inline CString FloatToString(float value)
{
	TCHAR buffer[8];
	_sntprintf(buffer, _countof(buffer), _T("%.6g"), value);
	return buffer;
}
inline void EnableDlgItem(IN HWND hDlg, IN int nIDDlgItem, IN BOOL bEnable)
{
	::EnableWindow(::GetDlgItem(hDlg, nIDDlgItem), bEnable);
}

void LogText(LPCTSTR lpMsg);
void Log(LPCTSTR lpFormat, ...);
void FatalError(const CString &msg);
void ProblemFound(LPCTSTR lpFormat, ...);
void FormatNumber(TCHAR* buffer, long value, UINT flags = 0);
void FormatNumber(TCHAR* buffer, double value, UINT flags = 0);
void ListView_HeaderSortReset(HWND listWnd);
void ListView_HeaderSort(HWND listWnd, int column, bool descending = false);
void ListView_HeaderSort(NMHDR* pNMHDR, LRESULT* pResult);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__79D0D619_1B7C_4C0E_916A_0D590AA8690C__INCLUDED_)
