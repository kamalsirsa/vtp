//
// Name: LinearStructDlg.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "LinearStructDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "LinearStructDlg.h"

#define HEIGHT_MIN	1.0f
#define HEIGHT_MAX	6.0f
#define SPACING_MIN	1.0f
#define SPACING_MAX	4.0f

// WDR: class implementations

//----------------------------------------------------------------------------
// LinearStructureDlg
//----------------------------------------------------------------------------

// WDR: event table for LinearStructureDlg

BEGIN_EVENT_TABLE(LinearStructureDlg, AutoDialog)
	EVT_INIT_DIALOG (LinearStructureDlg::OnInitDialog)
	EVT_CHOICE( ID_TYPE, LinearStructureDlg::OnFenceType )
	EVT_TEXT( ID_HEIGHTEDIT, LinearStructureDlg::OnHeightEdit )
	EVT_TEXT( ID_SPACINGEDIT, LinearStructureDlg::OnSpacingEdit )
	EVT_SLIDER( ID_HEIGHTSLIDER, LinearStructureDlg::OnHeightSlider )
	EVT_SLIDER( ID_SPACINGSLIDER, LinearStructureDlg::OnSpacingSlider )
END_EVENT_TABLE()

LinearStructureDlg::LinearStructureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	LinearStructDialogFunc( this, TRUE );

	m_pSpacingSlider = GetSpacingslider();
	m_pHeightSlider = GetHeightslider();
	m_pFenceChoice = GetFencetype();

	m_iType = 0;
	m_opts.fHeight = FENCE_DEFAULT_HEIGHT;
	m_opts.fSpacing = FENCE_DEFAULT_SPACING;

	AddValidator(ID_TYPE, &m_iType);

	AddValidator(ID_HEIGHTSLIDER, &m_iHeight);
	AddValidator(ID_SPACINGSLIDER, &m_iSpacing);

	AddNumValidator(ID_HEIGHTEDIT, &m_opts.fHeight);
	AddNumValidator(ID_SPACINGEDIT, &m_opts.fSpacing);

	// NB -- these must match the FT_ enum in order
	m_pFenceChoice->Clear();
	m_pFenceChoice->Append(_T("Wooden posts, 3 wires"));
	m_pFenceChoice->Append(_T("Metal poles, chain-link"));
	m_pFenceChoice->Append(_T("English Hedgerow"));
	m_pFenceChoice->Append(_T("English Drystone"));
	m_pFenceChoice->Append(_T("English Privet"));
	m_pFenceChoice->Append(_T("English Beech"));
	m_pFenceChoice->Append(_T("Coursed Stone"));
}

// WDR: handler implementations for LinearStructureDlg

void LinearStructureDlg::OnSpacingSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SlidersToValues();
	TransferDataToWindow();
	OnSetOptions(m_opts);
}

void LinearStructureDlg::OnHeightSlider( wxCommandEvent &event )
{
	OnSpacingSlider(event);
}

void LinearStructureDlg::OnSpacingEdit( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ValuesToSliders();
	TransferDataToWindow();
	OnSetOptions(m_opts);
}

void LinearStructureDlg::OnHeightEdit( wxCommandEvent &event )
{
	OnSpacingEdit(event);
}

void LinearStructureDlg::OnFenceType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_opts.eType = (FenceType) m_iType;
	OnSetOptions(m_opts);
}

void LinearStructureDlg::OnInitDialog(wxInitDialogEvent& event)
{
	ValuesToSliders();
	TransferDataToWindow();

	m_opts.eType = (FenceType) m_iType;
	OnSetOptions(m_opts);
}

void LinearStructureDlg::SlidersToValues()
{
	m_opts.fHeight = HEIGHT_MIN + m_iHeight * (HEIGHT_MAX - HEIGHT_MIN) / 100.0f;
	m_opts.fSpacing = SPACING_MIN + m_iSpacing * (SPACING_MAX - SPACING_MIN) / 100.0f;
}

void LinearStructureDlg::ValuesToSliders()
{
	m_iHeight = (int) ((m_opts.fHeight - HEIGHT_MIN) / (HEIGHT_MAX - HEIGHT_MIN) * 100.0f);
	m_iSpacing = (int) ((m_opts.fSpacing - SPACING_MIN) / (SPACING_MAX - SPACING_MIN) * 100.0f);
}

