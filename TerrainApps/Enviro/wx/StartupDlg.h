//
// Name:        StartupDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __StartupDlg_H__
#define __StartupDlg_H__

#ifdef __GNUG__
    #pragma interface "StartupDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"

class EnviroOptions;

// WDR: class declarations

//----------------------------------------------------------------------------
// StartupDlg
//----------------------------------------------------------------------------

class StartupDlg: public AutoDialog
{
public:
    // constructors and destructors
    StartupDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    void OnInitDialog(wxInitDialogEvent& event);

    void GetOptionsFrom(EnviroOptions &opt);
    void PutOptionsTo(EnviroOptions &opt);
    void UpdateState();
    void EditParameters(const char *filename);

    // WDR: method declarations for StartupDlg
    wxTextCtrl* GetTname()  { return (wxTextCtrl*) FindWindow( ID_TNAME ); }
    wxButton* GetTselect()  { return (wxButton*) FindWindow( ID_TSELECT ); }
    wxStaticText* GetImagetext()  { return (wxStaticText*) FindWindow( ID_IMAGETEXT ); }
    wxComboBox* GetImage()  { return (wxComboBox*) FindWindow( ID_IMAGE ); }
    
private:
    // WDR: member variable declarations for StartupDlg
    bool    m_bFullscreen;
    bool    m_bGravity;
    bool    m_bHtmlpane;
    bool    m_bFloatingToolbar;
    bool    m_bSound;
    bool    m_bVCursor;
    bool    m_bSpeedTest;
    bool    m_bQuakeNavigation;
    bool    m_bStartEarth;
    bool    m_bStartTerrain;
    wxString    m_strTName;
    wxString    m_strImage;
    float       m_fPlantScale;
    bool        m_bShadows;

    wxTextCtrl      *m_pTName;
    wxButton        *m_pTSelect;
    wxStaticText    *m_psImage;
    wxComboBox      *m_pImage;

	vtString    m_strFilename;

private:
    // WDR: handler declarations for StartupDlg
    void OnTerrain( wxCommandEvent &event );
    void OnEarthView( wxCommandEvent &event );
    void OnOpenGLInfo( wxCommandEvent &event );
    void OnOK( wxCommandEvent &event );
    void OnSelectTerrain( wxCommandEvent &event );
    void OnEditProp( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};

#endif
