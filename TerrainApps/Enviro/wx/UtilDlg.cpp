//
// Name: UtilDlg.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "UtilDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Route.h"
#include "UtilDlg.h"
#include "EnviroGUI.h"

extern UtilStructName s_Names[];

// WDR: class implementations

//----------------------------------------------------------------------------
// UtilDlg
//----------------------------------------------------------------------------

// WDR: event table for UtilDlg

BEGIN_EVENT_TABLE(UtilDlg,AutoDialog)
	EVT_CHOICE( ID_STRUCTTYPE, UtilDlg::OnStructType )
END_EVENT_TABLE()

UtilDlg::UtilDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	UtilDialogFunc( this, TRUE );

	m_pChoice = GetStructtype();
}

// WDR: handler implementations for UtilDlg

void UtilDlg::OnStructType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	g_App.SetRouteOptions(s_Names[m_iType].brief);
	g_App.start_new_fence();
}

void UtilDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddValidator(ID_STRUCTTYPE, &m_iType);

	m_iType = 0;

	m_pChoice->Clear();
	int i;
	for (i = 0; i < NUM_STRUCT_NAMES; i++)
	{
		m_pChoice->Append(wxString::FromAscii(s_Names[i].full));
	}

	TransferDataToWindow();

	g_App.SetRouteOptions(s_Names[m_iType].brief);
}

