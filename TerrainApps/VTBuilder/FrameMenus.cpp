//
//  The menus functions of the main Frame window of the VTBuilder application.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/progdlg.h>
#include <wx/choicdlg.h>

#include "vtdata/config_vtdata.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/Icosa.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtdata/WFSClient.h"

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
#include "UtilityLayer.h"
#include "VegLayer.h"
#include "WaterLayer.h"
// Dialogs
#include "DistribVegDlg.h"
#include "ExtentDlg.h"
#include "FeatInfoDlg.h"
#include "LayerPropDlg.h"
#include "MapServerDlg.h"
#include "OptionsDlg.h"
#include "Projection2Dlg.h"
#include "SelectDlg.h"
#include "VegDlg.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_FILE_NEW,		MainFrame::OnProjectNew)
EVT_MENU(ID_FILE_OPEN,		MainFrame::OnProjectOpen)
EVT_MENU(ID_FILE_SAVE,		MainFrame::OnProjectSave)
EVT_MENU(ID_SPECIAL_DYMAX_TEXTURES,	MainFrame::OnDymaxTexture)
EVT_MENU(ID_SPECIAL_PROCESS_BILLBOARD,	MainFrame::OnProcessBillboard)
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
EVT_MENU(ID_LAYER_IMPORTTIGER,	MainFrame::OnLayerImportTIGER)
EVT_MENU(ID_LAYER_IMPORTUTIL,	MainFrame::OnLayerImportUtil)
EVT_MENU(ID_LAYER_IMPORT_MS,	MainFrame::OnLayerImportMapSource)
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

EVT_MENU(ID_VIEW_SHOWLAYER,		MainFrame::OnLayerShow)
EVT_MENU(ID_VIEW_MAGNIFIER,		MainFrame::OnViewMagnifier)
EVT_MENU(ID_VIEW_PAN,			MainFrame::OnViewPan)
EVT_MENU(ID_VIEW_DISTANCE,		MainFrame::OnViewDistance)
EVT_MENU(ID_VIEW_ZOOMIN,		MainFrame::OnViewZoomIn)
EVT_MENU(ID_VIEW_ZOOMOUT,		MainFrame::OnViewZoomOut)
EVT_MENU(ID_VIEW_ZOOMALL,		MainFrame::OnViewZoomAll)
EVT_MENU(ID_VIEW_FULLVIEW,		MainFrame::OnViewFull)
EVT_MENU(ID_VIEW_SETAREA,		MainFrame::OnViewSetArea)
EVT_MENU(ID_VIEW_WORLDMAP,		MainFrame::OnViewWorldMap)
EVT_MENU(ID_VIEW_SHOWUTM,		MainFrame::OnViewUTMBounds)
EVT_MENU(ID_VIEW_OPTIONS,		MainFrame::OnViewOptions)

EVT_UPDATE_UI(ID_VIEW_SHOWLAYER,	MainFrame::OnUpdateLayerShow)
EVT_UPDATE_UI(ID_VIEW_MAGNIFIER,	MainFrame::OnUpdateMagnifier)
EVT_UPDATE_UI(ID_VIEW_PAN,			MainFrame::OnUpdatePan)
EVT_UPDATE_UI(ID_VIEW_DISTANCE,		MainFrame::OnUpdateDistance)
EVT_UPDATE_UI(ID_VIEW_FULLVIEW,		MainFrame::OnUpdateViewFull)
EVT_UPDATE_UI(ID_VIEW_SETAREA,		MainFrame::OnUpdateViewSetArea)
EVT_UPDATE_UI(ID_VIEW_WORLDMAP,		MainFrame::OnUpdateWorldMap)
EVT_UPDATE_UI(ID_VIEW_SHOWUTM,		MainFrame::OnUpdateUTMBounds)

EVT_MENU(ID_ROAD_SELECTROAD,	MainFrame::OnSelectLink)
EVT_MENU(ID_ROAD_SELECTNODE,	MainFrame::OnSelectNode)
EVT_MENU(ID_ROAD_SELECTWHOLE,	MainFrame::OnSelectWhole)
EVT_MENU(ID_ROAD_DIRECTION,		MainFrame::OnDirection)
EVT_MENU(ID_ROAD_EDIT,			MainFrame::OnRoadEdit)
EVT_MENU(ID_ROAD_SHOWNODES,		MainFrame::OnRoadShowNodes)
EVT_MENU(ID_ROAD_SHOWWIDTH,		MainFrame::OnRoadShowWidth)
EVT_MENU(ID_ROAD_SELECTHWY,		MainFrame::OnSelectHwy)
EVT_MENU(ID_ROAD_CLEAN,			MainFrame::OnRoadClean)
EVT_MENU(ID_ROAD_GUESS,			MainFrame::OnRoadGuess)
EVT_MENU(ID_ROAD_FLATTEN,		MainFrame::OnRoadFlatten)

EVT_UPDATE_UI(ID_ROAD_SELECTROAD,	MainFrame::OnUpdateSelectLink)
EVT_UPDATE_UI(ID_ROAD_SELECTNODE,	MainFrame::OnUpdateSelectNode)
EVT_UPDATE_UI(ID_ROAD_SELECTWHOLE,	MainFrame::OnUpdateSelectWhole)
EVT_UPDATE_UI(ID_ROAD_DIRECTION,	MainFrame::OnUpdateDirection)
EVT_UPDATE_UI(ID_ROAD_EDIT,			MainFrame::OnUpdateRoadEdit)
EVT_UPDATE_UI(ID_ROAD_SHOWNODES,	MainFrame::OnUpdateRoadShowNodes)
EVT_UPDATE_UI(ID_ROAD_SHOWWIDTH,	MainFrame::OnUpdateRoadShowWidth)
EVT_UPDATE_UI(ID_ROAD_FLATTEN,		MainFrame::OnUpdateRoadFlatten)

EVT_MENU(ID_ELEV_SELECT,			MainFrame::OnElevSelect)
EVT_MENU(ID_ELEV_REMOVERANGE,		MainFrame::OnRemoveElevRange)
EVT_MENU(ID_ELEV_SETUNKNOWN,		MainFrame::OnElevSetUnknown)
EVT_MENU(ID_ELEV_FILLIN,			MainFrame::OnFillIn)
EVT_MENU(ID_ELEV_SCALE,				MainFrame::OnScaleElevation)
EVT_MENU(ID_ELEV_EXPORTTERRAGEN,	MainFrame::OnExportTerragen)
EVT_MENU(ID_ELEV_EXPORT_GEOTIFF,	MainFrame::OnExportGeoTIFF)
EVT_MENU(ID_ELEV_EXPORT_BMP,		MainFrame::OnExportBMP)
EVT_MENU(ID_ELEV_SHOW,				MainFrame::OnElevShow)
EVT_MENU(ID_ELEV_SHADING,			MainFrame::OnElevShading)
EVT_MENU(ID_ELEV_HIDE,				MainFrame::OnElevHide)
EVT_MENU(ID_ELEV_BITMAP,			MainFrame::OnElevExportBitmap)
EVT_MENU(ID_ELEV_MERGETIN,			MainFrame::OnElevMergeTin)

EVT_UPDATE_UI(ID_ELEV_SELECT,		MainFrame::OnUpdateElevSelect)
EVT_UPDATE_UI(ID_ELEV_REMOVERANGE,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SETUNKNOWN,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_FILLIN,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SCALE,		MainFrame::OnUpdateScaleElevation)
EVT_UPDATE_UI(ID_ELEV_EXPORTTERRAGEN, MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_EXPORT_GEOTIFF, MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_EXPORT_BMP,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SHOW,			MainFrame::OnUpdateElevShow)
EVT_UPDATE_UI(ID_ELEV_SHADING,		MainFrame::OnUpdateElevShading)
EVT_UPDATE_UI(ID_ELEV_HIDE,			MainFrame::OnUpdateElevHide)
EVT_UPDATE_UI(ID_ELEV_BITMAP,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_MERGETIN,		MainFrame::OnUpdateElevMergeTin)

EVT_MENU(ID_TOWER_ADD,				MainFrame::OnTowerAdd)
EVT_MENU(ID_TOWER_SELECT,			MainFrame::OnTowerSelect)
EVT_MENU(ID_TOWER_EDIT,				MainFrame::OnTowerEdit)

EVT_UPDATE_UI(ID_TOWER_ADD,			MainFrame::OnUpdateTowerAdd)
EVT_UPDATE_UI(ID_TOWER_SELECT,		MainFrame::OnUpdateTowerSelect)
EVT_UPDATE_UI(ID_TOWER_EDIT,		MainFrame::OnUpdateTowerEdit)

EVT_MENU(ID_VEG_PLANTS,				MainFrame::OnVegPlants)
EVT_MENU(ID_VEG_BIOREGIONS,			MainFrame::OnVegBioregions)
EVT_MENU(ID_VEG_EXPORTSHP,			MainFrame::OnVegExportSHP)

EVT_UPDATE_UI(ID_VEG_EXPORTSHP,		MainFrame::OnUpdateVegExportSHP)

