//
// Name:	 frame.cpp
// Purpose:  The frame class for the wxWindows application.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef UNIX
#include <unistd.h>
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/DynTerrain.h"
#include "vtlib/core/TerrainSurface.h"
#include "vtlib/core/SkyDome.h"

#include "frame.h"

#include "SceneGraphDlg.h"
#include "PlantDlg.h"
#include "FenceDlg.h"
#include "CameraDlg.h"
#include "LocationDlg.h"
#include "BuildingDlg.h"

#include "../Enviro.h"
#include "../TerrainSceneWP.h"	// for GetCurrentTerrain

#include "app.h"
#include "canvas.h"
#include "menu_id.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__)
#  include "camera.xpm"
#  include "fence.xpm"
#  include "loc.xpm"
#  include "maintain.xpm"
#  include "move.xpm"
#  include "nav.xpm"
#  include "nav_fast.xpm"
#  include "nav_set.xpm"
#  include "nav_slow.xpm"
#  include "points.xpm"
#  include "select.xpm"
#  include "sgraph.xpm"
#  include "space.xpm"
#  include "sun.xpm"
#  include "terrain.xpm"
#  include "tree.xpm"
#endif

#if VTLIB_OSG
#include <osgDB/Registry>
#endif

DECLARE_APP(vtApp)

BEGIN_EVENT_TABLE(vtFrame, wxFrame)
	EVT_CHAR(vtFrame::OnChar)
	EVT_MENU(wxID_EXIT, vtFrame::OnExit)

	EVT_MENU(ID_TOOLS_SELECT, vtFrame::OnToolsSelect)
	EVT_UPDATE_UI(ID_TOOLS_SELECT, vtFrame::OnUpdateToolsSelect)
	EVT_MENU(ID_TOOLS_FENCES, vtFrame::OnToolsFences)
	EVT_UPDATE_UI(ID_TOOLS_FENCES, vtFrame::OnUpdateToolsFences)
	EVT_MENU(ID_TOOLS_TREES, vtFrame::OnToolsTrees)
	EVT_UPDATE_UI(ID_TOOLS_TREES, vtFrame::OnUpdateToolsTrees)
	EVT_MENU(ID_TOOLS_MOVE, vtFrame::OnToolsMove)
	EVT_UPDATE_UI(ID_TOOLS_MOVE, vtFrame::OnUpdateToolsMove)
	EVT_MENU(ID_TOOLS_NAVIGATE, vtFrame::OnToolsNavigate)
	EVT_UPDATE_UI(ID_TOOLS_NAVIGATE, vtFrame::OnUpdateToolsNavigate)

	EVT_MENU(ID_VIEW_MAINTAIN, vtFrame::OnViewMaintain)
	EVT_UPDATE_UI(ID_VIEW_MAINTAIN,	vtFrame::OnUpdateViewMaintain)
	EVT_MENU(ID_VIEW_WIREFRAME, vtFrame::OnViewWireframe)
	EVT_UPDATE_UI(ID_VIEW_WIREFRAME,	vtFrame::OnUpdateViewWireframe)
	EVT_MENU(ID_VIEW_FULLSCREEN, vtFrame::OnViewFullscreen)
	EVT_UPDATE_UI(ID_VIEW_FULLSCREEN,	vtFrame::OnUpdateViewFullscreen)
	EVT_MENU(ID_VIEW_TOPDOWN, vtFrame::OnViewTopDown)
	EVT_UPDATE_UI(ID_VIEW_TOPDOWN,	vtFrame::OnUpdateViewTopDown)
	EVT_MENU(ID_VIEW_FRAMERATE, vtFrame::OnViewFramerate)
	EVT_UPDATE_UI(ID_VIEW_FRAMERATE,	vtFrame::OnUpdateViewFramerate)
	EVT_MENU(ID_VIEW_SLOWER, vtFrame::OnViewSlower)
	EVT_UPDATE_UI(ID_VIEW_SLOWER,	vtFrame::OnUpdateViewSlower)
	EVT_MENU(ID_VIEW_FASTER, vtFrame::OnViewFaster)
	EVT_UPDATE_UI(ID_VIEW_FASTER,	vtFrame::OnUpdateViewFaster)
	EVT_MENU(ID_VIEW_SETTINGS, vtFrame::OnViewSettings)
	EVT_MENU(ID_VIEW_LOCATIONS, vtFrame::OnViewLocations)
	EVT_UPDATE_UI(ID_VIEW_LOCATIONS, vtFrame::OnUpdateViewLocations)

	EVT_MENU(ID_SCENE_SCENEGRAPH, vtFrame::OnSceneGraph)
	EVT_MENU(ID_SCENE_TERRAIN, vtFrame::OnSceneTerrain)
	EVT_UPDATE_UI(ID_SCENE_TERRAIN,	vtFrame::OnUpdateSceneTerrain)
	EVT_MENU(ID_SCENE_SPACE, vtFrame::OnSceneSpace)
	EVT_UPDATE_UI(ID_SCENE_SPACE, vtFrame::OnUpdateSceneSpace)
