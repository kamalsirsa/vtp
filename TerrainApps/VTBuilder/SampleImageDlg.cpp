//
// Name: SampleImageDlg.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "SampleImageDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "SampleImageDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SampleImageDlg
//----------------------------------------------------------------------------

// WDR: event table for SampleImageDlg

BEGIN_EVENT_TABLE(SampleImageDlg,AutoDialog)
	EVT_BUTTON( ID_SMALLER, SampleImageDlg::OnSmaller )
	EVT_BUTTON( ID_BIGGER, SampleImageDlg::OnBigger )
	EVT_CHECKBOX( ID_CONSTRAIN, SampleImageDlg::OnConstrain )
	EVT_CHECKBOX( ID_TILING, SampleImageDlg::OnConstrain )
	EVT_TEXT( ID_SIZEX, SampleImageDlg::OnSizeXY )
	EVT_TEXT( ID_SIZEY, SampleImageDlg::OnSizeXY )
	EVT_TEXT( ID_SPACINGX, SampleImageDlg::OnSpacingXY )
	EVT_TEXT( ID_SPACINGY, SampleImageDlg::OnSpacingXY )
END_EVENT_TABLE()

SampleImageDlg::SampleImageDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	SampleImageDialogFunc( this, TRUE ); 
	m_bSetting = false;
}

// WDR: handler implementations for SampleImageDlg

void SampleImageDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_power = 8;
	m_bConstraint = false;
	m_bTiling = false;

	m_fAreaX = m_area.Width();
	m_fAreaY = m_area.Height();

	// initial value: based on estimate spacing
	m_fSpacingX = m_fEstX;
	m_fSpacingY = m_fEstY;
	m_iSizeX = (int) (m_fAreaX / m_fSpacingX);
	m_iSizeY = (int) (m_fAreaY / m_fSpacingY);

	// sampling
	AddNumValidator(ID_SPACINGX, &m_fSpacingX);
	AddNumValidator(ID_SPACINGY, &m_fSpacingY);
	AddNumValidator(ID_SIZEX, &m_iSizeX);
	AddNumValidator(ID_SIZEY, &m_iSizeY);
	AddValidator(ID_CONSTRAIN, &m_bConstraint);
	AddValidator(ID_TILING, &m_bTiling);

	// informations
	AddNumValidator(ID_AREAX, &m_fAreaX);
	AddNumValidator(ID_AREAY, &m_fAreaY);

	AddNumValidator(ID_ESTX, &m_fEstX);
	AddNumValidator(ID_ESTY, &m_fEstY);

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	EnableBasedOnConstraint();
}

void SampleImageDlg::RecomputeSize()
{
	if (m_bConstraint)  // powers of 2 + 1
		m_iSizeX = m_iSizeY = (1 << m_power);

	if (m_bConstraint && m_bTiling)
	{
		m_iSizeX -= 3;
		m_iSizeY -= 3;
	}
	m_fSpacingX = m_fAreaX / m_iSizeX;
	m_fSpacingY = m_fAreaY / m_iSizeY;
}

// WDR: handler implementations for SampleImageDlg

void SampleImageDlg::OnSpacingXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_iSizeX = (int) (m_fAreaX / m_fSpacingX);
	m_iSizeY = (int) (m_fAreaY / m_fSpacingY);

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::OnSizeXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::OnConstrain( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	if (m_bConstraint)
	{
		// round up to a value at least as great as the current size
		m_power = 1;
		while (((1 << m_power) + 1) < m_iSizeX ||
			   ((1 << m_power) + 1) < m_iSizeY)
			m_power++;
	}
	RecomputeSize();
	EnableBasedOnConstraint();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::EnableBasedOnConstraint()
{
	GetSmaller()->Enable(m_bConstraint);
	GetBigger()->Enable(m_bConstraint);
	GetTiling()->Enable(m_bConstraint);
	GetSizeX()->SetEditable(!m_bConstraint);
	GetSizeY()->SetEditable(!m_bConstraint);
	GetSpacingX()->SetEditable(!m_bConstraint);
	GetSpacingY()->SetEditable(!m_bConstraint);
}

void SampleImageDlg::OnBigger( wxCommandEvent &event )
{
	m_power++;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::OnSmaller( wxCommandEvent &event )
{
	m_power--;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}



