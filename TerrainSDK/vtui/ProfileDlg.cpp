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

// WDR: class implementations

//----------------------------------------------------------------------------
// ProfileDlg
//----------------------------------------------------------------------------

// WDR: event table for ProfileDlg

BEGIN_EVENT_TABLE(ProfileDlg, AutoDialog)
EVT_PAINT(ProfileDlg::OnPaint)
EVT_SIZE(ProfileDlg::OnSize)
END_EVENT_TABLE()

ProfileDlg::ProfileDlg( wxWindow *parent, wxWindowID id,
	const wxString& title, const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	m_callback = NULL;
	m_clientsize.Set(0, 0);
	m_bHavePoints = false;
	m_bHaveValues = false;
	// WDR: dialog function ColorMapDialogFunc for ProfileDlg
//	ColorMapDialogFunc( this, TRUE );
}

void ProfileDlg::DrawChart(wxDC& dc)
{
	if (!m_bHavePoints)
		return;

	if (!m_bHaveValues)
		GetValues();

	wxPen pen(*wxBLACK_PEN);
	dc.SetPen(pen);

	wxPoint *pts = new wxPoint[m_xrange];
	for (int i = 0; i < m_xrange; i++)
	{
		pts[i].x = 10 + i;
		pts[i].y = m_clientsize.y - 10
			- (m_values[i] - m_fMin) / m_fRange * m_yrange;
	}
	dc.DrawLines(m_xrange, pts);
	delete [] pts;
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

	m_xrange = m_clientsize.x - 20;
	if (m_xrange < 2) m_xrange = 2;
	m_yrange = m_clientsize.y - 20;
	if (m_yrange < 2) m_yrange = 2;

	m_fMin = 1E9;
	m_fMax = -1E9;
	m_values.resize(m_xrange);
	m_callback->Begin();
	DPoint2 p;
	for (int i = 0; i < m_xrange; i++)
	{
		double ratio = (double)i / (m_xrange-1);
		p = m_p1 + (m_p2 - m_p1) * ratio;
		m_values[i] = m_callback->GetElevation(p);
		if (m_values[i] < m_fMin) m_fMin = m_values[i];
		if (m_values[i] > m_fMax) m_fMax = m_values[i];
	}
	m_fRange = m_fMax - m_fMin;
	m_bHaveValues = true;
}

// WDR: handler implementations for ProfileDlg

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
}
