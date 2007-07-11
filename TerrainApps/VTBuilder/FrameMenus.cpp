//
//  The menus functions of the main Frame window of the VTBuilder application.
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/progdlg.h>
#include <wx/choicdlg.h>
#include <wx/colordlg.h>

#include "vtdata/config_vtdata.h"
#include "vtdata/ChunkLOD.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/Icosa.h"
#include "vtdata/TripDub.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtdata/WFSClient.h"
#include "vtui/Helper.h"
#include "vtui/ProfileDlg.h"
#include "vtui/ProjectionDlg.h"

#include "gdal_priv.h"

#include "Frame.h"
#include "MenuEnum.h"
#include "BuilderView.h"
#include "TreeView.h"
#include "Helper.h"
#include "vtBitmap.h"
#include "FileFilters.h"
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
#include "DistanceDlg2d.h"
#include "DistribVegDlg.h"
#include "ExtentDlg.h"
#include "FeatInfoDlg.h"
#include "GeocodeDlg.h"
#include "ImageMapDlg.h"
#include "LayerPropDlg.h"
#include "MapServerDlg.h"
#include "MatchDlg.h"
#include "OptionsDlg.h"
#include "PrefDlg.h"
#include "RenderDlg.h"
#include "SelectDlg.h"
#include "TSDlg.h"
#include "VegDlg.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_FILE_NEW,		MainFrame::OnProjectNew)
EVT_MENU(ID_FILE_OPEN,		MainFrame::OnProjectOpen)
EVT_MENU(ID_FILE_SAVE,		MainFrame::OnProjectSave)
EVT_MENU(ID_FILE_PREFS,		MainFrame::OnProjectPrefs)
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
EVT_MENU(ID_LAYER_IMPORT_XML,	MainFrame::OnLayerImportXML)
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
EVT_MENU(ID_VIEW_ZOOMIN,		MainFrame::OnViewZoomIn)
EVT_MENU(ID_VIEW_ZOOMOUT,		MainFrame::OnViewZoomOut)
EVT_MENU(ID_VIEW_ZOOMALL,		MainFrame::OnViewZoomAll)
EVT_MENU(ID_VIEW_ZOOM_LAYER,	MainFrame::OnViewZoomToLayer)
EVT_MENU(ID_VIEW_FULLVIEW,		MainFrame::OnViewFull)
EVT_MENU(ID_VIEW_ZOOM_AREA,		MainFrame::OnViewZoomArea)
EVT_MENU(ID_VIEW_TOOLBAR,		MainFrame::OnViewToolbar)
EVT_MENU(ID_VIEW_LAYERS,		MainFrame::OnViewLayers)
EVT_MENU(ID_VIEW_MAGNIFIER,		MainFrame::OnViewMagnifier)
EVT_MENU(ID_VIEW_PAN,			MainFrame::OnViewPan)
EVT_MENU(ID_VIEW_DISTANCE,		MainFrame::OnViewDistance)
EVT_MENU(ID_VIEW_SETAREA,		MainFrame::OnViewSetArea)
EVT_MENU(ID_VIEW_WORLDMAP,		MainFrame::OnViewWorldMap)
EVT_MENU(ID_VIEW_SHOWUTM,		MainFrame::OnViewUTMBounds)
EVT_MENU(ID_VIEW_PROFILE,		MainFrame::OnViewProfile)
EVT_MENU(ID_VIEW_SCALE_BAR,		MainFrame::OnViewScaleBar)
EVT_MENU(ID_VIEW_OPTIONS,		MainFrame::OnViewOptions)

EVT_UPDATE_UI(ID_VIEW_SHOWLAYER,	MainFrame::OnUpdateLayerShow)
EVT_UPDATE_UI(ID_VIEW_LAYER_UP,		MainFrame::OnUpdateLayerUp)
EVT_UPDATE_UI(ID_VIEW_MAGNIFIER,	MainFrame::OnUpdateMagnifier)
EVT_UPDATE_UI(ID_VIEW_PAN,			MainFrame::OnUpdatePan)
EVT_UPDATE_UI(ID_VIEW_DISTANCE,		MainFrame::OnUpdateDistance)
EVT_UPDATE_UI(ID_VIEW_ZOOM_LAYER,	MainFrame::OnUpdateViewZoomToLayer)
EVT_UPDATE_UI(ID_VIEW_FULLVIEW,		MainFrame::OnUpdateViewFull)
EVT_UPDATE_UI(ID_VIEW_ZOOM_AREA,	MainFrame::OnUpdateViewZoomArea)
EVT_UPDATE_UI(ID_VIEW_TOOLBAR,		MainFrame::OnUpdateViewToolbar)
EVT_UPDATE_UI(ID_VIEW_LAYERS,		MainFrame::OnUpdateViewLayers)
EVT_UPDATE_UI(ID_VIEW_SETAREA,		MainFrame::OnUpdateViewSetArea)
EVT_UPDATE_UI(ID_VIEW_WORLDMAP,		MainFrame::OnUpdateWorldMap)
EVT_UPDATE_UI(ID_VIEW_SHOWUTM,		MainFrame::OnUpdateUTMBounds)
EVT_UPDATE_UI(ID_VIEW_PROFILE,		MainFrame::OnUpdateViewProfile)
EVT_UPDATE_UI(ID_VIEW_SCALE_BAR,	MainFrame::OnUpdateViewScaleBar)

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
EVT_MENU(ID_ELEV_EXPORT_TILES,		MainFrame::OnElevExportTiles)
EVT_MENU(ID_ELEV_COPY,				MainFrame::OnElevCopy)
EVT_MENU(ID_ELEV_PASTE_NEW,			MainFrame::OnElevPasteNew)
EVT_MENU(ID_ELEV_BITMAP,			MainFrame::OnElevExportBitmap)
EVT_MENU(ID_ELEV_MERGETIN,			MainFrame::OnElevMergeTin)
EVT_MENU(ID_ELEV_TRIMTIN,			MainFrame::OnElevTrimTin)

EVT_UPDATE_UI(ID_ELEV_SELECT,		MainFrame::OnUpdateElevSelect)
EVT_UPDATE_UI(ID_ELEV_REMOVERANGE,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SETUNKNOWN,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_FILLIN,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SCALE,		MainFrame::OnUpdateScaleElevation)
EVT_UPDATE_UI(ID_ELEV_EXPORT,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_EXPORT_TILES,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_COPY,			MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_BITMAP,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_MERGETIN,		MainFrame::OnUpdateElevMergeTin)
EVT_UPDATE_UI(ID_ELEV_TRIMTIN,		MainFrame::OnUpdateElevTrimTin)

EVT_MENU(ID_IMAGE_EXPORT_TILES,		MainFrame::OnImageExportTiles)
EVT_UPDATE_UI(ID_IMAGE_EXPORT_TILES,MainFrame::OnUpdateImageExportTiles)

EVT_MENU(ID_IMAGE_EXPORT_PPM,		MainFrame::OnImageExportPPM)
EVT_UPDATE_UI(ID_IMAGE_EXPORT_PPM,	MainFrame::OnUpdateImageExportPPM)

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
EVT_MENU(ID_STRUCTURE_SELECT_USING_POLYGONS, MainFrame::OnStructureSelectUsingPolygons)
EVT_MENU(ID_STRUCTURE_COLOUR_SELECTED_ROOFS, MainFrame::OnStructureColourSelectedRoofs)
EVT_MENU(ID_STRUCTURE_EXPORT_FOOTPRINTS, MainFrame::OnStructureExportFootprints)

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
EVT_UPDATE_UI(ID_STRUCTURE_SELECT_USING_POLYGONS, MainFrame::OnUpdateStructureSelectUsingPolygons)
EVT_UPDATE_UI(ID_STRUCTURE_COLOUR_SELECTED_ROOFS, MainFrame::OnUpdateStructureColourSelectedRoofs)
EVT_UPDATE_UI(ID_STRUCTURE_EXPORT_FOOTPRINTS, MainFrame::OnUpdateStructureExportFootprints)

