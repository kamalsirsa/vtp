//
// Name: PrefDlg.cpp
//
// Copyright (c) 2007-2008 Virtual Terrain Project
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
	EVT_INIT_DIALOG (PrefDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, PrefDlg::OnOK )
	EVT_RADIOBUTTON( ID_RADIO1, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO2, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO3, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO4, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO5, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO6, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO7, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO8, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO9, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO10, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO11, PrefDlg::OnRadio )
	EVT_CHECKBOX( ID_BLACK_TRANSP, PrefDlg::OnCheck )
	EVT_CHECKBOX( ID_DEFLATE_TIFF, PrefDlg::OnCheck )
	EVT_CHECKBOX( ID_DELAY_LOAD, PrefDlg::OnCheck )
END_EVENT_TABLE()

PrefDlg::PrefDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function PrefDialogFunc for PrefDlg
	PrefDialogFunc(this, true);

	AddValidator(ID_RADIO1, &b1);
	AddValidator(ID_RADIO2, &b2);
	AddValidator(ID_RADIO3, &b3);
	AddValidator(ID_RADIO4, &b4);
	AddValidator(ID_RADIO5, &b5);
	AddValidator(ID_RADIO6, &b6);
	AddValidator(ID_RADIO7, &b7);
	AddValidator(ID_RADIO8, &b8);
	AddValidator(ID_RADIO9, &b9);
	AddValidator(ID_RADIO10, &b10);
	AddValidator(ID_RADIO11, &b11);
	AddValidator(ID_BLACK_TRANSP, &b12);
	AddValidator(ID_DEFLATE_TIFF, &b13);
	AddValidator(ID_BT_GZIP, &b14);
	AddValidator(ID_DELAY_LOAD, &b15);
	AddNumValidator(ID_SAMPLING_N, &i1);
	AddNumValidator(ID_MAX_MEGAPIXELS, &i2);
	AddNumValidator(ID_ELEV_MAX_SIZE, &i3);
	AddNumValidator(ID_MAX_LOAD_GRID, &i4);
}

void PrefDlg::UpdateEnable()
{
	FindWindow(ID_MAX_LOAD_GRID)->Enable(b15);
}


// WDR: handler implementations for PrefDlg

void PrefDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateEnable();
}

void PrefDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
}

void PrefDlg::OnCheck( wxCommandEvent &event )
{
	TransferDataFromWindow();

	UpdateEnable();
}

void PrefDlg::OnOK( wxCommandEvent &event )
{
	TransferDataFromWindow();
	event.Skip();
}
