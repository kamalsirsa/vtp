//
// Name:	ChooseDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "ChooseDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtui/wxString2.h"
#include "../Enviro.h"			// for GetTerrainScene
#include "ChooseDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ChooseTerrainDlg
//----------------------------------------------------------------------------

// WDR: event table for ChooseTerrainDlg

BEGIN_EVENT_TABLE(ChooseTerrainDlg,AutoDialog)
	EVT_LISTBOX( ID_TLIST, ChooseTerrainDlg::OnTListSelect )
END_EVENT_TABLE()

ChooseTerrainDlg::ChooseTerrainDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	TerrainDialogFunc( this, TRUE ); 
}

// WDR: handler implementations for ChooseTerrainDlg

void ChooseTerrainDlg::OnTListSelect( wxCommandEvent &event )
{
	m_strTName = m_pTList->GetStringSelection();
}

void ChooseTerrainDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	m_pOK = GetOk();
	m_pTList = GetTlist();

	// list each known terrain
	vtTerrain *pTerr;
	wxString2 str;
	for (pTerr = GetTerrainScene()->GetFirstTerrain(); pTerr; pTerr=pTerr->GetNext())
	{
		str = pTerr->GetName();
		m_pTList->Append(str);
	}
	int sel = m_pTList->FindString(m_strTName);
	if (sel != -1)
		m_pTList->SetSelection(sel);

	wxWindow::OnInitDialog(event);
}

