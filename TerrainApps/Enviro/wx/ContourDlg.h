//
// Name:		ContourDlg.h
//
// Copyright (c) 2009-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef __ContourDlg_H__
#define __ContourDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ContourDlg
//----------------------------------------------------------------------------

class ContourDlg: public ContourDlgBase
{
public:
	// constructors and destructors
	ContourDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for ContourDlg
	wxChoice* GetChoiceLayer()  { return (wxChoice*) FindWindow( ID_CHOICE_LAYER ); }
	wxTextCtrl* GetElev()  { return (wxTextCtrl*) FindWindow( ID_ELEV ); }
	void UpdateEnabling();

	float m_fElev;
	bool m_bCreate;
	wxString m_strLayer;

private:
	// WDR: member variable declarations for ContourDlg
	
private:
	// WDR: handler declarations for ContourDlg
	void OnRadio( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ContourDlg_H__

