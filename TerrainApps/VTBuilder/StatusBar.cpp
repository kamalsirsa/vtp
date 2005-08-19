//
// Status bar implementation
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"
#include "vtdata/vtLog.h"

#include "Frame.h"
#include "StatusBar.h"
#include "BuilderView.h"
#include "vtui/Helper.h"	// for FormatCoord


MyStatusBar::MyStatusBar(wxWindow *parent) : wxStatusBar(parent, -1)
{
	VTLOG(" Creating Status Bar.\n");
	static const int widths[Field_Max] =
	{
		-1,		// main message area
		46,		// Geo or short projection identifier
		65,		// Zone
		65,		// Datum
		200,	// Units
		208,	// Coordinates of cursor
		86		// Elevation under cursor
	};

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
//	m_checkbox->SetSize(rect.x + 2, rect.y + 2, rect.width - 4, rect.height - 4);
//	event.Skip();
}


wxString MyStatusBar::FormatCoord(bool bGeo, double coord)
{
	wxString2 str;
	if (bGeo)
	{
		str = ::FormatCoord(bGeo, coord, m_bShowMinutes);
	}
	else	// something meters-based
	{
		str.Printf(_T("%.0f"), coord);
	}
	return str;
}

void MyStatusBar::SetTexts(MainFrame *frame)
{
//	VTLOG(" StatusBar SetTexts: ");

	vtProjection &proj = frame->GetAtProjection();
	bool bGeo = (proj.IsGeographic() != 0);

	wxString2 str = proj.GetProjectionNameShort();
	SetStatusText(str, Field_Coord);

	int zone = proj.GetUTMZone();
	if (zone != 0)
		str.Printf(_T("Zone %d"), zone);
	else
		str = _T("");
	SetStatusText(str, Field_Zone);

	str = DatumToStringShort(proj.GetDatum());
	SetStatusText(str, Field_Datum);

	DPoint2 p;
	BuilderView *pView = frame->GetView();
	if (pView)
	{
		// Scale and units
		double scale = pView->GetScale();
		LinearUnits lu = proj.GetUnits();
		if (lu == LU_DEGREES)
			str.Printf(_T("1 Pixel = %.6lg "), 1.0/scale);
		else
			str.Printf(_T("1 Pixel = %.2lf "), 1.0/scale);
		str += GetLinearUnitName(lu);
		SetStatusText(str, Field_HUnits);

		pView->GetMouseLocation(p);
		str = _("Mouse");
		str += _T(": ");
		str += FormatCoord(bGeo, p.x);
		str += _T(", ");
		str += FormatCoord(bGeo, p.y);

		SetStatusText(str, Field_Mouse);
//		VTLOG(" '%s' ", (const char *)str);

		float height = frame->GetHeightFromTerrain(p);
		if (height == INVALID_ELEVATION)
			str = _T("");
		else
		{
			if (m_ShowVertUnits == LU_METERS)
				str.Printf(_T("%.2f m"), height);
			else
				str.Printf(_T("%.2f ft"), height / GetMetersPerUnit(m_ShowVertUnits));
		}
		SetStatusText(str, Field_Height);
	}
	else
	{
		SetStatusText(_("Mouse"), Field_Mouse);
		SetStatusText(_T(""), Field_Height);
	}
//	VTLOG(" Done.\n");
}

//
// Not sure why the event table must be down here, but it does - for WinZip
//
BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
EVT_SIZE(MyStatusBar::OnSize)
END_EVENT_TABLE()

