//
//  The menus functions of the main Frame window of the VTBuilder application.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtDIB.h"

#include "Frame.h"
#include "MenuEnum.h"
#include "BuilderView.h"
#include "TreeView.h"
#include "Helper.h"
// Layers
#include "ElevLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "TowerLayer.h"
#include "WaterLayer.h"
// Dialogs
#include "ElevDlg.h"
#include "ExtentDlg.h"
#include "LayerPropDlg.h"
#include "ProjectionDlg.h"
#include "DistribVegDlg.h"
#include "TowerDLG.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(ID_FILE_NEW,		MainFrame::OnProjectNew)
	EVT_MENU(ID_FILE_OPEN,		MainFrame::OnProjectOpen)
	EVT_MENU(ID_FILE_SAVE,		MainFrame::OnProjectSave)
	EVT_MENU(ID_FILE_EXIT,		MainFrame::OnQuit)

	EVT_MENU(ID_EDIT_DELETE, MainFrame::OnEditDelete)
	EVT_MENU(ID_EDIT_DESELECTALL, MainFrame::OnEditDeselectAll)
	EVT_MENU(ID_EDIT_INVERTSELECTION, MainFrame::OnEditInvertSelection)
	EVT_MENU(ID_EDIT_CROSSINGSELECTION, MainFrame::OnEditCrossingSelection)

	EVT_UPDATE_UI(ID_EDIT_DELETE,	MainFrame::OnUpdateEditDelete)

	EVT_MENU(ID_LAYER_NEW,			MainFrame::OnLayerNew)
	EVT_MENU(ID_LAYER_OPEN,			MainFrame::OnLayerOpen)
	EVT_MENU(ID_LAYER_SAVE,			MainFrame::OnLayerSave)
	EVT_MENU(ID_LAYER_SAVE_AS,		MainFrame::OnLayerSaveAs)
	EVT_MENU(ID_LAYER_IMPORT,		MainFrame::OnLayerImport)
	EVT_MENU(ID_LAYER_PROPS,		MainFrame::OnLayerProperties)
	EVT_MENU(ID_LAYER_CONVERTPROJ,	MainFrame::OnLayerConvert)
	EVT_MENU(ID_LAYER_SETPROJ,		MainFrame::OnLayerSetProjection)
	EVT_MENU(ID_LAYER_FLATTEN,		MainFrame::OnLayerFlatten)
	EVT_MENU(ID_EDIT_OFFSET,		MainFrame::OnEditOffset)

	EVT_UPDATE_UI(ID_LAYER_SAVE,	MainFrame::OnUpdateLayerSave)
	EVT_UPDATE_UI(ID_LAYER_SAVE_AS,	MainFrame::OnUpdateLayerSaveAs)
	EVT_UPDATE_UI(ID_LAYER_PROPS,	MainFrame::OnUpdateLayerProperties)
	EVT_UPDATE_UI(ID_LAYER_FLATTEN,	MainFrame::OnUpdateLayerFlatten)
	EVT_UPDATE_UI(ID_EDIT_OFFSET,	MainFrame::OnUpdateEditOffset)

	EVT_MENU(ID_VIEW_TOOLBAR,		MainFrame::OnViewToolbar)
	EVT_MENU(ID_VIEW_SHOWLAYER,		MainFrame::OnLayerShow)
	EVT_MENU(ID_VIEW_LAYERPATHS,	MainFrame::OnViewLayerPaths)
	EVT_MENU(ID_VIEW_MAGNIFIER,		MainFrame::OnViewMagnifier)
	EVT_MENU(ID_VIEW_PAN,			MainFrame::OnViewPan)
	EVT_MENU(ID_VIEW_DISTANCE,		MainFrame::OnViewDistance)
	EVT_MENU(ID_VIEW_ZOOMIN,		MainFrame::OnViewZoomIn)
	EVT_MENU(ID_VIEW_ZOOMOUT,		MainFrame::OnViewZoomOut)
	EVT_MENU(ID_VIEW_ZOOMALL,		MainFrame::OnViewZoomAll)
	EVT_MENU(ID_VIEW_FULLVIEW,		MainFrame::OnViewFull)
	EVT_MENU(ID_VIEW_SETAREA,		MainFrame::OnElevBox)
	EVT_MENU(ID_VIEW_WORLDMAP,		MainFrame::OnViewWorldMap)
	EVT_MENU(ID_VIEW_SHOWUTM,		MainFrame::OnViewUTMBounds)
	EVT_MENU(ID_VIEW_SHOWMINUTES,	MainFrame::OnViewMinutes)
	EVT_MENU(ID_VIEW_FILLWATER,		MainFrame::OnViewFillWater)

	EVT_UPDATE_UI(ID_VIEW_TOOLBAR,		MainFrame::OnUpdateToolbar)
	EVT_UPDATE_UI(ID_VIEW_SHOWLAYER,	MainFrame::OnUpdateLayerShow)
	EVT_UPDATE_UI(ID_VIEW_LAYERPATHS,	MainFrame::OnUpdateLayerPaths)
	EVT_UPDATE_UI(ID_VIEW_MAGNIFIER,	MainFrame::OnUpdateMagnifier)
	EVT_UPDATE_UI(ID_VIEW_PAN,			MainFrame::OnUpdatePan)
	EVT_UPDATE_UI(ID_VIEW_DISTANCE,		MainFrame::OnUpdateDistance)
	EVT_UPDATE_UI(ID_VIEW_FULLVIEW,		MainFrame::OnUpdateViewFull)
	EVT_UPDATE_UI(ID_VIEW_SETAREA,		MainFrame::OnUpdateElevBox)
	EVT_UPDATE_UI(ID_VIEW_WORLDMAP,		MainFrame::OnUpdateWorldMap)
	EVT_UPDATE_UI(ID_VIEW_SHOWUTM,		MainFrame::OnUpdateUTMBounds)
	EVT_UPDATE_UI(ID_VIEW_SHOWMINUTES,	MainFrame::OnUpdateMinutes)
	EVT_UPDATE_UI(ID_VIEW_FILLWATER,	MainFrame::OnUpdateFillWater)

	EVT_MENU(ID_ROAD_SELECTROAD,	MainFrame::OnSelectRoad)
	EVT_MENU(ID_ROAD_SELECTNODE,	MainFrame::OnSelectNode)
	EVT_MENU(ID_ROAD_SELECTWHOLE,	MainFrame::OnSelectWhole)
	EVT_MENU(ID_ROAD_DIRECTION,		MainFrame::OnDirection)
	EVT_MENU(ID_ROAD_EDIT,			MainFrame::OnRoadEdit)
	EVT_MENU(ID_ROAD_SHOWNODES,		MainFrame::OnRoadShowNodes)
	EVT_MENU(ID_ROAD_SHOWWIDTH,		MainFrame::OnRoadShowWidth)
	EVT_MENU(ID_ROAD_SELECTHWY,		MainFrame::OnSelectHwy)
	EVT_MENU(ID_ROAD_CLEAN,			MainFrame::OnRoadClean)

	EVT_UPDATE_UI(ID_ROAD_SELECTROAD,	MainFrame::OnUpdateSelectRoad)
	EVT_UPDATE_UI(ID_ROAD_SELECTNODE,	MainFrame::OnUpdateSelectNode)
	EVT_UPDATE_UI(ID_ROAD_SELECTWHOLE,	MainFrame::OnUpdateSelectWhole)
	EVT_UPDATE_UI(ID_ROAD_DIRECTION,	MainFrame::OnUpdateDirection)
	EVT_UPDATE_UI(ID_ROAD_EDIT,			MainFrame::OnUpdateRoadEdit)
	EVT_UPDATE_UI(ID_ROAD_SHOWNODES,	MainFrame::OnUpdateRoadShowNodes)
	EVT_UPDATE_UI(ID_ROAD_SHOWWIDTH,	MainFrame::OnUpdateRoadShowWidth)

	EVT_MENU(ID_ELEV_SELECT,			MainFrame::OnElevSelect)
	EVT_MENU(ID_ELEV_REMOVEABOVESEA,	MainFrame::OnRemoveAboveSea)
	EVT_MENU(ID_ELEV_FILLIN,			MainFrame::OnFillIn)
	EVT_MENU(ID_ELEV_SCALEELEVATION,	MainFrame::OnScaleElevation)
	EVT_MENU(ID_ELEV_EXPORTTERRAGEN,	MainFrame::OnExportTerragen)
	EVT_MENU(ID_ELEV_SHOW,				MainFrame::OnElevShow)
	EVT_MENU(ID_ELEV_SHADING,			MainFrame::OnElevShading)
	EVT_MENU(ID_ELEV_HIDE,				MainFrame::OnElevHide)
	EVT_MENU(ID_ELEV_BITMAP,			MainFrame::OnElevExportBitmap)

	EVT_UPDATE_UI(ID_ELEV_SELECT,		MainFrame::OnUpdateElevSelect)
	EVT_UPDATE_UI(ID_ELEV_REMOVEABOVESEA, MainFrame::OnUpdateRemoveAboveSea)
	EVT_UPDATE_UI(ID_ELEV_FILLIN,		MainFrame::OnUpdateFillIn)
	EVT_UPDATE_UI(ID_ELEV_SCALEELEVATION, MainFrame::OnUpdateScaleElevation)
	EVT_UPDATE_UI(ID_ELEV_EXPORTTERRAGEN, MainFrame::OnUpdateExportTerragen)
	EVT_UPDATE_UI(ID_ELEV_SHOW,			MainFrame::OnUpdateElevShow)
	EVT_UPDATE_UI(ID_ELEV_SHADING,		MainFrame::OnUpdateElevShading)
	EVT_UPDATE_UI(ID_ELEV_HIDE,			MainFrame::OnUpdateElevHide)
	EVT_UPDATE_UI(ID_ELEV_BITMAP,		MainFrame::OnUpdateExportBitmap)

	EVT_MENU(ID_TOWER_ADD,				MainFrame::OnTowerAdd)
	EVT_MENU(ID_TOWER_SELECT,			MainFrame::OnTowerSelect)
	EVT_MENU(ID_TOWER_EDIT,				MainFrame::OnTowerEdit)

	EVT_UPDATE_UI(ID_TOWER_ADD,			MainFrame::OnUpdateTowerAdd)
	EVT_UPDATE_UI(ID_TOWER_SELECT,		MainFrame::OnUpdateTowerSelect)
	EVT_UPDATE_UI(ID_TOWER_EDIT,		MainFrame::OnUpdateTowerEdit)

	EVT_MENU(ID_VEG_PLANTS,				MainFrame::OnVegPlants)
	EVT_MENU(ID_VEG_BIOREGIONS,			MainFrame::OnVegBioregions)

	EVT_MENU(ID_FEATURE_SELECT,			MainFrame::OnFeatureSelect)
	EVT_MENU(ID_STRUCTURE_EDIT_BLD,		MainFrame::OnBuildingEdit)
	EVT_MENU(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnStructureAddLinear)

	EVT_UPDATE_UI(ID_FEATURE_SELECT,	MainFrame::OnUpdateFeatureSelect)
	EVT_UPDATE_UI(ID_STRUCTURE_EDIT_BLD,	MainFrame::OnUpdateBuildingEdit)
	EVT_UPDATE_UI(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnUpdateStructureAddLinear)

	EVT_MENU(ID_RAW_SETTYPE,			MainFrame::OnRawSetType)
	EVT_MENU(ID_RAW_ADDPOINTS,			MainFrame::OnRawAddPoints)
	EVT_MENU(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnRawAddPointText)
	EVT_MENU(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnRawAddPointsGPS)

	EVT_MENU(ID_AREA_STRETCH,			MainFrame::OnAreaStretch)
	EVT_MENU(ID_AREA_TYPEIN,			MainFrame::OnAreaTypeIn)
	EVT_MENU(ID_AREA_EXPORT_ELEV,		MainFrame::OnAreaExportElev)
	EVT_MENU(ID_AREA_GENERATE_VEG,		MainFrame::OnAreaGenerateVeg)

	EVT_UPDATE_UI(ID_AREA_STRETCH,		MainFrame::OnUpdateAreaStretch)
	EVT_UPDATE_UI(ID_AREA_EXPORT_ELEV,	MainFrame::OnUpdateAreaExportElev)
	EVT_UPDATE_UI(ID_AREA_GENERATE_VEG,	MainFrame::OnUpdateAreaGenerateVeg)

	EVT_MENU(wxID_HELP,				MainFrame::OnHelpAbout)

	EVT_SIZE(MainFrame::OnSize)

	EVT_CHAR(MainFrame::OnChar)

