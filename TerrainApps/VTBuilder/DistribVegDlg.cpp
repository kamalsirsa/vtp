//
// Name:        DistribVegDlg.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
    #pragma implementation "DistribVegDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "DistribVegDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// DistribVegDlg
//----------------------------------------------------------------------------

// WDR: event table for DistribVegDlg

BEGIN_EVENT_TABLE(DistribVegDlg,AutoDialog)
END_EVENT_TABLE()

DistribVegDlg::DistribVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    DistribVegFunc( this, TRUE ); 
}

// WDR: handler implementations for DistribVegDlg

void DistribVegDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddNumValidator(IDC_SAMPLING, &m_fSampling);
	AddNumValidator(IDC_SCARCITY, &m_fScarcity);

	wxDialog::OnInitDialog(event);
}



