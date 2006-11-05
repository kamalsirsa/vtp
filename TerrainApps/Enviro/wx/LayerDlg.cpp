//
// Name: LayerDlg.cpp
//
// Copyright (c) 2003-2006 Virtual Terrain Project
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
#include "vtdata/vtLog.h"
#include "EnviroGUI.h"  // for GetCurrentTerrain
#include "canvas.h"		// for EnableContinuousRendering

#include "LayerDlg.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "building.xpm"
#  include "road.xpm"
#  include "veg1.xpm"
#  include "raw.xpm"
#  include "fence.xpm"
#  include "instance.xpm"
#  include "icon8.xpm"
#endif

#define ICON_BUILDING	0
#define ICON_ROAD		1
#define ICON_VEG1		2
#define ICON_RAW		3
#define ICON_FENCE		4
#define ICON_INSTANCE	5
#define ICON_TOP		6

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
	EVT_BUTTON( ID_LAYER_SAVE, LayerDlg::OnLayerSave )
	EVT_BUTTON( ID_LAYER_SAVE_AS, LayerDlg::OnLayerSaveAs )
	EVT_BUTTON( ID_LAYER_CREATE, LayerDlg::OnLayerCreate )
	EVT_BUTTON( ID_LAYER_REMOVE, LayerDlg::OnLayerRemove )
END_EVENT_TABLE()

LayerDlg::LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function LayerDialogFunc for LayerDlg
	LayerDialogFunc( this, TRUE );

	m_pTree = GetTree();
	m_bShowAll = false;
	m_imageListNormal = NULL;

	CreateImageList(16);
}

LayerDlg::~LayerDlg()
{
	delete m_imageListNormal;
}

void LayerDlg::CreateImageList(int size)
{
	delete m_imageListNormal;

	if ( size == -1 )
	{
		m_imageListNormal = NULL;
		return;
	}
	// Make an image list containing small icons
	m_imageListNormal = new wxImageList(size, size, TRUE);

	wxIcon icons[7];
	icons[0] = wxICON(building);
	icons[1] = wxICON(road);
	icons[2] = wxICON(veg1);
	icons[3] = wxICON(raw);
	icons[4] = wxICON(fence);
	icons[5] = wxICON(instance);
	icons[6] = wxICON(icon8);

	int sizeOrig = icons[0].GetWidth();
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		if ( size == sizeOrig )
			m_imageListNormal->Add(icons[i]);
		else
			m_imageListNormal->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
	}
	m_pTree->SetImageList(m_imageListNormal);
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
vtNode *LayerDlg::GetNodeFromItem(wxTreeItemId item, bool bContainer)
{
	if (!item.IsOk())
		return NULL;

	LayerItemData *data = (LayerItemData *)m_pTree->GetItemData(item);
	if (!data)
		return NULL;
	if (data->m_type == LT_ABSTRACT)
		return data->m_alay->pContainer;
	if (data->m_item == -1)
		return NULL;

	vtStructure3d *str3d = data->m_slay->GetStructure3d(data->m_item);
	vtStructure *str = data->m_slay->GetAt(data->m_item);
	vtStructureType typ = str->GetType();

	if (bContainer && typ != ST_LINEAR)
		return str3d->GetContainer();
	else
		// always get contained geometry for linears; they have no container
		return str3d->GetContained();
}

vtStructureLayer *LayerDlg::GetStructureLayerFromItem(wxTreeItemId item)
{
	LayerItemData *data = GetLayerDataFromItem(item);
	if (!data)
		return NULL;
	if (data->m_item == -1)
		return data->m_slay;
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
	default:
		break;
	}
}

// Helper
wxString MakeVegLayerString(vtPlantInstanceArray3d &pia)
{
	wxString str(pia.GetFilename(), wxConvUTF8);
	wxString str2;
	str2.Printf(_T(" (Plants: %d)"), pia.GetNumEntities());
	str += str2;
	return str;
}

