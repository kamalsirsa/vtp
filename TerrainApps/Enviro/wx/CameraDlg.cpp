//
// Name:		CameraDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "CameraDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtlib/vtlib.h"
#include "CameraDlg.h"
#include "../Enviro.h"
#include "vtlib/core/LocalProjection.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CameraDlg
//----------------------------------------------------------------------------

// WDR: event table for CameraDlg

BEGIN_EVENT_TABLE(CameraDlg,AutoDialog)
	EVT_TEXT( ID_FOV, CameraDlg::OnFov )
	EVT_TEXT( ID_NEAR, CameraDlg::OnNear )
	EVT_TEXT( ID_FAR, CameraDlg::OnFar )
	EVT_TEXT( ID_SPEED, CameraDlg::OnSpeed )
	EVT_SLIDER( ID_FOVSLIDER, CameraDlg::OnFovSlider )
	EVT_SLIDER( ID_NEARSLIDER, CameraDlg::OnNearSlider )
	EVT_SLIDER( ID_FARSLIDER, CameraDlg::OnFarSlider )
	EVT_SLIDER( ID_SPEEDSLIDER, CameraDlg::OnSpeedSlider )
END_EVENT_TABLE()

CameraDlg::CameraDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	CameraDialogFunc( this, TRUE );
	m_bSet = true;
}

#define FOV_MIN		2.0f
#define FOV_RANGE	128.0f
#define CLIP_MIN	0.0f
#define CLIP_MAX	6.0f
#define CLIP_RANGE	((CLIP_MAX)-(CLIP_MIN))
#define SPEED_MIN	-1.0f
#define SPEED_MAX	4.0f
#define SPEED_RANGE	((SPEED_MAX)-(SPEED_MIN))

void CameraDlg::SlidersToValues(int w)
{
	if (w == 1)	m_fFov =	FOV_MIN + (m_iFov * FOV_RANGE / 100);
	if (w == 2)	m_fNear =	powf(10, (CLIP_MIN + m_iNear * CLIP_RANGE / 100));
	if (w == 3)	m_fFar =	powf(10, (CLIP_MIN + m_iFar * CLIP_RANGE / 100));
	if (w == 4)	m_fSpeed =	powf(10, (SPEED_MIN + m_iSpeed * SPEED_RANGE / 100));
}

void CameraDlg::ValuesToSliders()
{
	m_iFov =	(int) ((m_fFov - FOV_MIN) / FOV_RANGE * 100);
	m_iNear =	(int) ((log10f(m_fNear) - CLIP_MIN) / CLIP_RANGE * 100);
	m_iFar =	(int) ((log10f(m_fFar) - CLIP_MIN) / CLIP_RANGE * 100);
	m_iSpeed =	(int) ((log10f(m_fSpeed) - SPEED_MIN) / SPEED_RANGE * 100);
}

void CameraDlg::GetValues()
{
	vtCamera *pCam = vtGetScene()->GetCamera();
	m_fFov = pCam->GetFOV() * 180.0f / PIf;
	m_fNear = pCam->GetHither() / WORLD_SCALE;
	m_fFar = pCam->GetYon() / WORLD_SCALE;
	m_fSpeed = g_App.GetFlightSpeed() / WORLD_SCALE;
}

void CameraDlg::SetValues()
{
	if (m_bSet)
	{
		vtCamera *pCam = vtGetScene()->GetCamera();
		pCam->SetFOV(m_fFov / 180.0f * PIf);
		pCam->SetHither(m_fNear * WORLD_SCALE);
		pCam->SetYon(m_fFar * WORLD_SCALE);
		g_App.SetFlightSpeed(m_fSpeed * WORLD_SCALE);
	}
}

void CameraDlg::TransferToWindow()
{
	m_bSet = false;
	TransferDataToWindow();
	m_bSet = true;
}

// WDR: handler implementations for CameraDlg

void CameraDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddNumValidator(ID_FOV, &m_fFov);
	AddNumValidator(ID_NEAR, &m_fNear);
	AddNumValidator(ID_FAR, &m_fFar);
	AddNumValidator(ID_SPEED, &m_fSpeed);

	AddValidator(ID_FOVSLIDER, &m_iFov);
	AddValidator(ID_NEARSLIDER, &m_iNear);
	AddValidator(ID_FARSLIDER, &m_iFar);
	AddValidator(ID_SPEEDSLIDER, &m_iSpeed);

	GetValues();
	ValuesToSliders();
	TransferToWindow();
}

void CameraDlg::OnSpeedSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SlidersToValues(4);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnFarSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SlidersToValues(3);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnNearSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SlidersToValues(2);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnFovSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SlidersToValues(1);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSpeed( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnFar( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnNear( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnFov( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues();
	TransferToWindow();
}




