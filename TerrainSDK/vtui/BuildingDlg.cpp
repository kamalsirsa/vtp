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
	EVT_LISTBOX( ID_EDGE, BuildingDlg::OnEdge )
	EVT_BUTTON( ID_SET_EDGE_SLOPES, BuildingDlg::OnSetEdgeSlopes )
	EVT_BUTTON( ID_SET_MATERIAL, BuildingDlg::OnSetMaterial )
	EVT_SPINCTRL( ID_STORIES, BuildingDlg::OnSpinStories )
	EVT_TEXT( ID_STORY_HEIGHT, BuildingDlg::OnStoryHeight )
	EVT_BUTTON( ID_LEVEL_COPY, BuildingDlg::OnLevelCopy )
	EVT_BUTTON( ID_LEVEL_UP, BuildingDlg::OnLevelUp )
	EVT_BUTTON( ID_LEVEL_DEL, BuildingDlg::OnLevelDelete )
	EVT_BUTTON( ID_LEVEL_DOWN, BuildingDlg::OnLevelDown )
	EVT_BUTTON( ID_EDGES, BuildingDlg::OnEdges )
	EVT_TEXT( ID_EDGE_SLOPE, BuildingDlg::OnEdgeSlope )
	EVT_BUTTON( ID_FEAT_CLEAR, BuildingDlg::OnFeatClear )
	EVT_BUTTON( ID_FEAT_WALL, BuildingDlg::OnFeatWall )
	EVT_BUTTON( ID_FEAT_WINDOW, BuildingDlg::OnFeatWindow )
	EVT_BUTTON( ID_FEAT_DOOR, BuildingDlg::OnFeatDoor )
	EVT_CLOSE(BuildingDlg::OnCloseWindow)
END_EVENT_TABLE()

BuildingDlg::BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;
	m_bEdges = false;
	BuildingDialogFunc( this, TRUE ); 
}

void BuildingDlg::Setup(vtStructureArray *pSA, vtBuilding *bld)
{
	m_pSA = pSA;
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

		RGBi result(m_Color.Red(), m_Color.Green(), m_Color.Blue());
		if (m_bEdges)
			m_pEdge->m_Color = result;
		else
			m_pLevel->SetEdgeColor(result);

		UpdateColorControl();
	}
	EnableRendering(true);
}

void BuildingDlg::HighlightSelectedLevel()
{
	m_pLevelListBox->SetSelection(m_iLevel);
}

void BuildingDlg::HighlightSelectedEdge()
{
	m_pEdgeListBox->SetSelection(m_iEdge);
}

// WDR: handler implementations for BuildingDlg