#if VTLIB_OSG
	EVT_MENU(ID_SCENE_SAVE, vtFrame::OnSceneSave)
#endif

	EVT_MENU(ID_TERRAIN_REGULAR, vtFrame::OnRegular)
	EVT_MENU(ID_TERRAIN_DYNAMIC, vtFrame::OnDynamic)
	EVT_MENU(ID_TERRAIN_CULLEVERY, vtFrame::OnCullEvery)
	EVT_MENU(ID_TERRAIN_CULLONCE, vtFrame::OnCullOnce)
	EVT_MENU(ID_TERRAIN_SKY, vtFrame::OnSky)
	EVT_MENU(ID_TERRAIN_OCEAN, vtFrame::OnOcean)
	EVT_MENU(ID_TERRAIN_TREES, vtFrame::OnTrees)
	EVT_MENU(ID_TERRAIN_ROADS, vtFrame::OnRoads)
	EVT_MENU(ID_TERRAIN_INCREASE, vtFrame::OnIncrease)
	EVT_MENU(ID_TERRAIN_DECREASE, vtFrame::OnDecrease)
	EVT_MENU(ID_TERRAIN_SAVEVEG, vtFrame::OnSaveVeg)
	EVT_MENU(ID_TERRAIN_SAVESTRUCT, vtFrame::OnSaveStruct)

	EVT_UPDATE_UI(ID_TERRAIN_REGULAR, vtFrame::OnUpdateRegular)
	EVT_UPDATE_UI(ID_TERRAIN_DYNAMIC, vtFrame::OnUpdateDynamic)
	EVT_UPDATE_UI(ID_TERRAIN_CULLEVERY, vtFrame::OnUpdateCullEvery)
	EVT_UPDATE_UI(ID_TERRAIN_SKY, vtFrame::OnUpdateSky)
	EVT_UPDATE_UI(ID_TERRAIN_OCEAN, vtFrame::OnUpdateOcean)
	EVT_UPDATE_UI(ID_TERRAIN_TREES, vtFrame::OnUpdateTrees)
	EVT_UPDATE_UI(ID_TERRAIN_ROADS, vtFrame::OnUpdateRoads)

	EVT_MENU(ID_EARTH_SHOWTIME, vtFrame::OnEarthShowTime)
	EVT_UPDATE_UI(ID_EARTH_SHOWTIME, vtFrame::OnUpdateEarthShowTime)
	EVT_MENU(ID_EARTH_FLATTEN, vtFrame::OnEarthFlatten)
	EVT_UPDATE_UI(ID_EARTH_FLATTEN, vtFrame::OnUpdateEarthFlatten)
	EVT_MENU(ID_EARTH_POINTS, vtFrame::OnEarthPoints)
	EVT_UPDATE_UI(ID_EARTH_POINTS, vtFrame::OnUpdateEarthPoints)

	EVT_MENU(ID_HELP_ABOUT, vtFrame::OnHelpAbout)

	// Popup
	EVT_MENU(ID_POPUP_PROPERTIES, vtFrame::OnPopupProperties)
	EVT_MENU(ID_POPUP_DELETE, vtFrame::OnPopupDelete)
