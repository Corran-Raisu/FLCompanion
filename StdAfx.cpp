// stdafx.cpp : source file that includes just the standard includes
//	FLCompanion.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#define FMTNUM_USE_CENT 1

void FormatNumber(TCHAR* buffer, double value, UINT flags)
{
	_stprintf(buffer, _T("$%.2f"), value);
	size_t len = _tcslen(buffer);
	LPTSTR scan = buffer+len-1;
	len = (len-5)*4/3;
	LPTSTR scan2 = buffer+5+len;
	*scan2-- = '\0';
	*scan2-- = *scan--;
	*scan2-- = *scan--;
	*scan2-- = *scan--;
	len = 1;
	while (scan >= buffer+2)
	{
		*scan2-- = *scan--;
		if ((++len & 3) == 0)
			len++, *scan2-- = ',';
	}
}

void FormatNumber(TCHAR* buffer, long value, UINT flags)
{
	_stprintf(buffer, _T("$%d"), value);
	size_t len = _tcslen(buffer);
	LPTSTR scan = buffer+len-1;
	len = (len-2)*4/3;
	LPTSTR scan2 = buffer+2+len;
	*scan2-- = '\0';
	len = 1;
	while (scan >= buffer+2)
	{
		*scan2-- = *scan--;
		if ((++len & 3) == 0)
			len++, *scan2-- = ',';
	}
}
