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
#include "vtdata/vtLog.h"

#include "frame.h"

#include "SceneGraphDlg.h"
#include "PlantDlg.h"
#include "LinearStructDlg3d.h"
#include "UtilDlg.h"
#include "CameraDlg.h"
#include "LocationDlg.h"
#include "BuildingDlg3d.h"

#include "../Engines.h"
#include "../Enviro.h"	// for GetCurrentTerrain

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
#  include "navroute.xpm"
#  include "points.xpm"
#  include "route.xpm"
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

DECLARE_APP(vtApp);

BEGIN_EVENT_TABLE(vtFrame, wxFrame)
EVT_CHAR(vtFrame::OnChar)
EVT_MENU(wxID_EXIT, vtFrame::OnExit)
EVT_CLOSE(vtFrame::OnClose)

EVT_MENU(ID_TOOLS_SELECT, vtFrame::OnToolsSelect)
EVT_UPDATE_UI(ID_TOOLS_SELECT, vtFrame::OnUpdateToolsSelect)
EVT_MENU(ID_TOOLS_FENCES, vtFrame::OnToolsFences)
EVT_UPDATE_UI(ID_TOOLS_FENCES, vtFrame::OnUpdateToolsFences)
EVT_MENU(ID_TOOLS_ROUTES, vtFrame::OnToolsRoutes)
EVT_UPDATE_UI(ID_TOOLS_ROUTES, vtFrame::OnUpdateToolsRoutes)
EVT_MENU(ID_TOOLS_TREES, vtFrame::OnToolsTrees)
EVT_UPDATE_UI(ID_TOOLS_TREES, vtFrame::OnUpdateToolsTrees)
EVT_MENU(ID_TOOLS_MOVE, vtFrame::OnToolsMove)
EVT_UPDATE_UI(ID_TOOLS_MOVE, vtFrame::OnUpdateToolsMove)
EVT_MENU(ID_TOOLS_NAVIGATE, vtFrame::OnToolsNavigate)
EVT_UPDATE_UI(ID_TOOLS_NAVIGATE, vtFrame::OnUpdateToolsNavigate)

EVT_MENU(ID_VIEW_MAINTAIN, vtFrame::OnViewMaintain)
EVT_UPDATE_UI(ID_VIEW_MAINTAIN,	vtFrame::OnUpdateViewMaintain)
EVT_MENU(ID_VIEW_GRAB_PIVOT, vtFrame::OnViewGrabPivot)
EVT_UPDATE_UI(ID_VIEW_GRAB_PIVOT,	vtFrame::OnUpdateViewGrabPivot)
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
EVT_MENU(ID_VIEW_FOLLOW_ROUTE, vtFrame::OnViewFollowRoute)
EVT_UPDATE_UI(ID_VIEW_FOLLOW_ROUTE, vtFrame::OnUpdateViewFollowRoute)
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

EVT_MENU(ID_TERRAIN_DYNAMIC, vtFrame::OnDynamic)
EVT_MENU(ID_TERRAIN_CULLEVERY, vtFrame::OnCullEvery)
EVT_MENU(ID_TERRAIN_CULLONCE, vtFrame::OnCullOnce)
EVT_MENU(ID_TERRAIN_SKY, vtFrame::OnSky)
EVT_MENU(ID_TERRAIN_OCEAN, vtFrame::OnOcean)
EVT_MENU(ID_TERRAIN_TREES, vtFrame::OnTrees)
EVT_MENU(ID_TERRAIN_ROADS, vtFrame::OnRoads)
EVT_MENU(ID_TERRAIN_FOG, vtFrame::OnFog)
EVT_MENU(ID_TERRAIN_INCREASE, vtFrame::OnIncrease)
EVT_MENU(ID_TERRAIN_DECREASE, vtFrame::OnDecrease)
EVT_MENU(ID_TERRAIN_SAVEVEG, vtFrame::OnSaveVeg)
EVT_MENU(ID_TERRAIN_SAVESTRUCT, vtFrame::OnSaveStruct)

EVT_UPDATE_UI(ID_TERRAIN_DYNAMIC, vtFrame::OnUpdateDynamic)
EVT_UPDATE_UI(ID_TERRAIN_CULLEVERY, vtFrame::OnUpdateCullEvery)
EVT_UPDATE_UI(ID_TERRAIN_SKY, vtFrame::OnUpdateSky)
EVT_UPDATE_UI(ID_TERRAIN_OCEAN, vtFrame::OnUpdateOcean)
EVT_UPDATE_UI(ID_TERRAIN_TREES, vtFrame::OnUpdateTrees)
EVT_UPDATE_UI(ID_TERRAIN_ROADS, vtFrame::OnUpdateRoads)
EVT_UPDATE_UI(ID_TERRAIN_FOG, vtFrame::OnUpdateFog)