END_EVENT_TABLE()

// My frame constructor
vtFrame::vtFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style):
	wxFrame(parent, -1, title, pos, size, style)
{
	// Give it an icon
	SetIcon(wxIcon("enviro"));

	m_bCulleveryframe = true;
	m_bMaintainHeight = false;
	m_bFullscreen = false;
	m_bTopDown = false;

	CreateMenus();
	CreateToolbar();
	CreateStatusBar();

	// Make a vtGLCanvas
#ifdef __WXMOTIF__
	// FIXME:  Can remove this special case once wxMotif 2.3 is released
	int gl_attrib[20] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1,
			GLX_DOUBLEBUFFER, None };
#else
	int *gl_attrib = NULL;
#endif

	m_canvas = new vtGLCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
		"vtGLCanvas", gl_attrib);

	// Show the frame
	Show(TRUE);

	m_pSceneGraphDlg = new SceneGraphDlg(this, -1, "Scene Graph",
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(250, 350);

	m_pPlantDlg = new PlantDlg(this, -1, "Plants", wxDefaultPosition);
	m_pFenceDlg = new FenceDlg(this, -1, "Fences", wxDefaultPosition);
	m_pCameraDlg = new CameraDlg(this, -1, "Camera-View", wxDefaultPosition);
	m_pLocationDlg = new LocationDlg(this, -1, "Locations",
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pBuildingDlg = new BuildingDlg(this, -1, "Building Properties", wxDefaultPosition);

	m_canvas->SetCurrent();
}

vtFrame::~vtFrame()
{
	delete m_canvas;
	delete m_pSceneGraphDlg;
	delete m_pPlantDlg;
	delete m_pFenceDlg;
	delete m_pCameraDlg;
	delete m_pLocationDlg;
}

void vtFrame::CreateMenus()
{
	// Make menus
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_EXIT, "E&xit\tEsc", "Exit");

	wxMenu *toolsMenu = new wxMenu;
	toolsMenu->Append(ID_TOOLS_SELECT, "Select", "Select", true);
	toolsMenu->Append(ID_TOOLS_FENCES, "Fences", "Fences", true);
	toolsMenu->Append(ID_TOOLS_TREES, "Trees", "Trees", true);
	toolsMenu->Append(ID_TOOLS_MOVE, "Move Objects", "Move Objects", true);
	toolsMenu->Append(ID_TOOLS_NAVIGATE, "Navigate", "Navigate", true);

	wxMenu *sceneMenu = new wxMenu;
	sceneMenu->Append(ID_SCENE_SCENEGRAPH, "Scene Graph");
	sceneMenu->AppendSeparator();
	sceneMenu->Append(ID_SCENE_TERRAIN, "Go to Terain...");
	sceneMenu->Append(ID_SCENE_SPACE, "Go to Space");
#if VTLIB_OSG
	sceneMenu->AppendSeparator();
	sceneMenu->Append(ID_SCENE_SAVE, "Save scene graph to .osg");
#endif

	wxMenu *viewMenu = new wxMenu;
	viewMenu->Append(ID_VIEW_SLOWER, "Fly Slower (S)");
	viewMenu->Append(ID_VIEW_FASTER, "Fly Faster (F)");
	viewMenu->Append(ID_VIEW_MAINTAIN, "Maintain height above ground (A)", "Maintain height above ground", true);
	viewMenu->Append(ID_VIEW_WIREFRAME, "Wireframe\tCtrl+W", "Wireframe", true);
	viewMenu->Append(ID_VIEW_FULLSCREEN, "Fullscreen\tCtrl+F", "Fullscreen", true);
	viewMenu->Append(ID_VIEW_TOPDOWN, "Top-Down Camera\tCtrl+T", "Top-Down", true);
	viewMenu->Append(ID_VIEW_FRAMERATE, "Framerate Chart\tCtrl+R", "Framerate Chart", true);
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_SETTINGS, "Camera - View Settings", "View Settings");
	viewMenu->Append(ID_VIEW_LOCATIONS, "Store/Recall Locations", "View Locations");

	wxMenu *terrainMenu = new wxMenu;
	terrainMenu->Append(ID_TERRAIN_REGULAR, "Regular Terrain\tF2", "Regular Terrain", true);
	terrainMenu->Append(ID_TERRAIN_DYNAMIC, "Dynamic LOD Terrain\tF3", "Dynamic LOD Terrain", true);
	terrainMenu->Append(ID_TERRAIN_CULLEVERY, "Cull every frame\tCtrl+C", "Cull every frame", true);
	terrainMenu->Append(ID_TERRAIN_CULLONCE, "Cull once\tCtrl+K");
	terrainMenu->AppendSeparator();
	terrainMenu->Append(ID_TERRAIN_SKY, "Show Sky\tF4", "Show Sky", true);
	terrainMenu->Append(ID_TERRAIN_OCEAN, "Show Ocean\tF5", "Show Ocean", true);
	terrainMenu->Append(ID_TERRAIN_TREES, "Show Trees and Buildings\tF6", "Show Trees and Buildings", true);
	terrainMenu->Append(ID_TERRAIN_ROADS, "Show Roads\tF7", "Show Roads", true);
	terrainMenu->AppendSeparator();
	terrainMenu->Append(ID_TERRAIN_INCREASE, "Increase Detail (+)");
	terrainMenu->Append(ID_TERRAIN_DECREASE, "Decrease Detail (-)");
	terrainMenu->AppendSeparator();
	terrainMenu->Append(ID_TERRAIN_SAVEVEG, "Save Vegetation As...");
	terrainMenu->Append(ID_TERRAIN_SAVESTRUCT, "Save Built Structures As...");

	wxMenu *earthMenu = new wxMenu;
	earthMenu->Append(ID_EARTH_SHOWTIME, "&Show Time of Day\tCtrl+I", "Show Time of Day", true);
	earthMenu->Append(ID_EARTH_FLATTEN, "&Flatten\tCtrl+F", "Flatten", true);
	earthMenu->Append(ID_EARTH_POINTS, "&Add Point Data...\tCtrl+P");

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ID_HELP_ABOUT, "About VTP Enviro...");

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(toolsMenu, "&Tools");
	menuBar->Append(sceneMenu, "&Scene");
	menuBar->Append(viewMenu, "&View");
	menuBar->Append(terrainMenu, "Te&rrain");
	menuBar->Append(earthMenu, "&Earth");
	menuBar->Append(helpMenu, "&Help");
	SetMenuBar(menuBar);
}

