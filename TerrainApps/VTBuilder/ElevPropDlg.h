/////////////////////////////////////////////////////////////////////////////
// Name:        ElevPropDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __ElevPropDlg_H__
#define __ElevPropDlg_H__

#ifdef __GNUG__
    #pragma interface "ElevPropDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ElevPropDlg
//----------------------------------------------------------------------------

class ElevPropDlg: public AutoDialog
{
public:
    // constructors and destructors
    ElevPropDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for ElevPropDlg
	double m_fLeft, m_fTop, m_fRight, m_fBottom;
	wxString m_strText;

private:
    // WDR: member variable declarations for ElevPropDlg
    
private:
    // WDR: handler declarations for ElevPropDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
    DECLARE_EVENT_TABLE()
};




#endif
