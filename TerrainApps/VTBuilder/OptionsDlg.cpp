//
// Name:		OptionsDlg.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
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

BEGIN_EVENT_TABLE(OptionsDlg,AutoDialog)
END_EVENT_TABLE()

OptionsDlg::OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	OptionsDialogFunc( this, TRUE ); 
}

// WDR: handler implementations for OptionsDlg

void OptionsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetElevUnit()->Clear();
	GetElevUnit()->Append("Meters");
	GetElevUnit()->Append("Feet (International)");
	GetElevUnit()->Append("Feet (U.S. Survey)");

	AddValidator(ID_TOOLBAR, &m_bShowToolbar);
	AddValidator(ID_MINUTES, &m_bShowMinutes);
	AddValidator(ID_ELEVUNIT, &m_iElevUnits);
	AddValidator(ID_PATHNAMES, &m_bShowPath);

	wxDialog::OnInitDialog(event);
}