END_EVENT_TABLE()


void MainFrame::CreateMenus()
{
	// File (project) menu
	fileMenu = new wxMenu;
	fileMenu->Append(ID_FILE_NEW, "&New\tCtrl+N", "New Project");
	fileMenu->Append(ID_FILE_OPEN, "Open Project\tCtrl+O", "Open Project");
	fileMenu->Append(ID_FILE_SAVE, "Save Project\tCtrl+S", "Save Project As");
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_FILE_EXIT, "E&xit\tAlt-X", "Exit");

 	// Edit (1)
	editMenu = new wxMenu;
	editMenu->Append(ID_EDIT_DELETE, "Delete\tDEL", "Delete.");
	editMenu->AppendSeparator();
	editMenu->Append(ID_EDIT_DESELECTALL, "Deselect All", "Clears selection.");
	editMenu->Append(ID_EDIT_INVERTSELECTION, "Invert Selection", "Invert Selection.");
	editMenu->Append(ID_EDIT_CROSSINGSELECTION, "Crossing Selection", "Crossing Selection.", true);

	// Layer (2)
	layerMenu = new wxMenu;
	layerMenu->Append(ID_LAYER_NEW, "&New Layer", "Create New Layer");
	layerMenu->Append(ID_LAYER_OPEN, "Open Layer", "Open Existing Layer");
	layerMenu->Append(ID_LAYER_SAVE, "Save Layer", "Save Active Layer");
	layerMenu->Append(ID_LAYER_SAVE_AS, "Save Layer As...", "Save Active Layer As");
	layerMenu->Append(ID_LAYER_IMPORT, "Import Data\tCtrl+I", "Import Data");
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_PROPS, "Layer Properties", "Layer Properties");
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_EDIT_OFFSET, "Offset Coordinates", "Offset");
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_FLATTEN, "&Flatten Layers", "Flatten");
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_CONVERTPROJ, "Convert Projection", "Convert");
	layerMenu->Append(ID_LAYER_SETPROJ, "Set Projection", "Set Projection");

	// View (3)
	viewMenu = new wxMenu;
	viewMenu->Append(ID_VIEW_TOOLBAR, "Toolbar", "Show Toolbar", true);
	viewMenu->Append(ID_VIEW_STATUSBAR, "Status Bar", "Show Status Bar", true);
	viewMenu->Append(ID_VIEW_SHOWLAYER, "Current Layer Visible", "Toggle Visibility of the current Layer", true);
	viewMenu->Append(ID_VIEW_LAYERPATHS, "Show Full Layer Pathnames", "Show the full path information for each Layer", true);
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_ZOOMIN, "Zoom In\tCtrl++");
	viewMenu->Append(ID_VIEW_ZOOMOUT, "Zoom Out\tCtrl+-");
	viewMenu->Append(ID_VIEW_ZOOMALL, "Zoom All");
	viewMenu->Append(ID_VIEW_FULLVIEW, "Zoom to Full Res (1:1)");
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_MAGNIFIER, "Magnifier\tZ", "Magnifier", true);
	viewMenu->Append(ID_VIEW_PAN, "Pan\t<Space>", "Pan", true);
	viewMenu->Append(ID_VIEW_DISTANCE, "Obtain Distance", "Obtain Distance", true);
	viewMenu->Append(ID_VIEW_SETAREA, "Set Export Area", "Set Export Area", true);
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_WORLDMAP, "World Map", "Show/Hide World Map", true);
	viewMenu->Append(ID_VIEW_SHOWUTM, "Show UTM Boundaries", "Show UTM Boundaries", true);
