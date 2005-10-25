//
// Name:	 EnviroFrame.cpp
// Purpose:  The frame class for the wxEnviro application.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/progdlg.h"

#ifdef UNIX
#include <unistd.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/Contours.h"
#include "vtlib/core/DynTerrain.h"
#include "vtlib/core/Globe.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/SMTerrain.h"
#include "vtlib/core/SRTerrain.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/TiledGeom.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"	// for progress dialog

#include "EnviroFrame.h"
#include "StatusBar.h"

// dialogs
#include "BuildingDlg3d.h"
#include "CameraDlg.h"
#include "LayerDlg.h"
#include "LinearStructDlg3d.h"
#include "LocationDlg.h"
#include "LODDlg.h"
#include "PlantDlg.h"
#include "ScenarioSelectDialog.h"
#include "SceneGraphDlg.h"
#include "TimeDlg.h"
#include "UtilDlg.h"
#include "vtui/InstanceDlg.h"
#include "vtui/DistanceDlg.h"

#include "../Engines.h"
#include "../Options.h"
#include "EnviroGUI.h"	// for GetCurrentTerrain

#include "EnviroApp.h"
#include "canvas.h"
#include "menu_id.h"
#include "StatusBar.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "axes.xpm"
#  include "camera.xpm"
#  include "distance.xpm"
#  include "faster.xpm"
#  include "fence.xpm"
#  include "instances.xpm"
#  include "layers.xpm"
#  include "loc.xpm"
#  include "maintain.xpm"
#  include "move.xpm"
#  include "nav.xpm"
#  include "nav_fast.xpm"
#  include "nav_set.xpm"
#  include "nav_slow.xpm"
#  include "points.xpm"
#  include "route.xpm"
#  include "select.xpm"
#  include "sgraph.xpm"
#  include "snap.xpm"
#  include "snap_num.xpm"
#  include "space.xpm"
#  include "stop.xpm"
#  include "sun.xpm"
#  include "terrain.xpm"
#  include "tilt.xpm"
#  include "time.xpm"
#  include "tree.xpm"
#  include "unfold.xpm"
#endif

#if VTLIB_OSG
#include <osgDB/Registry>
#endif

DECLARE_APP(EnviroApp);

BEGIN_EVENT_TABLE(EnviroFrame, wxFrame)
EVT_CHAR(EnviroFrame::OnChar)
EVT_MENU(ID_FILE_LAYERS,		EnviroFrame::OnFileLayers)
EVT_MENU(wxID_EXIT, EnviroFrame::OnExit)
EVT_CLOSE(EnviroFrame::OnClose)
EVT_IDLE(EnviroFrame::OnIdle)

EVT_MENU(ID_TOOLS_SELECT,			EnviroFrame::OnToolsSelect)
EVT_UPDATE_UI(ID_TOOLS_SELECT,		EnviroFrame::OnUpdateToolsSelect)
EVT_MENU(ID_TOOLS_FENCES,			EnviroFrame::OnToolsFences)
EVT_UPDATE_UI(ID_TOOLS_FENCES,		EnviroFrame::OnUpdateToolsFences)
EVT_MENU(ID_TOOLS_ROUTES,			EnviroFrame::OnToolsRoutes)
EVT_UPDATE_UI(ID_TOOLS_ROUTES,		EnviroFrame::OnUpdateToolsRoutes)
EVT_MENU(ID_TOOLS_PLANTS,			EnviroFrame::OnToolsPlants)
EVT_UPDATE_UI(ID_TOOLS_PLANTS,		EnviroFrame::OnUpdateToolsPlants)
EVT_MENU(ID_TOOLS_INSTANCES,		EnviroFrame::OnToolsInstances)
EVT_UPDATE_UI(ID_TOOLS_INSTANCES,	EnviroFrame::OnUpdateToolsInstances)
//EVT_MENU(ID_TOOLS_MOVE,			EnviroFrame::OnToolsMove)
//EVT_UPDATE_UI(ID_TOOLS_MOVE,		EnviroFrame::OnUpdateToolsMove)
EVT_MENU(ID_TOOLS_NAVIGATE,			EnviroFrame::OnToolsNavigate)
EVT_UPDATE_UI(ID_TOOLS_NAVIGATE,	EnviroFrame::OnUpdateToolsNavigate)
EVT_MENU(ID_TOOLS_MEASURE,			EnviroFrame::OnToolsMeasure)
EVT_UPDATE_UI(ID_TOOLS_MEASURE,		EnviroFrame::OnUpdateToolsMeasure)

EVT_MENU(ID_VIEW_MAINTAIN,			EnviroFrame::OnViewMaintain)
EVT_UPDATE_UI(ID_VIEW_MAINTAIN,		EnviroFrame::OnUpdateViewMaintain)
EVT_MENU(ID_VIEW_WIREFRAME,			EnviroFrame::OnViewWireframe)
EVT_UPDATE_UI(ID_VIEW_WIREFRAME,	EnviroFrame::OnUpdateViewWireframe)
EVT_MENU(ID_VIEW_FULLSCREEN,		EnviroFrame::OnViewFullscreen)
EVT_UPDATE_UI(ID_VIEW_FULLSCREEN,	EnviroFrame::OnUpdateViewFullscreen)
EVT_MENU(ID_VIEW_TOPDOWN,			EnviroFrame::OnViewTopDown)
EVT_UPDATE_UI(ID_VIEW_TOPDOWN,		EnviroFrame::OnUpdateViewTopDown)
EVT_MENU(ID_VIEW_FRAMERATE,			EnviroFrame::OnViewFramerate)
EVT_UPDATE_UI(ID_VIEW_FRAMERATE,	EnviroFrame::OnUpdateViewFramerate)
EVT_MENU(ID_VIEW_ELEV_LEGEND,		EnviroFrame::OnViewElevLegend)
EVT_UPDATE_UI(ID_VIEW_ELEV_LEGEND,	EnviroFrame::OnUpdateViewElevLegend)
EVT_MENU(ID_VIEW_SETTINGS,			EnviroFrame::OnViewSettings)
EVT_MENU(ID_VIEW_LOCATIONS,			EnviroFrame::OnViewLocations)
EVT_UPDATE_UI(ID_VIEW_LOCATIONS,	EnviroFrame::OnUpdateViewLocations)
EVT_MENU(ID_VIEW_SNAPSHOT,			EnviroFrame::OnViewSnapshot)
EVT_MENU(ID_VIEW_SNAP_AGAIN,		EnviroFrame::OnViewSnapAgain)
EVT_MENU(ID_VIEW_STATUSBAR,			EnviroFrame::OnViewStatusBar)
EVT_UPDATE_UI(ID_VIEW_STATUSBAR,	EnviroFrame::OnUpdateViewStatusBar)
EVT_MENU(ID_VIEW_SCENARIOS,			EnviroFrame::OnViewScenarios)

EVT_MENU(ID_VIEW_SLOWER,		EnviroFrame::OnViewSlower)
EVT_UPDATE_UI(ID_VIEW_SLOWER,	EnviroFrame::OnUpdateViewSlower)
EVT_MENU(ID_VIEW_FASTER,		EnviroFrame::OnViewFaster)
EVT_UPDATE_UI(ID_VIEW_FASTER,	EnviroFrame::OnUpdateViewFaster)
EVT_MENU(ID_NAV_NORMAL,			EnviroFrame::OnNavNormal)
EVT_UPDATE_UI(ID_NAV_NORMAL,	EnviroFrame::OnUpdateNavNormal)
EVT_MENU(ID_NAV_VELO,			EnviroFrame::OnNavVelo)
EVT_UPDATE_UI(ID_NAV_VELO,		EnviroFrame::OnUpdateNavVelo)
EVT_MENU(ID_NAV_GRAB_PIVOT,		EnviroFrame::OnNavGrabPivot)
EVT_UPDATE_UI(ID_NAV_GRAB_PIVOT,EnviroFrame::OnUpdateNavGrabPivot)
EVT_MENU(ID_NAV_PANO,			EnviroFrame::OnNavPano)
EVT_UPDATE_UI(ID_NAV_PANO,		EnviroFrame::OnUpdateNavPano)