EVT_MENU(ID_RAW_SETTYPE,			MainFrame::OnRawSetType)
EVT_MENU(ID_RAW_ADDPOINTS,			MainFrame::OnRawAddPoints)
EVT_MENU(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnRawAddPointText)
EVT_MENU(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnRawAddPointsGPS)
EVT_MENU(ID_RAW_SELECTCONDITION,	MainFrame::OnRawSelectCondition)
EVT_MENU(ID_RAW_EXPORT_IMAGEMAP,	MainFrame::OnRawExportImageMap)
EVT_MENU(ID_RAW_STYLE,				MainFrame::OnRawStyle)
EVT_MENU(ID_RAW_SCALE,				MainFrame::OnRawScale)

EVT_UPDATE_UI(ID_RAW_SETTYPE,			MainFrame::OnUpdateRawSetType)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS,			MainFrame::OnUpdateRawAddPoints)
EVT_UPDATE_UI(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnUpdateRawAddPointText)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnUpdateRawAddPointsGPS)
EVT_UPDATE_UI(ID_RAW_SELECTCONDITION,	MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_EXPORT_IMAGEMAP,	MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_STYLE,				MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_SCALE,				MainFrame::OnUpdateRawIsActive)

EVT_MENU(ID_AREA_ZOOM_ALL,			MainFrame::OnAreaZoomAll)
EVT_MENU(ID_AREA_ZOOM_LAYER,		MainFrame::OnAreaZoomLayer)
EVT_MENU(ID_AREA_TYPEIN,			MainFrame::OnAreaTypeIn)
EVT_MENU(ID_AREA_MATCH,				MainFrame::OnAreaMatch)
EVT_MENU(ID_AREA_EXPORT_ELEV,		MainFrame::OnAreaExportElev)
EVT_MENU(ID_AREA_EXPORT_IMAGE,		MainFrame::OnAreaExportImage)
EVT_MENU(ID_AREA_EXPORT_ELEV_SPARSE,MainFrame::OnAreaOptimizedElevTileset)
EVT_MENU(ID_AREA_EXPORT_IMAGE_OPT,	MainFrame::OnAreaOptimizedImageTileset)
EVT_MENU(ID_AREA_GENERATE_VEG,		MainFrame::OnAreaGenerateVeg)
EVT_MENU(ID_AREA_VEG_DENSITY,		MainFrame::OnAreaVegDensity)
EVT_MENU(ID_AREA_REQUEST_WFS,		MainFrame::OnAreaRequestWFS)
EVT_MENU(ID_AREA_REQUEST_WMS,		MainFrame::OnAreaRequestWMS)
EVT_MENU(ID_AREA_REQUEST_TSERVE,	MainFrame::OnAreaRequestTServe)

EVT_UPDATE_UI(ID_AREA_ZOOM_ALL,		MainFrame::OnUpdateAreaZoomAll)
EVT_UPDATE_UI(ID_AREA_ZOOM_LAYER,	MainFrame::OnUpdateAreaZoomLayer)
EVT_UPDATE_UI(ID_AREA_MATCH,		MainFrame::OnUpdateAreaMatch)
EVT_UPDATE_UI(ID_AREA_EXPORT_ELEV,	MainFrame::OnUpdateAreaExportElev)
EVT_UPDATE_UI(ID_AREA_EXPORT_ELEV_SPARSE,MainFrame::OnUpdateAreaExportElev)
EVT_UPDATE_UI(ID_AREA_EXPORT_IMAGE_OPT,MainFrame::OnUpdateAreaExportImage)
EVT_UPDATE_UI(ID_AREA_EXPORT_IMAGE,	MainFrame::OnUpdateAreaExportImage)
EVT_UPDATE_UI(ID_AREA_GENERATE_VEG,	MainFrame::OnUpdateAreaGenerateVeg)
EVT_UPDATE_UI(ID_AREA_VEG_DENSITY,	MainFrame::OnUpdateAreaVegDensity)
EVT_UPDATE_UI(ID_AREA_REQUEST_WFS,	MainFrame::OnUpdateAreaRequestWMS)
EVT_UPDATE_UI(ID_AREA_REQUEST_WMS,	MainFrame::OnUpdateAreaRequestWMS)

EVT_MENU(wxID_HELP,				MainFrame::OnHelpAbout)
EVT_MENU(ID_HELP_DOC_LOCAL,		MainFrame::OnHelpDocLocal)
EVT_MENU(ID_HELP_DOC_ONLINE,	MainFrame::OnHelpDocOnline)

// Popup menu items
EVT_MENU(ID_DISTANCE_CLEAR,			MainFrame::OnDistanceClear)

EVT_CHAR(MainFrame::OnChar)
EVT_KEY_DOWN(MainFrame::OnKeyDown)
EVT_MOUSEWHEEL(MainFrame::OnMouseWheel)
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
	specialMenu->Append(ID_ELEV_COPY, _("Copy Elevation Layer to Clipboard"));
	specialMenu->Append(ID_ELEV_PASTE_NEW, _("New Elevation Layer from Clipboard"));
	fileMenu->Append(0, _("Special"), specialMenu);
#endif
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_FILE_PREFS, _("Preferences"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_FILE_EXIT, _("E&xit\tAlt-X"), _("Exit"));
#ifdef __WXMAC__
	wxApp::s_macPreferencesMenuItemId = ID_FILE_PREFS;
	wxApp::s_macExitMenuItemId = ID_FILE_EXIT;
