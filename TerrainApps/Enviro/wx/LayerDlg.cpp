//
// Name: LayerDlg.cpp
//
// Copyright (c) 2003-2007 Virtual Terrain Project
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
#include "StyleDlg.h"

#include "LayerDlg.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "building.xpm"
#  include "road.xpm"
#  include "veg1.xpm"
#  include "raw.xpm"
#  include "fence.xpm"
#  include "instance.xpm"
#  include "icon8.xpm"
#  include "image.xpm"
#endif

#define ICON_BUILDING	0
#define ICON_ROAD		1
#define ICON_VEG1		2
#define ICON_RAW		3
#define ICON_FENCE		4
#define ICON_INSTANCE	5
#define ICON_TOP		6
#define ICON_IMAGE		7

/////////////////////////////

#define ID_LAYER_TREE 10500

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
	EVT_CHECKBOX( ID_LAYER_SHADOW, LayerDlg::OnShadowVisible )

	EVT_MENU( ID_LAYER_CREATE, LayerDlg::OnLayerCreate )
	EVT_MENU( ID_LAYER_LOAD, LayerDlg::OnLayerLoad )
	EVT_MENU( ID_LAYER_SAVE, LayerDlg::OnLayerSave )
	EVT_MENU( ID_LAYER_SAVE_AS, LayerDlg::OnLayerSaveAs )
	EVT_MENU( ID_LAYER_DELETE, LayerDlg::OnLayerRemove )
	EVT_MENU( ID_LAYER_ZOOM_TO, LayerDlg::OnZoomTo )

	EVT_MENU( ID_LAYER_VISIBLE, LayerDlg::OnVisible )
	EVT_MENU( ID_LAYER_SHADOW, LayerDlg::OnShadowVisible )
	EVT_MENU( ID_SHOW_ALL, LayerDlg::OnShowAll )

	EVT_UPDATE_UI(ID_LAYER_CREATE,	LayerDlg::OnUpdateCreate)
	EVT_UPDATE_UI(ID_LAYER_VISIBLE,	LayerDlg::OnUpdateVisible)
	EVT_UPDATE_UI(ID_LAYER_SHADOW, LayerDlg::OnUpdateShadow)
	EVT_UPDATE_UI(ID_SHOW_ALL,	LayerDlg::OnUpdateShowAll)

END_EVENT_TABLE()

LayerDlg::LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	long tbstyle = wxTB_FLAT | wxTB_NODIVIDER;
	//tbstyle |= wxTB_HORZ_TEXT;
	m_pToolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
								   tbstyle);
	m_pToolbar->SetToolBitmapSize(wxSize(20, 20));
	LayerToolBarFunc(m_pToolbar);
	m_pToolbar->Realize();

	// tell wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);

	wxAuiPaneInfo api;
	api.Name(_T("toolbar"));
	api.ToolbarPane();
	api.Top();
	api.LeftDockable(false);
	api.RightDockable(false);
	wxSize best = m_pToolbar->GetBestSize();
	api.MinSize(best);
	api.Floatable(false);
	m_mgr.AddPane(m_pToolbar, api);

	m_main = new wxPanel( this, -1, wxDefaultPosition, wxDefaultSize, 0 );
	m_mgr.AddPane(m_main, wxAuiPaneInfo().
				  Name(wxT("pane1")).Caption(wxT("pane1")).
				  CenterPane());
	m_mgr.Update();

    wxBoxSizer *item0 = new wxBoxSizer( wxHORIZONTAL );
	m_pTree = new wxTreeCtrl( m_main, ID_LAYER_TREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxSUNKEN_BORDER );
    item0->Add( m_pTree, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	wxSizer *top = item0;

	m_main->SetSizer( item0 );

	m_bShowAll = false;
	m_imageListNormal = NULL;

	CreateImageList(16);

	// Note that the sizer for the _main_ pain informs the window of its
	//  size hints.  The top level window (the dialog) has no sizer at all.
	top->SetSizeHints(this);
}