//	viewMenu->Append(ID_VIEW_SHOWGRID, "Show 7.5\" Grid", "Show 7.5\" Grid", true);
	viewMenu->Append(ID_VIEW_SHOWMINUTES, "Show minutes and seconds", "Show minutes and seconds", true);
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_ELEV_SHOW, "Show Terrain Elevation", "Show Terrain Elevation", true);
	viewMenu->Append(ID_ELEV_SHADING, "Artificial Shading", "Artificial Shading", true);
	viewMenu->Append(ID_ELEV_HIDE, "Hide Unknown Areas", "Hide Unknown Areas", true);
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_FILLWATER, "Show Water Bodies Filled", "Show Water Bodies Filled", true);

	// Roads (4)
	roadMenu = new wxMenu;
	roadMenu->Append(ID_ROAD_SELECTROAD, "Select/Modify Roads", "Select/Modify Roads", true);
	roadMenu->Append(ID_ROAD_SELECTNODE, "Select/Modify Nodes", "Select/Modify Nodes", true);
	roadMenu->Append(ID_ROAD_SELECTWHOLE, "Select Whole Roads", "Select Whole Roads", true);
	roadMenu->Append(ID_ROAD_DIRECTION, "Set Road Direction", "Set Road Direction", true);
	roadMenu->Append(ID_ROAD_EDIT, "Edit Road Points", "Edit Road Points", true);
	roadMenu->AppendSeparator();
	roadMenu->Append(ID_ROAD_SHOWNODES, "Show Nodes", "Show Nodes", true);
	roadMenu->Append(ID_ROAD_SHOWWIDTH, "Show Width", "Show the width of each road", true);
	roadMenu->Append(ID_ROAD_SELECTHWY, "Select by Highway Number", "Select Highway", true);
	roadMenu->AppendSeparator();
	roadMenu->Append(ID_ROAD_CLEAN, "Clean RoadMap", "Clean");

	// Utilities(5)
	utilityMenu = new wxMenu;
	utilityMenu->Append(ID_TOWER_ADD, "Add a Transmission Tower", "Add a Transmission Tower",true);
	utilityMenu->AppendSeparator();
	utilityMenu->Append(ID_TOWER_SELECT, "Select Utility Layer", "Select Utility Layer",true);
	utilityMenu->Append(ID_TOWER_EDIT, "Edit Transmission Towers", "Edit Transmission Towers",true);

	// Elevation (6)
	elevMenu = new wxMenu;
	elevMenu->Append(ID_ELEV_SELECT, "Select Elevation Layer", "Select Elevation Layer", true);
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_REMOVEABOVESEA, "Remove Terrain Above Sea");
	elevMenu->Append(ID_ELEV_FILLIN, "Fill in unknown areas");
	elevMenu->Append(ID_ELEV_SCALEELEVATION, "Scale Elevation");            
	elevMenu->Append(ID_ELEV_EXPORTTERRAGEN, "Export to TerraGen");            
	elevMenu->Append(ID_ELEV_BITMAP, "Generate && Export Bitmap");
//	elevMenu->AppendSeparator();
//	elevMenu->Append(ID_AREA_EXPORT_ELEV, "&Merge Area and Export");

	// Vegetation (7)
	vegMenu = new wxMenu;
	vegMenu->Append(ID_VEG_PLANTS, "Plants List", "View/Edit list of available plant species");
	vegMenu->Append(ID_VEG_BIOREGIONS, "BioRegions", "View/Edit list of species & density for each BioRegion");
