//
// Name:		CameraDlg.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "CameraDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "CameraDlg.h"
#include "EnviroGUI.h"
#include "vtdata/LocalConversion.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CameraDlg
//----------------------------------------------------------------------------

// WDR: event table for CameraDlg

BEGIN_EVENT_TABLE(CameraDlg,AutoDialog)
	EVT_INIT_DIALOG (CameraDlg::OnInitDialog)

	EVT_TEXT_ENTER( ID_CAMX, CameraDlg::OnTextEnter )
	EVT_TEXT_ENTER( ID_CAMY, CameraDlg::OnTextEnter )
	EVT_TEXT_ENTER( ID_CAMZ, CameraDlg::OnTextEnter )

	EVT_TEXT( ID_FOV, CameraDlg::OnText )
	EVT_TEXT( ID_NEAR, CameraDlg::OnText )
	EVT_TEXT( ID_FAR, CameraDlg::OnText )
	EVT_TEXT( ID_SPEED, CameraDlg::OnText )

	EVT_TEXT( ID_LOD_VEG, CameraDlg::OnText )
	EVT_TEXT( ID_LOD_STRUCT, CameraDlg::OnText )
	EVT_TEXT( ID_LOD_ROAD, CameraDlg::OnText )

	EVT_SLIDER( ID_FOVSLIDER, CameraDlg::OnFovSlider )
	EVT_SLIDER( ID_NEARSLIDER, CameraDlg::OnNearSlider )
	EVT_SLIDER( ID_FARSLIDER, CameraDlg::OnFarSlider )
	EVT_SLIDER( ID_SPEEDSLIDER, CameraDlg::OnSpeedSlider )

	EVT_SLIDER( ID_SLIDER_VEG, CameraDlg::OnSliderVeg )
	EVT_SLIDER( ID_SLIDER_STRUCT, CameraDlg::OnSliderStruct )
	EVT_SLIDER( ID_SLIDER_ROAD, CameraDlg::OnSliderRoad )
	EVT_CHOICE( ID_SPEED_UNITS, CameraDlg::OnSpeedUnits )
	EVT_CHECKBOX( ID_ACCEL, CameraDlg::OnAccel )
END_EVENT_TABLE()

CameraDlg::CameraDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_iSpeedUnits = 0;
	CameraDialogFunc( this, TRUE );
	m_bSet = true;

	AddValidator(ID_CAMX, &m_camX);
	AddValidator(ID_CAMY, &m_camY);
	AddValidator(ID_CAMZ, &m_camZ);

	AddNumValidator(ID_FOV, &m_fFov);
	AddNumValidator(ID_NEAR, &m_fNear);
	AddNumValidator(ID_FAR, &m_fFar);
	AddNumValidator(ID_SPEED, &m_fSpeed);

	GetSpeedUnits()->Append(_("Meters/sec"));
	GetSpeedUnits()->Append(_("Km/hour"));
	GetSpeedUnits()->Append(_("Miles/hour"));
	AddValidator(ID_SPEED_UNITS, &m_iSpeedUnits);
	AddValidator(ID_ACCEL, &m_bAccel);

	AddNumValidator(ID_LOD_VEG, &m_fDistVeg);
	AddNumValidator(ID_LOD_STRUCT, &m_fDistStruct);
	AddNumValidator(ID_LOD_ROAD, &m_fDistRoad);

	AddValidator(ID_FOVSLIDER, &m_iFov);
	AddValidator(ID_NEARSLIDER, &m_iNear);
	AddValidator(ID_FARSLIDER, &m_iFar);
	AddValidator(ID_SPEEDSLIDER, &m_iSpeed);

	AddValidator(ID_SLIDER_VEG, &m_iDistVeg);
	AddValidator(ID_SLIDER_STRUCT, &m_iDistStruct);
	AddValidator(ID_SLIDER_ROAD, &m_iDistRoad);
}

#define FOV_MIN  2.0f
#define FOV_RANGE   128.0f
#define CLIP_MIN	0.0f
#define CLIP_MAX	6.0f
#define CLIP_RANGE  (CLIP_MAX-(CLIP_MIN)) // 1.0 to 1000000 meters
#define SPEED_MIN   -1.0f
#define SPEED_MAX   4.0f
#define SPEED_RANGE (SPEED_MAX-(SPEED_MIN)) // 0.1 to 10000 meters/sec
#define DIST_MIN	1.0f
#define DIST_MAX	5.0f
#define DIST_RANGE  (DIST_MAX-(DIST_MIN)) // 10 to 100000 meters

void CameraDlg::SlidersToValues(int w)
{
	if (w == 1) m_fFov =	FOV_MIN + (m_iFov * FOV_RANGE / 100);
	if (w == 2) m_fNear =   powf(10, (CLIP_MIN + m_iNear * CLIP_RANGE / 100));
	if (w == 3) m_fFar =	powf(10, (CLIP_MIN + m_iFar * CLIP_RANGE / 100));
	if (w == 4) m_fSpeed =  powf(10, (SPEED_MIN + m_iSpeed * SPEED_RANGE / 100));

	if (w == 5) m_fDistVeg =	powf(10, (DIST_MIN + m_iDistVeg * DIST_RANGE / 100));
	if (w == 6) m_fDistStruct = powf(10, (DIST_MIN + m_iDistStruct * DIST_RANGE / 100));
	if (w == 7) m_fDistRoad =   powf(10, (DIST_MIN + m_iDistRoad * DIST_RANGE / 100));

	// safety check to prevent user from putting Near > Far
	if (m_fNear >= m_fFar)
		m_fNear = m_fFar -1;
}

