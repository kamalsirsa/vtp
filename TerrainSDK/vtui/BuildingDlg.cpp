//
// Name:		BuildingDlg.cpp
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "BuildingDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include <wx/colordlg.h>
#include "BuildingDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// BuildingDlg
//----------------------------------------------------------------------------

// WDR: event table for BuildingDlg

BEGIN_EVENT_TABLE(BuildingDlg, AutoDialog)
	EVT_BUTTON( wxID_OK, BuildingDlg::OnOK )
	EVT_BUTTON( ID_SET_COLOR, BuildingDlg::OnColor1 )
	EVT_LISTBOX( ID_LEVEL, BuildingDlg::OnLevel )
	EVT_BUTTON( ID_SET_EDGE_SLOPES, BuildingDlg::OnSetEdgeSlopes )
	EVT_BUTTON( ID_SET_MATERIAL, BuildingDlg::OnSetMaterial )
	EVT_SPINCTRL( ID_STORIES, BuildingDlg::OnSpinStories )
	EVT_TEXT( ID_STORY_HEIGHT, BuildingDlg::OnStoryHeight )
	EVT_BUTTON( ID_LEVEL_COPY, BuildingDlg::OnLevelCopy )
	EVT_BUTTON( ID_LEVEL_UP, BuildingDlg::OnLevelUp )
	EVT_BUTTON( ID_LEVEL_DEL, BuildingDlg::OnLevelDelete )
	EVT_BUTTON( ID_LEVEL_DOWN, BuildingDlg::OnLevelDown )
END_EVENT_TABLE()

BuildingDlg::BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;
	BuildingDialogFunc( this, TRUE ); 
}

void BuildingDlg::Setup(vtBuilding *bld)
{
	m_pBuilding = bld;
}

wxBitmap *MakeColorBitmap(int xsize, int ysize, wxColour color)
{
	wxImage pImage(xsize, ysize);
	int i, j;
	for (i = 0; i < xsize; i++)
		for (j = 0; j < ysize; j++)
		{
			pImage.SetRGB(i, j, color.Red(), color.Green(), color.Blue());
		}

	wxBitmap *pBitmap = new wxBitmap(pImage.ConvertToBitmap());
	return pBitmap;
}

void BuildingDlg::EditColor()
{
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(m_Color);

	EnableRendering(false);
	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxColourData data2 = dlg.GetColourData();
		m_Color = data2.GetColour();

		m_pLevel->SetEdgeColor(RGBi(m_Color.Red(), m_Color.Green(), m_Color.Blue()));

		UpdateColorControl();
	}
	EnableRendering(true);
}

void BuildingDlg::HighlightSelectedLevel()
{
	m_pLevelListBox->SetSelection(m_iLevel);
}

// WDR: handler implementations for BuildingDlg

void BuildingDlg::OnLevelUp( wxCommandEvent &event )
{
	if (m_iLevel > 0)
	{
		m_pBuilding->SwapLevels(m_iLevel-1, m_iLevel);
		RefreshLevelsBox();
		SetLevel(m_iLevel-1);
		HighlightSelectedLevel();
	}
}

void BuildingDlg::OnLevelDown( wxCommandEvent &event )
{
	if (m_iLevel < m_pBuilding->GetNumLevels() - 1)
	{
		m_pBuilding->SwapLevels(m_iLevel, m_iLevel+1);
		RefreshLevelsBox();
		SetLevel(m_iLevel+1);
		HighlightSelectedLevel();
	}
}

void BuildingDlg::OnLevelCopy( wxCommandEvent &event )
{
	vtLevel *pNewLevel = new vtLevel(*m_pLevel);
	m_pBuilding->InsertLevel(m_iLevel, pNewLevel);
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
}

void BuildingDlg::OnLevelDelete( wxCommandEvent &event )
{
	m_pBuilding->DeleteLevel(m_iLevel);
	if (m_iLevel == m_pBuilding->GetNumLevels())
		m_iLevel--;
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
}

void BuildingDlg::OnStoryHeight( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_fStoryHeight = m_fStoryHeight;
}

void BuildingDlg::OnSpinStories( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_iStories = m_iStories;
	RefreshLevelsBox();
	HighlightSelectedLevel();
}

void BuildingDlg::OnColor1( wxCommandEvent &event )
{
	EditColor();
}

void BuildingDlg::OnOK( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxDialog::OnOK(event);
}

void BuildingDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_iLevel = 0;
	m_pLevel = NULL;
	m_fStoryHeight = 0.0f;
	m_strMaterial = "";

	m_pColorBitmapControl = GetColorBitmap();
	m_pLevelListBox = GetLevelCtrl();

	RefreshLevelsBox();

	AddValidator(ID_STORIES, &m_iStories);
	AddNumValidator(ID_STORY_HEIGHT, &m_fStoryHeight);
	AddValidator(ID_MATERIAL, &m_strMaterial);
	AddValidator(ID_EDGE_SLOPES, &m_strEdgeSlopes);

	m_pLevelListBox->SetSelection(0);
	SetLevel(0);

	m_bSetting = true;
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
	m_bSetting = false;
}

