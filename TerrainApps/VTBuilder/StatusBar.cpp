//
// Status bar implementation
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "vtdata/ElevationGrid.h"
#include "StatusBar.h"
#include "BuilderView.h"
#include "Helper.h"

MyStatusBar::MyStatusBar(wxWindow *parent)
           : wxStatusBar(parent, -1)
{
	static const int widths[Field_Max] = { -1, 38, 50, 65, 152, 68 };

	SetFieldsCount(Field_Max);
	SetStatusWidths(Field_Max, widths);

	m_bShowMinutes = false;
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
	if (bGeo)
	{
		// display degrees
		if (m_bShowMinutes)
			return DegreeToString(coord);
		else
		{
			wxString str;
			str = wxString::Format("%.5f", coord);
			return str;
		}
	}
	else	// something meters-based
	{
		wxString str;
		str = wxString::Format("%.0f", coord);
		return str;
	}
}

void MyStatusBar::SetTexts(MainFrame *frame)
{
	vtProjection proj;
	frame->GetProjection(proj);
	bool bGeo = (proj.IsGeographic() != 0);

	SetStatusText(proj.GetProjectionNameShort(), Field_Coord);

	wxString str;
	if (proj.IsUTM())
		str = wxString::Format("Zone %d", proj.GetUTMZone());
	else
		str = "";
	SetStatusText(str, Field_Zone);

	SetStatusText(datumToString(proj.GetDatum()), Field_Datum);

	double lx, ly;
	BuilderView *pView = frame->GetView();
	if (pView)
	{
		pView->GetMouseLocation(lx, ly);
		str = wxString::Format("Mouse: %s, %s", FormatCoord(bGeo, lx), FormatCoord(bGeo, ly));
		SetStatusText(str, Field_Mouse);

		float height = frame->GetHeightFromTerrain(lx, ly);
		if (height == INVALID_ELEVATION)
			str = "";
		else
			str = wxString::Format( "%.2f m", height);
		SetStatusText(str, Field_Height);
	}
	else
	{
		SetStatusText("Mouse", Field_Mouse);
		SetStatusText("", Field_Height);
	}
}

//
// Not sure why the event table must be down here, but it does - for WinZip
//
BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
	EVT_SIZE(MyStatusBar::OnSize)
END_EVENT_TABLE()