EVT_MENU(ID_FEATURE_SELECT,			MainFrame::OnFeatureSelect)
EVT_MENU(ID_FEATURE_PICK,			MainFrame::OnFeaturePick)
EVT_MENU(ID_FEATURE_TABLE,			MainFrame::OnFeatureTable)
EVT_MENU(ID_STRUCTURE_EDIT_BLD,		MainFrame::OnBuildingEdit)
EVT_MENU(ID_STRUCTURE_ADD_POINTS,	MainFrame::OnBuildingAddPoints)
EVT_MENU(ID_STRUCTURE_DELETE_POINTS,	MainFrame::OnBuildingDeletePoints)
EVT_MENU(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnStructureAddLinear)
EVT_MENU(ID_STRUCTURE_EDIT_LINEAR,	MainFrame::OnStructureEditLinear)
EVT_MENU(ID_STRUCTURE_ADD_FOUNDATION, MainFrame::OnStructureAddFoundation)
EVT_MENU(ID_STRUCTURE_CONSTRAIN,	MainFrame::OnStructureConstrain)

EVT_UPDATE_UI(ID_FEATURE_SELECT,	MainFrame::OnUpdateFeatureSelect)
EVT_UPDATE_UI(ID_FEATURE_PICK,		MainFrame::OnUpdateFeaturePick)
EVT_UPDATE_UI(ID_FEATURE_TABLE,		MainFrame::OnUpdateFeatureTable)
EVT_UPDATE_UI(ID_STRUCTURE_EDIT_BLD,	MainFrame::OnUpdateBuildingEdit)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_POINTS,	MainFrame::OnUpdateBuildingAddPoints)
EVT_UPDATE_UI(ID_STRUCTURE_DELETE_POINTS,	MainFrame::OnUpdateBuildingDeletePoints)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnUpdateStructureAddLinear)
EVT_UPDATE_UI(ID_STRUCTURE_EDIT_LINEAR,	MainFrame::OnUpdateStructureEditLinear)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_FOUNDATION,	MainFrame::OnUpdateStructureAddFoundation)
EVT_UPDATE_UI(ID_STRUCTURE_CONSTRAIN,	MainFrame::OnUpdateStructureConstrain)

EVT_MENU(ID_RAW_SETTYPE,			MainFrame::OnRawSetType)
EVT_MENU(ID_RAW_ADDPOINTS,			MainFrame::OnRawAddPoints)
EVT_MENU(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnRawAddPointText)
EVT_MENU(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnRawAddPointsGPS)
EVT_MENU(ID_RAW_SELECTCONDITION,	MainFrame::OnRawSelectCondition)

EVT_UPDATE_UI(ID_RAW_SETTYPE,		MainFrame::OnUpdateRawSetType)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS,		MainFrame::OnUpdateRawAddPoints)
EVT_UPDATE_UI(ID_RAW_ADDPOINT_TEXT,	MainFrame::OnUpdateRawAddPointText)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS_GPS,	MainFrame::OnUpdateRawAddPointsGPS)
EVT_UPDATE_UI(ID_RAW_SELECTCONDITION,	MainFrame::OnUpdateRawSelectCondition)

EVT_MENU(ID_AREA_STRETCH,			MainFrame::OnAreaStretch)
EVT_MENU(ID_AREA_TYPEIN,			MainFrame::OnAreaTypeIn)
EVT_MENU(ID_AREA_EXPORT_ELEV,		MainFrame::OnAreaExportElev)
EVT_MENU(ID_AREA_EXPORT_IMAGE,		MainFrame::OnAreaExportImage)
EVT_MENU(ID_AREA_GENERATE_VEG,		MainFrame::OnAreaGenerateVeg)
EVT_MENU(ID_AREA_REQUEST_LAYER,		MainFrame::OnAreaRequestLayer)
EVT_MENU(ID_AREA_REQUEST_WMS,		MainFrame::OnAreaRequestWMS)

EVT_UPDATE_UI(ID_AREA_STRETCH,		MainFrame::OnUpdateAreaStretch)
EVT_UPDATE_UI(ID_AREA_EXPORT_ELEV,	MainFrame::OnUpdateAreaExportElev)
EVT_UPDATE_UI(ID_AREA_EXPORT_IMAGE,	MainFrame::OnUpdateAreaExportImage)
EVT_UPDATE_UI(ID_AREA_GENERATE_VEG,	MainFrame::OnUpdateAreaGenerateVeg)

EVT_MENU(wxID_HELP,				MainFrame::OnHelpAbout)

EVT_CHAR(MainFrame::OnChar)
EVT_CLOSE(MainFrame::OnClose)

END_EVENT_TABLE()


void MainFrame::CreateMenus()
{
	int menu_num = 0;

	m_pMenuBar = new wxMenuBar;

	// Project menu
	fileMenu = new wxMenu;
	fileMenu->Append(ID_FILE_NEW, _T("&New\tCtrl+N"), _T("New Project"));
	fileMenu->Append(ID_FILE_OPEN, _T("Open Project\tCtrl+O"), _T("Open Project"));
	fileMenu->Append(ID_FILE_SAVE, _T("Save Project\tCtrl+S"), _T("Save Project As"));
#ifndef ELEVATION_ONLY
	fileMenu->AppendSeparator();
	wxMenu *specialMenu = new wxMenu;
	specialMenu->Append(ID_SPECIAL_DYMAX_TEXTURES, _T("Create Dymaxion Textures"));
	specialMenu->Append(ID_SPECIAL_PROCESS_BILLBOARD, _T("Process Billboard Texture"));
	fileMenu->Append(0, _T("Special"), specialMenu);
#endif
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_FILE_EXIT, _T("E&xit\tAlt-X"), _T("Exit"));
	m_pMenuBar->Append(fileMenu, _T("&Project"));
	menu_num++;

 	// Edit
	editMenu = new wxMenu;
	editMenu->Append(ID_EDIT_DELETE, _T("Delete\tDEL"), _T("Delete."));
	editMenu->AppendSeparator();
	editMenu->Append(ID_EDIT_DESELECTALL, _T("Deselect All"), _T("Clears selection."));
	editMenu->Append(ID_EDIT_INVERTSELECTION, _T("Invert Selection"), _T("Invert Selection."));
#ifndef ELEVATION_ONLY
	editMenu->AppendCheckItem(ID_EDIT_CROSSINGSELECTION, _T("Crossing Selection"));
#endif
	m_pMenuBar->Append(editMenu, _T("&Edit"));
	menu_num++;

	// Layer
	layerMenu = new wxMenu;
#ifndef ELEVATION_ONLY
	layerMenu->Append(ID_LAYER_NEW, _T("&New Layer"), _T("Create New Layer"));
#endif
	layerMenu->Append(ID_LAYER_OPEN, _T("Open Layer"), _T("Open Existing Layer"));
	layerMenu->Append(ID_LAYER_SAVE, _T("Save Layer"), _T("Save Active Layer"));
	layerMenu->Append(ID_LAYER_SAVE_AS, _T("Save Layer As..."), _T("Save Active Layer As"));
	layerMenu->Append(ID_LAYER_IMPORT, _T("Import Data\tCtrl+I"), _T("Import Data"));
#ifndef ELEVATION_ONLY
	layerMenu->Append(ID_LAYER_IMPORTTIGER, _T("Import Data From TIGER"), _T("Import Data From TIGER"));
	layerMenu->Append(ID_LAYER_IMPORTUTIL, _T("Import Utilites From SHP"), _T("Import Utilites From SHP"));
	layerMenu->Append(ID_LAYER_IMPORT_MS, _T("Import from MapSource file"));
