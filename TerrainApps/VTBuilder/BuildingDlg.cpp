//
// Name:		BuildingDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
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
END_EVENT_TABLE()

BuildingDlg::BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;
	BuildingDialog2Func( this, TRUE ); 
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

	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxColourData data2 = dlg.GetColourData();
		m_Color = data2.GetColour();

		wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color);
		m_pColorBitmapControl->SetBitmap(*pBitmap);

		m_pLevel->SetEdgeColor(RGBi(m_Color.Red(), m_Color.Green(), m_Color.Blue()));
	}
}

// WDR: handler implementations for BuildingDlg

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

	wxString str;
	int i, levels = m_pBuilding->GetNumLevels();
	for (i = 0; i < levels; i++)
	{
		str.Printf("%d", i);
		m_pLevelListBox->Append(str);
	}

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
	RGBi color;
	bool uniform = m_pLevel->GetOverallEdgeColor(color);
	if (uniform)
		m_Color.Set(color.r, color.g, color.b);
	else
		m_Color.Set(0, 0, 0);

	wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color);
	m_pColorBitmapControl->SetBitmap(*pBitmap);

	// slopes
	UpdateSlopes();
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


