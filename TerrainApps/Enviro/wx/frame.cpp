//
// Name:	 frame.cpp
// Purpose:  The frame class for the wxEnviro application.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifdef UNIX
#include <unistd.h>
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/DynTerrain.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/Globe.h"
#include "vtdata/vtLog.h"

#include "frame.h"
#include "StatusBar.h"

// dialogs
#include "SceneGraphDlg.h"
#include "PlantDlg.h"
#include "LinearStructDlg3d.h"
#include "UtilDlg.h"
#include "CameraDlg.h"
#include "LocationDlg.h"
#include "BuildingDlg3d.h"
#include "LayerDlg.h"
#include "vtui/InstanceDlg.h"
#include "vtui/DistanceDlg.h"

#include "../Engines.h"
#include "EnviroGUI.h"	// for GetCurrentTerrain

#include "app.h"
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
#  include "navroute.xpm"
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
#  include "tree.xpm"
#  include "unfold.xpm"
#endif

#if VTLIB_OSG
#include <osgDB/Registry>
#endif

DECLARE_APP(vtApp);

BEGIN_EVENT_TABLE(vtFrame, wxFrame)
EVT_CHAR(vtFrame::OnChar)
EVT_MENU(ID_FILE_LAYERS,		vtFrame::OnFileLayers)
EVT_MENU(wxID_EXIT, vtFrame::OnExit)
EVT_CLOSE(vtFrame::OnClose)

EVT_MENU(ID_TOOLS_SELECT,			vtFrame::OnToolsSelect)
EVT_UPDATE_UI(ID_TOOLS_SELECT,		vtFrame::OnUpdateToolsSelect)
EVT_MENU(ID_TOOLS_FENCES,			vtFrame::OnToolsFences)
EVT_UPDATE_UI(ID_TOOLS_FENCES,		vtFrame::OnUpdateToolsFences)
EVT_MENU(ID_TOOLS_ROUTES,			vtFrame::OnToolsRoutes)
EVT_UPDATE_UI(ID_TOOLS_ROUTES,		vtFrame::OnUpdateToolsRoutes)
EVT_MENU(ID_TOOLS_PLANTS,			vtFrame::OnToolsPlants)
EVT_UPDATE_UI(ID_TOOLS_PLANTS,		vtFrame::OnUpdateToolsPlants)
EVT_MENU(ID_TOOLS_INSTANCES,		vtFrame::OnToolsInstances)
EVT_UPDATE_UI(ID_TOOLS_INSTANCES,	vtFrame::OnUpdateToolsInstances)
EVT_MENU(ID_TOOLS_MOVE,				vtFrame::OnToolsMove)
EVT_UPDATE_UI(ID_TOOLS_MOVE,		vtFrame::OnUpdateToolsMove)
EVT_MENU(ID_TOOLS_NAVIGATE,			vtFrame::OnToolsNavigate)
EVT_UPDATE_UI(ID_TOOLS_NAVIGATE,	vtFrame::OnUpdateToolsNavigate)
EVT_MENU(ID_TOOLS_MEASURE,			vtFrame::OnToolsMeasure)
EVT_UPDATE_UI(ID_TOOLS_MEASURE,		vtFrame::OnUpdateToolsMeasure)

EVT_MENU(ID_VIEW_MAINTAIN,			vtFrame::OnViewMaintain)
EVT_UPDATE_UI(ID_VIEW_MAINTAIN,		vtFrame::OnUpdateViewMaintain)
EVT_MENU(ID_VIEW_WIREFRAME,			vtFrame::OnViewWireframe)
EVT_UPDATE_UI(ID_VIEW_WIREFRAME,	vtFrame::OnUpdateViewWireframe)
EVT_MENU(ID_VIEW_FULLSCREEN,		vtFrame::OnViewFullscreen)
EVT_UPDATE_UI(ID_VIEW_FULLSCREEN,	vtFrame::OnUpdateViewFullscreen)
EVT_MENU(ID_VIEW_TOPDOWN,			vtFrame::OnViewTopDown)
EVT_UPDATE_UI(ID_VIEW_TOPDOWN,		vtFrame::OnUpdateViewTopDown)
EVT_MENU(ID_VIEW_FRAMERATE,			vtFrame::OnViewFramerate)
EVT_MENU(ID_VIEW_SETTINGS,			vtFrame::OnViewSettings)
EVT_MENU(ID_VIEW_FOLLOW_ROUTE,		vtFrame::OnViewFollowRoute)
EVT_UPDATE_UI(ID_VIEW_FOLLOW_ROUTE, vtFrame::OnUpdateViewFollowRoute)
EVT_MENU(ID_VIEW_LOCATIONS,			vtFrame::OnViewLocations)
EVT_UPDATE_UI(ID_VIEW_LOCATIONS,	vtFrame::OnUpdateViewLocations)
EVT_MENU(ID_VIEW_SNAPSHOT,			vtFrame::OnViewSnapshot)
EVT_MENU(ID_VIEW_SNAP_AGAIN,		vtFrame::OnViewSnapAgain)
EVT_MENU(ID_VIEW_STATUSBAR,			vtFrame::OnViewStatusBar)
EVT_UPDATE_UI(ID_VIEW_STATUSBAR,	vtFrame::OnUpdateViewStatusBar)

