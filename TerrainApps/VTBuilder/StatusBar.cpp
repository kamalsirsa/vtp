//
// Status bar implementation
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"
#include "vtdata/vtLog.h"

#include "StatusBar.h"
#include "BuilderView.h"
#include "Helper.h"

MyStatusBar::MyStatusBar(wxWindow *parent) : wxStatusBar(parent, -1)
{
	VTLOG(" Creating Status Bar.\n");
	static const int widths[Field_Max] = { -1, 38, 50, 65, 52, 170, 76 };

	SetFieldsCount(Field_Max);
	SetStatusWidths(Field_Max, widths);

	m_bShowMinutes = false;
	m_ShowVertUnits = LU_METERS;
}

MyStatusBar::~MyStatusBar()
{
}

void MyStatusBar::OnSize(wxSizeEvent& event)
{
	// could do resizing of status bar elements here if needed
//    m_checkbox->SetSize(rect.x + 2, rect.y + 2, rect.width - 4, rect.height - 4);
//    event.Skip();
}


wxString MyStatusBar::FormatCoord(bool bGeo, double coord)
{
	wxString str;
	if (bGeo)
	{
		str = ::FormatCoord(bGeo, coord, m_bShowMinutes);
	}
	else	// something meters-based
	{
		str.Printf("%.0f", coord);
	}
	return str;
}

void MyStatusBar::SetTexts(MainFrame *frame)
{
//	VTLOG(" StatusBar SetTexts: ");

	vtProjection &proj = frame->GetAtProjection();
	bool bGeo = (proj.IsGeographic() != 0);

	SetStatusText(proj.GetProjectionNameShort(), Field_Coord);

	wxString str;
	int zone = proj.GetUTMZone();
	if (zone != 0)
		str.Printf("Zone %d", zone);
	else
		str = "";
	SetStatusText(str, Field_Zone);

	SetStatusText(datumToStringShort(proj.GetDatum()), Field_Datum);

	LinearUnits lu = proj.GetUnits();
	const char *text;
	switch (lu)
	{
	case LU_DEGREES:  text = "Degrees";    break;
	case LU_METERS:	  text = "Meters";     break;
	case LU_FEET_INT: text = "Feet"; break;
	case LU_FEET_US:  text = "Feet (US)";  break;
	}
	SetStatusText(text, Field_HUnits);

	DPoint2 p;
	BuilderView *pView = frame->GetView();
	if (pView)
	{
		pView->GetMouseLocation(p);
		str.Printf("Mouse: %s, %s",
				(const char *) FormatCoord(bGeo, p.x),
				(const char *) FormatCoord(bGeo, p.y));
		SetStatusText(str, Field_Mouse);
//		VTLOG(" '%s' ", (const char *)str);

		float height = frame->GetHeightFromTerrain(p);
		if (height == INVALID_ELEVATION)
			str = "";
		else
		{
			if (m_ShowVertUnits == LU_METERS)
				str.Printf("%.2f m", height);
			else
				str.Printf("%.2f ft", height / GetMetersPerUnit(m_ShowVertUnits));
		}
		SetStatusText(str, Field_Height);
	}
	else
	{
		SetStatusText("Mouse", Field_Mouse);
		SetStatusText("", Field_Height);
	}
//	VTLOG(" Done.\n");
}

//
// Not sure why the event table must be down here, but it does - for WinZip
//
BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
EVT_SIZE(MyStatusBar::OnSize)
END_EVENT_TABLE()