EVT_MENU(ID_EARTH_SHOWTIME, vtFrame::OnEarthShowTime)
EVT_UPDATE_UI(ID_EARTH_SHOWTIME, vtFrame::OnUpdateInOrbit)
EVT_MENU(ID_EARTH_FLATTEN, vtFrame::OnEarthFlatten)
EVT_UPDATE_UI(ID_EARTH_FLATTEN, vtFrame::OnUpdateInOrbit)
EVT_MENU(ID_EARTH_POINTS, vtFrame::OnEarthPoints)
EVT_UPDATE_UI(ID_EARTH_POINTS, vtFrame::OnUpdateInOrbit)
EVT_MENU(ID_EARTH_LINEAR, vtFrame::OnEarthLinear)
EVT_UPDATE_UI(ID_EARTH_LINEAR, vtFrame::OnUpdateInOrbit)

EVT_MENU(ID_HELP_ABOUT, vtFrame::OnHelpAbout)

	// Popup
EVT_MENU(ID_POPUP_PROPERTIES, vtFrame::OnPopupProperties)
EVT_MENU(ID_POPUP_FLIP, vtFrame::OnPopupFlip)
EVT_MENU(ID_POPUP_DELETE, vtFrame::OnPopupDelete)
END_EVENT_TABLE()