#endif
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_PROPS, _T("Layer Properties"), _T("Layer Properties"));
	layerMenu->Append(ID_EDIT_OFFSET, _T("Offset Coordinates"), _T("Offset"));
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_FLATTEN, _T("&Flatten Layers"), _T("Flatten"));
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_CONVERTPROJ, _T("Convert Projection"), _T("Convert"));
	layerMenu->Append(ID_LAYER_SETPROJ, _T("Set Projection"), _T("Set Projection"));
	m_pMenuBar->Append(layerMenu, _T("&Layer"));
	menu_num++;

	// View
	viewMenu = new wxMenu;
	viewMenu->AppendCheckItem(ID_VIEW_SHOWLAYER, _T("Current Layer Visible"),
		_T("Toggle Visibility of the current Layer"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_ZOOMIN, _T("Zoom In\tCtrl++"));
	viewMenu->Append(ID_VIEW_ZOOMOUT, _T("Zoom Out\tCtrl+-"));
	viewMenu->Append(ID_VIEW_ZOOMALL, _T("Zoom All"));
	viewMenu->Append(ID_VIEW_FULLVIEW, _T("Zoom to Full Res (1:1)"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_MAGNIFIER, _T("Magnifier\tZ"));
	viewMenu->AppendCheckItem(ID_VIEW_PAN, _T("Pan\t<Space>"));
	viewMenu->AppendCheckItem(ID_VIEW_DISTANCE, _T("Obtain Distance"));
	viewMenu->AppendCheckItem(ID_VIEW_SETAREA, _T("Set Export Area"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_WORLDMAP, _T("World Map"), _T("Show/Hide World Map"));
	viewMenu->AppendCheckItem(ID_VIEW_SHOWUTM, _T("Show UTM Boundaries"));
//	viewMenu->AppendCheckItem(ID_VIEW_SHOWGRID, _T("Show 7.5\" Grid"), _T("Show 7.5\" Grid"), true);
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_ELEV_SHOW, _T("Show Terrain Elevation"));
	viewMenu->AppendCheckItem(ID_ELEV_SHADING, _T("Artificial Shading"));
	viewMenu->AppendCheckItem(ID_ELEV_HIDE, _T("Hide Unknown Areas"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_OPTIONS, _T("Options"));
	m_pMenuBar->Append(viewMenu, _T("&View"));
	menu_num++;

#ifndef ELEVATION_ONLY
	// Roads
	roadMenu = new wxMenu;
	roadMenu->AppendCheckItem(ID_ROAD_SELECTROAD, _T("Select/Modify Roads"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTNODE, _T("Select/Modify Nodes"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTWHOLE, _T("Select Whole Roads"));
	roadMenu->AppendCheckItem(ID_ROAD_DIRECTION, _T("Set Road Direction"));
	roadMenu->AppendCheckItem(ID_ROAD_EDIT, _T("Edit Road Points"));
	roadMenu->AppendSeparator();
	roadMenu->AppendCheckItem(ID_ROAD_SHOWNODES, _T("Show Nodes"));
	roadMenu->AppendCheckItem(ID_ROAD_SHOWWIDTH, _T("Show Width"), _T("Show the width of each road"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTHWY, _T("Select by Highway Number"));
	roadMenu->AppendSeparator();
	roadMenu->Append(ID_ROAD_CLEAN, _T("Clean RoadMap"), _T("Clean"));
	roadMenu->Append(ID_ROAD_GUESS, _T("Guess Intersection Types"));
	roadMenu->Append(ID_ROAD_FLATTEN, _T("Flatten Elevation Grid Under Roads"));
	m_pMenuBar->Append(roadMenu, _T("&Roads"));
	m_iLayerMenu[LT_ROAD] = menu_num;
	menu_num++;

	// Utilities
	utilityMenu = new wxMenu;
	utilityMenu->AppendCheckItem(ID_TOWER_ADD, _T("Add a Transmission Tower"));
	utilityMenu->AppendSeparator();
	utilityMenu->AppendCheckItem(ID_TOWER_SELECT, _T("Select Utility Layer"));
	utilityMenu->AppendCheckItem(ID_TOWER_EDIT, _T("Edit Transmission Towers"));
	m_pMenuBar->Append(utilityMenu, _T("Util&ities"));
	m_iLayerMenu[LT_UTILITY] = menu_num;
	menu_num++;
#endif

	// Elevation
	elevMenu = new wxMenu;
	elevMenu->AppendCheckItem(ID_ELEV_SELECT, _T("Se&lect Elevation Layer"));
	elevMenu->Append(ID_ELEV_SCALE, _T("Sc&ale Elevation"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_REMOVERANGE, _T("&Remove Elevation Range..."));
	elevMenu->Append(ID_ELEV_FILLIN, _T("&Fill In Unknown Areas"));
	elevMenu->Append(ID_ELEV_SETUNKNOWN, _T("&Set Unknown Areas"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_EXPORTTERRAGEN, _T("E&xport to TerraGen"));            
	elevMenu->Append(ID_ELEV_EXPORT_GEOTIFF, _T("Export to &GeoTIFF"));            
	elevMenu->Append(ID_ELEV_EXPORT_BMP, _T("Export to &BMP"));
	elevMenu->Append(ID_ELEV_BITMAP, _T("Re&nder and Save Bitmap"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_MERGETIN, _T("&Merge shared TIN vertices"));
	m_pMenuBar->Append(elevMenu, _T("Elev&ation"));
	m_iLayerMenu[LT_ELEVATION] = menu_num;
	menu_num++;

#ifndef ELEVATION_ONLY
	// Vegetation
	vegMenu = new wxMenu;
	vegMenu->Append(ID_VEG_PLANTS, _T("Plants List"), _T("View/Edit list of available plant species"));
	vegMenu->Append(ID_VEG_BIOREGIONS, _T("BioRegions"), _T("View/Edit list of species & density for each BioRegion"));
	vegMenu->Append(ID_VEG_EXPORTSHP, _T("Export SHP"));
	m_pMenuBar->Append(vegMenu, _T("Veg&etation"));
	m_iLayerMenu[LT_VEG] = menu_num;
	menu_num++;

	// Structures
	bldMenu = new wxMenu;
	bldMenu->AppendCheckItem(ID_FEATURE_SELECT, _T("Select Features"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_EDIT_BLD, _T("Edit Buildings"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_POINTS, _T("Add points to building footprints"), _T(""));
	bldMenu->AppendCheckItem(ID_STRUCTURE_DELETE_POINTS, _T("Delete points from building footprints"), _T(""));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_LINEAR, _T("Add Linear Structures"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_EDIT_LINEAR, _T("Edit Linear Structures"));
	bldMenu->AppendSeparator();
	bldMenu->Append(ID_STRUCTURE_ADD_FOUNDATION, _T("Add Foundation Levels to Buildings"), _T(""));
	bldMenu->AppendSeparator();
	bldMenu->AppendCheckItem(ID_STRUCTURE_CONSTRAIN, _T("Constrain angles on footprint edit"));
	m_pMenuBar->Append(bldMenu, _T("&Structures"));
	m_iLayerMenu[LT_STRUCTURE] = menu_num;
	menu_num++;
#endif

	// Raw
	rawMenu = new wxMenu;
	rawMenu->AppendCheckItem(ID_FEATURE_SELECT, _T("Select Features"));
	rawMenu->AppendCheckItem(ID_FEATURE_PICK, _T("Pick Features"));
	rawMenu->AppendCheckItem(ID_FEATURE_TABLE, _T("Show Attribute Table"));
#ifndef ELEVATION_ONLY
	rawMenu->AppendSeparator();
	rawMenu->Append(ID_RAW_SETTYPE, _T("Set Entity Type"), _T("Set Entity Type"));
	rawMenu->AppendCheckItem(ID_RAW_ADDPOINTS, _T("Add Points with Mouse"));
	rawMenu->Append(ID_RAW_ADDPOINT_TEXT, _T("Add Point with Text\tCtrl+T"), _T("Add point"));
	rawMenu->Append(ID_RAW_ADDPOINTS_GPS, _T("Add Points with GPS"), _T("Add points with GPS"));
#endif
	rawMenu->AppendSeparator();
	rawMenu->Append(ID_RAW_SELECTCONDITION, _T("Select Features by Condition"));
	m_pMenuBar->Append(rawMenu, _T("Ra&w"));
	m_iLayerMenu[LT_RAW] = menu_num;
	menu_num++;

	// Area
	areaMenu = new wxMenu;
	areaMenu->Append(ID_AREA_STRETCH, _T("Set to Extents"),
		_T("Set the Export Area rectangle to the combined extent of all layers."));
	areaMenu->Append(ID_AREA_TYPEIN, _T("Numeric Values"),
		_T("Set the Export Area rectangle by text entry of coordinates."));
	areaMenu->AppendSeparator();
	areaMenu->Append(ID_AREA_EXPORT_ELEV, _T("&Merge && Export Elevation"),
		_T("Sample all elevation data within the Export Area to produce a single, new elevation."));
#ifndef ELEVATION_ONLY
	areaMenu->Append(ID_AREA_EXPORT_IMAGE, _T("Extract && Export Image"),
		_T("Sample imagery within the Export Area to produce a single, new image."));
	areaMenu->Append(ID_AREA_GENERATE_VEG, _T("Generate && Export Vegetation"),
		_T("Generate Vegetation File (*.vf) containg plant distribution."));
	areaMenu->Append(ID_AREA_REQUEST_LAYER, _T("Request Layer from WFS"));
	areaMenu->Append(ID_AREA_REQUEST_WMS, _T("Request Image from WMS"));
#endif
	m_pMenuBar->Append(areaMenu, _T("Export &Area"));
	menu_num++;

	// Help
	helpMenu = new wxMenu;
	helpMenu->Append(wxID_HELP, _T("&About"), _T("About ") _T(APPNAME));
	m_pMenuBar->Append(helpMenu, _T("&Help"));
	menu_num++;

	SetMenuBar(m_pMenuBar);

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

#define PROJECT_FILTER _T(APPNAME) _T(" Project Files (*.vtb)|*.vtb|")

void MainFrame::OnProjectOpen(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _T("Load Project"), _T(""), _T(""),
		PROJECT_FILTER, wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	LoadProject(loadFile.GetPath());
}

void MainFrame::OnProjectSave(wxCommandEvent &event)
{
	wxFileDialog saveFile(NULL, _T("Save Project"), _T(""), _T(""),
		PROJECT_FILTER, wxSAVE | wxOVERWRITE_PROMPT );
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strPathName = saveFile.GetPath();

	SaveProject(strPathName);
}

void MainFrame::OnDymaxTexture(wxCommandEvent &event)
{
	int i, x, y, face;
	DPoint3 uvw;
	uvw.z = 0.0f;
	double u, v;
	double lat, lon;

	wxFileDialog dlg(this, _T("Choose input file"), _T(""), _T(""), _T("*.bmp;*.png"));
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString choices[6];
	choices[0] = _T("128");
	choices[1] = _T("256");
	choices[2] = _T("512");
	choices[3] = _T("1024");
	choices[4] = _T("2048");
	choices[5] = _T("4096");
	wxSingleChoiceDialog dlg2(this, _T("Size of each output tile?"),
		_T("Query"), 6, choices);
	if (dlg2.ShowModal() == wxID_CANCEL)
		return;
	int sel = dlg2.GetSelection();
	int output_size = 1 << (7+sel);

	wxTextEntryDialog dlg3(this, _T("Prefix for output filenames?"), _T("Query"));
	if (dlg3.ShowModal() == wxID_CANCEL)
		return;
	wxString prefix = dlg3.GetValue();

	wxImage::AddHandler(new wxPNGHandler);

	wxProgressDialog prog(_T("Processing"), _T("Loading source bitmap.."), 100);
	prog.Show(TRUE);

	// read texture
	int input_x, input_y;
	unsigned char r, g, b;
	wxImage img;
	if (!img.LoadFile(dlg.GetPath()))
	{
		DisplayAndLog("File open failed");
		return;
	}
	input_x = img.GetWidth();
	input_y = img.GetHeight();

	DymaxIcosa ico;
	ico.InitIcosa();

	wxImage out[10];
	for (i = 0; i < 10; i++)
	{
		out[i].Create(output_size, output_size);

		wxString msg;
		msg.Printf(_T("Creating tile %d.."), i+1);
		prog.Update((i+1)*10, msg);

		for (x = 0; x < output_size; x++)
		{
			for (y = 0; y < output_size; y++)
			{
				if (y < output_size-1-x)
				{
					face = icosa_face_pairs[i][0];
					u = (double)x / output_size;
					v = (double)y / output_size;
				}
				else
				{
					face = icosa_face_pairs[i][1];
					u = (double)(output_size-1-x) / output_size;
					v = (double)(output_size-1-y) / output_size;
				}
				uvw.x = u;
				uvw.y = v;
				ico.FaceUVToGeo(face, uvw, lon, lat);

				int source_x = (int) (lon / PI2d * input_x);
				int source_y = (int) (lat / PId * input_y);

				r = img.GetRed(source_x, source_y);
				g = img.GetGreen(source_x, source_y);
				b = img.GetBlue(source_x, source_y);

				out[i].SetRGB(x, output_size-1-y, r, g, b);
			}
		}
		wxString name;
		name.Printf(_T("%s%02d%02d.png"), prefix.c_str(),
			icosa_face_pairs[i][0]+1, icosa_face_pairs[i][1]+1);
		bool success = out[i].SaveFile(name, wxBITMAP_TYPE_PNG);
		if (!success)
		{
			DisplayAndLog("Failed to write file %s.", name.mb_str());
			return;
		}
	}
	DisplayAndLog("Successful.");
}

bool ProcessBillboardTexture(const char *fname_in, const char *fname_out,
							 const RGBi &bg, void progress_callback(int) = NULL)
{
	float blend_factor;
	vtDIB dib1, dib2, dib3;
	if (!dib1.ReadPNG(fname_in))
	{
		DisplayAndLog("Couldn't read input file.");
		return false;
	}
	int i, j, width, height, x, y;
	width = dib1.GetWidth();
	height = dib1.GetHeight();

	// First pass: restore color of edge texels by guessing correct
	//  non-background color.
	RGBAi c, res, diff;
	dib2.Create(width, height, 32);
	for (i = 0; i < width; i++)
	{
		progress_callback(i*100/width);
		for (j = 0; j < height; j++)
		{
			dib1.GetPixel32(i, j, c);
			if (c.a == 0)
			{
				res = bg;
				res.a = 0;
			}
			else if (c.a == 255)
			{
				res = c;
			}
			else
			{
				blend_factor = c.a / 255.0f;

				diff = c - bg;
				res = bg + (diff * (1.0f / blend_factor));
				res.Crop();
				res.a = c.a;
			}
			dib2.SetPixel32(i, j, res);
		}
	}
	dib2.WritePNG("D:/2d/pass1.png");

	// Now make many passes over the bitmap, filling in areas of alpha==0
	//  with values from the nearest pixels.
	dib3.Create(width, height, 24);
	int filled_in = 1;
	int progress_target = -1;
	while (filled_in)
	{
		filled_in = 0;
		dib3.SetColor(RGBi(0,0,0));

		RGBi sum;
		int surround;
		for (i = 0; i < width; i++)
		{
			for (j = 0; j < height; j++)
			{
				dib2.GetPixel32(i, j, c);
				if (c.a != 0)
					continue;

				// collect surrounding values
				sum.Set(0,0,0);
				surround = 0;
				for (x = -1; x <= 1; x++)
				for (y = -1; y <= 1; y++)
				{
					if (x == 0 && y == 0) continue;
					if (i+x < 0) continue;
					if (i+x > width-1) continue;
					if (j+y < 0) continue;
					if (j+y > height-1) continue;
					dib2.GetPixel32(i+x, j+y, c);
					if (c.a != 0)
					{
						sum += c;
						surround++;
					}
				}
				if (surround > 2)
				{
					sum /= (float) surround;
					dib3.SetPixel24(i, j, sum);
				}
			}
		}
		for (i = 0; i < width; i++)
		{
			for (j = 0; j < height; j++)
			{
				dib2.GetPixel32(i, j, c);
				if (c.a == 0)
				{
					dib3.GetPixel24(i, j, sum);
					if (sum.r != 0 || sum.g != 0 || sum.b != 0)
					{
						c = sum;
						c.a = 1;
						dib2.SetPixel32(i, j, c);
						filled_in++;
					}
				}
			}
		}
		if (progress_target == -1 && filled_in > 0)
			progress_target = filled_in * 2 / 3;
		progress_callback((progress_target - filled_in) * 100 / progress_target);
	}
	// One final pass: changed the regions with alpha==1 to 0
	// (we were just using the value as a flag)
	for (i = 0; i < width; i++)
	{
		progress_callback(i*100/width);
		for (j = 0; j < height; j++)
		{
			dib2.GetPixel32(i, j, c);
			if (c.a == 1)
				c.a = 0;
			dib2.SetPixel32(i, j, c);
		}
	}

	if (dib2.WritePNG(fname_out))
		DisplayAndLog("Successful.");
	else
		DisplayAndLog("Unsuccessful.");
	return true;
}


void MainFrame::OnProcessBillboard(wxCommandEvent &event)
{
	wxTextEntryDialog dlg1(this,
		_T("This feature allows you to process a billboard texture to remove\n")
		_T("unwanted background effects on its edges.  The file should be in\n")
		_T("PNG format, 24-bit color plus 8-bit alpha.  To begin, specify the\n")
		_T("current background color of the image to process.  Enter the color\n")
		_T("as R G B, e.g. black is 0 0 0 and white is 255 255 255."), _T("Wizard"));
	if (dlg1.ShowModal() == wxID_CANCEL)
		return;
	RGBi bg;
	wxString2 str;
	str = dlg1.GetValue();
	const char *color = str.mb_str();
	int res = sscanf(color, "%d %d %d", &bg.r, &bg.g, &bg.b);
	if (res != 3)
	{
		DisplayAndLog("Couldn't parse color.");
		return;
	}
	wxFileDialog dlg2(this, _T("Choose input texture file"), _T(""), _T(""), _T("*.png"));
	if (dlg2.ShowModal() == wxID_CANCEL)
		return;
	str = dlg2.GetPath();
	const char *fname_in = str.mb_str();

	wxFileDialog dlg3(this, _T("Choose output texture file"), _T(""), _T(""), _T("*.png"), wxSAVE);
	if (dlg3.ShowModal() == wxID_CANCEL)
		return;
	str = dlg3.GetPath();
	const char *fname_out = str.mb_str();

	OpenProgressDialog(_T("Processing"));

	ProcessBillboardTexture(fname_in, fname_out, bg, progress_callback);

	CloseProgressDialog();
}

void MainFrame::OnQuit(wxCommandEvent &event)
{
	Close(FALSE);
}

//////////////////////////////////////////////////
// Edit menu

void MainFrame::OnEditDelete(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (pRL && (pRL->NumSelectedNodes() != 0 || pRL->NumSelectedLinks() != 0))
	{
		wxString str;
		str.Printf(_T("Deleting road selection: %d nodes and %d roads"),
			pRL->NumSelectedNodes(), pRL->NumSelectedLinks());
		SetStatusText(str);
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
	vtRawLayer *pRawL = GetActiveRawLayer();
	if (pRawL && pRawL->NumSelected() != 0)
	{
		pRawL->DeleteSelected();
		pRawL->SetModified(true);
		m_pView->Refresh();
		OnSelectionChanged();
		return;
	}

	vtLayer *pL = GetActiveLayer();
	if (pL)
	{
		int result = wxMessageBox(_T("Are you sure you want to delete the current layer?"),
				_T("Question"), wxYES_NO | wxICON_QUESTION, this);
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
		m_pView->Refresh(false);
	}
	vtStructureLayer *pSL = GetActiveStructureLayer();
	if (pSL) {
		pSL->InvertSelection();
		m_pView->Refresh(false);
	}
	vtRawLayer *pRawL = GetActiveRawLayer();
	if (pRawL) {
		pRawL->InvertSelection();
		m_pView->Refresh(false);
		OnSelectionChanged();
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
	wxTextEntryDialog dlg(this, _T("Offset"),
		_T("Please enter horizontal offset X, Y"), _T("0.0, 0.0"));
	if (dlg.ShowModal() != wxID_OK)
		return;

	DPoint2 offset;
	wxString2 str = dlg.GetValue();
	sscanf(str.mb_str(), "%lf, %lf", &offset.x, &offset.y);

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
		SetActiveLayer(pL);
		m_pView->SetActiveLayer(pL);
		AddLayer(pL);
		m_pTree->RefreshTreeItems(this);
		RefreshToolbar();
	}
}

void MainFrame::OnLayerOpen(wxCommandEvent &event)
{
	wxString filter = _T("Native Layer Formats||");

	AddType(filter, FSTRING_BT);	// elevation
	AddType(filter, FSTRING_BTGZ);	// compressed elevation
	AddType(filter, FSTRING_TIN);	// elevation
#ifndef ELEVATION_ONLY
	AddType(filter, FSTRING_RMF);	// roads
	AddType(filter, FSTRING_GML);	// raw
	AddType(filter, FSTRING_UTL);	// utility towers
	AddType(filter, FSTRING_VTST);	// structures
	AddType(filter, FSTRING_VF);	// vegetation files
	AddType(filter, FSTRING_TIF);	// image files
	AddType(filter, FSTRING_IMG);	// image or elevation file
#endif
	AddType(filter, FSTRING_SHP);	// raw files

	// ask the user for a filename
	wxFileDialog loadFile(NULL, _T("Open Layer"), _T(""), _T(""), filter, wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	LoadLayer(loadFile.GetPath());
}

void MainFrame::OnLayerSave(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	if (lp->GetLayerFilename().Left(9).CmpNoCase(_T("untitled.")) == 0)
	{
		if (!lp->AskForSaveFilename())
			return;
	}
	wxString2 msg = _T("Saving layer to file ") + lp->GetLayerFilename();
	SetStatusText(msg);
	VTLOG(msg.mb_str());
	VTLOG("\n");

	if (lp->Save())
		msg = _T("Saved layer to file ") + lp->GetLayerFilename();
	else
		msg = _T("Save failed.");
	SetStatusText(msg);
	VTLOG(msg.mb_str());
	VTLOG("\n");
}

void MainFrame::OnUpdateLayerSave(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp != NULL && lp->GetModified() && lp->CanBeSaved());
}

void MainFrame::OnLayerSaveAs(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();

	if (!lp->AskForSaveFilename())
		return;

	wxString2 msg = _T("Saving layer to file as ") + lp->GetLayerFilename();
	SetStatusText(msg);

	g_Log._Log(msg.mb_str());
	g_Log._Log("\n");

	bool success = lp->Save();
	if (success)
	{
		lp->SetModified(false);
		msg = _T("Saved layer to file as ") + lp->GetLayerFilename();
	}
	else
	{
		msg = _T("Failed to save layer to ") + lp->GetLayerFilename();
		wxMessageBox(msg, _T("Problem"));
	}
	SetStatusText(msg);

	g_Log._Log(msg.mb_str());
	g_Log._Log("\n");
}

void MainFrame::OnUpdateLayerSaveAs(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp != NULL && lp->CanBeSaved());
}

void MainFrame::OnUpdateLayerProperties(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnLayerImport(wxCommandEvent &event)
{
	LayerType lt;

#ifdef ELEVATION_ONLY
	lt = LT_ELEVATION;
#else
	// first ask what kind of data layer
	lt = AskLayerType();
	if (lt == LT_UNKNOWN)
		return;
#endif
	ImportData(lt);
}

void MainFrame::OnLayerImportTIGER(wxCommandEvent &event)
{
	// ask the user for a directory
	wxDirDialog getDir(NULL, _T("Import TIGER Data From Directory"));
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strDirName = getDir.GetPath();

	ImportDataFromTIGER(strDirName);
}

void MainFrame::OnLayerImportUtil(wxCommandEvent &event)
{
	// ask the user for a directory
	wxDirDialog getDir(NULL, _T("Import Utility Data from Directory of SHP Files"));
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString2 strDirName = getDir.GetPath();

//	dlg.m_strCaption = _T("Shapefiles do not contain projection information.  ")
//		_T("Please indicate the projection of this file:");
	// ask user for a projection
	Projection2Dlg dlg(NULL, -1, _T("Indicate Projection"), wxDefaultPosition);
	dlg.SetProjection(m_proj);

	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	vtProjection proj;
	dlg.GetProjection(proj);

	// create the new layers
	vtUtilityLayer *pUL = new vtUtilityLayer;
	if (pUL->ImportFromSHP(strDirName.mb_str(), proj))
	{
		pUL->SetLayerFilename(strDirName);
		pUL->SetModified(true);

		if (!AddLayerWithCheck(pUL, true))
			delete pUL;
	}
	else
		delete pUL;
}

void MainFrame::OnLayerImportMapSource(wxCommandEvent &event)
{
	// Use file dialog to open plant list text file.
	wxFileDialog loadFile(NULL, _T("Import MapSource File"), _T(""), _T(""),
		_T("MapSource Export Files (*.txt)|*.txt|"), wxOPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString2 str = loadFile.GetPath();

	FILE *fp = fopen(str.mb_str(), "r");
	if (!fp)
		return;

	vtRawLayer *pRL;
	Array<vtRawLayer *> layers;
	char buf[200];
	bool bUTM = false;
	fscanf(fp, "Grid %s\n", buf);
	if (!strcmp(buf, "UTM"))
		bUTM = true;
	fgets(buf, 200, fp); // assume "Datum   WGS 84" 

	vtProjection proj;

	bool bGotSRS = false;
	char ch;
	int i;

	while (fgets(buf, 200, fp))	// get a line
	{
		if (!strncmp(buf, "Track\t", 6))
		{
			pRL = new vtRawLayer();
			pRL->SetEntityType(SHPT_POINT);
			layers.Append(pRL);
			bGotSRS = false;

			// parse name
			char name[40];
			for (i = 6; ; i++)
			{
				ch = buf[i];
				if (ch == '\t' || ch == 0)
					break;
				name[i-6] = ch;
			}
			name[i] = 0;
			pRL->SetFilename(vtString(name));
		}
		if (!strncmp(buf, "Trackpoint", 10))
		{
			int zone;
			DPoint2 p;
			sscanf(buf+10, "%d %c %lf %lf", &zone, &ch, &p.x, &p.y);

			if (!bGotSRS)
			{
				proj.SetWellKnownGeogCS("WGS84");
				if (bUTM)
					proj.SetUTMZone(zone);
				pRL->SetProjection(proj);
				bGotSRS = true;
			}
			pRL->AddPoint(p);
		}
	}

	// Display the list of imported tracks to the user
	int n = layers.GetSize();
	wxString2 *choices = new wxString2[n];
	wxArrayInt selections;
	for (i = 0; i < n; i++)
	{
		choices[i] = layers[i]->GetFilename();

		choices[i] += _T(" (");
		if (bUTM)
		{
			layers[i]->GetProjection(proj);
			str.Printf(_T("zone %d, "), proj.GetUTMZone());
			choices[i] += str;
		}
		str.Printf(_T("points %d"), layers[i]->NumEntities());
		choices[i] += str;
		choices[i] += _T(")");
	}

	int nsel = wxGetMultipleChoices(selections, _T("Which layers to import?"),
		_T("Import Tracks"), n, choices);

	// for each of the layers the user wants, add them to our project
	for (i = 0; i < nsel; i++)
	{
		int sel = selections[i];
		AddLayerWithCheck(layers[sel]);
		layers[sel]->SetModified(false);
	}
	// for all the rest, delete 'em
	for (i = 0; i < n; i++)
	{
		if (layers[i]->GetModified())
			delete layers[i];
	}
	delete [] choices;
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
	title += _T(" Layer Properties");
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
			pEL->m_pGrid->SetEarthExtents(rect2);
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

void MainFrame::OnAreaExportImage(wxCommandEvent &event)
{
	ExportImage();
}

void MainFrame::OnUpdateAreaExportImage(wxUpdateUIEvent& event)
{
	event.Enable(LayersOfType(LT_IMAGE) > 0 && !m_area.IsEmpty());
}

void MainFrame::OnLayerConvert(wxCommandEvent &event)
{
	// ask for what projection to convert to
	Projection2Dlg dlg(NULL, 200, _T("Convert to what projection?"), wxDefaultPosition);
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
		if (layers == 1)
			DisplayAndLog("Failed to convert.");
		else
			DisplayAndLog("Failed to convert %d of %d layers.",
				layers-succeeded, layers);
	}

	SetProjection(proj);
	m_pView->ZoomAll();
	RefreshStatusBar();
}

void MainFrame::OnLayerSetProjection(wxCommandEvent &event)
{
	// Allow the user to directly specify the projection for all loaded
	// layers (override it, without reprojecting the layer's data)
	// ask for what projection to convert to
	Projection2Dlg dlg(NULL, -1, _T("Set to what projection?"), wxDefaultPosition);
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
	RefreshStatusBar();
}

void MainFrame::OnUpdateLayerConvert(wxUpdateUIEvent& event)
{
	event.Enable(m_Layers.GetSize() != 0);
}

void MainFrame::OnLayerFlatten(wxCommandEvent &event)
{
	vtLayer *pActive = GetActiveLayer();
	LayerType t = pActive->GetType();

	int layers_merged = 0;

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
			layers_merged++;
		}
	}

	if (layers_merged > 0)
	{
		wxString newname = _T("untitled");
		newname += pActive->GetFileExtension();
		pActive->SetLayerFilename(newname);
		pActive->SetModified(true);
	}
}

void MainFrame::OnUpdateLayerFlatten(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp &&
			(lp->GetType() == LT_ROAD ||
			 lp->GetType() == LT_VEG ||
			 lp->GetType() == LT_WATER ||
			 lp->GetType() == LT_STRUCTURE ||
			 lp->GetType() == LT_RAW));
}

////////////////////////////////////////////////////////////
// View menu

void MainFrame::OnLayerShow(wxCommandEvent &event)
{
	vtLayer *pLayer = GetActiveLayer();
	if (!pLayer)
		return;
	pLayer->SetVisible(!pLayer->GetVisible());

	DRECT r;
	pLayer->GetExtent(r);
	wxRect sr = m_pView->WorldToWindow(r);
	IncreaseRect(sr, 5);
	m_pView->Refresh(TRUE, &sr);
}

void MainFrame::OnUpdateLayerShow(wxUpdateUIEvent& event)
{
	vtLayer *pLayer = GetActiveLayer();

	event.Enable(pLayer != NULL);
	event.Check(pLayer && pLayer->GetVisible());
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

void MainFrame::OnViewSetArea(wxCommandEvent& event)
{
	m_pView->SetMode(LB_Box);
}

void MainFrame::OnUpdateViewSetArea(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_Box);
}

void MainFrame::OnViewZoomIn(wxCommandEvent &event)
{
	m_pView->SetScale(m_pView->GetScale() * sqrt(2.0));
}

void MainFrame::OnViewZoomOut(wxCommandEvent &event)
{
	m_pView->SetScale(m_pView->GetScale() / sqrt(2.0));
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
	vtImageLayer *pIL = GetActiveImageLayer();
	if (pIL)
		m_pView->MatchZoomToImage(pIL);
}

void MainFrame::OnUpdateViewFull(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp &&
			(lp->GetType() == LT_ELEVATION || lp->GetType() == LT_IMAGE));
}

void MainFrame::OnViewWorldMap(wxUpdateUIEvent& event)
{
	m_pView->SetShowMap(!m_pView->GetShowMap());
	m_pView->Refresh();
}

void MainFrame::OnUpdateWorldMap(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetShowMap());
}

void MainFrame::OnViewUTMBounds(wxUpdateUIEvent& event)
{
	m_pView->m_bShowUTMBounds = !m_pView->m_bShowUTMBounds;
	m_pView->Refresh();
}

void MainFrame::OnUpdateUTMBounds(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bShowUTMBounds);
}

void MainFrame::OnViewOptions(wxUpdateUIEvent& event)
{
	OptionsDlg dlg(this, -1, _T("Options"));

	dlg.m_bShowMinutes = m_statbar->m_bShowMinutes;
	dlg.m_bShowPath = m_pTree->GetShowPaths();
	dlg.m_bShowToolbar = toolBar_main->IsShown();
	dlg.m_iElevUnits = (int)(m_statbar->m_ShowVertUnits) - 1;

	if (dlg.ShowModal() == wxID_OK)
	{
		m_statbar->m_bShowMinutes = dlg.m_bShowMinutes;
		m_statbar->m_ShowVertUnits = (LinearUnits) (dlg.m_iElevUnits + 1);

		if (dlg.m_bShowToolbar != toolBar_main->IsShown())
		{
			toolBar_main->Show(dlg.m_bShowToolbar);
			// send a fake OnSize event so the frame will draw itself correctly
			wxSizeEvent dummy;
			wxFrame::OnSize(dummy);
		}
		if (dlg.m_bShowPath != m_pTree->GetShowPaths())
		{
			m_pTree->SetShowPaths(dlg.m_bShowPath);
			m_pTree->RefreshTreeItems(this);
		}
	}
}


//////////////////////////
// Road

void MainFrame::OnSelectLink(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Link);
}

void MainFrame::OnUpdateSelectLink(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Link );
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
	m_pView->SetMode(LB_LinkExtend);
}

void MainFrame::OnUpdateSelectWhole(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_LinkExtend );
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
	m_pView->SetMode(LB_LinkEdit);
}

void MainFrame::OnUpdateRoadEdit(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_LinkEdit );
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

	wxTextEntryDialog dlg(this, _T("Please enter highway number"), _T("Select Highway"), _T(""));
	if (dlg.ShowModal() == wxID_OK)
	{
		int num;
		wxString2 str = dlg.GetValue();
		sscanf(str.mb_str(), "%d", &num);
		if (pRL->SelectHwyNum(num))
			m_pView->Refresh();
	}
}

void MainFrame::OnRoadClean(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	// check projection
	vtProjection proj;
	pRL->GetProjection(proj);
	bool bDegrees = (proj.IsGeographic() != 0);

	int count;
	OpenProgressDialog(_T("Cleaning RoadMap"));

	UpdateProgressDialog(10, _T("Removing unused nodes"));
	count = pRL->RemoveUnusedNodes();
	if (count)
	{
		DisplayAndLog("Removed %i nodes", count);
	}

	UpdateProgressDialog(20, _T("Merging redundant nodes"));
	// potentially takes a long time...
	count = pRL->MergeRedundantNodes(bDegrees, progress_callback);
	if (count)
	{
		DisplayAndLog("Merged %d redundant roads", count);
	}

	UpdateProgressDialog(50, _T("Cleaning link points"));
	count = pRL->CleanLinkPoints();
	if (count)
	{
		DisplayAndLog("Cleaned %d link points", count);
	}

#if 0
	// The following cleanup operations are disabled until they are proven safe!

	UpdateProgressDialog(30, _T("Removing degenerate links"));
	count = pRL->RemoveDegenerateRoads();
	if (count)
	{
		DisplayAndLog("Removed %d degenerate links", count);
	}

	UpdateProgressDialog(40, _T("Removing unnecessary nodes"));
	count = pRL->RemoveUnnecessaryNodes();
	if (count)
	{
		DisplayAndLog("Removed %d unnecessary nodes", count);
	}

	UpdateProgressDialog(60, _T("Removing dangling links"));
	count = pRL->DeleteDanglingRoads();
	if (count)
	{
		DisplayAndLog("Removed %i dangling links", count);
	}

	UpdateProgressDialog(70, _T("Fixing overlapped roads"));
	count = pRL->FixOverlappedRoads(bDegrees);
	if (count)
	{
		DisplayAndLog("Fixed %i overlapped roads", count);
	}

	UpdateProgressDialog(80, _T("Fixing extraneous parallels"));
	count = pRL->FixExtraneousParallels();
	if (count)
	{
		DisplayAndLog("Fixed %i extraneous parallels", count);
	}

	UpdateProgressDialog(90, _T("Splitting looping roads"));
	count = pRL->SplitLoopingRoads();
	if (count)
	{
		DisplayAndLog("Split %d looping roads", count);
	}
#endif

	CloseProgressDialog();
	pRL->SetModified(true);
	pRL->ComputeExtents();

	m_pView->Refresh();
}

void MainFrame::OnRoadGuess(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	// Set visual properties
	pRL->GuessIntersectionTypes();

	for (NodeEdit *pN = pRL->GetFirstNode(); pN; pN = pN->GetNext())
		pN->DetermineVisualFromLinks();

	m_pView->Refresh();
}

void MainFrame::OnRoadFlatten(wxCommandEvent &event)
{
	float margin = 2.0;
	wxString2 str;
	str.Printf(_("%g"), margin);
	str = wxGetTextFromUser(_T("How many meters for the margin at the edge of each road?"),
		_T("Flatten elevation grid under roads"), str, this);
	if (str == _T(""))
		return;

	margin = atof(str.mb_str());

	vtRoadLayer *pR = (vtRoadLayer *)GetMainFrame()->FindLayerOfType(LT_ROAD);
	vtElevLayer *pE = (vtElevLayer *)GetMainFrame()->FindLayerOfType(LT_ELEVATION);

	pR->CarveRoadway(pE, margin);
	m_pView->Refresh();
}

void MainFrame::OnUpdateRoadFlatten(wxUpdateUIEvent& event)
{
	vtElevLayer *pE = (vtElevLayer *)GetMainFrame()->FindLayerOfType(LT_ELEVATION);

	event.Enable(pE != NULL && pE->m_pGrid != NULL);
}


//////////////////////////
// Elevation

void MainFrame::OnUpdateIsGrid(wxUpdateUIEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	event.Enable(pEL && pEL->IsGrid());
}

void MainFrame::OnElevSelect(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TSelect);
}

void MainFrame::OnUpdateElevSelect(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_TSelect);
}

void MainFrame::OnRemoveElevRange(wxCommandEvent &event)
{
	vtElevLayer *t = GetActiveElevLayer();
	if (!t && !t->m_pGrid)
		return;

	DRECT area;
	t->GetExtent(area);
	DPoint2 step = t->m_pGrid->GetSpacing();

	wxString2 str;
	str = wxGetTextFromUser(_T("Please specify the elevation range\n")
		_T("(minimum and maximum in the form \"X Y\")\n")
		_T("All values within this range (and within the area\n")
		_T("tool, if it is defined) will be set to Unknown."));

	float zmin, zmax;
	const char *text = str.mb_str();
	if (sscanf(text, "%f %f", &zmin, &zmax) != 2)
	{
		wxMessageBox(_T("Didn't get two numbers."));
		return;
	}

	bool bUseArea = !m_area.IsEmpty();

	vtElevationGrid *grid = t->m_pGrid;
	int iColumns, iRows;
	grid->GetDimensions(iColumns, iRows);
	float val;
	DPoint2 p;
	int i, j;

	for (i = 0; i < iColumns; i++)
	{
		for (j = 0; j < iRows; j++)
		{
			if (bUseArea)
			{
				p.x = area.left + (i * step.x);
				p.y = area.bottom + (j * step.y);
				if (!m_area.ContainsPoint(p))
					continue;
			}

			val = grid->GetFValue(i, j);
			if (val >= zmin && val <= zmax)
				grid->SetFValue(i, j, INVALID_ELEVATION);
		}
	}
	t->ReRender();

	m_pView->Refresh();
}

void MainFrame::OnElevSetUnknown(wxCommandEvent &event)
{
	vtElevLayer *t = GetActiveElevLayer();
	if (!t)	return;
	vtElevationGrid *grid = t->m_pGrid;

	static float fValue = 1.0f;
	wxString2 str;
	str.Printf(_("%g"), fValue);
	str = wxGetTextFromUser(_T("Set unknown areas to what value?"),
		_T("Set Unknown Areas"), str, this);
	if (str == _T(""))
		return;

	fValue = atof(str.mb_str());

	// If the Area tool defines an area, restrict ourselves to use it
	bool bUseArea = !m_area.IsEmpty();

	int iColumns, iRows;
	DRECT area;
	DPoint2 p, step;
	grid->GetDimensions(iColumns, iRows);
	t->GetExtent(area);
	step = grid->GetSpacing();

	for (int i = 0; i < iColumns; i++)
	{
		p.x = area.left + (i * step.x);
		for (int j = 0; j < iRows; j++)
		{
			p.y = area.bottom + (j * step.y);
			if (bUseArea)
			{
				if (!m_area.ContainsPoint(p))
					continue;
			}
			if (grid->GetFValue(i, j) == INVALID_ELEVATION)
				grid->SetFValue(i, j, fValue);
		}
	}
	t->ReRender();
	m_pView->Refresh();
}

void MainFrame::OnFillIn(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	el->FillGaps();
	el->ReRender();
	m_pView->Refresh();
}

void MainFrame::OnScaleElevation(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	if (!el)
		return;
	vtElevationGrid *grid = el->m_pGrid;
	if (!grid)
		return;

	wxString2 str = wxGetTextFromUser(_T("Please enter a scale factor"),
		_T("Scale Elevation"), _T("1.0"), this);
	if (str == _T(""))
		return;

	float fScale;
	fScale = atof(str.mb_str());
	if (fScale == 0.0f)
	{
		wxMessageBox(_T("Couldn't parse the number you typed."));
		return;
	}
	if (fScale == 1.0f)
		return;

	grid->Scale(fScale, true);
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

	wxString filter = _T("All Files|*.*|");
	AddType(filter, FSTRING_TER);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _T("Export Elevation"), _T(""), _T(""), filter, wxSAVE);
	if (saveFile.ShowModal() != wxID_OK)
		return;
	wxString2 strPathName = saveFile.GetPath();

	bool success = el->m_pGrid->SaveToTerragen(strPathName.mb_str());
	if (success)
		DisplayAndLog("Successfully wrote TerraGen file '%s'", strPathName.mb_str());
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::OnExportGeoTIFF(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	if (!el)
		return;

	wxString filter = _T("All Files|*.*|");
	AddType(filter, FSTRING_TIF);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _T("Export Elevation"), _T(""), _T(""), filter, wxSAVE);
	if (saveFile.ShowModal() != wxID_OK)
		return;
	wxString2 strPathName = saveFile.GetPath();

	bool success = el->m_pGrid->SaveToGeoTIFF(strPathName.mb_str());
	if (success)
		DisplayAndLog("Successfully wrote GeoTIFF file '%s'", strPathName.mb_str());
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::OnExportBMP(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	if (!el)
		return;

	wxString filter = _T("All Files|*.*|");
	AddType(filter, FSTRING_BMP);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _T("Export Elevation"), _T(""), _T(""), filter, wxSAVE);
	if (saveFile.ShowModal() != wxID_OK)
		return;
	wxString2 strPathName = saveFile.GetPath();

	bool success = el->m_pGrid->SaveToBMP(strPathName.mb_str());
	if (success)
		DisplayAndLog("Successfully wrote BMP file '%s'", strPathName.mb_str());
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::OnElevExportBitmap(wxCommandEvent& event)
{
	int size = 0;
	while (size < 32 || size > 8192)
	{
		wxTextEntryDialog dlg(this, _T("Please enter pixel size of bitmap"),
			_T("Export Bitmap"), _T(""));
		if (dlg.ShowModal() != wxID_OK)
			return;

		wxString2 str = dlg.GetValue();
		sscanf(str.mb_str(), "%d", &size);
	}

	// Ask for file name
	wxFileDialog loadFile(NULL, _T("Output filename for bitmap"), _T(""), _T(""),
		_T("Bitmap Files (*.bmp)|*.bmp|"), wxSAVE|wxOVERWRITE_PROMPT);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	OpenProgressDialog(_T("Generating Bitmap"));

	wxString2 fname = loadFile.GetPath();
	vtDIB dib;
	dib.Create(size, size, 24, false);

	vtElevLayer *pEL = GetActiveElevLayer();

	pEL->PaintDibFromElevation(&dib, true);

	// TEST - try coloring from water polygons
	int layers = m_Layers.GetSize();
	for (int l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_WATER)
			((vtWaterLayer*)lp)->PaintDibWithWater(&dib);
	}

	UpdateProgressDialog(100, _T("Writing bitmap to file."));
	bool success = dib.WriteBMP(fname.mb_str());
	CloseProgressDialog();
}

