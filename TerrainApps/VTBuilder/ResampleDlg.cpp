//
// Name: ResampleDlg.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "ResampleDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "ResampleDlg.h"
#include "Layer.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ResampleDlg
//----------------------------------------------------------------------------

// WDR: event table for ResampleDlg

BEGIN_EVENT_TABLE(ResampleDlg, AutoDialog)
	EVT_INIT_DIALOG (ResampleDlg::OnInitDialog)
	EVT_BUTTON( ID_SMALLER, ResampleDlg::OnSmaller )
	EVT_BUTTON( ID_BIGGER, ResampleDlg::OnBigger )
	EVT_CHECKBOX( ID_CONSTRAIN, ResampleDlg::OnConstrain )
	EVT_TEXT( ID_SIZEX, ResampleDlg::OnSizeXY )
	EVT_TEXT( ID_SIZEY, ResampleDlg::OnSizeXY )
	EVT_TEXT( ID_SPACINGX, ResampleDlg::OnSpacingXY )
	EVT_TEXT( ID_SPACINGY, ResampleDlg::OnSpacingXY )
	EVT_RADIOBUTTON( ID_FLOATS, ResampleDlg::OnFloats )
	EVT_RADIOBUTTON( ID_SHORTS, ResampleDlg::OnShorts )
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, ResampleDlg::OnRadioOutput )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, ResampleDlg::OnRadioOutput )
	EVT_BUTTON( ID_DOTDOTDOT, ResampleDlg::OnDotDotDot )
END_EVENT_TABLE()

ResampleDlg::ResampleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	ResampleDialogFunc( this, TRUE );
	m_bSetting = false;

	// output options
	AddValidator(ID_RADIO_CREATE_NEW, &m_bNewLayer);
	AddValidator(ID_RADIO_TO_FILE, &m_bToFile);
	AddValidator(ID_TEXT_TO_FILE, &m_strToFile);

	// sampling
	spacing1 = AddNumValidator(ID_SPACINGX, &m_fSpacingX);
	spacing2 = AddNumValidator(ID_SPACINGY, &m_fSpacingY);
	AddNumValidator(ID_SIZEX, &m_iSizeX);
	AddNumValidator(ID_SIZEY, &m_iSizeY);
	AddValidator(ID_CONSTRAIN, &m_bConstraint);

	// output grid
	AddValidator(ID_FLOATS, &m_bFloats);
	AddNumValidator(ID_VUNITS, &m_fVUnits);

	// informations
	AddNumValidator(ID_AREAX, &m_fAreaX);
	AddNumValidator(ID_AREAY, &m_fAreaY);

	AddNumValidator(ID_ESTX, &m_fEstX);
	AddNumValidator(ID_ESTY, &m_fEstY);
}

void ResampleDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_bNewLayer = true;
	m_bToFile = false;

	m_power = 8;
	m_bConstraint = false;
	m_fVUnits = 1.0f;

	m_fAreaX = m_area.Width();
	m_fAreaY = m_area.Height();

	// initial value: based on estimate spacing
	m_fSpacingX = m_fEstX;
	m_fSpacingY = m_fEstY;
	m_iSizeX = ((int) (m_fAreaX / m_fSpacingX + 0.5)) + 1;
	m_iSizeY = ((int) (m_fAreaY / m_fSpacingY + 0.5)) + 1;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	EnableBasedOnConstraint();

	GetShorts()->SetValue(!m_bFloats);
}

void ResampleDlg::RecomputeSize()
{
	if (m_bConstraint)  // powers of 2 + 1
		m_iSizeX = m_iSizeY = (1 << m_power) + 1;

	m_fSpacingX = m_fAreaX / (m_iSizeX - 1);
	m_fSpacingY = m_fAreaY / (m_iSizeY - 1);
}

// WDR: handler implementations for ResampleDlg

void ResampleDlg::OnDotDotDot( wxCommandEvent &event )
{
	wxString filter;
	filter += FSTRING_BT;
	filter += FSTRING_BTGZ;

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Save Elevation"), _T(""), _T(""), filter, wxSAVE);
	saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString2 name = saveFile.GetPath();

	// work around incorrect extension(s) that wxFileDialog added
	bool bPreferGZip = (saveFile.GetFilterIndex() == 1);

	if (!name.Right(3).CmpNoCase(_T(".gz")))
		name = name.Left(name.Len()-3);
	if (!name.Right(3).CmpNoCase(_T(".bt")))
		name = name.Left(name.Len()-3);

	if (bPreferGZip)
		name += _T(".bt.gz");
	else
		name += _T(".bt");

	m_strToFile = name;

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void ResampleDlg::OnRadioOutput( wxCommandEvent &event )
{
	TransferDataFromWindow();
	EnableBasedOnConstraint();
}

void ResampleDlg::OnShorts( wxCommandEvent &event )
{
	GetVUnits()->Enable(true);
}

void ResampleDlg::OnFloats( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_fVUnits = 1.0f;
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	EnableBasedOnConstraint();
}

void ResampleDlg::OnSpacingXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_iSizeX = (int) (m_fAreaX / m_fSpacingX)+1;
	m_iSizeY = (int) (m_fAreaY / m_fSpacingY)+1;

	m_bSetting = true;
	spacing1->Enable(false);
	spacing2->Enable(false);
	TransferDataToWindow();
	spacing1->Enable(true);
	spacing2->Enable(true);
	m_bSetting = false;
}

void ResampleDlg::OnSizeXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void ResampleDlg::OnConstrain( wxCommandEvent &event )
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

void ResampleDlg::EnableBasedOnConstraint()
{
	GetSmaller()->Enable(m_bConstraint);
	GetBigger()->Enable(m_bConstraint);
	GetSizeX()->SetEditable(!m_bConstraint);
	GetSizeY()->SetEditable(!m_bConstraint);
	GetSpacingX()->SetEditable(!m_bConstraint);
	GetSpacingY()->SetEditable(!m_bConstraint);
	GetVUnits()->Enable(!m_bFloats);

	GetDotDotDot()->Enable(m_bToFile);
	GetTextToFile()->Enable(m_bToFile);
}

void ResampleDlg::OnBigger( wxCommandEvent &event )
{
	m_power++;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void ResampleDlg::OnSmaller( wxCommandEvent &event )
{
	m_power--;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

