//
// Name: LayerDlg.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "LayerDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "LayerDlg.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/Globe.h"
#include "vtui/wxString2.h"
#include "EnviroGUI.h"	// for GetCurrentTerrain

// WDR: class implementations

//----------------------------------------------------------------------------
// LayerDlg
//----------------------------------------------------------------------------

// WDR: event table for LayerDlg

BEGIN_EVENT_TABLE(LayerDlg,wxDialog)
	EVT_INIT_DIALOG (LayerDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_LAYER_TREE, LayerDlg::OnSelChanged )
END_EVENT_TABLE()

LayerDlg::LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	m_pTree = NULL;

	// WDR: dialog function LayerDialogFunc for LayerDlg
	LayerDialogFunc( this, TRUE ); 
}

void LayerDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	m_pTree = GetTree();

	RefreshTreeContents();

	wxWindow::OnInitDialog(event);
}

void LayerDlg::RefreshTreeContents()
{
	if (!m_pTree)
		return;

	// start with a blank slate
	m_pTree->DeleteAllItems();

	switch (g_App.m_state)
	{
	case AS_Terrain:
		RefreshTreeTerrain();
		break;
	case AS_Orbit:
		RefreshTreeSpace();
		break;
	}
}

void LayerDlg::RefreshTreeTerrain()
{
	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_T("Layers"));

	unsigned int i, j;
	StructureSet &set = terr->GetStructureSet();
	vtStructureArray3d *sa;
	for (i = 0; i < set.GetSize(); i++)
	{
		wxString2 str;

		sa = set[i];

		str = sa->GetFilename();
		wxTreeItemId hItem = m_pTree->AppendItem(hRoot, str, -1, -1);
		if (sa == terr->GetStructures())
			m_pTree->SetItemBold(hItem, true);

		int bld = 0, fen = 0, inst = 0;
		for (j = 0; j < sa->GetSize(); j++)
		{
			if (sa->GetBuilding(j)) bld++;
			if (sa->GetFence(j)) fen++;
			if (sa->GetInstance(j)) inst++;
		}
		if (bld)
		{
			str.Printf(_T("%d Building%s"), bld, bld != 1 ? "s" : "");
			m_pTree->AppendItem(hItem, str, -1, -1);
		}
		if (fen)
		{
			str.Printf(_T("%d Fence%s"), fen, fen != 1 ? "s" : "");
			m_pTree->AppendItem(hItem, str, -1, -1);
		}
		if (inst)
		{
			str.Printf(_T("%d Instance%s"), inst, inst != 1 ? "s" : "");
			m_pTree->AppendItem(hItem, str, -1, -1);
		}
		m_pTree->Expand(hItem);
	}
	m_pTree->Expand(hRoot);
}

void LayerDlg::RefreshTreeSpace()
{
	IcoGlobe *globe = g_App.GetGlobe();
	if (!globe)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_T("Layers"));

	vtFeaturesSet &feats = globe->GetFeaturesSet();
	for (unsigned int i = 0; i < feats.GetSize(); i++)
	{
		wxString2 str;
		vtFeatures *feat = feats[i];

		str = feat->GetFilename();
		wxTreeItemId hItem = m_pTree->AppendItem(hRoot, str, -1, -1);

		int type = feat->GetEntityType();
		int num = feat->NumEntities();
		str.Printf(_T("%d "), num);
		if (type == SHPT_POINT)
			str += _T("Point");
		if (type == SHPT_ARC)
			str += _T("Arc");
		if (type == SHPT_POLYGON)
			str += _T("Polygon");
		str += _T(" Feature");
		if (num != 1)
			str += _T("s");
		m_pTree->AppendItem(hItem, str, -1, -1);
		m_pTree->Expand(hItem);
	}
	m_pTree->Expand(hRoot);
}

// WDR: handler implementations for LayerDlg

void LayerDlg::OnSelChanged( wxTreeEvent &event )
{
}