void MainFrame::OnElevMergeTin(wxCommandEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	pEL->MergeSharedVerts();
}

void MainFrame::OnUpdateElevMergeTin(wxUpdateUIEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	event.Enable(pEL && !pEL->IsGrid());
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
	ExtentDlg dlg(NULL, -1, _T("Edit Area"), wxDefaultPosition);
	dlg.SetArea(m_area, (m_proj.IsGeographic() != 0));
	if (dlg.ShowModal() == wxID_OK)
	{
		m_area = dlg.m_area;
		m_pView->Refresh();
	}
}

void MainFrame::OnAreaRequestLayer(wxCommandEvent& event)
{
#if SUPPORT_HTTP
	bool success;

	wxTextEntryDialog dlg(this, _T("WFS Server address"),
		_T("Please enter server base URL"), _T("http://10.254.0.29:8081/"));
	if (dlg.ShowModal() != wxID_OK)
		return;
	wxString2 value = dlg.GetValue();
	const char *server = value.mb_str();

	WFSLayerArray layers;
	success = GetLayersFromWFS(server, layers);

	int numlayers = layers.size();
	wxString choices[100];
	for (int i = 0; i < numlayers; i++)
		choices[i] = wxString::FromAscii(layers[i]->GetValue("Name"));

	wxSingleChoiceDialog dlg2(this, _T("Choice Layer"),
		_T("Please indicate layer:"), numlayers, (const wxString *)choices);

	if (dlg2.ShowModal() != wxID_OK)
		return;

	vtRawLayer *pRL = new vtRawLayer();
	success = pRL->ReadFeaturesFromWFS(server, "rail");
	if (success)
		AddLayerWithCheck(pRL);
	else
		delete pRL;
#endif
}

