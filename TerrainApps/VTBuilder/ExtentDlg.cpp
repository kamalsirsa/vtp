//
// Name: ExtentDlg.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "ExtentDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "ExtentDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ExtentDlg
//----------------------------------------------------------------------------

// WDR: event table for ExtentDlg

BEGIN_EVENT_TABLE(ExtentDlg,AutoDialog)
	EVT_TEXT_ENTER( ID_EXTENT_N, ExtentDlg::OnExtentN )
	EVT_TEXT_ENTER( ID_EXTENT_W, ExtentDlg::OnExtentW )
	EVT_TEXT_ENTER( ID_EXTENT_E, ExtentDlg::OnExtentE )
	EVT_TEXT_ENTER( ID_EXTENT_S, ExtentDlg::OnExtentS )
	EVT_TEXT( ID_EXTENT_ALL, ExtentDlg::OnExtentAll )
	EVT_CHECKBOX( ID_DMS, ExtentDlg::OnDMS )
END_EVENT_TABLE()

ExtentDlg::ExtentDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;
	m_bDMS = false;
	ExtentDialogFunc( this, TRUE ); 
}

void ExtentDlg::SetArea(DRECT area, bool bDegrees)
{
	m_area = area;
	m_bDegrees = bDegrees;
	if (m_bDegrees)
		m_fs = _T("%4.8f");
	else
		m_fs = _T("%8.1f");
}

void ExtentDlg::FormatExtent(wxString &str, double value)
{
	if (m_bDMS)
	{
		bool sign = value > 0;
		value = fabs(value);
		int degrees = (int) value;
		value = (value - degrees) * 60;
		int minutes = (int) value;
		value = (value - minutes) * 60;
		double seconds = value;

		str.Printf(_T("%s%d %d %.2lf"), sign?"":"-", degrees, minutes, seconds);
	}
	else
		str.Printf(m_fs, value);
}

double ExtentDlg::GetValueFrom(const wxString2 &str)
{
	if (m_bDMS)
	{
		const char *cstr = str.mb_str();
		int degrees, minutes;
		double seconds;
		sscanf(cstr, "%d %d %lf", &degrees, &minutes, &seconds);
		bool negative = (degrees < 0);
		if (negative)
			degrees = -degrees;
		double value = degrees + (minutes / 60.0) + (seconds / 3600.0);
		if (negative)
			value = -value;
		return value;
	}
	else
		return atof(str.mb_str());
}

void ExtentDlg::FormatStrings(int which)
{
	m_bSetting = true;
	if (which == 1)
	{
		FormatExtent(m_strWest, m_area.left);
		FormatExtent(m_strEast, m_area.right);
		FormatExtent(m_strNorth, m_area.top);
		FormatExtent(m_strSouth, m_area.bottom);
	}
	if (which == 2)
	{
		m_strAll.Printf(_T("(")+m_fs+_T(", ")+m_fs+_T("), (")+m_fs+_T(", ")+m_fs+_T(")"),
			m_area.left, m_area.bottom, m_area.Width(), m_area.Height());
	}
	TransferDataToWindow();
	m_bSetting = false;
}

// WDR: handler implementations for ExtentDlg

void ExtentDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	GetDMS()->Enable(m_bDegrees);

	AddValidator(ID_EXTENT_ALL, &m_strAll);
	AddValidator(ID_EXTENT_E, &m_strEast);
	AddValidator(ID_EXTENT_N, &m_strNorth);
	AddValidator(ID_EXTENT_S, &m_strSouth);
	AddValidator(ID_EXTENT_W, &m_strWest);
	AddValidator(ID_DMS, &m_bDMS);

	FormatStrings(1);
	FormatStrings(2);

	wxWindow::OnInitDialog(event);
}

void ExtentDlg::OnExtentAll( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	double d1, d2, d3, d4;
	int ret = sscanf(m_strAll.mb_str(), "(%lf, %lf), (%lf, %lf)",
		&d1, &d2, &d3, &d4);
	if (ret == 4)
	{
		m_area.left = d1;
		m_area.right = d1 + d3;
		m_area.bottom = d2;
		m_area.top = d2 + d4;
		FormatStrings(1);
	}
}

void ExtentDlg::OnExtentS( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.bottom = GetValueFrom(m_strSouth);
	FormatStrings(2);
}

void ExtentDlg::OnExtentE( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.right = GetValueFrom(m_strEast);
	FormatStrings(2);
}

void ExtentDlg::OnExtentW( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.left = GetValueFrom(m_strWest);
	FormatStrings(2);   
}

void ExtentDlg::OnExtentN( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.top = GetValueFrom(m_strNorth);
	FormatStrings(2);
}

void ExtentDlg::OnDMS( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	FormatStrings(1);
}

