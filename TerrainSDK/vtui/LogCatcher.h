//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/vtLog.h"

/** Catch wxWidgets logging message, direct them to the vtLog */
class LogCatcher : public wxLog
{
	void DoLogText(const wxChar *szString, time_t t)
	{
		VTLOG1(" wxLog: ");
		wxString str(szString);
		VTLOG1(str.ToUTF8());
		VTLOG1("\n");
	}
};
