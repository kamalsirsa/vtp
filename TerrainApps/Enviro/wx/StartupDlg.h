//
// Name: StartupDlg.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __StartupDlg_H__
#define __StartupDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "StartupDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "../Options.h"

class EnviroOptions;

// WDR: class declarations

//----------------------------------------------------------------------------
// StartupDlg
//----------------------------------------------------------------------------

class StartupDlg: public AutoDialog
{
public:
	// constructors and destructors
	StartupDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	void GetOptionsFrom(EnviroOptions &opt);
	void PutOptionsTo(EnviroOptions &opt);
	void UpdateState();
	void RefreshTerrainChoices();

	// WDR: method declarations for StartupDlg
	wxButton* GetTerrMan()  { return (wxButton*) FindWindow( ID_TERRMAN ); }
	wxButton* GetEditprop()  { return (wxButton*) FindWindow( ID_EDITPROP ); }
	wxChoice* GetTname()  { return (wxChoice*) FindWindow( ID_TNAME ); }
	wxStaticText* GetImagetext()  { return (wxStaticText*) FindWindow( ID_IMAGETEXT ); }
	wxComboBox* GetImage()  { return (wxComboBox*) FindWindow( ID_IMAGE ); }

private:
	// WDR: member variable declarations for StartupDlg
	EnviroOptions	m_opt;

	bool	m_bStartEarth;
	bool	m_bStartTerrain;
	wxString2   m_strTName;
	wxString2   m_strEarthImage;
	wxStaticText  *m_psImage;
	wxComboBox  *m_pImage;

private:
	// WDR: handler declarations for StartupDlg
	void OnInitDialog(wxInitDialogEvent& event);
	void OnTnameChoice( wxCommandEvent &event );
	void OnTerrMan( wxCommandEvent &event );
	void OnTerrain( wxCommandEvent &event );
	void OnEarthView( wxCommandEvent &event );
	void OnOpenGLInfo( wxCommandEvent &event );
	void OnOptions( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );
	void OnEditProp( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __StartupDlg_H__

