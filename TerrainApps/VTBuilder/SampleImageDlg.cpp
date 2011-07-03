//
// Name: SampleImageDlg.cpp
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtui/AutoDialog.h"

#include "SampleImageDlg.h"
#include "TileDlg.h"
#include "FileFilters.h"
#include "BuilderView.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SampleImageDlg
//----------------------------------------------------------------------------

// WDR: event table for SampleImageDlg

BEGIN_EVENT_TABLE(SampleImageDlg, SampleImageDlgBase)
	EVT_INIT_DIALOG (SampleImageDlg::OnInitDialog)
	EVT_BUTTON( ID_SMALLER, SampleImageDlg::OnSmaller )
	EVT_BUTTON( ID_BIGGER, SampleImageDlg::OnBigger )
	EVT_CHECKBOX( ID_CONSTRAIN, SampleImageDlg::OnConstrain )
	EVT_TEXT( ID_SIZEX, SampleImageDlg::OnSizeXY )
	EVT_TEXT( ID_SIZEY, SampleImageDlg::OnSizeXY )
	EVT_TEXT( ID_SPACINGX, SampleImageDlg::OnSpacingXY )
	EVT_TEXT( ID_SPACINGY, SampleImageDlg::OnSpacingXY )
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, SampleImageDlg::OnRadioOutput )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, SampleImageDlg::OnRadioOutput )
	EVT_RADIOBUTTON( ID_RADIO_TO_TILES, SampleImageDlg::OnRadioOutput )
	EVT_BUTTON( ID_DOTDOTDOT, SampleImageDlg::OnDotDotDot )
	EVT_BUTTON( ID_TILE_OPTIONS, SampleImageDlg::OnTileOptions )
END_EVENT_TABLE()

SampleImageDlg::SampleImageDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	SampleImageDlgBase( parent, id, title, position, size, style )
{
	m_power = 8;
	m_bConstraint = false;
	m_bSetting = false;

	m_bNewLayer = true;
	m_bToFile = false;
	m_bToTiles = false;

	m_tileopts.cols = 4;
	m_tileopts.rows = 4;
	m_tileopts.lod0size = 256;
	m_tileopts.numlods = 3;

	FormatTilingString();

	// output options
	AddValidator(this, ID_RADIO_CREATE_NEW, &m_bNewLayer);
	AddValidator(this, ID_RADIO_TO_FILE, &m_bToFile);
	AddValidator(this, ID_RADIO_TO_TILES, &m_bToTiles);

	AddValidator(this, ID_TEXT_TO_FILE, &m_strToFile);
	AddValidator(this, ID_TEXT_TILE_INFO, &m_strTileInfo);

	// sampling
	AddNumValidator(this, ID_SPACINGX, &m_fSpacingX);
	AddNumValidator(this, ID_SPACINGY, &m_fSpacingY);
	AddNumValidator(this, ID_SIZEX, &m_iSizeX);
	AddNumValidator(this, ID_SIZEY, &m_iSizeY);
	AddValidator(this, ID_CONSTRAIN, &m_bConstraint);

	// informations
	AddNumValidator(this, ID_AREAX, &m_fAreaX);
	AddNumValidator(this, ID_AREAY, &m_fAreaY);

	AddNumValidator(this, ID_ESTX, &m_fEstX);
	AddNumValidator(this, ID_ESTY, &m_fEstY);

	GetSizer()->SetSizeHints(this);
}

// WDR: handler implementations for SampleImageDlg

void SampleImageDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_fAreaX = m_area.Width();
	m_fAreaY = m_area.Height();

	// initial value: based on estimate spacing
	m_fSpacingX = m_fEstX;
	m_fSpacingY = m_fEstY;

	// don't just truncate, round slightly to avoid precision issues
	m_iSizeX = (int) (m_fAreaX / m_fSpacingX + 0.5);
	m_iSizeY = (int) (m_fAreaY / m_fSpacingY + 0.5);

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	EnableBasedOnConstraint();
}

void SampleImageDlg::RecomputeSize()
{
	if (m_bToTiles)
	{
		m_iSizeX = m_tileopts.cols * m_tileopts.lod0size + 1;
		m_iSizeY = m_tileopts.rows * m_tileopts.lod0size + 1;
	}
	else if (m_bConstraint)  // powers of 2 + 1
		m_iSizeX = m_iSizeY = (1 << m_power);

	m_fSpacingX = m_fAreaX / m_iSizeX;
	m_fSpacingY = m_fAreaY / m_iSizeY;
}

void SampleImageDlg::FormatTilingString()
{
	m_strTileInfo.Printf(_T("%d x %d @ %d"), m_tileopts.cols,
		m_tileopts.rows, m_tileopts.lod0size);
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
	GetSizeX()->GetValidator()->TransferToWindow();
	GetSizeY()->GetValidator()->TransferToWindow();
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
	GetTextToFile()->Enable(m_bToFile);
	GetDotDotDot()->Enable(m_bToFile);

	GetTextTileInfo()->Enable(m_bToTiles);
	GetTileOptions()->Enable(m_bToTiles);

	GetConstrain()->Enable(!m_bToTiles);
	GetSmaller()->Enable(m_bConstraint && !m_bToTiles);
	GetBigger()->Enable(m_bConstraint && !m_bToTiles);

	GetSizeX()->SetEditable(!m_bConstraint && !m_bToTiles);
	GetSizeY()->SetEditable(!m_bConstraint && !m_bToTiles);
	GetSpacingX()->SetEditable(!m_bConstraint && !m_bToTiles);
	GetSpacingY()->SetEditable(!m_bConstraint && !m_bToTiles);
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

void SampleImageDlg::OnRadioOutput( wxCommandEvent &event )
{
	TransferDataFromWindow();
	EnableBasedOnConstraint();
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	if (m_pView)
	{
		if (m_bToTiles)
			m_pView->ShowGridMarks(m_area, m_tileopts.cols, m_tileopts.rows, -1, -1);
		else
			m_pView->HideGridMarks();
	}
}

void SampleImageDlg::OnTileOptions( wxCommandEvent &event )
{
	TileDlg dlg(this, -1, _("Tiling Options"));

	dlg.m_fEstX = m_fEstX;
	dlg.m_fEstY = m_fEstY;
	dlg.SetElevation(false);
	dlg.SetArea(m_area);
	dlg.SetTilingOptions(m_tileopts);
	dlg.SetView(m_pView);

	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetTilingOptions(m_tileopts);
		FormatTilingString();
		RecomputeSize();
		TransferDataToWindow();
	}
}

void SampleImageDlg::OnDotDotDot( wxCommandEvent &event )
{
	wxString filter = _("All Files|*.*");
	AddType(filter, FSTRING_TIF);
	AddType(filter, FSTRING_JPEG);
	AddType(filter, FSTRING_ECW);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Save Imagery"), _T(""), _T(""), filter, wxFD_SAVE);
	saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	m_strToFile = saveFile.GetPath();

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}