void vtFrame::CreateToolbar()
{
	// tool bar
	m_pToolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_DOCKABLE);
	m_pToolbar->SetMargins(2, 2);
	m_pToolbar->SetToolBitmapSize(wxSize(20, 20));

	ADD_TOOL(ID_TOOLS_SELECT, wxBITMAP(select), _("Select"), true);
	ADD_TOOL(ID_TOOLS_FENCES, wxBITMAP(fence), _("Create Fences"), true);
	ADD_TOOL(ID_TOOLS_TREES, wxBITMAP(tree), _("Create Plants"), true);
//	ADD_TOOL(ID_TOOLS_MOVE, wxBITMAP(move), _("Move Objects"), true);	// not yet
	ADD_TOOL(ID_TOOLS_NAVIGATE, wxBITMAP(nav), _("Navigate"), true);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_VIEW_MAINTAIN, wxBITMAP(maintain), _("Maintain Height"), true);
	ADD_TOOL(ID_VIEW_FASTER, wxBITMAP(nav_fast), _("Fly Faster"), false);
	ADD_TOOL(ID_VIEW_SLOWER, wxBITMAP(nav_slow), _("Fly Slower"), false);
	ADD_TOOL(ID_VIEW_SETTINGS, wxBITMAP(nav_set), _("Camera Dialog"), false);
	ADD_TOOL(ID_VIEW_LOCATIONS, wxBITMAP(loc), _("Locations"), false);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_SCENE_SCENEGRAPH, wxBITMAP(sgraph), _("Scene Graph"), false);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_SCENE_SPACE, wxBITMAP(space), _("Go to Space"), false);
	ADD_TOOL(ID_SCENE_TERRAIN, wxBITMAP(terrain), _("Go to Terrain"), false);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_EARTH_SHOWTIME, wxBITMAP(sun), _("Time of Day"), true);
	ADD_TOOL(ID_EARTH_POINTS, wxBITMAP(points), _("Add Point Data"), false);

	m_pToolbar->Realize();
}

