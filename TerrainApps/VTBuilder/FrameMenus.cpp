//
//  The menus functions of the main Frame window of the VTBuilder application.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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
#include "vtdata/GEOnet.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtdata/TripDub.h"
#include "vtdata/WFSClient.h"

#include "Frame.h"
#include "MenuEnum.h"
#include "BuilderView.h"
#include "TreeView.h"
#include "Helper.h"
// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
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
#include "GeocodeDlg.h"
#include "ImageMapDlg.h"
#include "LayerPropDlg.h"
#include "MapServerDlg.h"
#include "OptionsDlg.h"
#include "Projection2Dlg.h"
#include "RenderDlg.h"
#include "SelectDlg.h"
#include "TSDlg.h"
#include "VegDlg.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_FILE_NEW,		MainFrame::OnProjectNew)
EVT_MENU(ID_FILE_OPEN,		MainFrame::OnProjectOpen)
EVT_MENU(ID_FILE_SAVE,		MainFrame::OnProjectSave)
EVT_MENU(ID_SPECIAL_DYMAX_TEXTURES,	MainFrame::OnDymaxTexture)
EVT_MENU(ID_SPECIAL_PROCESS_BILLBOARD,	MainFrame::OnProcessBillboard)
EVT_MENU(ID_SPECIAL_GEOCODE,	MainFrame::OnGeocode)
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
EVT_MENU(ID_LAYER_IMPORTNTF,	MainFrame::OnLayerImportNTF)
EVT_MENU(ID_LAYER_IMPORTUTIL,	MainFrame::OnLayerImportUtil)
EVT_MENU(ID_LAYER_IMPORT_MS,	MainFrame::OnLayerImportMapSource)
EVT_MENU(ID_LAYER_IMPORT_POINT,	MainFrame::OnLayerImportPoint)
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
EVT_MENU(ID_VIEW_LAYER_UP,		MainFrame::OnLayerUp)
EVT_MENU(ID_VIEW_MAGNIFIER,		MainFrame::OnViewMagnifier)
EVT_MENU(ID_VIEW_PAN,			MainFrame::OnViewPan)
EVT_MENU(ID_VIEW_DISTANCE,		MainFrame::OnViewDistance)
EVT_MENU(ID_VIEW_ZOOMIN,		MainFrame::OnViewZoomIn)
EVT_MENU(ID_VIEW_ZOOMOUT,		MainFrame::OnViewZoomOut)
EVT_MENU(ID_VIEW_ZOOMALL,		MainFrame::OnViewZoomAll)
EVT_MENU(ID_VIEW_ZOOM_LAYER,	MainFrame::OnViewZoomToLayer)
EVT_MENU(ID_VIEW_FULLVIEW,		MainFrame::OnViewFull)
EVT_MENU(ID_VIEW_SETAREA,		MainFrame::OnViewSetArea)
EVT_MENU(ID_VIEW_WORLDMAP,		MainFrame::OnViewWorldMap)
EVT_MENU(ID_VIEW_SHOWUTM,		MainFrame::OnViewUTMBounds)
EVT_MENU(ID_VIEW_OPTIONS,		MainFrame::OnViewOptions)

EVT_UPDATE_UI(ID_VIEW_SHOWLAYER,	MainFrame::OnUpdateLayerShow)
EVT_UPDATE_UI(ID_VIEW_LAYER_UP,		MainFrame::OnUpdateLayerUp)
EVT_UPDATE_UI(ID_VIEW_MAGNIFIER,	MainFrame::OnUpdateMagnifier)
EVT_UPDATE_UI(ID_VIEW_PAN,			MainFrame::OnUpdatePan)
EVT_UPDATE_UI(ID_VIEW_DISTANCE,		MainFrame::OnUpdateDistance)
EVT_UPDATE_UI(ID_VIEW_ZOOM_LAYER,	MainFrame::OnUpdateViewZoomToLayer)
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
EVT_UPDATE_UI(ID_ROAD_FLATTEN,		MainFrame::OnUpdateRoadFlatten)

EVT_MENU(ID_ELEV_SELECT,			MainFrame::OnElevSelect)
EVT_MENU(ID_ELEV_REMOVERANGE,		MainFrame::OnRemoveElevRange)
EVT_MENU(ID_ELEV_SETUNKNOWN,		MainFrame::OnElevSetUnknown)
EVT_MENU(ID_ELEV_FILLIN,			MainFrame::OnFillIn)
EVT_MENU(ID_ELEV_SCALE,				MainFrame::OnScaleElevation)
EVT_MENU(ID_ELEV_EXPORT,			MainFrame::OnElevExport)
EVT_MENU(ID_ELEV_BITMAP,			MainFrame::OnElevExportBitmap)
EVT_MENU(ID_ELEV_MERGETIN,			MainFrame::OnElevMergeTin)

EVT_UPDATE_UI(ID_ELEV_SELECT,		MainFrame::OnUpdateElevSelect)
EVT_UPDATE_UI(ID_ELEV_REMOVERANGE,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SETUNKNOWN,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_FILLIN,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SCALE,		MainFrame::OnUpdateScaleElevation)
EVT_UPDATE_UI(ID_ELEV_EXPORT,		MainFrame::OnUpdateIsGrid)
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
EVT_MENU(ID_VEG_REMAP,				MainFrame::OnVegRemap)
EVT_MENU(ID_VEG_EXPORTSHP,			MainFrame::OnVegExportSHP)

EVT_UPDATE_UI(ID_VEG_REMAP,			MainFrame::OnUpdateVegExportSHP)
EVT_UPDATE_UI(ID_VEG_EXPORTSHP,		MainFrame::OnUpdateVegExportSHP)

EVT_MENU(ID_FEATURE_SELECT,			MainFrame::OnFeatureSelect)
EVT_MENU(ID_FEATURE_PICK,			MainFrame::OnFeaturePick)
EVT_MENU(ID_FEATURE_TABLE,			MainFrame::OnFeatureTable)
EVT_MENU(ID_STRUCTURE_EDIT_BLD,		MainFrame::OnBuildingEdit)
EVT_MENU(ID_STRUCTURE_ADD_POINTS,	MainFrame::OnBuildingAddPoints)
EVT_MENU(ID_STRUCTURE_DELETE_POINTS, MainFrame::OnBuildingDeletePoints)
EVT_MENU(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnStructureAddLinear)
EVT_MENU(ID_STRUCTURE_EDIT_LINEAR,	MainFrame::OnStructureEditLinear)
EVT_MENU(ID_STRUCTURE_ADD_INST,		MainFrame::OnStructureAddInstances)
EVT_MENU(ID_STRUCTURE_ADD_FOUNDATION, MainFrame::OnStructureAddFoundation)
EVT_MENU(ID_STRUCTURE_CONSTRAIN,	MainFrame::OnStructureConstrain)

EVT_UPDATE_UI(ID_FEATURE_SELECT,		MainFrame::OnUpdateFeatureSelect)
EVT_UPDATE_UI(ID_FEATURE_PICK,			MainFrame::OnUpdateFeaturePick)
EVT_UPDATE_UI(ID_FEATURE_TABLE,			MainFrame::OnUpdateFeatureTable)
EVT_UPDATE_UI(ID_STRUCTURE_EDIT_BLD,	MainFrame::OnUpdateBuildingEdit)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_POINTS,	MainFrame::OnUpdateBuildingAddPoints)
EVT_UPDATE_UI(ID_STRUCTURE_DELETE_POINTS,	MainFrame::OnUpdateBuildingDeletePoints)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnUpdateStructureAddLinear)
EVT_UPDATE_UI(ID_STRUCTURE_EDIT_LINEAR,	MainFrame::OnUpdateStructureEditLinear)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_INST,	MainFrame::OnUpdateStructureAddInstances)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_FOUNDATION,	MainFrame::OnUpdateStructureAddFoundation)
EVT_UPDATE_UI(ID_STRUCTURE_CONSTRAIN,	MainFrame::OnUpdateStructureConstrain)

EVT_MENU(ID_RAW_SETTYPE,			MainFrame::OnRawSetType)
EVT_MENU(ID_RAW_ADDPOINTS,			MainFrame::OnRawAddPoints)
EVT_MENU(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnRawAddPointText)
EVT_MENU(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnRawAddPointsGPS)
EVT_MENU(ID_RAW_SELECTCONDITION,	MainFrame::OnRawSelectCondition)
EVT_MENU(ID_RAW_EXPORT_IMAGEMAP,	MainFrame::OnRawExportImageMap)
EVT_MENU(ID_RAW_STYLE,				MainFrame::OnRawStyle)