void MainFrame::OnAreaRequestWMS(wxCommandEvent& event)
{
#if SUPPORT_HTTP

	MapServerDlg dlg(this, -1, _T("WMS Request"));

	dlg.m_area = m_area;

	if (dlg.ShowModal() != wxID_OK)
		return;
	wxString2 query = dlg.m_query;

/*	bool success;
	success = GetLayersFromWMS(query);

	vtImageLayer *pIL = new vtImageLayer();
	success = pRL->ReadFeaturesFromWFS(server, "rail");
	if (success)
		AddLayerWithCheck(pRL);
	else
		delete pRL;
*/
#endif
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

	if (!LayersOfType(LT_ELEVATION))
		return;

	// refresh the display
	for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
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
	for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
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
	if (m_strPlantListFilename == "")
	{
		// Use file dialog to open plant list text file.
		wxFileDialog loadFile(NULL, _T("Load Plant Info"), _T(""), _T(""),
			PLANTS_FILTER, wxOPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		wxString2 str = loadFile.GetPath();
		if (!LoadPlantFile(str.mb_str()))
			return;
	}
	if (!m_SpeciesListDlg)
	{
		// Create new Plant List Dialog
		m_SpeciesListDlg = new SpeciesListDlg(this, WID_PLANTS, _T("Plants List"), 
				wxPoint(140, 100), wxSize(950, 400), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	}

	// Display plant list data, calling OnInitDialog.
	m_SpeciesListDlg->Show(true);
}


void MainFrame::OnVegBioregions(wxCommandEvent& event)
{
	// if data isn't there, get the data first
	if (!m_BioRegionDlg)
	{
		// Use file dialog to open bioregion text file.
		wxFileDialog loadFile(NULL, _T("Load BioRegion Info"), _T(""), _T(""),
			BIOREGIONS_FILTER, wxOPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		// Read bioregions, data kept on frame with m_pBioRegion.
		wxString2 str = loadFile.GetPath();
		if (!m_BioRegions.Read(str.mb_str()))
		{
			DisplayAndLog("Couldn't read bioregion list from that file.");
			return;
		}

		// Create new Bioregion Dialog
		m_BioRegionDlg = new BioRegionDlg(this, WID_BIOREGIONS, _T("BioRegions List"), 
				wxPoint(120, 80), wxSize(300, 500), wxSYSTEM_MENU | wxCAPTION);
	}

	// Display bioregion data, calling OnInitDialog.
	m_BioRegionDlg->Show(true);
}

void MainFrame::OnVegExportSHP(wxCommandEvent& event)
{
	vtVegLayer *pVeg = GetMainFrame()->GetActiveVegLayer();
	if (!pVeg) return;

	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _T("Export vegetation to SHP"), _T(""), _T(""),
		_T("Vegetation Files (*.shp)|*.shp|"), wxSAVE | wxOVERWRITE_PROMPT);
	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString2 strPathName = saveFile.GetPath();

	pVeg->ExportToSHP(strPathName.mb_str());
}

void MainFrame::OnUpdateVegExportSHP(wxUpdateUIEvent& event)
{
	vtVegLayer *pVeg = GetMainFrame()->GetActiveVegLayer();
	event.Enable(pVeg && pVeg->IsNative());
}

void MainFrame::OnAreaGenerateVeg(wxCommandEvent& event)
{
	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _T("Save Vegetation File"), _T(""), _T(""),
		_T("Vegetation Files (*.vf)|*.vf|"), wxSAVE | wxOVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString2 strPathName = saveFile.GetPath();

	DistribVegDlg dlg(this, -1, _T("Vegetation Distribution Options"));
	dlg.m_fSampling = 40.0f;
	dlg.m_fScarcity = 0.001f;
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	// Generate the plants
	vtVegLayer *Density = NULL, *BioMap = NULL;
	FindVegLayers(&Density, &BioMap);

	float fTreeSpacing = dlg.m_fSampling;
	float fTreeScarcity = dlg.m_fScarcity;

	GenerateVegetation(strPathName.mb_str(), m_area, Density, BioMap, fTreeSpacing,
		fTreeScarcity);
}

void MainFrame::OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event)
{
	vtVegLayer *Density = NULL, *BioMap = NULL;

	FindVegLayers(&Density, &BioMap);

	// density is now optional, defaults to 1 if there is no density layer
	event.Enable(m_SpeciesListDlg && m_BioRegionDlg && BioMap &&
			!m_area.IsEmpty());
}


//////////////////////////////
// Utilities Menu

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
}
void MainFrame::OnUpdateTowerEdit(wxUpdateUIEvent &event)
{
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

void MainFrame::OnFeaturePick(wxCommandEvent &event)
{
	m_pView->SetMode(LB_FeatInfo);
}

void MainFrame::OnFeatureTable(wxCommandEvent &event)
{
	if (m_pFeatInfoDlg && m_pFeatInfoDlg->IsShown())
		m_pFeatInfoDlg->Show(false);
	else
	{
		ShowFeatInfoDlg();
		m_pFeatInfoDlg->SetFeatureSet(GetActiveRawLayer());
	}
}

void MainFrame::OnUpdateFeaturePick(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_FeatInfo);
}