EVT_UPDATE_UI(ID_VIEW_FRAMERATE,vtFrame::OnUpdateViewFramerate)
EVT_MENU(ID_VIEW_SLOWER,		vtFrame::OnViewSlower)
EVT_UPDATE_UI(ID_VIEW_SLOWER,	vtFrame::OnUpdateViewSlower)
EVT_MENU(ID_VIEW_FASTER,		vtFrame::OnViewFaster)
EVT_UPDATE_UI(ID_VIEW_FASTER,	vtFrame::OnUpdateViewFaster)
EVT_MENU(ID_NAV_NORMAL,			vtFrame::OnNavNormal)
EVT_UPDATE_UI(ID_NAV_NORMAL,	vtFrame::OnUpdateNavNormal)
EVT_MENU(ID_NAV_VELO,			vtFrame::OnNavVelo)
EVT_UPDATE_UI(ID_NAV_VELO,		vtFrame::OnUpdateNavVelo)
EVT_MENU(ID_NAV_GRAB_PIVOT,		vtFrame::OnNavGrabPivot)
EVT_UPDATE_UI(ID_NAV_GRAB_PIVOT,vtFrame::OnUpdateNavGrabPivot)
EVT_MENU(ID_NAV_PANO,			vtFrame::OnNavPano)
EVT_UPDATE_UI(ID_NAV_PANO,		vtFrame::OnUpdateNavPano)

EVT_MENU(ID_SCENE_SCENEGRAPH,	vtFrame::OnSceneGraph)
EVT_MENU(ID_SCENE_TERRAIN,		vtFrame::OnSceneTerrain)
EVT_UPDATE_UI(ID_SCENE_TERRAIN,	vtFrame::OnUpdateSceneTerrain)
EVT_MENU(ID_SCENE_SPACE,		vtFrame::OnSceneSpace)
EVT_UPDATE_UI(ID_SCENE_SPACE,	vtFrame::OnUpdateSceneSpace)
#if VTLIB_OSG
EVT_MENU(ID_SCENE_SAVE,			vtFrame::OnSceneSave)
#endif
EVT_MENU(ID_TIME_STOP,			vtFrame::OnTimeStop)
EVT_MENU(ID_TIME_FASTER,		vtFrame::OnTimeFaster)

EVT_UPDATE_UI(ID_TIME_STOP,		vtFrame::OnUpdateInOrbit)
EVT_UPDATE_UI(ID_TIME_FASTER,	vtFrame::OnUpdateInOrbit)

EVT_MENU(ID_TERRAIN_DYNAMIC,	vtFrame::OnDynamic)
EVT_MENU(ID_TERRAIN_CULLEVERY,	vtFrame::OnCullEvery)
EVT_MENU(ID_TERRAIN_CULLONCE,	vtFrame::OnCullOnce)
EVT_MENU(ID_TERRAIN_SKY,		vtFrame::OnSky)
EVT_MENU(ID_TERRAIN_OCEAN,		vtFrame::OnOcean)
EVT_MENU(ID_TERRAIN_PLANTS,		vtFrame::OnPlants)
EVT_MENU(ID_TERRAIN_STRUCTURES,	vtFrame::OnStructures)
EVT_MENU(ID_TERRAIN_ROADS,		vtFrame::OnRoads)
EVT_MENU(ID_TERRAIN_FOG,		vtFrame::OnFog)
EVT_MENU(ID_TERRAIN_INCREASE,	vtFrame::OnIncrease)
EVT_MENU(ID_TERRAIN_DECREASE,	vtFrame::OnDecrease)
EVT_MENU(ID_TERRAIN_SAVEVEG,	vtFrame::OnSaveVeg)
EVT_MENU(ID_TERRAIN_SAVESTRUCT,	vtFrame::OnSaveStruct)
EVT_MENU(ID_TERRAIN_FOUNDATIONS, vtFrame::OnToggleFoundations)

EVT_UPDATE_UI(ID_TERRAIN_DYNAMIC,	vtFrame::OnUpdateDynamic)
EVT_UPDATE_UI(ID_TERRAIN_CULLEVERY, vtFrame::OnUpdateCullEvery)
EVT_UPDATE_UI(ID_TERRAIN_SKY,		vtFrame::OnUpdateSky)
EVT_UPDATE_UI(ID_TERRAIN_OCEAN,		vtFrame::OnUpdateOcean)
EVT_UPDATE_UI(ID_TERRAIN_PLANTS,	vtFrame::OnUpdatePlants)
EVT_UPDATE_UI(ID_TERRAIN_STRUCTURES, vtFrame::OnUpdateStructures)
EVT_UPDATE_UI(ID_TERRAIN_ROADS,		vtFrame::OnUpdateRoads)
EVT_UPDATE_UI(ID_TERRAIN_FOG,		vtFrame::OnUpdateFog)
EVT_UPDATE_UI(ID_TERRAIN_FOUNDATIONS, vtFrame::OnUpdateFoundations)

EVT_MENU(ID_EARTH_SHOWSHADING,	vtFrame::OnEarthShowShading)
EVT_MENU(ID_EARTH_SHOWAXES,		vtFrame::OnEarthShowAxes)
EVT_MENU(ID_EARTH_TILT,			vtFrame::OnEarthTilt)
EVT_MENU(ID_EARTH_FLATTEN,		vtFrame::OnEarthFlatten)
EVT_MENU(ID_EARTH_UNFOLD,		vtFrame::OnEarthUnfold)
EVT_MENU(ID_EARTH_POINTS,		vtFrame::OnEarthPoints)

