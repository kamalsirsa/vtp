//
// Name: SnapSizeDlg.cpp
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "SnapSizeDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SnapSizeDlg
//----------------------------------------------------------------------------

// WDR: event table for SnapSizeDlg

BEGIN_EVENT_TABLE(SnapSizeDlg,AutoDialog)
EVT_SLIDER( ID_SLIDER_RATIO, SnapSizeDlg::OnSlider )
END_EVENT_TABLE()

SnapSizeDlg::SnapSizeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
						 const wxPoint &position, const wxSize& size, long style ) :
AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function SnapSizeDialogFunc for SnapSizeDlg
	SnapSizeDialogFunc( this, TRUE ); 

	m_fRatio = 1.0f;
	m_iRatio = 0;

	AddValidator(ID_SLIDER_RATIO, &m_iRatio);
	AddNumValidator(ID_TEXT_X, &m_Current.x);
	AddNumValidator(ID_TEXT_Y, &m_Current.y);

	GetSliderRatio()->SetFocus();
}

void SnapSizeDlg::SetBase(const IPoint2 &size)
{
	m_Base = size;
	Update();
}

void SnapSizeDlg::Update()
{
	m_Current = m_Base * m_fRatio;
}

// WDR: handler implementations for SnapSizeDlg

void SnapSizeDlg::OnSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_fRatio = 1.0f + (float) m_iRatio / 100.0f;
	Update();
	TransferDataToWindow();
}

