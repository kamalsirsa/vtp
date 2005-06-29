//
// Name: ProfileDlg.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ProfileDlg.h"

#define MARGIN_LEFT   60
#define MARGIN_BOTTOM 110

// WDR: class implementations

//----------------------------------------------------------------------------
// ProfileDlg
//----------------------------------------------------------------------------

// WDR: event table for ProfileDlg

BEGIN_EVENT_TABLE(ProfileDlg, AutoDialog)
EVT_PAINT(ProfileDlg::OnPaint)
EVT_SIZE(ProfileDlg::OnSize)
EVT_LEFT_DOWN(ProfileDlg::OnLeftDown)
EVT_LEFT_UP(ProfileDlg::OnLeftUp)
EVT_MOTION(ProfileDlg::OnMouseMove)
	EVT_CHECKBOX( ID_LINE_OF_SIGHT, ProfileDlg::OnLineOfSight )
	EVT_TEXT( ID_HEIGHT1, ProfileDlg::OnHeight1 )
	EVT_TEXT( ID_HEIGHT2, ProfileDlg::OnHeight2 )
END_EVENT_TABLE()


ProfileDlg::ProfileDlg( wxWindow *parent, wxWindowID id,
	const wxString& title, const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style | wxRESIZE_BORDER | wxMAXIMIZE)
{
	m_callback = NULL;
	m_clientsize.Set(0, 0);
	m_bHavePoints = false;
	m_bHaveValues = false;
	m_bMouseOnLine = false;
	m_bLeftButton = false;
	m_bLineOfSight = false;
	m_fHeight1 = 1;
	m_fHeight2 = 1;

	// WDR: dialog function ColorMapDialogFunc for ProfileDlg
	ProfileDialogFunc( this, TRUE );

	AddNumValidator(ID_HEIGHT1, &m_fHeight1);
	AddNumValidator(ID_HEIGHT2, &m_fHeight2);

	GetHeight1()->Enable(false);
	GetHeight2()->Enable(false);

	SetBackgroundColour(wxColour(255,255,255));
}

void ProfileDlg::SetPoints(const DPoint2 &p1, const DPoint2 &p2)
{
	m_p1 = p1;
	m_p2 = p2;
	m_bHavePoints = true;
	m_bHaveValues = false;
	Refresh();
}

void ProfileDlg::SetCallback(ProfileCallback *callback)
{
	m_callback = callback;
}

void ProfileDlg::GetValues()
{
	if (!m_callback)
		return;

	m_clientsize = GetClientSize();

	m_xrange = m_clientsize.x - MARGIN_LEFT - 10;
	if (m_xrange < 2) m_xrange = 2;
	m_yrange = m_clientsize.y - MARGIN_BOTTOM - 10;
	if (m_yrange < 2) m_yrange = 2;

	m_fTotalDist = (m_p2 - m_p1).Length();
	m_fMin = 1E9;
	m_fMax = -1E9;
	m_values.resize(m_xrange);
	m_callback->Begin();
	DPoint2 p;
	m_bHaveValidData = false;
	m_bHaveInvalid = false;
	for (int i = 0; i < m_xrange; i++)
	{
		double ratio = (double)i / (m_xrange-1);
		p = m_p1 + (m_p2 - m_p1) * ratio;

		float f = m_callback->GetElevation(p);
		m_values[i] = f;

		if (f == INVALID_ELEVATION)
			m_bHaveInvalid = true;
		else
		{
			m_bHaveValidData  = true;
			if (f < m_fMin)
			{
				m_fMin = f;
				m_iMin = i;
				m_fMinDist = ratio * m_fTotalDist;
			}
			if (f > m_fMax)
			{
				m_fMax = f;
				m_iMax = i;
				m_fMaxDist = ratio * m_fTotalDist;
			}
		}
	}
	m_fRange = m_fMax - m_fMin;
	m_bHaveValues = true;
	m_bMouseOnLine = false;
}

void ProfileDlg::MakePoint(wxPoint &p, int i, float value)
{
	p.x = m_base.x + i;
	p.y = m_base.y - (value - m_fMin) / m_fDrawRange * m_yrange;
}

