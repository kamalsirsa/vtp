//
// Name: TSDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TSDlg_H__
#define __TSDlg_H__

#include "VTBuilder_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TSDialog
//----------------------------------------------------------------------------

class TSDialog: public TSDlgBase
{
public:
	// constructors and destructors
	TSDialog( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for TSDialog
	wxButton* GetDotDotDot()  { return (wxButton*) FindWindow( ID_DOTDOTDOT ); }
	wxTextCtrl* GetTextToFile()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TO_FILE ); }
	wxChoice* GetMpp()  { return (wxChoice*) FindWindow( ID_MPP ); }
	wxChoice* GetTheme()  { return (wxChoice*) FindWindow( ID_THEME ); }

	int m_iTheme;
	int m_iMpp;
	int m_iMetersPerPixel;
	bool m_bNewLayer;
	bool m_bToFile;
	wxString m_strToFile;

private:
	void UpdateMpp();
	void EnableBasedOnConstraint();
	void UpdateMetersPerPixel();

	// WDR: member variable declarations for TSDialog
	int s1, s2;

private:
	// WDR: handler declarations for TSDialog
	void OnDotDotDot( wxCommandEvent &event );
	void OnRadioOutput( wxCommandEvent &event );
	void OnTheme( wxCommandEvent &event );
	void OnMpp( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __TSDlg_H__