EVT_MENU(ID_SCENE_SCENEGRAPH,	EnviroFrame::OnSceneGraph)
EVT_MENU(ID_SCENE_TERRAIN,		EnviroFrame::OnSceneTerrain)
EVT_UPDATE_UI(ID_SCENE_TERRAIN,	EnviroFrame::OnUpdateSceneTerrain)
EVT_MENU(ID_SCENE_SPACE,		EnviroFrame::OnSceneSpace)
EVT_UPDATE_UI(ID_SCENE_SPACE,	EnviroFrame::OnUpdateSceneSpace)
#if VTLIB_OSG
EVT_MENU(ID_SCENE_SAVE,			EnviroFrame::OnSceneSave)
#endif
EVT_MENU(ID_TIME_DIALOG,		EnviroFrame::OnTimeDialog)
EVT_MENU(ID_TIME_STOP,			EnviroFrame::OnTimeStop)
EVT_MENU(ID_TIME_FASTER,		EnviroFrame::OnTimeFaster)

EVT_UPDATE_UI(ID_TIME_DIALOG,	EnviroFrame::OnUpdateInOrbitOrTerrain)
EVT_UPDATE_UI(ID_TIME_STOP,		EnviroFrame::OnUpdateInOrbitOrTerrain)
EVT_UPDATE_UI(ID_TIME_FASTER,	EnviroFrame::OnUpdateInOrbitOrTerrain)

EVT_MENU(ID_TERRAIN_DYNAMIC,	EnviroFrame::OnDynamic)
EVT_MENU(ID_TERRAIN_CULLEVERY,	EnviroFrame::OnCullEvery)
EVT_MENU(ID_TERRAIN_CULLONCE,	EnviroFrame::OnCullOnce)
EVT_MENU(ID_TERRAIN_SKY,		EnviroFrame::OnSky)
EVT_MENU(ID_TERRAIN_OCEAN,		EnviroFrame::OnOcean)
EVT_MENU(ID_TERRAIN_PLANTS,		EnviroFrame::OnPlants)
EVT_MENU(ID_TERRAIN_STRUCTURES,	EnviroFrame::OnStructures)
EVT_MENU(ID_TERRAIN_ROADS,		EnviroFrame::OnRoads)
EVT_MENU(ID_TERRAIN_FOG,		EnviroFrame::OnFog)
EVT_MENU(ID_TERRAIN_INCREASE,	EnviroFrame::OnIncrease)
EVT_MENU(ID_TERRAIN_DECREASE,	EnviroFrame::OnDecrease)
EVT_MENU(ID_TERRAIN_LOD,		EnviroFrame::OnLOD)
EVT_MENU(ID_TERRAIN_SAVEVEG,	EnviroFrame::OnSaveVeg)
EVT_MENU(ID_TERRAIN_SAVESTRUCT,	EnviroFrame::OnSaveStruct)
EVT_MENU(ID_TERRAIN_FOUNDATIONS, EnviroFrame::OnToggleFoundations)
EVT_MENU(ID_TERRAIN_RESHADE,	EnviroFrame::OnTerrainReshade)

EVT_UPDATE_UI(ID_TERRAIN_DYNAMIC,	EnviroFrame::OnUpdateDynamic)
EVT_UPDATE_UI(ID_TERRAIN_CULLEVERY, EnviroFrame::OnUpdateCullEvery)
EVT_UPDATE_UI(ID_TERRAIN_SKY,		EnviroFrame::OnUpdateSky)
EVT_UPDATE_UI(ID_TERRAIN_OCEAN,		EnviroFrame::OnUpdateOcean)
EVT_UPDATE_UI(ID_TERRAIN_PLANTS,	EnviroFrame::OnUpdatePlants)
EVT_UPDATE_UI(ID_TERRAIN_STRUCTURES, EnviroFrame::OnUpdateStructures)
EVT_UPDATE_UI(ID_TERRAIN_ROADS,		EnviroFrame::OnUpdateRoads)
EVT_UPDATE_UI(ID_TERRAIN_FOG,		EnviroFrame::OnUpdateFog)
EVT_UPDATE_UI(ID_TERRAIN_INCREASE,	EnviroFrame::OnUpdateLOD)
EVT_UPDATE_UI(ID_TERRAIN_DECREASE,	EnviroFrame::OnUpdateLOD)
EVT_UPDATE_UI(ID_TERRAIN_LOD,		EnviroFrame::OnUpdateLOD)
EVT_UPDATE_UI(ID_TERRAIN_FOUNDATIONS, EnviroFrame::OnUpdateFoundations)

EVT_MENU(ID_EARTH_SHOWSHADING,	EnviroFrame::OnEarthShowShading)
EVT_MENU(ID_EARTH_SHOWAXES,		EnviroFrame::OnEarthShowAxes)
EVT_MENU(ID_EARTH_TILT,			EnviroFrame::OnEarthTilt)
EVT_MENU(ID_EARTH_FLATTEN,		EnviroFrame::OnEarthFlatten)
EVT_MENU(ID_EARTH_UNFOLD,		EnviroFrame::OnEarthUnfold)
EVT_MENU(ID_EARTH_POINTS,		EnviroFrame::OnEarthPoints)

EVT_UPDATE_UI(ID_EARTH_SHOWSHADING, EnviroFrame::OnUpdateEarthShowShading)
EVT_UPDATE_UI(ID_EARTH_SHOWAXES, EnviroFrame::OnUpdateEarthShowAxes)
EVT_UPDATE_UI(ID_EARTH_TILT,	EnviroFrame::OnUpdateEarthTilt)
EVT_UPDATE_UI(ID_EARTH_FLATTEN, EnviroFrame::OnUpdateEarthFlatten)
EVT_UPDATE_UI(ID_EARTH_UNFOLD,	EnviroFrame::OnUpdateEarthUnfold)
EVT_UPDATE_UI(ID_EARTH_POINTS,	EnviroFrame::OnUpdateInOrbit)

EVT_MENU(ID_HELP_ABOUT, EnviroFrame::OnHelpAbout)

// Popup
EVT_MENU(ID_POPUP_PROPERTIES, EnviroFrame::OnPopupProperties)
EVT_MENU(ID_POPUP_FLIP, EnviroFrame::OnPopupFlip)
EVT_MENU(ID_POPUP_RELOAD, EnviroFrame::OnPopupReload)
EVT_MENU(ID_POPUP_DELETE, EnviroFrame::OnPopupDelete)
END_EVENT_TABLE()


