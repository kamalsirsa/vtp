//
// Name: TextureDlg.cpp
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"
#include "vtdata/FilePath.h"	// for FindFileOnPaths
#include "vtui/Helper.h"		// for AddFilenamesToChoice
#include "vtui/ColorMapDlg.h"

#include "TextureDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TextureDlg
//----------------------------------------------------------------------------

// WDR: event table for TextureDlg

BEGIN_EVENT_TABLE(TextureDlg,TextureDlgBase)
	EVT_INIT_DIALOG (TextureDlg::OnInitDialog)

	EVT_RADIOBUTTON( ID_SINGLE, TextureDlg::OnRadio )
	EVT_RADIOBUTTON( ID_DERIVED, TextureDlg::OnRadio )
	EVT_COMBOBOX( ID_TFILE_SINGLE, TextureDlg::OnComboTFileSingle )
	EVT_BUTTON( ID_EDIT_COLORS, TextureDlg::OnEditColors )
END_EVENT_TABLE()

TextureDlg::TextureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TextureDlgBase( parent, id, title, position, size, style )
{
	m_pSingle = GetSingle();
	m_pDerived = GetDerived();
	m_pColorMap = GetColorMap();
	m_pTextureFileSingle = GetTfileSingle();

	AddValidator(this, ID_TFILE_SINGLE, &m_strTextureSingle);
	AddValidator(this, ID_CHOICE_COLORS, &m_strColorMap);

	GetSizer()->SetSizeHints(this);
}

void TextureDlg::SetParams(const TParams &Params)
{
	VTLOG("TParamsDlg::SetParams\n");
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// texture
	m_iTexture = Params.GetTextureEnum();

	// single
	if (m_iTexture != TE_TILESET)
		m_strTextureSingle = wxString(Params.GetValueString(STR_TEXTUREFILE), wxConvUTF8);

	// derived
	m_strColorMap = wxString(Params.GetValueString(STR_COLOR_MAP), wxConvUTF8);

	VTLOG("   Finished SetParams\n");
}

//
// get the values from the dialog into the supplied paramter structure
//
void TextureDlg::GetParams(TParams &Params)
{
	VTLOG("TextureDlg::GetParams\n");
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// texture
	Params.SetTextureEnum((enum TextureEnum)m_iTexture);

	// single
	if (m_iTexture != TE_TILESET)
		Params.SetValueString(STR_TEXTUREFILE, (const char *) m_strTextureSingle.mb_str(wxConvUTF8));

	// derived
	Params.SetValueString(STR_COLOR_MAP, (const char *) m_strColorMap.mb_str(wxConvUTF8));

	VTLOG("   Finished GetParams\n");
}

void TextureDlg::UpdateEnableState()
{
	FindWindow(ID_TFILE_SINGLE)->Enable(m_iTexture == TE_SINGLE);
	FindWindow(ID_CHOICE_COLORS)->Enable(m_iTexture == TE_DERIVED);
	FindWindow(ID_EDIT_COLORS)->Enable(m_iTexture == TE_DERIVED);
}

void TextureDlg::UpdateColorMapChoice()
{
	m_pColorMap->Clear();
	vtStringArray &paths = vtGetDataPath();
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		// fill the "colormap" control with available colormap files
		AddFilenamesToChoice(m_pColorMap, paths[i] + "GeoTypical", "*.cmt");
		int sel = m_pColorMap->FindString(m_strColorMap);
		if (sel != -1)
			m_pColorMap->SetSelection(sel);
	}
}


// WDR: handler implementations for TextureDlg

void TextureDlg::OnEditColors( wxCommandEvent &event )
{
	TransferDataFromWindow();

	// Look on data paths, to give a complete path to the dialog
	vtString name = "GeoTypical/";
	name += m_strColorMap.mb_str(wxConvUTF8);
	name = FindFileOnPaths(vtGetDataPath(), name);
	if (name == "")
	{
		wxMessageBox(_("Couldn't locate file."));
		return;
	}

	ColorMapDlg dlg(this, -1, _("ColorMap"));
	dlg.SetFile(name);
	dlg.ShowModal();

	// They may have added or removed some color map files on the data path
	UpdateColorMapChoice();
}

void TextureDlg::OnComboTFileSingle( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
}

void TextureDlg::OnRadio( wxCommandEvent &event )
{
	if (m_bSetting || !event.IsChecked())
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

void TextureDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("TextureDlg::OnInitDialog\n");

	bool bShowProgress = (vtGetDataPath().size() > 1);
	if (bShowProgress)
		OpenProgressDialog(_("Looking for files on data paths"), false, this);

	m_bSetting = true;

	unsigned int i;
	int sel;

	vtStringArray &paths = vtGetDataPath();

	for (i = 0; i < paths.size(); i++)
	{
		if (bShowProgress)
			UpdateProgressDialog(i * 100 / paths.size(), wxString(paths[i], wxConvUTF8));

		// Gather all possible texture image filenames
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.bmp");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.jpg");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.jpeg");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.png");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.tif");
	}

	// fill the "single texture filename" control with available image files
	m_pTextureFileSingle->Clear();
	for (i = 0; i < m_TextureFiles.size(); i++)
	{
		wxString str(m_TextureFiles[i], wxConvUTF8);
		m_pTextureFileSingle->Append(str);
	}
	sel = m_pTextureFileSingle->FindString(m_strTextureSingle);
	if (sel != -1)
		m_pTextureFileSingle->SetSelection(sel);

	UpdateColorMapChoice();

	wxWindow::OnInitDialog(event);

	UpdateEnableState();

	if (bShowProgress)
		CloseProgressDialog();

	m_bSetting = false;
}

bool TextureDlg::TransferDataToWindow()
{
	m_bSetting = true;

	m_pSingle->SetValue(m_iTexture == TE_SINGLE);
	m_pDerived->SetValue(m_iTexture == TE_DERIVED);
	bool result = wxDialog::TransferDataToWindow();
	m_bSetting = false;

	return result;
}

bool TextureDlg::TransferDataFromWindow()
{
	if (m_pSingle->GetValue()) m_iTexture = TE_SINGLE;
	if (m_pDerived->GetValue()) m_iTexture = TE_DERIVED;

	return wxDialog::TransferDataFromWindow();
}