EVT_UPDATE_UI(ID_EARTH_SHOWSHADING, vtFrame::OnUpdateEarthShowShading)
EVT_UPDATE_UI(ID_EARTH_SHOWAXES, vtFrame::OnUpdateEarthShowAxes)
EVT_UPDATE_UI(ID_EARTH_TILT,	vtFrame::OnUpdateEarthTilt)
EVT_UPDATE_UI(ID_EARTH_FLATTEN, vtFrame::OnUpdateInOrbit)
EVT_UPDATE_UI(ID_EARTH_UNFOLD,	vtFrame::OnUpdateInOrbit)
EVT_UPDATE_UI(ID_EARTH_POINTS,	vtFrame::OnUpdateInOrbit)

EVT_MENU(ID_HELP_ABOUT, vtFrame::OnHelpAbout)

// Popup
EVT_MENU(ID_POPUP_PROPERTIES, vtFrame::OnPopupProperties)
EVT_MENU(ID_POPUP_FLIP, vtFrame::OnPopupFlip)
EVT_MENU(ID_POPUP_RELOAD, vtFrame::OnPopupReload)
EVT_MENU(ID_POPUP_DELETE, vtFrame::OnPopupDelete)
END_EVENT_TABLE()

// My frame constructor
vtFrame::vtFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style, bool bVerticalToolbar, bool bEnableEarth):
wxFrame(parent, -1, title, pos, size, style)
{
	// Give it an icon
	SetIcon(wxIcon(_T(ICON_NAME)));

	m_bCulleveryframe = true;
	m_bAlwaysMove = false;
	m_bFullscreen = false;
	m_bTopDown = false;
	m_ToggledMode = MM_NONE;
	m_bEnableEarth = bEnableEarth;

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
	int gl_attrib[7] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
		WX_GL_BUFFER_SIZE, 24, WX_GL_DEPTH_SIZE, 24, 0	};

	VTLOG("Frame window: creating view canvas.\n");
	m_canvas = new vtGLCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
			_T("vtGLCanvas"), gl_attrib);

	// Show the frame
	Show(true);

	m_pSceneGraphDlg = new SceneGraphDlg(this, -1, _T("Scene Graph"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(250, 350);

	m_pPlantDlg = new PlantDlg(this, -1, _T("Plants"));
	m_pFenceDlg = new LinearStructureDlg3d(this, -1, _T("Linear Structures"));
	m_pUtilDlg = new UtilDlg(this, -1, _T("Utility"));
	m_pCameraDlg = new CameraDlg(this, -1, _T("Camera-View"));
	m_pLocationDlg = new LocationDlg(this, -1, _T("Locations"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pBuildingDlg = new BuildingDlg3d(this, -1, _T("Building Properties"));
	m_pLayerDlg = new LayerDlg(this, -1, _T("Layers"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pLayerDlg->SetSize(600, 250);
	m_pInstanceDlg = new InstanceDlg(this, -1, _T("Instances"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pDistanceDlg = new DistanceDlg(this, -1, _T("Distance"));

	m_canvas->SetCurrent();
}

vtFrame::~vtFrame()
{
	VTLOG("Deleting Frame\n");
	delete m_canvas;
	delete m_pSceneGraphDlg;
	delete m_pPlantDlg;
	delete m_pFenceDlg;
	delete m_pUtilDlg;
	delete m_pCameraDlg;
	delete m_pLocationDlg;
	delete m_pLayerDlg;

	delete m_pStatusBar;
	delete m_pToolbar;
	SetStatusBar(NULL);
	SetToolBar(NULL);
}

void vtFrame::CreateMenus()
{
	// Make menu bar
	m_pMenuBar = new wxMenuBar;

	// Make menus
	m_pFileMenu = new wxMenu;
	m_pFileMenu->Append(ID_FILE_LAYERS, _T("Layers"), _T("Layers"));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(wxID_EXIT, _T("E&xit (Esc)"), _T("Exit"));
	m_pMenuBar->Append(m_pFileMenu, _T("&File"));

	wxMenu *toolsMenu = new wxMenu;
	toolsMenu->AppendCheckItem(ID_TOOLS_SELECT, _T("Select"));
	toolsMenu->AppendCheckItem(ID_TOOLS_FENCES, _T("Fences"));
	toolsMenu->AppendCheckItem(ID_TOOLS_ROUTES, _T("Routes"));
	toolsMenu->AppendCheckItem(ID_TOOLS_PLANTS, _T("Plants"));
	toolsMenu->AppendCheckItem(ID_TOOLS_INSTANCES, _T("Instances"));
	toolsMenu->AppendCheckItem(ID_TOOLS_MOVE, _T("Move Objects"));
	toolsMenu->AppendCheckItem(ID_TOOLS_NAVIGATE, _T("Navigate"));
	toolsMenu->AppendCheckItem(ID_TOOLS_MEASURE, _T("Measure Distances"));
	m_pMenuBar->Append(toolsMenu, _T("&Tools"));

	if (m_bEnableEarth)
	{
		m_pSceneMenu = new wxMenu;
		m_pSceneMenu->Append(ID_SCENE_SCENEGRAPH, _T("Scene Graph"));
		m_pSceneMenu->AppendSeparator();
		m_pSceneMenu->Append(ID_SCENE_TERRAIN, _T("Go to Terrain..."));
		m_pSceneMenu->Append(ID_SCENE_SPACE, _T("Go to Space"));
#if VTLIB_OSG
		m_pSceneMenu->AppendSeparator();
		m_pSceneMenu->Append(ID_SCENE_SAVE, _T("Save scene graph to .osg"));
#endif
		m_pSceneMenu->AppendSeparator();
		m_pSceneMenu->Append(ID_TIME_STOP, _T("Time Stop"));
		m_pSceneMenu->Append(ID_TIME_FASTER, _T("Time Faster"));
		m_pMenuBar->Append(m_pSceneMenu, _T("&Scene"));
	}

	// shortcuts:
	// Ctrl+A Show Axes
	// Ctrl+C Cull Every Frame
	// Ctrl+E Flatten
	// Ctrl+F Fullscreen
	// Ctrl+K Cull Once
	// Ctrl+L Add Linear Features
	// Ctrl+N Save Numbered Snapshot
	// Ctrl+P Load Point Data
	// Ctrl+R Framerate Chart
	// Ctrl+S Camera - View Settings
	// Ctrl+T Top-Down
	// Ctrl+U Unfold
	// Ctrl+W Wireframe
	// A Maintain height above ground
	// F Faster
	// S Faster

	wxMenu *viewMenu = new wxMenu;
	viewMenu->AppendCheckItem(ID_VIEW_WIREFRAME, _T("Wireframe\tCtrl+W"));
	viewMenu->AppendCheckItem(ID_VIEW_FULLSCREEN, _T("Fullscreen\tCtrl+F"));
	viewMenu->AppendCheckItem(ID_VIEW_TOPDOWN, _T("Top-Down Camera\tCtrl+T"));
	viewMenu->AppendCheckItem(ID_VIEW_FRAMERATE, _T("Framerate Chart\tCtrl+R"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_SETTINGS, _T("Camera - View Settings\tCtrl+S"));
	viewMenu->Append(ID_VIEW_LOCATIONS, _T("Store/Recall Locations"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_SNAPSHOT, _T("Save Window Snapshot"));
	viewMenu->Append(ID_VIEW_SNAP_AGAIN, _T("Save Numbered Snapshot\tCtrl+N"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_STATUSBAR, _T("&Status Bar"));
	m_pMenuBar->Append(viewMenu, _T("&View"));

	wxMenu *navMenu = new wxMenu;
	navMenu->Append(ID_VIEW_SLOWER, _T("Fly Slower (S)"));
	navMenu->Append(ID_VIEW_FASTER, _T("Fly Faster (F)"));
	navMenu->AppendCheckItem(ID_VIEW_MAINTAIN, _T("Maintain height above ground (A)"));
	m_pMenuBar->Append(navMenu, _T("&Navigate"));

		// submenu
		wxMenu *navstyleMenu = new wxMenu;
		navstyleMenu->AppendCheckItem(ID_NAV_NORMAL, _T("Normal Terrain Flyer"));
		navstyleMenu->AppendCheckItem(ID_NAV_VELO, _T("Flyer with Velocity"));
		navstyleMenu->AppendCheckItem(ID_NAV_GRAB_PIVOT, _T("Grab-Pivot"));
		navstyleMenu->AppendCheckItem(ID_NAV_PANO, _T("Panoramic Flyer"));
//		navstyleMenu->AppendCheckItem(ID_NAV_QUAKE, _T("Keyboard Walk"));
		navMenu->Append(0, _T("Navigation Style"), navstyleMenu);

	wxMenu *terrainMenu = new wxMenu;
	terrainMenu->AppendCheckItem(ID_TERRAIN_DYNAMIC, _T("LOD Terrain Surface\tF3"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_CULLEVERY, _T("Cull every frame\tCtrl+C"));
	terrainMenu->Append(ID_TERRAIN_CULLONCE, _T("Cull once\tCtrl+K"));
	terrainMenu->AppendSeparator();
	terrainMenu->AppendCheckItem(ID_TERRAIN_SKY, _T("Show Sky\tF4"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_OCEAN, _T("Show Ocean\tF5"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_PLANTS, _T("Show Plants\tF6"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_STRUCTURES, _T("Show Structures\tF7"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_ROADS, _T("Show Roads\tF8"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_FOG, _T("Show Fog\tF9"));
	terrainMenu->AppendSeparator();
	terrainMenu->Append(ID_TERRAIN_INCREASE, _T("Increase Detail (+)"));
	terrainMenu->Append(ID_TERRAIN_DECREASE, _T("Decrease Detail (-)"));
	terrainMenu->AppendSeparator();
	terrainMenu->Append(ID_TERRAIN_SAVEVEG, _T("Save Vegetation As..."));
	terrainMenu->Append(ID_TERRAIN_SAVESTRUCT, _T("Save Built Structures As..."));
	terrainMenu->AppendSeparator();
	terrainMenu->AppendCheckItem(ID_TERRAIN_FOUNDATIONS, _T("Toggle Artificial Foundations"));
	m_pMenuBar->Append(terrainMenu, _T("Te&rrain"));

	if (m_bEnableEarth)
	{
		m_pEarthMenu = new wxMenu;
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWSHADING, _T("&Show Shading\tCtrl+I"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWAXES, _T("&Show Axes\tCtrl+A"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_TILT, _T("Seasonal Tilt"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_FLATTEN, _T("&Flatten\tCtrl+E"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_UNFOLD, _T("&Unfold\tCtrl+U"));
		m_pEarthMenu->Append(ID_EARTH_POINTS, _T("&Load Point Data...\tCtrl+P"));
		m_pMenuBar->Append(m_pEarthMenu, _T("&Earth"));
	}

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ID_HELP_ABOUT, _T("About ") WSTRING_APPORG _T("..."));
	m_pMenuBar->Append(helpMenu, _T("&Help"));

	SetMenuBar(m_pMenuBar);
}

void vtFrame::CreateToolbar(bool bVertical)
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
	ADD_TOOL(ID_VIEW_FOLLOW_ROUTE, wxBITMAP(navroute), _("Follow Route"), true);
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
		ADD_TOOL(ID_EARTH_SHOWSHADING, wxBITMAP(sun), _("Time of Day"), true);
		ADD_TOOL(ID_EARTH_SHOWAXES, wxBITMAP(axes), _("Axes"), true);
		ADD_TOOL(ID_EARTH_TILT, wxBITMAP(tilt), _("Tilt"), true);
		ADD_TOOL(ID_EARTH_POINTS, wxBITMAP(points), _("Add Point Data"), false);
		ADD_TOOL(ID_EARTH_UNFOLD, wxBITMAP(unfold), _("Unfold"), true);
		m_pToolbar->AddSeparator();
		ADD_TOOL(ID_TIME_STOP, wxBITMAP(stop), _("Time Stop"), false);
		ADD_TOOL(ID_TIME_FASTER, wxBITMAP(faster), _("Time Faster"), false);
		m_pToolbar->AddSeparator();
		ADD_TOOL(ID_SCENE_SCENEGRAPH, wxBITMAP(sgraph), _("Scene Graph"), false);
	}

//	m_pToolbar->SetRows(32);
	m_pToolbar->Realize();
//	SendSizeEvent();
//	m_pToolbar->Refresh();
}

//
// Utility methods
//

void vtFrame::SetMode(MouseMode mode)
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

void vtFrame::OnChar(wxKeyEvent& event)
{
	static NavType prev = NT_Normal;
	vtTerrain *pTerr = GetCurrentTerrain();
	long key = event.KeyCode();

	// Keyboard shortcuts ("accelerators")
	switch (key)
	{
	case 27:
		// Esc: exit application
		m_canvas->m_bRunning = false;
		delete m_canvas;
		m_canvas = NULL;
		Destroy();
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

	case 'z':
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
#else
		{
			vtTerrain *pTerr = GetCurrentTerrain();

			static int hour = 6, min = 30;
			vtTime time;
			time.SetTimeOfDay(hour, min, 0);
			pTerr->recreate_textures(time);
			min += 30;
			if (min == 60)
			{
				min = 0;
				hour ++;
			}
		}
#endif
		break;

	case 4:	// Ctrl-D
		// dump camera info
		g_App.DumpCameraInfo();
		break;

	case 2:	// Ctrl-B
		// toggle logo
		g_App.ToggleLogo();
		break;

	default:
		event.Skip();
		break;
	}
}

void vtFrame::ToggleNavigate()
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

void vtFrame::ChangeFlightSpeed(float factor)
{
	float speed = g_App.GetFlightSpeed();
	g_App.SetFlightSpeed(speed * factor);

	VTLOG("Change flight speed to %f\n", speed * factor);

	m_pCameraDlg->GetValues();
	m_pCameraDlg->ValuesToSliders();
	m_pCameraDlg->TransferToWindow();
	m_pCameraDlg->Refresh();
}

void vtFrame::ChangeTerrainDetail(bool bIncrease)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	vtDynTerrainGeom *pTerr = t->GetDynTerrain();
	if (!pTerr) return;

	if (bIncrease)
	{
		pTerr->SetPixelError(pTerr->GetPixelError()*1.1f);
		pTerr->SetPolygonCount(pTerr->GetPolygonCount()+1000);
	}
	else
	{
		pTerr->SetPixelError(pTerr->GetPixelError()/1.1f);
		pTerr->SetPolygonCount(pTerr->GetPolygonCount()-1000);
	}
}

void vtFrame::SetFullScreen(bool bFull)
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
  /*    Do full-screen another way.                           */
#endif
}

//
// Intercept menu commands
//

void vtFrame::OnExit(wxCommandEvent& event)
{
	VTLOG("Got Exit event.\n");
	if (m_canvas)
	{
		m_canvas->m_bRunning = false;
		delete m_canvas;
		m_canvas = NULL;
	}
	Destroy();
}

void vtFrame::OnClose(wxCloseEvent &event)
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

void vtFrame::OnHelpAbout(wxCommandEvent& event)
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
 	str += _T("\nThis version built with the ");
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

void vtFrame::OnFileLayers(wxCommandEvent& event)
{
	m_pLayerDlg->Show(true);
}


//////////////////// View menu //////////////////////////

void vtFrame::OnViewMaintain(wxCommandEvent& event)
{
	g_App.SetMaintain(!g_App.GetMaintain());
}

void vtFrame::OnUpdateViewMaintain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.GetMaintain());
}

void vtFrame::OnNavNormal(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Normal);
}

void vtFrame::OnUpdateNavNormal(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Normal);
}

void vtFrame::OnNavVelo(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Velo);
}

void vtFrame::OnUpdateNavVelo(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Velo);
}

void vtFrame::OnNavGrabPivot(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Grab);
}

void vtFrame::OnUpdateNavGrabPivot(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Grab);
}

void vtFrame::OnNavPano(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Pano);
}

void vtFrame::OnUpdateNavPano(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Pano);
}

void vtFrame::OnViewWireframe(wxCommandEvent& event)
{
	vtGetScene()->SetGlobalWireframe(!vtGetScene()->GetGlobalWireframe());
}

void vtFrame::OnUpdateViewWireframe(wxUpdateUIEvent& event)
{
	event.Check(vtGetScene()->GetGlobalWireframe());
}

void vtFrame::OnViewFullscreen(wxCommandEvent& event)
{
	SetFullScreen(!m_bFullscreen);
}

void vtFrame::OnUpdateViewFullscreen(wxUpdateUIEvent& event)
{
	event.Check(m_bFullscreen);
}

void vtFrame::OnViewTopDown(wxCommandEvent& event)
{
	m_bTopDown = !m_bTopDown;
	g_App.SetTopDown(m_bTopDown);
}

void vtFrame::OnUpdateViewTopDown(wxUpdateUIEvent& event)
{
	event.Check(m_bTopDown);
}

void vtFrame::OnViewFramerate(wxCommandEvent& event)
{
	m_canvas->m_bShowFrameRateChart = !m_canvas->m_bShowFrameRateChart;
}

void vtFrame::OnUpdateViewFramerate(wxUpdateUIEvent& event)
{
	event.Check(m_canvas && m_canvas->m_bShowFrameRateChart);
}

void vtFrame::OnViewSlower(wxCommandEvent& event)
{
	ChangeFlightSpeed(1.0f / 1.8f);
}

void vtFrame::OnUpdateViewSlower(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain && g_App.m_mode == MM_NAVIGATE);
}

void vtFrame::OnViewFaster(wxCommandEvent& event)
{
	ChangeFlightSpeed(1.8f);
}

void vtFrame::OnUpdateViewFaster(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain && g_App.m_mode == MM_NAVIGATE);
}

void vtFrame::OnViewSettings(wxCommandEvent& event)
{
	m_pCameraDlg->Show(true);
}

void vtFrame::OnViewFollowRoute(wxCommandEvent& event)
{
	if (!g_App.m_pCurRoute)
		return;

	bool bOn = g_App.GetRouteFollower();
	g_App.SetRouteFollower(!bOn);
}

void vtFrame::OnUpdateViewFollowRoute(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.GetRouteFollower());
}

void vtFrame::OnViewLocations(wxCommandEvent& event)
{
	m_pLocationDlg->Show(true);
}

void vtFrame::OnUpdateViewLocations(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void vtFrame::Snapshot(bool bNumbered)
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

		wxString filter = _T("JPEG Files (*.jpg)|*.jpg|")
			_T("BMP Files (*.bmp)|*.bmp|")
			_T("PNG Files (*.png)|*.png|")
			_T("TIF Files (*.tif)|*.tif|");
		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _T("Save View Snapshot"), _T(""), _T(""),
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
			use_name = saveFile.GetPath();;
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

void vtFrame::OnViewSnapshot(wxCommandEvent& event)
{
	Snapshot(false); // do ask for explicit filename always
}

void vtFrame::OnViewSnapAgain(wxCommandEvent& event)
{
	Snapshot(true); // number, and don't ask for filename if we already have one
}

void vtFrame::OnViewStatusBar(wxCommandEvent& event)
{
	GetStatusBar()->Show(!GetStatusBar()->IsShown());
	SendSizeEvent();
}

void vtFrame::OnUpdateViewStatusBar(wxUpdateUIEvent& event)
{
	event.Check(GetStatusBar()->IsShown());
}


///////////////////// Tools menu //////////////////////////

void vtFrame::OnToolsSelect(wxCommandEvent& event)
{
	SetMode(MM_SELECT);
}

void vtFrame::OnUpdateToolsSelect(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
	event.Check(g_App.m_mode == MM_SELECT);
}

void vtFrame::OnToolsFences(wxCommandEvent& event)
{
	SetMode(MM_FENCES);
}

void vtFrame::OnUpdateToolsFences(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_FENCES);
}

void vtFrame::OnToolsRoutes(wxCommandEvent& event)
{
	SetMode(MM_ROUTES);
}

void vtFrame::OnUpdateToolsRoutes(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_ROUTES);
}