//
// Frame constructor
//
EnviroFrame::EnviroFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style, bool bVerticalToolbar, bool bEnableEarth):
		wxFrame(parent, -1, title, pos, size, style)
{
	m_bCloseOnIdle = false;

	// Give it an icon
	SetIcon(wxIcon(_T(ICON_NAME)));

	m_bCulleveryframe = true;
	m_bAlwaysMove = false;
	m_bFullscreen = false;
	m_bTopDown = false;
	m_ToggledMode = MM_SELECT;
	m_bEnableEarth = bEnableEarth;
	m_bEarthLines = false;

	m_pStatusBar = NULL;
	m_pToolbar = NULL;

	VTLOG("Frame window: creating menus and toolbars.\n");
	CreateMenus();
	CreateToolbar(bVerticalToolbar);	// argument: vertical

	// Create StatusBar
	m_pStatusBar = new MyStatusBar(this);
	SetStatusBar(m_pStatusBar);
	m_pStatusBar->Show();
	m_pStatusBar->UpdateText();
	PositionStatusBar();

	// We definitely want full color and a 24-bit Z-buffer!
	int gl_attrib[8] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
		WX_GL_BUFFER_SIZE, 24, WX_GL_DEPTH_SIZE, 24, 0, 0 };
	if (g_Options.m_bStereo && g_Options.m_iStereoMode == 1)
	{
		gl_attrib[6] = WX_GL_STEREO;
		gl_attrib[7] = 0;
	}

	VTLOG("Frame window: creating view canvas.\n");
	m_canvas = new vtGLCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
			_T("vtGLCanvas"), gl_attrib);

	// Show the frame
	Show(true);

	m_pBuildingDlg = new BuildingDlg3d(this, -1, _("Building Properties"));
	m_pCameraDlg = new CameraDlg(this, -1, _("Camera-View"));
	m_pDistanceDlg = new DistanceDlg(this, -1, _("Distance"));
	m_pFenceDlg = new LinearStructureDlg3d(this, -1, _("Linear Structures"));
	m_pInstanceDlg = new InstanceDlg(this, -1, _("Instances"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pLODDlg = new LODDlg(this, -1, _("Terrain LOD Info"));

	m_pPlantDlg = new PlantDlg(this, -1, _("Plants"));
	m_pPlantDlg->ShowOnlyAvailableSpecies(g_Options.m_bOnlyAvailableSpecies);

	m_pLocationDlg = new LocationDlg(this, -1, _("Locations"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pLayerDlg = new LayerDlg(this, -1, _("Layers"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pLayerDlg->SetSize(600, 250);
	m_pSceneGraphDlg = new SceneGraphDlg(this, -1, _("Scene Graph"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(450, 600);
	m_pTimeDlg = new TimeDlg(this, -1, _("Time"));
	m_pUtilDlg = new UtilDlg(this, -1, _("Routes"));
	m_pScenarioSelectDialog = new CScenarioSelectDialog(this, -1, _("Scenarios"));

	m_canvas->SetCurrent();
}

EnviroFrame::~EnviroFrame()
{
	VTLOG("Deleting Frame\n");
	delete m_canvas;
	delete m_pSceneGraphDlg;
	delete m_pPlantDlg;
	delete m_pFenceDlg;
	delete m_pUtilDlg;
	delete m_pCameraDlg;
	delete m_pLocationDlg;
	delete m_pInstanceDlg;
	delete m_pLayerDlg;

	delete m_pStatusBar;
	delete m_pToolbar;
	SetStatusBar(NULL);
	SetToolBar(NULL);
}


void EnviroFrame::CreateMenus()
{
	// Make menu bar
	m_pMenuBar = new wxMenuBar;

	// Make menus
	m_pFileMenu = new wxMenu;
	m_pFileMenu->Append(ID_FILE_LAYERS, _("Layers"), _("Layers"));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(wxID_EXIT, _("E&xit (Esc)"), _("Exit"));
	m_pMenuBar->Append(m_pFileMenu, _("&File"));

	m_pToolsMenu = new wxMenu;
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_SELECT, _("Select"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_FENCES, _("Fences"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_ROUTES, _("Routes"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_PLANTS, _("Plants"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_INSTANCES, _("Instances"));
//	m_pToolsMenu->AppendCheckItem(ID_TOOLS_MOVE, _("Move Objects"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_NAVIGATE, _("Navigate"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_MEASURE, _("Measure Distances\tCtrl+D"));
	m_pMenuBar->Append(m_pToolsMenu, _("&Tools"));

	// shortcuts:
	// Ctrl+A Show Axes
	// Ctrl+C Cull Every Frame
	// Ctrl+D Measure Distances
	// Ctrl+E Flatten
	// Ctrl+F Fullscreen
	// Ctrl+G Go to Terrain
	// Ctrl+K Cull Once
	// Ctrl+I Time
	// Ctrl+L Store/Recall Locations
	// Ctrl+N Save Numbered Snapshot
	// Ctrl+P Load Point Data
	// Ctrl+Q Terrain LOD Info
	// Ctrl+S Camera - View Settings
	// Ctrl+T Top-Down
	// Ctrl+U Unfold
	// Ctrl+W Wireframe
	// Ctrl+Z Framerate Chart
	// A Maintain height above ground
	// D Toggle Grab-Pivot
	// F Faster
	// S Faster

	if (m_bEnableEarth)
	{
		m_pSceneMenu = new wxMenu;
		m_pSceneMenu->Append(ID_SCENE_SCENEGRAPH, _("Scene Graph"));
		m_pSceneMenu->AppendSeparator();
		m_pSceneMenu->Append(ID_SCENE_TERRAIN, _("Go to Terrain...\tCtrl+G"));
		m_pSceneMenu->Append(ID_SCENE_SPACE, _("Go to Space"));
#if VTLIB_OSG
		m_pSceneMenu->AppendSeparator();
		m_pSceneMenu->Append(ID_SCENE_SAVE, _("Save scene graph to .osg"));
#endif
		m_pSceneMenu->AppendSeparator();
		m_pSceneMenu->Append(ID_TIME_DIALOG, _("Time...\tCtrl+I"));
		m_pSceneMenu->Append(ID_TIME_STOP, _("Time Stop"));
		m_pSceneMenu->Append(ID_TIME_FASTER, _("Time Faster"));
		m_pMenuBar->Append(m_pSceneMenu, _("&Scene"));
	}

	m_pViewMenu = new wxMenu;
	m_pViewMenu->AppendCheckItem(ID_VIEW_WIREFRAME, _("Wireframe\tCtrl+W"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_FULLSCREEN, _("Fullscreen\tCtrl+F"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_TOPDOWN, _("Top-Down Camera\tCtrl+T"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_FRAMERATE, _("Framerate Chart\tCtrl+Z"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_ELEV_LEGEND, _("Elevation Legend"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->Append(ID_VIEW_SETTINGS, _("Camera - View Settings\tCtrl+S"));
	m_pViewMenu->Append(ID_VIEW_LOCATIONS, _("Store/Recall Locations\tCtrl+L"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->Append(ID_VIEW_SNAPSHOT, _("Save Window Snapshot"));
	m_pViewMenu->Append(ID_VIEW_SNAP_AGAIN, _("Save Numbered Snapshot\tCtrl+N"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Status Bar"));
	m_pViewMenu->Append(ID_VIEW_SCENARIOS, _("Scenarios"));
	m_pMenuBar->Append(m_pViewMenu, _("&View"));

	m_pNavMenu = new wxMenu;
	m_pNavMenu->Append(ID_VIEW_SLOWER, _("Fly Slower (S)"));
	m_pNavMenu->Append(ID_VIEW_FASTER, _("Fly Faster (F)"));
	m_pNavMenu->AppendCheckItem(ID_VIEW_MAINTAIN, _("Maintain height above ground (A)"));
	m_pMenuBar->Append(m_pNavMenu, _("&Navigate"));

		// submenu
		wxMenu *navstyleMenu = new wxMenu;
		navstyleMenu->AppendCheckItem(ID_NAV_NORMAL, _("Normal Terrain Flyer"));
		navstyleMenu->AppendCheckItem(ID_NAV_VELO, _("Flyer with Velocity"));
		navstyleMenu->AppendCheckItem(ID_NAV_GRAB_PIVOT, _("Grab-Pivot"));
//		navstyleMenu->AppendCheckItem(ID_NAV_QUAKE, _T("Keyboard Walk"));
		navstyleMenu->AppendCheckItem(ID_NAV_PANO, _("Panoramic Flyer"));
		m_pNavMenu->Append(0, _("Navigation Style"), navstyleMenu);

	m_pTerrainMenu = new wxMenu;
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_DYNAMIC, _("LOD Terrain Surface\tF3"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_CULLEVERY, _("Cull every frame\tCtrl+C"));
	m_pTerrainMenu->Append(ID_TERRAIN_CULLONCE, _("Cull once\tCtrl+K"));
	m_pTerrainMenu->AppendSeparator();
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_SKY, _("Show Sky\tF4"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_OCEAN, _("Show Ocean\tF5"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_PLANTS, _("Show Plants\tF6"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_STRUCTURES, _("Show Structures\tF7"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_ROADS, _("Show Roads\tF8"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_FOG, _("Show Fog\tF9"));
	m_pTerrainMenu->AppendSeparator();
	m_pTerrainMenu->Append(ID_TERRAIN_INCREASE, _("Increase Detail (+)"));
	m_pTerrainMenu->Append(ID_TERRAIN_DECREASE, _("Decrease Detail (-)"));
	m_pTerrainMenu->Append(ID_TERRAIN_LOD, _("Level of Detail Info\tCtrl+Q"));
	m_pTerrainMenu->AppendSeparator();
	m_pTerrainMenu->Append(ID_TERRAIN_SAVEVEG, _("Save Vegetation As..."));
	m_pTerrainMenu->Append(ID_TERRAIN_SAVESTRUCT, _("Save Built Structures As..."));
	m_pTerrainMenu->AppendSeparator();
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_FOUNDATIONS, _("Toggle Artificial Foundations"));
	m_pTerrainMenu->Append(ID_TERRAIN_RESHADE, _("&Recalculate Shading\tCtrl+R"));
	m_pMenuBar->Append(m_pTerrainMenu, _("Te&rrain"));

	if (m_bEnableEarth)
	{
		m_pEarthMenu = new wxMenu;
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWSHADING, _("&Show Shading"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWAXES, _("Show &Axes\tCtrl+A"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_TILT, _("Seasonal &Tilt"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_FLATTEN, _("&Flatten\tCtrl+E"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_UNFOLD, _("&Unfold\tCtrl+U"));
		m_pEarthMenu->Append(ID_EARTH_POINTS, _("&Load Point Data...\tCtrl+P"));
		m_pMenuBar->Append(m_pEarthMenu, _("&Earth"));
	}

	wxMenu *helpMenu = new wxMenu;
	wxString about = _("About");
	about += _T(" ");
	about += WSTRING_APPORG;
	about += _T("...");
	helpMenu->Append(ID_HELP_ABOUT, about);
	m_pMenuBar->Append(helpMenu, _("&Help"));

	SetMenuBar(m_pMenuBar);
}

void EnviroFrame::CreateToolbar(bool bVertical)
{
	SetToolBar(NULL);

	long style = wxNO_BORDER | wxTB_DOCKABLE;
	if (bVertical)
		style |= wxTB_VERTICAL;
	else
		style |= wxTB_HORIZONTAL;

	if (m_pToolbar != NULL)
		delete m_pToolbar;

	// tool bar
	m_pToolbar = CreateToolBar(style);
	m_pToolbar->SetMargins(2, 2);
	m_pToolbar->SetToolBitmapSize(wxSize(20, 20));

	ADD_TOOL(ID_TOOLS_SELECT, wxBITMAP(select), _("Select"), true);
	ADD_TOOL(ID_TOOLS_FENCES, wxBITMAP(fence), _("Create Fences"), true);
	ADD_TOOL(ID_TOOLS_ROUTES, wxBITMAP(route), _("Create Routes"), true);
	ADD_TOOL(ID_TOOLS_PLANTS, wxBITMAP(tree), _("Create Plants"), true);
//	ADD_TOOL(ID_TOOLS_MOVE, wxBITMAP(move), _("Move Objects"), true);	// not yet
	ADD_TOOL(ID_TOOLS_INSTANCES, wxBITMAP(instances), _("Create Instances"), true);
	ADD_TOOL(ID_TOOLS_NAVIGATE, wxBITMAP(nav), _("Navigate"), true);
	ADD_TOOL(ID_TOOLS_MEASURE, wxBITMAP(distance), _("Measure Distance"), true);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_VIEW_MAINTAIN, wxBITMAP(maintain), _("Maintain Height"), true);
	ADD_TOOL(ID_VIEW_FASTER, wxBITMAP(nav_fast), _("Fly Faster"), false);
	ADD_TOOL(ID_VIEW_SLOWER, wxBITMAP(nav_slow), _("Fly Slower"), false);
	ADD_TOOL(ID_VIEW_SETTINGS, wxBITMAP(nav_set), _("Camera Dialog"), false);
	ADD_TOOL(ID_VIEW_LOCATIONS, wxBITMAP(loc), _("Locations"), false);
	ADD_TOOL(ID_VIEW_SNAPSHOT, wxBITMAP(snap), _("Snapshot"), false);
	ADD_TOOL(ID_VIEW_SNAP_AGAIN, wxBITMAP(snap_num), _("Numbered Snapshot"), false);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_FILE_LAYERS, wxBITMAP(layers), _("Show Layer Dialog"), false);

	if (m_bEnableEarth)
	{
		m_pToolbar->AddSeparator();
		ADD_TOOL(ID_SCENE_SPACE, wxBITMAP(space), _("Go to Space"), false);
		ADD_TOOL(ID_SCENE_TERRAIN, wxBITMAP(terrain), _("Go to Terrain"), false);
		m_pToolbar->AddSeparator();
		ADD_TOOL(ID_EARTH_SHOWSHADING, wxBITMAP(sun), _("Show Sunlight"), true);
		ADD_TOOL(ID_EARTH_SHOWAXES, wxBITMAP(axes), _("Axes"), true);
		ADD_TOOL(ID_EARTH_TILT, wxBITMAP(tilt), _("Tilt"), true);
		ADD_TOOL(ID_EARTH_POINTS, wxBITMAP(points), _("Add Point Data"), false);
		ADD_TOOL(ID_EARTH_UNFOLD, wxBITMAP(unfold), _("Unfold"), true);
	}
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_TIME_DIALOG, wxBITMAP(time), _("Time"), false);
	ADD_TOOL(ID_TIME_FASTER, wxBITMAP(faster), _("Time Faster"), false);
	ADD_TOOL(ID_TIME_STOP, wxBITMAP(stop), _("Time Stop"), false);

	if (m_bEnableEarth)
	{
		m_pToolbar->AddSeparator();
		ADD_TOOL(ID_SCENE_SCENEGRAPH, wxBITMAP(sgraph), _("Scene Graph"), false);
	}

//	m_pToolbar->SetRows(32);
	m_pToolbar->Realize();
//	SendSizeEvent();
//	m_pToolbar->Refresh();
}

void EnviroFrame::Setup3DScene()
{
}

//
// Utility methods
//

void EnviroFrame::SetMode(MouseMode mode)
{
	// Show/hide the modeless dialogs as appropriate
	m_pFenceDlg->Show(mode == MM_FENCES);
	m_pUtilDlg->Show(mode == MM_ROUTES);
	m_pInstanceDlg->Show(mode == MM_INSTANCES);
	m_pDistanceDlg->Show(mode == MM_MEASURE);

	g_App.SetMode(mode);

	// Show/hide plant dialog
	if (mode == MM_PLANTS)
	{
		m_pPlantDlg->SetPlantList(g_App.GetPlantList());
		m_pPlantDlg->SetPlantOptions(g_App.GetPlantOptions());
		m_pPlantDlg->Show(true);
	}
	else
		m_pPlantDlg->Show(false);
}

vtTransform *contourtrans = NULL;

void EnviroFrame::OnChar(wxKeyEvent& event)
{
	static NavType prev = NT_Normal;
	vtTerrain *pTerr = GetCurrentTerrain();
	long key = event.GetKeyCode();

	// Keyboard shortcuts ("accelerators")
	switch (key)
	{
	case 27:
		// Esc: exit application
		// It's not safe to close immediately, as that will kill the canvas,
		//  and it might some Canvas event that caused us to close.  So,
		//  simply stop rendering, and delay closing until the next Idle event.
		m_canvas->m_bRunning = false;
		m_bCloseOnIdle = true;
		break;

	case ' ':
		if (g_App.m_state == AS_Terrain)
			ToggleNavigate();
		break;

	case 'f':
		ChangeFlightSpeed(1.8f);
		break;
	case 's':
		ChangeFlightSpeed(1.0f / 1.8f);
		break;
	case 'a':
		g_App.SetMaintain(!g_App.GetMaintain());
		break;

	case '+':
		ChangeTerrainDetail(true);
		break;
	case '-':
		ChangeTerrainDetail(false);
		break;

	case 'd':
		// Toggle grab-pivot
		if (g_App.m_nav == NT_Grab)
			g_App.SetNavType(prev);
		else
		{
			prev = g_App.m_nav;
			g_App.SetNavType(NT_Grab);
		}
		break;

	case 'w':
		m_bAlwaysMove = !m_bAlwaysMove;
		if (g_App.m_pTFlyer != NULL)
			g_App.m_pTFlyer->SetAlwaysMove(m_bAlwaysMove);
		break;

	case '[':
		{
			float exag = pTerr->GetVerticalExag();
			exag /= 1.01;
			pTerr->SetVerticalExag(exag);
			if (contourtrans)
			{
				contourtrans->Identity();
				contourtrans->Scale3(1, exag, 1);
			}
		}
		break;
	case ']':
		{
			float exag = pTerr->GetVerticalExag();
			exag *= 1.01;
			pTerr->SetVerticalExag(exag);
			if (contourtrans)
			{
				contourtrans->Identity();
				contourtrans->Scale3(1, exag, 1);
			}
		}
		break;

	case 'e':
		m_bEarthLines = !m_bEarthLines;
		g_App.ShowEarthLines(m_bEarthLines);
		break;

	case 'z':
		// A handy place to put test code
#if 0
		if (pTerr && g_App.m_bSelectedStruct)
		{
			vtStructureArray3d *sa = pTerr->GetStructures();
			int i = 0;
			while (!sa->GetAt(i)->IsSelected())
				i++;
			vtBuilding3d *bld = sa->GetBuilding(i);
			// (Do something to the building as a test)
			sa->ConstructStructure(bld);
		}
#endif
#if SUPPORT_QUIKGRID
		if (pTerr)
		{
			ContourConverter cc;
			contourtrans = cc.Setup(pTerr, RGBf(1,1,0), 10);
			cc.GenerateContours(500);
			cc.Finish();
		}
#endif
		break;

	case 'D':	// Ctrl-D
		// dump camera info
		g_App.DumpCameraInfo();
		break;

	case 2:	// Ctrl-B
		// toggle logo
		g_App.ToggleLogo();
		break;

	case WXK_F12:
		m_pSceneGraphDlg->Show(true);
		break;

	default:
		event.Skip();
		break;
	}
}

void EnviroFrame::ToggleNavigate()
{
	MouseMode current = g_App.m_mode;
	if (current == MM_NAVIGATE && m_ToggledMode != MM_NAVIGATE)
		SetMode(m_ToggledMode);
	else
	{
		m_ToggledMode = current;
		SetMode(MM_NAVIGATE);
	}
}

void EnviroFrame::ChangeFlightSpeed(float factor)
{
	float speed = g_App.GetFlightSpeed();
	g_App.SetFlightSpeed(speed * factor);

	VTLOG("Change flight speed to %f\n", speed * factor);

	m_pCameraDlg->GetValues();
	m_pCameraDlg->ValuesToSliders();
	m_pCameraDlg->TransferToWindow();
	m_pCameraDlg->Refresh();
}

void EnviroFrame::ChangeTerrainDetail(bool bIncrease)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr) return;
	vtDynTerrainGeom *pDyn = pTerr->GetDynTerrain();
	if (pDyn)
	{
		if (bIncrease)
			pDyn->SetPolygonTarget(pDyn->GetPolygonTarget()+1000);
		else
			pDyn->SetPolygonTarget(pDyn->GetPolygonTarget()-1000);
	}
	vtTiledGeom *pTiled = pTerr->GetTiledGeom();
	if (pTiled)
	{
		if (bIncrease)
			pTiled->SetVertexTarget(pTiled->GetVertexTarget()+1000);
		else
			pTiled->SetVertexTarget(pTiled->GetVertexTarget()-1000);
	}
}

void EnviroFrame::SetFullScreen(bool bFull)
{
	m_bFullscreen = bFull;
#ifdef __WXMSW__
	if (m_bFullscreen)
	{
		ShowFullScreen(true, wxFULLSCREEN_NOMENUBAR |
//							 wxFULLSCREEN_NOTOOLBAR |	// leave toolbar visible
			wxFULLSCREEN_NOSTATUSBAR |
			wxFULLSCREEN_NOBORDER |
			wxFULLSCREEN_NOCAPTION );
	}
	else
		ShowFullScreen(false);
#else
  /*  FIXME - ShowFullScreen not implemented in wxGTK 2.2.5.  */
  /*   Do full-screen another way.                           */
#endif
}

//
// Intercept menu commands
//

void EnviroFrame::OnExit(wxCommandEvent& event)
{
	VTLOG("Got Exit event, shutting down.\n");
	if (m_canvas)
	{
		m_canvas->m_bRunning = false;
		delete m_canvas;
		m_canvas = NULL;
	}
	Destroy();
}

void EnviroFrame::OnClose(wxCloseEvent &event)
{
	VTLOG("Got Close event, shutting down.\n");
	if (m_canvas)
	{
		m_canvas->m_bRunning = false;
		delete m_canvas;
		m_canvas = NULL;
	}
	event.Skip();
}

void EnviroFrame::OnIdle(wxIdleEvent& event)
{
	// Check if we were requested to close on the next Idle event.
	if (m_bCloseOnIdle)
		Close();
	else
		event.Skip();
}

void EnviroFrame::OnHelpAbout(wxCommandEvent& event)
{
	EnableContinuousRendering(false);

 	wxString str = WSTRING_APPORG _T("\n\n");
#ifdef ENVIRO_NATIVE
	str += _T("The runtime environment for the Virtual Terrain Project.\n\n");
 	str += _T("Please read the HTML documentation and license.\n\n");
 	str += _T("Send feedback to: ben@vterrain.org\n");
#else
	str += _T("Based on the Virtual Terrain Project 3D Runtime Environment.\n");
#endif
 	str += _T("\nThis version was built with the ");
#if VTLIB_DSM
 	str += _T("DSM");
#elif VTLIB_OSG
 	str += _T("OSG");
#elif VTLIB_SGL
 	str += _T("SGL");
#elif VTLIB_SSG
 	str += _T("SSG");
#endif
 	str += _T(" Library.\n\n");
	str += _T("Build date: ");
	str += _T(__DATE__);
	wxMessageBox(str, _T("About ") WSTRING_APPORG);

	EnableContinuousRendering(true);
}


//////////////////// File menu //////////////////////////

void EnviroFrame::OnFileLayers(wxCommandEvent& event)
{
	m_pLayerDlg->Show(true);
}


//////////////////// View menu //////////////////////////

void EnviroFrame::OnViewMaintain(wxCommandEvent& event)
{
	g_App.SetMaintain(!g_App.GetMaintain());
}

void EnviroFrame::OnUpdateViewMaintain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.GetMaintain());
}

void EnviroFrame::OnNavNormal(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Normal);
}

void EnviroFrame::OnUpdateNavNormal(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Normal);
}

void EnviroFrame::OnNavVelo(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Velo);
}

void EnviroFrame::OnUpdateNavVelo(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Velo);
}

void EnviroFrame::OnNavGrabPivot(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Grab);
}

void EnviroFrame::OnUpdateNavGrabPivot(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Grab);
}

void EnviroFrame::OnNavPano(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Pano);
}

void EnviroFrame::OnUpdateNavPano(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Pano);
}

void EnviroFrame::OnViewWireframe(wxCommandEvent& event)
{
	vtGetScene()->SetGlobalWireframe(!vtGetScene()->GetGlobalWireframe());
}

void EnviroFrame::OnUpdateViewWireframe(wxUpdateUIEvent& event)
{
	event.Check(vtGetScene()->GetGlobalWireframe());
}

void EnviroFrame::OnViewFullscreen(wxCommandEvent& event)
{
	SetFullScreen(!m_bFullscreen);
}

void EnviroFrame::OnUpdateViewFullscreen(wxUpdateUIEvent& event)
{
	event.Check(m_bFullscreen);
}

void EnviroFrame::OnViewTopDown(wxCommandEvent& event)
{
	m_bTopDown = !m_bTopDown;
	g_App.SetTopDown(m_bTopDown);
}

void EnviroFrame::OnUpdateViewTopDown(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(m_bTopDown);
}

void EnviroFrame::OnViewFramerate(wxCommandEvent& event)
{
	m_canvas->m_bShowFrameRateChart = !m_canvas->m_bShowFrameRateChart;
}

void EnviroFrame::OnUpdateViewFramerate(wxUpdateUIEvent& event)
{
	event.Check(m_canvas && m_canvas->m_bShowFrameRateChart);
}

void EnviroFrame::OnViewElevLegend(wxCommandEvent& event)
{
	g_App.ShowElevationLegend(!g_App.GetShowElevationLegend());
}

void EnviroFrame::OnUpdateViewElevLegend(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.GetShowElevationLegend());
}

void EnviroFrame::OnViewSlower(wxCommandEvent& event)
{
	ChangeFlightSpeed(1.0f / 1.8f);
}

void EnviroFrame::OnUpdateViewSlower(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain && g_App.m_mode == MM_NAVIGATE);
}

void EnviroFrame::OnViewFaster(wxCommandEvent& event)
{
	ChangeFlightSpeed(1.8f);
}

void EnviroFrame::OnUpdateViewFaster(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain && g_App.m_mode == MM_NAVIGATE);
}

void EnviroFrame::OnViewSettings(wxCommandEvent& event)
{
	m_pCameraDlg->Show(true);
}

void EnviroFrame::OnViewLocations(wxCommandEvent& event)
{
	m_pLocationDlg->Show(true);
}

void EnviroFrame::OnUpdateViewLocations(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::Snapshot(bool bNumbered)
{
	vtScene *scene = vtGetScene();
	IPoint2 size = scene->GetWindowSize();
	osg::ref_ptr<osg::Image> pImage = new osg::Image;
	pImage->readPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE);

	wxString2 use_name;
	if (!bNumbered || (bNumbered && m_strSnapshotFilename == _T("")))
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString filter = _T("JPEG Files (*.jpg)|*.jpg") _T("|")
			_T("BMP Files (*.bmp)|*.bmp") _T("|")
			_T("PNG Files (*.png)|*.png") _T("|")
			_T("TIF Files (*.tif)|*.tif");
		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save View Snapshot"), _T(""), _T(""),
			filter, wxSAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return;
		}
		m_iFormat = saveFile.GetFilterIndex();
		if (bNumbered)
		{
			m_strSnapshotFilename = saveFile.GetPath();
			m_iSnapshotNumber = 0;
		}
		else
			use_name = saveFile.GetPath();
	}
	if (bNumbered)
	{
		wxString start, number, extension;
		start = m_strSnapshotFilename.BeforeLast(_T('.'));
		extension = m_strSnapshotFilename.AfterLast(_T('.'));
		number.Printf(_T("_%03d."), m_iSnapshotNumber);
		m_iSnapshotNumber++;
		use_name = start + number + extension;
	}

	unsigned char *data;
	vtDIB dib;
	dib.Create(size.x, size.y, 24);
	int x, y;
	short r, g, b;
	for (y = 0; y < size.y; y++)
	{
		data = pImage->data(0, y);
		for (x = 0; x < size.x; x++)
		{
			r = *data++;
			g = *data++;
			b = *data++;
			dib.SetPixel24(x, size.y-1-y, RGBi(r, g, b));
		}
	}
	switch (m_iFormat)
	{
	case 0:
		dib.WriteJPEG(use_name.mb_str(), 98);
		break;
	case 1:
		dib.WriteBMP(use_name.mb_str());
		break;
	case 2:
		dib.WritePNG(use_name.mb_str());
		break;
	case 3:
		dib.WriteTIF(use_name.mb_str());
		break;
	}
}

void EnviroFrame::OnViewSnapshot(wxCommandEvent& event)
{
	Snapshot(false); // do ask for explicit filename always
}

void EnviroFrame::OnViewSnapAgain(wxCommandEvent& event)
{
	Snapshot(true); // number, and don't ask for filename if we already have one
}

void EnviroFrame::OnViewStatusBar(wxCommandEvent& event)
{
	GetStatusBar()->Show(!GetStatusBar()->IsShown());
	SendSizeEvent();
}

void EnviroFrame::OnUpdateViewStatusBar(wxUpdateUIEvent& event)
{
	event.Check(GetStatusBar()->IsShown());
}

void EnviroFrame::OnViewScenarios(wxCommandEvent& event)
{
	m_pScenarioSelectDialog->Show(true);
}

///////////////////// Tools menu //////////////////////////

void EnviroFrame::OnToolsSelect(wxCommandEvent& event)
{
	SetMode(MM_SELECT);
}

void EnviroFrame::OnUpdateToolsSelect(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
	event.Check(g_App.m_mode == MM_SELECT);
}

void EnviroFrame::OnToolsFences(wxCommandEvent& event)
{
	SetMode(MM_FENCES);
}

void EnviroFrame::OnUpdateToolsFences(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_FENCES);
}

void EnviroFrame::OnToolsRoutes(wxCommandEvent& event)
{
	SetMode(MM_ROUTES);
}

void EnviroFrame::OnUpdateToolsRoutes(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_ROUTES);
}

void EnviroFrame::OnToolsPlants(wxCommandEvent& event)
{
	SetMode(MM_PLANTS);
}

void EnviroFrame::OnUpdateToolsPlants(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_PLANTS);
}

void EnviroFrame::OnToolsInstances(wxCommandEvent& event)
{
	SetMode(MM_INSTANCES);
}

void EnviroFrame::OnUpdateToolsInstances(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_INSTANCES);
}

void EnviroFrame::OnToolsMove(wxCommandEvent& event)
{
	SetMode(MM_MOVE);
}

void EnviroFrame::OnUpdateToolsMove(wxUpdateUIEvent& event)
{
	// not yet implemented
	event.Enable(false);
	event.Check(g_App.m_mode == MM_MOVE);
}

void EnviroFrame::OnToolsNavigate(wxCommandEvent& event)
{
	SetMode(MM_NAVIGATE);
}

void EnviroFrame::OnUpdateToolsNavigate(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_NAVIGATE);
}

void EnviroFrame::OnToolsMeasure(wxCommandEvent& event)
{
	SetMode(MM_MEASURE);
}

void EnviroFrame::OnUpdateToolsMeasure(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
	event.Check(g_App.m_mode == MM_MEASURE);
}

/////////////////////// Scene menu ///////////////////////////

void EnviroFrame::OnSceneGraph(wxCommandEvent& event)
{
	m_pSceneGraphDlg->Show(true);
}

void EnviroFrame::OnSceneTerrain(wxCommandEvent& event)
{
	wxString2 str;
	if (wxGetApp().AskForTerrainName(this, str))
		g_App.SwitchToTerrain(str.to_utf8());
}

void EnviroFrame::OnUpdateSceneTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain ||
		g_App.m_state == AS_Orbit ||
		g_App.m_state == AS_Neutral);
}

