//
// Name:        ChooseDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
    #pragma implementation "ChooseDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "../TerrainSceneWP.h"
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
    for (pTerr = GetTerrainScene().m_pFirstTerrain; pTerr; pTerr=pTerr->GetNext())
    {
        m_pTList->Append((const char *)(pTerr->GetName()));
    }
    int sel = m_pTList->FindString(m_strTName);
    if (sel != -1)
        m_pTList->SetSelection(sel);

    wxWindow::OnInitDialog(event);
}