#endif
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
	layerMenu->Append(ID_LAYER_IMPORT_XML, _("Import Point Data From XML"));
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
	viewMenu->Append(ID_VIEW_ZOOM_AREA, _("Zoom to Area Tool"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_TOOLBAR, _("Toolbar"));
	viewMenu->AppendCheckItem(ID_VIEW_LAYERS, _("Layers"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_MAGNIFIER, _("&Magnifier\tZ"));
	viewMenu->AppendCheckItem(ID_VIEW_PAN, _("&Pan\tSPACE"));
	viewMenu->AppendCheckItem(ID_VIEW_DISTANCE, _("Obtain &Distance"));
	viewMenu->AppendCheckItem(ID_VIEW_SETAREA, _("Area &Tool"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_WORLDMAP, _("&World Map"), _("Show/Hide World Map"));
	viewMenu->AppendCheckItem(ID_VIEW_SHOWUTM, _("Show &UTM Boundaries"));
//	viewMenu->AppendCheckItem(ID_VIEW_SHOWGRID, _("Show 7.5\" Grid"), _("Show 7.5\" Grid"), true);
	viewMenu->AppendCheckItem(ID_VIEW_PROFILE, _("Elevation Profile"));
	viewMenu->AppendCheckItem(ID_VIEW_SCALE_BAR, _("Scale Bar"));
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
	elevMenu->Append(ID_ELEV_EXPORT_TILES, _("Export to Tiles..."));
	elevMenu->Append(ID_ELEV_BITMAP, _("Re&nder to Bitmap..."));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_MERGETIN, _("&Merge shared TIN vertices"));
	elevMenu->AppendCheckItem(ID_ELEV_TRIMTIN, _("Trim TIN triangles by line segment"));
	m_pMenuBar->Append(elevMenu, _("Elev&ation"));
	m_iLayerMenu[LT_ELEVATION] = menu_num;
	menu_num++;

#ifndef ELEVATION_ONLY
	// Imagery
	imgMenu = new wxMenu;
	imgMenu->Append(ID_IMAGE_EXPORT_TILES, _("Export to Tiles..."));
	imgMenu->Append(ID_IMAGE_EXPORT_PPM, _("Export to PPM"));
	m_pMenuBar->Append(imgMenu, _("Imagery"));
	m_iLayerMenu[LT_IMAGE] = menu_num;
	menu_num++;

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
	bldMenu->Append(ID_STRUCTURE_SELECT_USING_POLYGONS, _("Select Using Polygons"), _("Select buildings using selected raw layer polygons"));
	bldMenu->Append(ID_STRUCTURE_COLOUR_SELECTED_ROOFS, _("Colour Selected Roofs"), _("Set roof colour on selected buildings"));
	bldMenu->AppendSeparator();
	bldMenu->Append(ID_STRUCTURE_EXPORT_FOOTPRINTS, _("Export footprints to SHP"));

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
	rawMenu->Append(ID_RAW_SCALE, _("Scale"));
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
	areaMenu->Append(ID_AREA_MATCH, _("Match Area and Tiling to Layer"),
		_("Set the Area Tool rectangle by matching the resolution of a layer."));
	areaMenu->AppendSeparator();
	areaMenu->Append(ID_AREA_EXPORT_ELEV, _("Merge && Resample &Elevation"),
		_("Sample all elevation data within the Area Tool to produce a single, new elevation."));
#ifndef ELEVATION_ONLY
	areaMenu->Append(ID_AREA_EXPORT_IMAGE, _("Merge && Resample &Imagery"),
		_("Sample imagery within the Area Tool to produce a single, new image."));
	areaMenu->Append(ID_AREA_GENERATE_VEG, _("Generate Vegetation"),
		_("Generate Vegetation File (*.vf) containing plant distribution."));
	areaMenu->Append(ID_AREA_VEG_DENSITY, _("Compute Vegetation Density"),
		_("Compute and display the density of each species of vegetation in the given area."));
#if SUPPORT_HTTP
	areaMenu->Append(ID_AREA_REQUEST_WFS, _("Request Layer from WFS"));
	areaMenu->Append(ID_AREA_REQUEST_WMS, _("Request Image from WMS"));
	areaMenu->Append(ID_AREA_REQUEST_TSERVE, _("Request Image from Terraserver"));
#endif // SUPPORT_HTTP
#endif
	areaMenu->AppendSeparator();
	areaMenu->Append(ID_AREA_EXPORT_ELEV_SPARSE, _("Optimized Resample Elevation to Tileset"),
		_("Sample all elevation data within the Area Tool efficiently to produce an elevation tileset."));
	areaMenu->Append(ID_AREA_EXPORT_IMAGE_OPT, _("Optimized Resample Imagery to Tileset"),
		_("Sample all image data within the Area Tool efficiently to produce an image tileset."));
	m_pMenuBar->Append(areaMenu, _("&Area Tool"));
	menu_num++;

	// Help
	helpMenu = new wxMenu;
	wxString msg = _("About ");
	msg += wxString(APPNAME, wxConvUTF8);
#ifdef __WXMAC__
#endif
	helpMenu->Append(wxID_HELP, _("&About"), msg);
	helpMenu->Append(ID_HELP_DOC_LOCAL, _("Documentation (local)"), msg);
	helpMenu->Append(ID_HELP_DOC_ONLINE, _("Documentation (on the web)"), msg);
	m_pMenuBar->Append(helpMenu, _("&Help"));
#ifdef __WXMAC__
	wxApp::s_macAboutMenuItemId = wxID_HELP;
	wxApp::s_macHelpMenuTitleName = _("&Help");
#endif
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
	RefreshToolbars();

	vtProjection p;
	SetProjection(p);
}

wxString GetProjectFilter()
{
	wxString str(APPNAME, wxConvUTF8);
	str += _T(" ");
	str += _("Project Files (*.vtb)|*.vtb");
	return str;
}

void MainFrame::OnProjectOpen(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Load Project"), _T(""), _T(""),
		GetProjectFilter(), wxFD_OPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	LoadProject(loadFile.GetPath());
}

void MainFrame::OnProjectSave(wxCommandEvent &event)
{
	wxFileDialog saveFile(NULL, _("Save Project"), _T(""), _T(""),
		GetProjectFilter(), wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strPathName = saveFile.GetPath();

	SaveProject(strPathName);
}

void MainFrame::OnProjectPrefs(wxCommandEvent &event)
{
	PrefDlg dlg(this, wxID_ANY, _("Preferences"));
	dlg.b1 = (m_bUseCurrentCRS == true);
	dlg.b2 = (m_bUseCurrentCRS == false);
	dlg.b3 = (m_bLoadImagesAlways == true);
	dlg.b4 = (m_bLoadImagesNever == true);
	dlg.b5 = (!m_bLoadImagesAlways && !m_bLoadImagesNever);
	dlg.b6 = !m_bSlowFillGaps;
	dlg.b7 = m_bSlowFillGaps;
    dlg.TransferDataToWindow();
	if (dlg.ShowModal() == wxID_OK)
	{
		m_bUseCurrentCRS = dlg.b1;
		m_bLoadImagesAlways = dlg.b3;
		m_bLoadImagesNever = dlg.b4;
		m_bSlowFillGaps = dlg.b7;
	}
}

void MainFrame::OnDymaxTexture(wxCommandEvent &event)
{
	ExportDymaxTexture();
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
	wxString str = dlg1.GetValue();
	vtString color = (const char *) str.mb_str(wxConvUTF8);
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
	vtString fname_in = (const char *) str.mb_str(wxConvUTF8);

	wxFileDialog dlg3(this, _T("Choose output texture file"), _T(""), _T(""),
		_T("*.png"), wxFD_SAVE);
	if (dlg3.ShowModal() == wxID_CANCEL)
		return;
	str = dlg3.GetPath();
	vtString fname_out = (const char *) str.mb_str(wxConvUTF8);

	OpenProgressDialog(_T("Processing"));

	ProcessBillboardTexture(fname_in, fname_out, bg, progress_callback);

	CloseProgressDialog();
}

void MainFrame::OnGeocode(wxCommandEvent &event)
{
	DoGeocode();
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
	wxString str = dlg.GetValue();
	sscanf(str.mb_str(wxConvUTF8), "%lf, %lf", &offset.x, &offset.y);

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
	RefreshToolbars();
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
	wxFileDialog loadFile(NULL, _("Open Layer"), _T(""), _T(""), filter, wxFD_OPEN);
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
	wxString msg = _("Saving layer to file ") + lp->GetLayerFilename();
	SetStatusText(msg);
	VTLOG(msg.mb_str(wxConvUTF8));
	VTLOG("\n");

	if (lp->Save())
		msg = _("Saved layer to file ") + lp->GetLayerFilename();
	else
		msg = _("Save failed.");
	SetStatusText(msg);
	VTLOG(msg.mb_str(wxConvUTF8));
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

	wxString msg = _("Saving layer to file as ") + lp->GetLayerFilename();
	SetStatusText(msg);

	VTLOG1(msg.mb_str(wxConvUTF8));
	VTLOG1("\n");

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

	VTLOG1(msg.mb_str(wxConvUTF8));
	VTLOG1("\n");
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
		_("NTF Files (*.ntf)|*.ntf"), wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString str = loadFile.GetPath();
	ImportDataFromNTF(str);
}

void MainFrame::OnLayerImportUtil(wxCommandEvent &event)
{
	// ask the user for a directory
	wxDirDialog getDir(NULL, _("Import Utility Data from Directory of SHP Files"));
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strDirName = getDir.GetPath();

//	dlg.m_strCaption = _T("Shapefiles do not contain projection information.  ")
//		_T("Please indicate the projection of this file:");
	// ask user for a projection
	ProjectionDlg dlg(NULL, -1, _("Indicate Projection"));
	dlg.SetProjection(m_proj);

	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	vtProjection proj;
	dlg.GetProjection(proj);

	// create the new layers
	vtUtilityLayer *pUL = new vtUtilityLayer;
	if (pUL->ImportFromSHP(strDirName.mb_str(wxConvUTF8), proj))
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
		_("MapSource Export Files (*.txt)|*.txt"), wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString str = loadFile.GetPath();
	ImportFromMapSource(str.mb_str(wxConvUTF8));
}

void MainFrame::OnLayerImportPoint(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import Point Data"), _T(""), _T(""),
		_("Tabular Data Files (*.dbf)|*.dbf"), wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString str = loadFile.GetPath();
	ImportDataPointsFromTable(str.mb_str(wxConvUTF8));
}

void MainFrame::OnLayerImportXML(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import XML Data"), _T(""), _T(""),
		_("XML files (*.xml)|*.xml"), wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString str = loadFile.GetPath();
	vtRawLayer *pRL = new vtRawLayer;
	if (pRL->ImportFromXML(str.mb_str(wxConvUTF8)))
	{
		pRL->SetLayerFilename(str);
		pRL->SetModified(true);

		if (!AddLayerWithCheck(pRL, true))
			delete pRL;
	}
	else
		delete pRL;
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
	MergeResampleElevation();
}

void MainFrame::OnAreaOptimizedElevTileset(wxCommandEvent &event)
{
	ExportAreaOptimizedElevTileset();
}

void MainFrame::OnAreaOptimizedImageTileset(wxCommandEvent &event)
{
	ExportAreaOptimizedImageTileset();
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
	ProjectionDlg dlg(NULL, 200, _("Convert to what projection?"));
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
	ProjectionDlg dlg(NULL, -1, _("Set to what projection?"));
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
	ShowDistanceDlg();
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

void MainFrame::OnUpdateViewZoomToLayer(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
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

void MainFrame::OnViewZoomArea(wxCommandEvent& event)
{
	m_pView->ZoomToRect(m_area, 0.1f);
}

void MainFrame::OnUpdateViewZoomArea(wxUpdateUIEvent& event)
{
	event.Enable(!m_area.IsEmpty());
}

void MainFrame::OnViewToolbar(wxCommandEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pToolbar);
	info.Show(!info.IsShown());
	m_mgr.Update();
}

void MainFrame::OnUpdateViewToolbar(wxUpdateUIEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pToolbar);
	event.Check(info.IsShown());
}

void MainFrame::OnViewLayers(wxCommandEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pTree);
	info.Show(!info.IsShown());
	m_mgr.Update();
}

void MainFrame::OnUpdateViewLayers(wxUpdateUIEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pTree);
	event.Check(info.IsShown());
}

