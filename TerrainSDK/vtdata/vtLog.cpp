//
// Log - simple message logging to a file.
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "cpl_error.h"	// From GDAL/include
#include "vtString.h"
#include "vtLog.h"
#include <stdarg.h>

#ifndef __DARWIN_OSX__
#include <wchar.h>		// for fputws()
#endif

#ifdef _MSC_VER
#include <windows.h>	// for OutputDebugString, unfortunately
#endif

vtLog g_Log;

void CPL_STDCALL cpl_error_handler(CPLErr eErrClass, int err_no, const char *msg)
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
	// also send to the console, for those console-mode developers!
	fputs(msg, stdout);
}

void vtLog::_Log(char ch)
{
	if (m_log)
	{
		fputc(ch, m_log);
		fflush(m_log);
	}
#ifdef _MSC_VER
	char str[2];
	str[0] = ch;
	str[1] = 0;
	OutputDebugStringA(str);
#endif
	// also send to the console, for those console-mode developers!
	fputc(ch, stdout);
}

void vtLog::Printf(const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	char ach[2048];
	vsprintf(ach, pFormat, va);

	_Log(ach);
}


#if SUPPORT_WSTRING

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

void vtLog::Printf(const wchar_t *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	// Use wide characters
	wchar_t ach[1024];
#ifdef _MSC_VER
	vswprintf(ach, pFormat, va);
#else
	// apparently on non-MSVC platforms this takes 4 arguments (safer)
	vswprintf(ach, 1024, pFormat, va);
#endif

	_Log(ach);
}

#endif // SUPPORT_WSTRING