void LayerDlg::RefreshTreeTerrain()
{
	g_pLayerSizer1->Show(g_pLayerSizer2, true);
	g_pLayerSizer1->Layout();

	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	m_root = m_pTree->AddRoot(_("Layers"), ICON_TOP, ICON_TOP);

	wxString str;
	vtString vs;
	unsigned int i, j;
	LayerSet &layers = terr->GetLayers();
	for (i = 0; i < layers.GetSize(); i++)
	{
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(layers[i]);
		if (slay)
		{
			str = wxString(slay->GetFilename(), wxConvUTF8);
			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, ICON_BUILDING, ICON_BUILDING);
			if (slay == terr->GetStructureLayer())
				m_pTree->SetItemBold(hLayer, true);
			m_pTree->SetItemData(hLayer, new LayerItemData(slay, i, -1));

			wxTreeItemId hItem;
			if (m_bShowAll)
			{
				for (j = 0; j < slay->GetSize(); j++)
				{
					if (slay->GetBuilding(j))
						hItem = m_pTree->AppendItem(hLayer, _("Building"), ICON_BUILDING, ICON_BUILDING);
					if (slay->GetFence(j))
						hItem = m_pTree->AppendItem(hLayer, _("Fence"), ICON_FENCE, ICON_FENCE);
					if (vtStructInstance *inst = slay->GetInstance(j))
					{
						vs = inst->GetValueString("filename", true);
						if (vs != "")
						{
							str = _T("File ");
							str += vs.UTF8ToWideString().c_str();
						}
						else
						{
							vs = inst->GetValueString("itemname", true);
							str = _T("Item ");
							str += vs.UTF8ToWideString().c_str();
						}
						hItem = m_pTree->AppendItem(hLayer, str, ICON_INSTANCE, ICON_INSTANCE);
					}
					m_pTree->SetItemData(hItem, new LayerItemData(slay, i, j));
				}
			}
			else
			{
				int bld = 0, fen = 0, inst = 0;
				for (j = 0; j < slay->GetSize(); j++)
				{
					if (slay->GetBuilding(j)) bld++;
					if (slay->GetFence(j)) fen++;
					if (slay->GetInstance(j)) inst++;
				}
				if (bld)
				{
					str.Printf(_("Buildings: %d"), bld);
					hItem = m_pTree->AppendItem(hLayer, str, ICON_BUILDING, ICON_BUILDING);
					m_pTree->SetItemData(hItem, new LayerItemData(slay, i, -1));
				}
				if (fen)
				{
					str.Printf(_("Fences: %d"), fen);
					hItem = m_pTree->AppendItem(hLayer, str, ICON_FENCE, ICON_FENCE);
					m_pTree->SetItemData(hItem, new LayerItemData(slay, i, -1));
				}
				if (inst)
				{
					str.Printf(_("Instances: %d"), inst);
					hItem = m_pTree->AppendItem(hLayer, str, ICON_INSTANCE, ICON_INSTANCE);
					m_pTree->SetItemData(hItem, new LayerItemData(slay, i, -1));
				}
			}
			m_pTree->Expand(hLayer);
		}

		// Now, abstract layers
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(layers[i]);
		if (alay)
		{
			vtFeatureSet *fset = alay->pSet;

			vs = fset->GetFilename();
			str = wxString(vs, wxConvUTF8);

			str += _(" (Type: ");
			str += wxString(OGRGeometryTypeToName(fset->GetGeomType()), wxConvLibc);

			str += _(", Features: ");
			vs.Format("%d", fset->GetNumEntities());
			str += wxString(vs, wxConvLibc);
			str += _T(")");

			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, ICON_RAW, ICON_RAW);
			//if (sa == terr->GetStructureLayer())
			//	m_pTree->SetItemBold(hLayer, true);
			m_pTree->SetItemData(hLayer, new LayerItemData(alay, fset));
		}
	}

	// Vegetation
	if (terr->GetPlantList())
	{
		vtPlantInstanceArray3d &pia = terr->GetPlantInstances();
		wxString str = MakeVegLayerString(pia);

		wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, ICON_VEG1, ICON_VEG1);
		m_pTree->SetItemData(hLayer, new LayerItemData(LT_VEG));
	}

	m_pTree->Expand(m_root);
}