//
// Utility methods
//

void vtFrame::SetMode(MouseMode mode)
{
	// Show/hide plant dialog
	if (mode == MM_PLANTS)
	{
		m_pPlantDlg->SetPlantList(g_App.GetPlantList());
		m_pPlantDlg->Show(TRUE);
	}
	else
		m_pPlantDlg->Show(FALSE);

	// Show/hide fence dialog
	m_pFenceDlg->Show(mode == MM_FENCES);

	g_App.SetMode(mode);
}

void vtFrame::OnChar(wxKeyEvent& event)
{
	long key = event.KeyCode();

	if (key == 27)
	{
		// Esc: exit application
		m_canvas->m_bRunning = false;
		Destroy();
	}

	// Keyboard shortcuts ("accelerators")
	if (key == 'f')
		ChangeFlightSpeed(1.8f);
	if (key == 's')
		ChangeFlightSpeed(1.0f / 1.8f);
	if (key == 'a')
	{
		m_bMaintainHeight = !m_bMaintainHeight;
		if (g_App.m_pTFlyer != NULL)
		{
			g_App.m_pTFlyer->MaintainHeight(m_bMaintainHeight);
			g_App.m_pTFlyer->SetMaintainHeight(0);
		}
	}
	if (key == '+')
		ChangeTerrainDetail(true);
	if (key == '-')
		ChangeTerrainDetail(false);
}

void vtFrame::ChangeFlightSpeed(float factor)
{
	float speed = g_App.GetFlightSpeed();
	g_App.SetFlightSpeed(speed * factor);

	m_pCameraDlg->GetValues();
	m_pCameraDlg->ValuesToSliders();
	m_pCameraDlg->TransferToWindow();
	m_pCameraDlg->Refresh();
}

void vtFrame::ChangeTerrainDetail(bool bIncrease)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	vtDynTerrainGeom *pTerr = t->m_pDynGeom;
	if (!pTerr) return;

	if (bIncrease)
	{
		pTerr->SetPixelError(pTerr->GetPixelError()+0.1f);
		pTerr->SetPolygonCount(pTerr->GetPolygonCount()+1000);
	}
	else
	{
		pTerr->SetPixelError(pTerr->GetPixelError()-0.1f);
		pTerr->SetPolygonCount(pTerr->GetPolygonCount()-1000);
	}
}