void vtFrame::OnToolsPlants(wxCommandEvent& event)
{
	SetMode(MM_PLANTS);
}

void vtFrame::OnUpdateToolsPlants(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_PLANTS);
}

void vtFrame::OnToolsInstances(wxCommandEvent& event)
{
	SetMode(MM_INSTANCES);
}

void vtFrame::OnUpdateToolsInstances(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_INSTANCES);
}

void vtFrame::OnToolsMove(wxCommandEvent& event)
{
	SetMode(MM_MOVE);
}

void vtFrame::OnUpdateToolsMove(wxUpdateUIEvent& event)
{
	// not yet implemented
	event.Enable(false);
	event.Check(g_App.m_mode == MM_MOVE);
}

void vtFrame::OnToolsNavigate(wxCommandEvent& event)
{
	SetMode(MM_NAVIGATE);
}

void vtFrame::OnUpdateToolsNavigate(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_NAVIGATE);
}

void vtFrame::OnToolsMeasure(wxCommandEvent& event)
{
	SetMode(MM_MEASURE);
}

void vtFrame::OnUpdateToolsMeasure(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
	event.Check(g_App.m_mode == MM_MEASURE);
}

/////////////////////// Scene menu ///////////////////////////

void vtFrame::OnSceneGraph(wxCommandEvent& event)
{
	m_pSceneGraphDlg->Show(true);
}

