//
// Name:		UtilDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __UtilDlg_H__
#define __UtilDlg_H__

#ifdef __GNUG__
	#pragma interface "UtilDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "enviro_wdr.h"
#include "AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// UtilDlg
//----------------------------------------------------------------------------

class UtilDlg: public AutoDialog
{
public:
	// constructors and destructors
	UtilDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	void OnInitDialog(wxInitDialogEvent& event);

	// WDR: method declarations for UtilDlg
	wxChoice *GetStructtype()  { return (wxChoice*) FindWindow( ID_STRUCTTYPE ); }
	
private:
	// WDR: member variable declarations for UtilDlg
	wxChoice *m_pChoice;
	int m_iType;
	
private:
	// WDR: handler declarations for UtilDlg
	void OnStructType( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
