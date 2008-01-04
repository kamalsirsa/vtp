//
// CameraDlg.cpp : implementation file
//
#include "StdAfx.h"
#include "vtlib/vtlib.h"
#include "EnviroApp.h"

#include "CameraDlg.h"
#include "EnviroGUI.h"

#define MIN_EXP		-2.0f
#define EXP_RANGE	6.0f
#define MIN_ANG		0.05f
#define ANG_RANGE	1.60f
#define SPEED_MIN	-1.0f
#define SPEED_MAX	4.0f
#define SPEED_RANGE	((SPEED_MAX)-(SPEED_MIN))


/////////////////////////////////////////////////////////////////////////////
// CCameraDlg dialog

CCameraDlg::CCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCameraDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCameraDlg)
	m_fHither = 0.0f;
	m_fViewAngle = 0.0f;
	m_fYon = 0.0f;
	m_fFlightSpeed = 0.0f;
	//}}AFX_DATA_INIT
}


void CCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCameraDlg)
	DDX_Control(pDX, IDC_FSPEED_S, m_FlightSpeed);
	DDX_Control(pDX, IDC_YON_S, m_Yon);
	DDX_Control(pDX, IDC_VANGLE_S, m_ViewAngle);
	DDX_Control(pDX, IDC_HITHER_S, m_Hither);
	DDX_Text(pDX, IDC_HITHER, m_fHither);
	DDV_MinMaxFloat(pDX, m_fHither, 1.e-002f, 10.f);
	DDX_Text(pDX, IDC_VANGLE, m_fViewAngle);
	DDV_MinMaxFloat(pDX, m_fViewAngle, 0.1f, 3.f);
	DDX_Text(pDX, IDC_YON, m_fYon);
	DDV_MinMaxFloat(pDX, m_fYon, 0.1f, 10000.f);
	DDX_Text(pDX, IDC_FSPEED, m_fFlightSpeed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCameraDlg, CDialog)
	//{{AFX_MSG_MAP(CCameraDlg)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_HITHER, OnChangeHither)
	ON_EN_CHANGE(IDC_YON, OnChangeYon)
	ON_EN_CHANGE(IDC_VANGLE, OnChangeVangle)
	ON_EN_CHANGE(IDC_FSPEED, OnChangeFspeed)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCameraDlg message handlers

BOOL CCameraDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateValues();
	m_Hither.SetRange(0, 1000);
	m_Hither.SetTicFreq((int)(1000.0f/EXP_RANGE));

	m_Yon.SetRange(0, 1000);
	m_Yon.SetTicFreq((int)(1000.0f/EXP_RANGE));

	m_ViewAngle.SetRange(0, 1000);
	m_ViewAngle.SetTicFreq((int)(1000.0f / ANG_RANGE / 16));

	m_FlightSpeed.SetRange(1, 99);
	m_FlightSpeed.SetTicFreq(10);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CCameraDlg::UpdateValues()
{
	vtCamera *pCam = vtGetScene()->GetCamera();

	m_fHither = pCam->GetHither();
	m_fYon = pCam->GetYon();
	m_fViewAngle = pCam->GetFOV();

	GetFlightspeed();

	UpdateData(FALSE);
}

void CCameraDlg::SetFlightspeed()
{
	g_App.SetFlightSpeed(m_fFlightSpeed);
}

void CCameraDlg::GetFlightspeed()
{
	m_fFlightSpeed = g_App.GetFlightSpeed();
}

void CCameraDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl *pSlider = (CSliderCtrl *)pScrollBar;

	switch (nSBCode)
	{
		case SB_PAGELEFT:
		case SB_PAGERIGHT:
			SliderMoved(pSlider, pSlider->GetPos());
			break;
		case SB_LEFT:
		case SB_RIGHT:
		case SB_LINELEFT:
		case SB_LINERIGHT:
		case SB_ENDSCROLL:
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
//			SliderMoved(pSlider, nPos);
			SliderMoved(pSlider, pSlider->GetPos());
			break;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCameraDlg::SliderMoved(CSliderCtrl *pSlider, int nPos)
{
	vtCamera *pCam = vtGetScene()->GetCamera();

	int iMin, iMax;
	pSlider->GetRange(iMin, iMax);
	float fNewVal = (float)nPos / (iMax-iMin);

	if (pSlider == &m_Hither)
	{
		// exponential range from 0.1 to 1000.0
		m_fHither = (float)pow(10.0, (double) (fNewVal * EXP_RANGE + MIN_EXP));
		pCam->SetHither(m_fHither);
		UpdateData(FALSE);
	}
	if (pSlider == &m_Yon)
	{
		// exponential range from 0.1 to 1000.0
		m_fYon = (float)pow(10.0, (double) (fNewVal * EXP_RANGE + MIN_EXP));
		pCam->SetYon(m_fYon);
		UpdateData(FALSE);
	}
	if (pSlider == &m_ViewAngle)
	{
		// linear angular range from MIN_ANG to MIN_ANG + ANG_RANGE
		m_fViewAngle = MIN_ANG + fNewVal * ANG_RANGE;
		pCam->SetFOV(m_fViewAngle);
		UpdateData(FALSE);
	}
	if (pSlider == &m_FlightSpeed)
	{
		m_fFlightSpeed = powf(10, (SPEED_MIN + nPos * SPEED_RANGE / 100));
		UpdateData(FALSE);
		SetFlightspeed();
	}
}

void CCameraDlg::OnChangeHither()
{
	vtCamera *pCam = vtGetScene()->GetCamera();

	UpdateData(TRUE);

	// range check
	if (m_fHither > m_fYon)
	{
		m_fHither = m_fYon;
		UpdateData(FALSE);
	}

	// update camera and slider
	pCam->SetHither(m_fHither);
	UpdateSliders();
}

void CCameraDlg::OnChangeYon()
{
	vtCamera *pCam = vtGetScene()->GetCamera();

	UpdateData(TRUE);

	// range check
	if (m_fYon < m_fHither)
	{
		m_fYon = m_fHither;
		UpdateData(FALSE);
	}

	// update camera and slider
	pCam->SetYon(m_fYon);
	UpdateSliders();
}

void CCameraDlg::OnChangeVangle()
{
	vtCamera *pCam = vtGetScene()->GetCamera();

	UpdateData(TRUE);

	// update camera and slider
	pCam->SetFOV(m_fViewAngle);
	UpdateSliders();
}

void CCameraDlg::UpdateSliders()
{
	int iMin, iMax;
	float fNewVal;

	m_Hither.GetRange(iMin, iMax);
	fNewVal = (float)((log10(m_fHither)-MIN_EXP)/EXP_RANGE) * (iMax-iMin);
	m_Hither.SetPos((int)fNewVal);

	fNewVal = (float)((log10(m_fYon)-MIN_EXP)/EXP_RANGE) * (iMax-iMin);
	m_Yon.SetPos((int)fNewVal);

	fNewVal = (float)((m_fViewAngle-MIN_ANG)/ANG_RANGE) * (iMax-iMin);
	m_ViewAngle.SetPos((int)fNewVal);
}

/////////////////

void CCameraDlg::OnChangeFspeed()
{
	UpdateData(TRUE);

	// range check
	if (m_fFlightSpeed < 0.0001f)
	{
		m_fFlightSpeed = 0.0001f;
		UpdateData(FALSE);
	}

	// update flight engine and slider
	SetFlightspeed();
	UpdateSliders();
}