void vtFrame::OnSceneTerrain(wxCommandEvent& event)
{
	wxString2 str;
	if (wxGetApp().AskForTerrainName(this, str))
		g_App.SwitchToTerrain(str.mb_str());
}

void vtFrame::OnUpdateSceneTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain ||
		g_App.m_state == AS_Orbit ||
		g_App.m_state == AS_Neutral);
}

void vtFrame::OnSceneSpace(wxCommandEvent& event)
{
	g_App.FlyToSpace();
}

void vtFrame::OnUpdateSceneSpace(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void vtFrame::OnSceneSave(wxCommandEvent& event)
{
#if VTLIB_OSG
	vtGroup *pRoot = GetTerrainScene()->GetTop();
	osgDB::Registry::instance()->writeNode(*pRoot->GetOsgGroup(), "scene.osg");
#endif
}

void vtFrame::OnTimeStop(wxCommandEvent& event)
{
	g_App.SetSpeed(0.0f);
}

void vtFrame::OnTimeFaster(wxCommandEvent& event)
{
	float x = g_App.GetSpeed();
	if (x == 0.0f)
		g_App.SetSpeed(150.0f);
	else
		g_App.SetSpeed(x*1.5f);
}

/////////////////////// Terrain menu ///////////////////////////

void vtFrame::OnDynamic(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	bool on = t->GetFeatureVisible(TFT_TERRAINSURFACE);

	t->SetFeatureVisible(TFT_TERRAINSURFACE, !on);
}

void vtFrame::OnUpdateDynamic(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	bool on = t->GetFeatureVisible(TFT_TERRAINSURFACE);

	event.Enable(true);
	event.Check(on);
}

void vtFrame::OnCullEvery(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;

	m_bCulleveryframe = !m_bCulleveryframe;
	t->GetDynTerrain()->SetCull(m_bCulleveryframe);
}

void vtFrame::OnUpdateCullEvery(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && t->GetDynTerrain());
	event.Check(m_bCulleveryframe);
}

