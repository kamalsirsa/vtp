//
// Log - simple message logging to a file.
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "cpl_error.h"	// From GDAL/include
#include "vtString.h"
#include "vtLog.h"
#include <stdarg.h>
#include <wchar.h>		// for fputws()

#ifdef _MSC_VER
#include <windows.h>	// for OutputDebugString, unfortunately
#endif

vtLog g_Log;

void cpl_error_handler(CPLErr eErrClass, int err_no, const char *msg)
{
	if (eErrClass == CE_Debug)
		g_Log._Log("CPL Debug: ");
	else if (eErrClass == CE_Warning)
		g_Log.Printf("CPL Warning %d: ", err_no);
	else
		g_Log.Printf("CPL Error %d: ", err_no);
	g_Log._Log(msg);
	g_Log._Log("\n");
}

vtLog::vtLog()
{
	m_log = NULL;
}

vtLog::~vtLog()
{
	if (m_log)
	{
		CPLPopErrorHandler();
		fclose(m_log);
		m_log = NULL;
	}
}

void vtLog::_StartLog(const char *fname)
{
	m_log = fopen(fname, "wb");
	CPLPushErrorHandler(cpl_error_handler);
}

void vtLog::_Log(const char *msg)
{
	if (m_log)
	{
		fputs(msg, m_log);
		fflush(m_log);
	}
#ifdef _MSC_VER
	OutputDebugStringA(msg);
#endif
}

void vtLog::_Log(const wchar_t *msg)
{
	if (m_log)
	{
		// it is not so useful to write wide characters to the file, which
		// otherwise contains 8-bit text, so convert back first
//		fputws(msg, m_log);
		wstring2 str = msg;
		fputs(str.eb_str(), m_log);
		fflush(m_log);
	}
#ifdef _MSC_VER
	OutputDebugStringW(msg);
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

void vtLog::Printf(const wchar_t *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

#if defined (__DARWIN_OSX__) || defined (macintosh)
	// Mac seems to not have all the ANSI functions like vswprintf, so
	//  convert the format string to 8-bit and use vsprintf instead.
	char ach[1024];
	wstring2 strFormat = pFormat;
	vsprintf(ach, strFormat.eb_str(), va);
#else
	// Use wide characters
	wchar_t ach[1024];
#ifdef _MSC_VER
	vswprintf(ach, pFormat, va);
#else
	// apparently on non-MSVC platforms this takes 4 arguments (safer)
	vswprintf(ach, 1024, pFormat, va);
#endif
#endif

	_Log(ach);
}
