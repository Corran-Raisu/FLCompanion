// ResourceProvider.cpp: implementation of the ResourceProvider class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResourceProvider.h"
#include "Datas.h"
#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ResourceProvider::ResourceProvider()
{
	memset(m_hLibModule, 0, sizeof(m_hLibModule));
}

void ResourceProvider::Init(const CString &appPath)
{
	IniFile iniFile(L"..\\EXE\\Freelancer.ini");
	IniSection section = NULL;
	CString name;
	m_hNames.AddTail(L"Resources.dll");
	CHECKERR((m_hLibModule[0] = LoadLibraryEx(appPath+_T("\\EXE\\Resources.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE)) != NULL);
	OutputDebugFormat(L"%d => %s\n", 0, L"Resources.dll");
	m_count = 1;
	while (iniFile.GetNextSection(section, name))
	{
		if (name == L"Resources")
		{
			IniEntry entry;
			for (UINT entriesCount = iniFile.EnumEntries(section, entry); entriesCount; entriesCount--)
			{
				UINT valuesCount;
				IniValue *values;
				CHECK(iniFile.GetNextEntry(entry, valuesCount, values) == L"DLL");
				name = iniFile.GetValue(values,0);
				name = name.SpanExcluding(L";");
				name.TrimRight();
				//if (!m_hNames.Find(name))
				{
					m_hNames.AddTail(name);
					name = appPath+_T("\\EXE\\")+name;
					if (!PathFileExists(name))
						ProblemFound(L"Resource DLL %s referenced in Freelancer.ini but not found on disk", name);
					else
						CHECKERR((m_hLibModule[m_count] = LoadLibraryEx(name, NULL, LOAD_LIBRARY_AS_DATAFILE)) != NULL);
					OutputDebugFormat(L"%d => %s\n", m_count, name);
					m_count++;
				}
			}
			break;
		}
	}
}

ResourceProvider::~ResourceProvider()
{
	Free();
}

void ResourceProvider::Free()
{
	OutputDebugFormat(L".\n");
	for (UINT index = 0; index < _countof(m_hLibModule); index++)
		if (m_hLibModule[index] != 0)
		{
			CHECKERR(FreeLibrary(m_hLibModule[index]));
			m_hLibModule[index] = 0;
		}
}

CString ResourceProvider::GetStringFromID(DWORD id)
{
	if (HIWORD(id) >= m_count)
	{
		ProblemFound(L"IDS %d is used, but only %d resource DLLs were declared", id, m_count);
		return L"[IDS "+IntToString(id)+L"]";
	}
	TCHAR buffer[512];
	if (!LoadString(m_hLibModule[HIWORD(id)], LOWORD(id), buffer, _countof(buffer)))
	{
		ProblemFound(L"IDS %d is used, but %s does not contain a string #%d", id, m_hNames.GetAt(m_hNames.FindIndex(HIWORD(id))), LOWORD(id));
		return L"[IDS "+IntToString(id)+L"]";
	}
	return buffer;
}

CString ResourceProvider::GetHTMLFromID(DWORD id)
{
	if (HIWORD(id) >= m_count)
	{
		ProblemFound(L"IDS %d is used, but only %d resource DLLs were declared", id, m_count);
		return L"[IDS "+IntToString(id)+L"]";
	}
	HMODULE hModule = m_hLibModule[HIWORD(id)];
	HRSRC hRsrc = FindResource(hModule, MAKEINTRESOURCE(LOWORD(id)), RT_HTML);
	if (!hRsrc)
	{
		if (id == 0) return CString();
		ProblemFound(L"IDS %d is used, but %s does not contain a string #%d", id, m_hNames.GetAt(m_hNames.FindIndex(HIWORD(id))), LOWORD(id));
		return L"[IDS "+IntToString(id)+L"]";
	}
	DWORD len = SizeofResource(hModule, hRsrc);
	LPVOID html = LockResource(LoadResource(hModule, hRsrc));
	return CString(LPCWSTR(html), len/2);
}
