/////////////////////////////////////////////////////////////////////////////
// Name:        ElevPropDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "ElevPropDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "ElevPropDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ElevPropDlg
//----------------------------------------------------------------------------

// WDR: event table for ElevPropDlg

BEGIN_EVENT_TABLE(ElevPropDlg,AutoDialog)
END_EVENT_TABLE()

ElevPropDlg::ElevPropDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    ElevPropDialogFunc( this, TRUE ); 
}

// WDR: handler implementations for ElevPropDlg

void ElevPropDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddNumValidator(ID_LEFT, &m_fLeft);
	AddNumValidator(ID_TOP, &m_fTop);
	AddNumValidator(ID_RIGHT, &m_fRight);
	AddNumValidator(ID_BOTTOM, &m_fBottom);

	AddValidator(ID_PROPS, &m_strText);

    wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}


