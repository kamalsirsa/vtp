//
// Name:		LightDlg.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LightDlg_H__
#define __LightDlg_H__

#ifdef __GNUG__
	#pragma interface "LightDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "cmanager_wdr.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// LightDlg
//----------------------------------------------------------------------------

class LightDlg: public AutoDialog
{
public:
	// constructors and destructors
	LightDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void UseLight(vtMovLight *pLight);
	void UpdateColorBitmaps();

	// WDR: method declarations for LightDlg
	wxTextCtrl* GetDirz()  { return (wxTextCtrl*) FindWindow( ID_DIRZ ); }
	wxTextCtrl* GetDiry()  { return (wxTextCtrl*) FindWindow( ID_DIRY ); }
	wxTextCtrl* GetDirx()  { return (wxTextCtrl*) FindWindow( ID_DIRX ); }
	wxBitmapButton* GetDiffuse()  { return (wxBitmapButton*) FindWindow( ID_DIFFUSE ); }
	wxBitmapButton* GetAmbient()  { return (wxBitmapButton*) FindWindow( ID_AMBIENT ); }
	wxChoice* GetLight()  { return (wxChoice*) FindWindow( ID_LIGHT ); }
	
private:
	// WDR: member variable declarations for LightDlg
	vtMovLight *m_pMovLight;
	vtLight *m_pLight;
	FPoint3 m_dir;

	wxColour m_ambient, m_diffuse;
	wxColourData m_data;

private:
	// WDR: handler declarations for LightDlg
	void OnText( wxCommandEvent &event );
	void OnDiffuse( wxCommandEvent &event );
	void OnAmbient( wxCommandEvent &event );
	void OnLight( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