void vtFrame::SetFullScreen(bool bFull)
{
	m_bFullscreen = bFull;
#ifdef __WXMSW__
	if (m_bFullscreen)
	{
#if 0
		int fx, fy, cx, cy;
		GetSize(&fx, &fy);
		m_canvas->GetSize(&cx, &cy);
#endif
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
	m_canvas->m_bRunning = false;
	Destroy();
}

void vtFrame::OnHelpAbout(wxCommandEvent& event)
{
	m_canvas->m_bRunning = false;	// stop rendering

	wxString str = "VTP Enviro\n";
	str += "The runtime environment for the Virtual Terrain Project\n\n";
	str += "Please read the HTML documentation and license.\n\n";
	str += "Send feedback to: ben@vterrain.org\n\n";
	str += "This version was built with the ";
#if VTLIB_DSM
	str += "DSM";
#elif VTLIB_OSG
	str += "OSG";
#elif VTLIB_SGL
	str += "SGL";
#elif VTLIB_SSG
	str += "SSG";
#endif
	str += " Library.\n\n";
	str += "Build date: ";
	str += __DATE__;
	wxMessageBox(str, "About VTP Enviro");

	m_canvas->m_bRunning = true;	// start rendering again
	m_canvas->QueueRefresh(FALSE);
}


//////////////////// View menu //////////////////////////

void vtFrame::OnViewMaintain(wxCommandEvent& event)
{
	m_bMaintainHeight = !m_bMaintainHeight;

	if (g_App.m_pTFlyer != NULL)
	{
		g_App.m_pTFlyer->MaintainHeight(m_bMaintainHeight);
		g_App.m_pTFlyer->SetMaintainHeight(0);
	}
}

void vtFrame::OnUpdateViewMaintain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(m_bMaintainHeight);
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
	event.Check(m_canvas->m_bShowFrameRateChart);
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

void vtFrame::OnViewLocations(wxCommandEvent& event)
{
	m_pLocationDlg->SetTarget(vtGetScene()->GetCamera());
	m_pLocationDlg->Show(true);
}

void vtFrame::OnUpdateViewLocations(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}


///////////////////// Tools menu //////////////////////////

void vtFrame::OnToolsSelect(wxCommandEvent& event)
{
	SetMode(MM_SELECT);
	g_App.EnableFlyerEngine(false);
}

void vtFrame::OnUpdateToolsSelect(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
	event.Check(g_App.m_mode == MM_SELECT);
}

void vtFrame::OnToolsFences(wxCommandEvent& event)
{
	SetMode(MM_FENCES);
	g_App.EnableFlyerEngine(false);
}

void vtFrame::OnUpdateToolsFences(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_FENCES);
}

void vtFrame::OnToolsTrees(wxCommandEvent& event)
{
	SetMode(MM_PLANTS);
	g_App.EnableFlyerEngine(false);
}

void vtFrame::OnUpdateToolsTrees(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_PLANTS);
}

void vtFrame::OnToolsMove(wxCommandEvent& event)
{
	SetMode(MM_MOVE);
	g_App.EnableFlyerEngine(false);
}

void vtFrame::OnUpdateToolsMove(wxUpdateUIEvent& event)
{
	// not yet implemented
	event.Enable(FALSE);
	event.Check(g_App.m_mode == MM_MOVE);
}

void vtFrame::OnToolsNavigate(wxCommandEvent& event)
{
	SetMode(MM_NAVIGATE);
	g_App.EnableFlyerEngine(true);
}

void vtFrame::OnUpdateToolsNavigate(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_NAVIGATE);
}

/////////////////////// Scene menu ///////////////////////////

void vtFrame::OnSceneGraph(wxCommandEvent& event)
{
	m_pSceneGraphDlg->Show(TRUE);
}

void vtFrame::OnSceneTerrain(wxCommandEvent& event)
{
#if 0
	ChooseTerrainDlg dlg(this, -1, "Choose Terrain", wxDefaultPosition);
	dlg.m_strTName = "none";
	dlg.CenterOnParent();
	int result = dlg.ShowModal();
	if (result == wxID_OK)
	{
		wxString fname = dlg.m_strTName;
		g_App.SwitchToTerrain(fname);
	}
#else
	wxString str;
	if (AskForTerrainName(this, str))
		g_App.SwitchToTerrain(str);
#endif
}

