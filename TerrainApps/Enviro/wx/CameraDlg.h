//
// Name: CameraDlg.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __CameraDlg_H__
#define __CameraDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
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
	wxChoice* GetSpeedUnits()  { return (wxChoice*) FindWindow( ID_SPEED_UNITS ); }
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
	int m_iDistVeg;
	int m_iDistStruct;
	int m_iDistRoad;

	bool m_bAccel;
	int m_iSpeedUnits;

	float m_fFov;
	float m_fNear;
	float m_fFar;
	float m_fSpeed;
	float m_fDistVeg;
	float m_fDistStruct;
	float m_fDistRoad;

	bool m_bSet;

private:
	// WDR: handler declarations for CameraDlg
	void OnAccel( wxCommandEvent &event );
	void OnSpeedUnits( wxCommandEvent &event );
	void OnSpeedSlider( wxCommandEvent &event );
	void OnFarSlider( wxCommandEvent &event );
	void OnNearSlider( wxCommandEvent &event );
	void OnFovSlider( wxCommandEvent &event );

	void OnSliderVeg( wxCommandEvent &event );
	void OnSliderStruct( wxCommandEvent &event );
	void OnSliderRoad( wxCommandEvent &event );

	void OnText( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