void MainFrame::OnUpdateFeatureTable(wxUpdateUIEvent& event)
{
	event.Check(m_pFeatInfoDlg && m_pFeatInfoDlg->IsShown());
}

void MainFrame::OnBuildingEdit(wxCommandEvent &event)
{
	m_pView->SetMode(LB_BldEdit);
}


void MainFrame::OnUpdateBuildingEdit(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_BldEdit);
}

void MainFrame::OnBuildingAddPoints(wxCommandEvent &event)
{
	m_pView->SetMode(LB_BldAddPoints);
}

void MainFrame::OnUpdateBuildingAddPoints(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_BldAddPoints);
}

void MainFrame::OnBuildingDeletePoints(wxCommandEvent &event)
{
	m_pView->SetMode(LB_BldDeletePoints);
}

void MainFrame::OnUpdateBuildingDeletePoints(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_BldDeletePoints);
}

void MainFrame::OnStructureAddLinear(wxCommandEvent &event)
{
	m_pView->SetMode(LB_AddLinear);
}

void MainFrame::OnUpdateStructureAddLinear(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_AddLinear);
}

void MainFrame::OnStructureEditLinear(wxCommandEvent &event)
{
	m_pView->SetMode(LB_EditLinear);
}

void MainFrame::OnUpdateStructureEditLinear(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_EditLinear);
}

