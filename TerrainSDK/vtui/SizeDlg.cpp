//
// Name: SizeDlg.cpp
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SizeDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SizeDlg
//----------------------------------------------------------------------------

// WDR: event table for SizeDlg

BEGIN_EVENT_TABLE(SizeDlg,AutoDialog)
EVT_SLIDER( ID_SLIDER_RATIO, SizeDlg::OnSlider )
END_EVENT_TABLE()

SizeDlg::SizeDlg(wxWindow *parent, wxWindowID id, const wxString &title,
				 const wxPoint &position, const wxSize& size, long style) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function SnapSizeDialogFunc for SizeDlg
	SizeDialogFunc( this, TRUE ); 

	m_fRatio = 1.0f;
	m_iRatio = 0;

	AddValidator(ID_SLIDER_RATIO, &m_iRatio);
	AddNumValidator(ID_TEXT_X, &m_Current.x);
	AddNumValidator(ID_TEXT_Y, &m_Current.y);

	GetSliderRatio()->SetFocus();
}

void SizeDlg::SetBase(const IPoint2 &size)
{
	m_Base = size;
	Update();
}

void SizeDlg::SetRatioRange(float fMin, float fMax)
{
	m_fRatioMin = fMin;
	m_fRatioMax = fMax;
	Update();
}

void SizeDlg::Update()
{
	m_Current = m_Base * m_fRatio;
}

// WDR: handler implementations for SizeDlg

void SizeDlg::OnSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_fRatio = m_fRatioMin + ((float) m_iRatio / 320.0f) * (m_fRatioMax - m_fRatioMin);
	Update();
	TransferDataToWindow();
}