void vtFrame::OnCullOnce(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	vtDynTerrainGeom *pTerr = t->GetDynTerrain();
	if (!pTerr) return;

	pTerr->CullOnce();
}

void vtFrame::OnSky(wxCommandEvent& event)
{
	vtSkyDome *sky = GetTerrainScene()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	sky->SetEnabled(!on);
}

void vtFrame::OnUpdateSky(wxUpdateUIEvent& event)
{
	vtSkyDome *sky = GetTerrainScene()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	event.Check(on);
}

void vtFrame::OnOcean(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_OCEAN, !t->GetFeatureVisible(TFT_OCEAN));
}

void vtFrame::OnUpdateOcean(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_OCEAN);
	event.Enable(t != NULL);
	event.Check(on);
}

void vtFrame::OnPlants(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_VEGETATION, !t->GetFeatureVisible(TFT_VEGETATION));
}

void vtFrame::OnUpdatePlants(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_VEGETATION);
	event.Enable(t != NULL);
	event.Check(on);
}

void vtFrame::OnStructures(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_STRUCTURES, !t->GetFeatureVisible(TFT_STRUCTURES));
}

void vtFrame::OnUpdateStructures(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_STRUCTURES);
	event.Enable(t != NULL);
	event.Check(on);
}

void vtFrame::OnRoads(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_ROADS, !t->GetFeatureVisible(TFT_ROADS));
}

