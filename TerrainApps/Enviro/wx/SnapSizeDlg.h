//
// Name: SnapSizeDlg.h
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SnapSizeDlg_H__
#define __SnapSizeDlg_H__

#include "vtdata/MathTypes.h"
#include "vtui/AutoDialog.h"
#include "enviro_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// SnapSizeDlg
//----------------------------------------------------------------------------

class SnapSizeDlg: public AutoDialog
{
public:
	// constructors and destructors
	SnapSizeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for SnapSizeDlg
	wxSlider* GetSliderRatio()  { return (wxSlider*) FindWindow( ID_SLIDER_RATIO ); }
	wxTextCtrl* GetTextY()  { return (wxTextCtrl*) FindWindow( ID_TEXT_Y ); }
	wxTextCtrl* GetTextX()  { return (wxTextCtrl*) FindWindow( ID_TEXT_X ); }

	void SetBase(const IPoint2 &size);
	void Update();

	IPoint2 m_Base, m_Current;
	int m_iRatio;
	float m_fRatio;

private:
	// WDR: member variable declarations for SnapSizeDlg

private:
	// WDR: handler declarations for SnapSizeDlg
	void OnSlider( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __SnapSizeDlg_H__