void BuildingDlg::OnFeatDoor( wxCommandEvent &event )
{
	vtEdgeFeature f;
	f.m_code = WFC_DOOR;
	f.m_width = -1.0f;
	f.m_vf1 = 0.0f;
	f.m_vf2 = 0.8f;
	m_pEdge->m_Features.Append(f);
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnFeatWindow( wxCommandEvent &event )
{
	vtEdgeFeature f;
	f.m_code = WFC_WINDOW;
	f.m_width = -1.0f;
	f.m_vf1 = 0.3f;
	f.m_vf2 = 0.8f;
	m_pEdge->m_Features.Append(f);
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnFeatWall( wxCommandEvent &event )
{
	vtEdgeFeature f;
	f.m_code = WFC_WALL;
	f.m_width = -1.0f;
	f.m_vf1 = 0;
	f.m_vf2 = 1;
	m_pEdge->m_Features.Append(f);
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnFeatClear( wxCommandEvent &event )
{
	m_pEdge->m_Features.Empty();
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnEdgeSlope( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pEdge->m_iSlope = m_iEdgeSlope;
	Modified();
}

void BuildingDlg::OnLevelUp( wxCommandEvent &event )
{
	if (m_iLevel > 0)
	{
		m_pBuilding->SwapLevels(m_iLevel-1, m_iLevel);
		RefreshLevelsBox();
		SetLevel(m_iLevel-1);
		HighlightSelectedLevel();
		Modified();
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
		Modified();
	}
}

void BuildingDlg::OnLevelCopy( wxCommandEvent &event )
{
	vtLevel *pNewLevel = new vtLevel(*m_pLevel);
	m_pBuilding->InsertLevel(m_iLevel, pNewLevel);
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnLevelDelete( wxCommandEvent &event )
{
	m_pBuilding->DeleteLevel(m_iLevel);
	if (m_iLevel == m_pBuilding->GetNumLevels())
		m_iLevel--;
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnStoryHeight( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_fStoryHeight = m_fStoryHeight;
	Modified();
}

void BuildingDlg::OnSpinStories( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_iStories = m_iStories;
	RefreshLevelsBox();
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnColor1( wxCommandEvent &event )
{
	EditColor();
	Modified();
}

void BuildingDlg::OnOK( wxCommandEvent &event )
{
	// All edits are live, so no need to do anything on OK other than
	// close the window.
	m_pSA->SetEditedEdge(NULL, 0, 0);
	wxDialog::OnOK(event);
}

void BuildingDlg::OnCloseWindow(wxCloseEvent& event)
{
	m_pSA->SetEditedEdge(NULL, 0, 0);
	wxDialog::OnCloseWindow(event);
}

void BuildingDlg::SetupControls()
{
	AddValidator(ID_STORIES, &m_iStories);
	AddNumValidator(ID_STORY_HEIGHT, &m_fStoryHeight);

	if (m_bEdges == false)
	{
		m_pColorBitmapControl = GetColorBitmap1();

		AddValidator(ID_MATERIAL1, &m_strMaterial);
		AddValidator(ID_EDGE_SLOPES, &m_strEdgeSlopes);
	}
	else
	{
		m_pColorBitmapControl = GetColorBitmap2();

		AddValidator(ID_MATERIAL2, &m_strMaterial);
		AddNumValidator(ID_EDGE_SLOPE, &m_iEdgeSlope);
		AddValidator(ID_FEATURES, &m_strFeatures);
	}

	m_pLevelListBox = GetLevelCtrl();
	if (m_bEdges)
		m_pEdgeListBox = GetEdgeCtrl();

	RefreshLevelsBox();
//	if (m_bEdges)
//		RefreshEdgesBox();

	SetLevel(m_iLevel);
	HighlightSelectedLevel();
}

void BuildingDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_iLevel = 0;
	m_pLevel = NULL;
	m_pEdge = NULL;
	m_fStoryHeight = 0.0f;
	m_strMaterial = "";

	SetupControls();

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

void BuildingDlg::RefreshEdgesBox()
{
	m_pEdgeListBox->Clear();
	wxString str;
	int i, edges = m_pLevel->GetNumEdges();
	for (i = 0; i < edges; i++)
	{
		str.Printf("%d", i);
		m_pEdgeListBox->Append(str);
	}
}


/////////////////////////////////////////////////////////////

void BuildingDlg::OnEdge( wxCommandEvent &event )
{
	int sel = m_pEdgeListBox->GetSelection();

	SetEdge(sel);
}

void BuildingDlg::SetEdge(int iEdge)
{
	m_iEdge = iEdge;
	m_pEdge = m_pLevel->GetEdge(iEdge);
	m_iEdgeSlope = m_pEdge->m_iSlope;

	// material
	UpdateMaterialControl();

	// color
	UpdateColorControl();

	// slopes
	UpdateSlopes();

	// features
	UpdateFeatures();

	m_pSA->SetEditedEdge(m_pBuilding, m_iLevel, m_iEdge);
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

	if (m_bEdges)
	{
		RefreshEdgesBox();
		SetEdge(0);
		HighlightSelectedEdge();
	}
	else
	{
		// material
		UpdateMaterialControl();

		// color
		UpdateColorControl();

		// slopes
		UpdateSlopes();
	}

	// enable up/down
	GetLevelUp()->Enable(m_iLevel > 0);
	GetLevelDown()->Enable(m_iLevel < m_pBuilding->GetNumLevels()-1);
	GetLevelDel()->Enable(m_pBuilding->GetNumLevels() > 1);
	GetLevelCopy()->Enable(true);
}

void BuildingDlg::UpdateMaterialControl()
{
	BldMaterial mat;

	// In the case of a whole level, attempt to show the most
	//  commonly occuring material.
	if (m_bEdges == false)
		mat = m_pLevel->GetOverallEdgeMaterial();
	else
		mat = m_pEdge->m_Material;

	m_strMaterial = vtBuilding::GetMaterialString(mat);
}

void BuildingDlg::UpdateColorControl()
{
	RGBi color;

	// In the case of a whole level, attempt to show the most
	//  commonly occuring material.
	if (m_bEdges == false)
	{
		// color
		bool uniform = m_pLevel->GetOverallEdgeColor(color);
		if (uniform)
			m_Color.Set(color.r, color.g, color.b);
		else
			m_Color.Set(0, 0, 0);

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
	else
	{
		// Case of a single edge, much simpler.
		color = m_pEdge->m_Color;
		m_Color.Set(color.r, color.g, color.b);
		wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color);
		m_pColorBitmapControl->SetBitmap(*pBitmap);
	}
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
	Modified();
}

void BuildingDlg::UpdateSlopes()
{
	if (m_bEdges == false)
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
	}
	else
	{
		// nothing special, m_iEdgeSlope is passed with Transfer
	}
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::UpdateFeatures()
{
	m_strFeatures = "";
	int feats = m_pEdge->NumFeatures();
	for (int i = 0; i < feats; i++)
	{
		vtEdgeFeature &feat = m_pEdge->m_Features[i];
		if (feat.m_code == WFC_WALL)
			m_strFeatures += "[W] ";
		else if (feat.m_code == WFC_GAP)
			m_strFeatures += "[Gap] ";
		else if (feat.m_code == WFC_POST)
			m_strFeatures += "[Post] ";
		else if (feat.m_code == WFC_WINDOW)
			m_strFeatures += "[Win] ";
		else if (feat.m_code == WFC_DOOR)
			m_strFeatures += "[Door] ";
	}
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::OnSetMaterial( wxCommandEvent &event )
{
	int i;
	BldMaterial bm;
	wxString choices[9];

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
	if (m_bEdges)
		m_pEdge->m_Material = bm;
	else
		m_pLevel->SetEdgeMaterial(bm);

	m_strMaterial = vtBuilding::GetMaterialString(bm);
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	Modified();
}


void BuildingDlg::OnEdges( wxCommandEvent &event )
{
	m_bEdges = !m_bEdges;
	if (m_bEdges)
	{
		DestroyChildren();
		BuildingEdgesDialogFunc( this, TRUE ); 
	}
	else
	{
		DestroyChildren();
		BuildingDialogFunc( this, TRUE ); 
		m_pSA->SetEditedEdge(NULL, 0, 0);
	}
	SetupControls();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