EVT_UPDATE_UI(ID_RAW_SETTYPE,			MainFrame::OnUpdateRawSetType)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS,			MainFrame::OnUpdateRawAddPoints)
EVT_UPDATE_UI(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnUpdateRawAddPointText)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnUpdateRawAddPointsGPS)
EVT_UPDATE_UI(ID_RAW_SELECTCONDITION,	MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_EXPORT_IMAGEMAP,	MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_STYLE,				MainFrame::OnUpdateRawIsActive)

EVT_MENU(ID_AREA_ZOOM_ALL,			MainFrame::OnAreaZoomAll)
EVT_MENU(ID_AREA_ZOOM_LAYER,		MainFrame::OnAreaZoomLayer)
EVT_MENU(ID_AREA_TYPEIN,			MainFrame::OnAreaTypeIn)
EVT_MENU(ID_AREA_EXPORT_ELEV,		MainFrame::OnAreaExportElev)
EVT_MENU(ID_AREA_EXPORT_IMAGE,		MainFrame::OnAreaExportImage)
EVT_MENU(ID_AREA_GENERATE_VEG,		MainFrame::OnAreaGenerateVeg)
EVT_MENU(ID_AREA_REQUEST_LAYER,		MainFrame::OnAreaRequestLayer)
EVT_MENU(ID_AREA_REQUEST_WMS,		MainFrame::OnAreaRequestWMS)
EVT_MENU(ID_AREA_REQUEST_TSERVE,	MainFrame::OnAreaRequestTServe)

EVT_UPDATE_UI(ID_AREA_ZOOM_ALL,		MainFrame::OnUpdateAreaZoomAll)
EVT_UPDATE_UI(ID_AREA_ZOOM_LAYER,	MainFrame::OnUpdateAreaZoomLayer)
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
	fileMenu->Append(ID_FILE_NEW, _("&New\tCtrl+N"), _("New Project"));
	fileMenu->Append(ID_FILE_OPEN, _("Open Project\tCtrl+O"), _("Open Project"));
	fileMenu->Append(ID_FILE_SAVE, _("Save Project\tCtrl+S"), _("Save Project As"));
#ifndef ELEVATION_ONLY
	fileMenu->AppendSeparator();
	wxMenu *specialMenu = new wxMenu;
	specialMenu->Append(ID_SPECIAL_DYMAX_TEXTURES, _("Create Dymaxion Textures"));
	specialMenu->Append(ID_SPECIAL_PROCESS_BILLBOARD, _("Process Billboard Texture"));
	specialMenu->Append(ID_SPECIAL_GEOCODE, _("Geocode"));
	fileMenu->Append(0, _("Special"), specialMenu);
#endif
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_FILE_EXIT, _("E&xit\tAlt-X"), _("Exit"));
	m_pMenuBar->Append(fileMenu, _("&Project"));
	menu_num++;

 	// Edit
	editMenu = new wxMenu;
	editMenu->Append(ID_EDIT_DELETE, _("Delete\tDEL"), _("Delete"));
	editMenu->AppendSeparator();
	editMenu->Append(ID_EDIT_DESELECTALL, _("Deselect All"), _("Clears selection"));
	editMenu->Append(ID_EDIT_INVERTSELECTION, _("Invert Selection"));
#ifndef ELEVATION_ONLY
	editMenu->AppendCheckItem(ID_EDIT_CROSSINGSELECTION, _("Crossing Selection"));
#endif
	m_pMenuBar->Append(editMenu, _("&Edit"));
	menu_num++;

	// Layer
	layerMenu = new wxMenu;
#ifndef ELEVATION_ONLY
	layerMenu->Append(ID_LAYER_NEW, _("&New Layer"), _("Create New Layer"));
#endif
	layerMenu->Append(ID_LAYER_OPEN, _("Open Layer"), _("Open Existing Layer"));
	layerMenu->Append(ID_LAYER_SAVE, _("Save Layer"), _("Save Active Layer"));
	layerMenu->Append(ID_LAYER_SAVE_AS, _("Save Layer As..."), _("Save Active Layer As"));
	layerMenu->Append(ID_LAYER_IMPORT, _("Import Data\tCtrl+I"), _("Import Data"));
#ifndef ELEVATION_ONLY
	layerMenu->Append(ID_LAYER_IMPORTTIGER, _("Import Data From TIGER"), _("Import Data From TIGER"));
	layerMenu->Append(ID_LAYER_IMPORTNTF, _("Import Data From NTF"), _("Import Data From TIGER"));
	layerMenu->Append(ID_LAYER_IMPORTUTIL, _("Import Utilites From SHP"), _("Import Utilites From SHP"));
	layerMenu->Append(ID_LAYER_IMPORT_MS, _("Import From MapSource File"));
	layerMenu->Append(ID_LAYER_IMPORT_POINT, _("Import Point Data From Table"));