void CameraDlg::ValuesToSliders()
{
	m_iFov =	(int) ((m_fFov - FOV_MIN) / FOV_RANGE * 100);
	m_iNear =   (int) ((log10f(m_fNear) - CLIP_MIN) / CLIP_RANGE * 100);
	m_iFar =	(int) ((log10f(m_fFar) - CLIP_MIN) / CLIP_RANGE * 100);
	m_iSpeed =  (int) ((log10f(m_fSpeed) - SPEED_MIN) / SPEED_RANGE * 100);

	m_iDistVeg =	(int) ((log10f(m_fDistVeg) - DIST_MIN) / DIST_RANGE * 100);
	m_iDistStruct = (int) ((log10f(m_fDistStruct) - DIST_MIN) / DIST_RANGE * 100);
	m_iDistRoad =   (int) ((log10f(m_fDistRoad) - DIST_MIN) / DIST_RANGE * 100);
}

void CameraDlg::GetValues()
{
	vtCamera *pCam = vtGetScene()->GetCamera();
	m_fFov = pCam->GetFOV() * 180.0f / PIf;
	m_fNear = pCam->GetHither();
	m_fFar = pCam->GetYon();

	float speed = g_App.GetFlightSpeed();
	switch (m_iSpeedUnits)
	{
	case 0: // m/s
		m_fSpeed = speed;
		break;
	case 1: // Km/h
		m_fSpeed = speed / 1000 * (60*60);  // m -> km, sec -> hour
		break;
	case 2: // Miles/h
		m_fSpeed = speed / 1000 * (60*60);  // m -> km, sec -> hour
		m_fSpeed /= 1.609347;   // km -> miles
		break;
	}
	m_bAccel = g_App.GetFlightAccel();

	vtTerrain *t = GetCurrentTerrain();
	if (t)
	{
		m_fDistVeg =	t->GetLODDistance(TFT_VEGETATION);
		m_fDistStruct = t->GetLODDistance(TFT_STRUCTURES);
		m_fDistRoad =   t->GetLODDistance(TFT_ROADS);
	}
}

void CameraDlg::SetValues()
{
	if (!m_bSet)
		return;

	vtCamera *pCam = vtGetScene()->GetCamera();
	pCam->SetFOV(m_fFov / 180.0f * PIf);
	pCam->SetHither(m_fNear);
	pCam->SetYon(m_fFar);

	float speed=0;
	switch (m_iSpeedUnits)
	{
	case 0: // m/s
		speed = m_fSpeed;
		break;
	case 1: // Km/h
		speed = m_fSpeed * 1000 / (60*60);  // km -> m, hour -> sec
		break;
	case 2: // Miles/h
		speed = m_fSpeed * 1000 / (60*60);  // km -> m, hour -> sec
		speed *= 1.609347;   // miles -> km
		break;
	}
	g_App.SetFlightSpeed(speed);
	g_App.SetFlightAccel(m_bAccel);

	vtTerrain *t = GetCurrentTerrain();
	if (t)
	{
		t->SetLODDistance(TFT_VEGETATION, m_fDistVeg);
		t->SetLODDistance(TFT_STRUCTURES, m_fDistStruct);
		t->SetLODDistance(TFT_ROADS, m_fDistRoad);
	}
}

void CameraDlg::CheckAndUpdatePos()
{
	FPoint3 fpos = vtGetScene()->GetCamera()->GetTrans();
	g_Conv.ConvertToEarth(fpos, m_pos);

	wxString2 newx, newy, newz;
	newx.Printf(_T("%.7g"), m_pos.x);
	newy.Printf(_T("%.7g"), m_pos.y);
	newz.Printf(_T("%.7g"), m_pos.z);
	if (newx != m_camX || newy != m_camY || newz != m_camZ)
	{
		// new to refresh values
		m_camX = newx;
		m_camY = newy;
		m_camZ = newz;
		TransferDataToWindow();
	}
}

void CameraDlg::TransferToWindow()
{
	m_bSet = false;
	TransferDataToWindow();
	m_bSet = true;
}

// WDR: handler implementations for CameraDlg

void CameraDlg::OnAccel( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SetValues();
}

void CameraDlg::OnSpeedUnits( wxCommandEvent &event )
{
	TransferDataFromWindow();
	GetValues();
	ValuesToSliders();
	TransferToWindow();
}

void CameraDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetValues();
	ValuesToSliders();
	wxDialog::OnInitDialog(event);	// calls TransferDataToWindow
}

void CameraDlg::OnFovSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(1);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnNearSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(2);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnFarSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(3);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSpeedSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(4);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSliderVeg( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(5);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSliderStruct( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(6);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSliderRoad( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(7);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnText( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnTextEnter( wxCommandEvent &event )
{
	TransferDataFromWindow();

	m_pos.x = atof(m_camX.mb_str());
	m_pos.y = atof(m_camY.mb_str());
	m_pos.z = atof(m_camZ.mb_str());

	FPoint3 fpos;
	g_Conv.ConvertFromEarth(m_pos, fpos);
	vtGetScene()->GetCamera()->SetTrans(fpos);
}

