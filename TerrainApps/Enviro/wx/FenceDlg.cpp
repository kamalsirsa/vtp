//
// Name:		FenceDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "FenceDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtlib/vtlib.h"
#include "FenceDlg.h"
#include "../Enviro.h"

#define HEIGHT_MIN	1.0f
#define HEIGHT_MAX	6.0f
#define SPACING_MIN	1.0f
#define SPACING_MAX	4.0f

// WDR: class implementations

//----------------------------------------------------------------------------
// FenceDlg
//----------------------------------------------------------------------------

// WDR: event table for FenceDlg

BEGIN_EVENT_TABLE(FenceDlg,AutoDialog)
	EVT_CHOICE( ID_FENCETYPE, FenceDlg::OnFenceType )
	EVT_TEXT( ID_HEIGHTEDIT, FenceDlg::OnHeightEdit )
	EVT_TEXT( ID_SPACINGEDIT, FenceDlg::OnSpacingEdit )
	EVT_SLIDER( ID_HEIGHTSLIDER, FenceDlg::OnHeightSlider )
	EVT_SLIDER( ID_SPACINGSLIDER, FenceDlg::OnSpacingSlider )
END_EVENT_TABLE()

FenceDlg::FenceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	FenceDialogFunc( this, TRUE ); 

	m_pSpacingSlider = GetSpacingslider();
	m_pHeightSlider = GetHeightslider();
	m_pFenceChoice = GetFencetype();
}

// WDR: handler implementations for FenceDlg

void FenceDlg::OnSpacingSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SlidersToValues();
	TransferDataToWindow();
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
}

void FenceDlg::OnHeightSlider( wxCommandEvent &event )
{
	OnSpacingSlider(event);
}

void FenceDlg::OnSpacingEdit( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ValuesToSliders();
	TransferDataToWindow();
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
}

void FenceDlg::OnHeightEdit( wxCommandEvent &event )
{
	OnSpacingEdit(event);
}

void FenceDlg::OnFenceType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_fencetype = (FenceType) m_iType;
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
	g_App.start_new_fence();
}

void FenceDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	AddValidator(ID_FENCETYPE, &m_iType);

	AddValidator(ID_HEIGHTSLIDER, &m_iHeight);
	AddValidator(ID_SPACINGSLIDER, &m_iSpacing);

	AddNumValidator(ID_HEIGHTEDIT, &m_fHeight);
	AddNumValidator(ID_SPACINGEDIT, &m_fSpacing);

	m_iType = 0;
	m_fHeight = FENCE_DEFAULT_HEIGHT;
	m_fSpacing = FENCE_DEFAULT_SPACING;

	ValuesToSliders();

	m_pFenceChoice->Clear();
	m_pFenceChoice->Append("Wooden posts, 3 wires");
//	m_pFenceChoice->SetClientData(0, FT_WIRE);
	m_pFenceChoice->Append("Metal poles, chain-link");
//	m_pFenceChoice->SetClientData(1, FT_CHAINLINK);

	TransferDataToWindow();

	m_fencetype = (FenceType) m_iType;
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
}

void FenceDlg::SlidersToValues()
{
	m_fHeight = HEIGHT_MIN + m_iHeight * (HEIGHT_MAX - HEIGHT_MIN) / 100.0f;
	m_fSpacing = SPACING_MIN + m_iSpacing * (SPACING_MAX - SPACING_MIN) / 100.0f;
}

void FenceDlg::ValuesToSliders()
{
	m_iHeight = (int) ((m_fHeight - HEIGHT_MIN) / (HEIGHT_MAX - HEIGHT_MIN) * 100.0f);
	m_iSpacing = (int) ((m_fSpacing - SPACING_MIN) / (SPACING_MAX - SPACING_MIN) * 100.0f);
}