LayerDlg::~LayerDlg()
{
    m_mgr.UnInit();

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

	wxIcon icons[8];
	icons[0] = wxICON(building);
	icons[1] = wxICON(road);
	icons[2] = wxICON(veg1);
	icons[3] = wxICON(raw);
	icons[4] = wxICON(fence);
	icons[5] = wxICON(instance);
	icons[6] = wxICON(icon8);
	icons[7] = wxICON(image);

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
	if (data->m_alay)
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

vtLayer *LayerDlg::GetLayerFromItem(wxTreeItemId item)
{
	LayerItemData *data = GetLayerDataFromItem(item);
	if (!data)
		return NULL;
	return data->m_layer;
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
	m_item = m_pTree->GetSelection();
	UpdateEnabling();
}

// Helper
wxString MakeVegLayerString(vtPlantInstanceArray3d &pia)
{
	wxString str(pia.GetFilename(), wxConvUTF8);
	wxString str2;
	str2.Printf(_(" (Plants: %d)"), pia.GetNumEntities());
	str += str2;
	return str;
}

void LayerDlg::RefreshTreeTerrain()
{
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
							str = _("File ");
							str += vs.UTF8ToWideString().c_str();
						}
						else
						{
							vs = inst->GetValueString("itemname", true);
							str = _("Item ");
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
			m_pTree->SetItemData(hLayer, new LayerItemData(alay, fset));
		}
		vtImageLayer *ilay = dynamic_cast<vtImageLayer*>(layers[i]);
		if (ilay)
		{
			vs = ilay->GetLayerName();
			str = wxString(vs, wxConvUTF8);

			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, ICON_IMAGE, ICON_IMAGE);
			m_pTree->SetItemData(hLayer, new LayerItemData(ilay));
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
	vtIcoGlobe *globe = g_App.GetGlobe();
	if (!globe)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_("Layers"), ICON_TOP, ICON_TOP);

	GlobeLayerArray &glayers = globe->GetGlobeLayers();
	for (unsigned int i = 0; i < glayers.GetSize(); i++)
	{
		GlobeLayer *glay = glayers[i];
		vtFeatureSet *feat = glay->m_pSet;

		wxString str(feat->GetFilename(), wxConvUTF8);
		wxTreeItemId hItem = m_pTree->AppendItem(hRoot, str, ICON_RAW, ICON_RAW);
		m_pTree->SetItemData(hItem, new LayerItemData(glay));

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
		m_pTree->AppendItem(hItem, str, ICON_RAW, ICON_RAW);
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
	if (data->m_glay != NULL)
	{
		g_App.RemoveGlobeAbstractLayer(data->m_glay);
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
	int index = wxGetSingleChoiceIndex(_("Layer type:"), _("Create new layer"), choices, this);
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
bool SaveAbstractLayer(vtFeatureSet *set, bool bAskFilename)
{
	vtString fname = set->GetFilename();

	if (bAskFilename)
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString default_file(StartOfFilename(fname), wxConvUTF8);
		wxString default_dir(ExtractPath(fname, false), wxConvUTF8);

		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save Abstract Data"), default_dir,
			default_file, _("GIS Files (*.shp)|*.shp"), wxFD_SAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return false;
		}
		wxString str = saveFile.GetPath();
		fname = str.mb_str(wxConvUTF8);
		set->SetFilename(fname);
	}
	set->SaveToSHP(fname);
	return true;
}

void LayerDlg::OnLayerLoad( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnLayerLoad\n");

	bool bTerrain = (g_App.m_state == AS_Terrain);

	wxString filter = _("GIS Files (*.shp)|*.shp");
	if (bTerrain)
	{
		filter += _T("|");
		filter += _("Structure Files (*.vtst)|*.vtst");
		filter += _T("|");
		filter += _("All supported layer formats (*.shp;*.vtst)|*.shp;*.vtst");
	}

	wxFileDialog loadFile(NULL, _("Load Layer"), _T(""), _T(""), filter, wxFD_OPEN);
	if (bTerrain)
		loadFile.SetFilterIndex(2);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString str = loadFile.GetPath();
	vtString fname = (const char *) str.mb_str(wxConvUTF8);

	VTLOG1(" File dialog: ");
	VTLOG1(fname);
	VTLOG1("\n");

	if (bTerrain)
	{
		vtTerrain *terr = GetCurrentTerrain();
		vtLayer *lay = terr->LoadLayer(fname);

		if (!lay)
		{
			VTLOG1(" OnLayerLoad exit.\n");
			return;
		}

		vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(lay);
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(lay);

		if (slay)
			terr->CreateStructures(slay);

		if (alay)
		{
			// Ask style for the newly loaded layer
			vtTagArray &props = alay->pSet->GetProperties();

			StyleDlg dlg(NULL, -1, _("Style"));
			dlg.SetFeatureSet(alay->pSet);
			dlg.SetOptions(vtGetDataPath(), props);
			if (dlg.ShowModal() != wxID_OK)
			{
				terr->GetLayers().Remove(alay);
				delete alay;
				return;
			}
			// Copy all the style attributes to the new featureset
			VTLOG1("  Setting featureset properties.\n");
			dlg.GetOptions(props);

			alay->CreateStyledFeatures(terr);
		}

		if (lay)
			RefreshTreeContents();
	}
	else
	{
		// earth view
		int ret = g_App.AddGlobeAbstractLayer(fname);
		if (ret == -1)
			wxMessageBox(_("Couldn't open"));
		else if (ret == -2)
			wxMessageBox(_("That file isn't point data."));
		else
			RefreshTreeContents();
	}
}

void LayerDlg::OnLayerSave( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnLayerSave\n");

	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	bool bSaved = false;
	if (data->m_type == LT_STRUCTURE && data->m_slay != NULL)
	{
		GetCurrentTerrain()->SetStructureLayer(data->m_slay);
		bSaved = g_App.SaveStructures(false);	// don't ask for filename
	}

	if (data->m_type == LT_VEG)
		bSaved = g_App.SaveVegetation(false);	// don't ask for filename

	if (data->m_fset)
		bSaved = SaveAbstractLayer(data->m_fset, false);	// don't ask for filename

	// Update the (*) next to the modified layer name
	if (bSaved)
		RefreshTreeContents();
}