//	vegMenu->Append(ID_AREA_GENERATE_VEG, "Generate");

	// Structures (8)
	bldMenu = new wxMenu;
	bldMenu->Append(ID_FEATURE_SELECT, "Select Features", "Select Features", true);
	bldMenu->Append(ID_STRUCTURE_EDIT_BLD, "Edit Buildings", "Edit Buildings", true);
	bldMenu->Append(ID_STRUCTURE_ADD_LINEAR, "Add Linear Features", "Add Linear Features", true);

	// Raw (9)
	rawMenu = new wxMenu;
	rawMenu->Append(ID_RAW_SETTYPE, "Set Entity Type", "Set Entity Type");
	rawMenu->Append(ID_RAW_ADDPOINTS, "Add Points with Mouse", "Add points with the mouse", true);
	rawMenu->Append(ID_RAW_ADDPOINT_TEXT, "Add Point with Text\tCtrl+T", "Add point");
	rawMenu->Append(ID_RAW_ADDPOINTS_GPS, "Add Points with GPS", "Add points with GPS");

	// Area (10)
	areaMenu = new wxMenu;
	areaMenu->Append(ID_AREA_STRETCH, "Set to Extents",
		"Set the Export Area rectangle to the combined extent of all layers.");
	areaMenu->Append(ID_AREA_TYPEIN, "Numeric Values",
		"Set the Export Area rectangle by text entry of coordinates.");
	areaMenu->AppendSeparator();
	areaMenu->Append(ID_AREA_EXPORT_ELEV, "&Merge && Export Elevation",
		"Sample all elevation data within the Export Area to produce a single, new elevation.");
	areaMenu->Append(ID_AREA_GENERATE_VEG, "Generate && Export Vegetation",
		"Generate Vegetation File (*.vf) containg plant distribution.");

	// Help (11)
	helpMenu = new wxMenu;
    helpMenu->Append(wxID_HELP, "&About", "About VTBuilder");

	menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, "&Project");
	menuBar->Append(editMenu, "&Edit");
	menuBar->Append(layerMenu, "&Layer");
	menuBar->Append(viewMenu, "&View");
	menuBar->Append(roadMenu, "&Roads");
	menuBar->Append(utilityMenu, "Util&ities");
	menuBar->Append(elevMenu, "Elev&ation");
	menuBar->Append(vegMenu, "Veg&etation");
	menuBar->Append(bldMenu, "&Structures");
	menuBar->Append(rawMenu, "Ra&w");
	menuBar->Append(areaMenu, "Export &Area");
	menuBar->Append(helpMenu, "&Help");
	SetMenuBar(menuBar);

#if 0
    // Accelerators
    wxAcceleratorEntry entries[5];
    entries[0].Set(wxACCEL_CTRL, (int) 'O', ID_FILE_OPEN);
    entries[1].Set(wxACCEL_CTRL, (int) 'S', ID_FILE_SAVE);
    entries[2].Set(wxACCEL_CTRL, (int) '+', ID_VIEW_ZOOMIN);
    entries[3].Set(wxACCEL_CTRL, (int) '-', ID_VIEW_ZOOMOUT);
    entries[4].Set(wxACCEL_NORMAL,  WXK_DELETE,    ID_EDIT_DELETE);
    wxAcceleratorTable accel(5, entries);
    SetAcceleratorTable(accel);
#endif
}

////////////////////////////////////////////////////////////////
// Project menu

void MainFrame::OnProjectNew(wxCommandEvent &event)
{
	SetActiveLayer(NULL);
	DeleteContents();
	m_area.SetRect(0.0, 0.0, 0.0, 0.0);
	m_pView->Refresh();
	Refresh();

	RefreshTreeView();
	RefreshToolbar();

	vtProjection p;
	SetProjection(p);
}

#define PROJECT_FILTER "VTBuilder Project Files (*.vtb)|*.vtb|"

void MainFrame::OnProjectOpen(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, "Load Project", "", "", PROJECT_FILTER, wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	LoadProject(loadFile.GetPath());
}

void MainFrame::OnProjectSave(wxCommandEvent &event)
{
	wxFileDialog saveFile(NULL, "Save Project", "", "", PROJECT_FILTER,
		wxSAVE | wxOVERWRITE_PROMPT );
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strPathName = saveFile.GetPath();

	SaveProject(strPathName);
}

void MainFrame::OnQuit(wxCommandEvent &event)
{
	Close(TRUE);
}

//////////////////////////////////////////////////
// Edit menu

void MainFrame::OnEditDelete(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (pRL && (pRL->NumSelectedNodes() != 0 || pRL->NumSelectedRoads() != 0))
	{
		m_pView->DeleteSelected(pRL);
		pRL->SetModified(true);
		return;
	}
	vtStructureLayer *pSL = GetActiveStructureLayer();
	if (pSL && pSL->NumSelected() != 0)
	{
		pSL->DeleteSelected();
		pSL->SetModified(true);
		m_pView->Refresh();
		return;
	}

	vtLayer *pL = GetActiveLayer();
	if (pL)
	{
		int result = wxMessageBox("Are you sure you want to delete the current layer?",
			"Question", wxYES_NO | wxICON_QUESTION, this);
		if (result == wxYES)
			RemoveLayer(pL);
	}
}

void MainFrame::OnUpdateEditDelete(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnEditDeselectAll(wxCommandEvent &event)
{
	m_pView->DeselectAll();
}

void MainFrame::OnEditInvertSelection(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (pRL) {
		pRL->InvertSelection();
		m_pView->Refresh();
	}	
	vtStructureLayer *pSL = GetActiveStructureLayer();
	if (pSL) {
		pSL->InvertSelection();
		m_pView->Refresh();
	}	
}

void MainFrame::OnEditCrossingSelection(wxCommandEvent &event)
{
	m_pView->m_bCrossSelect = !m_pView->m_bCrossSelect;
}

void MainFrame::OnUpdateCrossingSelection(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bCrossSelect);
}

void MainFrame::OnEditOffset(wxCommandEvent &event)
{
	wxTextEntryDialog dlg(this, "Offset",
		"Please enter horizontal offset X, Y", "0.0, 0.0");
	if (dlg.ShowModal() != wxID_OK)
		return;

	DPoint2 offset;
	wxString str = dlg.GetValue();
	sscanf(str, "%lf, %lf", &offset.x, &offset.y);

	GetActiveLayer()->Offset(offset);
	GetActiveLayer()->SetModified(true);
	m_pView->Refresh();
}

void MainFrame::OnUpdateEditOffset(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}


//////////////////////////////////////////////////
// Layer menu

void MainFrame::OnLayerNew(wxCommandEvent &event)
{
	LayerType t = AskLayerType();
	if (t == LT_UNKNOWN)
		return;

	vtLayer *pL = vtLayer::CreateNewLayer(t);
	if (pL)
	{
		pL->SetProjection(m_proj);
		AddLayer(pL);
		m_pTree->RefreshTreeItems(this);
	}
}