void vtFrame::OnFog(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFog(!t->GetFog());
}

void vtFrame::OnUpdateRoads(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_ROADS);
	event.Enable(t != NULL);
	event.Check(on);
}

void vtFrame::OnUpdateFog(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Check(t && t->GetFog());
}

void vtFrame::OnIncrease(wxCommandEvent& event)
{
	ChangeTerrainDetail(true);
}

void vtFrame::OnDecrease(wxCommandEvent& event)
{
	ChangeTerrainDetail(false);
}

void vtFrame::OnSaveVeg(wxCommandEvent& event)
{
	// save current directory
	wxString path = wxGetCwd();

	EnableContinuousRendering(false);
	wxFileDialog saveFile(NULL, _T("Save Vegetation Data"), _T(""), _T(""),
		_T("Vegetation Files (*.vf)|*.vf|"), wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
	{
		wxSetWorkingDirectory(path);	// restore
		return;
	}
	wxString2 str = saveFile.GetPath();

	vtTerrain *pTerr = GetCurrentTerrain();
	vtPlantInstanceArray &pia = pTerr->GetPlantInstances();
	pia.WriteVF(str.mb_str());
}

void vtFrame::OnSaveStruct(wxCommandEvent& event)
{
	// save current directory
	wxString path = wxGetCwd();

	EnableContinuousRendering(false);
	wxFileDialog saveFile(this, _T("Save Built Structures Data"), _T(""),
		_T(""), _T("Structure Files (*.vtst)|*.vtst|"), wxSAVE | wxOVERWRITE_PROMPT);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
	{
		wxSetWorkingDirectory(path);	// restore
		return;
	}
	wxString2 str = saveFile.GetPath();

	vtStructureArray3d *sa = GetCurrentTerrain()->GetStructures();
	sa->SetFilename(str.mb_str());
	sa->WriteXML(str.mb_str());

	// update the displayed filename
	m_pLayerDlg->RefreshTreeContents();
}

static bool s_bBuilt = false;

void vtFrame::OnToggleFoundations(wxCommandEvent& event)
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

void vtFrame::OnUpdateFoundations(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && t->GetStructures() && t->GetStructures()->GetSize() > 0);
	event.Check(s_bBuilt);
}