void vtFrame::OnUpdateSceneTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
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
	vtRoot *pRoot = GetTerrainScene().m_pTop;
	osgDB::Registry::instance()->writeNode(*pRoot->m_pOsgRoot, "scene.osg");
#endif
}

/////////////////////// Terrain menu ///////////////////////////

void vtFrame::OnRegular(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pTerrainGeom) return;
	bool on = t->m_pTerrainGeom->GetEnabled();

	t->m_pTerrainGeom->SetEnabled(!on);
}

void vtFrame::OnUpdateRegular(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pTerrainGeom)
	{
		event.Enable(false);
		return;
	}
	bool on = t->m_pTerrainGeom->GetEnabled();

	event.Enable(true);
	event.Check(on);
}

void vtFrame::OnDynamic(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pDynGeom) return;
	bool on = t->m_pDynGeom->GetEnabled();

	t->m_pDynGeom->SetEnabled(!on);
}

void vtFrame::OnUpdateDynamic(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pDynGeom)
	{
		event.Enable(false);
		return;
	}
	bool on = t->m_pDynGeom->GetEnabled();

	event.Enable(true);
	event.Check(on);
}

void vtFrame::OnCullEvery(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;

	m_bCulleveryframe = !m_bCulleveryframe;
	t->m_pDynGeom->SetCull(m_bCulleveryframe);
}

void vtFrame::OnUpdateCullEvery(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && t->m_pDynGeom);
	event.Check(m_bCulleveryframe);
}

void vtFrame::OnCullOnce(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	vtDynTerrainGeom *pTerr = t->m_pDynGeom;
	if (!pTerr) return;

	pTerr->CullOnce();
}

void vtFrame::OnSky(wxCommandEvent& event)
{
	vtSkyDome *sky = GetTerrainScene().m_pSkyDome;
	if (!sky) return;
	bool on = sky->GetEnabled();
	sky->SetEnabled(!on);
}

void vtFrame::OnUpdateSky(wxUpdateUIEvent& event)
{
	vtSkyDome *sky = GetTerrainScene().m_pSkyDome;
	if (!sky) return;
	bool on = sky->GetEnabled();
	event.Check(on);
}

void vtFrame::OnOcean(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(OCEAN, !t->GetFeatureVisible(OCEAN));
}

void vtFrame::OnUpdateOcean(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(OCEAN);
	event.Enable(t != NULL);
	event.Check(on);
}

void vtFrame::OnTrees(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(VEGETATION, !t->GetFeatureVisible(VEGETATION));
}

void vtFrame::OnUpdateTrees(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(VEGETATION);
	event.Enable(t != NULL);
	event.Check(on);
}

void vtFrame::OnRoads(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(VEGETATION, !t->GetFeatureVisible(ROADS));
}