void EnviroFrame::OnSceneSpace(wxCommandEvent& event)
{
	g_App.FlyToSpace();
}

void EnviroFrame::OnUpdateSceneSpace(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnSceneSave(wxCommandEvent& event)
{
#if VTLIB_OSG
	vtGroup *pRoot = vtGetTS()->GetTop();
	osgDB::Registry::instance()->writeNode(*pRoot->GetOsgGroup(), "scene.osg");
#endif
}

void EnviroFrame::OnTimeDialog(wxCommandEvent& event)
{
	m_pTimeDlg->Show(true);
}

void EnviroFrame::OnTimeStop(wxCommandEvent& event)
{
	g_App.SetSpeed(0.0f);
}

void EnviroFrame::OnTimeFaster(wxCommandEvent& event)
{
	float x = g_App.GetSpeed();
	if (x == 0.0f)
		g_App.SetSpeed(150.0f);
	else
		g_App.SetSpeed(x*1.5f);
}

/////////////////////// Terrain menu ///////////////////////////

void EnviroFrame::OnDynamic(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	bool on = t->GetFeatureVisible(TFT_TERRAINSURFACE);

	t->SetFeatureVisible(TFT_TERRAINSURFACE, !on);
}

void EnviroFrame::OnUpdateDynamic(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	bool on = t->GetFeatureVisible(TFT_TERRAINSURFACE);

	event.Enable(true);
	event.Check(on);
}

void EnviroFrame::OnCullEvery(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;

	m_bCulleveryframe = !m_bCulleveryframe;
	t->GetDynTerrain()->SetCull(m_bCulleveryframe);
}

void EnviroFrame::OnUpdateCullEvery(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && t->GetDynTerrain());
	event.Check(m_bCulleveryframe);
}

