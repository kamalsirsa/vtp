//
// Name:		OptionsDlg.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "OptionsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "OptionsDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// OptionsDlg
//----------------------------------------------------------------------------

// WDR: event table for OptionsDlg

BEGIN_EVENT_TABLE(OptionsDlg, AutoDialog)
	EVT_INIT_DIALOG (OptionsDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_RADIO_OUTLINE_ONLY, OptionsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_COLOR, OptionsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_NO_SHADING, OptionsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_SIMPLE_SHADING, OptionsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_CAST_SHADOWS, OptionsDlg::OnRadio )
END_EVENT_TABLE()

OptionsDlg::OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	OptionsDialogFunc( this, TRUE );

	GetElevUnit()->Append(_("Meters"));
	GetElevUnit()->Append(_("Feet (International)"));
	GetElevUnit()->Append(_("Feet (U.S. Survey)"));

	AddValidator(ID_TOOLBAR, &m_bShowToolbar);
	AddValidator(ID_MINUTES, &m_bShowMinutes);
	AddValidator(ID_ELEVUNIT, &m_iElevUnits);

	AddValidator(ID_RADIO_OUTLINE_ONLY, &m_bShowOutlines);
	AddValidator(ID_RADIO_COLOR, &m_bColor);
	AddValidator(ID_RADIO_NO_SHADING, &m_bNoShading);
	AddValidator(ID_RADIO_SIMPLE_SHADING, &m_bSimpleShading);
	AddValidator(ID_RADIO_CAST_SHADOWS, &m_bCastShadows);
	AddValidator(ID_SPIN_CAST_ANGLE, &m_iCastAngle);
	AddValidator(ID_SPIN_CAST_DIRECTION, &m_iCastDirection);
	AddValidator(ID_CHECK_HIDE_UNKNOWN, &m_bHideUnknown);

	AddValidator(ID_CHECK_SHOW_ROAD_WIDTH, &m_bShowRoadWidth);
	AddValidator(ID_PATHNAMES, &m_bShowPath);
}

// WDR: handler implementations for OptionsDlg

void OptionsDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnables();
}

void OptionsDlg::UpdateEnables()
{
	GetRadioNoShading()->Enable(m_bColor);
	GetRadioSimpleShading()->Enable(m_bColor);
	GetRadioCastShadows()->Enable(m_bColor);
	GetRadioCastShadows()->Enable(m_bColor);
	GetSpinCastAngle()->Enable(m_bColor && m_bCastShadows);
	GetSpinCastDirection()->Enable(m_bColor && m_bCastShadows);
}

void OptionsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateEnables();
	wxDialog::OnInitDialog(event);
}

void OptionsDlg::SetElevDrawOptions(const ElevDrawOptions &opt)
{
	m_bShowOutlines = !opt.m_bShowElevation;
	m_bColor = opt.m_bShowElevation;
	m_bNoShading = !opt.m_bShading;
	m_bSimpleShading = opt.m_bShading && !opt.m_bCastShadows;
	m_bCastShadows = opt.m_bShading && opt.m_bCastShadows;
	m_iCastAngle = opt.m_iCastAngle;
	m_iCastDirection = opt.m_iCastDirection;
	m_bHideUnknown = opt.m_bDoMask;
}

void OptionsDlg::GetElevDrawOptions(ElevDrawOptions &opt)
{
	opt.m_bShowElevation = m_bColor;
	opt.m_bShading = !m_bNoShading;
	opt.m_bCastShadows = m_bCastShadows;
	opt.m_iCastAngle = m_iCastAngle;
	opt.m_iCastDirection = m_iCastDirection;
	opt.m_bDoMask = m_bHideUnknown;
}

