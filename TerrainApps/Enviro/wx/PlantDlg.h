//
// Name:        PlantDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __PlantDlg_H__
#define __PlantDlg_H__

#ifdef __GNUG__
    #pragma interface "PlantDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "AutoDialog.h"

class vtPlantList3d;

// WDR: class declarations

//----------------------------------------------------------------------------
// PlantDlg
//----------------------------------------------------------------------------

class PlantDlg: public AutoDialog
{
public:
    // constructors and destructors
    PlantDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    void SetPlantList(vtPlantList3d *plants);

    void OnInitDialog(wxInitDialogEvent& event);
    wxSlider    *m_pSizeSlider;
    wxChoice    *m_pSpecies;

    int				m_iSpecies;
    float			m_fSize;
    int				m_iSizeSlider;
    float			m_fSpacing;
    vtPlantList3d	*m_pPlantList;

    // WDR: method declarations for PlantDlg
    wxSlider* GetSizeslider()  { return (wxSlider*) FindWindow( ID_SIZESLIDER ); }
    wxChoice* GetSpecies()  { return (wxChoice*) FindWindow( ID_SPECIES ); }
    
private:
    // WDR: member variable declarations for PlantDlg
    
private:
    // WDR: handler declarations for PlantDlg
    void OnSpacingEdit( wxCommandEvent &event );
    void OnSelChangeSpecies( wxCommandEvent &event );
    void OnSizeSlider( wxCommandEvent &event );
    void OnSizeEdit( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};




#endif