//
// Refresh only the state (text) of the items: don't destroy or create any.
// 
void LayerDlg::UpdateTreeTerrain()
{
	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	wxTreeItemIdValue cookie;
	wxTreeItemId id;
	int count = 0;
	for (id = m_pTree->GetFirstChild(m_root, cookie);
		id.IsOk();
		id = m_pTree->GetNextChild(m_root, cookie))
	{
		LayerItemData *data = GetLayerDataFromItem(id);

		if (data)
		{
			// Hightlight the active structure layer in Bold
			if (data->m_slay && data->m_slay == terr->GetStructureLayer() && data->m_item == -1)
				m_pTree->SetItemBold(id, true);
			else
				m_pTree->SetItemBold(id, false);

			// Refresh the vegetation count
			if (data->m_type == LT_VEG)
			{
				vtPlantInstanceArray3d &pia = terr->GetPlantInstances();
				m_pTree->SetItemText(id, MakeVegLayerString(pia));
			}
		}
		count++;
	}
}

void LayerDlg::RefreshTreeSpace()
{
	g_pLayerSizer1->Show(g_pLayerSizer2, false);
	g_pLayerSizer1->Layout();

	vtIcoGlobe *globe = g_App.GetGlobe();
	if (!globe)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_("Layers"));

	vtFeaturesSet &feats = globe->GetFeaturesSet();
	for (unsigned int i = 0; i < feats.GetSize(); i++)
	{
		vtFeatureSet *feat = feats[i];

		wxString str(feat->GetFilename(), wxConvUTF8);
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

	if (data->m_layer != NULL)
	{
		GetCurrentTerrain()->RemoveLayer(data->m_layer);
		RefreshTreeContents();
	}
}

void LayerDlg::OnLayerCreate( wxCommandEvent &event )
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	wxArrayString choices;
	choices.Add(_("Abstract (Points with labels)"));
	choices.Add(_("Structure"));
	int index = wxGetSingleChoiceIndex(_("Msg"), _("caption"), choices, this);
	if (index == 0)
	{
		if (CreateNewAbstractPointLayer(pTerr))
			RefreshTreeContents();
	}
	else if (index == 1)
	{
		// make a new structure layer
		vtStructureLayer *slay = pTerr->NewStructureLayer();
		slay->SetFilename("Untitled.vtst");
		slay->m_proj = pTerr->GetProjection();
		RefreshTreeContents();
	}
}

//Helper
void SaveAbstractLayer(vtFeatureSet *set, bool bAskFilename)
{
	vtString fname = set->GetFilename();

	if (bAskFilename)
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString default_file(StartOfFilename(fname), wxConvUTF8);
		wxString default_dir(ExtractPath(fname), wxConvUTF8);

		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save Abstract Data"), default_dir,
			default_file, _("GIS Files (*.shp)|*.shp"), wxSAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return;
		}
		wxString str = saveFile.GetPath();
		fname = str.mb_str(wxConvUTF8);
		set->SetFilename(fname);
	}
	set->SaveToSHP(fname);
}

void LayerDlg::OnLayerSave( wxCommandEvent &event )
{
	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	if (data->m_type == LT_STRUCTURE && data->m_slay != NULL)
	{
		GetCurrentTerrain()->SetStructureLayer(data->m_slay);
		RefreshTreeContents();
		g_App.SaveStructures(false);	// don't ask for filename
	}

	if (data->m_type == LT_VEG)
		g_App.SaveVegetation(false);	// don't ask for filename

	if (data->m_type == LT_ABSTRACT)
		SaveAbstractLayer(data->m_fset, false);	// don't ask for filename
}

