//
// ElevDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/resource.h"
#include <wx/valgen.h>
#include "dialog1.h"
#include "dialog2.h"
#include "ElevDlg.h"
#include "Helper.h"

//////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ScaleDlg, AutoDialog)
END_EVENT_TABLE()

void ScaleDlg::OnInitDialog(wxInitDialogEvent& event)
{
	AddValidator(ID_RB_SCALEUP, &m_bScaleUp);
	AddValidator(ID_TC_UPBY, &m_strUpBy);
	AddValidator(ID_TC_DOWNBY, &m_strDownBy);
	wxWindow::OnInitDialog(event);
}

//////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ResampleDlg, AutoDialog)
EVT_RADIOBUTTON	(IDC_RADIO1, ResampleDlg::OnRadio)
EVT_TEXT		(IDC_SAMPLEFACTOR, ResampleDlg::OnChangeSamplefactor)
EVT_BUTTON		(IDC_LESS, ResampleDlg::OnButtonLess)
EVT_BUTTON		(IDC_MORE, ResampleDlg::OnButtonMore)
END_EVENT_TABLE()

ResampleDlg::ResampleDlg(bool bMeters, float fSampleFactor) : AutoDialog()
{
	m_bMeters = bMeters;
	if (bMeters)
		m_fSampleFactor = fSampleFactor;
	else
		m_fSampleFactor = 1.0f / fSampleFactor;
	m_bSetting = false;
}

void ResampleDlg::RecomputeSamples()
{
	if (m_bConstraint)	// powers of 2 + 1
		m_iXSamples = m_iYSamples = (1 << m_power) + 1;
}


void ResampleDlg::RecomputeSize()
{
	int x, y;
	if (m_bMeters)
	{
		x = (int)(m_area.Width() / (double)m_fSampleFactor + 0.9999) + 1;
		y = (int)(m_area.Height() / (double)m_fSampleFactor + 0.9999) + 1;
		m_strSFLabel = "meters per grid point";
	}
	else
	{
		x = (int)(m_area.Width() * (double)m_fSampleFactor + 0.9999) + 1;
		y = (int)(m_area.Height() * (double)m_fSampleFactor + 0.9999) + 1;
		m_strSFLabel = "grid points per degree";
	}
	m_strCurrentSize = wxString::Format("%d x %d", x, y);
}

void ResampleDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_power = 8;
	m_bConstraint = true;
	if (m_bMeters)
	{
		m_strSaveArea = wxString::Format("%.0f x %.0f meters",
			m_area.Width(), m_area.Height());
	}
	else
	{
		m_strSaveArea = wxString::Format("%s x %s",
			(const char *) FormatCoord(true, m_area.Width()),
			(const char *) FormatCoord(true, m_area.Height()));
	}
	RecomputeSamples();
	RecomputeSize();

	AddValidator(IDC_CURRENTSIZE, &m_strCurrentSize);
	AddNumValidator(IDC_XSAMPLE, &m_iXSamples);
	AddNumValidator(IDC_YSAMPLE, &m_iYSamples);
	AddNumValidator(IDC_SAMPLEFACTOR, &m_fSampleFactor);
	AddValidator(IDC_SAVEAREA, &m_strSaveArea);
	AddValidator(IDC_SFLABEL, &m_strSFLabel);
	AddValidator(IDC_RADIO1, &m_bConstraint);

	m_pLess = FindWindow(IDC_LESS);
	m_pMore = FindWindow(IDC_MORE);

	wxWindow::OnInitDialog(event);
}

void ResampleDlg::OnChangeSamplefactor(wxCommandEvent &event) 
{
	if (m_bSetting) return;
	TransferDataFromWindow();
	RecomputeSize();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void ResampleDlg::OnButtonLess(wxCommandEvent &event) 
{
	m_power--;
	RecomputeSamples();
	TransferDataToWindow();
}

void ResampleDlg::OnButtonMore(wxCommandEvent &event) 
{
	m_power++;
	RecomputeSamples();
	TransferDataToWindow();
}

void ResampleDlg::OnRadio(wxCommandEvent &event) 
{
	TransferDataFromWindow();
	RecomputeSamples();
	TransferDataToWindow();

	m_pLess->Enable(m_bConstraint);
	m_pMore->Enable(m_bConstraint);
}

