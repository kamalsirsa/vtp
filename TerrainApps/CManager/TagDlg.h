/////////////////////////////////////////////////////////////////////////////
// Name:	TagDlg.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __TagDlg_H__
#define __TagDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "TagDlg.cpp"
#endif

#include "cmanager_wdr.h"
#include "vtui/wxString2.h"

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
	wxString2 m_strName;
	wxString2 m_strValue;
	
private:
	// WDR: member variable declarations for TagDlg
	
private:
	// WDR: handler declarations for TagDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
