//
// LODDlg.cpp
//
// Copyright (c) 2005-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/TiledGeom.h"

#include "LODDlg.h"
#include "EnviroFrame.h"

#include "mini.h"
#include "miniload.hpp"
#include "datacloud.hpp"

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

	GetTileStatus()->SetValue(_T("No paging threads"));

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
	int ires = (int) res;
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
		int ires0 = (int) res0;
		int ires1 = (int) res1;

		wxPen pen1(wxColour(255,0,0), 1, wxSOLID);	// red
		dc.SetPen(pen1);

		dc.DrawLine(ires0-10, 0, ires0, 10);
		dc.DrawLine(ires0, 10, ires0-10, 20);

		dc.DrawLine(ires1+10, 0, ires1, 10);
		dc.DrawLine(ires1, 10, ires1+10, 20);
	}

	wxPen pen2(wxColour(0,0,255), 3, wxSOLID);	// blue
	dc.SetPen(pen2);

	dc.DrawLine(ires, 0, ires, 20);

	wxPen pen3(wxColour(0,128,0), 1, wxSOLID);	// green
	dc.SetPen(pen3);

	dc.DrawLine(irange1-10, 20, irange1, 30);
	dc.DrawLine(irange1, 30, irange1-10, 40);

	dc.DrawLine(itarget, 20, itarget, 40);

	dc.DrawLine(irange2+10, 20, irange2, 30);
	dc.DrawLine(irange2, 30, irange2+10, 40);

	dc.DrawLine(irange1, 30, irange2, 30);

	wxPen pen4(wxColour(255,0,255), 3, wxSOLID);	// purple
	dc.SetPen(pen4);

	dc.DrawLine(icount, 20, icount, 40);
}

void LODDlg::DrawTilesetState(vtTiledGeom *tg, vtCamera *cam)
{
	wxPanel *panel = GetPanel2();
	wxClientDC dc(panel);
	PrepareDC(dc);
	dc.Clear();

	//wxPen pen3(wxColour(0,128,0), 1, wxSOLID);	// green
	//dc.SetPen(pen3);
	wxBrush b1(wxColour(255,0,0), wxSOLID);
	wxBrush b2(wxColour(255,128,0), wxSOLID);
	wxBrush b3(wxColour(255,255,0), wxSOLID);
	wxBrush b4(wxColour(0,255,0), wxSOLID);
	wxBrush b5(wxColour(0,255,255), wxSOLID);
	wxBrush b6(wxColour(0,0,255), wxSOLID);
	wxBrush b7(wxColour(255,0,255), wxSOLID);
	wxBrush bwhite(wxColour(255,255,255), wxSOLID);

	wxPen p1(wxColour(255,0,0), 2, wxSOLID);
	wxPen p2(wxColour(255,128,0), 2, wxSOLID);
	wxPen p3(wxColour(255,255,0), 2, wxSOLID);
	wxPen p4(wxColour(0,255,0), 2, wxSOLID);
	wxPen p5(wxColour(0,255,255), 2, wxSOLID);
	wxPen p6(wxColour(0,0,255), 2, wxSOLID);
	wxPen p7(wxColour(255,0,255), 2, wxSOLID);
	wxPen pwhite(wxColour(255,255,255), 2, wxSOLID);
	wxPen pblack(wxColour(0,0,0), 1, wxSOLID);

	// draw rectangles for texture state
	minitile *mt = tg->GetMiniTile();
	wxSize size = panel->GetSize();
	int border = 20;
	int sx = (size.x - border*2) / tg->cols;
	int sy = (size.y - border*2) / tg->rows;
	for (int i = 0; i < tg->cols; i++)
	{
		for (int j = 0; j < tg->rows; j++)
		{
			int t = mt->gettexw(i,j);
			switch (t)
			{
			case 64: dc.SetBrush(b1); break;
			case 128: dc.SetBrush(b2); break;
			case 256: dc.SetBrush(b3); break;
			case 512: dc.SetBrush(b4); break;
			case 1024: dc.SetBrush(b5); break;
			case 2048: dc.SetBrush(b6); break;
			default: dc.SetBrush(bwhite); break;
			}
			/*
			int s = mt->getsize(i,j);
			switch (s)
			{
			case 65: dc.SetPen(p1); break;
			case 129: dc.SetPen(p2); break;
			case 257: dc.SetPen(p3); break;
			case 513: dc.SetPen(p4); break;
			case 1025: dc.SetPen(p5); break;
			case 2049: dc.SetPen(p6); break;
			default: dc.SetPen(pwhite); break;
			}
			*/
			dc.DrawRectangle(border + i*sx, border + j*sy, sx-1, sy-1);
		}
	}
	// draw camera FOV
	FPoint3 p = cam->GetTrans();
	float fx = p.x / tg->coldim;
	float fy = tg->rows + (p.z / tg->coldim);
	int csx = border + fx * sx;
	int csy = border + fy * sy;
	dc.SetPen(pblack);
	dc.DrawLine(csx - 10, csy, csx + 10, csy);
	dc.DrawLine(csx, csy - 10, csx, csy + 10);

	datacloud *cloud = tg->GetDataCloud();
	if (cloud)
	{
		double mem = cloud->getmem();
		wxString str;
		str.Printf(_T("Mem %.1f MB, Total %d, Pending %d, Visible %d, Active %d, Missing %d"),
			mem, cloud->gettotal(), cloud->getpending(), cloud->getvisible(),
			cloud->getactive(), cloud->getmissing());
		GetTileStatus()->SetValue(str);
	}

	// These always return the whole extents, for some reason
	int left = mt->getvisibleleft();
	int right = mt->getvisibleright();
	int bottom = mt->getvisiblebottom();
	int top = mt->getvisibletop();
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

