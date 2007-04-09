//
// Name: PrefDlg.cpp
//
// Copyright (c) 2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "PrefDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// PrefDlg
//----------------------------------------------------------------------------

// WDR: event table for PrefDlg

BEGIN_EVENT_TABLE(PrefDlg,AutoDialog)
	EVT_RADIOBUTTON( ID_RADIO1, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO2, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO3, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO4, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO5, PrefDlg::OnRadio )
END_EVENT_TABLE()

PrefDlg::PrefDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function PrefDialogFunc for PrefDlg
	PrefDialogFunc( this, TRUE ); 

	AddValidator(ID_RADIO1, &b1);
	AddValidator(ID_RADIO2, &b2);
	AddValidator(ID_RADIO3, &b3);
	AddValidator(ID_RADIO4, &b4);
	AddValidator(ID_RADIO5, &b5);
}

// WDR: handler implementations for PrefDlg

void PrefDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
}
