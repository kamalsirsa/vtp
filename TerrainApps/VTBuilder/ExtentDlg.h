//
// Name:        ExtentDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ExtentDlg_H__
#define __ExtentDlg_H__

#ifdef __GNUG__
    #pragma interface "ExtentDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
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
    
	wxString2	m_strAll;
	wxString2	m_strEast;
	wxString2	m_strNorth;
	wxString2	m_strSouth;
	wxString2	m_strWest;

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
