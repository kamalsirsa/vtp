//
// Name: OptionsDlg.cpp
//
// Copyright (c) 2004-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "OptionsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	  #include "wx/wx.h"
#endif

#include "vtui/Helper.h"	// for AddFilenamesToComboBox
#include "OptionsDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// OptionsDlg
//----------------------------------------------------------------------------

// WDR: event table for OptionsDlg

BEGIN_EVENT_TABLE(OptionsDlg,AutoDialog)
	EVT_INIT_DIALOG (OptionsDlg::OnInitDialog)
	EVT_CHECKBOX( ID_FULLSCREEN, OptionsDlg::OnCheck )
	EVT_CHECKBOX( ID_STEREO, OptionsDlg::OnCheck )
	EVT_CHECKBOX( ID_DIRECT_PICKING, OptionsDlg::OnCheck )
	EVT_BUTTON( wxID_OK, OptionsDlg::OnOK )
END_EVENT_TABLE()

OptionsDlg::OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function OptionsDialogFunc for OptionsDlg
	OptionsDialogFunc( this, TRUE );

	AddValidator(ID_FULLSCREEN, &m_bFullscreen);
	AddValidator(ID_STEREO, &m_bStereo);
	AddNumValidator(ID_WINX, &m_WinPos.x);
	AddNumValidator(ID_WINY, &m_WinPos.y);
	AddNumValidator(ID_WIN_XSIZE, &m_WinSize.x);
	AddNumValidator(ID_WIN_YSIZE, &m_WinSize.y);
	AddValidator(ID_SIZE_INSIDE, &m_bLocationInside);

//  AddValidator(ID_HTML_PANE, &m_bHtmlpane);
//  AddValidator(ID_FLOATING, &m_bFloatingToolbar);
	AddValidator(ID_TEXTURE_COMPRESSION, &m_bTextureCompression);
//  AddValidator(ID_SHADOWS, &m_bShadows);
	AddValidator(ID_DISABLE_MIPMAPS, &m_bDisableMipmaps);

	AddValidator(ID_DIRECT_PICKING, &m_bDirectPicking);
	AddNumValidator(ID_SELECTION_CUTOFF, &m_fSelectionCutoff, 2);
	AddNumValidator(ID_SELECTION_RADIUS, &m_fMaxPickableInstanceRadius, 2);

	AddNumValidator(ID_PLANTSIZE, &m_fPlantScale, 2);
	AddValidator(ID_ONLY_AVAILABLE_SPECIES, &m_bOnlyAvailableSpecies);

	AddValidator(ID_CHOICE_CONTENT, &m_iContentFile);
	AddValidator(ID_CHOICE_CONTENT, &m_strContentFile);
}


void OptionsDlg::GetOptionsFrom(EnviroOptions &opt)
{
	m_bFullscreen = opt.m_bFullscreen;
	m_bStereo = opt.m_bStereo;
	m_iStereoMode = opt.m_iStereoMode;
	m_WinPos = opt.m_WinPos;
	m_WinSize = opt.m_WinSize;
	m_bLocationInside = opt.m_bLocationInside;

//  m_bHtmlpane = opt.m_bHtmlpane;
//  m_bFloatingToolbar = opt.m_bFloatingToolbar;
	m_bTextureCompression = opt.m_bTextureCompression;
	m_bDisableMipmaps = opt.m_bDisableModelMipmaps;

	m_bDirectPicking = opt.m_bDirectPicking;
	m_fSelectionCutoff = opt.m_fSelectionCutoff;
	m_fMaxPickableInstanceRadius = opt.m_fMaxPickableInstanceRadius;

	m_fPlantScale = opt.m_fPlantScale;
//  m_bShadows = opt.m_bShadows;
	m_bOnlyAvailableSpecies = opt.m_bOnlyAvailableSpecies;

	m_strContentFile = opt.m_strContentFile;
}

void OptionsDlg::PutOptionsTo(EnviroOptions &opt)
{
	opt.m_bFullscreen = m_bFullscreen;
	opt.m_bStereo = m_bStereo;
	opt.m_iStereoMode = m_iStereoMode;
	opt.m_WinPos = m_WinPos;
	opt.m_WinSize = m_WinSize;
	opt.m_bLocationInside = m_bLocationInside;

	//  opt.m_bHtmlpane = m_bHtmlpane;
//  opt.m_bFloatingToolbar = m_bFloatingToolbar;
	opt.m_bTextureCompression = m_bTextureCompression;
	opt.m_bDisableModelMipmaps = m_bDisableMipmaps;

	opt.m_bDirectPicking = m_bDirectPicking;
	opt.m_fSelectionCutoff = m_fSelectionCutoff;
	opt.m_fMaxPickableInstanceRadius = m_fMaxPickableInstanceRadius;

	opt.m_fPlantScale = m_fPlantScale;
//  opt.m_bShadows = m_bShadows;
	opt.m_bOnlyAvailableSpecies = m_bOnlyAvailableSpecies;

	opt.m_strContentFile = m_strContentFile.mb_str();
}

void OptionsDlg::UpdateEnabling()
{
	GetStereo1()->Enable(m_bStereo);
	GetStereo2()->Enable(m_bStereo);

	GetWinx()->Enable(!m_bFullscreen);
	GetWiny()->Enable(!m_bFullscreen);
	GetWinXsize()->Enable(!m_bFullscreen);
	GetWinYsize()->Enable(!m_bFullscreen);
	GetSizeInside()->Enable(!m_bFullscreen);

	GetSelectionCutoff()->Enable(!m_bDirectPicking);
	GetSelectionRadius()->Enable(!m_bDirectPicking);
}

// WDR: handler implementations for OptionsDlg

void OptionsDlg::OnOK(wxCommandEvent &event)
{
	if (GetStereo1()->GetValue()) m_iStereoMode = 0;
	if (GetStereo2()->GetValue()) m_iStereoMode = 1;
	event.Skip();
}

void OptionsDlg::OnCheck( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void OptionsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Populate Content files choices
	vtStringArray &paths = g_Options.m_DataPaths;
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		vtString path = paths[i];
		AddFilenamesToChoice(GetContent(), path, "*.vtco");
	}
	m_iContentFile = GetContent()->FindString(m_strContentFile);
	if (m_iContentFile != -1)
		GetContent()->SetSelection(m_iContentFile);

	UpdateEnabling();

	if (m_iStereoMode == 0) GetStereo1()->SetValue(true);
	if (m_iStereoMode == 1) GetStereo2()->SetValue(true);

	event.Skip();
}

