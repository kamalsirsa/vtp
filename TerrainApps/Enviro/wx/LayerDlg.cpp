//
// Name: LayerDlg.cpp
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "LayerDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/Globe.h"
#include "vtui/wxString2.h"
#include "EnviroGUI.h"  // for GetCurrentTerrain

#include "LayerDlg.h"

/////////////////////////////

// WDR: class implementations

//----------------------------------------------------------------------------
// LayerDlg
//----------------------------------------------------------------------------

// WDR: event table for LayerDlg

BEGIN_EVENT_TABLE(LayerDlg,wxDialog)
	EVT_INIT_DIALOG (LayerDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_LAYER_TREE, LayerDlg::OnSelChanged )
	EVT_CHECKBOX( ID_SHOW_ALL, LayerDlg::OnShowAll )
	EVT_CHECKBOX( ID_LAYER_VISIBLE, LayerDlg::OnVisible )
	EVT_CHECKBOX( ID_SHADOW_VISIBLE, LayerDlg::OnShadowVisible )
	EVT_BUTTON( ID_LAYER_ZOOM_TO, LayerDlg::OnZoomTo )
	EVT_BUTTON( ID_LAYER_ACTIVE, LayerDlg::OnLayerActivate )
	EVT_BUTTON( ID_LAYER_SAVE, LayerDlg::OnLayerSave )
	EVT_BUTTON( ID_LAYER_CREATE, LayerDlg::OnLayerCreate )
	EVT_BUTTON( ID_LAYER_REMOVE, LayerDlg::OnLayerRemove )
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
vtNode *LayerDlg::GetNodeFromItem(wxTreeItemId item)
{
	if (!item.IsOk())
		return NULL;

	LayerItemData *data = (LayerItemData *)m_pTree->GetItemData(item);
	if (!data)
		return NULL;

	if (data->m_item == -1)
		return NULL;
	vtStructure3d *str3d = data->m_sa->GetStructure3d(data->m_item);
	return str3d->GetContained();
}

vtStructureArray3d *LayerDlg::GetStructureArray3dFromItem(wxTreeItemId item)
{
	if (!item.IsOk())
		return NULL;
	LayerItemData *data = (LayerItemData *)m_pTree->GetItemData(item);
	if (!data)
		return NULL;
	if (data->m_item == -1)
		return data->m_sa;
	return NULL;
}

LayerItemData *LayerDlg::GetLayerDataFromItem(wxTreeItemId item)
{
	if (!item.IsOk())
		return NULL;
	LayerItemData *data = (LayerItemData *)m_pTree->GetItemData(item);
	return data;
}

void LayerDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_pTree = GetTree();

	RefreshTreeContents();
	m_item = m_pTree->GetSelection();
	UpdateEnabling();

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
	g_pLayerSizer1->Show(g_pLayerSizer2, true);
	g_pLayerSizer1->Layout();

	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_("Layers"));

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
		m_pTree->SetItemData(hLayer, new LayerItemData(sa, i, -1));

		wxTreeItemId hItem;
		if (m_bShowAll)
		{
			for (j = 0; j < sa->GetSize(); j++)
			{
				if (sa->GetBuilding(j))
					hItem = m_pTree->AppendItem(hLayer, _("Building"), -1, -1);
				if (sa->GetFence(j))
					hItem = m_pTree->AppendItem(hLayer, _("Fence"), -1, -1);
				if (vtStructInstance *inst = sa->GetInstance(j))
				{
					vtString vs = inst->GetValueString("filename", true, true);
					if (vs != "")
					{
						str = "File ";
						str += vs;
					}
					else
					{
						vs = inst->GetValueString("itemname", false, true);
						str = "Item ";
						str += vs;
					}
					hItem = m_pTree->AppendItem(hLayer, str, -1, -1);
				}
				m_pTree->SetItemData(hItem, new LayerItemData(sa, i, j));
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
				str.Printf(_("Buildings: %d"), bld);
				m_pTree->AppendItem(hLayer, str, -1, -1);
			}
			if (fen)
			{
				str.Printf(_("Fences: %d"), fen);
				m_pTree->AppendItem(hLayer, str, -1, -1);
			}
			if (inst)
			{
				str.Printf(_("Instances: %d"), inst);
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

	wxTreeItemId hRoot = m_pTree->AddRoot(_("Layers"));

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

void LayerDlg::OnLayerRemove( wxCommandEvent &event )
{
	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	if (data->m_sa != NULL)
	{
		GetCurrentTerrain()->DeleteStructureSet(data->m_index);
		RefreshTreeContents();
	}
}

void LayerDlg::OnLayerCreate( wxCommandEvent &event )
{
	vtTerrain *pTerr = GetCurrentTerrain();

	vtStructureArray3d *sa = pTerr->NewStructureArray();
	sa->SetFilename("Untitled.vtst");
	sa->m_proj = pTerr->GetProjection();
	RefreshTreeContents();
}

void LayerDlg::OnLayerSave( wxCommandEvent &event )
{
	g_App.SaveStructures();
	RefreshTreeContents();
}

void LayerDlg::OnLayerActivate( wxCommandEvent &event )
{
	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	if (data->m_sa != NULL)
	{
		GetCurrentTerrain()->SetStructureIndex(data->m_index);
		RefreshTreeContents();
	}
}

void LayerDlg::OnZoomTo( wxCommandEvent &event )
{
	vtNodeBase *pThing = GetNodeFromItem(m_item);
	if (pThing)
	{
		FSphere sphere;
		pThing->GetBoundSphere(sphere, true);   // get global bounds
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


void LayerDlg::OnShadowVisible( wxCommandEvent &event)
{
	bool bVis = event.IsChecked();

	vtNode *pThing = GetNodeFromItem(m_item);
	if (pThing) {
		vtGetScene()->ShadowVisibleNode(pThing, bVis);
	}

	vtStructureArray3d *sa = GetStructureArray3dFromItem(m_item);
	if (sa) {
		for (unsigned int j = 0; j < sa->GetSize(); j++) {
			vtStructure3d *str3d = sa->GetStructure3d(j);
			if (str3d) {
				pThing = str3d->GetContained();
				if (pThing) 
					vtGetScene()->ShadowVisibleNode(pThing, bVis);
			}
		}
		LayerItemData *data = GetLayerDataFromItem(m_item);
		if (data)
			data->shadow_last_visible = bVis;
	}
}

void LayerDlg::OnVisible( wxCommandEvent &event )
{
	bool bVis = event.IsChecked();

	vtNode *pThing = GetNodeFromItem(m_item);
	if (pThing) {
		pThing->SetEnabled(bVis);
		return;
	}

	vtStructureArray3d *sa = GetStructureArray3dFromItem(m_item);
	if (sa) {
		for (unsigned int j = 0; j < sa->GetSize(); j++) {
			vtStructure3d *str3d = sa->GetStructure3d(j);
			if (str3d) {
				pThing = str3d->GetContained();
				if (pThing) 
					pThing->SetEnabled(bVis);
			}
		}
		LayerItemData *data = GetLayerDataFromItem(m_item);
		if (data)
			data->last_visible = bVis;
	}
}

void LayerDlg::OnShowAll( wxCommandEvent &event )
{
	m_bShowAll = event.IsChecked();
	RefreshTreeContents();
	m_item = m_pTree->GetSelection();
	UpdateEnabling();
}

void LayerDlg::OnSelChanged( wxTreeEvent &event )
{
	m_item = event.GetItem();
	UpdateEnabling();
}

void LayerDlg::UpdateEnabling()
{
	vtNode *pThing = GetNodeFromItem(m_item);
	vtStructureArray3d *sa = GetStructureArray3dFromItem(m_item);
	LayerItemData *data = GetLayerDataFromItem(m_item);

	GetZoomTo()->Enable(pThing != NULL);
	GetVisible()->Enable((pThing != NULL) || (sa != NULL));
	GetShadow()->Enable((pThing != NULL) || (sa != NULL));

	if (pThing)
		GetVisible()->SetValue(pThing->GetEnabled());
	if (sa)
	{
		if (data) {
			GetVisible()->SetValue(data->last_visible);
			GetShadow()->SetValue(data->shadow_last_visible);
		}
	}

	GetLayerActivate()->Enable(sa != NULL);
	GetLayerRemove()->Enable(sa != NULL);
	GetLayerSave()->Enable(sa != NULL);
}