void EnviroFrame::OnCullOnce(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	vtDynTerrainGeom *pTerr = t->GetDynTerrain();
	if (!pTerr) return;

	pTerr->CullOnce();
}

void EnviroFrame::OnSky(wxCommandEvent& event)
{
	vtSkyDome *sky = vtGetTS()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	sky->SetEnabled(!on);
}

void EnviroFrame::OnUpdateSky(wxUpdateUIEvent& event)
{
	vtSkyDome *sky = vtGetTS()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	event.Check(on);
}

void EnviroFrame::OnOcean(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_OCEAN, !t->GetFeatureVisible(TFT_OCEAN));
}

void EnviroFrame::OnUpdateOcean(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_OCEAN);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnPlants(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_VEGETATION, !t->GetFeatureVisible(TFT_VEGETATION));
}

void EnviroFrame::OnUpdatePlants(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_VEGETATION);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnStructures(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_STRUCTURES, !t->GetFeatureVisible(TFT_STRUCTURES));
}

void EnviroFrame::OnUpdateStructures(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_STRUCTURES);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnRoads(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_ROADS, !t->GetFeatureVisible(TFT_ROADS));
}

void EnviroFrame::OnFog(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFog(!t->GetFog());
}

void EnviroFrame::OnUpdateRoads(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_ROADS);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnUpdateFog(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Check(t && t->GetFog());
}

