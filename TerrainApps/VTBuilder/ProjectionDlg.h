/////////////////////////////////////////////////////////////////////////////
// Name:        ProjectionDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __ProjectionDlg_H__
#define __ProjectionDlg_H__

#ifdef __GNUG__
    #pragma interface "ProjectionDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"
#include "vtdata/Projections.h"

enum ProjType
{
    PT_GEO,
    PT_UTM,
    PT_ALBERS,
    PT_STATE
};

// WDR: class declarations

//----------------------------------------------------------------------------
// ProjectionDlg
//----------------------------------------------------------------------------

class ProjectionDlg: public AutoDialog
{
public:
    // constructors and destructors
    ProjectionDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for ProjectionDlg
    wxRadioButton* GetStateplane()  { return (wxRadioButton*) FindWindow( ID_STATEPLANE ); }
    wxRadioButton* GetAlbers()  { return (wxRadioButton*) FindWindow( ID_ALBERS ); }
    wxRadioButton* GetUtm()  { return (wxRadioButton*) FindWindow( ID_UTM ); }
    wxChoice* GetDatum()  { return (wxChoice*) FindWindow( ID_DATUM ); }
    wxChoice* GetSpchoice()  { return (wxChoice*) FindWindow( ID_SPCHOICE ); }
    wxTextCtrl* GetMeridian()  { return (wxTextCtrl*) FindWindow( ID_MERIDIAN ); }
    wxTextCtrl* GetPar2()  { return (wxTextCtrl*) FindWindow( ID_PAR2 ); }
    wxTextCtrl* GetPar1()  { return (wxTextCtrl*) FindWindow( ID_PAR1 ); }
    wxTextCtrl* GetUtmzone()  { return (wxTextCtrl*) FindWindow( ID_UTMZONE ); }
    wxRadioButton* GetGeo()  { return (wxRadioButton*) FindWindow( ID_GEO ); }
    
    void SetRadioButtons();
    void GetRadioButtons();
    void DoEnables();

    void SetProjection(vtProjection &proj);
    void GetProjection(vtProjection &proj);

    ProjType    m_iProj;
    int         m_iZone, m_iDatum, m_iStatePlane;
    double      m_fPar1, m_fPar2, m_fMeridian;
    wxString    m_strCaption;

private:
    // WDR: member variable declarations for ProjectionDlg
    
private:
    // WDR: handler declarations for ProjectionDlg
    void OnDatumChoice( wxCommandEvent &event );
    void OnStatePlaneChoice( wxCommandEvent &event );
    void OnRadio( wxCommandEvent &event );
    void OnInitDialog(wxInitDialogEvent& event);

private:
    DECLARE_EVENT_TABLE()
};




#endif