void MainFrame::OnViewWorldMap(wxCommandEvent& event)
{
	m_pView->SetShowMap(!m_pView->GetShowMap());
	m_pView->Refresh();
}

void MainFrame::OnUpdateWorldMap(wxUpdateUIEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pView);
	event.Check(info.IsShown());
}

void MainFrame::OnViewUTMBounds(wxCommandEvent& event)
{
	m_pView->m_bShowUTMBounds = !m_pView->m_bShowUTMBounds;
	m_pView->Refresh();
}

void MainFrame::OnUpdateUTMBounds(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bShowUTMBounds);
}

void MainFrame::OnViewProfile(wxCommandEvent& event)
{
	if (m_pProfileDlg && m_pProfileDlg->IsShown())
		m_pProfileDlg->Hide();
	else
		ShowProfileDlg();
}

void MainFrame::OnUpdateViewProfile(wxUpdateUIEvent& event)
{
	event.Check(m_pProfileDlg && m_pProfileDlg->IsShown());
	event.Enable(LayersOfType(LT_ELEVATION) > 0);
}

void MainFrame::OnViewScaleBar(wxCommandEvent& event)
{
	m_pView->SetShowScaleBar(!m_pView->GetShowScaleBar());
}

void MainFrame::OnUpdateViewScaleBar(wxUpdateUIEvent& event)
{
	event.Check(m_pView && m_pView->GetShowScaleBar());
}

void MainFrame::OnViewOptions(wxCommandEvent& event)
{
	OptionsDlg dlg(this, -1, _("Options"));

	dlg.m_bShowToolbar = m_pToolbar->IsShown();
	dlg.m_bShowMinutes = m_statbar->m_bShowMinutes;
	dlg.m_iElevUnits = (int)(m_statbar->m_ShowVertUnits) - 1;

	dlg.SetElevDrawOptions(vtElevLayer::m_draw);

	dlg.m_bShowRoadWidth = vtRoadLayer::GetDrawWidth();
	dlg.m_bShowPath = m_pTree->GetShowPaths();

	if (dlg.ShowModal() != wxID_OK)
		return;

	bool bNeedRefresh = false;

	if (dlg.m_bShowToolbar != m_pToolbar->IsShown())
	{
		m_pToolbar->Show(dlg.m_bShowToolbar);
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
		wxString str = dlg.GetValue();
		sscanf(str.mb_str(wxConvUTF8), "%d", &num);
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
	if (m_proj.IsGeographic())
	{
		wxMessageBox(_("Sorry, but precise grid operations require a non-geographic coordinate\n system (meters as horizontal units, not degrees.)"),
			_("Info"), wxOK);
		return;
	}

	float margin = 2.0;
	wxString str;
	str.Printf(_("%g"), margin);
	str = wxGetTextFromUser(_("How many meters for the margin at the edge of each road?"),
		_("Flatten elevation grid under roads"), str, this);
	if (str == _T(""))
		return;

	margin = atof(str.mb_str(wxConvUTF8));

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

	wxString str;
	str = wxGetTextFromUser(_("Please specify the elevation range\n(minimum and maximum in the form \"X Y\")\nAll values within this range (and within the area\ntool, if it is defined) will be set to Unknown."));

	float zmin, zmax;
	vtString text = (const char *) str.mb_str(wxConvUTF8);
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

	int count = 0;
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
			{
				grid->SetFValue(i, j, INVALID_ELEVATION);
				count++;
			}
		}
	}
	if (count)
	{
		wxString str;
		str.Printf(_("Set %d heixels to unknown"), count);
		wxMessageBox(str);
		t->SetModified(true);
		t->ReRender();
		m_pView->Refresh();
	}
}

void MainFrame::OnElevSetUnknown(wxCommandEvent &event)
{
	vtElevLayer *t = GetActiveElevLayer();
	if (!t)	return;
	vtElevationGrid *grid = t->m_pGrid;

	static float fValue = 1.0f;
	wxString str;
	str.Printf(_("%g"), fValue);
	str = wxGetTextFromUser(_("Set unknown areas to what value?"),
		_("Set Unknown Areas"), str, this);
	if (str == _T(""))
		return;

	fValue = atof(str.mb_str(wxConvUTF8));

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
	FillElevGaps(el);
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

	wxString str = wxGetTextFromUser(_("Please enter a scale factor"),
		_("Scale Elevation"), _T("1.0"), this);
	if (str == _T(""))
		return;

	float fScale;
	fScale = atof(str.mb_str(wxConvUTF8));
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

	wxString choices[11];
	choices[0] = _T("ArcInfo ASCII Grid");
	choices[1] = _T("GeoTIFF");
	choices[2] = _T("TerraGen");
	choices[3] = _T("BMP");
	choices[4] = _T("STM");
	choices[5] = _T("MSI Planet");
	choices[6] = _T("VRML ElevationGrid");
	choices[7] = _T("RAW/INF for MS Flight Simulator");
	choices[8] = _T("ChunkLOD (.chu)");
	choices[9] = _T("PNG (16-bit greyscale)");
	choices[10] = _T("3TX");

	wxSingleChoiceDialog dlg(this, _("Please choose"),
		_("Export to file format:"), 11, choices);
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
	case 7: ExportRAWINF(); break;
	case 8: ExportChunkLOD(); break;
	case 9: ExportPNG16(); break;
	case 10: Export3TX(); break;
	}
}

void MainFrame::OnElevExportTiles(wxCommandEvent& event)
{
	ElevExportTiles();
}

#if WIN32
#include "vtdata/LevellerTag.h"
static bool VerifyHFclip(BYTE* pMem, size_t size)
{
	// Verify the signature and endianness of received hf clipping.
	if (size < sizeof(daylon::TAG))
		return false;

	// The first two bytes must be 'hf'.
	if (::memcmp(pMem, "hf", 2) != 0)
		return false;

	// THe next two must be an int16 that equals 0x3031.
	unsigned __int16 endian = *((unsigned __int16*)(pMem+2));
	return endian == 0x3031;
}

