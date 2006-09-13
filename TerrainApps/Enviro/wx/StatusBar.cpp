//
// Status bar implementation for Enviro
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "vtlib/vtlib.h"

#include "StatusBar.h"

#include "EnviroGUI.h"


MyStatusBar::MyStatusBar(wxWindow *parent) :
	wxStatusBar(parent, -1)
{
	VTLOG(" Creating Status Bar.\n");
	static const int widths[Field_Max] =
	{
		-1,		// main message area
		60,		// Fps
		220,	// Coordinates of cursor
		160		// Value of thing under cursor, e.g. Elevation or Terrain
	};

	SetFieldsCount(Field_Max);
	SetStatusWidths(Field_Max, widths);
}

void MyStatusBar::UpdateText()
{
	vtString str;
	wxString ws;

	str = g_App.GetMessage();
	if (str != "")
	{
		ws = wxString(str, wxConvUTF8);
		SetStatusText(ws, Field_Text);
	}

	str = g_App.GetStatusString(0);
	ws = wxString(str, wxConvUTF8);
	SetStatusText(ws, Field_Fps);

	str = g_App.GetStatusString(1);
	ws = wxString(str, wxConvUTF8);
	SetStatusText(ws, Field_Cursor);

	str = g_App.GetStatusString(2);
	ws = wxString(str, wxConvUTF8);
	SetStatusText(ws, Field_CursorVal);
}

//
// Not sure why the event table must be down here, but it does - for WinZip
//
BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
//EVT_SIZE(MyStatusBar::OnSize)
END_EVENT_TABLE()

