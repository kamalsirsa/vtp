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

#define NTILES 4

// WDR: class implementations

//----------------------------------------------------------------------------
// TextureDlg
//----------------------------------------------------------------------------

// WDR: event table for TextureDlg

BEGIN_EVENT_TABLE(TextureDlg,TextureDlgBase)
	EVT_INIT_DIALOG (TextureDlg::OnInitDialog)

	EVT_CHOICE( ID_CHOICE_TILESIZE, TextureDlg::OnTileSize )
	EVT_CHOICE( ID_TFILE_BASE, TextureDlg::OnTextureFileBase )
	EVT_RADIOBUTTON( ID_SINGLE, TextureDlg::OnRadio )
	EVT_RADIOBUTTON( ID_DERIVED, TextureDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TILED_4BY4, TextureDlg::OnRadio )
	EVT_COMBOBOX( ID_TFILE_SINGLE, TextureDlg::OnComboTFileSingle )
	EVT_BUTTON( ID_EDIT_COLORS, TextureDlg::OnEditColors )
END_EVENT_TABLE()

TextureDlg::TextureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TextureDlgBase( parent, id, title, position, size, style )
{
	m_pSingle = GetSingle();
	m_pDerived = GetDerived();
	m_pTiled = GetTiled4by4();
	m_pColorMap = GetColorMap();
	m_pTextureFileSingle = GetTfileSingle();

	GetTilesize()->Clear();
	GetTilesize()->Append(_T("256"));
	GetTilesize()->Append(_T("512"));
	GetTilesize()->Append(_T("1024"));
	GetTilesize()->Append(_T("2048"));
	GetTilesize()->Append(_T("4096"));
	GetTilesize()->SetSelection(2);
	m_iTilesizeIndex = 2;
	m_iTilesize = 1024;

	AddValidator(this, ID_TFILE_SINGLE, &m_strTextureSingle);
	AddValidator(this, ID_CHOICE_TILESIZE, &m_iTilesizeIndex);
	AddValidator(this, ID_TFILE_BASE, &m_strTextureBase);
	AddValidator(this, ID_TFILENAME, &m_strTexture4x4);
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

	// tile4x4
	m_strTextureBase = wxString(Params.GetValueString(STR_TEXTUREBASE), wxConvUTF8);
	m_iTilesize = Params.GetValueInt(STR_TILESIZE);
	m_iTilesizeIndex = vt_log2(m_iTilesize)-8;
	m_strTexture4x4 = wxString(Params.GetValueString(STR_TEXTURE4BY4), wxConvUTF8);

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

	// tile4x4
	Params.SetValueInt(STR_TILESIZE, m_iTilesize);
	Params.SetValueString(STR_TEXTUREBASE, (const char *) m_strTextureBase.mb_str(wxConvUTF8));
	Params.SetValueString(STR_TEXTURE4BY4, (const char *) m_strTexture4x4.mb_str(wxConvUTF8));

	// derived
	Params.SetValueString(STR_COLOR_MAP, (const char *) m_strColorMap.mb_str(wxConvUTF8));

	VTLOG("   Finished GetParams\n");
}

void TextureDlg::UpdateFilenameBases()
{
	int totalsize = NTILES * (m_iTilesize-1) + 1;

	vtString filter;
	filter.Format("*%d.*", totalsize);
	vtString number;
	number.Format("%d", totalsize);

	GetTfileBase()->Clear();
	for (unsigned int i = 0; i < m_TextureFiles.size(); i++)
	{
		// fill the "single texture filename" control with available bitmap files
		if (m_TextureFiles[i].Matches(filter))
		{
			vtString s = m_TextureFiles[i];
			int offset = s.Find(number);
			if (offset != -1)
				s = s.Left(offset);
			GetTfileBase()->Append(wxString(s, wxConvUTF8));
		}
	}
	if (GetTfileBase()->GetCount() == 0)
		GetTfileBase()->Append(_("<none>"));

	int sel = GetTfileBase()->FindString(m_strTextureBase);
	if (sel != -1)
		GetTfileBase()->SetSelection(sel);
	else
	{
		GetTfileBase()->SetSelection(0);
		m_strTextureBase = GetTfileBase()->GetString(0);
	}
}

void TextureDlg::UpdateTiledTextureFilename()
{
	int totalsize = NTILES * (m_iTilesize-1) + 1;

	vtString filter;
	filter.Format("%s%d.*", (const char *) m_strTextureBase.mb_str(wxConvUTF8), totalsize);

	bool bFound = false;
	m_strTexture4x4 = _("<none>");
	for (unsigned int i = 0; i < m_TextureFiles.size(); i++)
	{
		// fill the "single texture filename" control with available bitmap files
		if (m_TextureFiles[i].Matches(filter))
		{
			m_strTexture4x4 = wxString(m_TextureFiles[i], wxConvUTF8);
			bFound = true;
			break;
		}
	}
	TransferDataToWindow();
}

void TextureDlg::UpdateEnableState()
{
	FindWindow(ID_TFILE_SINGLE)->Enable(m_iTexture == TE_SINGLE);
	FindWindow(ID_CHOICE_COLORS)->Enable(m_iTexture == TE_DERIVED);
	FindWindow(ID_EDIT_COLORS)->Enable(m_iTexture == TE_DERIVED);
	FindWindow(ID_CHOICE_TILESIZE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILE_BASE)->Enable(m_iTexture == TE_TILED);
	FindWindow(ID_TFILENAME)->Enable(m_iTexture == TE_TILED);
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

void TextureDlg::OnTextureFileBase( wxCommandEvent &event )
{
	if (m_bSetting) return;
	TransferDataFromWindow();
	UpdateTiledTextureFilename();
}

void TextureDlg::OnTileSize( wxCommandEvent &event )
{
	if (m_bSetting) return;
	TransferDataFromWindow();
	m_iTilesize = 1 << (m_iTilesizeIndex + 8);
	UpdateFilenameBases();
	UpdateTiledTextureFilename();
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
	UpdateFilenameBases();
	UpdateTiledTextureFilename();

	wxWindow::OnInitDialog(event);

	UpdateEnableState();

	// This dialog implements a limitation that the TParamsDlg doesn't have:
	//  texture mode by be [either TE_SINGLE and TE_DERIVED] OR TE_TILED.
	// Enforce this by disabling radio buttons.
	if (m_iTexture == TE_SINGLE || m_iTexture == TE_DERIVED)
		m_pTiled->Enable(false);
	if (m_iTexture == TE_TILED)
	{
		m_pSingle->Enable(false);
		m_pDerived->Enable(false);
	}

	if (bShowProgress)
		CloseProgressDialog();

	m_bSetting = false;
}

bool TextureDlg::TransferDataToWindow()
{
	m_bSetting = true;

	m_pSingle->SetValue(m_iTexture == TE_SINGLE);
	m_pDerived->SetValue(m_iTexture == TE_DERIVED);
	m_pTiled->SetValue(m_iTexture == TE_TILED);
	bool result = wxDialog::TransferDataToWindow();
	m_bSetting = false;

	return result;
}

bool TextureDlg::TransferDataFromWindow()
{
	if (m_pSingle->GetValue()) m_iTexture = TE_SINGLE;
	if (m_pDerived->GetValue()) m_iTexture = TE_DERIVED;
	if (m_pTiled->GetValue()) m_iTexture = TE_TILED;

	return wxDialog::TransferDataFromWindow();
}