void BuildingDlg::RefreshLevelsBox()
{
	m_pLevelListBox->Clear();
	wxString str;
	int i, levels = m_pBuilding->GetNumLevels();
	for (i = 0; i < levels; i++)
	{
		vtLevel *pLev = m_pBuilding->GetLevel(i);
		str.Printf("%d", i);
		RoofType rt = pLev->GuessRoofType();
		if (rt == ROOF_FLAT)
			str += " (flat roof)";
		else if (rt == ROOF_SHED)
			str += " (shed roof)";
		else if (rt == ROOF_GABLE)
			str += " (gable roof)";
		else if (rt == ROOF_HIP)
			str += " (hip roof)";
		else
		{
			wxString str2;
			str2.Printf(" (%d stor%s)", pLev->m_iStories,
				pLev->m_iStories == 1 ? "y" : "ies");
			str += str2;
		}
		m_pLevelListBox->Append(str);
	}
}

void BuildingDlg::OnLevel( wxCommandEvent &event )
{
	int sel = m_pLevelListBox->GetSelection();

	SetLevel(sel);
}

void BuildingDlg::SetLevel(int iLev)
{
	m_iLevel = iLev;
	m_pLevel = m_pBuilding->GetLevel(iLev);

	m_iStories = m_pLevel->m_iStories;
	m_fStoryHeight = m_pLevel->m_fStoryHeight;

	// material
	BldMaterial most = m_pLevel->GetOverallEdgeMaterial();
	m_strMaterial = vtBuilding::GetMaterialString(most);

	// color
	UpdateColorControl();

	// slopes
	UpdateSlopes();

	// enable up/down
	GetLevelUp()->Enable(m_iLevel > 0);
	GetLevelDown()->Enable(m_iLevel < m_pBuilding->GetNumLevels()-1);
	GetLevelDel()->Enable(m_pBuilding->GetNumLevels() > 1);
	GetLevelCopy()->Enable(true);
}

void BuildingDlg::UpdateColorControl()
{
	// color
	RGBi color;
	bool uniform = m_pLevel->GetOverallEdgeColor(color);
	if (uniform)
		m_Color.Set(color.r, color.g, color.b);
	else
		m_Color.Set(0, 0, 0);

//	wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color);
//	m_pColorBitmapControl->SetBitmap(*pBitmap);

	// Draw the image with vertical bands corresponding to all the edges
	int xsize = 32, ysize = 18;
	int edges = m_pLevel->GetNumEdges();
	float factor = (float) edges / (float) xsize * .9999f;
	wxImage pImage(xsize, ysize);
	int i, j;
	for (i = 0; i < xsize; i++)
	{
		vtEdge *pEdge = m_pLevel->GetEdge((int) (i * factor));
		RGBi col = pEdge->m_Color;
		for (j = 0; j < ysize; j++)
		{
			pImage.SetRGB(i, j, col.r, col.g, col.b);
		}
	}

	wxBitmap *pBitmap = new wxBitmap(pImage.ConvertToBitmap());
	m_pColorBitmapControl->SetBitmap(*pBitmap);
}

void BuildingDlg::OnSetEdgeSlopes( wxCommandEvent &event )
{
	wxString choices[5];
	choices[0] = "Flat (all edges 0°)";
	choices[1] = "Shed";
	choices[2] = "Gable";
	choices[3] = "Hip";
	choices[4] = "Vertical (all edges 180°)";

	wxSingleChoiceDialog dialog(this, "Choice",
		"Please indicate edge slopes", 5, (const wxString *)choices);

	dialog.SetSelection(0);

	if (dialog.ShowModal() != wxID_OK)
		return;

	int sel = dialog.GetSelection();
	if (sel == 1 || sel == 2 || sel == 3)
	{
		// need slope
		int slope;
		if (sel == 1) slope = 4;
		if (sel == 2) slope = 15;
		if (sel == 3) slope = 15;
		slope = wxGetNumberFromUser("Sloped edges", "Degrees",
			"Slope", slope, 0, 90);
		if (slope == -1)
			return;
		m_pLevel->SetRoofType((RoofType)sel, slope);
	}
	else if (sel == 0)
		m_pLevel->SetRoofType(ROOF_FLAT, 0);
	else if (sel == 4)
	{
		int i, edges = m_pLevel->GetNumEdges();
		for (i = 0; i < edges; i++)
			m_pLevel->GetEdge(i)->m_iSlope = 90;
	}
	UpdateSlopes();
	RefreshLevelsBox();
	HighlightSelectedLevel();
}

void BuildingDlg::UpdateSlopes()
{
	wxString str;
	m_strEdgeSlopes = "";
	int i, edges = m_pLevel->GetNumEdges();
	for (i = 0; i < edges; i++)
	{
		vtEdge *edge = m_pLevel->GetEdge(i);
		str.Printf(" %d", edge->m_iSlope);
		m_strEdgeSlopes += str;
	}
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::OnSetMaterial( wxCommandEvent &event )
{
	int i;
	BldMaterial bm;
	wxString choices[8];

	for (i = BMAT_PLAIN; i < BMAT_DOOR; i++)
	{
		bm = (BldMaterial) i;
		choices[i-1] = vtBuilding::GetMaterialString(bm);
	}

	wxSingleChoiceDialog dialog(this, "Choice",
		"Set Building Material for All Edges", 8, (const wxString *)choices);

	bm = m_pLevel->GetOverallEdgeMaterial();
	if (bm != BMAT_UNKNOWN)
		dialog.SetSelection((int) (bm - 1));

	if (dialog.ShowModal() != wxID_OK)
		return;

	int sel = dialog.GetSelection();
	bm = (BldMaterial) (sel + 1);
	m_pLevel->SetEdgeMaterial(bm);

	m_strMaterial = vtBuilding::GetMaterialString(bm);
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}