void MainFrame::OnLayerOpen(wxCommandEvent &event)
{
	wxString filter = "Native Layer Formats||";

	AddType(filter, FSTRING_RMF);	// roads
	AddType(filter, FSTRING_BT);	// elevation
	AddType(filter, FSTRING_SHP);	// raw
	AddType(filter, FSTRING_VTST);	// structures
	AddType(filter, FSTRING_UTL);	//utility towers

	// ask the user for a filename
	wxFileDialog loadFile(NULL, "Open Layer", "", "", filter, wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	LoadLayer(loadFile.GetPath());
}

void MainFrame::OnLayerSave(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	if (lp->GetFilename().Left(9).CmpNoCase("untitled.") == 0)
	{
		if (!lp->AskForSaveFilename())
			return;
	}
	wxString msg = "Saving layer to file " + lp->GetFilename();
	SetStatusText(msg);

	if (lp->Save())
		msg = "Saved layer to file " + lp->GetFilename();
	else
		msg = "Save failed.";
	SetStatusText(msg);
}

void MainFrame::OnUpdateLayerSave(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp != NULL && lp->GetModified());
}

void MainFrame::OnLayerSaveAs(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();

	if (!lp->AskForSaveFilename())
			return;

	wxString msg = "Saving layer to file as " + lp->GetFilename();
	SetStatusText(msg);

	lp->Save();
	lp->SetModified(false);

	msg = "Saved layer to file as " + lp->GetFilename();
	SetStatusText(msg);
}

void MainFrame::OnUpdateLayerSaveAs(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnUpdateLayerProperties(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnLayerImport(wxCommandEvent &event)
{
	// first ask what kind of data layer
	LayerType t = AskLayerType();
	if (t != LT_UNKNOWN)
		ImportData(t);
}

void MainFrame::OnLayerProperties(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	if (!lp)
		return;

	//
	// Currently, we only have a dialog for Elevation layer properties
	//
	LayerType ltype = lp->GetType();

	wxString title;
	title += vtLayer::LayerTypeName[ltype];
	title += " Layer Properties";
	LayerPropDlg dlg(NULL, -1, title, wxDefaultPosition);

	// Fill in initial values for the dialog
	DRECT rect, rect2;
	lp->GetExtent(rect);
	dlg.m_fLeft = rect.left;
	dlg.m_fTop = rect.top;
	dlg.m_fRight = rect.right;
	dlg.m_fBottom = rect.bottom;

	lp->GetPropertyText(dlg.m_strText);

	// For elevation layers, if the user changes the extents, apply.
	if (dlg.ShowModal() == wxID_OK && ltype == LT_ELEVATION)
	{
		vtElevLayer *pEL = (vtElevLayer *) lp;
		rect2.left = dlg.m_fLeft;
		rect2.top = dlg.m_fTop;
		rect2.right = dlg.m_fRight;
		rect2.bottom = dlg.m_fBottom;
		if (rect2 != rect)
		{
			pEL->m_pGrid->SetGridExtents(rect2);
			m_pView->Refresh();
		}
	}
}

void MainFrame::OnAreaExportElev(wxCommandEvent &event)
{
	ExportElevation();
}

void MainFrame::OnUpdateAreaExportElev(wxUpdateUIEvent& event)
{
	event.Enable(LayersOfType(LT_ELEVATION) > 0 && !m_area.IsEmpty());
}

void MainFrame::OnLayerConvert(wxCommandEvent &event)
{
	// ask for what projection to convert to
	ProjectionDlg dlg(NULL, -1, "Projection", wxDefaultPosition);
	dlg.m_strCaption = "Convert to what projection?";
	dlg.SetProjection(m_proj);

	// might go from geo to utm, provide a good guess for UTM zone
	if (m_proj.IsGeographic())
	{
		DRECT extents = GetExtents();
		dlg.m_iZone = GuessZoneFromLongitude((extents.left + extents.right) / 2.0);
	}

	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	vtProjection proj;
	dlg.GetProjection(proj);

	// count through the layer array, converting
	int layers = m_Layers.GetSize();
	int succeeded = 0;
	for (int i = 0; i < layers; i++)
	{
		vtLayer *lp = m_Layers.GetAt(i);
		bool success = lp->ConvertProjection(proj);
		if (success)
			succeeded++;
	}
	if (succeeded < layers)
	{
		wxString str;
		if (layers == 1)
			str.Printf("Failed to convert.");
		else
			str.Printf("Failed to convert %d of %d layers.",
				layers-succeeded, layers);
		wxMessageBox(str);
	}

	SetProjection(proj);
	m_pView->ZoomAll();
}

void MainFrame::OnLayerSetProjection(wxCommandEvent &event)
{
	// Allow the user to directly specify the projection for all loaded
	// layers (override it, without reprojecting the layer's data)
	// ask for what projection to convert to
	ProjectionDlg dlg(NULL, -1, "Projection", wxDefaultPosition);
	dlg.m_strCaption = "Set to what projection?";
	dlg.SetProjection(m_proj);

	if (m_proj.IsGeographic())
	{
		// might go from geo to utm, provide a good guess for UTM zone
		DRECT extents = GetExtents();
		dlg.m_iZone = GuessZoneFromLongitude((extents.left + extents.right) / 2.0);
	}

	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	vtProjection proj;
	dlg.GetProjection(proj);

	// count through the layer array, converting
	int layers = m_Layers.GetSize();
	for (int i = 0; i < layers; i++)
	{
		vtLayer *lp = m_Layers.GetAt(i);
		lp->SetProjection(proj);
	}

	SetProjection(proj);
	m_pView->ZoomAll();
}

void MainFrame::OnUpdateLayerConvert(wxUpdateUIEvent& event)
{
	event.Enable(m_Layers.GetSize() != 0);
}

void MainFrame::OnLayerFlatten(wxCommandEvent &event)
{
	vtLayer *pActive = GetActiveLayer();
	LayerType t = pActive->GetType();

	// count down through the layer array, flattening
	int layers = m_Layers.GetSize();
	for (int i = layers-1; i >= 0; i--)
	{
		vtLayer *pL = m_Layers.GetAt(i);
		if (pL == pActive) continue;
		if (pL->GetType() != t) continue;

//		TRACE("Merging layer %s/%x with %s/%x\n",
//			pL->GetFilename(), pL, pActive->GetFilename(), pActive);
		if (pActive->AppendDataFrom(pL))
		{
			// successfully merged contents, so second layer can be deleted
			RemoveLayer(pL);
		}
	}

	wxString newname = "untitled";
	newname += vtLayer::LayerFileExtension[t];
	pActive->SetFilename(newname);
	pActive->SetModified(true);
}

void MainFrame::OnUpdateLayerFlatten(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp &&
		(lp->GetType() == LT_ROAD ||
		 lp->GetType() == LT_VEG ||
		 lp->GetType() == LT_STRUCTURE));
}

////////////////////////////////////////////////////////////
// View menu

void MainFrame::OnViewToolbar(wxCommandEvent &event)
{
	toolBar_main->Show( !toolBar_main->IsShown() );

	wxSizeEvent dummy;
	wxFrame::OnSize(dummy);
}

void MainFrame::OnUpdateToolbar(wxUpdateUIEvent& event)
{
	event.Check( toolBar_main->IsShown() );
}

void MainFrame::OnLayerShow(wxCommandEvent &event)
{
	vtLayer *pLayer = GetActiveLayer();
	pLayer->SetVisible(!pLayer->GetVisible());

	DRECT r;
	pLayer->GetExtent(r);
	wxRect sr = m_pView->WorldToWindow(r);
	IncreaseRect(sr, 1);
	m_pView->Refresh(TRUE, &sr);
}

void MainFrame::OnUpdateLayerShow(wxUpdateUIEvent& event)
{
	vtLayer *pLayer = GetActiveLayer();

	event.Enable(pLayer != NULL);
	event.Check(pLayer && pLayer->GetVisible());
}

void MainFrame::OnViewLayerPaths(wxCommandEvent &event)
{
	m_pTree->SetShowPaths(!m_pTree->GetShowPaths());
	m_pTree->RefreshTreeItems(this);
}

void MainFrame::OnUpdateLayerPaths(wxUpdateUIEvent& event)
{
	event.Check(m_pTree->GetShowPaths());
}

void MainFrame::OnViewMagnifier(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Mag);
	m_pView->SetCorrectCursor();
}

