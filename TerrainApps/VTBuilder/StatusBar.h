//
// A custom status bar
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STATUSBARH
#define STATUSBARH

#include "Frame.h"

#include <wx/statusbr.h>

class MainFrame;

class MyStatusBar : public wxStatusBar
{
public:
	MyStatusBar(wxWindow *parent);
	virtual ~MyStatusBar();

    // event handlers
	void OnSize(wxSizeEvent& event);
	void SetTexts(MainFrame *frame);

	bool	m_bShowMinutes;
	LinearUnits		m_ShowVertUnits;

private:
	wxString FormatCoord(bool bGeo, double coord);

	enum
	{
		Field_Text,
		Field_Coord,
		Field_Zone,
		Field_Datum,
		Field_Mouse,
		Field_Height,
		Field_Max
	};

	DECLARE_EVENT_TABLE()
};

#endif
