//
// Name: TileDlg.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "TileDlg.h"
#include "Layer.h"	// for FSTRING_INI

// WDR: class implementations

//----------------------------------------------------------------------------
// TileDlg
//----------------------------------------------------------------------------

// WDR: event table for TileDlg

BEGIN_EVENT_TABLE(TileDlg,AutoDialog)
	EVT_BUTTON( ID_DOTDOTDOT, TileDlg::OnDotDotDot )
	EVT_TEXT( ID_COLUMNS, TileDlg::OnSize )
	EVT_TEXT( ID_ROWS, TileDlg::OnSize )
	EVT_CHOICE( ID_CHOICE_LOD0_SIZE, TileDlg::OnLODSize )
END_EVENT_TABLE()

TileDlg::TileDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function TileDialogFunc for TileDlg
	TileDialogFunc( this, TRUE );

	m_fEstX = -1;
	m_fEstY = -1;

	m_bSetting = false;

	AddValidator(ID_TEXT_TO_FOLDER, &m_strToFile);
	AddNumValidator(ID_COLUMNS, &m_iColumns);
	AddNumValidator(ID_ROWS, &m_iRows);
	AddValidator(ID_CHOICE_LOD0_SIZE, &m_iLODChoice);
	AddValidator(ID_SPIN_NUM_LODS, &m_iNumLODs);

	// informations
	AddNumValidator(ID_TOTALX, &m_iTotalX);
	AddNumValidator(ID_TOTALY, &m_iTotalY);

	AddNumValidator(ID_AREAX, &m_fAreaX);
	AddNumValidator(ID_AREAY, &m_fAreaY);

	AddNumValidator(ID_ESTX, &m_fEstX);
	AddNumValidator(ID_ESTY, &m_fEstY);

	AddNumValidator(ID_CURX, &m_fCurX);
	AddNumValidator(ID_CURY, &m_fCurY);
}

void TileDlg::SetTilingOptions(TilingOptions &opt)
{
	m_iColumns = opt.cols;
	m_iRows = opt.rows;
	m_iLOD0Size = opt.lod0size;
	m_iNumLODs = opt.numlods;
	m_strToFile = opt.fname;

	m_iLODChoice = vt_log2(m_iLOD0Size)-6;

	UpdateInfo();
}

void TileDlg::GetTilingOptions(TilingOptions &opt) const
{
	opt.cols = m_iColumns;
	opt.rows = m_iRows;
	opt.lod0size = m_iLOD0Size;
	opt.numlods = m_iNumLODs;
	opt.fname = m_strToFile.mb_str();
}

void TileDlg::SetArea(const DRECT &area)
{
	m_area = area;

	UpdateInfo();
	TransferDataToWindow();
}

void TileDlg::UpdateInfo()
{
	m_iTotalX = m_iLOD0Size * m_iColumns + 1;
	m_iTotalY = m_iLOD0Size * m_iRows + 1;

	m_fAreaX = m_area.Width();
	m_fAreaY = m_area.Height();

	m_fCurX = m_fAreaX / (m_iTotalX - 1);
	m_fCurY = m_fAreaY / (m_iTotalY - 1);

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

// WDR: handler implementations for TileDlg

void TileDlg::OnLODSize( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_iLOD0Size = 1 << (m_iLODChoice + 6);
	UpdateInfo();
}

void TileDlg::OnSize( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateInfo();
}

void TileDlg::OnDotDotDot( wxCommandEvent &event )
{
	// ask the user for a directory
	//wxDirDialog getDir(NULL, _("Write Tiles to Directory"));
	//bool bResult = (getDir.ShowModal() == wxID_OK);
	//if (!bResult)
	//	return;
	//m_strToFile = getDir.GetPath();
	// ask the user for a filename
	wxString filter;
	filter += FSTRING_INI;
	wxFileDialog saveFile(NULL, _T(".Ini file"), _T(""), _T(""), filter, wxSAVE);
	//saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	// update controls
	m_strToFile = saveFile.GetPath();

	TransferDataToWindow();
}

