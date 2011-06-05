//
// Name: StartupDlg.h
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __StartupDlg_H__
#define __StartupDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "../Options.h"

class EnviroOptions;

// WDR: class declarations

//----------------------------------------------------------------------------
// StartupDlg
//----------------------------------------------------------------------------

class StartupDlg: public StartupDlgBase
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
	wxString   m_strTName;
	wxString   m_strEarthImage;
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

