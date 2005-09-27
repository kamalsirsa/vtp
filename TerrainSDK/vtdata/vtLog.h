//
// Log - simple message logging to a file.
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLOG_H
#define VTLOG_H

#include "config_vtdata.h"
#include <stdio.h>

class vtLog
{
public:
	vtLog();
	~vtLog();

	void _StartLog(const char *fname);
	void _Log(const char *str);
	void _Log(char ch);
	void Printf(const char *pFormat, ...);

#if SUPPORT_WSTRING
	void _Log(const wchar_t *str);
	void Printf(const wchar_t *pFormat, ...);
#endif

private:
	FILE *m_log;
};

extern vtLog g_Log;
#define VTLOG	g_Log.Printf
#define VTLOG1	g_Log._Log		// for simple strings, takes 1 argument

#endif // VTLOG_H