void EnviroFrame::OnIncrease(wxCommandEvent& event)
{
	ChangeTerrainDetail(true);
}

void EnviroFrame::OnDecrease(wxCommandEvent& event)
{
	ChangeTerrainDetail(false);
}

void EnviroFrame::OnLOD(wxCommandEvent& event)
{
	m_pLODDlg->Show();
}

void EnviroFrame::OnUpdateLOD(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t &&
		(t->GetDynTerrain() != NULL || t->GetTiledGeom() != NULL));
}

void EnviroFrame::OnSaveVeg(wxCommandEvent& event)
{
	g_App.SaveVegetation();
}

void EnviroFrame::OnSaveStruct(wxCommandEvent& event)
{
	g_App.SaveStructures();

	// update the displayed filename
	m_pLayerDlg->RefreshTreeContents();
}

static bool s_bBuilt = false;

void EnviroFrame::OnToggleFoundations(wxCommandEvent& event)
{
	s_bBuilt = !s_bBuilt;

	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructures();

	if (s_bBuilt)
	{
		sa->AddFoundations(pTerr->GetHeightField());
	}
	else
	{
		sa->RemoveFoundations();
	}
	int i, size = sa->GetSize();
	for (i = 0; i < size; i++)
		sa->ConstructStructure(i);
}

