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
#include "EnviroGUI.h"  // for GetCurrentTerrain


/////////////////////////////

class LayerItemData : public wxTreeItemData
{
public:
	LayerItemData(vtStructureArray3d *sa, int item)
	{
		m_sa = sa;
		m_item = item;
	}
	vtStructureArray3d *m_sa;
	int m_item;
};


// WDR: class implementations

//----------------------------------------------------------------------------
// LayerDlg
//----------------------------------------------------------------------------

// WDR: event table for LayerDlg

BEGIN_EVENT_TABLE(LayerDlg,wxDialog)
	EVT_INIT_DIALOG (LayerDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_LAYER_TREE, LayerDlg::OnSelChanged )
	EVT_CHECKBOX( ID_SHOW_ALL, LayerDlg::OnShowAll )
	EVT_CHECKBOX( ID_VISIBLE, LayerDlg::OnVisible )
	EVT_BUTTON( ID_ZOOM_TO, LayerDlg::OnZoomTo )
END_EVENT_TABLE()

LayerDlg::LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	m_pTree = NULL;
	m_bShowAll = false;

	// WDR: dialog function LayerDialogFunc for LayerDlg
	LayerDialogFunc( this, TRUE );
}

void LayerDlg::SetShowAll(bool bTrue)
{
	m_bShowAll = bTrue;
	GetShowAll()->SetValue(bTrue);
}

//
// For an item in the tree which corresponds to an actual structure,
//  return the node associated with that structure.
//
vtNodeBase *LayerDlg::GetNodeFromItem(wxTreeItemId item)
{
	if (!item.IsOk())
		return NULL;

	LayerItemData *data = (LayerItemData *)m_pTree->GetItemData(item);
	if (!data)
		return NULL;

	vtStructure3d *str3d = data->m_sa->GetStructure3d(data->m_item);
	return str3d->GetContained();
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
	GetVisible()->Enable(false);
	GetZoomTo()->Enable(false);

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
	g_pLayerSizer1->Show(g_pLayerSizer2, true);
	g_pLayerSizer1->Layout();

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
		wxTreeItemId hLayer = m_pTree->AppendItem(hRoot, str, -1, -1);
		if (sa == terr->GetStructures())
			m_pTree->SetItemBold(hLayer, true);

		wxTreeItemId hItem;
		if (m_bShowAll)
		{
			for (j = 0; j < sa->GetSize(); j++)
			{
				if (sa->GetBuilding(j))
					hItem = m_pTree->AppendItem(hLayer, _T("Building"), -1, -1);
				if (sa->GetFence(j))
					hItem = m_pTree->AppendItem(hLayer, _T("Fence"), -1, -1);
				if (vtStructInstance *inst = sa->GetInstance(j))
				{
					vtString vs = inst->GetValue("filename");
					if (vs != "")
					{
						str = "File ";
						str += vs;
					}
					else
					{
						vs = inst->GetValue("itemname");
						str = "Item ";
						str += vs;
					}
					hItem = m_pTree->AppendItem(hLayer, str, -1, -1);
				}
				m_pTree->SetItemData(hItem, new LayerItemData(sa, j));
			}
		}
		else
		{
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
				m_pTree->AppendItem(hLayer, str, -1, -1);
			}
			if (fen)
			{
				str.Printf(_T("%d Fence%s"), fen, fen != 1 ? "s" : "");
				m_pTree->AppendItem(hLayer, str, -1, -1);
			}
			if (inst)
			{
				str.Printf(_T("%d Instance%s"), inst, inst != 1 ? "s" : "");
				m_pTree->AppendItem(hLayer, str, -1, -1);
			}
		}
		m_pTree->Expand(hLayer);
	}
	m_pTree->Expand(hRoot);
}

void LayerDlg::RefreshTreeSpace()
{
	g_pLayerSizer1->Show(g_pLayerSizer2, false);
	g_pLayerSizer1->Layout();

	IcoGlobe *globe = g_App.GetGlobe();
	if (!globe)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_T("Layers"));

	vtFeaturesSet &feats = globe->GetFeaturesSet();
	for (unsigned int i = 0; i < feats.GetSize(); i++)
	{
		wxString2 str;
		vtFeatureSet *feat = feats[i];

		str = feat->GetFilename();
		wxTreeItemId hItem = m_pTree->AppendItem(hRoot, str, -1, -1);

		OGRwkbGeometryType type = feat->GetGeomType();
		int num = feat->GetNumEntities();
		str.Printf(_T("%d "), num);
		if (type == wkbPoint)
			str += _T("Point");
		if (type == wkbPoint25D)
			str += _T("PointZ");
		if (type == wkbLineString)
			str += _T("Arc");
		if (type == wkbPolygon)
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

void LayerDlg::OnZoomTo( wxCommandEvent &event )
{
	vtNodeBase *pThing = GetNodeFromItem(m_pTree->GetSelection());
	if (pThing)
	{
		FSphere sphere;
		pThing->GetBoundSphere(sphere, true);	// get global bounds
		vtCamera *pCam = vtGetScene()->GetCamera();

		// Put the camera a bit back from the sphere; sufficiently so that
		//  the whole volume of the bounding sphere is visible.
		float alpha = pCam->GetFOV() / 2.0f;
		float distance = sphere.radius / sinf(alpha);
		pCam->Identity();
		pCam->Rotate2(FPoint3(1,0,0), -PID2f/2);	// tilt down a little
		pCam->Translate1(sphere.center);
		pCam->TranslateLocal(FPoint3(0.0f, 0.0f, distance));
	}
}

void LayerDlg::OnVisible( wxCommandEvent &event )
{
	bool bVis = event.IsChecked();

	vtNodeBase *pThing = GetNodeFromItem(m_pTree->GetSelection());
	if (pThing)
		pThing->SetEnabled(bVis);
}

void LayerDlg::OnShowAll( wxCommandEvent &event )
{
	m_bShowAll = event.IsChecked();
	RefreshTreeContents();
	if (!m_bShowAll)
	{
		GetVisible()->SetValue(false);
		GetVisible()->Enable(false);
		GetZoomTo()->Enable(false);
	}
}

void LayerDlg::OnSelChanged( wxTreeEvent &event )
{
	wxTreeItemId item = event.GetItem();
	vtNodeBase *pThing = GetNodeFromItem(item);

	GetZoomTo()->Enable(pThing != NULL);
	GetVisible()->Enable(pThing != NULL);

	if (pThing)
		GetVisible()->SetValue(pThing->GetEnabled());
}