#endif

//
// Create a new elevation layer by pasting data from the clipboard, using the
//  Daylone Leveller clipboard format for heightfields.
//
void MainFrame::ElevCopy()
{
#if WIN32
	UINT eFormat = ::RegisterClipboardFormat(_T("daylon_elev"));
	if (eFormat == 0)
	{
		wxMessageBox(_("Can't register clipboard format"));
		return;
	}

	vtElevLayer *pEL = GetActiveElevLayer();
	if (!pEL)
		return;
	vtElevationGrid *grid = pEL->m_pGrid;

	int cw, cb;
	grid->GetDimensions(cw, cb);

	/* We're going to make the following tags:

		hf01 : nil

		header
			version: ui32(0)

		body
			heixels
				extents
					width	ui32
					breadth	ui32

				format
					depth		ui32(32)
					fp			ui32(1)

				data		b()

			coordsys
			    geometry    ui32    (0=flat, 1=earth)
				projection
					format	ui32	(0=wkt if geometry=earth)
					data	b()		(the proj. string)
				pixelmapping			(raster-to-proj mapping)
					transform				(affine matrix)
						origin
							x	d
							z	d
						scale
							x	d
							z	d
				altitude			(assumes raw elevs are zero-based)
					units	ui32	(EPSG measure code; 9001=m, 9002=ft, 9003=sft, etc.)
					scale	d		(raw-to-units scaling)
					offset	d		(raw-to-units base)
			[
			alpha
				format
					depth	ui32

				data		b()
			]

		  5-7 parent tags  +7 (12-14)
		    1 nil tag
		  5-6 ui32 tags		+3 (8-9)
		    0 double tags   +6
		  1-2 binary tags   +1 (2-3)

	*/
	size_t nParentTags = 5+7;
	size_t nIntTags = 5+3;
	size_t nDblTags = 0+6;

	// Determine alpha tags needed.
	bool bAlpha = false;
	for (int i = 0; i < cw; i++)
	{
		for (int j = 0; j < cb; j++)
		{
			if (grid->GetValue(i, j) == INVALID_ELEVATION)
			{
				bAlpha = true;
				break;
			}
		}
	}

	if (bAlpha)
	{
		// There were void pixels.
		nParentTags++;	// "body/alpha"
		nParentTags++;	// "body/alpha/format"
		nIntTags++;		// "body/alpha/format/depth"
	}

	size_t clipSize = 0;

	daylon::CRootTag clip;

	clipSize += clip.CalcNormalStorage(1, daylon::VALKIND_NONE);

	clipSize += clip.CalcNormalStorage(nParentTags, daylon::VALKIND_NONE);
	clipSize += clip.CalcNormalStorage(nIntTags, daylon::VALKIND_UINT32);
	clipSize += clip.CalcNormalStorage(nDblTags, daylon::VALKIND_DOUBLE);
	// HF data.
	clipSize += clip.CalcBinaryTagStorage(cw * cb * sizeof(float));

	// Void data.
	if (bAlpha)
		clipSize += clip.CalcBinaryTagStorage(cw * cb * sizeof(unsigned char));

	// Projection string.
	char *wkt = NULL;
	m_proj.exportToWkt( &wkt );
	grid->GetProjection().exportToWkt(&wkt);
	vtString wkt_str = wkt;
	CPLFree(wkt);
	clipSize += clip.CalcBinaryTagStorage(wkt_str.GetLength());

	// Allocate.
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, clipSize);
	BYTE* pMem = (BYTE*)::GlobalLock(hMem);

	// Write data to clipboard.
	try
	{
		clip.SetStorage(pMem, clipSize);

		clip.Open("w");

		// Write ID and endianness.
		// This works because tag names are at the
		// top of the TAG structure, hence they
		// start the entire memory block.
		unsigned __int16 endian = 0x3031; //'01'
		char szEnd[] = { 'h', 'f', ' ', ' ', 0 };
		::memcpy(szEnd+2, &endian, sizeof(endian));
		clip.Write(szEnd, daylon::TAGRELATION_SIBLING);

		clip.WriteParent("header", true);
		clip.Write("version", (unsigned __int32)0, false);

		clip.WriteParent("body", false);
		clip.WriteParent("heixels", true);
		clip.WriteParent("extents", true);
		clip.Write("width", (daylon::uint32) cw, true);
		clip.Write("breadth", (daylon::uint32) cb, false);

		clip.WriteParent("format", true);
		clip.Write("depth", (unsigned __int32)(sizeof(float)*8), true);
		clip.Write("fp", (unsigned __int32)1, false);
		//clip.Dump();

		float* phv = (float*)
			clip.WriteBinary("data", false,
			cw * cb * sizeof(float));

		// Transfer selected heightfield pixels to clipboard.
		int x, z;
		for(z = 0; z < cb; z++)
		{
			for(x = 0; x < cw; x++)
			{
				*phv = grid->GetFValue(x, cb-1-z);
				phv++;
			}
		}

		// Tack on coordsys tag.
		clip.WriteParent("coordsys", bAlpha);
			clip.Write("geometry", (unsigned __int32)1, true);
			clip.WriteParent("projection", true);
				clip.Write("format", (unsigned __int32)0, true);
				unsigned char* pstrproj =
					(unsigned char*)clip.WriteBinary(
						"data", false,
					wkt_str.GetLength());
				::memcpy(pstrproj, (const char *)wkt_str, wkt_str.GetLength());

			DRECT &ext = grid->GetEarthExtents();
			double xp = ext.left;
			double yp = ext.top;
			double xscale = ext.Width() / (cw - 1);
			double zscale = -ext.Height() / (cb - 1);
			clip.WriteParent("pixelmapping", true);
				clip.WriteParent("transform", false);
					clip.WriteParent("origin", true);
						clip.Write("x", xp, true);
						clip.Write("z", yp, false);
					clip.WriteParent("scale", false);
						clip.Write("x", xscale, true);
						clip.Write("z", zscale, false);

			double fElevScale = grid->GetScale();
			double fOffset = 0.0f;
			clip.WriteParent("altitude", false);
				clip.Write("units", (daylon::uint32) 9001, true);
				clip.Write("scale", fElevScale, true);
				clip.Write("offset", fOffset, false);

		// Transfer any mask pixels to clipboard.
		if (bAlpha)
		{
			clip.WriteParent("alpha", false);

			clip.WriteParent("format", true);
			clip.Write("depth", (unsigned __int32)(sizeof(unsigned char)*8), false);

			unsigned char* pa = (unsigned char*)
				clip.WriteBinary("data", false,
					cw * cb * sizeof(unsigned char));

			for(z = 0; z < cb; z++)
			{
				for(x = 0; x < cw; x++)
				{
					*pa++ = (grid->GetValue(x, cb-1-z)==INVALID_ELEVATION ? 0 : 255);
				}
			}
		}
	}
	catch(...)
	{
		wxMessageBox(_T("Cannot place data on clipboard"));
	}
	clip.Close();
#if VTDEBUG
	VTLOG1("Copying grid to clipboard: ");
	clip.Dump();
#endif

  	::GlobalUnlock(hMem);

	if (::SetClipboardData(eFormat, hMem) == NULL)
	{
		DWORD err = ::GetLastError();
		VTLOG("Cannot put data on clipboard. Error %d", (int)err);
	}
	// Undo our allocation
	::GlobalFree(hMem);

#endif	// WIN32
}