void LayerDlg::OnLayerSaveAs( wxCommandEvent &event )
{
	LayerItemData *data = GetLayerDataFromItem(m_item);

	if (data->m_type == LT_STRUCTURE)
		g_App.SaveStructures(true);		// ask for filename

	if (data->m_type == LT_VEG)
		g_App.SaveVegetation(true);	// ask for filename

	if (data->m_type == LT_ABSTRACT)
		SaveAbstractLayer(data->m_fset, true);	// ask for filename

	// The filename may have changed
	RefreshTreeContents();
}

void LayerDlg::OnZoomTo( wxCommandEvent &event )
{
	vtNode *pThing = GetNodeFromItem(m_item, true);	// get container
	if (pThing)
	{
		FSphere sphere;
		pThing->GetBoundSphere(sphere, true);   // get global bounds
		vtCamera *pCam = vtGetScene()->GetCamera();

		// Put the camera a bit back from the sphere; sufficiently so that
		//  the whole volume of the bounding sphere is visible.
		float smallest = min(pCam->GetFOV(), pCam->GetVertFOV());
		float alpha = smallest / 2.0f;
		float distance = sphere.radius / tanf(alpha);
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
	if (pThing)
		vtGetScene()->ShadowVisibleNode(pThing, bVis);

	vtStructureLayer *slay = GetStructureLayerFromItem(m_item);
	if (slay)
	{
		for (unsigned int j = 0; j < slay->GetSize(); j++)
		{
			vtStructure3d *str3d = slay->GetStructure3d(j);
			if (str3d)
			{
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
	if (pThing)
	{
		pThing->SetEnabled(bVis);
		return;
	}
	vtStructureLayer *slay = GetStructureLayerFromItem(m_item);
	if (slay)
	{
		slay->SetEnabled(bVis);
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

	LayerItemData *data = GetLayerDataFromItem(m_item);
	VTLOG("OnSelChanged, item %d, data %d\n", m_item.m_pItem, data);
	if (data && data->m_slay != NULL)
	{
		vtStructureLayer *newlay = data->m_slay;
		vtStructureLayer *oldlay = GetCurrentTerrain()->GetStructureLayer();
		if (newlay != oldlay)
		{
			GetCurrentTerrain()->SetStructureLayer(newlay);
			UpdateTreeTerrain();
		}
	}

	UpdateEnabling();
}

void LayerDlg::UpdateEnabling()
{
	vtNode *pThing = GetNodeFromItem(m_item);
	LayerItemData *data = GetLayerDataFromItem(m_item);
	vtStructureLayer *slay = GetStructureLayerFromItem(m_item);

	GetZoomTo()->Enable(pThing != NULL);
	GetVisible()->Enable((pThing != NULL) || (slay != NULL));

	bool bShadows = false;
#if VTLIB_OSG
	vtTerrain *terr = GetCurrentTerrain();
	bShadows = (terr && terr->GetParams().GetValueBool(STR_STRUCT_SHADOWS) &&
		((pThing != NULL) || (slay != NULL)));
#endif
	GetShadow()->Enable(bShadows);

	if (pThing)
		GetVisible()->SetValue(pThing->GetEnabled());
	if (slay)
	{
		if (data) {
			GetVisible()->SetValue(data->last_visible);
			GetShadow()->SetValue(data->shadow_last_visible);
		}
	}

	bool bRemovable = false, bSaveable = false;
	if (data != NULL)
	{
		// We can save a structure layer if it is selected
		if (data->m_type == LT_STRUCTURE && slay)
			bRemovable = bSaveable = true;
		
		// We can save always save or remove an abstract layer
		if (data->m_type == LT_ABSTRACT)
			bRemovable = bSaveable = true;

		// We can save (but not remove) a vegetation layer
		if (data->m_type == LT_VEG)
			bSaveable = true;
	}

	GetLayerRemove()->Enable(bRemovable);
	GetLayerSave()->Enable(bSaveable);
	GetLayerSaveAs()->Enable(bSaveable);
}