void EnviroFrame::OnUpdateFoundations(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && t->GetStructures() && t->GetStructures()->GetSize() > 0);
	event.Check(s_bBuilt);
}

void EnviroFrame::OnTerrainReshade(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);
	OpenProgressDialog(_T("Recalculating Shading"), false);
	pTerr->RecreateTextures(vtGetTS()->GetSunLight(), progress_callback);
	CloseProgressDialog();
	EnableContinuousRendering(true);
}


////////////////// Earth Menu //////////////////////

void EnviroFrame::OnEarthShowShading(wxCommandEvent& event)
{
	g_App.SetEarthShading(!g_App.GetEarthShading());
}

void EnviroFrame::OnUpdateEarthShowShading(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthShading());
}

void EnviroFrame::OnEarthShowAxes(wxCommandEvent& event)
{
	g_App.SetSpaceAxes(!g_App.GetSpaceAxes());
}

void EnviroFrame::OnUpdateEarthShowAxes(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit || g_App.m_state == AS_Terrain);
	event.Check(g_App.GetSpaceAxes());
}

void EnviroFrame::OnEarthTilt(wxCommandEvent& event)
{
	g_App.SetEarthTilt(!g_App.GetEarthTilt());
}

void EnviroFrame::OnUpdateEarthTilt(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthTilt());
}

void EnviroFrame::OnUpdateEarthFlatten(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthShape());
}

void EnviroFrame::OnUpdateEarthUnfold(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthUnfold());
}

void EnviroFrame::OnUpdateInOrbit(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
}

void EnviroFrame::OnUpdateInOrbitOrTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit || g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnEarthFlatten(wxCommandEvent& event)
{
	g_App.SetEarthShape(!g_App.GetEarthShape());
}

void EnviroFrame::OnEarthUnfold(wxCommandEvent& event)
{
	g_App.SetEarthUnfold(!g_App.GetEarthUnfold());
}