//
// Create a new elevation layer by pasting data from the clipboard, using the
//  Daylone Leveller clipboard format for heightfields.
//
void MainFrame::ElevPasteNew()
{
#if WIN32
	UINT eFormat = ::RegisterClipboardFormat(_T("daylon_elev"));
	if (eFormat == 0)
	{
		wxMessageBox(_("Can't register clipboard format"));
		return;
	}
	// Get handle to clipboard data.
	HANDLE hMem = ::GetClipboardData(eFormat);
	if (hMem == NULL)
		return;
	void* pMem = (void*)::GlobalLock(hMem);
	if (pMem == NULL)
		return;
	size_t nbytes = ::GlobalSize(hMem);

	daylon::CRootTag clip;

	BYTE* pData = ((BYTE*)pMem) /*+ sizeof(kPublicHFclipID)*/;
	clip.SetStorage(pData, nbytes /*-
				sizeof(kPublicHFclipID)*/);
	if (!VerifyHFclip((BYTE*)pMem, nbytes))
		return;

	clip.Open("r");
	const int width = clip.ReadUINT32("body/heixels/extents/width", 0);
	const int breadth = clip.ReadUINT32("body/heixels/extents/breadth", 0);
	// See what format the elevations are in.
	// Lev 2.5, and DEMEdit support floating-point for now.
	const int bpp = clip.ReadUINT32("body/heixels/format/depth", 0);
	const bool bFP = (0 != clip.ReadUINT32("body/heixels/format/fp", 0));

	// Get any coordsys info. Ignore geometries other
	// than code 1 (planetary body, Earth). Ignore projection
	// formats other than WKT.

	vtProjection proj;
	DRECT area;
	float fElevScale = 1.0;

	const int geomcode =
		clip.ReadUINT32("body/coordsys/geometry", 0);
	const int projFmt =
		clip.ReadUINT32("body/coordsys/projection/format", 0);
	if (geomcode == 1 && projFmt == 0)
	{
		void* pv = NULL;
		size_t n =
			clip.Read("body/coordsys/projection/data", &pv);

		if (n != 0)
		{
			char* psz = new char[n + 1];
			memcpy(psz, pv, n);
			psz[n] = 0;
			//m_georef_info.set_projection(psz);
			char *wkt = psz;
			proj.importFromWkt(&wkt);
			delete psz;
		}

		// Read extents
		double d0 = clip.ReadDouble("body/coordsys/pixelmapping/transform/origin/x", 0.0);
		double d3 = clip.ReadDouble("body/coordsys/pixelmapping/transform/origin/z", 0.0);
		double d1 = clip.ReadDouble("body/coordsys/pixelmapping/transform/scale/x", 1.0);
		double d5 = clip.ReadDouble("body/coordsys/pixelmapping/transform/scale/z", 1.0);
		area.left = d0;
		area.right = d0 + (width-1) * d1;
		area.top = d3;
		area.bottom = d3 + (breadth-1) * d5;

		// Read vertical units and scale
		UINT units = clip.ReadUINT32("body/coordsys/altitude/units", 9001);
		fElevScale = clip.ReadDouble("body/coordsys/altitude/scale", 1.0);
		double offset = clip.ReadDouble("body/coordsys/altitude/offset", 0.0);
		switch (units)
		{
		case 9001: break;	// meter
		case 9002: fElevScale *= 0.3048f; break;	// foot
		case 9003: fElevScale *= (1200.0f/3937.0f); break; // U.S. survey foot
		}
	}

	// Create new layer
	vtElevLayer *pEL = new vtElevLayer(area, width, breadth, bFP, 1.0f, proj);

	// Copy the elevations.
	// Require packed pixel storage.
	float* pElevs;
	size_t n = clip.Read("body/heixels/data", (void**)&pElevs);
	int i = 0;
	for (int z = 0; z < breadth; z++)
	{
		for(int x = 0; x < width; x++, i++)
			pEL->m_pGrid->SetFValue(x, breadth-1-z, pElevs[i] * fElevScale);
	}
	pEL->m_pGrid->ComputeHeightExtents();
	GetMainFrame()->AddLayerWithCheck(pEL);

	::GlobalUnlock(hMem);
#endif	// WIN32
}

void MainFrame::OnElevCopy(wxCommandEvent& event)
{
#if WIN32
	int opened = ::OpenClipboard((HWND)GetHandle());
	if (opened)
	{
		if (!::EmptyClipboard())
		{
			::CloseClipboard();
			wxMessageBox(_T("Cannot empty clipboard."));
			return;
		}

		ElevCopy();
		::CloseClipboard();
	}
#endif	// WIN32
}

void MainFrame::OnElevPasteNew(wxCommandEvent& event)
{
#if WIN32
	int opened = ::OpenClipboard((HWND)GetHandle());
	if (opened)
	{
		ElevPasteNew();
		::CloseClipboard();
	}
#endif	// WIN32
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

void MainFrame::OnElevTrimTin(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TrimTIN);
	m_pView->SetCorrectCursor();
}

void MainFrame::OnUpdateElevTrimTin(wxUpdateUIEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	event.Enable(pEL && !pEL->IsGrid());
	event.Check(m_pView->GetMode() == LB_TrimTIN);
}



//////////////////////////////////////////////////////////////////////////
// Image Menu
//

void MainFrame::OnImageExportTiles(wxCommandEvent& event)
{
	ImageExportTiles();
}

void MainFrame::OnUpdateImageExportTiles(wxUpdateUIEvent& event)
{
	vtImageLayer *pIL = GetActiveImageLayer();
	event.Enable(pIL != NULL);
}

void MainFrame::OnImageExportPPM(wxCommandEvent& event)
{
	ImageExportPPM();
}

