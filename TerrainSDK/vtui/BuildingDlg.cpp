//
// Name: BuildingDlg.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
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
#include "HeightDlg.h"
#include "vtui/Helper.h"

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
	EVT_CHAR_HOOK(BuildingDlg::OnCharHook)
	EVT_TEXT_ENTER( ID_FACADE, BuildingDlg::OnFacadeEnter )
	EVT_BUTTON( ID_EDITHEIGHTS, BuildingDlg::OnEditHeights )
	EVT_BUTTON( ID_MODIFY_FACADE, BuildingDlg::OnModifyFacade )
END_EVENT_TABLE()

BuildingDlg::BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_pHeightField = NULL;
	m_bSetting = false;
	m_bEdges = false;
	BuildingDialogFunc( this, TRUE );
}

void BuildingDlg::Setup(vtStructureArray *pSA, vtBuilding *bld, vtHeightField *pHeightField)
{
	m_pSA = pSA;
	m_pBuilding = bld;
	m_pHeightField = pHeightField;
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

void BuildingDlg::OnModifyFacade( wxCommandEvent &event )
{
	wxFileDialog SelectFile(this, _("Choose facade texture"),
							_T(""),
							_T(""),
							_T("Jpeg files (*.jpg)|*.jpg|PNG files(*.png)|*.png|Bitmap files (*.bmp)|*.bmp|All files(*.*)|*.*"),
							wxOPEN);

	if (SelectFile.ShowModal() != wxID_OK)
		return;

	m_strFacade = SelectFile.GetFilename();

	UpdateFacade();

	SetEdgeFacade();

}

void BuildingDlg::OnEditHeights( wxCommandEvent &event )
{
	if (!m_pHeightField)
		return;

	CHeightDialog HeightDialog(this, -1, _("Baseline Editor"));

	HeightDialog.Setup(m_pBuilding, m_pHeightField);

	HeightDialog.ShowModal();
}

void BuildingDlg::OnFacadeEnter( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SetEdgeFacade();
}

void BuildingDlg::OnFeatDoor( wxCommandEvent &event )
{
	vtEdgeFeature f;
	f.m_code = WFC_DOOR;
	f.m_width = -1.0f;
	f.m_vf1 = 0.0f;
	f.m_vf2 = 0.8f;
	m_pEdge->m_Features.push_back(f);
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
	m_pEdge->m_Features.push_back(f);
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
	m_pEdge->m_Features.push_back(f);
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnFeatClear( wxCommandEvent &event )
{
	m_pEdge->m_Features.clear();
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
	if (m_iLevel < (int) m_pBuilding->GetNumLevels() - 1)
	{
		m_pBuilding->SwapLevels(m_iLevel, m_iLevel+1);
		RefreshLevelsBox();
		SetLevel(m_iLevel+1);
		HighlightSelectedLevel();
		Modified();
	}
}

void BuildingDlg::CopyCurrentLevel()
{
	vtLevel *pNewLevel = new vtLevel(*m_pLevel);
	m_pBuilding->InsertLevel(m_iLevel, pNewLevel);
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnLevelCopy( wxCommandEvent &event )
{
	CopyCurrentLevel();
}

void BuildingDlg::DeleteCurrentLevel()
{
	m_pBuilding->DeleteLevel(m_iLevel);
	if (m_iLevel == m_pBuilding->GetNumLevels())
		m_iLevel--;
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnLevelDelete( wxCommandEvent &event )
{
	DeleteCurrentLevel();
}

void BuildingDlg::OnStoryHeight( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_fStoryHeight = m_fStoryHeight;
	m_pBuilding->DetermineLocalFootprints();
	Modified();
}

void BuildingDlg::OnSpinStories( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_iStories = m_iStories;
	m_pBuilding->DetermineLocalFootprints();
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
		AddValidator(ID_FACADE, &m_strFacade);
	}

	m_pLevelListBox = GetLevelCtrl();
	if (m_bEdges)
		m_pEdgeListBox = GetEdgeCtrl();

	RefreshLevelsBox();
//  if (m_bEdges)
//  RefreshEdgesBox();

	SetLevel(m_iLevel);
	HighlightSelectedLevel();
}

void BuildingDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_iLevel = 0;
	m_pLevel = NULL;
	m_pEdge = NULL;
	m_fStoryHeight = 0.0f;
	m_strMaterial = _T("");

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
		str.Printf(_T("%d"), i);
		RoofType rt = pLev->GuessRoofType();
		if (rt == ROOF_FLAT)
			str += _(" (flat roof)");
		else if (rt == ROOF_SHED)
			str += _(" (shed roof)");
		else if (rt == ROOF_GABLE)
			str += _(" (gable roof)");
		else if (rt == ROOF_HIP)
			str += _(" (hip roof)");
		else
		{
			wxString str2;
			str2.Printf(_(" (stories : %d)"), pLev->m_iStories);
			str += str2;
		}
		m_pLevelListBox->Append(str);
	}
}

void BuildingDlg::RefreshEdgesBox()
{
	m_pEdgeListBox->Clear();
	wxString str;
	int i, edges = m_pLevel->NumEdges();
	for (i = 0; i < edges; i++)
	{
		str.Printf(_T("%d"), i);
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
	SetEdgeFacade();
	m_iEdge = iEdge;
	m_pEdge = m_pLevel->GetEdge(iEdge);
	m_iEdgeSlope = m_pEdge->m_iSlope;
	m_strFacade = wxString::FromAscii((const char *) m_pEdge->m_Facade);

	// material
	UpdateMaterialControl();

	// color
	UpdateColorControl();

	// slopes
	UpdateSlopes();

	// features
	UpdateFeatures();

	UpdateFacade();

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
	GetLevelDown()->Enable(m_iLevel < (int) m_pBuilding->GetNumLevels()-1);
	GetLevelDel()->Enable(m_pBuilding->GetNumLevels() > 1);
	GetLevelCopy()->Enable(true);
}

void BuildingDlg::UpdateMaterialControl()
{

	// In the case of a whole level, attempt to show the most
	//  commonly occuring material.
	if (m_bEdges == false)
		m_strMaterial = m_pLevel->GetOverallEdgeMaterial();
	else
	{
		if (m_pEdge->m_pMaterial == NULL)
			m_strMaterial = "Unknown";
		else
			m_strMaterial = *m_pEdge->m_pMaterial;
	}
}

void BuildingDlg::UpdateColorControl()
{
	RGBi color;

	// In the case of a whole level, attempt to show the most
	//  commonly occuring material.
	if (m_bEdges == false)
	{
		int edges = m_pLevel->NumEdges();
		if (edges == 0)
		{
			// badly formed building; don't crash
			m_Color.Set(0, 0, 0);
			wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color);
			m_pColorBitmapControl->SetBitmap(*pBitmap);
			delete pBitmap;
			return;
		}
		// color
		bool uniform = m_pLevel->GetOverallEdgeColor(color);
		if (uniform)
			m_Color.Set(color.r, color.g, color.b);
		else
			m_Color.Set(0, 0, 0);

		// Draw the image with vertical bands corresponding to all the edges
		int xsize = 32, ysize = 18;
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
		wxBitmap *pBitmap = new wxBitmap(pImage);
		m_pColorBitmapControl->SetBitmap(*pBitmap);
		delete pBitmap;
	}
	else
	{
		// Case of a single edge, much simpler.
		color = m_pEdge->m_Color;
		m_Color.Set(color.r, color.g, color.b);
		wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color);
		m_pColorBitmapControl->SetBitmap(*pBitmap);
		delete pBitmap;
	}
}

