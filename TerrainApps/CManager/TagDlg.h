/////////////////////////////////////////////////////////////////////////////
// Name:        TagDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __TagDlg_H__
#define __TagDlg_H__

#ifdef __GNUG__
    #pragma interface "TagDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "cmanager_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TagDlg
//----------------------------------------------------------------------------

class TagDlg: public wxDialog
{
public:
    // constructors and destructors
    TagDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for TagDlg
	wxString m_strName;
	wxString m_strValue;
    
private:
    // WDR: member variable declarations for TagDlg
    
private:
    // WDR: handler declarations for TagDlg
    void OnInitDialog(wxInitDialogEvent& event);

private:
    DECLARE_EVENT_TABLE()
};




#endif
