/////////////////////////////////////////////////////////////////////////////
// Name:        TowerDLG.h
// Author:      Michael Cressler
// Created:     10/15/01
// Copyright:   (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
/////////////////////////////////////////////////////////////////////////////

#ifndef __TowerDLG_H__
#define __TowerDLG_H__

#ifdef __GNUG__
    #pragma interface "TowerDLG.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"
#include "vtdata/Tower.h"


// WDR: class declarations

//----------------------------------------------------------------------------
// TowerDLG
//----------------------------------------------------------------------------

class TowerDLG: public AutoDialog
{
public:
    // constructors and destructors
    TowerDLG( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for TowerDLG
    wxChoice* GetTowerMaterial()  { return (wxChoice*) FindWindow( ID_TOWER_MATERIAL ); }
    wxChoice* GetTowerType()  { return (wxChoice*) FindWindow( ID_TOWER_TYPE ); }
    wxTextCtrl* GetTowerXy()  { return (wxTextCtrl*) FindWindow( ID_TOWER_XY ); }
     void Setup(vtTower *twr);
     
private:
    // WDR: member variable declarations for TowerDLG
    vtTower *m_pTower;
    int m_iTowerType;
    int m_iTowerMaterial;
    float m_fX;
    float m_fY;
    bool m_bElevated;
    wxChoice *m_pcTowerType;
    wxChoice *m_pcTowerMaterial;
    
    
private:                                                     
    // WDR: handler declarations for TowerDLG
    void OnMaterial( wxCommandEvent &event );
    void OnTowerType( wxCommandEvent &event );
   
    void OnOK( wxCommandEvent &event );
    void OnInitDialog(wxInitDialogEvent &event);

private:
    DECLARE_EVENT_TABLE()
};




#endif