void EnviroFrame::OnEarthPoints(wxCommandEvent& event)
{
	// save current directory
	wxString path = wxGetCwd();

	wxFileDialog loadFile(NULL, _("Load Point Data"), _T(""), _T(""),
		_("Point Data Sources (*.shp)|*.shp"), wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
	{
		// restore
		wxSetWorkingDirectory(path);
		return;
	}

	wxString2 str = loadFile.GetPath();

	int ret = g_App.AddGlobePoints(str.mb_str());
	if (ret == -1)
		wxMessageBox(_("Couldn't Open"));
	if (ret == -2)
		wxMessageBox(_("That file isn't point data."));

	// restore
	wxSetWorkingDirectory(path);
}


//////////////////////////////////////////////////////

//
// Called when the GUI needs to be informed of a new terrain
//
void EnviroFrame::SetTerrainToGUI(vtTerrain *pTerrain)
{
	if (pTerrain)
	{
		m_pLocationDlg->SetLocSaver(pTerrain->GetLocSaver());
		m_pLocationDlg->SetAnimContainer(pTerrain->GetAnimContainer());

		// Only do this the first time:
		if (!pTerrain->IsVisited())
			m_pLocationDlg->RecallFrom(pTerrain->GetParams().GetValueString(STR_INITLOCATION));
		pTerrain->Visited(true);

		m_pInstanceDlg->SetProjection(pTerrain->GetProjection());
		m_pDistanceDlg->SetProjection(pTerrain->GetProjection());

		// Fill instance dialog with global and terrain-specific content
		m_pInstanceDlg->ClearContent();
		m_pInstanceDlg->AddContent(&vtGetContent());
		if (pTerrain->m_Content.NumItems() != 0)
			m_pInstanceDlg->AddContent(&pTerrain->m_Content);

		// Also switch the time dialog to the time engine of the terrain,
		//  not the globe.
		SetTimeEngine(vtGetTS()->GetTimeEngine());
		m_pScenarioSelectDialog->SetTerrain(pTerrain);

		// While the paging is in early testing, always display the LOD
		//  dialog for vtTiledGeom
		if (pTerrain->GetParams().GetValueInt(STR_SURFACE_TYPE) == 2)
			m_pLODDlg->Show(true);
	}
	else
	{
		vtProjection geo;
		geo.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
		m_pDistanceDlg->SetProjection(geo);
	}
}

//
// Called when the Earth View has been constructed
//
void EnviroFrame::SetTimeEngine(TimeEngine *pEngine)
{
	m_pTimeDlg->SetTimeEngine(pEngine);
	// poke it once to let the time dialog know
	pEngine->SetTime(pEngine->GetTime());
}

void EnviroFrame::EarthPosUpdated(const DPoint3 &pos)
{
	m_pInstanceDlg->SetLocation(DPoint2(pos.x, pos.y));
}

void EnviroFrame::CameraChanged()
{
	// we are dealing with a new camera, so update with its values
	if (m_pCameraDlg && m_pCameraDlg->IsShown())
		m_pCameraDlg->CameraChanged();
}

void EnviroFrame::UpdateStatus()
{
	if (m_pStatusBar)
		m_pStatusBar->UpdateText();

	if (m_pCameraDlg && m_pCameraDlg->IsShown())
		m_pCameraDlg->CheckAndUpdatePos();

	if (m_pLocationDlg && m_pLocationDlg->IsShown())
		m_pLocationDlg->Update();
}

void EnviroFrame::UpdateLODInfo()
{
	if (!m_pLODDlg)
		return;
	vtTerrain *terr = g_App.GetCurrentTerrain();
	if (!terr)
		return;
	vtTiledGeom *geom = terr->GetTiledGeom();
	if (geom)
	{
		float fmin = log(TILEDGEOM_RESOLUTION_MIN);
		float fmax = log(TILEDGEOM_RESOLUTION_MAX);
		float scale = 300 / (fmax -fmin);
		float log0 = log(geom->m_fLResolution);
		float log1 = log(geom->m_fResolution);
		float log2 = log(geom->m_fHResolution);
		m_pLODDlg->Refresh((log0-fmin) * scale,
			(log1-fmin) * scale,
			(log2-fmin) * scale,
			geom->m_iVertexTarget, geom->m_iVertexCount,
			geom->m_iMaxCacheSize, geom->m_iCacheSize,
			geom->m_iTileLoads, geom->m_iCacheHits);
	}
	vtDynTerrainGeom *dyn = terr->GetDynTerrain();
	if (dyn)
	{
		SRTerrain *sr = dynamic_cast<SRTerrain*>(dyn);
		if (sr)
		{
			m_pLODDlg->Refresh(log(sr->m_fLResolution)*17,
				log(sr->m_fResolution)*17,
				log(sr->m_fHResolution)*17,
				sr->GetPolygonTarget(), sr->GetNumDrawnTriangles(),
				-1, -1, -1, -1);
		}
		SMTerrain *sm = dynamic_cast<SMTerrain*>(dyn);
		if (sm)
		{
			m_pLODDlg->Refresh(-1,
				log((sm->GetQualityConstant()-0.002f)*10000)*40, -1,
				sm->GetPolygonTarget(), sm->GetNumDrawnTriangles(),
				-1, -1, -1, -1);
		}
	}
}

///////////////////////////////////////////////////////////////////

void EnviroFrame::ShowPopupMenu(const IPoint2 &pos)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructures();

	wxMenu *popmenu = new wxMenu;
	wxMenuItem *item;
	item = popmenu->Append(ID_POPUP_PROPERTIES, _("Properties"));
	if (sa)
	{
		// Can't display properties for more than one structure
		if (sa->NumSelected() > 1)
			item->Enable(false);

		int sel = sa->GetFirstSelected();
		if (sel != -1)
		{
			vtStructureType type = sa->GetAt(sel)->GetType();
			if (type == ST_BUILDING)
				popmenu->Append(ID_POPUP_FLIP, _("Flip Footprint Direction"));
			if (type == ST_INSTANCE)
				popmenu->Append(ID_POPUP_RELOAD, _("Reload from Disk"));
		}
	}
	popmenu->AppendSeparator();
	popmenu->Append(ID_POPUP_DELETE, _("Delete"));

	m_canvas->PopupMenu(popmenu, pos.x, pos.y);
	delete popmenu;
}

void EnviroFrame::OnPopupProperties(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructures();
	if (sa)
	{
		int sel = sa->GetFirstSelected();
		if (sel != -1)
		{
			vtBuilding3d *bld;
			vtFence3d *fen;

			bld = sa->GetBuilding(sel);
			fen = sa->GetFence(sel);
			if (bld)
			{
				m_pBuildingDlg->Setup(bld, pTerr->GetHeightField());
				m_pBuildingDlg->Show(true);
			}
			if (fen)
			{
				// TODO? Enable editing of fence properties
				m_pFenceDlg->Show(true);
			}
			return;
		}
	}

	vtPlantInstanceArray3d &plants = pTerr->GetPlantInstances();
	if (plants.NumSelected() != 0)
	{
		int found = -1;
		unsigned int count = plants.GetNumEntities();
		for (unsigned int i = 0; i < count; i++)
		{
			if (plants.IsSelected(i))
			{
				found = i;
				break;
			}
		}
		if (found != -1)
		{
			// Show properties for this plant
			PlantingOptions &opt = g_App.GetPlantOptions();

			float size;
			short species_id;
			plants.GetPlant(found, size, species_id);
			opt.m_iSpecies = species_id;
			opt.m_fHeight = size;

			m_pPlantDlg->SetPlantList(g_App.GetPlantList());
			m_pPlantDlg->SetPlantOptions(opt);
			m_pPlantDlg->Show(true);
		}
	}
}

void EnviroFrame::OnPopupFlip(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *structures = pTerr->GetStructures();

	int count = structures->GetSize();
	vtStructure *str;
	vtBuilding3d *bld;
	for (int i = 0; i < count; i++)
	{
		str = structures->GetAt(i);
		if (!str->IsSelected())
			continue;

		bld = structures->GetBuilding(i);
		if (!bld)
			continue;
		bld->FlipFootprintDirection();
		structures->ConstructStructure(structures->GetStructure3d(i));
	}
}

void EnviroFrame::OnPopupReload(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *structures = pTerr->GetStructures();

	int count = structures->GetSize();
	vtStructure *str;
	vtStructInstance3d *inst;
	for (int i = 0; i < count; i++)
	{
		str = structures->GetAt(i);
		if (!str->IsSelected())
			continue;

		inst = structures->GetInstance(i);
		if (!inst)
			continue;
		structures->ConstructStructure(structures->GetStructure3d(i));
	}
}

void EnviroFrame::OnPopupDelete(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	pTerr->DeleteSelectedStructures();
	pTerr->DeleteSelectedPlants();

	// layer dialog needs to reflect the change
	m_pLayerDlg->RefreshTreeContents();
}

