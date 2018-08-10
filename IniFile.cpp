// IniFile.cpp: implementation of the IniFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CString IniFile::g_basePath;

#define INIFILE_MAGIC 'INIB'
#define INIFILE_VERSION 1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IniFile::IniFile(const CString &path)
{
	m_path = path;
	CString fullpath = PathIsRelative(path) ? g_basePath+path : path;
	if (!PathFileExists(fullpath))
		FatalError(L"File "+path+L" not found");
	CHECKERR((m_hFile = CreateFile(fullpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE);
	DWORD size = GetFileSize(m_hFile, NULL);
	if (size == 0)
	{
		m_hFileMappingObject = INVALID_HANDLE_VALUE;
		m_textMode = false;
		m_firstSection = NULL;
		m_stringtable = NULL;
	}
	else
	{
		CHECKERR((m_hFileMappingObject = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL);
		CHECKERR((m_baseAddress = reinterpret_cast<LPSTR>(MapViewOfFile(m_hFileMappingObject, FILE_MAP_READ, 0, 0, 0))) != NULL);
		m_endAddress = m_baseAddress+size;
		struct IniHeader
		{
			DWORD magic; // "BINI"
			DWORD version; // 1
			DWORD stringtableOffset;
		} *header = reinterpret_cast<IniHeader*>(m_baseAddress);
		m_textMode = (header->magic != INIFILE_MAGIC);
		if (!m_textMode)
		{
			
			if (header->version != INIFILE_VERSION)
				ProblemFound(L"%s: Unexpected BINI header version (%d)", path, header->version);
			m_stringtable = m_baseAddress+header->stringtableOffset;
			m_firstSection = reinterpret_cast<LPWORD>(header+1);
		}
	}
}

IniFile::~IniFile()
{
	if (m_hFileMappingObject != INVALID_HANDLE_VALUE)
	{
		CHECKERR(UnmapViewOfFile(m_baseAddress));
		CHECKERR(CloseHandle(m_hFileMappingObject));
	}
	CHECKERR(CloseHandle(m_hFile));
}

IniSection IniFile::FindFirstSection(LPCSTR sectionName)
{
	// TODO: optimiser eventuellement
	IniSection section = NULL;
	CString name;
	while (GetNextSection(section, name))
		if (name == sectionName)
			return m_curSection = section;
	return m_curSection = NULL;
}

CString IniFile::GetCurrentSectionName()
{
	if (m_textMode)
	{
		LPSTR scanEnd = reinterpret_cast<LPSTR>(m_curSection);
		while (*--scanEnd != ']');
		LPSTR scan = scanEnd;
		while (*--scan != '[');
		return CString(scan+1, scanEnd-scan-1);
	}
	else
		return m_stringtable+*m_curSection;
}

bool IniFile::GetNextSection(IniSection &section, CString &name)
{
	if (m_textMode)
	{
		LPSTR sectionStart = (section == NULL) ? m_baseAddress : reinterpret_cast<LPSTR>(section);
		LPSTR sectionHead;
		do
		{
			sectionHead = sectionStart;
			sectionStart = reinterpret_cast<LPSTR>(memchr(sectionHead,'\n', m_endAddress-sectionHead));
			if (sectionStart)
				sectionStart++;
			else
				return false;
		} while (*sectionHead != '[');
		name = CString(sectionHead+1, strchr(sectionHead, ']')-sectionHead-1);
		m_curSection = section = (IniSection) sectionStart;
		return true;
	}
	else
	{
		if (section == NULL) 
			section = m_firstSection;
		else
		{
			section++;
			WORD entriesCount = *section++;
			while (entriesCount)
			{
				section++;
				BYTE valuesCount = *reinterpret_cast<const BYTE*>(section);
				reinterpret_cast<BYTE*&>(section) += 1+valuesCount*5;
				entriesCount--;
			}
		}
		if (section == reinterpret_cast<IniSection>(m_stringtable))
		{
			section = NULL;
			return false;
		}
		m_curSection = section;
		name = m_stringtable+*section;
	}
	return true;
}

CString IniFile::GetValue(IniSection section, LPCSTR valueName)
{
	if (m_textMode)
	{
		size_t nameLen = strlen(valueName);
		LPCSTR scan = reinterpret_cast<LPCSTR>(section);
		LPCSTR lineEnd;
		do
		{
			if (*scan == '[') break;
			lineEnd = reinterpret_cast<LPCSTR>(memchr(scan, '\n', m_endAddress-scan));
			if (lineEnd == NULL) lineEnd = m_endAddress;
			if (memicmp(scan, valueName, nameLen) == 0)
			{
				scan += nameLen;
				while ((*scan == ' ') || (*scan == '\t')) scan++;
				if (*scan++ == '=')
				{
					while ((*scan == ' ') || (*scan == '\t')) scan++;
					if (scan < lineEnd)
					{
						LPCSTR comment = reinterpret_cast<LPCSTR>(memchr(scan, ';', lineEnd-scan));
						if (comment)
							lineEnd = comment;
						while (isspace(lineEnd[-1])) lineEnd--;
						if (scan < lineEnd)
							return CString(scan, lineEnd-scan);
						else
							return CString();
					}
				}
			}
			scan = lineEnd+1;
		} while (lineEnd < m_endAddress);
	}
	else
	{
		section++;
		WORD entriesCount = *section++;
		while (entriesCount)
		{
			LPCSTR name = m_stringtable+*section++;
			BYTE valuesCount = *reinterpret_cast<LPBYTE>(section);
			if (stricmp(valueName, name) == 0)
			{
				if (valuesCount != 1)
					ProblemFound(L"%s: [%s] %s has a %hs= with %d values instead of 1 expected", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName, valuesCount);
				CHECK(reinterpret_cast<LPBYTE>(section)[1] == 3);
				return m_stringtable+*reinterpret_cast<LPDWORD>(section+1);
			}
			reinterpret_cast<LPBYTE&>(section) += 1+valuesCount*5;
			entriesCount--;
		}
	}
	ProblemFound(L"%s: [%s] %s is missing a %hs=", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName);
	return CString();
}

int IniFile::GetValueInt(IniSection section, LPCSTR valueName)
{
	if (m_textMode)
		return _ttoi(GetValue(section, valueName));
	else
	{
		LPWORD scan = section+1;
		WORD entriesCount = *scan++;
		while (entriesCount)
		{
			LPCSTR name = m_stringtable+*scan++;
			BYTE valuesCount = *reinterpret_cast<LPBYTE>(scan);
			if (stricmp(valueName, name) == 0)
			{
				if (valuesCount != 1)
					ProblemFound(L"%s: [%s] %s has a %hs= with %d values instead of 1 expected", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName, valuesCount);
				CHECK(reinterpret_cast<LPBYTE>(scan)[1] == 1);
				return *reinterpret_cast<LPDWORD>(scan+1);
			}
			reinterpret_cast<LPBYTE&>(scan) += 1+valuesCount*5;
			entriesCount--;
		}
	}
	ProblemFound(L"%s: [%s] %s is missing a %hs=", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName);
	return 0;
}

float IniFile::GetValueFloat(IniSection section, LPCSTR valueName)
{
	if (m_textMode)
		return (float) _tcstod(GetValue(section, valueName), NULL);
	else
	{
		section++;
		WORD entriesCount = *section++;
		while (entriesCount)
		{
			LPCSTR name = m_stringtable+*section++;
			BYTE valuesCount = *reinterpret_cast<LPBYTE&>(section)++;
			if (stricmp(valueName, name) == 0)
			{
				if (valuesCount != 1)
					ProblemFound(L"%s: [%s] %s has a %hs= with %d values instead of 1 expected", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName, valuesCount);
				IniValue *value = reinterpret_cast<IniValue*>(section);
				if (value->type == IniValue::vtFloat)
					return value->vFloat;
				else if (value->type == IniValue::vtInt)
					return (float) value->vInt;
				else
				{	
					BadType(value, 0, L"float");
					if (value->type == IniValue::vtString)
						return (float) strtod(m_stringtable+value->vInt, NULL);
					else
						return 0.0;
				}
			}
			reinterpret_cast<LPBYTE&>(section) += valuesCount*5;
			entriesCount--;
		}
	}
	ProblemFound(L"%s: [%s] %s is missing a %hs=", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName);
	return 0;
}


CString IniFile::GetValue0(IniSection section, LPCSTR valueName) // same as GetValue but does not break if missing
{
	if (m_textMode)
	{
		size_t nameLen = strlen(valueName);
		LPCSTR scan = reinterpret_cast<LPCSTR>(section);
		LPCSTR lineEnd;
		do
		{
			if (*scan == '[') break;
			lineEnd = reinterpret_cast<LPCSTR>(memchr(scan, '\n', m_endAddress-scan));
			if (lineEnd == NULL) lineEnd = m_endAddress;
			if (memicmp(scan, valueName, nameLen) == 0)
			{
				scan += nameLen;
				while ((*scan == ' ') || (*scan == '\t')) scan++;
				if (*scan++ == '=')
				{
					while ((*scan == ' ') || (*scan == '\t')) scan++;
					if (scan < lineEnd)
					{
						LPCSTR comment = reinterpret_cast<LPCSTR>(memchr(scan, ';', lineEnd-scan));
						if (comment)
							lineEnd = comment;
						while (isspace(lineEnd[-1])) lineEnd--;
						if (scan < lineEnd)
							return CString(scan, lineEnd-scan);
						else
							return CString();
					}
				}
			}
			scan = lineEnd+1;
		} while (lineEnd < m_endAddress);
	}
	else
	{
		section++;
		WORD entriesCount = *section++;
		while (entriesCount)
		{
			LPCSTR name = m_stringtable+*section++;
			BYTE valuesCount = *reinterpret_cast<LPBYTE>(section);
			if (stricmp(valueName, name) == 0)
			{
				if (valuesCount != 1)
					ProblemFound(L"%s: [%s] %s has a %hs= with %d values instead of 1 expected", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName, valuesCount);
				CHECK(reinterpret_cast<LPBYTE>(section)[1] == 3);
				return m_stringtable+*reinterpret_cast<LPDWORD>(section+1);
			}
			reinterpret_cast<LPBYTE&>(section) += 1+valuesCount*5;
			entriesCount--;
		}
	}
	return CString();
}

int IniFile::GetValueInt0(IniSection section, LPCSTR valueName) // same as GetValueInt but does not break if missing
{
	if (m_textMode)
		return _ttoi(GetValue0(section, valueName));
	else
	{
		section++;
		WORD entriesCount = *section++;
		while (entriesCount)
		{
			LPCSTR name = m_stringtable+*section++;
			BYTE valuesCount = *reinterpret_cast<LPBYTE>(section);
			if (stricmp(valueName, name) == 0)
			{
				if (valuesCount != 1)
					ProblemFound(L"%s: [%s] %s has a %hs= with %d values instead of 1 expected", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName, valuesCount);
				CHECK(reinterpret_cast<LPBYTE>(section)[1] == 1);
				return *reinterpret_cast<LPDWORD>(section+1);
			}
			reinterpret_cast<LPBYTE&>(section) += 1+valuesCount*5;
			entriesCount--;
		}
	}
	return 0;
}

float IniFile::GetValueFloat0(IniSection section, LPCSTR valueName)
{
	if (m_textMode)
		return (float) _tcstod(GetValue0(section, valueName), NULL);
	else
	{
		section++;
		WORD entriesCount = *section++;
		while (entriesCount)
		{
			LPCSTR name = m_stringtable+*section++;
			BYTE valuesCount = *reinterpret_cast<LPBYTE&>(section)++;
			if (stricmp(valueName, name) == 0)
			{
				if (valuesCount != 1)
					ProblemFound(L"%s: [%s] %s has a %hs= with %d values instead of 1 expected", m_path, GetCurrentSectionName(), GetValue0(section, "nickname"), valueName, valuesCount);
				IniValue *value = reinterpret_cast<IniValue*>(section);
				if (value->type == IniValue::vtFloat)
					return value->vFloat;
				else if (value->type == IniValue::vtInt)
					return (float) value->vInt;
				else
				{	// rare case of a string
					CHECK(value->type == IniValue::vtString);
					return (float) strtod(m_stringtable+value->vInt, NULL);
				}
			}
			reinterpret_cast<LPBYTE&>(section) += valuesCount*5;
			entriesCount--;
		}
	}
	return 0;
}

UINT IniFile::GetValues(IniSection section, LPCSTR valueName, IniValue *&values)
{
	values = NULL;
	m_curEntry = valueName;
	if (m_textMode)
	{
		size_t nameLen = strlen(valueName);
		LPSTR scan = reinterpret_cast<LPSTR>(section);
		LPSTR lineEnd;
		do
		{
			if (*scan == '[') break;
			lineEnd = reinterpret_cast<LPSTR>(memchr(scan, '\n', m_endAddress-scan));
			if (lineEnd == NULL) lineEnd = m_endAddress;
			if (memicmp(scan, valueName, nameLen) == 0)
			{
				scan += nameLen;
				scan += strspn(scan, " \t");
				if (*scan++ == '=')
				{
					scan += strspn(scan, " \t");
					values = reinterpret_cast<IniValue*>(scan);
					UINT valuesCount = 1;
					while (scan = reinterpret_cast<LPSTR>(memchr(scan, ',', lineEnd-scan)))
						valuesCount++, scan++;
					return valuesCount;
				}
			}
			scan = lineEnd+1;
		} while (lineEnd < m_endAddress);
	}
	else
	{
		section++;
		WORD entriesCount = *section++;
		while (entriesCount)
		{
			LPCSTR name = m_stringtable+*section++;
			BYTE valuesCount = *reinterpret_cast<LPBYTE&>(section)++;
			if (stricmp(valueName, name) == 0)
			{
				CHECK(valuesCount > 0);
				values = reinterpret_cast<IniValue*>(section);
				return valuesCount;
			}
			reinterpret_cast<LPBYTE&>(section) += valuesCount*5;
			entriesCount--;
		}
	}
	return 0;
}

CString IniFile::GetValueName(IniValue* value)
{
	if (m_textMode)
	{
		LPCSTR scan = reinterpret_cast<LPCSTR>(value);
		while (*--scan != '=');
		while (isspace(*--scan));
		LPCSTR scanEnd = scan+1;
		while (*--scan != '\n');
		while (isspace(*++scan));
		return CString(scan, scanEnd-scan);
	}
	else
		return m_stringtable+*reinterpret_cast<LPWORD>(reinterpret_cast<LPBYTE>(value)-3);
}

UINT IniFile::EnumEntries(IniSection section, IniEntry &entry)
{
	if (m_textMode)
	{
		entry = section;
		LPCSTR scan = reinterpret_cast<LPCSTR>(section);
		UINT count = 0;
		while ((*scan != '[') && (scan < m_endAddress))
		{
			while ((*scan == ' ') || (*scan == '\t')) scan++;
			if ((*scan != '\r') && (*scan != '\n') && (*scan != ';'))
				count++;
			scan = reinterpret_cast<LPCSTR>(memchr(scan, '\n', m_endAddress-scan));
			if (!scan) break;
			scan++;
		}
		return count;
	}
	else
	{
		entry = section+2;
		return section[1];
	}
}

CString IniFile::GetNextEntry(IniEntry &entry, UINT &valuesCount, IniValue *&values)
{
	if (m_textMode)
	{
		LPSTR scan = reinterpret_cast<LPSTR>(entry);
		LPSTR lineEnd;
		while (*scan != '[')
		{
			CHECK(scan < m_endAddress);
			lineEnd = reinterpret_cast<LPSTR>(memchr(scan, '\n', m_endAddress-scan));
			if (lineEnd == NULL) lineEnd = m_endAddress;
			while ((*scan == ' ') || (*scan == '\t')) scan++;
			if ((*scan != '\r') && (*scan != '\n') && (*scan != ';'))
			{
				entry = reinterpret_cast<IniEntry>(lineEnd+1);
				while (isspace(lineEnd[-1])) lineEnd--;
				LPSTR scan2 = reinterpret_cast<LPSTR>(memchr(scan, '=', lineEnd-scan));
				if (scan2 == NULL)
				{
					values = NULL;
					valuesCount = 0;
					return m_curEntry = CString(scan, lineEnd-scan);
				}
				else
					scan2++;
				CString name = m_curEntry = CString(scan, scan2-scan-1);
				name.TrimRight();
				scan2 += strspn(scan2, " \t");
				values = reinterpret_cast<IniValue*>(scan2);
				if (lineEnd < scan2)
					valuesCount = 0;
				else
				{
					valuesCount = 1;
					while (scan2 = reinterpret_cast<LPSTR>(memchr(scan2, ',', lineEnd-scan2)))
						valuesCount++, scan2++;
				}
				return name;
			}
			else
				scan = lineEnd+1;
		}
		return CString();
	}
	else
	{
		CString name = m_curEntry = m_stringtable+*entry++;
		valuesCount = *reinterpret_cast<LPBYTE&>(entry)++;
		values = reinterpret_cast<IniValue*>(entry);
		entry = reinterpret_cast<IniEntry>(values+valuesCount);
		return name;
	}
}

CString IniFile::GetValue(IniValue *value, UINT indice)
{
	if (m_textMode)
	{
		LPSTR scan = reinterpret_cast<LPSTR>(value);
		while (indice)
		{
			indice--;
			scan = strchr(scan,',')+1;
			scan += strspn(scan, " \t");
		}
		LPSTR lineEnd = reinterpret_cast<LPSTR>(memchr(scan, '\n', m_endAddress-scan));
		if (lineEnd == NULL) lineEnd = m_endAddress;
		while (isspace(lineEnd[-1])) lineEnd--;
		LPSTR scan2 = reinterpret_cast<LPSTR>(memchr(scan, ',', lineEnd-scan));
		if (scan2 > scan) while (isspace(scan2[-1])) scan2--;
		CString val = CString(scan, (scan2 ? scan2 : lineEnd)-scan);
		return val;
	}
	else
	{
		value += indice;
		CHECK(value->type == IniValue::vtString);
		return m_stringtable+value->vInt;
	}
}

int IniFile::GetValueInt(IniValue *value, UINT indice)
{
	if (m_textMode)
	{
		LPSTR scan = reinterpret_cast<LPSTR>(value);
		while (indice)
			indice--, scan = strchr(scan,',')+1;
		return atoi(scan);
	}
	else
	{
		value += indice;
		if (value->type == IniValue::vtInt)
			return value->vInt;
		else if (value->type == IniValue::vtFloat)
			return (int) value->vFloat;
		else
		{
			BadType(value-indice, indice, L"integer");
			return 0;
		}
	}
}

LPCTSTR TypeToString(BYTE type)
{
	switch (type)
	{
	case IniValue::vtInt: return L"integer";
	case IniValue::vtFloat: return L"float";
	case IniValue::vtString: return L"string";
	default: return L"invalid type";
	}
}

CString IniFile::EntryToString(IniValue *value)
{
	BYTE valuesCount = reinterpret_cast<LPBYTE&>(value)[-1];
	CString line;
	line = m_curEntry+" = ";
	while (valuesCount--)
	{
		switch (value->type)
		{
		case IniValue::vtInt:   line += IntToString(value->vInt); break;
		case IniValue::vtFloat: line += FloatToString(value->vFloat); break;
		case IniValue::vtString:line += m_stringtable+value->vInt; break;
		default:				line += "???";
		}
		if (valuesCount) line += ", ";
		value++;
	}
	return line;
}

void IniFile::BadType(IniValue *value, UINT indice, LPCTSTR expected)
{
	ProblemFound(L"%s: [%s] %s has a %s= with argument %d stored as %s instead of %s", m_path, GetCurrentSectionName(), GetValue0(m_curSection, "nickname"), GetValueName(value), indice+1, TypeToString(value->type), expected);
}

float IniFile::GetValueFloat(IniValue *value, UINT indice)
{
	if (m_textMode)
	{
		LPSTR scan = reinterpret_cast<LPSTR>(value);
		while (indice)
			indice--, scan = strchr(scan,',')+1;
		return (float) atof(scan);
	}
	else
	{
		value += indice;
		if (value->type == IniValue::vtFloat)
			return value->vFloat;
		else if (value->type == IniValue::vtInt)
			return (float) value->vInt;
		else
		{
			BadType(value-indice, indice, L"float");
			return 0.0;
		}
	}
}
