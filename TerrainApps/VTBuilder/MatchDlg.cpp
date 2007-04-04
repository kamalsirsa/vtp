//
// Name: MatchDlg.cpp
//
// Copyright (c) 2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "MatchDlg.h"
#include "Frame.h"
#include "BuilderView.h"
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "vtdata/ElevationGrid.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// MatchDlg
//----------------------------------------------------------------------------

// WDR: event table for MatchDlg

BEGIN_EVENT_TABLE(MatchDlg,AutoDialog)
	EVT_SPIN_UP( ID_SIZE_SPIN, MatchDlg::OnSpinUp )
	EVT_SPIN_DOWN( ID_SIZE_SPIN, MatchDlg::OnSpinDown )
	EVT_CHOICE( ID_MATCH_LAYER, MatchDlg::OnMatchLayer )
	EVT_CHECKBOX( ID_GROW, MatchDlg::OnGrow )
	EVT_CHECKBOX( ID_SHRINK, MatchDlg::OnShrink )
END_EVENT_TABLE()

MatchDlg::MatchDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function MatchDialogFunc for MatchDlg
	MatchDialogFunc( this, TRUE );

	AddValidator(ID_EXTENT1, &m_strExtent1);
	AddValidator(ID_EXTENT2, &m_strExtent2);
	AddValidator(ID_MATCH_LAYER, &m_iLayer);
	AddValidator(ID_LAYER_RES, &m_strLayerRes);
	AddValidator(ID_GROW, &m_bGrow);
	AddValidator(ID_SHRINK, &m_bShrink);
	AddNumValidator(ID_TILE_SIZE, &m_iTileSize);
	AddValidator(ID_TILING, &m_strTiling);

	UpdateLayers();
	GetLayerSpacing();
}

void MatchDlg::SetArea(const DRECT &area, bool bIsGeo)
{
	m_area = area;
	m_original = area;
	m_bIsGeo = bIsGeo;
	m_tile.Set(2, 2);
	m_iTileSize = 1024;
	m_bGrow = true;
	m_bShrink = true;

	if (m_bIsGeo)
		m_fs = _T("%4.8f");
	else
		m_fs = _T("%8.1f");

	UpdateGuess();
	UpdateValues();
}

void MatchDlg::UpdateValues()
{
	m_strExtent1.Printf(_T("(")+m_fs+_T(", ")+m_fs+_T("), (")+m_fs+_T(", ")+m_fs+_T(")"),
		m_original.left, m_original.bottom, m_original.Width(), m_original.Height());

	m_strExtent2.Printf(_T("(")+m_fs+_T(", ")+m_fs+_T("), (")+m_fs+_T(", ")+m_fs+_T(")"),
		m_area.left, m_area.bottom, m_area.Width(), m_area.Height());

	m_strTiling.Printf(_T("%d x %d"), m_tile.x, m_tile.y);
}

void MatchDlg::UpdateLayers()
{
	GetMatchLayer()->Clear();
	MainFrame *frame = GetMainFrame();
	for (int i = 0; i < frame->NumLayers(); i++)
	{
		vtLayer *lay = frame->GetLayer(i);

		wxString name = StartOfFilename(lay->GetLayerFilename());
		GetMatchLayer()->Append(name);
	}
	GetMatchLayer()->SetSelection(0);
	m_iLayer = 0;
}

void MatchDlg::GetLayerSpacing()
{
	vtLayer *lay = GetMainFrame()->GetLayer(m_iLayer);

	m_strLayerRes = _("n/a");
	m_spacing.Set(0,0);

	if (lay->GetType() == LT_ELEVATION)
	{
		vtElevLayer *elay = (vtElevLayer *)lay;
		if (elay->m_pGrid)
			m_spacing = elay->m_pGrid->GetSpacing();
	}
	if (lay->GetType() == LT_IMAGE)
		m_spacing = ((vtImageLayer *)lay)->GetSpacing();

	if (m_spacing != DPoint2(0,0))
	{
		m_strLayerRes.Printf(_T("%.2f, %.2f"), m_spacing.x, m_spacing.y);
	}
}

void MatchDlg::UpdateGuess()
{
	DPoint2 tilearea;
	bool go = true;
	double estx, esty;
	while (go)
	{
		tilearea = m_spacing * m_iTileSize;
		estx = m_original.Width() / tilearea.x;
		esty = m_original.Height() / tilearea.y;
		if (estx < 1.0 || esty < 1.0)
		{
			m_iTileSize >>= 1;
			if (m_iTileSize == 1)
				go = false;
		}
		else
			go = false;
	}
	if (m_bGrow && m_bShrink)
	{
		m_tile.x = (int) (estx + 0.5);
		m_tile.y = (int) (esty + 0.5);
	}
	else if (m_bGrow)
	{
		// grow but not shrink; force round up
		m_tile.x = (int) (estx + 0.99999);
		m_tile.y = (int) (esty + 0.99999);
	}
	else if (m_bShrink)
	{
		// shrink but not grow: force round down
		m_tile.x = (int) estx;
		m_tile.y = (int) esty;
	}

	DPoint2 center = m_original.GetCenter();
	DPoint2 new_area(m_tile.x * tilearea.x, m_tile.y * tilearea.y);
	m_area.left   = center.x - 0.5 * new_area.x;
	m_area.right  = center.x + 0.5 * new_area.x;
	m_area.bottom = center.y - 0.5 * new_area.y;
	m_area.top	= center.y + 0.5 * new_area.y;

	GetMainFrame()->GetView()->ShowGridMarks(m_area, m_tile.x, m_tile.y, -1, -1);
}

// WDR: handler implementations for MatchDlg

void MatchDlg::OnShrink( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (!m_bShrink)
		m_bGrow = true;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnGrow( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (!m_bGrow)
		m_bShrink = true;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnMatchLayer( wxCommandEvent &event )
{
	TransferDataFromWindow();
	GetLayerSpacing();
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnSpinUp( wxSpinEvent &event )
{
	m_iTileSize <<= 1;
	if (m_iTileSize > 2048)
		m_iTileSize = 2048;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnSpinDown( wxSpinEvent &event )
{
	m_iTileSize >>= 1;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}


