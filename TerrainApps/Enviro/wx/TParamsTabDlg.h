//
// Name:		TParamsTabDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TParamsTabDlg_H__
#define __TParamsTabDlg_H__

#ifdef __GNUG__
	#pragma interface "TParamsTabDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "enviro_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TParamsTabDlg
//----------------------------------------------------------------------------

class TParamsTabDlg: public wxDialog
{
public:
	// constructors and destructors
	TParamsTabDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for TParamsTabDlg
	wxNotebook* GetNoteBook()  { return (wxNotebook*) FindWindow( ID_NOTEBOOK ); }
	
private:
	// WDR: member variable declarations for TParamsTabDlg
	
private:
	// WDR: handler declarations for TParamsTabDlg

private:
	DECLARE_EVENT_TABLE()
};




#endif
