/////////////////////////////////////////////////////////////////////////////
// Name:        ImportVegDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __ImportVegDlg_H__
#define __ImportVegDlg_H__

#ifdef __GNUG__
    #pragma interface "ImportVegDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ImportVegDlg
//----------------------------------------------------------------------------

class ImportVegDlg: public wxDialog
{
public:
    // constructors and destructors
    ImportVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );

    // Methods
    void SetShapefileName(const char *filename);

    // Data
    int m_fieldindex, m_datatype;
    wxComboBox *m_pcbField;
	wxRadioButton *m_pDensity, *m_pBiotype1, *m_pBiotype2;
    wxString m_filename;

    // WDR: method declarations for ImportVegDlg
    wxRadioButton* GetDensity()  { return (wxRadioButton*) FindWindow( ID_DENSITY ); }
    wxRadioButton* GetBiotype2()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE2 ); }
    wxRadioButton* GetBiotype1()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE1 ); }
    wxComboBox* GetField()  { return (wxComboBox*) FindWindow( ID_FIELD ); }
    
private:
    // WDR: member variable declarations for ImportVegDlg
    void OnInitDialog(wxInitDialogEvent& event);

private:
    // WDR: handler declarations for ImportVegDlg
    void OnOK( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};


#endif
