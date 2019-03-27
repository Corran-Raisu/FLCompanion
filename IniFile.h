// IniFile.h: interface for the IniFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIFILE_H__B8BCFE26_4867_4EF7_8511_17D776720685__INCLUDED_)
#define AFX_INIFILE_H__B8BCFE26_4867_4EF7_8511_17D776720685__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef CONST WORD far            *LPCWORD;

typedef LPWORD IniSection;
typedef LPWORD IniEntry;
#pragma pack(push,1)
struct IniValue
{
	BYTE type;	enum { vtInt = 1, vtFloat = 2, vtString = 3, vtdword };
	union {
		int vInt;
		float vFloat;
		DWORD vDWORD;
	};
};
#pragma pack(pop)


class IniFile  
{
public:
	static void SetBasePath(LPCTSTR basePath) { g_basePath = basePath; }

public:
	IniFile(const CString &path);
	~IniFile();

	IniSection FindFirstSection(LPCSTR sectionName);
	BOOL	GetNextSection(IniSection &section, CString &name);
	CString GetCurrentSectionName();

	CString	GetValue(IniSection section, LPCSTR valueName);
	int		GetValueInt(IniSection section, LPCSTR valueName);
	float	GetValueFloat(IniSection section, LPCSTR valueName);
	CString	GetValue0(IniSection section, LPCSTR valueName);
	int	    GetValueInt0(IniSection section, LPCSTR valueName);
	float	GetValueFloat0(IniSection section, LPCSTR valueName);

	UINT	EnumEntries(IniSection section, IniEntry &entry);
	CString	GetNextEntry(IniEntry &entry, UINT &valuesCount, IniValue *&values);
	UINT	GetValues(IniSection section, LPCSTR valueName, IniValue *&values);
	CString GetValueName(IniValue* value);
	CString	GetValue(IniValue *value, UINT indice);
	int		GetValueInt(IniValue *value, UINT indice);
	float	GetValueFloat(IniValue *value, UINT indice);
	DWORD   GetValueDWORD(IniValue *value, UINT indice);

private:
	CString EntryToString(IniValue *value);
	void	BadType(IniValue *value, UINT indice, LPCTSTR expected);

private:
	CString m_path;
	HANDLE m_hFile;
	HANDLE m_hFileMappingObject;
	LPSTR m_baseAddress;
	LPSTR m_endAddress;
	BOOL m_textMode;
	IniSection m_firstSection;
	LPSTR m_stringtable;
	IniSection m_curSection; // binary only (filled by GetNextSection)
	CString m_curEntry; // binary only (filled by GetValues & GetNextEntry)
public:
	static CString g_basePath;
};

#endif // !defined(AFX_INIFILE_H__B8BCFE26_4867_4EF7_8511_17D776720685__INCLUDED_)
