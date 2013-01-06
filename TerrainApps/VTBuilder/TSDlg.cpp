//
// Name: TSDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "TSDlg.h"
#include "vtdata/FileFilters.h"	// for FSTRING_JPEG
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TSDialog
//----------------------------------------------------------------------------

// WDR: event table for TSDialog

BEGIN_EVENT_TABLE(TSDialog, TSDlgBase)
	EVT_CHOICE( ID_MPP, TSDialog::OnMpp )
	EVT_CHOICE( ID_THEME, TSDialog::OnTheme )
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, TSDialog::OnRadioOutput )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, TSDialog::OnRadioOutput )
	EVT_BUTTON( ID_DOTDOTDOT, TSDialog::OnDotDotDot )
END_EVENT_TABLE()

TSDialog::TSDialog( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TSDlgBase( parent, id, title, position, size, style )
{
	AddValidator(this, ID_THEME, &m_iTheme);
	AddValidator(this, ID_MPP, &m_iMpp);

	// output options
	AddValidator(this, ID_RADIO_CREATE_NEW, &m_bNewLayer);
	AddValidator(this, ID_RADIO_TO_FILE, &m_bToFile);
	AddValidator(this, ID_TEXT_TO_FILE, &m_strToFile);

	GetTheme()->Append(_("Relief"));
	GetTheme()->Append(_("Image"));
	GetTheme()->Append(_("Topo"));

	m_bNewLayer = false;
	m_bToFile = true;

	m_iTheme = 1;
	m_iMpp = 0;
	UpdateMpp();
	UpdateMetersPerPixel();

	GetSizer()->SetSizeHints(this);
}

void TSDialog::UpdateMpp()
{
	//S: scale, ranges are:
	//	 T=0: 20-24
	//	 T=1: 10-16
	//	 T=2: 11-21
	// Meters per pixel is 2 ^ ( scale - 10 ).
	switch (m_iTheme)
	{
	case 0:
		s1 = 20;
		s2 = 24;
		break;
	case 1:
		s1 = 10;
		s2 = 16;
		break;
	case 2:
		s1 = 11;
		s2 = 21;
		break;
	}
	GetMpp()->Clear();
	for (int i = s1; i <= s2; i++)
	{
		wxString str;
		str.Printf(_T("%d"), 1 << (i-10));
		GetMpp()->Append(str);
	}
}

void TSDialog::EnableBasedOnConstraint()
{
	GetDotDotDot()->Enable(m_bToFile);
	GetTextToFile()->Enable(m_bToFile);
}


// WDR: handler implementations for TSDialog

void TSDialog::OnMpp( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateMetersPerPixel();
}

void TSDialog::UpdateMetersPerPixel()
{
	m_iMetersPerPixel = 1 << ((s1 + m_iMpp)-10);
}

void TSDialog::OnTheme( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateMpp();
	TransferDataToWindow();
}

void TSDialog::OnRadioOutput( wxCommandEvent &event )
{
	TransferDataFromWindow();
	EnableBasedOnConstraint();
}

void TSDialog::OnDotDotDot( wxCommandEvent &event )
{
	TransferDataFromWindow();

	wxString filter;
	filter += FSTRING_JPEG;

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Image Filename"), _T(""), _T(""), filter, wxFD_SAVE);
	saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	// update controls
	m_strToFile = saveFile.GetPath();
	TransferDataToWindow();
}

