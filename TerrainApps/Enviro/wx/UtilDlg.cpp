//
// Name: UtilDlg.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
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
#include "vtlib/core/TerrainScene.h"
#include "UtilDlg.h"
#include "EnviroGUI.h"

#include "vtui/wxString2.h"

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
	wxString2 val = m_pChoice->GetStringSelection();
	g_App.SetRouteOptions(val.mb_str());
	g_App.start_new_fence();
}

void UtilDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddValidator(ID_STRUCTTYPE, &m_iType);

	m_iType = 0;

	vtContentManager &mng = vtGetContent();

	m_pChoice->Clear();
	for (unsigned int i = 0; i < mng.NumItems(); i++)
	{
		vtString str;
		vtItem *item = mng.GetItem(i);
		if (item->GetValueString("type", str))
		{
			if (str == "utility pole")
				m_pChoice->Append(wxString::FromAscii(item->m_name));
		}
	}

	TransferDataToWindow();

	wxString2 val = m_pChoice->GetStringSelection();
	g_App.SetRouteOptions(val.mb_str());
}

