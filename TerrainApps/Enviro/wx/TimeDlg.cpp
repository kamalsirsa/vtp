//
// Name: TimeDlg.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TimeDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "TimeDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TimeDlg
//----------------------------------------------------------------------------

// WDR: event table for TimeDlg

BEGIN_EVENT_TABLE(TimeDlg, AutoDialog)
	EVT_BUTTON( ID_STOP, TimeDlg::OnStop )
	EVT_TEXT( ID_TEXT_SPEED, TimeDlg::OnTextSpeed )
	EVT_SLIDER( ID_SLIDER_SPEED, TimeDlg::OnSliderSpeed )
	EVT_SPINCTRL( ID_SPIN_YEAR, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_MONTH, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_DAY, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_HOUR, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_MINUTE, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_SECOND, TimeDlg::OnSpinner )
END_EVENT_TABLE()

TimeDlg::TimeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function TimeDialogFunc for TimeDlg
	TimeDialogFunc( this, TRUE ); 

	m_bGoing = false;
	m_iSpeed = 0;
	m_fSpeed = 0.0f;
	m_bSetting = false;

	year = 2000;
	month = 1;
	day = 1;
	hour = 12;
	minute = 0;
	second = 0;

	m_pTimeEngine = NULL;
}

void TimeDlg::PullTime()
{
	if (m_pTimeEngine)
	{
		vtTime time = m_pTimeEngine->GetTime();
		SetTimeControls(time);
	}
}

void TimeDlg::PushTime()
{
	TransferDataFromWindow();

	if (m_pTimeEngine)
	{
		vtTime time;
		time.SetDate(year, month, day);
		time.SetTimeOfDay(hour, minute, second);

		m_pTimeEngine->SetTime(time);

		time.GetDate(year, month, day);
		time.GetTimeOfDay(hour, minute, second);
	}
}

void TimeDlg::SetTime(const vtTime &time)
{
	m_fSpeed = m_pTimeEngine->GetSpeed();
	m_bGoing = (m_fSpeed != 0.0f);

	UpdateSlider();

	UpdateEnabling();

	SetTimeControls(time);
}

void TimeDlg::SetTimeEngine(TimeEngine *pEngine)
{
	m_pTimeEngine = pEngine;

	TimeTarget *self = dynamic_cast<TimeTarget *>(this);

	// If the engine is not yet connected to us, connect it.
	bool bFound = false;
	for (int i = 0; i < pEngine->NumTargets(); i++)
	{
		if (pEngine->GetTarget(i) == self)
			bFound = true;
	}
	if (!bFound)
		pEngine->AddTarget(self);
}

void TimeDlg::SetTimeControls(const vtTime &time)
{
	time.GetDate(year, month, day);
	time.GetTimeOfDay(hour, minute, second);

	TransferDataToWindow();
}

void TimeDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddValidator(ID_SPIN_YEAR, &year);
	AddValidator(ID_SPIN_MONTH, &month);
	AddValidator(ID_SPIN_DAY, &day);

	AddValidator(ID_SPIN_HOUR, &hour);
	AddValidator(ID_SPIN_MINUTE, &minute);
	AddValidator(ID_SPIN_SECOND, &second);

	AddNumValidator(ID_TEXT_SPEED, &m_fSpeed);
	AddValidator(ID_SLIDER_SPEED, &m_iSpeed);

	PullTime();

	wxDialog::OnInitDialog(event);
}

// WDR: handler implementations for TimeDlg

void TimeDlg::OnSpinner( wxSpinEvent &event )
{
	PushTime();
}

void TimeDlg::UpdateEnabling()
{
	GetStop()->Enable(m_bGoing);
}

void TimeDlg::UpdateSlider()
{
	if (m_fSpeed == 0.0f)
		m_iSpeed = 0;
	else
		m_iSpeed = (int) sqrt(m_fSpeed);
	m_bSetting = true;
	GetSliderSpeed()->SetValue(m_iSpeed);
	m_bSetting = false;
}

void TimeDlg::OnSliderSpeed( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_fSpeed = m_iSpeed * m_iSpeed;

	m_bSetting = true;
	m_pTimeEngine->SetSpeed(m_fSpeed);
	m_bSetting = false;
}

void TimeDlg::OnTextSpeed( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateSlider();

	m_pTimeEngine->SetSpeed(m_fSpeed);
}

void TimeDlg::OnStop( wxCommandEvent &event )
{
	m_pTimeEngine->SetSpeed(0.0f);
	UpdateEnabling();
}