void MainFrame::OnUpdateImageExportPPM(wxUpdateUIEvent& event)
{
	vtImageLayer *pIL = GetActiveImageLayer();
	event.Enable(pIL != NULL);
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

void MainFrame::OnUpdateAreaMatch(wxUpdateUIEvent& event)
{
	int iRasters = LayersOfType(LT_ELEVATION) + LayersOfType(LT_IMAGE);
	event.Enable(!m_area.IsEmpty() && iRasters > 0);
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

void MainFrame::OnAreaMatch(wxCommandEvent &event)
{
	MatchDlg dlg(NULL, -1, _("Match Area and Tiling to Layer"));
	dlg.SetArea(m_area, (m_proj.IsGeographic() != 0));
	if (dlg.ShowModal() == wxID_OK)
	{
		m_tileopts.cols = dlg.m_tile.x;
		m_tileopts.rows = dlg.m_tile.y;
		m_tileopts.lod0size = dlg.m_iTileSize;
		m_area = dlg.m_area;
		m_pView->Refresh();
	}
	GetView()->HideGridMarks();
}

void MainFrame::OnAreaRequestWFS(wxCommandEvent& event)
{
#if SUPPORT_HTTP
	bool success;

	wxTextEntryDialog dlg(this, _T("WFS Server address"),
		_T("Please enter server base URL"), _T("http://10.254.0.29:8081/"));
	if (dlg.ShowModal() != wxID_OK)
		return;
	wxString value = dlg.GetValue();
	vtString server = (const char *) value.mb_str(wxConvUTF8);

	OGCLayerArray layers;
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

	vtRawLayer *pRL = new vtRawLayer;
	success = pRL->ReadFeaturesFromWFS(server, "rail");
	if (success)
		AddLayerWithCheck(pRL);
	else
		delete pRL;
#endif
}

void MainFrame::OnAreaRequestWMS(wxCommandEvent& event)
{
	if (m_wms_servers.empty())
	{
		// supply some hardcoded well-known servers
		OGCServer s;
		s.m_url = "http://wmt.jpl.nasa.gov/wms.cgi";
		m_wms_servers.push_back(s);
		s.m_url = "http://globe.digitalearth.gov/viz-bin/wmt.cgi";
		m_wms_servers.push_back(s);
	}

#if SUPPORT_HTTP
	// Ask the user for what server and layer they want
	MapServerDlg dlg(this, -1, _T("WMS Request"));

	dlg.m_area = m_area;
	dlg.m_proj = m_proj;
	dlg.SetServerArray(m_wms_servers);

	if (dlg.ShowModal() != wxID_OK)
		return;

	// Prepare to receive the WMS data
	if (dlg.m_bNewLayer)
	{
		// Enforce PNG, that's all we support so far
		dlg.m_iFormat = 1;	// png
		dlg.UpdateURL();
	}
	FILE *fp;
	wxString str;
	if (dlg.m_bToFile)
	{
		// Very simple: just write the buffer to disk
		fp = vtFileOpen(dlg.m_strToFile.mb_str(wxConvUTF8), "wb");
		if (!fp)
		{
			str = _("Could not open file");
			str += _T(" '");
			str += dlg.m_strToFile;
			str += _T("'");
			wxMessageBox(str);
			return;
		}
	}

	// Bring down the WMS data
	OpenProgressDialog(_("Requesting data"), false, this);
	ReqContext rc;
	rc.SetProgressCallback(progress_callback);
	vtBytes data;
	bool success = rc.GetURL(dlg.m_strQueryURL.mb_str(wxConvUTF8), data);
	CloseProgressDialog();

	if (!success)
	{
		str = wxString(rc.GetErrorMsg(), wxConvUTF8);	// the HTTP request failed
		wxMessageBox(str);
		return;
	}
	if (data.Len() > 5 && !strncmp((char *)data.Get(), "<?xml", 5))
	{
		// We got an XML-formatted response, not the image we were expecting.
		// The XML probably contains diagnostic error msg.
		// So show it to the user.
		unsigned char ch = 0;
		data.Append(&ch, 1);
		str = wxString((const char*) data.Get(), wxConvUTF8);
		wxMessageBox(str);
		return;
	}

	if (dlg.m_bNewLayer)
	{
		// Now data contains the PNG file in memory, so parse it.
		vtImageLayer *pIL = new vtImageLayer;
		success = pIL->ReadPNGFromMemory(data.Get(), data.Len());
		if (success)
		{
			pIL->SetExtent(m_area);
			pIL->SetProjection(m_proj);
			AddLayerWithCheck(pIL);
		}
		else
			delete pIL;
	}
	if (dlg.m_bToFile)
	{
		fwrite(data.Get(), data.Len(), 1, fp);
		fclose(fp);
	}
#endif
}

void MainFrame::OnUpdateAreaRequestWMS(wxUpdateUIEvent& event)
{
	event.Enable(!m_area.IsEmpty() && SUPPORT_HTTP);
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

	vtImageLayer *pIL = new vtImageLayer;
	bool success = pIL->ReadFeaturesFromTerraserver(m_area, dlg.m_iTheme,
		dlg.m_iMetersPerPixel, m_proj.GetUTMZone(), dlg.m_strToFile.mb_str(wxConvUTF8));

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
		pIL->SaveToFile(dlg.m_strToFile.mb_str(wxConvUTF8));
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
			filter, wxFD_OPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		wxString str = loadFile.GetPath();
		if (!LoadSpeciesFile(str.mb_str(wxConvUTF8)))
			return;
	}
	if (!m_SpeciesListDlg)
	{
		// Create new Plant List Dialog
		m_SpeciesListDlg = new SpeciesListDlg(this, wxID_ANY, _("Plants List"),
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
			filter, wxFD_OPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		// Read bioregions, data kept on frame with m_pBioRegion.
		wxString str = loadFile.GetPath();
		if (!LoadBiotypesFile(str.mb_str(wxConvUTF8)))
			return;
	}
	if (!m_BioRegionDlg)
	{
		// Create new Bioregion Dialog
		m_BioRegionDlg = new BioRegionDlg(this, wxID_ANY, _("BioRegions List"),
				wxPoint(120, 80), wxSize(300, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
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
	unsigned int i, n = list->NumSpecies();
	for (i = 0; i < n; i++)
	{
		vtPlantSpecies *spe = list->GetSpecies(i);
		wxString str(spe->GetSciName(), wxConvUTF8);
		choices.Add(str);
	}

	wxString result1 = wxGetSingleChoice(_("Remap FROM Species"), _T("Species"),
		choices, this);
	if (result1 == _T(""))	// cancelled
		return;
	short species_from = list->GetSpeciesIdByName(result1.mb_str(wxConvUTF8));

	wxString result2 = wxGetSingleChoice(_("Remap TO Species"), _T("Species"),
		choices, this);
	if (result2 == _T(""))	// cancelled
		return;
	short species_to = list->GetSpeciesIdByName(result2.mb_str(wxConvUTF8));

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
	wxString str;
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
		_("Vegetation Files (*.shp)|*.shp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	pVeg->ExportToSHP(strPathName.mb_str(wxConvUTF8));
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
		_("Vegetation Files (*.vf)|*.vf"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	DistribVegDlg dlg(this, -1, _("Vegetation Distribution Options"));

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	// Generate the plants
	GenerateVegetation(strPathName.mb_str(wxConvUTF8), m_area, dlg.m_opt);
}

void MainFrame::OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event)
{
	// we needs some species, and an area to plant them in
	event.Enable(m_strSpeciesFilename != "" && !m_area.IsEmpty());
}

void MainFrame::OnAreaVegDensity(wxCommandEvent& event)
{
	wxString str, s;

	LinearUnits lu = m_proj.GetUnits();
	float xsize = m_area.Width() * GetMetersPerUnit(lu);
	float ysize = m_area.Height() * GetMetersPerUnit(lu);
	float area = xsize * ysize;

	s.Printf(_("Total area: %.1f square meters (%.3f hectares)\n"), area, area/10000);
	str += s;

	// Get all the objects we'll need
	vtVegLayer *vlay = (vtVegLayer *) FindLayerOfType(LT_VEG);
	if (!vlay) return;
	vtPlantInstanceArray *pia = vlay->GetPIA();
	if (!pia) return;
	unsigned int ent = pia->GetNumEntities();
	vtSpeciesList *list = GetMainFrame()->GetPlantList();

	// Put the results in a biotype as well
	vtBioType btype;

	float size;
	short species;
	int total = 0;
	for (unsigned int i = 0; i < list->NumSpecies(); i++)
	{
		int count = 0;
		float height = 0;
		for (unsigned int j = 0; j < ent; j++)
		{
			pia->GetPlant(j, size, species);
			DPoint2 &p = pia->GetPoint(j);
			if (species == i && m_area.ContainsPoint(p))
			{
				total++;
				count++;
				height += size;
			}
		}
		if (count != 0)
		{
			vtPlantSpecies *spe = list->GetSpecies(i);
			float density = (float) count / area;
			s.Printf(_("  %d instances of species %hs: %.5f per m^2, average height %.1f\n"),
				count, spe->GetSciName(), density, height/count);
			str += s;

			btype.AddPlant(spe, density, height/count);
		}
	}
	s.Printf(_("Total plant instances: %d\n"), total);
	str += s;
	wxMessageBox(str, _("Info"));

	vtBioRegion bregion;
	btype.m_name = "Default";
	bregion.AddType(&btype);
	bregion.WriteXML("bioregion.xml");
	bregion.Clear();
}

void MainFrame::OnUpdateAreaVegDensity(wxUpdateUIEvent& event)
{
	// we needs some plants, and an area to estimate
	vtVegLayer *vlay = (vtVegLayer *) FindLayerOfType(LT_VEG);
	event.Enable(m_strSpeciesFilename != "" && vlay != NULL &&
		vlay->GetVegType() == VLT_Instances && !m_area.IsEmpty());
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

void MainFrame::OnStructureSelectUsingPolygons(wxCommandEvent &event)
{
	vtStructureLayer *pStructureLayer = GetActiveStructureLayer();

	if (NULL != pStructureLayer)
	{
		pStructureLayer->DeselectAll();

		int iNumLayers = m_Layers.GetSize();
		for (int i = 0; i < iNumLayers; i++)
		{
			vtLayer *pLayer = m_Layers.GetAt(i);
			if (LT_RAW == pLayer->GetType())
			{
				vtRawLayer* pRawLayer = dynamic_cast<vtRawLayer*>(pLayer);
				if ((NULL != pRawLayer) && (wkbPolygon == wkbFlatten(pRawLayer->GetGeomType())))
				{
					vtFeatureSetPolygon *pFeatureSetPolygon = dynamic_cast<vtFeatureSetPolygon*>(pRawLayer->GetFeatureSet());
					if (NULL != pFeatureSetPolygon)
					{
						unsigned int iNumEntities = pFeatureSetPolygon->GetNumEntities();
						unsigned int iIndex;
						for (iIndex = 0; iIndex < iNumEntities; iIndex++)
						{
							if (pFeatureSetPolygon->IsSelected(iIndex))
							{
								unsigned int iIndex2;
								const DPolygon2 Polygon = pFeatureSetPolygon->GetPolygon(iIndex);
								unsigned int iNumStructures = pStructureLayer->GetSize();
								for (iIndex2 = 0; iIndex2 < iNumStructures; iIndex2++)
								{
									DRECT Extents;
									if (pStructureLayer->GetAt(iIndex2)->GetExtents(Extents))
									{
										DPoint2 Point((Extents.left + Extents.right)/2, (Extents.bottom + Extents.top)/2);
										if (Polygon.ContainsPoint(Point))
											pStructureLayer->GetAt(iIndex2)->Select(true);
									}
								}
							}
						}
					}
				}
			}
		}
		m_pView->Refresh();
	}
}

void MainFrame::OnUpdateStructureSelectUsingPolygons(wxUpdateUIEvent &event)
{
	bool bFoundSelectedPolygons = false;
	int iNumLayers = m_Layers.GetSize();
	for (int i = 0; i < iNumLayers; i++)
	{
		vtLayer *pLayer = m_Layers.GetAt(i);
		if (LT_RAW == pLayer->GetType())
		{
			vtRawLayer* pRawLayer = dynamic_cast<vtRawLayer*>(pLayer);
			if ((NULL != pRawLayer) && (wkbPolygon == wkbFlatten(pRawLayer->GetGeomType())))
			{
				vtFeatureSet *pFeatureSet = pRawLayer->GetFeatureSet();
				if ((NULL != pFeatureSet) && (pFeatureSet->NumSelected() > 0))
				{
					bFoundSelectedPolygons = true;
					break;
				}
			}
		}
	}
	event.Enable(bFoundSelectedPolygons);
}

void MainFrame::OnStructureColourSelectedRoofs(wxCommandEvent& event)
{
	vtStructureLayer *pLayer = GetActiveStructureLayer();
	if (NULL != pLayer)
	{
		int iNumSelected = pLayer->NumSelected();
		if (iNumSelected > 0)
		{
			wxColour Colour = wxGetColourFromUser(this);
			if (Colour.Ok())
			{
				RGBi RoofColour(Colour.Red(), Colour.Green(), Colour.Blue());
				for (int i = 0; i < iNumSelected; i++)
				{
					vtStructure *pStructure = pLayer->GetAt(i);
					if (pStructure->IsSelected())
					{
						vtBuilding* pBuilding = pStructure->GetBuilding();
						if (NULL != pBuilding)
							pBuilding->GetLevel(pBuilding->GetNumLevels() - 1)->SetEdgeColor(RoofColour);
					}
				}
			}
		}
	}
}

void MainFrame::OnUpdateStructureColourSelectedRoofs(wxUpdateUIEvent& event)
{
	event.Enable((NULL != GetActiveStructureLayer()) && (GetActiveStructureLayer()->NumSelected() > 0));
}

void MainFrame::OnStructureExportFootprints(wxCommandEvent& event)
{
	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _("Export footprints to SHP"), _T(""), _T(""),
		_("SHP Files (*.shp)|*.shp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	vtStructureLayer *pLayer = GetActiveStructureLayer();
	pLayer->WriteFootprintsToSHP(strPathName.mb_str(wxConvUTF8));
}

void MainFrame::OnUpdateStructureExportFootprints(wxUpdateUIEvent& event)
{
	event.Enable(NULL != GetActiveStructureLayer());
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
		wxString name = pRL->GetLayerFilename();
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
	wxString str = wxGetTextFromUser(_("(X, Y) in current projection"),
			_("Enter coordinate"));
	if (str == _T(""))
		return;
	double x, y;
	int num = sscanf(str.mb_str(wxConvUTF8), "%lf, %lf", &x, &y);
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
		wxString str = dlg.m_strValue;
		int selected = pFS->SelectByCondition(dlg.m_iField, dlg.m_iCondition,
			str.mb_str(wxConvUTF8));

		wxString msg;
		if (selected == -1)
			msg = _("Unable to select");
		else if (selected == 1)
			msg.Printf(_("Selected 1 entity"));
		else
			msg.Printf(_("Selected %d entities"), selected);
		SetStatusText(msg);

		msg += _T("\n");
		VTLOG1(msg.mb_str(wxConvUTF8));

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
		FSTRING_PNG, wxFD_SAVE);
	if (loadFile.ShowModal() != wxID_OK)
		return;
	vtString fullname = (const char *) loadFile.GetPath().mb_str(wxConvUTF8);
	vtString filename = (const char *) loadFile.GetFilename().mb_str(wxConvUTF8);

	dib.WritePNG(fullname);

	// Then write imagemap
	wxFileDialog loadFile2(NULL, _("Save to Image File"), _T(""), _T(""),
		FSTRING_HTML, wxFD_SAVE);
	if (loadFile2.ShowModal() != wxID_OK)
		return;
	vtString htmlname = (const char *) loadFile2.GetPath().mb_str(wxConvUTF8);

	FILE *fp = vtFileOpen(htmlname, "wb");
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

void MainFrame::OnRawScale(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();

	wxString str = _T("1");
	str = wxGetTextFromUser(_("Scale factor?"),	_("Scale Raw Layer"),
		str, this);
	if (str == _T(""))
		return;

	double value = atof(str.mb_str(wxConvUTF8));
	pRL->Scale(value);
	m_pView->Refresh();
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
	str += _("Send feedback to: ");
	str += _T("ben@vterrain.org\n");
	str += _("Build date: ");
	str += wxString(__DATE__, wxConvUTF8);
	str += _T("\n");

#if defined(_MSC_VER) && defined(_DEBUG)	// == 1300 for VC7.1
	_CrtMemState state;
	_CrtMemCheckpoint(&state);
	int iAllocated = state.lSizes[1] + state.lSizes[2];
	wxString str3;
	str3.Printf(_T("Memory in use: %d bytes (%.0fK, %.1fMB)"), iAllocated,
		(float)iAllocated/1024, (float)iAllocated/1024/1024);
	str += _T("\n");
	str += str3;
#endif

	wxString str2 = _("About ");
	str2 += wxString(APPNAME, wxConvUTF8);
	wxMessageBox(str, str2);
#endif
}

void MainFrame::OnHelpDocLocal(wxCommandEvent &event)
{
	// Launch default web browser with documentation pages
	wxString wxcwd = wxGetCwd();
	vtString cwd = (const char *) wxcwd.mb_str(wxConvUTF8);

	VTLOG("OnHelpDocLocal: cwd is '%s'\n", (const char *) cwd);

	vtStringArray paths;
	paths.push_back(cwd + "/../Docs/VTBuilder/");
	paths.push_back(cwd + "/Docs/");
	vtString result = FindFileOnPaths(paths, "index.html");
	if (result == "")
	{
		wxMessageBox(_("Couldn't find local documentation files"));
		return;
	}
	vtString url;
	url.FormatForURL(result);
	url = "file:///" + url;
	VTLOG("Launching URL: %s\n", (const char *) url);
	wxLaunchDefaultBrowser(wxString(url, wxConvUTF8));
}

void MainFrame::OnHelpDocOnline(wxCommandEvent &event)
{
	// Launch default web browser with documentation pages
	wxLaunchDefaultBrowser(_T("http://vterrain.org/Doc/VTBuilder/"));
}

////////////////////
// Popup menu items

void MainFrame::OnDistanceClear(wxCommandEvent &event)
{
	ClearDistance();
}

