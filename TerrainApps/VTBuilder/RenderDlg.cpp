//
// Name: RenderDlg.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RenderDlg.h"
#include "Layer.h"
#include "vtdata/FilePath.h"
#include "vtui/Helper.h"
#include "vtui/ColorMapDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// RenderDlg
//----------------------------------------------------------------------------

// WDR: event table for RenderDlg

BEGIN_EVENT_TABLE(RenderDlg, AutoDialog)
	EVT_INIT_DIALOG (RenderDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, RenderDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, RenderDlg::OnRadio )
	EVT_RADIOBUTTON( ID_GEOTIFF, RenderDlg::OnRadio )
	EVT_RADIOBUTTON( ID_JPEG, RenderDlg::OnRadio )
	EVT_BUTTON( ID_DOTDOTDOT, RenderDlg::OnDotdotdot )
	EVT_CHECKBOX( ID_CONSTRAIN, RenderDlg::OnConstrain )
	EVT_CHECKBOX( ID_TILING, RenderDlg::OnConstrain )
	EVT_CHECKBOX( ID_CONSTRAIN, RenderDlg::OnConstrain )
	EVT_BUTTON( ID_SMALLER, RenderDlg::OnSmaller )
	EVT_BUTTON( ID_BIGGER, RenderDlg::OnBigger )
	EVT_BUTTON( ID_EDIT_COLORS, RenderDlg::OnEditColors )
END_EVENT_TABLE()

RenderDlg::RenderDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function RenderBitmapDialogFunc for RenderDlg
	RenderBitmapDialogFunc( this, TRUE ); 

	m_power = 8;
	m_bConstraint = false;
	m_bTiling = false;
	m_bToFile = false;

	m_iSizeX = 256;
	m_iSizeY = 256;

	// sampling
	AddValidator(ID_RADIO_TO_FILE, &m_bToFile);
	AddValidator(ID_TEXT_TO_FILE, &m_strToFile);
	AddValidator(ID_JPEG, &m_bJPEG);

	AddValidator(ID_CHOICE_COLORS, &m_strColorMap);
	AddValidator(ID_SHADING, &m_bShading);

	AddNumValidator(ID_SIZEX, &m_iSizeX);
	AddNumValidator(ID_SIZEY, &m_iSizeY);
	AddValidator(ID_CONSTRAIN, &m_bConstraint);
	AddValidator(ID_TILING, &m_bTiling);

	UpdateEnabling();
}

void RenderDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateColorMapChoice();

	m_bSetting = true;
	wxDialog::OnInitDialog(event);
	m_bSetting = false;
}

void RenderDlg::RecomputeSize()
{
	if (m_bConstraint)  // powers of 2 + 1
		m_iSizeX = m_iSizeY = (1 << m_power);

	if (m_bConstraint && m_bTiling)
	{
		m_iSizeX -= 3;
		m_iSizeY -= 3;
	}
}

void RenderDlg::UpdateEnabling()
{
	GetTextToFile()->Enable(m_bToFile);
	GetJpeg()->Enable(m_bToFile);
	GetGeotiff()->Enable(m_bToFile);
	GetDotdotdot()->Enable(m_bToFile);

	GetSmaller()->Enable(m_bConstraint);
	GetBigger()->Enable(m_bConstraint);
	GetTiling()->Enable(m_bConstraint);
	GetSizeX()->SetEditable(!m_bConstraint);
	GetSizeY()->SetEditable(!m_bConstraint);
}

void RenderDlg::UpdateColorMapChoice()
{
	GetColorMap()->Clear();
	for (unsigned int i = 0; i < m_datapaths.size(); i++)
	{
		// fill the "colormap" control with available colormap files
		AddFilenamesToChoice(GetColorMap(), m_datapaths[i] + "GeoTypical", "*.cmt");
		int sel = GetColorMap()->FindString(m_strColorMap);
		if (sel != -1)
			GetColorMap()->SetSelection(sel);
	}
}

// WDR: handler implementations for RenderDlg

void RenderDlg::OnEditColors( wxCommandEvent &event )
{
	TransferDataFromWindow();

	ColorMapDlg dlg(this, -1, _("ColorMap"));

	// Look on data paths, to give a complete path to the dialog
	if (m_strColorMap != _T(""))
	{
		vtString name = "GeoTypical/";
		name += m_strColorMap.mb_str();
		name = FindFileOnPaths(m_datapaths, name);
		if (name == "")
		{
			wxMessageBox(_("Couldn't locate file."));
			return;
		}
		dlg.SetFile(name);
	}
	dlg.ShowModal();

	// They may have added or removed some color map files on the data path
	UpdateColorMapChoice();
}


void RenderDlg::OnDotdotdot( wxCommandEvent &event )
{
	wxString filter;
	filter += FSTRING_JPEG;
	filter += FSTRING_TIF;

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Specify image file"), _T(""), _T(""), filter, wxSAVE);
	saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString2 name = saveFile.GetPath();

	// work around incorrect extension(s) that wxFileDialog added
	if (!name.Right(4).CmpNoCase(_T(".jpg")))
		name = name.Left(name.Len()-4);
	if (!name.Right(4).CmpNoCase(_T(".tif")))
		name = name.Left(name.Len()-4);

	if (m_bJPEG)
		name += _T(".jpeg");
	else
		name += _T(".tif");

	m_strToFile = name;

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void RenderDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();

	vtString fname = m_strToFile.vt_str();
	RemoveFileExtensions(fname);
	if (fname != "")
	{
		if (m_bJPEG)
			fname += ".jpeg";
		else
			fname += ".tif";
	}
	m_strToFile = fname;

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	UpdateEnabling();
}

void RenderDlg::OnConstrain( wxCommandEvent &event )
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
	UpdateEnabling();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void RenderDlg::OnSmaller( wxCommandEvent &event )
{
	m_power--;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void RenderDlg::OnBigger( wxCommandEvent &event )
{
	m_power++;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}






