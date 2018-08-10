// ResourceProvider.h: interface for the ResourceProvider class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESOURCEPROVIDER_H__31662CB1_29CD_4144_94AC_574EEA66D2DE__INCLUDED_)
#define AFX_RESOURCEPROVIDER_H__31662CB1_29CD_4144_94AC_574EEA66D2DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ResourceProvider  
{
public:
	ResourceProvider();
	void Init(const CString &appPath);
	void Free();
	~ResourceProvider();
	CString GetStringFromID(DWORD id);
	CString GetHTMLFromID(DWORD id);
private:
	HMODULE m_hLibModule[32];
	CStringList m_hNames;
	UINT m_count;
};


#endif // !defined(AFX_RESOURCEPROVIDER_H__31662CB1_29CD_4144_94AC_574EEA66D2DE__INCLUDED_)
