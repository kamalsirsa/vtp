//
// LODDlg.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "LODDlg.h"
#include "EnviroFrame.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// LODDlg
//----------------------------------------------------------------------------

// WDR: event table for LODDlg

BEGIN_EVENT_TABLE(LODDlg,wxDialog)
	EVT_SPIN_UP( ID_TARGET, LODDlg::OnSpinTargetUp )
	EVT_SPIN_DOWN( ID_TARGET, LODDlg::OnSpinTargetDown )
END_EVENT_TABLE()

LODDlg::LODDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function LODDialogFunc for LODDlg
	LODDialogFunc( this, TRUE ); 

	m_iTarget = 0;
}

void LODDlg::Refresh(float res0, float res, float res1, int target,
					 int count, int cache_size, int cache_used,
					 int disk_loads, int cache_hits)
{
	// don't bother updating if window isn't shown
	if (!IsShown())
		return;

	wxString str;
	if (target != m_iTarget)
	{
		m_iTarget = target;
		str.Printf(_T("%d"), m_iTarget);
		GetTarget()->SetValue(str);
	}

	str.Printf(_T("%d"), count);
	GetCurrent()->SetValue(str);

	if (cache_size != m_iCacheSize || cache_used != m_iCacheUsed)
	{
		m_iCacheSize = cache_size;
		m_iCacheUsed = cache_used;
		if (m_iCacheSize == -1)
			str = _T("n/a");
		else
			str.Printf(_T("%d / %d"), m_iCacheUsed/1024, m_iCacheSize/1024);
		GetCacheUsed()->SetValue(str);
	}

	if (disk_loads != m_iDiskLoads)
	{
		m_iDiskLoads = disk_loads;
		if (m_iDiskLoads == -1)
			str = _T("n/a");
		else
			str.Printf(_T("%d"), m_iDiskLoads);
		GetTileLoads()->SetValue(str);
	}

	if (cache_hits != m_iCacheHits)
	{
		m_iCacheHits = cache_hits;
		if (m_iCacheHits == -1)
			str = _T("n/a");
		else
			str.Printf(_T("%d"), m_iCacheHits);
		GetTileHits()->SetValue(str);
	}

	// Now draw the chart
	DrawChart(res0, res, res1, target, count);
}

void LODDlg::DrawChart(float res0, float res, float res1, int target, int count)
{
	int ires = res;
	int imax = target * 2;
	int itarget = target * 300 / imax;
	int icount = count * 300 / imax;
	int range = (target / 10);
	int irange1 = (target-range) * 300 / imax;
	int irange2 = (target+range) * 300 / imax;

	wxPanel *panel = GetPanel1();
	wxClientDC dc(panel);
	PrepareDC(dc);
	dc.Clear();

	if (res0 != -1)
	{
		int ires0 = res0;
		int ires1 = res1;

		wxPen pen1(wxColour(255,0,0), 1, wxSOLID);
		dc.SetPen(pen1);

		dc.DrawLine(ires0-10, 0, ires0, 10);
		dc.DrawLine(ires0, 10, ires0-10, 20);

		dc.DrawLine(ires1+10, 0, ires1, 10);
		dc.DrawLine(ires1, 10, ires1+10, 20);
	}

	wxPen pen2(wxColour(0,0,255), 3, wxSOLID);
	dc.SetPen(pen2);

	dc.DrawLine(ires, 0, ires, 20);

	wxPen pen3(wxColour(0,128,0), 1, wxSOLID);
	dc.SetPen(pen3);

	dc.DrawLine(irange1, 20, irange1, 40);
	dc.DrawLine(itarget, 20, itarget, 40);
	dc.DrawLine(irange2, 20, irange2, 40);
	dc.DrawLine(irange1, 30, irange2, 30);

	wxPen pen4(wxColour(255,0,255), 3, wxSOLID);
	dc.SetPen(pen4);

	dc.DrawLine(icount, 20, icount, 40);
}

// WDR: handler implementations for LODDlg

void LODDlg::OnSpinTargetUp( wxSpinEvent &event )
{
	m_pFrame->ChangeTerrainDetail(true);
	event.Veto();
}

void LODDlg::OnSpinTargetDown( wxSpinEvent &event )
{
	m_pFrame->ChangeTerrainDetail(false);
	event.Veto();
}