void LayerDlg::OnLayerSaveAs( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnLayerSaveAs\n");

	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	bool bSaved = false;
	if (data->m_type == LT_STRUCTURE)
		bSaved = g_App.SaveStructures(true);		// ask for filename

	if (data->m_type == LT_VEG)
		bSaved = g_App.SaveVegetation(true);	// ask for filename

	if (data->m_fset)
		bSaved = SaveAbstractLayer(data->m_fset, true);	// ask for filename

	// The filename may have changed
	if (bSaved)
		RefreshTreeContents();
}

void LayerDlg::OnZoomTo( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnZoomTo\n");

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
	{
		vtGetScene()->ShadowVisibleNode(pThing, bVis);
		vtGetScene()->ComputeShadows();
	}
}

void LayerDlg::OnUpdateShadow(wxUpdateUIEvent& event)
{
	if (!IsShown())
		return;

	bool bShadows = false;
	vtNode *pThing = NULL;
#if VTLIB_OSG
	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (data && data->m_slay)
	{
		pThing = GetNodeFromItem(m_item);
		vtTerrain *terr = GetCurrentTerrain();
		bShadows = (terr && terr->GetParams().GetValueBool(STR_STRUCT_SHADOWS) &&
			pThing != NULL);
	}
#endif
	event.Enable(bShadows);

	if (pThing)
		event.Check(vtGetScene()->IsShadowVisibleNode(pThing));
}

void LayerDlg::OnVisible( wxCommandEvent &event )
{
	bool bVis = event.IsChecked();

	if (g_App.m_state == AS_Terrain)
	{
		vtStructureLayer *slay = GetStructureLayerFromItem(m_item);
		vtNode *pThing = GetNodeFromItem(m_item);
		if (pThing && slay != NULL)
		{
			pThing->SetEnabled(bVis);
			return;
		}
		vtLayer *lay = GetLayerFromItem(m_item);
		if (lay)
			lay->SetVisible(bVis);
	}
	else if (g_App.m_state == AS_Orbit)
	{
		LayerItemData *data = GetLayerDataFromItem(m_item);
		if (data && data->m_glay)
			data->m_glay->SetEnabled(bVis);
	}
}

void LayerDlg::OnUpdateVisible(wxUpdateUIEvent& event)
{
	if (!IsShown())
		return;

	if (g_App.m_state == AS_Terrain)
	{
		vtStructureLayer *slay = GetStructureLayerFromItem(m_item);
		vtNode *pThing = GetNodeFromItem(m_item);
		if (pThing && slay != NULL)
		{
			event.Check(pThing->GetEnabled());
			return;
		}
		vtLayer *lay = GetLayerFromItem(m_item);
		if (lay)
		{
			event.Check(lay->GetVisible());
		}
		event.Enable(pThing != NULL || lay != NULL);
	}
	else if (g_App.m_state == AS_Orbit)
	{
		LayerItemData *data = GetLayerDataFromItem(m_item);
		if (data && data->m_glay)
			event.Check(data->m_glay->GetEnabled());
		event.Enable(data && data->m_glay);
	}
}

void LayerDlg::OnUpdateCreate(wxUpdateUIEvent& event)
{
	if (!IsShown())
		return;
	event.Enable(g_App.m_state == AS_Terrain);
}


void LayerDlg::OnShowAll( wxCommandEvent &event )
{
	// Check to see if this might be more than they expected
	if (g_App.m_state == AS_Terrain && m_bShowAll == false && event.IsChecked())
	{
		// Count all the structures in all the layers
		int total = 0;
		vtTerrain *terr = GetCurrentTerrain();
		if (!terr)
			return;
		LayerSet &layers = terr->GetLayers();
		for (unsigned int i = 0; i < layers.GetSize(); i++)
		{
			vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(layers[i]);
			if (slay)
				total += slay->GetSize();
		}
		if (total > 5000)
		{
			wxString str;
			str.Printf(_("There are %d structures.  Are you sure you want to display them all?"), total);
			int res = wxMessageBox(str, _("Warning"), wxYES_NO);
			if (res == wxNO)
				return;
		}
	}

	m_bShowAll = event.IsChecked();
	RefreshTreeContents();
}

void LayerDlg::OnUpdateShowAll(wxUpdateUIEvent& event)
{
	event.Check(m_bShowAll);
	event.Enable(g_App.m_state == AS_Terrain);
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
	bool bRemovable = false, bSaveable = false;

	LayerItemData *data = GetLayerDataFromItem(m_item);
	vtNode *pThing = GetNodeFromItem(m_item);
	vtStructureLayer *slay = GetStructureLayerFromItem(m_item);

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

	m_pToolbar->EnableTool(ID_LAYER_DELETE, bRemovable);

	m_pToolbar->EnableTool(ID_LAYER_SAVE, bSaveable);
	m_pToolbar->EnableTool(ID_LAYER_SAVE_AS, bSaveable);

	m_pToolbar->EnableTool(ID_LAYER_ZOOM_TO, pThing != NULL);
}

