//
// Name: TimeDlg.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TimeDlg_H__
#define __TimeDlg_H__

#ifdef __GNUG__
	#pragma interface "TimeDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/vtTime.h"
#include "vtlib/core/TimeEngines.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TimeDlg
//----------------------------------------------------------------------------

class TimeDlg: public AutoDialog, public TimeTarget
{
public:
	// constructors and destructors
	TimeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	void OnInitDialog(wxInitDialogEvent& event);

	// WDR: method declarations for TimeDlg
	wxSlider* GetSliderSpeed()  { return (wxSlider*) FindWindow( ID_SLIDER_SPEED ); }
	wxTextCtrl* GetTextSpeed()  { return (wxTextCtrl*) FindWindow( ID_TEXT_SPEED ); }
	wxButton* GetStop()  { return (wxButton*) FindWindow( ID_STOP ); }
	wxSpinCtrl* GetSpinSecond()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_SECOND ); }
	wxSpinCtrl* GetSpinMinute()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_MINUTE ); }
	wxSpinCtrl* GetSpinHour()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_HOUR ); }
	wxSpinCtrl* GetSpinDay()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_DAY ); }
	wxSpinCtrl* GetSpinMonth()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_MONTH ); }
	wxSpinCtrl* GetSpinYear()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_YEAR ); }

	// implement TimeTarget method, to catch events from the engine
	void SetTime(const vtTime &time);
	void SetTimeEngine(TimeEngine *pEngine);

private:
	void PullTime();
	void PushTime();
	void UpdateEnabling();
	void UpdateSlider();
	void SetTimeControls(const vtTime &time);

	// WDR: member variable declarations for TimeDlg
	int year, month, day;
	int hour, minute, second;

	bool m_bGoing;
	int m_iSpeed;
	float m_fSpeed;

	bool m_bSetting;

	TimeEngine	*m_pTimeEngine;

private:
	// WDR: handler declarations for TimeDlg
	void OnSpinner( wxSpinEvent& event );
	void OnSliderSpeed( wxCommandEvent &event );
	void OnTextSpeed( wxCommandEvent &event );
	void OnStop( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
