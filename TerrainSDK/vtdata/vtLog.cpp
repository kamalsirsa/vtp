//
// Log - simple message logging to a file.
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtLog.h"
#include <stdarg.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

vtLog::vtLog()
{
	m_log = NULL;
}

void vtLog::_StartLog(const char *fname)
{
	m_log = fopen(fname, "wb");
}

void vtLog::_Log(const char *str)
{
	if (m_log)
	{
		fputs(str, m_log);
		fflush(m_log);
	}
#ifdef _MSC_VER
	OutputDebugString(str);
#endif
}

void vtLog::Printf(const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	char ach[1024];
	vsprintf(ach, pFormat, va);

	_Log(ach);
}

vtLog g_Log;
