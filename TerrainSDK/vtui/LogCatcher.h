//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/vtLog.h"

/** Catch wxWidgets logging message, direct them to the vtLog */
class LogCatcher : public wxLog
{
	void DoLogText(const wxString& msg)
	{
		VTLOG1(" wxLog: ");
		VTLOG1(msg.ToUTF8());
		VTLOG1("\n");
	}

	virtual void DoLogString(const wxChar *msg, time_t timestamp)
	{
	    wxString message(msg);
	    DoLogText(message);
	}
};
