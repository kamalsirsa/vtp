//
// Log - simple message logging to a file.
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLOG_H
#define VTLOG_H

#include <stdio.h>

class vtLog
{
public:
	vtLog();
	~vtLog();

	void _StartLog(const char *fname);
	void _Log(const char *str);
	void _Log(const wchar_t *str);
	void Printf(const char *pFormat, ...);
	void Printf(const wchar_t *pFormat, ...);

private:
	FILE *m_log;
};

extern vtLog g_Log;
#define VTLOG	g_Log.Printf

#endif // VTLOG_H

