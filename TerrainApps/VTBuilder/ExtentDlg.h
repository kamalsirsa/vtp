/////////////////////////////////////////////////////////////////////////////
// Name:        ExtentDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __ExtentDlg_H__
#define __ExtentDlg_H__

#ifdef __GNUG__
    #pragma interface "ExtentDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"
#include "vtdata/MathTypes.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ExtentDlg
//----------------------------------------------------------------------------

class ExtentDlg: public AutoDialog
{
public:
    // constructors and destructors
    ExtentDlg( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    // WDR: method declarations for ExtentDlg
	void SetArea(DRECT area, bool bMeters);
    
	wxString	m_strAll;
	wxString	m_strEast;
	wxString	m_strNorth;
	wxString	m_strSouth;
	wxString	m_strWest;

	DRECT m_area;
	bool m_bMeters;
	bool m_bSetting;

private:
    // WDR: member variable declarations for ExtentDlg
    
private:
    // WDR: handler declarations for ExtentDlg
    void OnExtentAll( wxCommandEvent &event );
    void OnExtentS( wxCommandEvent &event );
    void OnExtentE( wxCommandEvent &event );
    void OnExtentW( wxCommandEvent &event );
    void OnExtentN( wxCommandEvent &event );

	void FormatStrings(int which);
	void OnInitDialog(wxInitDialogEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

#endif