void ProfileDlg::DrawChart(wxDC& dc)
{
	if (!m_bHavePoints)
	{
		GetText()->SetValue(_("No Endpoints"));
		return;
	}

	if (!m_bHaveValues)
		GetValues();

	wxPen pen1(*wxMEDIUM_GREY_PEN);
	pen1.SetWidth(2);
	wxPen pen2(*wxLIGHT_GREY_PEN);
	pen2.SetStyle(wxDOT);
	wxPen pen3(wxColour(0,128,0));  // dark green

	wxFont font(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	dc.SetFont(font);

	m_base.x = MARGIN_LEFT;
	m_base.y = m_clientsize.y - MARGIN_BOTTOM;

	m_fDrawRange = m_fRange;
	m_bValidLine = false;
	if (m_bLineOfSight)
	{
		m_fHeightAtStart = m_values[0];
		m_fHeightAtEnd = m_values[m_xrange - 1];
		if (m_fHeightAtStart != INVALID_ELEVATION && m_fHeightAtEnd != INVALID_ELEVATION)
		{
			m_bValidLine = true;
			m_fHeightAtStart += m_fHeight1;
			m_fHeightAtEnd += m_fHeight2;
			float DrawMax;
			DrawMax = std::max(m_fMax, m_fHeightAtStart);
			DrawMax = std::max(DrawMax, m_fHeightAtEnd);
			m_fDrawRange = DrawMax - m_fMin;
		}
	}

	dc.SetPen(pen1);
	dc.DrawLine(m_base.x, m_base.y, m_base.x + m_xrange, m_base.y);
	dc.DrawLine(m_base.x, m_base.y, m_base.x, m_base.y - m_yrange);
//  dc.DrawLine(m_base.x + m_xrange, m_base.y, m_base.x + m_xrange, m_base.y - m_yrange);

	if (!m_bHaveValidData)
	{
		GetText()->SetValue(_("No Data"));
		return;
	}

	// Draw tick marks
	int numticks, x, y, tick_spacing = 32;
	wxString str;
	int w, h;
	int i;

	// Vertical ticks
	numticks = (m_yrange / tick_spacing)+2;
	for (i = 0; i < numticks; i++)
	{
		y = m_base.y - (i * m_yrange / (numticks-1));

		if (i > 0)
		{
			dc.SetPen(pen2);
			dc.DrawLine(m_base.x, y, m_base.x + m_xrange, y);
		}

		dc.SetPen(pen1);
		dc.DrawLine(m_base.x - 5, y, m_base.x + 5, y);

		str.Printf(_T("%5.1f"), m_fMin + (m_fDrawRange / (numticks-1) * i));
		dc.GetTextExtent(str, &w, &h);
		dc.DrawText(str, MARGIN_LEFT - w - 8, y-(h/2));
	}
	// Horizontal ticks
	numticks = (m_xrange / tick_spacing)+2;
	for (i = 0; i < numticks; i++)
	{
		x = m_base.x + (i * m_xrange / (numticks-1));

//  if (i > 0)
//  {
//	dc.SetPen(pen2);
//	dc.DrawLine(x, m_base.y, x, m_base.y - m_yrange);
//  }

		dc.SetPen(pen1);
		dc.DrawLine(x, m_base.y - 5, x, m_base.y + 5);

		str.Printf(_T("%5.1f"), m_fTotalDist / (numticks-1) * i);
		dc.GetTextExtent(str, &w, &h);
		dc.DrawRotatedText(str, x-(h/2), m_base.y + w + 8, 90);
	}

	// Draw surface line
	dc.SetPen(pen3);
	wxPoint p1, p2;
	if (m_bHaveInvalid)
	{
		// slow way, one datapoint at a time
		for (i = 0; i < m_xrange-1; i++)
		{
			float v1 = m_values[i];
			float v2 = m_values[i+1];
			if (v1 == INVALID_ELEVATION || v2 == INVALID_ELEVATION)
				continue;
			MakePoint(p1, i, v1);
			MakePoint(p2, i+1, v2);
			dc.DrawLine(p1, p2);
		}
	}
	else
	{
		// faster way, pass an array
		wxPoint *pts = new wxPoint[m_xrange];
		for (i = 0; i < m_xrange; i++)
		{
			MakePoint(pts[i], i, m_values[i]);
		}
		dc.DrawLines(m_xrange, pts);
		delete [] pts;
	}

	if (m_bValidLine)
	{
		wxPen orange(wxColour(255,128,0));
		dc.SetPen(orange);
		MakePoint(p1, 0, m_values[0] + m_fHeight1);
		MakePoint(p2, m_xrange - 1, m_values[m_xrange - 1] + m_fHeight2);
		dc.DrawLine(p1, p2);
	}

	// Draw min/max/mouse markers
	wxPen nopen;
	nopen.SetStyle(wxTRANSPARENT);
	dc.SetPen(nopen);

	wxBrush brush1(wxColour(0,0,255));
	dc.SetBrush(brush1);
	MakePoint(p1, m_iMin, m_fMin);
	dc.DrawCircle(p1, 5);

	wxBrush brush2(wxColour(255,0,0));
	dc.SetBrush(brush2);
	MakePoint(p1, m_iMax, m_fMax);
	dc.DrawCircle(p1, 5);

	if (m_bMouseOnLine)
	{
		wxBrush brush3(wxColour(0,255,0));
		dc.SetBrush(brush3);
		MakePoint(p1, m_iMouse, m_fMouse);
		dc.DrawCircle(p1, 5);
	}

	m_bIntersectsGround = false;
	if (m_bValidLine)
	{
		float diff = m_fHeightAtEnd - m_fHeightAtStart;
		for (i = 0; i < m_xrange; i++)
		{
			float fLineHeight = m_fHeightAtStart + diff * i / m_xrange;
			if (fLineHeight < m_values[i])
			{
				// line of sight intersects the ground
				m_bIntersectsGround = true;
				m_fIntersectHeight = m_values[i];
				m_fIntersectDistance = (float)i / (m_xrange-1) * m_fTotalDist;

				wxBrush brush3(wxColour(255,128,0));
				dc.SetBrush(brush3);
				MakePoint(p1, i, m_values[i]);
				dc.DrawCircle(p1, 5);
				break;
			}
		}
	}

	// Also update message text
	UpdateMessageText();
}

void ProfileDlg::UpdateMessageText()
{
	wxString str, str2;

	str.Printf(_("Minimum: %.2f m at distance %.1f\nMaximum: %.2f m at distance %.1f"),
		m_fMin, m_fMinDist, m_fMax, m_fMaxDist);

	if (m_bMouseOnLine)
	{
		str += _T("\n");
		str2.Printf(_("Mouse: %.2f m at distance %.1f"), m_fMouse, m_fMouseDist);
		str += str2;
		if (m_bHaveSlope)
		{
			str += _T(", ");
			str2.Printf(_("Slope %.3f (%.1f degrees)"), m_fSlope,
				atan(m_fSlope) * 180 / PId);
			str += str2;
		}
	}
	if (m_bValidLine && m_bIntersectsGround)
	{
		str += _T("\n");
		str2.Printf(_("Intersects ground at height %.2f, distance %.1f"),
			m_fIntersectHeight, m_fIntersectDistance);
		str += str2;
	}
	GetText()->SetValue(str);
}

// WDR: handler implementations for ProfileDlg

void ProfileDlg::OnHeight2( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Refresh();
}

void ProfileDlg::OnHeight1( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Refresh();
}

// This calls OnDraw, having adjusted the origin according to the current
// scroll position
void ProfileDlg::OnPaint(wxPaintEvent &event)
{
	// don't use m_targetWindow here, this is always called for ourselves
	wxPaintDC dc(this);
	OnDraw(dc);
}

void ProfileDlg::OnDraw(wxDC& dc)  // overridden to draw this view
{
	DrawChart(dc);
}

void ProfileDlg::OnSize(wxSizeEvent& event)
{
	wxSize size = GetClientSize();
	if (size != m_clientsize)
	{
		m_clientsize = size;
		m_bHaveValues = false;
		Refresh();
	}
	event.Skip();
}

void ProfileDlg::OnLeftDown(wxMouseEvent& event)
{
	m_bLeftButton = true;
	OnMouseMove(event);
}
void ProfileDlg::OnLeftUp(wxMouseEvent& event)
{
	m_bLeftButton = false;
}

void ProfileDlg::OnMouseMove(wxMouseEvent& event)
{
	if (!m_bHaveValidData || !m_bHaveValues || !m_bLeftButton)
	{
		m_bMouseOnLine = false;
		return;
	}
	wxPoint point = event.GetPosition();
	if (point.x > m_base.x && point.x < m_base.x + m_xrange - 1 &&
		point.y < m_base.y && point.y > m_base.x - m_yrange)
	{
		int offset = point.x - m_base.x;
		m_fMouse = m_values[offset];

		if (m_fMouse != INVALID_ELEVATION)
		{
			m_fMouseDist = (float)offset / m_xrange * m_fTotalDist;
			m_iMouse = offset;
			m_bMouseOnLine = true;
			Refresh();

			// calculate slope
			float v2 = m_values[offset+1];
			if (v2 != INVALID_ELEVATION)
			{
				m_bHaveSlope = true;
				m_fSlope = (v2 - m_fMouse) / (m_fTotalDist / m_xrange);
			}
		}
	}
	else
	{
		// mouse out
		if (m_bMouseOnLine)
			Refresh();
		m_bMouseOnLine = false;
	}
}

void ProfileDlg::OnLineOfSight( wxCommandEvent &event )
{
	m_bLineOfSight = event.IsChecked();
	Refresh();
	GetHeight1()->Enable(m_bLineOfSight);
	GetHeight2()->Enable(m_bLineOfSight);
}

