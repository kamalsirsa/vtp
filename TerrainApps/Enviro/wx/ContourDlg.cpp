//
// Name:		ContourDlg.cpp
//
// Copyright (c) 2009 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ContourDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ContourDlg
//----------------------------------------------------------------------------

// WDR: event table for ContourDlg

BEGIN_EVENT_TABLE(ContourDlg,AutoDialog)
	EVT_RADIOBUTTON( ID_RADIO_CREATE, ContourDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_ADD, ContourDlg::OnRadio )
END_EVENT_TABLE()

ContourDlg::ContourDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function ContourDlgFunc for ContourDlg
	ContourDlgFunc( this, TRUE ); 

	m_fElev = 1000;
	m_bCreate = true;
	m_strLayer = _T("");

	AddNumValidator(ID_ELEV, &m_fElev);
	AddValidator(ID_RADIO_CREATE, &m_bCreate);
	AddValidator(ID_CHOICE_LAYER, &m_strLayer);

	UpdateEnabling();
}

// WDR: handler implementations for ContourDlg

void ContourDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void ContourDlg::UpdateEnabling()
{
	GetChoiceLayer()->Enable(!m_bCreate);
}

