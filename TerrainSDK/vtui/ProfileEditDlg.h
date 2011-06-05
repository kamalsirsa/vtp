//
// Name: ProfileEditDlg.h
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ProfileEditDlg_H__
#define __ProfileEditDlg_H__

#include "vtui_UI.h"
#include "vtdata/MathTypes.h"

//----------------------------------------------------------------------------
// ProfileEditDlg
//----------------------------------------------------------------------------

class ProfileEditDlg: public ProfileEditDlgBase
{
public:
	// constructors and destructors
	ProfileEditDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ProfileEditDlg
	void UpdateEnabling();
	wxButton* GetSave()  { return (wxButton*) FindWindow( ID_SAVE_PROF ); }

	void SetFilename(const char *fname);
	void CheckClockwisdom();

	// The profile view methods
	void OnViewSize(wxSizeEvent& event);
	void NewSize(const wxSize &s);
	void OnViewPaint();
	void OnViewMouseEvent(wxMouseEvent &event);

	// The profile view members
	FLine2 m_profile;
	vtArray<wxPoint> m_screen;
	wxPoint m_org;
	float m_scale;
	int m_mode;
	bool m_bDragging;
	int m_iDragging;

private:
	// WDR: member variable declarations for ProfileEditDlg
	//ProfDlgView *m_pView;
	wxString	m_strFilename;

private:
	// WDR: handler declarations for ProfileEditDlg
	void OnOK( wxCommandEvent &event );
	void OnLoad( wxCommandEvent &event );
	void OnSaveAs( wxCommandEvent &event );
	void OnSave( wxCommandEvent &event );
	void OnRemove( wxCommandEvent &event );
	void OnMove( wxCommandEvent &event );
	void OnAdd( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __ProfileEditDlg_H__
