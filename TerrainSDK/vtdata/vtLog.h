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

	void _StartLog(const char *fname);
	void _Log(const char *str);
	void Printf(const char *pFormat, ...);

private:
	FILE *m_log;
};

extern vtLog g_Log;
#define VTLOG	g_Log.Printf

#endif // VTLOG_H

