//
// Name: OptionsDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __OptionsDlg_H__
#define __OptionsDlg_H__

#ifdef __GNUG__
	#pragma interface "OptionsDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// OptionsDlg
//----------------------------------------------------------------------------

class OptionsDlg: public AutoDialog
{
public:
	// constructors and destructors
	OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for OptionsDlg
	wxChoice* GetElevUnit()  { return (wxChoice*) FindWindow( ID_ELEVUNIT ); }

	bool m_bShowToolbar;
	bool m_bShowMinutes;
	int  m_iElevUnits;
	bool m_bShowPath;

private:
	// WDR: member variable declarations for OptionsDlg

private:
	// WDR: handler declarations for OptionsDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