void MainFrame::OnStructureAddFoundation(wxCommandEvent &event)
{
	vtStructureLayer *pSL = GetActiveStructureLayer();
	vtElevLayer *pEL = (vtElevLayer *) FindLayerOfType(LT_ELEVATION);
	pSL->AddFoundations(pEL);
}

void MainFrame::OnUpdateStructureAddFoundation(wxUpdateUIEvent& event)
{
	vtStructureLayer *pSL = GetActiveStructureLayer();
	vtElevLayer *pEL = (vtElevLayer *) FindLayerOfType(LT_ELEVATION);
	event.Enable(pSL != NULL && pEL != NULL);
}

void MainFrame::OnStructureConstrain(wxCommandEvent &event)
{
	m_pView->m_bConstrain = !m_pView->m_bConstrain;
}

void MainFrame::OnUpdateStructureConstrain(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bConstrain);
}


///////////////////////////////////
// Raw menu

void MainFrame::OnRawSetType(wxCommandEvent& event)
{
	static int types[4] = { SHPT_NULL, SHPT_POINT, SHPT_ARC, SHPT_POLYGON };
	wxString choices[4];
	for (int i = 0; i < 4; i++)
		choices[i] = wxString::FromAscii(SHPTypeName(types[i]));

	int n = 4;
	int cur_type = 0;

	wxSingleChoiceDialog dialog(this, _T("These are your choices"),
		_T("Please indicate entity type:"), n, (const wxString *)choices);

	dialog.SetSelection(cur_type);

	if (dialog.ShowModal() == wxID_OK)
	{
		cur_type = dialog.GetSelection();
		vtRawLayer *pRL = (vtRawLayer *) GetActiveLayer();
		pRL->SetEntityType(types[cur_type]);
	}
}

