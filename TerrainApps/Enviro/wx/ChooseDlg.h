//
// Name:        ChooseDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ChooseDlg_H__
#define __ChooseDlg_H__

#ifdef __GNUG__
    #pragma interface "ChooseDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ChooseTerrainDlg
//----------------------------------------------------------------------------

class ChooseTerrainDlg: public AutoDialog
{
public:
    // constructors and destructors
    ChooseTerrainDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );

    wxString m_strTName;

	void OnInitDialog(wxInitDialogEvent& event);

    // WDR: method declarations for ChooseTerrainDlg
    wxButton* GetOk()  { return (wxButton*) FindWindow( wxID_OK ); }
    wxListBox* GetTlist()  { return (wxListBox*) FindWindow( ID_TLIST ); }
    
private:
    // WDR: member variable declarations for ChooseTerrainDlg
    wxButton* m_pOK;
    wxListBox* m_pTList;

private:
    // WDR: handler declarations for ChooseTerrainDlg
    void OnTListSelect( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};

#endif