void MainFrame::OnUpdateMagnifier(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Mag );
}

void MainFrame::OnViewPan(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Pan);
	m_pView->SetCorrectCursor();
}

void MainFrame::OnUpdatePan(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Pan );
}

void MainFrame::OnViewDistance(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Dist);
	m_pView->SetCorrectCursor();
}

void MainFrame::OnUpdateDistance(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Dist );
}

void MainFrame::OnViewZoomIn(wxCommandEvent &event)
{
	m_pView->SetScale(m_pView->GetScale() * 1.2);
}

void MainFrame::OnViewZoomOut(wxCommandEvent &event)
{
	m_pView->SetScale(m_pView->GetScale() / 1.2);
}

void MainFrame::OnViewZoomAll(wxCommandEvent &event)
{
	m_pView->ZoomAll();
}

void MainFrame::OnViewFull(wxCommandEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	if (pEL)
		m_pView->MatchZoomToElev(pEL);
}

void MainFrame::OnUpdateViewFull(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp &&
		(lp->GetType() == LT_ELEVATION || lp->GetType() == LT_IMAGE));
}

void MainFrame::OnViewWorldMap()
{
	m_pView->m_bShowMap = !m_pView->m_bShowMap;
	m_pView->Refresh();
}

void MainFrame::OnUpdateWorldMap(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bShowMap);
}

void MainFrame::OnViewUTMBounds()
{
	m_pView->m_bShowUTMBounds = !m_pView->m_bShowUTMBounds;
	m_pView->Refresh();
}

void MainFrame::OnUpdateUTMBounds(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bShowUTMBounds);
}

void MainFrame::OnViewMinutes()
{
	m_bShowMinutes = !m_bShowMinutes;
	m_statbar->SetShowMinutes(m_bShowMinutes);
}

void MainFrame::OnUpdateMinutes(wxUpdateUIEvent& event)
{
	event.Check(m_bShowMinutes);
}

void MainFrame::OnViewFillWater()
{
	vtWaterLayer::m_bFill = !vtWaterLayer::m_bFill;
}

void MainFrame::OnUpdateFillWater(wxUpdateUIEvent& event)
{
	event.Check(vtWaterLayer::m_bFill);
}

//////////////////////////
// Road

void MainFrame::OnSelectRoad(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Road);
}

void MainFrame::OnUpdateSelectRoad(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Road );
}

void MainFrame::OnSelectNode(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Node);
}

void MainFrame::OnUpdateSelectNode(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Node );
}

void MainFrame::OnSelectWhole(wxCommandEvent &event)
{
	m_pView->SetMode(LB_RoadExtend);
}

void MainFrame::OnUpdateSelectWhole(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_RoadExtend );
}

void MainFrame::OnDirection(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Dir);
}

void MainFrame::OnUpdateDirection(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Dir );
}

void MainFrame::OnRoadEdit(wxCommandEvent &event)
{
	m_pView->SetMode(LB_RoadEdit);
}

void MainFrame::OnUpdateRoadEdit(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_RoadEdit );
}

void MainFrame::OnRoadShowNodes(wxCommandEvent &event)
{
	bool state = vtRoadLayer::GetDrawNodes();
	vtRoadLayer::SetDrawNodes(!state);
	m_pView->Refresh(state);
}

void MainFrame::OnUpdateRoadShowNodes(wxUpdateUIEvent& event)
{
	event.Check(vtRoadLayer::GetDrawNodes());
}

void MainFrame::OnRoadShowWidth(wxCommandEvent &event)
{
	bool state = vtRoadLayer::GetDrawWidth();
	vtRoadLayer::SetDrawWidth(!state);
	m_pView->Refresh();
}

void MainFrame::OnUpdateRoadShowWidth(wxUpdateUIEvent& event)
{
	event.Check(vtRoadLayer::GetDrawWidth());
}

void MainFrame::OnSelectHwy(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	wxTextEntryDialog dlg(this, "Please enter highway number", "Select Highway", "");
	if (dlg.ShowModal() == wxID_OK)
	{
		int num;
		wxString str = dlg.GetValue();
		sscanf(str, "%d", &num);
		if (pRL->SelectHwyNum(num))
			m_pView->Refresh();
	}
}