void MainFrame::OnUpdateRawSetType(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetEntityType() == SHPT_NULL);
}

void MainFrame::OnRawAddPoints(wxCommandEvent& event)
{
	m_pView->SetMode(LB_AddPoints);
}

void MainFrame::OnUpdateRawAddPoints(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetEntityType() == SHPT_POINT);
	event.Check(m_pView->GetMode() == LB_AddPoints);
}

void MainFrame::OnRawAddPointText(wxCommandEvent& event)
{
	wxString2 str = wxGetTextFromUser(_T("(X, Y) in current projection"),
			_T("Enter coordinate"));
	if (str == _T(""))
		return;
	double x, y;
	int num = sscanf(str.mb_str(), "%lf, %lf", &x, &y);
	if (num != 2)
		return;
	DPoint2 p(x, y);
	vtRawLayer *pRL = GetActiveRawLayer();
	pRL->AddPoint(p);
	pRL->SetModified(true);
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
//	vtRawLayer *pRL = GetActiveRawLayer();
//	event.Enable(pRL != NULL && pRL->GetEntityType() == SHPT_POINT);
	event.Enable(false);	// not implemented yet
}

void MainFrame::OnRawSelectCondition(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();

	if (pRL->GetNumFields() == 0)
	{
		DisplayAndLog("Can't select by condition because the current\n"
					  "layer has no fields defined.");
		return;
	}
	SelectDlg dlg(this, -1, _T("Select"));
	dlg.SetRawLayer(pRL);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString2 str = dlg.m_strValue;
		int selected = pRL->SelectByCondition(dlg.m_iField, dlg.m_iCondition,
			str.mb_str());

		wxString2 msg;
		if (selected == -1)
			msg = _T("Unable to select");
		else
			msg.Printf(_T("Selected %d entit%hs"), selected, selected == 1 ? "y" : "ies");
		SetStatusText(msg);

		msg += _T("\n");
		g_Log._Log(msg.mb_str());

		m_pView->Refresh(false);
		OnSelectionChanged();
	}
}

void MainFrame::OnUpdateRawSelectCondition(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL);
}


////////////////////
// Help

void MainFrame::OnHelpAbout(wxCommandEvent &event)
{
#ifdef ELEVATION_ONLY
	wxString str = _T("Elevation Tool\n\n");
	str += _T("Build date: ");
	str += _(__DATE__);
	wxMessageBox(str, _T("About ElevTool"));
#else
	wxString str = _T("Virtual Terrain Builder\nPowerful, easy to use, free!\n\n");
	str += _T("Please read the HTML documentation and license.\n\n");
	str += _T("Send feedback to: ben@vterrain.org\n");
	str += _T("Build date: ");
	str += _(__DATE__);
	wxMessageBox(str, _T("About ") _T(APPNAME));
#endif
}
