/////////////////////////////////////////////////////////////////////////////
// Name:        TowerDLG.cpp
// Author:      Michael Cressler
// Created:     10/15/01
// Copyright:   (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
/////////////////////////////////////////////////////////////////////////////
                                                    
                                                     
#ifdef __GNUG__
    #pragma implementation "TowerDLG.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "TowerDLG.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TowerDLG
//----------------------------------------------------------------------------

// WDR: event table for TowerDLG

BEGIN_EVENT_TABLE(TowerDLG,AutoDialog)
    EVT_BUTTON( wxID_OK, TowerDLG::OnOK )
    EVT_CHOICE( ID_TOWER_TYPE, TowerDLG::OnTowerType )
    EVT_CHOICE( ID_TOWER_MATERIAL, TowerDLG::OnMaterial )
END_EVENT_TABLE()

TowerDLG::TowerDLG( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    TowerDialogFunc( this, TRUE ); 
}

void TowerDLG::Setup(vtTower *twr)
{
    m_pTower = twr;
}
// WDR: handler implementations for TowerDLG

void TowerDLG::OnMaterial( wxCommandEvent &event )
{
    int sel = GetTowerMaterial()->GetSelection();
    m_iTowerMaterial = ((int) GetTowerMaterial()->GetClientData(sel));
}

void TowerDLG::OnTowerType( wxCommandEvent &event )
{
      int tel = GetTowerType()->GetSelection();
    m_iTowerType = ((int) GetTowerType()->GetClientData(tel));
}



void TowerDLG::OnOK( wxCommandEvent &event )
{
 TransferDataFromWindow();
 m_pTower->SetTowerType((enum TowerType) m_iTowerType);
 m_pTower->SetTowerMaterial((enum TowerMaterial) m_iTowerMaterial);
 m_pTower->m_bElevated = m_bElevated;

    wxDialog::OnOK(event);
 
}
void TowerDLG::OnInitDialog(wxInitDialogEvent& event)
{
     m_pcTowerType=  (wxChoice *)FindWindow(ID_TOWER_TYPE);
     m_pcTowerMaterial = (wxChoice *)FindWindow(ID_TOWER_MATERIAL);

     m_iTowerMaterial = m_pTower->GetTowerMaterial();
     m_iTowerType = m_pTower->GetTowerType();
     m_bElevated = m_pTower->m_bElevated;
     
    m_pcTowerType->Append("Lattice Tower");
    m_pcTowerType->Append("Tubular Frame");
    m_pcTowerType->Append("H Frame");
    
    m_pcTowerMaterial->Append("Wood");
    m_pcTowerMaterial->Append("Steel");
    m_pcTowerMaterial->Append("Fiberglass");
    m_pcTowerMaterial->Append("Concrete");
    
    AddValidator(ID_TOWER_TYPE, &m_iTowerType);
    AddValidator(ID_TOWER_MATERIAL, &m_iTowerMaterial);
    AddValidator (ID_TOWER_ELEV, &m_bElevated);
   // AddNumValidator( ID_TOWER_XY,
    
     wxWindow::OnInitDialog(event);// calls TransferDataToWindow()
 }