void BuildingDlg::OnSetEdgeSlopes( wxCommandEvent &event )
{
	wxString choices[5];
	choices[0] = _("Flat (all edges 0°)");
	choices[1] = _("Shed");
	choices[2] = _("Gable");
	choices[3] = _("Hip");
	choices[4] = _("Vertical (all edges 90°)");

	wxSingleChoiceDialog dialog(this, _T("Choice"),
		_("Please indicate edge slopes"), 5, (const wxString *)choices);

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
		slope = wxGetNumberFromUser(_("Sloped edges"), _("Degrees"),
			_("Slope"), slope, 0, 90);
		if (slope == -1)
			return;
		m_pLevel->SetRoofType((RoofType)sel, slope);
	}
	else if (sel == 0)
		m_pLevel->SetRoofType(ROOF_FLAT, 0);
	else if (sel == 4)
	{
		int i, edges = m_pLevel->NumEdges();
		for (i = 0; i < edges; i++)
			m_pLevel->GetEdge(i)->m_iSlope = 90;
	}
	m_fStoryHeight = m_pLevel->m_fStoryHeight;
	UpdateSlopes();
	RefreshLevelsBox();
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::UpdateSlopes()
{
	if (m_bEdges == false)
	{
		wxString2 str;
		m_strEdgeSlopes = _T("");
		int i, edges = m_pLevel->NumEdges();
		for (i = 0; i < edges; i++)
		{
			vtEdge *edge = m_pLevel->GetEdge(i);
			str.Printf(_T(" %d"), edge->m_iSlope);
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
	m_strFeatures = _T("");
	int feats = m_pEdge->NumFeatures();
	for (int i = 0; i < feats; i++)
	{
		vtEdgeFeature &feat = m_pEdge->m_Features[i];
		if (feat.m_code == WFC_WALL)
			m_strFeatures += _T("[W] ");
		else if (feat.m_code == WFC_GAP)
			m_strFeatures += _T("[Gap] ");
		else if (feat.m_code == WFC_POST)
			m_strFeatures += _T("[Post] ");
		else if (feat.m_code == WFC_WINDOW)
			m_strFeatures += _T("[Win] ");
		else if (feat.m_code == WFC_DOOR)
			m_strFeatures += _T("[Door] ");
	}
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::UpdateFacade()
{
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::OnSetMaterial( wxCommandEvent &event )
{
	int i, j;
	int iInitialSelection = -1;
	int iNumberofMaterials = GetGlobalMaterials()->GetSize();

	m_strMaterial = m_pLevel->GetOverallEdgeMaterial();

	wxString *pChoices = new wxString[iNumberofMaterials];

	int iShown = 0;
	for (i = 0; i < iNumberofMaterials; i++)
	{
		vtMaterialDescriptor *mat = GetGlobalMaterials()->GetAt(i);

		// only show surface materials, not typed feature materials
		if (mat->GetType() > 0)
			continue;

		const vtString& MaterialName = mat->GetName();
		wxString2 matname = (const char *) MaterialName;

		// for multiple materials with the same name, only show them once
		bool bFound = false;
		for (j = 0; j < iShown; j++)
		{
			if (pChoices[j] == matname) bFound = true;
		}
		if (bFound)
			continue;

		pChoices[iShown] = matname;
		if (pChoices[iShown] == m_strMaterial)
			iInitialSelection = iShown;
		iShown++;
	}

	wxSingleChoiceDialog dialog(this, _T("Choice"),
		_("Set Building Material for All Edges"), iNumberofMaterials, pChoices);

	if (iInitialSelection != -1)
		dialog.SetSelection(iInitialSelection);

	if (dialog.ShowModal() != wxID_OK)
		return;

	m_strMaterial = pChoices[dialog.GetSelection()];

	delete[] pChoices;

	if (m_bEdges)
		m_pEdge->m_pMaterial = GetGlobalMaterials()->FindName(m_strMaterial.mb_str());
	else
		m_pLevel->SetEdgeMaterial(*GetGlobalMaterials()->FindName(m_strMaterial.mb_str()));

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
		SetEdgeFacade();
		DestroyChildren();
		BuildingDialogFunc( this, TRUE );
		m_pSA->SetEditedEdge(NULL, 0, 0);
	}
	SetupControls();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::SetEdgeFacade()
{
	if (m_bEdges && (NULL != m_pEdge))
	{
		// Store current facade
		TransferDataFromWindow();
		if (0 != m_pEdge->m_Facade.Compare(m_strFacade.mb_str()))
		{
			m_pEdge->m_Facade = m_strFacade.mb_str();
			Modified();
		}
	}
}


#include <vtdata/Vocab.h>

void BuildingDlg::OnCharHook( wxKeyEvent &event )
{
	int foo = event.GetKeyCode();
	if (foo != WXK_F2)
	{
		event.Skip();
		return;
	}

	// Test text input
	wxString2 str = wxGetTextFromUser(_T("Test Message"), _T("Test Caption"), _T(""), this);
	TestParser par;
	par.ParseInput(str.mb_str());

	MatchToken *tok;

	// Pattern sentence: "Select floor/level <number>"
	SentenceMatch sen1;
	sen1.AddLiteral(true, "select");
	sen1.AddLiteral(true, "floor", "level");
	tok = sen1.AddToken(true, NUMBER);

	if (par.Matches(sen1))
	{
		SetLevel( (int) tok->number);
		HighlightSelectedLevel();
	}

	// Pattern sentence: "Select [the] <counter> floor/level"
	SentenceMatch sen2;
	sen2.AddLiteral(true, "select");
	sen2.AddLiteral(false, "the");
	tok = sen2.AddToken(true, COUNTER);
	sen2.AddLiteral(true, "floor", "level");

	if (par.Matches(sen2))
	{
		SetLevel( tok->counter);
		HighlightSelectedLevel();
	}

	// Pattern sentence: "Delete/remove [it]"
	SentenceMatch sen3;
	sen3.AddLiteral(true, "delete", "remove");
	sen3.AddLiteral(false, "it");

	if (par.Matches(sen3))
		DeleteCurrentLevel();

	// Pattern sentence: "Copy/duplicate [it]"
	SentenceMatch sen4;
	sen4.AddLiteral(true, "copy", "duplicate");
	sen4.AddLiteral(false, "it");

	if (par.Matches(sen4))
		CopyCurrentLevel();
}