// My frame constructor
vtFrame::vtFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style):
wxFrame(parent, -1, title, pos, size, style)
{
	// Give it an icon
	SetIcon(wxIcon(_T("enviro")));

	m_bCulleveryframe = true;
	m_bMaintainHeight = false;
	m_bAlwaysMove = false;
	m_bFullscreen = false;
	m_bTopDown = false;

	VTLOG("Frame window: creating menus and toolbars.\n");
	CreateMenus();
	CreateToolbar();
	CreateStatusBar();

	// Make a vtGLCanvas
/*#ifdef __WXMOTIF__
	// FIXME:  Can remove this special case once wxMotif 2.3 is released?
	int gl_attrib[20] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1,
		GLX_DOUBLEBUFFER, None };
#else */

	// We definitely want full color and a 24-bit Z-buffer!
	int gl_attrib[7] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
		WX_GL_BUFFER_SIZE, 24, WX_GL_DEPTH_SIZE, 24, 0	};

	VTLOG("Frame window: creating view canvas.\n");
	m_canvas = new vtGLCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
			_T("vtGLCanvas"), gl_attrib);

	// Show the frame
	Show(TRUE);

	m_pSceneGraphDlg = new SceneGraphDlg(this, -1, _T("Scene Graph"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(250, 350);

	m_pPlantDlg = new PlantDlg(this, -1, _T("Plants"), wxDefaultPosition);
	m_pFenceDlg = new LinearStructureDlg3d(this, -1, _T("Linear Structures"), wxDefaultPosition);
	m_pUtilDlg = new UtilDlg(this, -1, _T("Utility"), wxDefaultPosition);
	m_pCameraDlg = new CameraDlg(this, -1, _T("Camera-View"), wxDefaultPosition);
	m_pLocationDlg = new LocationDlg(this, -1, _T("Locations"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pBuildingDlg = new BuildingDlg3d(this, -1, _T("Building Properties"), wxDefaultPosition);

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
}

void vtFrame::CreateMenus()
{
	// Make menus
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_EXIT, _T("E&xit\tEsc"), _T("Exit"));

	wxMenu *toolsMenu = new wxMenu;
	toolsMenu->AppendCheckItem(ID_TOOLS_SELECT, _T("Select"));
	toolsMenu->AppendCheckItem(ID_TOOLS_FENCES, _T("Fences"));
	toolsMenu->AppendCheckItem(ID_TOOLS_ROUTES, _T("Routes"));
	toolsMenu->AppendCheckItem(ID_TOOLS_TREES, _T("Trees"));
	toolsMenu->AppendCheckItem(ID_TOOLS_MOVE, _T("Move Objects"));
	toolsMenu->AppendCheckItem(ID_TOOLS_NAVIGATE, _T("Navigate"));

	wxMenu *sceneMenu = new wxMenu;
	sceneMenu->Append(ID_SCENE_SCENEGRAPH, _T("Scene Graph"));
	sceneMenu->AppendSeparator();
	sceneMenu->Append(ID_SCENE_TERRAIN, _T("Go to Terrain..."));
	sceneMenu->Append(ID_SCENE_SPACE, _T("Go to Space"));
#if VTLIB_OSG
	sceneMenu->AppendSeparator();
	sceneMenu->Append(ID_SCENE_SAVE, _T("Save scene graph to .osg"));
#endif

	wxMenu *viewMenu = new wxMenu;
	viewMenu->Append(ID_VIEW_SLOWER, _T("Fly Slower (S)"));
	viewMenu->Append(ID_VIEW_FASTER, _T("Fly Faster (F)"));
	viewMenu->AppendCheckItem(ID_VIEW_MAINTAIN, _T("Maintain height above ground (A)"));
	viewMenu->AppendCheckItem(ID_VIEW_GRAB_PIVOT, _T("Use Grab-Pivot Navigation (D)"));
	viewMenu->AppendCheckItem(ID_VIEW_WIREFRAME, _T("Wireframe\tCtrl+W"));
	viewMenu->AppendCheckItem(ID_VIEW_FULLSCREEN, _T("Fullscreen\tCtrl+F"));
	viewMenu->AppendCheckItem(ID_VIEW_TOPDOWN, _T("Top-Down Camera\tCtrl+T"));
	viewMenu->AppendCheckItem(ID_VIEW_FRAMERATE, _T("Framerate Chart\tCtrl+R"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_SETTINGS, _T("Camera - View Settings"));
	viewMenu->Append(ID_VIEW_LOCATIONS, _T("Store/Recall Locations"));

	wxMenu *terrainMenu = new wxMenu;
	terrainMenu->AppendCheckItem(ID_TERRAIN_DYNAMIC, _T("LOD Terrain Surface\tF3"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_CULLEVERY, _T("Cull every frame\tCtrl+C"));
	terrainMenu->Append(ID_TERRAIN_CULLONCE, _T("Cull once\tCtrl+K"));
	terrainMenu->AppendSeparator();
	terrainMenu->AppendCheckItem(ID_TERRAIN_SKY, _T("Show Sky\tF4"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_OCEAN, _T("Show Ocean\tF5"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_TREES, _T("Show Trees and Buildings\tF6"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_ROADS, _T("Show Roads\tF7"));
	terrainMenu->AppendCheckItem(ID_TERRAIN_FOG, _T("Show Fog\tF8"));
	terrainMenu->AppendSeparator();
	terrainMenu->Append(ID_TERRAIN_INCREASE, _T("Increase Detail (+)"));
	terrainMenu->Append(ID_TERRAIN_DECREASE, _T("Decrease Detail (-)"));
	terrainMenu->AppendSeparator();
	terrainMenu->Append(ID_TERRAIN_SAVEVEG, _T("Save Vegetation As..."));
	terrainMenu->Append(ID_TERRAIN_SAVESTRUCT, _T("Save Built Structures As..."));

	wxMenu *earthMenu = new wxMenu;
	earthMenu->AppendCheckItem(ID_EARTH_SHOWTIME, _T("&Show Time of Day\tCtrl+I"));
	earthMenu->AppendCheckItem(ID_EARTH_FLATTEN, _T("&Flatten\tCtrl+E"));
	earthMenu->Append(ID_EARTH_POINTS, _T("&Load Point Data...\tCtrl+P"));
	earthMenu->Append(ID_EARTH_LINEAR, _T("Add &Linear Features...\tCtrl+L"));

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ID_HELP_ABOUT, _T("About VTP Enviro..."));

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(toolsMenu, _T("&Tools"));
	menuBar->Append(sceneMenu, _T("&Scene"));
	menuBar->Append(viewMenu, _T("&View"));
	menuBar->Append(terrainMenu, _T("Te&rrain"));
	menuBar->Append(earthMenu, _T("&Earth"));
	menuBar->Append(helpMenu, _T("&Help"));
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
	ADD_TOOL(ID_TOOLS_ROUTES, wxBITMAP(route), _("Create Routes"), true);
	ADD_TOOL(ID_TOOLS_TREES, wxBITMAP(tree), _("Create Plants"), true);
//	ADD_TOOL(ID_TOOLS_MOVE, wxBITMAP(move), _("Move Objects"), true);	// not yet
	ADD_TOOL(ID_TOOLS_NAVIGATE, wxBITMAP(nav), _("Navigate"), true);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_VIEW_MAINTAIN, wxBITMAP(maintain), _("Maintain Height"), true);
	ADD_TOOL(ID_VIEW_FASTER, wxBITMAP(nav_fast), _("Fly Faster"), false);
	ADD_TOOL(ID_VIEW_SLOWER, wxBITMAP(nav_slow), _("Fly Slower"), false);
	ADD_TOOL(ID_VIEW_SETTINGS, wxBITMAP(nav_set), _("Camera Dialog"), false);
	ADD_TOOL(ID_VIEW_FOLLOW_ROUTE, wxBITMAP(navroute), _("Follow Route"), true);
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
		m_pPlantDlg->SetPlantOptions(g_App.GetPlantOptions());
		m_pPlantDlg->Show(TRUE);
	}
	else
		m_pPlantDlg->Show(FALSE);

	// Show/hide fence dialog
	m_pFenceDlg->Show(mode == MM_FENCES);

	// Show/hide fence dialog
	m_pUtilDlg->Show(mode == MM_ROUTES);

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
	if (key == 'a')
	{
		m_bMaintainHeight = !m_bMaintainHeight;
		if (g_App.m_pTFlyer != NULL)
		{
			g_App.m_pTFlyer->MaintainHeight(m_bMaintainHeight);
			g_App.m_pTFlyer->SetMaintainHeight(0);
		}
	}
	if (key == 'd')
	{
		// Toggle grab-pivot
		if (g_App.m_nav == NT_Normal)
			g_App.m_nav = NT_Grab;
		else
			g_App.m_nav = NT_Normal;
		g_App.EnableFlyerEngine(true);
	}
	if (key == 'f')
		ChangeFlightSpeed(1.8f);
	if (key == 's')
		ChangeFlightSpeed(1.0f / 1.8f);
	if (key == 'w')
	{
		m_bAlwaysMove = !m_bAlwaysMove;
		if (g_App.m_pTFlyer != NULL)
			g_App.m_pTFlyer->SetAlwaysMove(m_bAlwaysMove);
	}
	if (key == '+')
		ChangeTerrainDetail(true);
	if (key == '-')
		ChangeTerrainDetail(false);
	if (key == 'z')
	{
		vtTerrain *pTerr = GetCurrentTerrain();
		if (pTerr && g_App.m_bSelectedStruct)
		{
			vtStructureArray3d &sa = pTerr->GetStructures();
			int i = 0;
			while (!sa.GetAt(i)->IsSelected())
				i++;
			vtBuilding3d *bld = sa.GetBuilding(i);
			// (Do something to the building as a test)
			sa.ConstructStructure(bld);
		}
	}
	if (key == 'Z')
	{
		vtTerrain *pTerr = GetCurrentTerrain();
		if (pTerr && g_App.m_bSelectedStruct)
		{
			vtStructureArray3d &sa = pTerr->GetStructures();
			int i = 0;
			while (!sa.GetAt(i)->IsSelected())
				i++;
			vtBuilding3d *bld = sa.GetBuilding(i);
			sa.ConstructStructure(bld);
		}
	}
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
	m_canvas->m_bRunning = false;
	Destroy();
}

void vtFrame::OnClose(wxCloseEvent &event)
{
	VTLOG("Got Close event, shutting down.\n");
	m_canvas->m_bRunning = false;
	delete m_canvas;
	m_canvas = NULL;
	event.Skip();
}

void vtFrame::OnHelpAbout(wxCommandEvent& event)
{
	m_canvas->m_bRunning = false;	// stop rendering

	wxString str = _T("VTP Enviro\n");
	str += _T("The runtime environment for the Virtual Terrain Project\n\n");
	str += _T("Please read the HTML documentation and license.\n\n");
	str += _T("Send feedback to: ben@vterrain.org\n\n");
	str += _T("This version was built with the ");
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
	wxMessageBox(str, _T("About VTP Enviro"));

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

void vtFrame::OnViewGrabPivot(wxCommandEvent& event)
{
	if (g_App.m_nav == NT_Normal)
		g_App.m_nav = NT_Grab;
	else
		g_App.m_nav = NT_Normal;

	g_App.EnableFlyerEngine(true);
}

void vtFrame::OnUpdateViewGrabPivot(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Grab);
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
	m_pLocationDlg->SetTarget(vtGetScene()->GetCamera(),
		GetCurrentTerrain()->GetProjection(), g_Conv);
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

void vtFrame::OnToolsRoutes(wxCommandEvent& event)
{
	SetMode(MM_ROUTES);
	g_App.EnableFlyerEngine(false);
}

void vtFrame::OnUpdateToolsRoutes(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_ROUTES);
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
	wxString2 str;
	if (AskForTerrainName(this, str))
		g_App.SwitchToTerrain(str.mb_str());
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
	vtRoot *pRoot = GetTerrainScene()->m_pTop;
	osgDB::Registry::instance()->writeNode(*pRoot->m_pOsgRoot, "scene.osg");
#endif
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
	vtSkyDome *sky = GetTerrainScene()->m_pSkyDome;
	if (!sky) return;
	bool on = sky->GetEnabled();
	sky->SetEnabled(!on);
}

void vtFrame::OnUpdateSky(wxUpdateUIEvent& event)
{
	vtSkyDome *sky = GetTerrainScene()->m_pSkyDome;
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

void vtFrame::OnTrees(wxCommandEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_VEGETATION, !t->GetFeatureVisible(TFT_VEGETATION));
}

void vtFrame::OnUpdateTrees(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_VEGETATION);
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
	GetTerrainScene()->ToggleFog();
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
	event.Check(GetTerrainScene()->GetFog());
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

	wxFileDialog saveFile(NULL, _T("Save Vegetation Data"), _T(""), _T(""),
		_T("Vegetation Files (*.vf)|*.vf|"), wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
	{
		wxSetWorkingDirectory(path);	// restore
		return;
	}
	wxString2 str = saveFile.GetPath();

	vtPlantInstanceArray &pia = GetCurrentTerrain()->GetPlantInstances();
	pia.WriteVF(str.mb_str());
}

void vtFrame::OnSaveStruct(wxCommandEvent& event)
{
	// save current directory
	wxString path = wxGetCwd();

	wxFileDialog saveFile(NULL, _T("Save Built Structures Data"), _T(""),
		_T(""), _T("Structure Files (*.vtst)|*.vtst|"), wxSAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
	{
		wxSetWorkingDirectory(path);	// restore
		return;
	}
	wxString2 str = saveFile.GetPath();

	vtStructureArray3d &SA = GetCurrentTerrain()->GetStructures();
	SA.WriteXML(str.mb_str());
}


////////////////// Earth Menu //////////////////////

void vtFrame::OnEarthShowTime(wxCommandEvent& event)
{
	g_App.SetShowTime(!g_App.GetShowTime());
}

void vtFrame::OnUpdateInOrbit(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
}

void vtFrame::OnEarthFlatten(wxCommandEvent& event)
{
	g_App.SetEarthShape(!g_App.GetEarthShape());
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

void vtFrame::OnEarthLinear(wxCommandEvent& event)
{
	SetMode(MM_LINEAR);
}

//////////////////////////////////////////////////////

//
// Called by Enviro when the GUI needs to be informed of a new terrain
//
void SetTerrainToGUI(vtTerrain *pTerrain)
{
	vtFrame *pFrame = (vtFrame *) (wxGetApp().GetTopWindow());

	vtString loc = "Locations/";
	loc += pTerrain->GetParams().m_strLocFile;
	vtString path = FindFileOnPaths(pTerrain->m_DataPaths, loc);
	if (path != "")
		pFrame->m_pLocationDlg->SetLocFile((const char *)path);
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
	popmenu->Append(ID_POPUP_PROPERTIES, _T("Properties"));
	popmenu->Append(ID_POPUP_FLIP, _T("Flip Footprint Direction"));
	popmenu->AppendSeparator();
	popmenu->Append(ID_POPUP_DELETE, _T("Delete"));

	m_canvas->PopupMenu(popmenu, pos.x, pos.y);
}

void vtFrame::OnPopupProperties(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d &structures = pTerr->GetStructures();

	int count = structures.GetSize();
	vtStructure *str;
	vtBuilding3d *bld;
	vtFence3d *fen;
	for (int i = 0; i < count; i++)
	{
		str = structures.GetAt(i);
		if (!str->IsSelected())
			continue;

		bld = structures.GetBuilding(i);
		fen = structures.GetFence(i);
		if (bld)
		{
			m_pBuildingDlg->Setup(bld, pTerr->GetHeightField());
			m_pBuildingDlg->Show(true);
			return;
		}
		if (fen)
		{
			m_pFenceDlg->Show(true);
			return;
		}
	}
}

void vtFrame::OnPopupFlip(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d &structures = pTerr->GetStructures();

	int count = structures.GetSize();
	vtStructure *str;
	vtBuilding3d *bld;
	for (int i = 0; i < count; i++)
	{
		str = structures.GetAt(i);
		if (!str->IsSelected())
			continue;

		bld = structures.GetBuilding(i);
		if (!bld)
			continue;
		bld->FlipFootprintDirection();
		structures.ConstructStructure(structures.GetStructure3d(i));
	}
}

void vtFrame::OnPopupDelete(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	pTerr->DeleteSelectedStructures();
}
