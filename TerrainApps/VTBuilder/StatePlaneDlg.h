//
// Name: StatePlaneDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __StatePlaneDlg_H__
#define __StatePlaneDlg_H__

#ifdef __GNUG__
	#pragma interface "StatePlaneDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Projections.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// StatePlaneDlg
//----------------------------------------------------------------------------

class StatePlaneDlg: public AutoDialog
{
public:
	// constructors and destructors
	StatePlaneDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for StatePlaneDlg
	wxListBox* GetStatePlanes()  { return (wxListBox*) FindWindow( ID_STATEPLANES ); }
	
public:
	// WDR: member variable declarations for StatePlaneDlg
	int m_iStatePlane;
	bool m_bNAD27;
	
private:
	// WDR: handler declarations for StatePlaneDlg
	void OnListBox( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
