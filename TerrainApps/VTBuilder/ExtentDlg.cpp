//
// Name:        ExtentDlg.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
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
    EVT_TEXT( ID_EXTENT_N, ExtentDlg::OnExtentN )
    EVT_TEXT( ID_EXTENT_W, ExtentDlg::OnExtentW )
    EVT_TEXT( ID_EXTENT_E, ExtentDlg::OnExtentE )
    EVT_TEXT( ID_EXTENT_S, ExtentDlg::OnExtentS )
    EVT_TEXT( ID_EXTENT_ALL, ExtentDlg::OnExtentAll )
END_EVENT_TABLE()

ExtentDlg::ExtentDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;
    ExtentDialogFunc( this, TRUE ); 
}

void ExtentDlg::SetArea(DRECT area, bool bMeters)
{
	m_area = area;
	m_bMeters = bMeters;
}

void ExtentDlg::FormatStrings(int which)
{
	wxString fs;		// format string depends on coordiante scheme
	if (m_bMeters)
		fs = "%8.1f";
	else
		fs = "%4.8f";

	m_bSetting = true;
	if (which == 1)
	{
		m_strWest.Printf(fs, m_area.left);
		m_strEast.Printf(fs, m_area.right);
		m_strNorth.Printf(fs, m_area.top);
		m_strSouth.Printf(fs, m_area.bottom);
	}
	if (which == 2)
	{
		m_strAll.Printf("("+fs+", "+fs+"), ("+fs+", "+fs+")",
			m_area.left, m_area.bottom, m_area.Width(), m_area.Height());
	}
	TransferDataToWindow();
	m_bSetting = false;
}

// WDR: handler implementations for ExtentDlg

void ExtentDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	AddValidator(ID_EXTENT_ALL, &m_strAll);
	AddValidator(ID_EXTENT_E, &m_strEast);
	AddValidator(ID_EXTENT_N, &m_strNorth);
	AddValidator(ID_EXTENT_S, &m_strSouth);
	AddValidator(ID_EXTENT_W, &m_strWest);

	FormatStrings(1);
	FormatStrings(2);

	wxWindow::OnInitDialog(event);
}

void ExtentDlg::OnExtentAll( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	double d1, d2, d3, d4;
	int ret = sscanf(m_strAll, "(%lf, %lf), (%lf, %lf)", &d1, &d2, &d3, &d4);
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
	m_area.bottom = atof(m_strSouth);
	FormatStrings(2);
}

void ExtentDlg::OnExtentE( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.right = atof(m_strEast);
	FormatStrings(2);
}

void ExtentDlg::OnExtentW( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.left = atof(m_strWest);
	FormatStrings(2);	
}

void ExtentDlg::OnExtentN( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.top = atof(m_strNorth);
	FormatStrings(2);
}




