//
// Name:		TParamsTabDlg.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
	#pragma implementation "TParamsTabDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtlib/vtlib.h"

#include "TParamsDlg.h"
#include "TParamsTabDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TParamsTabDlg
//----------------------------------------------------------------------------

// WDR: event table for TParamsTabDlg

BEGIN_EVENT_TABLE(TParamsTabDlg,wxDialog)
END_EVENT_TABLE()

TParamsTabDlg::TParamsTabDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	TParamsTabFunc( this, TRUE );

	SetAutoLayout( TRUE );
}

// WDR: handler implementations for TParamsTabDlg