void MainFrame::OnRoadClean(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	int count;
	wxString str;
	OpenProgressDialog("Cleaning RoadMap");

	UpdateProgressDialog(10, "Removing unused nodes");
	count = pRL->RemoveUnusedNodes();
	if (count)
	{
		str = wxString::Format("Removed %i nodes\n", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(20, "Merging redundant nodes");
	// potentially takes a long time...
	count = pRL->MergeRedundantNodes(progress_callback);
	if (count)
	{
		str = wxString::Format("Merged %d redundant roads", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(30, "Removing degenerate roads");
	count = pRL->RemoveDegenerateRoads();
	if (count)
	{
		str = wxString::Format("Removed %d degenerate roads", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(40, "Removing unnecessary nodes");
	count = pRL->RemoveUnnecessaryNodes();
	if (count)
	{
		str = wxString::Format("Removed %d unnecessary nodes", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(50, "Cleaning road points");
	count = pRL->CleanRoadPoints();
	if (count)
	{
		str = wxString::Format("Cleaned %d road points", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(60, "Removing dangling roads");
	count = pRL->DeleteDanglingRoads();
	if (count)
	{
		str = wxString::Format("Removed %i dangling roads", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(70, "Fixing overlapped roads");
	count = pRL->FixOverlappedRoads();
	if (count)
	{
		str = wxString::Format("Fixed %i overlapped roads", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(80, "Fixing extraneous parallels");
	count = pRL->FixExtraneousParallels();
	if (count)
	{
		str = wxString::Format("Fixed %i extraneous parallels", count);
		wxMessageBox(str, "", wxOK);
	}

	UpdateProgressDialog(90, "Splitting looping roads");
	count = pRL->SplitLoopingRoads();
	if (count)
	{
		str = wxString::Format("Split %d looping roads", count);
		wxMessageBox(str, "", wxOK);
	}

	CloseProgressDialog();
	pRL->SetModified(true);
	pRL->ComputeExtents();

	m_pView->Refresh();
}


//////////////////////////
// Elevation

void MainFrame::OnElevSelect(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TSelect);
}

void MainFrame::OnUpdateElevSelect(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_TSelect);
}

void MainFrame::OnElevBox(wxCommandEvent& event)
{
	m_pView->SetMode(LB_Box);
}

void MainFrame::OnUpdateElevBox(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_Box);
}

void MainFrame::OnRemoveAboveSea(wxCommandEvent &event)
{
	vtElevLayer *t = GetActiveElevLayer();
	if (!t)
		return;

	vtElevationGrid *grid = t->m_pGrid;
	int iColumns, iRows;
	grid->GetDimensions(iColumns, iRows);
	for (int i = 0; i < iColumns; i++)
	{
		for (int j = 0; j < iRows; j++)
		{
			if (grid->GetFValue(i, j) > 0.0f)
				grid->SetFValue(i, j, INVALID_ELEVATION);
		}
	}
	t->ReRender();

	m_pView->Refresh();
}

void MainFrame::OnUpdateRemoveAboveSea(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveElevLayer() != NULL);
}

void MainFrame::OnFillIn(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	el->FillGaps();
	el->ReRender();
	m_pView->Refresh();
}

void MainFrame::OnUpdateFillIn(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveElevLayer() != NULL);
}

void MainFrame::OnScaleElevation(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	if (!el)
		return;

	ScaleDlg dlg;
	dlg.LoadFromResource(this, "dialog2");
	dlg.m_bScaleUp = true;
	dlg.m_strUpBy = "1.0";
	dlg.m_strDownBy = "1.0";
	if (dlg.ShowModal() != wxID_OK)
		return;

	float fScale;
	if (dlg.m_bScaleUp)
		fScale = atof(dlg.m_strUpBy);
	else
		fScale = 1.0f / atof(dlg.m_strDownBy);

	vtElevationGrid *grid = el->m_pGrid;

	int iColumns, iRows;
	grid->GetDimensions(iColumns, iRows);
	for (int i = 0; i < iColumns; i++)
	{
		for (int j = 0; j < iRows; j++)
		{
			float f = grid->GetFValue(i, j);
			if (f != INVALID_ELEVATION)
				grid->SetFValue(i, j, f * fScale);
		}
	}
	el->SetModified(true);
	el->ReRender();

	m_pView->Refresh();
}

void MainFrame::OnUpdateScaleElevation(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveElevLayer() != NULL);
}

void MainFrame::OnExportTerragen(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	if (!el)
		return;

	wxString filter = "All Files|*.*|";
	AddType(filter, FSTRING_TER);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, "Export Elevation", "", "", filter, wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strPathName = saveFile.GetPath();

	bool success = el->m_pGrid->SaveToTerragen(strPathName);
	if (!success)
	{
		wxMessageBox("Couldn't open file for writing.");
		return;
	}

	wxString str = "Successfully wrote TerraGen file ";
	str += strPathName;
	wxMessageBox(str);
}

void MainFrame::OnUpdateExportTerragen(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveElevLayer() != NULL);
}

void MainFrame::OnElevExportBitmap(wxCommandEvent& event)
{
	int size = 0;
	while (size < 32 || size > 8192)
	{
		wxTextEntryDialog dlg(this, "Please enter pixel size of bitmap",
			"Export Bitmap", "");
		if (dlg.ShowModal() != wxID_OK)
			return;

		wxString str = dlg.GetValue();
		sscanf(str, "%d", &size);
	}

	// Ask for file name
	wxFileDialog loadFile(NULL, "Output filename for bitmap", "", "",
		"Bitmap Files (*.bmp)|*.bmp|", wxSAVE|wxOVERWRITE_PROMPT);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	OpenProgressDialog("Generating Bitmap");

	wxString fname = loadFile.GetPath();
	vtDIB dib(size, size, 24, false);

	vtElevLayer *pEL = GetActiveElevLayer();

	pEL->PaintDibFromElevation(&dib, true);

	UpdateProgressDialog(100, "Writing bitmap to file.");
	bool success = dib.WriteBMP(fname);
	CloseProgressDialog();
}

void MainFrame::OnUpdateExportBitmap(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveElevLayer() != NULL);
}


//////////////////////////////////////////////////////////////////////////
// Area Menu
//

void MainFrame::OnAreaStretch(wxCommandEvent &event)
{
	m_pView->AreaStretch();
}

void MainFrame::OnUpdateAreaStretch(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnAreaTypeIn(wxCommandEvent &event)
{
	ExtentDlg dlg(NULL, -1, "Edit Area", wxDefaultPosition);
	dlg.SetArea(m_area, !m_proj.IsGeographic());
	if (dlg.ShowModal() == wxID_OK)
	{
		m_area = dlg.m_area;
		m_pView->Refresh();
	}
}

void MainFrame::OnElevShow(wxCommandEvent &event)
{
	vtElevLayer::m_bShowElevation = !vtElevLayer::m_bShowElevation;
	m_pView->Refresh();
}

void MainFrame::OnUpdateElevShow(wxUpdateUIEvent& event)
{
	event.Check(vtElevLayer::m_bShowElevation);
}

void MainFrame::OnElevShading(wxCommandEvent &event)
{
	vtElevLayer::m_bShading = !vtElevLayer::m_bShading;
}

void MainFrame::OnUpdateElevShading(wxUpdateUIEvent& event)
{
	event.Check(vtElevLayer::m_bShading);
}

void MainFrame::OnElevHide(wxCommandEvent &event)
{
	vtElevLayer::m_bDoMask = !vtElevLayer::m_bDoMask;

	if (!LayersOfType(LT_ELEVATION))
		return;

	// refresh the display
	for (int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *lp = m_Layers.GetAt(i);
		if (lp->GetType() == LT_ELEVATION)
		{
			vtElevLayer *elp = (vtElevLayer *)lp;
			elp->ReRender();
		}
	}
	m_pView->Refresh();
}

void MainFrame::OnUpdateElevHide(wxUpdateUIEvent& event)
{
	event.Check(vtElevLayer::m_bDoMask);
}


//////////////////////////
// Vegetation menu

void MainFrame::OnVegPlants(wxCommandEvent& event)
{
	// if PlantList has not previously been open, get the data from file first
	if (!m_PlantListDlg)
	{
		// Use file dialog to open plant list text file.
		wxFileDialog loadFile(NULL, "Load Plant Info", "", "", PLANTS_FILTER,
			wxOPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		// Read Plantlist file.
		if (!GetPlantList()->Read(loadFile.GetPath()))
		{
			wxMessageBox("Couldn't read plant list from that file.", "Error",
				wxOK, this);
			return;
		}

		// Create new Plant List Dialog
		m_PlantListDlg = new PlantListDlg(this, WID_PLANTS, "Plants List", 
		wxPoint(140, 100), wxSize(950, 400), wxSYSTEM_MENU | wxCAPTION);
	}

	// Display plant list data, calling OnInitDialog.
	m_PlantListDlg->Show(true);
}


void MainFrame::OnVegBioregions(wxCommandEvent& event)
{
	// if data isn't there, get the data first
	if (!m_BioRegionDlg)
	{
		// Use file dialog to open bioregion text file.
		wxFileDialog loadFile(NULL, "Load BioRegion Info", "", "",
			BIOREGIONS_FILTER, wxOPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		// Read bioregions, data kept on frame with m_pBioRegion.
		if (!GetBioRegion()->Read(loadFile.GetPath()))
		{
			wxMessageBox("Couldn't read bioregion list from that file.",
				"Error", wxOK, this);
			return;
		}

		// Create new Bioregion Dialog
		m_BioRegionDlg = new BioRegionDlg(this, WID_BIOREGIONS, "BioRegions List", 
			wxPoint(120, 80), wxSize(300, 500), wxSYSTEM_MENU | wxCAPTION);
	}

	// Display bioregion data, calling OnInitDialog.
	m_BioRegionDlg->Show(true);
}


void MainFrame::OnAreaGenerateVeg(wxCommandEvent& event)
{
	// Open File Save Dialog
	wxFileDialog saveFile(NULL, "Save Vegetation File", "", "",
		"Vegetation Files (*.vf)|*.vf|", wxSAVE | wxOVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	DistribVegDlg dlg(this, -1, "Vegetation Distribution Options");
	dlg.m_fSampling = 40.0f;
	dlg.m_fScarcity = 0.001f;
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	// Generate the plants
	vtVegLayer *Density = NULL, *BioMap = NULL;
	FindVegLayers(&Density, &BioMap);

	float fTreeSpacing = dlg.m_fSampling;
	float fTreeScarcity = dlg.m_fScarcity;

	GenerateVegetation(strPathName, m_area, Density, BioMap, fTreeSpacing,
		fTreeScarcity);
}

void MainFrame::OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event)
{
	vtVegLayer *Density = NULL, *BioMap = NULL;

	FindVegLayers(&Density, &BioMap);

	event.Enable(m_PlantListDlg && m_BioRegionDlg && Density && BioMap &&
		!m_area.IsEmpty());
}


//////////////////////////////
// Utility Menu

void MainFrame::OnTowerSelect(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TowerSelect);

}
void MainFrame::OnUpdateTowerSelect(wxUpdateUIEvent &event)
{
	event.Check(m_pView->GetMode()== LB_TowerSelect);
}
void MainFrame::OnTowerEdit(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TowerEdit);
}
void MainFrame::OnUpdateTowerEdit(wxUpdateUIEvent &event)
{
	event.Check(m_pView->GetMode() == LB_TowerEdit);
}
void MainFrame::OnTowerAdd(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TowerAdd);
}
void MainFrame::OnUpdateTowerAdd(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode()==LB_TowerAdd);
}


/////////////////////////////////////
// Buildings / Features

void MainFrame::OnFeatureSelect(wxCommandEvent &event)
{
	m_pView->SetMode(LB_FSelect);
}

void MainFrame::OnUpdateFeatureSelect(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_FSelect);
}

void MainFrame::OnBuildingEdit(wxCommandEvent &event)
{
	m_pView->SetMode(LB_BldEdit);
}

void MainFrame::OnUpdateBuildingEdit(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_BldEdit);
}

void MainFrame::OnStructureAddLinear(wxCommandEvent &event)
{
	m_pView->SetMode(LB_AddLinear);
}

void MainFrame::OnUpdateStructureAddLinear(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_AddLinear);
}


///////////////////////////////////
// Raw menu

void MainFrame::OnRawSetType(wxCommandEvent& event)
{
	static int types[4] = { SHPT_NULL, SHPT_POINT, SHPT_ARC, SHPT_POLYGON };
	wxString choices[4];
	for (int i = 0; i < 4; i++)
		choices[i] = SHPTypeName(types[i]);

    int n = 4;
	int cur_type = 0;

    wxSingleChoiceDialog dialog(this, "These are your choices",
        "Please indicate entity type:", n, (const wxString *)choices);

    dialog.SetSelection(cur_type);

    if (dialog.ShowModal() == wxID_OK)
	{
		cur_type = dialog.GetSelection();
		vtRawLayer *pRL = (vtRawLayer *) GetActiveLayer();
		pRL->SetEntityType(types[cur_type]);
	}
}

void MainFrame::OnRawAddPoints(wxCommandEvent& event)
{
	m_pView->SetMode(LB_AddPoints);
}

void MainFrame::OnUpdateRawAddPoints(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetEntityType() == SHPT_POINT);
}

void MainFrame::OnRawAddPointText(wxCommandEvent& event)
{
	wxString str = wxGetTextFromUser("(X, Y) in current projection",
		"Enter coordinate");
	if (str == "")
		return;
	double x, y;
	int num = sscanf((const char *)str, "%lf, %lf", &x, &y);
	if (num != 2)
		return;
	DPoint2 p(x, y);
	vtRawLayer *pRL = GetActiveRawLayer();
	pRL->AddPoint(p);
	m_pView->Refresh();
}

void MainFrame::OnUpdateRawAddPointText(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetEntityType() == SHPT_POINT);
}

void MainFrame::OnRawAddPointsGPS(wxCommandEvent& event)
{
}

void MainFrame::OnUpdateRawAddPointsGPS(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetEntityType() == SHPT_POINT);
}


////////////////////
// Help

void MainFrame::OnHelpAbout(wxCommandEvent &event)
{
	wxString str = "Virtual Terrain Builder\nPowerful, easy to use, free!\n\n";
	str += "Please read the HTML documentation and license.\n\n";
	str += "Send feedback to: ben@vterrain.org\n";
	str += "Build date: ";
	str += __DATE__;
	wxMessageBox(str, "About VTBuilder");
}