void vtFrame::OnUpdateRoads(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(ROADS);
	event.Enable(t != NULL);
	event.Check(on);
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
	char path[256];
	// save current directory
	GetCwd( path, sizeof(path) );

	wxFileDialog saveFile(NULL, "Save Vegetation Data", "", "",
		"Vegetation Files (*.vf)|*.vf|", wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
	{
		SetCwd(path);	// restore
		return;
	}
	wxString str = saveFile.GetPath();

	GetCurrentTerrain()->m_PIA.WriteVF(str);
}

void vtFrame::OnSaveStruct(wxCommandEvent& event)
{
	char path[256];
	// save current directory
	GetCwd( path, sizeof(path) );

	wxFileDialog saveFile(NULL, "Save Built Structures Data", "", "",
		"Structure Files (*.vtst)|*.vtst|", wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
	{
		SetCwd(path);	// restore
		return;
	}
	wxString str = saveFile.GetPath();

	vtStructureArray3d &SA = GetCurrentTerrain()->GetStructures();
	SA.WriteXML(str);
}


////////////////// Earth Menu //////////////////////

void vtFrame::OnEarthShowTime(wxCommandEvent& event)
{
	g_App.SetShowTime(!g_App.GetShowTime());
}

void vtFrame::OnUpdateEarthShowTime(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
}

void vtFrame::OnEarthFlatten(wxCommandEvent& event)
{
	g_App.SetEarthShape(!g_App.GetEarthShape());
}

void vtFrame::OnUpdateEarthFlatten(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
}

void vtFrame::OnEarthPoints(wxCommandEvent& event)
{
	char path[256];
	// save current directory
	GetCwd( path, sizeof(path) );

	wxFileDialog loadFile(NULL, "Load Point Data", "", "",
		"Point Data Sources (*.shp)|*.shp|", wxOPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
	{
		// restore
		SetCwd(path);
		return;
	}

	wxString str = loadFile.GetPath();

	int ret = g_App.AddGlobePoints(str);
	if (ret == -1)
		wxMessageBox("Couldn't Open");
	if (ret == -2)
		wxMessageBox("That file isn't point data.");

	// restore
	SetCwd(path);
}

void vtFrame::OnUpdateEarthPoints(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
}

//////////////////////////////////////////////////////

//
// Called by Enviro when the GUI needs to be informed of a new terrain
//
void SetTerrainToGUI(vtTerrain *pTerrain)
{
	vtFrame *pFrame = (vtFrame *) (wxGetApp().GetTopWindow());

	vtString loc = pTerrain->GetParams().m_strLocFile;
	pFrame->m_pLocationDlg->SetLocFile(pTerrain->m_strDataPath + "Locations/" + loc);
}


///////////////////////////////////////////////////////////////////

void ShowPopupMenu(const IPoint2 &pos)
{
	vtFrame *pFrame = (vtFrame *) (wxGetApp().GetTopWindow());
	pFrame->ShowPopupMenu(pos);
}

void vtFrame::ShowPopupMenu(const IPoint2 &pos)
{
	wxMenu *popmenu = new wxMenu;
	popmenu->Append(ID_POPUP_PROPERTIES, "Properties");
	popmenu->AppendSeparator();
	popmenu->Append(ID_POPUP_DELETE, "Delete");

	m_canvas->PopupMenu(popmenu, pos.x, pos.y);
}

void vtFrame::OnPopupProperties(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d &structures = pTerr->GetStructures();

	int count = structures.GetSize();
	int sel = structures.NumSelected();	// TEMP
	vtStructure3d *str;
	vtBuilding3d *bld;
	vtFence3d *fen;
	for (int i = 0; i < count; i++)
	{
		str = (vtStructure3d *) structures.GetAt(i);
		if (!str->IsSelected())
			continue;

		bld = (vtBuilding3d *) str->GetBuilding();
		fen = (vtFence3d *) str->GetFence();
		if (bld)
		{
			m_pBuildingDlg->Setup(str);
			m_pBuildingDlg->Show(true);
			return;
		}
		if (fen)
		{
//			m_pFenceDlg->Setup(bld);
			m_pFenceDlg->Show(true);
			return;
		}
	}
}

void vtFrame::OnPopupDelete(wxCommandEvent& event)
{
}


// Helper functions for directories

// Get the current working directory
void GetCwd( char path[], int path_size )
{
#if defined(WIN32)
	GetCurrentDirectory(path_size, path);
#elif defined(UNIX)
	if ( getcwd(path, path_size) == NULL ) 
		wxFatalError( "Cannot determine current working directory" );
#else
#  error "I dont know this platform"
#endif
}

// Set the current working directory
void SetCwd( const char path[] )
{
#if defined(WIN32)
	SetCurrentDirectory(path);
#elif defined(UNIX)
	if ( chdir(path) < 0 )
		wxFatalError( "Cannot set current working directory" );
#else
#  error "I dont know this platform"
#endif
}