#endif
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_PROPS, _("Layer Properties"), _("Layer Properties"));
	layerMenu->Append(ID_EDIT_OFFSET, _("Offset Coordinates"), _("Offset"));
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_FLATTEN, _("&Flatten Layers"), _("Flatten"));
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_CONVERTPROJ, _("Convert Projection"), _("Convert"));
	layerMenu->Append(ID_LAYER_SETPROJ, _("Set Projection"), _("Set Projection"));
	m_pMenuBar->Append(layerMenu, _("&Layer"));
	menu_num++;

	// View
	viewMenu = new wxMenu;
	viewMenu->AppendCheckItem(ID_VIEW_SHOWLAYER, _("Current Layer &Visible"),
		_("Toggle Visibility of the current Layer"));
	viewMenu->Append(ID_VIEW_LAYER_UP, _("Move Layer &Up"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_ZOOMIN, _("Zoom &In\tCtrl++"));
	viewMenu->Append(ID_VIEW_ZOOMOUT, _("Zoom Out\tCtrl+-"));
	viewMenu->Append(ID_VIEW_ZOOMALL, _("Zoom &All"));
	viewMenu->Append(ID_VIEW_ZOOM_LAYER, _("Zoom to Current &Layer"));
	viewMenu->Append(ID_VIEW_FULLVIEW, _("Zoom to &Full Res (1:1)"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_MAGNIFIER, _("&Magnifier\tZ"));
	viewMenu->AppendCheckItem(ID_VIEW_PAN, _("&Pan\tSPACE"));
	viewMenu->AppendCheckItem(ID_VIEW_DISTANCE, _("Obtain &Distance"));
	viewMenu->AppendCheckItem(ID_VIEW_SETAREA, _("Area &Tool"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_WORLDMAP, _("&World Map"), _("Show/Hide World Map"));
	viewMenu->AppendCheckItem(ID_VIEW_SHOWUTM, _("Show &UTM Boundaries"));
//	viewMenu->AppendCheckItem(ID_VIEW_SHOWGRID, _("Show 7.5\" Grid"), _("Show 7.5\" Grid"), true);
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_OPTIONS, _("&Options"));
	m_pMenuBar->Append(viewMenu, _("&View"));
	menu_num++;

#ifndef ELEVATION_ONLY
	// Roads
	roadMenu = new wxMenu;
	roadMenu->AppendCheckItem(ID_ROAD_SELECTROAD, _("Select/Modify Roads"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTNODE, _("Select/Modify Nodes"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTWHOLE, _("Select Whole Roads"));
	roadMenu->AppendCheckItem(ID_ROAD_DIRECTION, _("Set Road Direction"));
	roadMenu->AppendCheckItem(ID_ROAD_EDIT, _("Edit Road Points"));
	roadMenu->AppendSeparator();
	roadMenu->AppendCheckItem(ID_ROAD_SHOWNODES, _("Show Nodes"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTHWY, _("Select by Highway Number"));
	roadMenu->AppendSeparator();
	roadMenu->Append(ID_ROAD_CLEAN, _("Clean RoadMap"), _("Clean"));
	roadMenu->Append(ID_ROAD_GUESS, _("Guess Intersection Types"));
	roadMenu->Append(ID_ROAD_FLATTEN, _("Flatten Elevation Grid Under Roads"));
	m_pMenuBar->Append(roadMenu, _("&Roads"));
	m_iLayerMenu[LT_ROAD] = menu_num;
	menu_num++;

	// Utilities
	utilityMenu = new wxMenu;
	utilityMenu->AppendCheckItem(ID_TOWER_ADD, _("Add a Transmission Tower"));
	utilityMenu->AppendSeparator();
	utilityMenu->AppendCheckItem(ID_TOWER_SELECT, _("Select Utility Layer"));
	utilityMenu->AppendCheckItem(ID_TOWER_EDIT, _("Edit Transmission Towers"));
	m_pMenuBar->Append(utilityMenu, _("Util&ities"));
	m_iLayerMenu[LT_UTILITY] = menu_num;
	menu_num++;
#endif

	// Elevation
	elevMenu = new wxMenu;
	elevMenu->AppendCheckItem(ID_ELEV_SELECT, _("Se&lect Elevation Layer"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_SCALE, _("Sc&ale Elevation"));
	elevMenu->Append(ID_ELEV_REMOVERANGE, _("&Remove Elevation Range..."));
	elevMenu->Append(ID_ELEV_FILLIN, _("&Fill In Unknown Areas"));
	elevMenu->Append(ID_ELEV_SETUNKNOWN, _("&Set Unknown Areas"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_EXPORT, _("E&xport To..."));
	elevMenu->Append(ID_ELEV_BITMAP, _("Re&nder to Bitmap..."));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_MERGETIN, _("&Merge shared TIN vertices"));
	m_pMenuBar->Append(elevMenu, _("Elev&ation"));
	m_iLayerMenu[LT_ELEVATION] = menu_num;
	menu_num++;

#ifndef ELEVATION_ONLY
	// Vegetation
	vegMenu = new wxMenu;
	vegMenu->Append(ID_VEG_PLANTS, _("Species List"), _("View/Edit list of available plant species"));
	vegMenu->Append(ID_VEG_BIOREGIONS, _("BioRegions"), _("View/Edit list of species & density for each BioRegion"));
	vegMenu->AppendSeparator();
	vegMenu->Append(ID_VEG_REMAP, _("Remap Species"));
	vegMenu->Append(ID_VEG_EXPORTSHP, _("Export SHP"));
	m_pMenuBar->Append(vegMenu, _("Veg&etation"));
	m_iLayerMenu[LT_VEG] = menu_num;
	menu_num++;

	// Structures
	bldMenu = new wxMenu;
	bldMenu->AppendCheckItem(ID_FEATURE_SELECT, _("Select Features"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_EDIT_BLD, _("Edit Buildings"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_POINTS, _("Add points to building footprints"), _T(""));
	bldMenu->AppendCheckItem(ID_STRUCTURE_DELETE_POINTS, _("Delete points from building footprints"), _T(""));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_LINEAR, _("Add Linear Structures"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_EDIT_LINEAR, _("Edit Linear Structures"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_INST, _("Add Instances"));
	bldMenu->AppendSeparator();
	bldMenu->Append(ID_STRUCTURE_ADD_FOUNDATION, _("Add Foundation Levels to Buildings"), _T(""));
	bldMenu->AppendSeparator();
	bldMenu->AppendCheckItem(ID_STRUCTURE_CONSTRAIN, _("Constrain angles on footprint edit"));
	m_pMenuBar->Append(bldMenu, _("&Structures"));
	m_iLayerMenu[LT_STRUCTURE] = menu_num;
	menu_num++;
#endif

	// Raw
	rawMenu = new wxMenu;
	rawMenu->AppendCheckItem(ID_FEATURE_SELECT, _("Select Features"));
	rawMenu->AppendCheckItem(ID_FEATURE_PICK, _("Pick Features"));
	rawMenu->AppendCheckItem(ID_FEATURE_TABLE, _("Show Attribute Table"));
#ifndef ELEVATION_ONLY
	rawMenu->AppendSeparator();
	rawMenu->Append(ID_RAW_SETTYPE, _("Set Entity Type"), _("Set Entity Type"));
	rawMenu->AppendCheckItem(ID_RAW_ADDPOINTS, _("Add Points with Mouse"));
	rawMenu->Append(ID_RAW_ADDPOINT_TEXT, _("Add Point with Text\tCtrl+T"), _("Add point"));
	rawMenu->Append(ID_RAW_ADDPOINTS_GPS, _("Add Points with GPS"), _("Add points with GPS"));
	rawMenu->Append(ID_RAW_STYLE, _("Style..."));
#endif
	rawMenu->AppendSeparator();
	rawMenu->Append(ID_RAW_SELECTCONDITION, _("Select Features by Condition"));
	rawMenu->Append(ID_RAW_EXPORT_IMAGEMAP, _("Export as HTML ImageMap"));
	m_pMenuBar->Append(rawMenu, _("Ra&w"));
	m_iLayerMenu[LT_RAW] = menu_num;
	menu_num++;

	// Area
	areaMenu = new wxMenu;
	areaMenu->Append(ID_AREA_ZOOM_ALL, _("Set to Full Extents"),
		_("Set the Area Tool rectangle to the combined extent of all layers."));
	areaMenu->Append(ID_AREA_ZOOM_LAYER, _("Set to Layer Extents"),
		_("Set the Area Tool rectangle to the extent of the active layer."));
	areaMenu->Append(ID_AREA_TYPEIN, _("Numeric Values"),
		_("Set the Area Tool rectangle by text entry of coordinates."));
	areaMenu->AppendSeparator();
	areaMenu->Append(ID_AREA_EXPORT_ELEV, _("&Merge && Resample Elevation"),
		_("Sample all elevation data within the Area Tool to produce a single, new elevation."));
#ifndef ELEVATION_ONLY
	areaMenu->Append(ID_AREA_EXPORT_IMAGE, _("Extract && Export Image"),
		_("Sample imagery within the Area Tool to produce a single, new image."));
	areaMenu->Append(ID_AREA_GENERATE_VEG, _("Generate Vegetation"),
		_("Generate Vegetation File (*.vf) containg plant distribution."));
#if SUPPORT_HTTP
	areaMenu->Append(ID_AREA_REQUEST_LAYER, _("Request Layer from WFS"));
	areaMenu->Append(ID_AREA_REQUEST_WMS, _("Request Image from WMS"));
	areaMenu->Append(ID_AREA_REQUEST_TSERVE, _("Request Image from Terraserver"));
#endif // SUPPORT_HTTP
#endif
	m_pMenuBar->Append(areaMenu, _("&Area Tool"));
	menu_num++;

	// Help
	helpMenu = new wxMenu;
	wxString2 msg = _("About ");
	msg += _T(APPNAME);
	helpMenu->Append(wxID_HELP, _("&About"), msg);
	m_pMenuBar->Append(helpMenu, _("&Help"));
	menu_num++;

	SetMenuBar(m_pMenuBar);
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

	// reset veg too
	m_strSpeciesFilename = "";
	m_strBiotypesFilename = "";
	m_PlantList.Clear();
	m_BioRegion.Clear();

	RefreshTreeView();
	RefreshToolbar();

	vtProjection p;
	SetProjection(p);
}

wxString2 GetProjectFilter()
{
	wxString2 str = _T(APPNAME);
	str += _T(" ");
	str += _("Project Files (*.vtb)|*.vtb");
	return str;
}

void MainFrame::OnProjectOpen(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Load Project"), _T(""), _T(""),
		GetProjectFilter(), wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	LoadProject(loadFile.GetPath());
}

void MainFrame::OnProjectSave(wxCommandEvent &event)
{
	wxFileDialog saveFile(NULL, _("Save Project"), _T(""), _T(""),
		GetProjectFilter(), wxSAVE | wxOVERWRITE_PROMPT );
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

	wxFileDialog dlg(this, _("Choose input file"), _T(""), _T(""), _T("*.bmp;*.png"));
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString choices[6];
	choices[0] = _T("128");
	choices[1] = _T("256");
	choices[2] = _T("512");
	choices[3] = _T("1024");
	choices[4] = _T("2048");
	choices[5] = _T("4096");
	wxSingleChoiceDialog dlg2(this, _("Size of each output tile?"),
		_("Query"), 6, choices);
	if (dlg2.ShowModal() == wxID_CANCEL)
		return;
	int sel = dlg2.GetSelection();
	int output_size = 1 << (7+sel);

	wxTextEntryDialog dlg3(this, _("Prefix for output filenames?"), _("Query"));
	if (dlg3.ShowModal() == wxID_CANCEL)
		return;
	wxString prefix = dlg3.GetValue();

	wxImage::AddHandler(new wxPNGHandler);

	wxProgressDialog prog(_("Processing"), _("Loading source bitmap.."), 100);
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

	wxImage out[10];
	for (i = 0; i < 10; i++)
	{
		out[i].Create(output_size, output_size);

		wxString msg;
		msg.Printf(_("Creating tile %d ..."), i+1);
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
		name.Printf(_T("%s_%02d%02d.png"), prefix.c_str(),
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
							 const RGBi &bg, bool progress_callback(int) = NULL)
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

bool FindGeoPointInBuffer(const char *buffer, DPoint2 &p)
{
	char *lon = strstr(buffer, "<geo:long>");
	char *lat = strstr(buffer, "<geo:lat>");
	if (lon && lat)
	{
		sscanf(lon+10, "%lf", &p.x);
		sscanf(lat+9, "%lf", &p.y);
		return true;
	}
	return false;
}

bool FindWithGeocoderUS(ReqContext &webcontext,
		const vtString &strStreet, const vtString &strCity,
		const vtString &strState, DPoint2 &p)
{
	bool bFound = false;

	// Must have street address; a city or zipcode won't suffice.
	if (strStreet == "")
		return false;

	if (!strStreet.Left(6).CompareNoCase("po box") ||
		!strStreet.Left(8).CompareNoCase("p.o. box"))
	{
		// forget it, geocoder.us doesn't do PO boxes
		return false;
	}
	vtString url;
	vtString result;
	int offset, chop;

	// Clean up the street address for geocoder.us
	vtString street = strStreet;

	// First, is this a multi-line street address?  If so, it's likely that only
	//  one of the lines is the proper street that TIGER knows.
	offset = street.Find('\n');
	if (offset != -1)
	{
		// chop it and examine each part
		vtString part1 = street.Left(offset);
		vtString part2 = street.Mid(offset+1);
		if (isdigit(part1[0]))
			street = part1;
		else if (isdigit(part2[0]))
			street = part2;
		else
		{
			int foo = 1;
		}
	}

	static const char *snames[21] = {
		"street ", "st. ", "avenue ", "ave. ",
		"drive ", "drive, ", "dr. ", " dr ", "dr., ", 
		"boulevard ", "blvd. ", "bl ", "blvd, ",
		"place ", "pl. ",
		"road ", "rd. ", "road, ", "rd., ",
		"way ", "lane "};

	// Look for apartment numbers, suite etc. (they usually follow
	//  the "St." designation) and get rid of them.
	vtString lower = street;
	lower.MakeLower();
	int n;
	for (n = 0; n < (sizeof(snames)/sizeof(char*)); n++)
	{
		offset = lower.Find(snames[n]);
		if (offset != -1)
			break;
	}
	if (offset != -1)
	{
		chop = offset + strlen(snames[n]) - 1;
		street = street.Left(chop);
	}

	url = "http://rpc.geocoder.us/service/rest?address=";
	if (street != "")
	{
		url += street.FormatForURL();
		url += "+";
	}
	if (strCity != "")
	{
		url += strCity.FormatForURL();
		url += "+";
	}
	if (strState != "")
	{
		url += strState.FormatForURL();
	}
	if (webcontext.GetURL(url, result))
	{
		bFound = FindGeoPointInBuffer(result, p);
	}
	return bFound;
}

void MainFrame::OnGeocode(wxCommandEvent &event)
{
	GeocodeDlg dlg(this, -1, _("Geocode"));
	dlg.m_bGeocodeUS = false;
	dlg.m_bGazetteer = false;
	dlg.m_bGNS = false;
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString fname = dlg.m_strData;
	bool success;
	vtFeatureSetPoint2D feat;
	const DPoint2 zero(0,0);

	wxString shpname;
	if (!fname.Right(3).CmpNoCase(_T("shp")))
	{
		shpname = fname;
		OpenProgressDialog(_T("Reading"), false);
		success = feat.LoadFromSHP(shpname.mb_str(), progress_callback);
		CloseProgressDialog();
	}
	else if (!fname.Right(3).CmpNoCase(_T("dbf")))
	{
		DBFHandle hDBF = DBFOpen(fname.mb_str(), "rb");
		if (!hDBF)
			return;
		int iDBFRecords = DBFGetRecordCount(hDBF);
		DBFClose(hDBF);

		vtFeatureSetPoint2D newfeat;
		newfeat.SetNumEntities(iDBFRecords);
		for (int i = 0; i < iDBFRecords; i++)
			newfeat.SetPoint(i, zero);

		shpname = fname.Left(fname.Length()-3);
		shpname += _T("shp");
		newfeat.SaveToSHP(shpname.mb_str());

		// now re-open
		success = feat.LoadFromSHP(shpname.mb_str());
	}
	else if (!fname.Right(3).CmpNoCase(_T("csv")))
	{
//		feat.ImportDataFromCSV(fname.mb_str());
	}

	if (!success)
		return;
	int iRecords = feat.GetNumEntities();
	vtString strStreet;
	vtString strCity;
	vtString strState;
	vtString strCode;
	vtString strCountry;

	int rec, iKnown = 0, iFound = 0; // How many are already known
	DPoint2 p;

	if (dlg.m_bGeocodeUS)
	{
	}

	// Used for geocode.us requests (when available)
	ReqContext webcontext;

	// Used by the Census Bureau Gazetteer
	Gazetteer gaz;
	bool bHaveGaz = false;
	bool bHaveZip = false;
	if (dlg.m_bGazetteer)
	{
		bHaveGaz = gaz.ReadPlaces(dlg.m_strGaz.mb_str());
		bHaveZip = gaz.ReadZips(dlg.m_strZip.mb_str());
	}

	// Use by the GEOnet Name Server (GNS) data files
	Countries countries;
	bool bHaveGNS = false;
	if (dlg.m_bGNS)
	{
		OpenProgressDialog(_T("Reading GNS file..."), false);
		bHaveGNS = countries.ReadGCF(dlg.m_strGNS.mb_str(), progress_callback);
		CloseProgressDialog();
	}

	OpenProgressDialog(_T("Geocoding"), true);
	bool bFound;
	for (rec = 0; rec < iRecords; rec++)
	{
		if (progress_callback(rec*100/iRecords))
			break;

		feat.GetPoint(rec, p);
		if (p != zero)
		{
			iKnown++;
			continue;
		}

		bFound = false;

		feat.GetValueAsString(rec, 7, strStreet);
		feat.GetValueAsString(rec, 8, strCity);
		feat.GetValueAsString(rec, 9, strState);
		feat.GetValueAsString(rec, 10, strCode);
		feat.GetValueAsString(rec, 11, strCountry);

		// Try geocode.us first; it has the most detail
		if (!bFound && dlg.m_bGeocodeUS &&
			!strCountry.CompareNoCase("United States of America"))
		{
			bFound = FindWithGeocoderUS(webcontext, strStreet, strCity,
				strState, p);
		}

		// Then (for US addresses) the gazetteer can look up a point for a
		//  zip code or city.
		if (!bFound && dlg.m_bGazetteer &&
			!strCountry.CompareNoCase("United States of America"))
		{
			// USA: Use zip code, if we have it
			if (bHaveZip && strCode != "")
			{
				// We only use the 5-digit code
				vtString five = strCode.Left(5);
				int zip = atoi(five);
				bFound = gaz.FindZip(zip, p);
			}
			if (!bFound && bHaveGaz)
			{
				// Use city/place name
				bFound = gaz.FindPlace(strState, strCity, p);
			}
		}

		// Then (for International addresses) GNS can get a point for a city
		if (!bFound && bHaveGNS &&
			strCountry.CompareNoCase("United States of America") != 0)
		{
			bFound = countries.FindPlaceWithGuess(strCountry, strCity, p);
		}

		if (bFound)
		{
			feat.SetPoint(rec, p);
			iFound++;
		}
	}
	CloseProgressDialog();

	wxString str;
	str.Printf(_T("%d records, %d already known\n%d/%d resolved, %d remain unknown"),
		iRecords, iKnown, iFound, iRecords-iKnown, iRecords-iKnown-iFound);
	wxMessageBox(str, _T("Results"));

	if (iFound != 0)
	{
		// Save to SHP
		wxFileDialog saveFile(NULL, _T("Save to SHP"), _T(""), _T(""),
			_T("SHP Files (*.shp)|*.shp"), wxSAVE);
		if (saveFile.ShowModal() == wxID_OK)
		{
			shpname = saveFile.GetPath();
			OpenProgressDialog(_T("Saving"), false);
			feat.SaveToSHP(shpname.mb_str(), progress_callback);
			CloseProgressDialog();
		}
	}

	if (bHaveGNS)
	{
		OpenProgressDialog(_T("Freeing GNS data..."), false);
		countries.Free(progress_callback);
		CloseProgressDialog();
	}
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
		str.Printf(_("Deleting road selection: %d nodes and %d roads"),
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
	if (pRawL)
	{
		vtFeatureSet *pSet = pRawL->GetFeatureSet();
		if (pSet && pSet->NumSelected() != 0)
		{
			pSet->DeleteSelected();
			pRawL->SetModified(true);
			m_pView->Refresh();
			OnSelectionChanged();
			return;
		}
	}

	vtLayer *pL = GetActiveLayer();
	if (pL)
	{
		int result = wxMessageBox(_("Are you sure you want to delete the current layer?"),
				_("Question"), wxYES_NO | wxICON_QUESTION, this);
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
		pRawL->GetFeatureSet()->InvertSelection();
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
	wxTextEntryDialog dlg(this, _("Offset"),
		_("Please enter horizontal offset X, Y"), _T("0, 0"));
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
	LayerType lt = AskLayerType();
	if (lt == LT_UNKNOWN)
		return;

	vtLayer *pL = vtLayer::CreateNewLayer(lt);
	if (!pL)
		return;

	if (lt == LT_ELEVATION)
	{
		vtElevLayer *pEL = (vtElevLayer *)pL;
		pEL->m_pGrid = new vtElevationGrid(m_area, 1025, 1025, false, m_proj);
		pEL->m_pGrid->FillWithSingleValue(1000);
	}
	else
	{
		pL->SetProjection(m_proj);
	}

	SetActiveLayer(pL);
	m_pView->SetActiveLayer(pL);
	AddLayer(pL);
	m_pTree->RefreshTreeItems(this);
	RefreshToolbar();
}

void MainFrame::OnLayerOpen(wxCommandEvent &event)
{
	wxString filter = _("Native Layer Formats|");

	AddType(filter, FSTRING_BT);	// elevation
	AddType(filter, FSTRING_BTGZ);	// compressed elevation
	AddType(filter, FSTRING_TIN);	// elevation
#ifndef ELEVATION_ONLY
	AddType(filter, FSTRING_RMF);	// roads
	AddType(filter, FSTRING_GML);	// raw
	AddType(filter, FSTRING_UTL);	// utility towers
	AddType(filter, FSTRING_VTST);	// structures
	AddType(filter, FSTRING_VTSTGZ);// compressed structures
	AddType(filter, FSTRING_VF);	// vegetation files
	AddType(filter, FSTRING_TIF);	// image files
	AddType(filter, FSTRING_IMG);	// image or elevation file
#endif
	AddType(filter, FSTRING_SHP);	// raw files

	// ask the user for a filename
	wxFileDialog loadFile(NULL, _("Open Layer"), _T(""), _T(""), filter, wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	LoadLayer(loadFile.GetPath());
}

void MainFrame::OnLayerSave(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	if (lp->GetLayerFilename().Left(8).CmpNoCase(_("Untitled")) == 0)
	{
		if (!lp->AskForSaveFilename())
			return;
	}
	wxString2 msg = _("Saving layer to file ") + lp->GetLayerFilename();
	SetStatusText(msg);
	VTLOG(msg.mb_str());
	VTLOG("\n");

	if (lp->Save())
		msg = _("Saved layer to file ") + lp->GetLayerFilename();
	else
		msg = _("Save failed.");
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

	wxString2 msg = _("Saving layer to file as ") + lp->GetLayerFilename();
	SetStatusText(msg);

	g_Log._Log(msg.mb_str());
	g_Log._Log("\n");

	bool success = lp->Save();
	if (success)
	{
		lp->SetModified(false);
		msg = _("Saved layer to file as ") + lp->GetLayerFilename();
	}
	else
	{
		msg = _("Failed to save layer to ") + lp->GetLayerFilename();
		wxMessageBox(msg, _("Problem"));
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
	wxDirDialog getDir(NULL, _("Import TIGER Data From Directory"));
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strDirName = getDir.GetPath();

	ImportDataFromTIGER(strDirName);
}

void MainFrame::OnLayerImportNTF(wxCommandEvent &event)
{
	// Use file dialog to open plant list text file.
	wxFileDialog loadFile(NULL, _("Import Layers from NTF File"), _T(""), _T(""),
		_("NTF Files (*.ntf)|*.ntf|"), wxOPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString2 str = loadFile.GetPath();
	ImportDataFromNTF(str);
}

void MainFrame::OnLayerImportUtil(wxCommandEvent &event)
{
	// ask the user for a directory
	wxDirDialog getDir(NULL, _("Import Utility Data from Directory of SHP Files"));
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString2 strDirName = getDir.GetPath();

//	dlg.m_strCaption = _T("Shapefiles do not contain projection information.  ")
//		_T("Please indicate the projection of this file:");
	// ask user for a projection
	Projection2Dlg dlg(NULL, -1, _("Indicate Projection"));
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

//
// Import from a Garmin MapSource GPS export file (.txt)
//
void MainFrame::OnLayerImportMapSource(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import MapSource File"), _T(""), _T(""),
		_("MapSource Export Files (*.txt)|*.txt|"), wxOPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString2 str = loadFile.GetPath();
	ImportFromMapSource(str.mb_str());
}

void MainFrame::OnLayerImportPoint(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import Point Data"), _T(""), _T(""),
		_("Tabular Data Files (*.dbf)|*.dbf|"), wxOPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString2 str = loadFile.GetPath();
	ImportDataPointsFromTable(str.mb_str());
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
	title += vtLayer::LayerTypeNames[ltype];
	title += _(" Layer Properties");
	LayerPropDlg dlg(NULL, -1, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	// Fill in initial values for the dialog
	DRECT rect, rect2;
	lp->GetExtent(rect);
	dlg.m_fLeft = rect.left;
	dlg.m_fTop = rect.top;
	dlg.m_fRight = rect.right;
	dlg.m_fBottom = rect.bottom;

	lp->GetPropertyText(dlg.m_strText);

	// For elevation layers, if the user changes the extents, apply.
	if (dlg.ShowModal() != wxID_OK)
		return;

	rect2.left = dlg.m_fLeft;
	rect2.top = dlg.m_fTop;
	rect2.right = dlg.m_fRight;
	rect2.bottom = dlg.m_fBottom;
	if (rect2 != rect)
	{
		// user changed the extents
		if (lp->SetExtent(rect2))
		{
			wxMessageBox(_("Changed extents."));
			m_pView->Refresh();
		}
		else
			wxMessageBox(_("Could not change extents."));
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
	Projection2Dlg dlg(NULL, 200, _("Convert to what projection?"));
	dlg.SetProjection(m_proj);

	// might switch to utm, help provide a good guess for UTM zone
	DPoint2 pos = EstimateGeoDataCenter();
	dlg.SetGeoRefPoint(pos);

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
		bool success = lp->TransformCoords(proj);
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
	ZoomAll();
	RefreshStatusBar();
}

void MainFrame::OnLayerSetProjection(wxCommandEvent &event)
{
	// Allow the user to directly specify the projection for all loaded
	// layers (override it, without reprojecting the layer's data)
	// ask for what projection to convert to
	Projection2Dlg dlg(NULL, -1, _("Set to what projection?"));
	dlg.SetProjection(m_proj);

	// might switch to utm, help provide a good guess for UTM zone
	DPoint2 pos = EstimateGeoDataCenter();
	dlg.SetGeoRefPoint(pos);

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
	ZoomAll();
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
		wxString newname = _("Untitled");
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

void MainFrame::OnLayerUp(wxCommandEvent &event)
{
	vtLayer *pLayer = GetActiveLayer();
	if (!pLayer)
		return;
	int num = LayerNum(pLayer);
	if (num < NumLayers()-1)
		SwapLayerOrder(num, num+1);

	DRECT r;
	pLayer->GetExtent(r);
	wxRect sr = m_pView->WorldToWindow(r);
	IncreaseRect(sr, 5);
	m_pView->Refresh(TRUE, &sr);
}

void MainFrame::OnUpdateLayerUp(wxUpdateUIEvent& event)
{
	vtLayer *pLayer = GetActiveLayer();

	event.Enable(pLayer != NULL && LayerNum(pLayer) < NumLayers()-1);
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
	RefreshStatusBar();
}

void MainFrame::OnViewZoomOut(wxCommandEvent &event)
{
	m_pView->SetScale(m_pView->GetScale() / sqrt(2.0));
	RefreshStatusBar();
}

void MainFrame::OnViewZoomAll(wxCommandEvent &event)
{
	ZoomAll();
}

void MainFrame::OnViewZoomToLayer(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	DRECT rect;
	if (lp->GetExtent(rect))
		m_pView->ZoomToRect(rect, 0.1f);
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

void MainFrame::OnUpdateViewZoomToLayer(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
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
	OptionsDlg dlg(this, -1, _("Options"));

	dlg.m_bShowToolbar = toolBar_main->IsShown();
	dlg.m_bShowMinutes = m_statbar->m_bShowMinutes;
	dlg.m_iElevUnits = (int)(m_statbar->m_ShowVertUnits) - 1;

	dlg.SetElevDrawOptions(vtElevLayer::m_draw);

	dlg.m_bShowRoadWidth = vtRoadLayer::GetDrawWidth();
	dlg.m_bShowPath = m_pTree->GetShowPaths();

	if (dlg.ShowModal() != wxID_OK)
		return;

	bool bNeedRefresh = false;

	if (dlg.m_bShowToolbar != toolBar_main->IsShown())
	{
		toolBar_main->Show(dlg.m_bShowToolbar);
		// send a fake OnSize event so the frame will draw itself correctly
		wxSizeEvent dummy;
		wxFrame::OnSize(dummy);
	}
	m_statbar->m_bShowMinutes = dlg.m_bShowMinutes;
	m_statbar->m_ShowVertUnits = (LinearUnits) (dlg.m_iElevUnits + 1);

	ElevDrawOptions opt;
	dlg.GetElevDrawOptions(opt);

	if (vtElevLayer::m_draw != opt)
	{
		vtElevLayer::m_draw = opt;

		// tell them to redraw themselves
		for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
		{
			vtLayer *lp = m_Layers.GetAt(i);
			if (lp->GetType() == LT_ELEVATION)
			{
				vtElevLayer *elp = (vtElevLayer *)lp;
				elp->ReRender();
				bNeedRefresh = true;
			}
		}
	}

	bool bWidth = dlg.m_bShowRoadWidth;
	if (vtRoadLayer::GetDrawWidth() != bWidth && LayersOfType(LT_ROAD) > 0)
		bNeedRefresh = true;
	vtRoadLayer::SetDrawWidth(bWidth);

	if (dlg.m_bShowPath != m_pTree->GetShowPaths())
	{
		m_pTree->SetShowPaths(dlg.m_bShowPath);
		m_pTree->RefreshTreeItems(this);
	}

	if (bNeedRefresh)
		m_pView->Refresh();
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

void MainFrame::OnSelectHwy(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	wxTextEntryDialog dlg(this, _("Please enter highway number"),
		_("Select Highway"), _T(""));
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
	OpenProgressDialog(_("Cleaning RoadMap"));

	UpdateProgressDialog(10, _("Removing unused nodes"));
	count = pRL->RemoveUnusedNodes();
	if (count)
	{
		DisplayAndLog("Removed %i nodes", count);
		pRL->SetModified(true);
	}

	UpdateProgressDialog(20, _("Merging redundant nodes"));
	// potentially takes a long time...
	count = pRL->MergeRedundantNodes(bDegrees, progress_callback);
	if (count)
	{
		DisplayAndLog("Merged %d redundant roads", count);
		pRL->SetModified(true);
	}

	UpdateProgressDialog(30, _("Cleaning link points"));
	count = pRL->CleanLinkPoints();
	if (count)
	{
		DisplayAndLog("Cleaned %d link points", count);
		pRL->SetModified(true);
	}

	UpdateProgressDialog(40, _T("Removing degenerate links"));
	count = pRL->RemoveDegenerateLinks();
	if (count)
	{
		DisplayAndLog("Removed %d degenerate links", count);
		pRL->SetModified(true);
	}

#if 0
	// The following cleanup operations are disabled until they are proven safe!

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
	str = wxGetTextFromUser(_("How many meters for the margin at the edge of each road?"),
		_("Flatten elevation grid under roads"), str, this);
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
	str = wxGetTextFromUser(_("Please specify the elevation range\n(minimum and maximum in the form \"X Y\")\nAll values within this range (and within the area\ntool, if it is defined) will be set to Unknown."));

	float zmin, zmax;
	const char *text = str.mb_str();
	if (sscanf(text, "%f %f", &zmin, &zmax) != 2)
	{
		wxMessageBox(_("Didn't get two numbers."));
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
	str = wxGetTextFromUser(_("Set unknown areas to what value?"),
		_("Set Unknown Areas"), str, this);
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

	wxString2 str = wxGetTextFromUser(_("Please enter a scale factor"),
		_("Scale Elevation"), _T("1.0"), this);
	if (str == _T(""))
		return;

	float fScale;
	fScale = atof(str.mb_str());
	if (fScale == 0.0f)
	{
		wxMessageBox(_("Couldn't parse the number you typed."));
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

void MainFrame::OnElevExport(wxCommandEvent &event)
{
	if (!GetActiveElevLayer())
		return;

	wxString choices[7];
	choices[0] = _T("ArcInfo ASCII Grid");
	choices[1] = _T("GeoTIFF");
	choices[2] = _T("TerraGen");
	choices[3] = _T("BMP");
	choices[4] = _T("STM");
	choices[5] = _T("MSI Planet");
	choices[6] = _T("VRML ElevationGrid");

	wxSingleChoiceDialog dlg(this, _("Please choose"),
		_("Export to file format:"), 7, choices);
	if (dlg.ShowModal() != wxID_OK)
		return;

	switch (dlg.GetSelection())
	{
	case 0: ExportASC(); break;
	case 1: ExportGeoTIFF(); break;
	case 2: ExportTerragen(); break;
	case 3: ExportBMP(); break;
	case 4: ExportSTM(); break;
	case 5: ExportPlanet(); break;
	case 6: ExportVRML(); break;
	}
}

vtString GetExportFilename(const wxString &format_filter)
{
	wxString2 filter = _("All Files|*.*");
	AddType(filter, format_filter);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Export Elevation"), _T(""), _T(""), filter, wxSAVE);
	saveFile.SetFilterIndex(1);
	if (saveFile.ShowModal() != wxID_OK)
		return vtString("");
	wxString2 result = saveFile.GetPath();
	return result.vt_str();
}

void MainFrame::ExportASC()
{
	// check spacing
	vtElevationGrid *grid = GetActiveElevLayer()->m_pGrid;
	DPoint2 spacing = grid->GetSpacing();
	double ratio = spacing.x / spacing.y;
	if (ratio < 0.999 || ratio > 1.001)
	{
		wxString str, str2;
		str = _("The Arc ASCII format only supports evenly spaced elevation grids.\n");
		str2.Printf(_("The spacing of this grid is %g x %g\n"), spacing.x, spacing.y);
		str += str2;
		str += _("The result my be stretched.  Do you want to continue anyway?");
		int result = wxMessageBox(str, _("Warning"), wxYES_NO | wxICON_QUESTION, this);
		if (result != wxYES)
			return;
	}

	vtString fname = GetExportFilename(FSTRING_ASC);
	if (fname == "")
		return;
	bool success = grid->SaveToASC(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportTerragen()
{
	vtString fname = GetExportFilename(FSTRING_TER);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToTerragen(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportGeoTIFF()
{
	vtString fname = GetExportFilename(FSTRING_TIF);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToGeoTIFF(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportBMP()
{
	vtString fname = GetExportFilename(FSTRING_BMP);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToBMP(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportSTM()
{
	vtString fname = GetExportFilename(FSTRING_STM);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToSTM(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportPlanet()
{
	static wxString default_path = wxEmptyString;

	// ask the user for a directory
	wxDirDialog getDir(this, _("Export Planet Data to Directory"),
		default_path, wxDD_DEFAULT_STYLE);
	getDir.SetWindowStyle(getDir.GetWindowStyle() | wxDD_NEW_DIR_BUTTON);
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString strDirName = getDir.GetPath();
	default_path = strDirName;	// save it for next time

	if (strDirName == _T(""))
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToPlanet(strDirName.mb_str());
	if (success)
		DisplayAndLog("Successfully wrote Planet dataset to '%s'", (const char *) strDirName.mb_str());
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::ExportVRML()
{
	vtString fname = GetExportFilename(FSTRING_WRL);
	if (fname == "")
		return;
	bool success = GetActiveElevLayer()->m_pGrid->SaveToVRML(fname);
	if (success)
		DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
	else
		DisplayAndLog("Error writing file.");
}

void MainFrame::OnElevExportBitmap(wxCommandEvent& event)
{
	int cols, rows;
	vtElevLayer *pEL = GetActiveElevLayer();
	pEL->m_pGrid->GetDimensions(cols, rows);

	RenderDlg dlg(this, -1, _("Render Elevation to Bitmap"));
	dlg.m_datapaths = m_datapaths;
	dlg.m_iSizeX = cols;
	dlg.m_iSizeY = rows;

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	OpenProgressDialog(_("Generating Bitmap"));
	ExportBitmap(dlg);
	CloseProgressDialog();
}

void MainFrame::ExportBitmap(RenderDlg &dlg)
{
	int xsize = dlg.m_iSizeX;
	int ysize = dlg.m_iSizeY;

	vtElevLayer *pEL = GetActiveElevLayer();

	ColorMap cmap;
	vtString fname = dlg.m_strColorMap.vt_str();
	vtString path = FindFileOnPaths(m_datapaths, "GeoTypical/" + fname);
	if (path == "")
	{
		DisplayAndLog("Couldn't load color map.");
		return;
	}
	if (!cmap.Load(path))
	{
		DisplayAndLog("Couldn't load color map.");
		return;
	}

	// Get attributes of existing layer
	DRECT area;
	vtProjection proj;
	pEL->GetExtent(area);
	pEL->GetProjection(proj);

	vtImageLayer *pOutput = NULL;
	vtBitmapBase *pBitmap = NULL;
	vtDIB dib;

	if (dlg.m_bToFile)
	{
		if (!dib.Create(xsize, ysize, 24))
		{
			DisplayAndLog("Failed to create bitmap.");
			return;
		}
		pBitmap = &dib;
	}
	else
	{
		pOutput = new vtImageLayer(area, xsize, ysize, proj);
		pBitmap = pOutput->GetBitmap();
	}

	pEL->m_pGrid->ColorDibFromElevation(pBitmap, &cmap, 8000, progress_callback);
	if (dlg.m_bShading)
	{
		// Quick and simple sunlight vector
		FPoint3 light_dir = LightDirection(vtElevLayer::m_draw.m_iCastAngle,
			vtElevLayer::m_draw.m_iCastDirection);

		if (vtElevLayer::m_draw.m_bCastShadows)
			pEL->m_pGrid->ShadowCastDib(pBitmap, light_dir, 1.0, progress_callback);
		else
			pEL->m_pGrid->ShadeDibFromElevation(pBitmap, light_dir, 1.0, true, progress_callback);
	}

	if (dlg.m_bToFile)
	{
		wxString2 fname = dlg.m_strToFile;
		bool success;
		if (dlg.m_bJPEG)
			success = dib.WriteJPEG(fname.mb_str(), 99);
		else
			success = dib.WriteTIF(fname.mb_str(), &area, &proj);
		if (success)
			DisplayAndLog("Successfully wrote to file '%s'", fname.mb_str());
		else
			DisplayAndLog("Couldn't open file for writing.");
	}
	else
	{
		AddLayerWithCheck(pOutput);
	}
#if 0
	int percent, last = -1;
	percent = i * 100 / w;
	if (percent != last)
	{
		wxString str;
		str.Printf(_T("%d%%"), percent);
		UpdateProgressDialog(percent, str);
		last = percent;
	}

	// TEST - try coloring from water polygons
	int layers = m_Layers.GetSize();
	for (int l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_WATER)
			((vtWaterLayer*)lp)->PaintDibWithWater(&dib);
	}

	UpdateProgressDialog(100, _("Writing bitmap to file."));
	bool success = dib.WriteBMP(fname.mb_str());
#endif
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

void MainFrame::OnAreaZoomAll(wxCommandEvent &event)
{
	m_pView->InvertAreaTool(m_area);
	m_area = GetExtents();
	m_pView->InvertAreaTool(m_area);
}

void MainFrame::OnUpdateAreaZoomAll(wxUpdateUIEvent& event)
{
	event.Enable(NumLayers() != 0);
}

void MainFrame::OnAreaZoomLayer(wxCommandEvent &event)
{
	DRECT area;
	if (GetActiveLayer()->GetExtent(area))
	{
		m_pView->InvertAreaTool(m_area);
		m_area = area;
		m_pView->InvertAreaTool(m_area);
	}
}

void MainFrame::OnUpdateAreaZoomLayer(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnAreaTypeIn(wxCommandEvent &event)
{
	ExtentDlg dlg(NULL, -1, _("Edit Area"));
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
	{
		const char *string = layers[i]->GetValueString("Name");
		choices[i] = wxString::FromAscii(string);
	}

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

void MainFrame::OnAreaRequestTServe(wxCommandEvent& event)
{
	int zone = m_proj.GetUTMZone();
	if (zone == 0 || m_area.IsEmpty())
	{
		wxMessageBox(
			_T("In order to request data from Terraserver, first set your CRS to\n")
			_T("a UTM zone (4 through 19) and use the Area Tool to indicate the\n")
			_T("area that you want to download."),
			_T("Note"));
		return;
	}

	TSDialog dlg(this, -1, _("Terraserver"));
	if (dlg.ShowModal() != wxID_OK)
		return;
	if (dlg.m_strToFile == _T(""))
		return;

	OpenProgressDialog(_("Requesting data from Terraserver..."));

	vtImageLayer *pIL = new vtImageLayer();
	bool success = pIL->ReadFeaturesFromTerraserver(m_area, dlg.m_iTheme,
		dlg.m_iMetersPerPixel, m_proj.GetUTMZone(), dlg.m_strToFile.mb_str());

	CloseProgressDialog();

	if (success)
		wxMessageBox(_("Successfully wrote file."));
	if (!success)
	{
		wxMessageBox(_("Unable to download."));
	}
	delete pIL;
	return;
#if 0
	if (dlg.m_bNewLayer)
	{
		if (!AddLayerWithCheck(pIL))
			delete pIL;
	}
	else
	{
		pIL->SaveToFile(dlg.m_strToFile.mb_str());
		delete pIL;
	}
#endif
}


//////////////////////////
// Vegetation menu

void MainFrame::OnVegPlants(wxCommandEvent& event)
{
	// if PlantList has not previously been open, get the data from file first
	if (m_strSpeciesFilename == "")
	{
		wxString filter = _("Plant Species List Files (*.xml)|*.xml");

		// Use file dialog to open plant list text file.
		wxFileDialog loadFile(NULL, _("Load Plant Info"), _T(""), _T(""),
			filter, wxOPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		wxString2 str = loadFile.GetPath();
		if (!LoadSpeciesFile(str.mb_str()))
			return;
	}
	if (!m_SpeciesListDlg)
	{
		// Create new Plant List Dialog
		m_SpeciesListDlg = new SpeciesListDlg(this, WID_PLANTS, _("Plants List"), 
				wxPoint(140, 100), wxSize(950, 400), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	}

	// Display plant list data, calling OnInitDialog.
	m_SpeciesListDlg->Show(true);
}


void MainFrame::OnVegBioregions(wxCommandEvent& event)
{
	// if data isn't there, get the data first
	if (m_strBiotypesFilename == "")
	{
		wxString filter = _("Bioregion Files (*.txt)|*.txt");

		// Use file dialog to open bioregion text file.
		wxFileDialog loadFile(NULL, _("Load BioRegion Info"), _T(""), _T(""),
			filter, wxOPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		// Read bioregions, data kept on frame with m_pBioRegion.
		wxString2 str = loadFile.GetPath();
		if (!LoadBiotypesFile(str.mb_str()))
			return;
	}
	if (!m_BioRegionDlg)
	{
		// Create new Bioregion Dialog
		m_BioRegionDlg = new BioRegionDlg(this, WID_BIOREGIONS, _("BioRegions List"), 
				wxPoint(120, 80), wxSize(300, 500), wxSYSTEM_MENU | wxCAPTION);
	}

	// Display bioregion data, calling OnInitDialog.
	m_BioRegionDlg->Show(true);
}

void MainFrame::OnVegRemap(wxCommandEvent& event)
{
	vtVegLayer *pVeg = GetMainFrame()->GetActiveVegLayer();
	if (!pVeg) return;

	vtSpeciesList *list = GetPlantList();

	wxArrayString choices;
	wxString2 str;
	unsigned int i, n = list->NumSpecies();
	for (i = 0; i < n; i++)
	{
		vtPlantSpecies *spe = list->GetSpecies(i);
		str = spe->GetSciName();
		choices.Add(str);
	}

	wxString2 result1 = wxGetSingleChoice(_("Remap FROM Species"), _T("Species"),
		choices, this);
	if (result1 == _T(""))	// cancelled
		return;
	short species_from = list->GetSpeciesIdByName(result1.mb_str());

	wxString2 result2 = wxGetSingleChoice(_("Remap TO Species"), _T("Species"),
		choices, this);
	if (result2 == _T(""))	// cancelled
		return;
	short species_to = list->GetSpeciesIdByName(result2.mb_str());

	vtFeatureSet *pSet = pVeg->GetFeatureSet();
	vtPlantInstanceArray *pPIA = dynamic_cast<vtPlantInstanceArray *>(pSet);
	if (!pPIA)
		return;

	float size;
	short species_id;
	int count = 0;
	for (i = 0; i < pPIA->GetNumEntities(); i++)
	{
		pPIA->GetPlant(i, size, species_id);
		if (species_id == species_from)
		{
			pPIA->SetPlant(i, size, species_to);
			count++;
		}
	}
	str.Printf(_("Remap successful, %d plants remapped.\n"), count);
	wxMessageBox(str, _("Info"));
	if (count > 0)
		pVeg->SetModified(true);
}

void MainFrame::OnVegExportSHP(wxCommandEvent& event)
{
	vtVegLayer *pVeg = GetMainFrame()->GetActiveVegLayer();
	if (!pVeg) return;

	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _("Export vegetation to SHP"), _T(""), _T(""),
		_("Vegetation Files (*.shp)|*.shp|"), wxSAVE | wxOVERWRITE_PROMPT);
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
	wxFileDialog saveFile(NULL, _("Save Vegetation File"), _T(""), _T(""),
		_("Vegetation Files (*.vf)|*.vf|"), wxSAVE | wxOVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString2 strPathName = saveFile.GetPath();

	DistribVegDlg dlg(this, -1, _("Vegetation Distribution Options"));

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	// Generate the plants
	GenerateVegetation(strPathName.mb_str(), m_area, dlg.m_opt);
}

void MainFrame::OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event)
{
	// we needs some species, and an area to plant them in
	event.Enable(m_strSpeciesFilename != "" && !m_area.IsEmpty());
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
		m_pFeatInfoDlg->SetLayer(GetActiveLayer());
		m_pFeatInfoDlg->SetFeatureSet(GetActiveRawLayer()->GetFeatureSet());
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

void MainFrame::OnStructureAddInstances(wxCommandEvent &event)
{
	m_pView->SetMode(LB_AddInstance);
}

void MainFrame::OnUpdateStructureAddInstances(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_AddInstance);
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

void MainFrame::OnUpdateRawIsActive(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL);
}

void MainFrame::OnRawSetType(wxCommandEvent& event)
{
	static OGRwkbGeometryType types[5] = {
		wkbNone,
		wkbPoint,
		wkbPoint25D,
		wkbLineString,
		wkbPolygon
	};
	wxString choices[5];
	for (int i = 0; i < 5; i++)
		choices[i] = wxString::FromAscii(OGRGeometryTypeToName(types[i]));

	int n = 5;
	int cur_type = 0;

	wxSingleChoiceDialog dialog(this, _("Raw Layer Type"),
		_("Please indicate entity type:"), n, (const wxString *)choices);

	dialog.SetSelection(cur_type);

	if (dialog.ShowModal() == wxID_OK)
	{
		cur_type = dialog.GetSelection();
		vtRawLayer *pRL = (vtRawLayer *) GetActiveLayer();

		// must set the projection and layername again, as they are reset on
		//  setting geom type
		wxString2 name = pRL->GetLayerFilename();
		pRL->SetGeomType(types[cur_type]);
		pRL->SetProjection(m_proj);
		pRL->SetLayerFilename(name);
		RefreshTreeStatus();
	}
}

void MainFrame::OnUpdateRawSetType(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetGeomType() == wkbNone);
}

void MainFrame::OnRawAddPoints(wxCommandEvent& event)
{
	m_pView->SetMode(LB_AddPoints);
}

void MainFrame::OnUpdateRawAddPoints(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL &&
		(pRL->GetGeomType() == wkbPoint ||
		 pRL->GetGeomType() == wkbPoint25D));
	event.Check(m_pView->GetMode() == LB_AddPoints);
}

void MainFrame::OnRawAddPointText(wxCommandEvent& event)
{
	wxString2 str = wxGetTextFromUser(_("(X, Y) in current projection"),
			_("Enter coordinate"));
	if (str == _T(""))
		return;
	double x, y;
	int num = sscanf(str.mb_str(), "%lf, %lf", &x, &y);
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
	event.Enable(pRL != NULL && pRL->GetGeomType() == wkbPoint);
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
	vtFeatureSet *pFS = pRL->GetFeatureSet();

	if (!pFS || pFS->GetNumFields() == 0)
	{
		DisplayAndLog("Can't select by condition because the current\n"
					  "layer has no fields defined.");
		return;
	}
	SelectDlg dlg(this, -1, _("Select"));
	dlg.SetRawLayer(pRL);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString2 str = dlg.m_strValue;
		int selected = pFS->SelectByCondition(dlg.m_iField, dlg.m_iCondition,
			str.mb_str());

		wxString2 msg;
		if (selected == -1)
			msg = _("Unable to select");
		else
			msg.Printf(_("Selected %d entit%hs"), selected, selected == 1 ? "y" : "ies");
		SetStatusText(msg);

		msg += _T("\n");
		g_Log._Log(msg.mb_str());

		m_pView->Refresh(false);
		OnSelectionChanged();
	}
}

void CapWords(vtString &str)
{
	bool bStart = true;
	for (int i = 0; i < str.GetLength(); i++)
	{
		char ch = str.GetAt(i);
		if (bStart)
			ch = toupper(ch);
		else
			ch = tolower(ch);
		str.SetAt(i, ch);

		if (ch == ' ')
			bStart = true;
		else
			bStart = false;
	}
}

void MainFrame::OnRawExportImageMap(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	if (!pRL)
		return;

	OGRwkbGeometryType type = pRL->GetGeomType();
	if (type != wkbPolygon)
		return;

	// First grab image
	BuilderView *view = GetView();
	int xsize, ysize;
	view->GetClientSize(&xsize, &ysize);

	wxClientDC dc(view);
	view->PrepareDC(dc);

	vtDIB dib;
	dib.Create(xsize, ysize, 24);

	int x, y;
	int xx, yy;
	wxColour color;
	for (x = 0; x < xsize; x++)
	{
		for (y = 0; y < ysize; y++)
		{
			view->CalcUnscrolledPosition(x, y, &xx, &yy);
			dc.GetPixel(xx, yy, &color);
			dib.SetPixel24(x, y, RGBi(color.Red(), color.Green(), color.Blue()));
		}
	}

	vtFeatureSet *fset = pRL->GetFeatureSet();

	ImageMapDlg dlg(this, -1, _("Export Image Map"));
	dlg.SetFields(fset);
	if (dlg.ShowModal() != wxID_OK)
		return;

	wxFileDialog loadFile(NULL, _("Save to Image File"), _T(""), _T(""),
		FSTRING_PNG, wxSAVE);
	if (loadFile.ShowModal() != wxID_OK)
		return;
	vtString fullname = loadFile.GetPath().mb_str();
	vtString filename = loadFile.GetFilename().mb_str();

	dib.WritePNG(fullname);

	// Then write imagemap
	wxFileDialog loadFile2(NULL, _("Save to Image File"), _T(""), _T(""),
		FSTRING_HTML, wxSAVE);
	if (loadFile2.ShowModal() != wxID_OK)
		return;
	vtString htmlname = loadFile2.GetPath().mb_str();

	FILE *fp = fopen(htmlname, "wb");
	if (!fp)
		return;
	fprintf(fp, "<html>\n");
	fprintf(fp, "<body>\n");
	fprintf(fp, "<map name=\"ImageMap\">\n");

	vtFeatureSetPolygon *polyset = (vtFeatureSetPolygon *) fset;
	unsigned int i, num = polyset->GetNumEntities();
	wxPoint sp;		// screen point

	for (i = 0; i < num; i++)
	{
		vtString str;
		polyset->GetValueAsString(i, dlg.m_iField, str);

//		CapWords(str);
//		str += ".sid";

		fprintf(fp, "<area href=\"%s\" shape=\"polygon\" coords=\"",
			(const char *) str);

		const DPolygon2 &poly = polyset->GetPolygon(i);

		DLine2 dline;
		poly.GetAsDLine2(dline);

		unsigned int j, points = dline.GetSize();
		for (j = 0; j < points; j++)
		{
			DPoint2 p = dline[j];
			if (j == points-1 && p == dline[0])
				continue;

			if (j > 0)
				fprintf(fp, ", ");

			view->screen(p, sp);
			view->CalcScrolledPosition(sp.x, sp.y, &xx, &yy);

			fprintf(fp, "%d, %d", xx, yy);
		}
		fprintf(fp, "\">\n");
	}
	fprintf(fp, "</map>\n");
	fprintf(fp, "<img border=\"0\" src=\"%s\" usemap=\"#ImageMap\" width=\"%d\" height=\"%d\">\n",
		(const char *) filename, xsize, ysize);
	fprintf(fp, "</body>\n");
	fprintf(fp, "</html>\n");
	fclose(fp);
}

void MainFrame::OnRawStyle(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();

	DrawStyle style = pRL->GetDrawStyle();
	style.m_LineColor.Set(0,0,0);
	pRL->SetDrawStyle(style);
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
	wxString str = _("Virtual Terrain Builder\nPowerful, easy to use, free!\n");
	str += _T("\n");
	str += _("Please read the HTML documentation and license.\n");
	str += _T("\n");
	str += _("Send feedback to: ben@vterrain.org\n");
	str += _("Build date: ");
	str += _T(__DATE__);
	wxString str2 = _("About ");
	str2 += _T(APPNAME);
	wxMessageBox(str, str2);
#endif
}

