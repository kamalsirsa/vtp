//
// Name:	CameraDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __CameraDlg_H__
#define __CameraDlg_H__

#ifdef __GNUG__
	#pragma interface "CameraDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// CameraDlg
//----------------------------------------------------------------------------

class CameraDlg: public AutoDialog
{
public:
	// constructors and destructors
	CameraDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for CameraDlg
	void SlidersToValues(int w);
	void ValuesToSliders();
	void GetValues();
	void SetValues();
	void TransferToWindow();

private:
	// WDR: member variable declarations for CameraDlg
	int m_iFov;
	int m_iNear;
	int m_iFar;
	int m_iSpeed;
	float m_fFov;
	float m_fNear;
	float m_fFar;
	float m_fSpeed;
	bool m_bSet;

private:
	// WDR: handler declarations for CameraDlg
	void OnSpeedSlider( wxCommandEvent &event );
	void OnFarSlider( wxCommandEvent &event );
	void OnNearSlider( wxCommandEvent &event );
	void OnFovSlider( wxCommandEvent &event );
	void OnSpeed( wxCommandEvent &event );
	void OnFar( wxCommandEvent &event );
	void OnNear( wxCommandEvent &event );
	void OnFov( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