////////////////// Earth Menu //////////////////////

void vtFrame::OnEarthShowShading(wxCommandEvent& event)
{
	g_App.SetEarthShading(!g_App.GetEarthShading());
}

void vtFrame::OnUpdateEarthShowShading(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthShading());
}

void vtFrame::OnEarthShowAxes(wxCommandEvent& event)
{
	g_App.SetSpaceAxes(!g_App.GetSpaceAxes());
}

void vtFrame::OnUpdateEarthShowAxes(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetSpaceAxes());
}

void vtFrame::OnEarthTilt(wxCommandEvent& event)
{
	g_App.SetEarthTilt(!g_App.GetEarthTilt());
}

void vtFrame::OnUpdateEarthTilt(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthTilt());
}

void vtFrame::OnUpdateInOrbit(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
}

void vtFrame::OnEarthFlatten(wxCommandEvent& event)
{
	g_App.SetEarthShape(!g_App.GetEarthShape());
}

void vtFrame::OnEarthUnfold(wxCommandEvent& event)
{
	g_App.SetEarthUnfold(!g_App.GetEarthUnfold());
}

void vtFrame::OnEarthPoints(wxCommandEvent& event)
{
	// save current directory
	wxString path = wxGetCwd();

	wxFileDialog loadFile(NULL, _T("Load Point Data"), _T(""), _T(""),
		_T("Point Data Sources (*.shp)|*.shp|"), wxOPEN);
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
		wxMessageBox(_T("Couldn't Open"));
	if (ret == -2)
		wxMessageBox(_T("That file isn't point data."));

	// restore
	wxSetWorkingDirectory(path);
}


//////////////////////////////////////////////////////

//
// Called when the GUI needs to be informed of a new terrain
//
void vtFrame::SetTerrainToGUI(vtTerrain *pTerrain)
{
	if (pTerrain)
	{
		m_pLocationDlg->SetTarget(vtGetScene()->GetCamera(),
			pTerrain->GetProjection(), g_Conv);

		vtString loc = "Locations/";
		loc += pTerrain->GetParams().m_strLocFile;
		vtString path = FindFileOnPaths(pTerrain->s_DataPaths, loc);
		if (path != "")
		{
			m_pLocationDlg->SetLocFile((const char *)path);
			m_pLocationDlg->RecallFrom(pTerrain->GetParams().m_strInitLocation);
		}

		m_pInstanceDlg->SetProjection(pTerrain->GetProjection());
		m_pInstanceDlg->SetDataPaths(pTerrain->s_DataPaths);
		m_pDistanceDlg->SetProjection(pTerrain->GetProjection());
	}
	else
	{
		vtProjection geo;
		geo.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
		m_pDistanceDlg->SetProjection(geo);
	}
}

void vtFrame::EarthPosUpdated(const DPoint3 &pos)
{
	m_pInstanceDlg->SetLocation(DPoint2(pos.x, pos.y));
}

void vtFrame::UpdateStatus()
{
	if (m_pStatusBar)
		m_pStatusBar->UpdateText();

#if 0
	vtString vs;
	g_App.GetStatusText(vs);

	wxString2 str;
#if SUPPORT_WSTRING && UNICODE
	wstring2 ws;
	ws.from_utf8(vs);
	str = ws.c_str();
#else
	str = vs;
#endif

	SetStatusText(str);
#endif
}


///////////////////////////////////////////////////////////////////

void vtFrame::ShowPopupMenu(const IPoint2 &pos)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructures();

	wxMenu *popmenu = new wxMenu;
	popmenu->Append(ID_POPUP_PROPERTIES, _T("Properties"));
	if (sa)
	{
		int sel = sa->GetFirstSelected();
		if (sel != -1)
		{
			vtStructureType type = sa->GetAt(sel)->GetType();
			if (type == ST_BUILDING)
				popmenu->Append(ID_POPUP_FLIP, _T("Flip Footprint Direction"));
			if (type == ST_INSTANCE)
				popmenu->Append(ID_POPUP_RELOAD, _T("Reload from Disk"));
		}
	}
	popmenu->AppendSeparator();
	popmenu->Append(ID_POPUP_DELETE, _T("Delete"));

	m_canvas->PopupMenu(popmenu, pos.x, pos.y);
	delete popmenu;
}

void vtFrame::OnPopupProperties(wxCommandEvent& event)
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
				// TODO
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
			// TODO: show properties for this plant
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

void vtFrame::OnPopupFlip(wxCommandEvent& event)
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

void vtFrame::OnPopupReload(wxCommandEvent& event)
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

void vtFrame::OnPopupDelete(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	pTerr->DeleteSelectedStructures();
	pTerr->DeleteSelectedPlants();

	// layer dialog needs to reflect the change
	m_pLayerDlg->RefreshTreeContents();
}

