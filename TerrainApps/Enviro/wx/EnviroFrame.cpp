//
// Name:	 EnviroFrame.cpp
// Purpose:  The frame class for the wxEnviro application.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/progdlg.h"
#include "wx/numdlg.h"

#ifdef VTUNIX
#include <unistd.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/Contours.h"
#include "vtlib/core/Fence3d.h"
#include "vtlib/core/PickEngines.h"
#include "vtlib/core/SMTerrain.h"
#include "vtlib/core/SRTerrain.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/TiledGeom.h"
#include "vtlib/vtosg/ScreenCaptureHandler.h"
#include "vtlib/vtosg/SaveImageOSG.h"
#include "vtdata/vtLog.h"
#include "vtdata/TripDub.h"
#include "vtdata/Version.h"	// for About box
#include "vtui/Helper.h"	// for progress dialog
#include "wxosg/GraphicsWindowWX.h"

#include "EnviroFrame.h"
#include "StatusBar.h"

// dialogs
#include "CameraDlg.h"
#include "DistanceDlg3d.h"
#include "DriveDlg.h"
#include "EphemDlg.h"
#include "FeatureTableDlg3d.h"
#include "LayerDlg.h"
#include "LinearStructDlg3d.h"
#include "LocationDlg.h"
#include "LODDlg.h"
#include "PlantDlg.h"
#include "ScenarioSelectDialog.h"
#include "TextureDlg.h"
#include "UtilDlg.h"
#include "VehicleDlg.h"
#include "VIADlg.h"
#include "VIAGDALOptionsDlg.h"

#include "vtui/ContourDlg.h"
#include "vtui/InstanceDlg.h"
#include "vtui/ProfileDlg.h"
#include "vtui/SizeDlg.h"
#include "vtui/TagDlg.h"

#include "wxosg/BuildingDlg3d.h"
#include "wxosg/SceneGraphDlg.h"
#include "wxosg/TimeDlg.h"

#ifdef NVIDIA_PERFORMANCE_MONITORING
#include "PerformanceMonitor.h"
#endif

#include "../Engines.h"
#include "../Options.h"
#include "EnviroGUI.h"	// for GetCurrentTerrain

#include "EnviroApp.h"
#include "canvas.h"
#include "menu_id.h"
#include "StatusBar.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "Enviro_32x32.xpm"
#  include "axes.xpm"
#  include "bld_corner.xpm"
#  include "building.xpm"
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
#  include "placemark.xpm"
#  include "route.xpm"
#  include "scenario.xpm"
#  include "select.xpm"
#  include "select_box.xpm"
#  include "select_move.xpm"
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
#  include "vehicles.xpm"
#  include "view_profile.xpm"
#endif

#include <osg/Version>
#include <osgDB/Registry>	// for "scene save"

#include <gdal_priv.h>
#include <cpl_csv.h>

DECLARE_APP(EnviroApp);

BEGIN_EVENT_TABLE(EnviroFrame, wxFrame)
EVT_CHAR(EnviroFrame::OnChar)
EVT_MENU(ID_FILE_LAYERS,			EnviroFrame::OnFileLayers)
EVT_MENU(ID_FILE_LAYER_CREATE,		EnviroFrame::OnLayerCreate)
EVT_UPDATE_UI(ID_FILE_LAYER_CREATE,	EnviroFrame::OnUpdateLayerCreate)
EVT_MENU(ID_FILE_LAYER_LOAD,		EnviroFrame::OnLayerLoad)
EVT_UPDATE_UI(ID_FILE_LAYER_LOAD,	EnviroFrame::OnUpdateLayerLoad)
EVT_MENU(ID_FILE_SAVE_TERRAIN,		EnviroFrame::OnSaveTerrain)
EVT_MENU(ID_FILE_SAVE_TERRAIN_AS,	EnviroFrame::OnSaveTerrainAs)
EVT_UPDATE_UI(ID_FILE_SAVE_TERRAIN,	EnviroFrame::OnUpdateSaveTerrain)
EVT_MENU(wxID_EXIT,					EnviroFrame::OnExit)
EVT_CLOSE(EnviroFrame::OnClose)
EVT_IDLE(EnviroFrame::OnIdle)

EVT_MENU(ID_TOOLS_SELECT,			EnviroFrame::OnToolsSelect)
EVT_UPDATE_UI(ID_TOOLS_SELECT,		EnviroFrame::OnUpdateToolsSelect)
EVT_MENU(ID_TOOLS_SELECT_BOX,		EnviroFrame::OnToolsSelectBox)
EVT_UPDATE_UI(ID_TOOLS_SELECT_BOX,	EnviroFrame::OnUpdateToolsSelectBox)
EVT_MENU(ID_TOOLS_SELECT_MOVE,		EnviroFrame::OnToolsSelectMove)
EVT_UPDATE_UI(ID_TOOLS_SELECT_MOVE,	EnviroFrame::OnUpdateToolsSelectMove)
EVT_MENU(ID_TOOLS_FENCES,			EnviroFrame::OnToolsFences)
EVT_UPDATE_UI(ID_TOOLS_FENCES,		EnviroFrame::OnUpdateToolsFences)
EVT_MENU(ID_TOOLS_BUILDINGS,		EnviroFrame::OnToolsBuildings)
EVT_UPDATE_UI(ID_TOOLS_BUILDINGS,	EnviroFrame::OnUpdateToolsBuildings)
EVT_MENU(ID_TOOLS_ROUTES,			EnviroFrame::OnToolsRoutes)
EVT_UPDATE_UI(ID_TOOLS_ROUTES,		EnviroFrame::OnUpdateToolsRoutes)
EVT_MENU(ID_TOOLS_PLANTS,			EnviroFrame::OnToolsPlants)
EVT_UPDATE_UI(ID_TOOLS_PLANTS,		EnviroFrame::OnUpdateToolsPlants)
EVT_MENU(ID_TOOLS_POINTS,			EnviroFrame::OnToolsPoints)
EVT_UPDATE_UI(ID_TOOLS_POINTS,		EnviroFrame::OnUpdateToolsPoints)
EVT_MENU(ID_TOOLS_INSTANCES,		EnviroFrame::OnToolsInstances)
EVT_UPDATE_UI(ID_TOOLS_INSTANCES,	EnviroFrame::OnUpdateToolsInstances)
EVT_MENU(ID_TOOLS_VEHICLES,			EnviroFrame::OnToolsVehicles)
EVT_UPDATE_UI(ID_TOOLS_VEHICLES,	EnviroFrame::OnUpdateToolsVehicles)
EVT_MENU(ID_TOOLS_MOVE,				EnviroFrame::OnToolsMove)
EVT_UPDATE_UI(ID_TOOLS_MOVE,		EnviroFrame::OnUpdateToolsMove)
EVT_MENU(ID_TOOLS_NAVIGATE,			EnviroFrame::OnToolsNavigate)
EVT_UPDATE_UI(ID_TOOLS_NAVIGATE,	EnviroFrame::OnUpdateToolsNavigate)
EVT_MENU(ID_TOOLS_MEASURE,			EnviroFrame::OnToolsMeasure)
EVT_UPDATE_UI(ID_TOOLS_MEASURE,		EnviroFrame::OnUpdateToolsMeasure)
EVT_MENU(ID_TOOLS_CONSTRAIN,		EnviroFrame::OnToolsConstrain)
EVT_UPDATE_UI(ID_TOOLS_CONSTRAIN,	EnviroFrame::OnUpdateToolsConstrain)
// Visual impact submenu
EVT_MENU(ID_VIA_CALCULATE, EnviroFrame::OnVIACalculate)
EVT_UPDATE_UI(ID_VIA_CALCULATE, EnviroFrame::OnUpdateVIACalculate)
EVT_MENU(ID_VIA_PLOT, EnviroFrame::OnVIAPlot)
EVT_UPDATE_UI(ID_VIA_PLOT, EnviroFrame::OnUpdateVIAPlot)
EVT_MENU(ID_VIA_CLEAR, EnviroFrame::OnVIAClear)
EVT_UPDATE_UI(ID_VIA_CLEAR, EnviroFrame::OnUpdateVIAClear)

EVT_MENU(ID_VIEW_MAINTAIN,			EnviroFrame::OnViewMaintain)
EVT_UPDATE_UI(ID_VIEW_MAINTAIN,		EnviroFrame::OnUpdateViewMaintain)
EVT_MENU(ID_VIEW_WIREFRAME,			EnviroFrame::OnViewWireframe)
EVT_UPDATE_UI(ID_VIEW_WIREFRAME,	EnviroFrame::OnUpdateViewWireframe)
EVT_MENU(ID_VIEW_FULLSCREEN,		EnviroFrame::OnViewFullscreen)
EVT_UPDATE_UI(ID_VIEW_FULLSCREEN,	EnviroFrame::OnUpdateViewFullscreen)
EVT_MENU(ID_VIEW_TOPDOWN,			EnviroFrame::OnViewTopDown)
EVT_UPDATE_UI(ID_VIEW_TOPDOWN,		EnviroFrame::OnUpdateViewTopDown)
EVT_MENU(ID_VIEW_STATS,				EnviroFrame::OnViewStats)
EVT_MENU(ID_VIEW_ELEV_LEGEND,		EnviroFrame::OnViewElevLegend)
EVT_UPDATE_UI(ID_VIEW_ELEV_LEGEND,	EnviroFrame::OnUpdateViewElevLegend)
EVT_MENU(ID_VIEW_COMPASS,			EnviroFrame::OnViewCompass)
EVT_UPDATE_UI(ID_VIEW_COMPASS,		EnviroFrame::OnUpdateViewCompass)
EVT_MENU(ID_VIEW_MAP_OVERVIEW,		EnviroFrame::OnViewMapOverView)
EVT_UPDATE_UI(ID_VIEW_MAP_OVERVIEW,	EnviroFrame::OnUpdateViewMapOverView)
EVT_MENU(ID_VIEW_DRIVE,				EnviroFrame::OnViewDrive)
EVT_UPDATE_UI(ID_VIEW_DRIVE,		EnviroFrame::OnUpdateViewDrive)
EVT_MENU(ID_VIEW_SETTINGS,			EnviroFrame::OnViewSettings)
EVT_MENU(ID_VIEW_LOCATIONS,			EnviroFrame::OnViewLocations)
EVT_UPDATE_UI(ID_VIEW_LOCATIONS,	EnviroFrame::OnUpdateViewLocations)
EVT_MENU(ID_VIEW_RESET,				EnviroFrame::OnViewReset)
EVT_MENU(ID_VIEW_SNAPSHOT,			EnviroFrame::OnViewSnapshot)
EVT_MENU(ID_VIEW_SNAP_AGAIN,		EnviroFrame::OnViewSnapAgain)
EVT_MENU(ID_VIEW_SNAP_HIGH,			EnviroFrame::OnViewSnapHigh)
EVT_MENU(ID_VIEW_STATUSBAR,			EnviroFrame::OnViewStatusBar)
EVT_UPDATE_UI(ID_VIEW_STATUSBAR,	EnviroFrame::OnUpdateViewStatusBar)
EVT_MENU(ID_VIEW_SCENARIOS,			EnviroFrame::OnViewScenarios)
EVT_MENU(ID_VIEW_PROFILE,			EnviroFrame::OnViewProfile)
EVT_UPDATE_UI(ID_VIEW_PROFILE,		EnviroFrame::OnUpdateViewProfile)

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

EVT_MENU(ID_VIEW_TOOLS_CULTURE,			EnviroFrame::OnViewToolCulture)
EVT_MENU(ID_VIEW_TOOLS_SNAPSHOT,		EnviroFrame::OnViewToolSnapshot)
EVT_MENU(ID_VIEW_TOOLS_TIME,			EnviroFrame::OnViewToolTime)
EVT_UPDATE_UI(ID_VIEW_TOOLS_CULTURE,	EnviroFrame::OnUpdateViewToolCulture)
EVT_UPDATE_UI(ID_VIEW_TOOLS_SNAPSHOT,	EnviroFrame::OnUpdateViewToolSnapshot)
EVT_UPDATE_UI(ID_VIEW_TOOLS_TIME,		EnviroFrame::OnUpdateViewToolTime)

EVT_MENU(ID_SCENE_SCENEGRAPH,	EnviroFrame::OnSceneGraph)
EVT_MENU(ID_SCENE_TERRAIN,		EnviroFrame::OnSceneTerrain)
EVT_UPDATE_UI(ID_SCENE_TERRAIN,	EnviroFrame::OnUpdateSceneTerrain)
EVT_MENU(ID_SCENE_SPACE,		EnviroFrame::OnSceneSpace)
EVT_UPDATE_UI(ID_SCENE_SPACE,	EnviroFrame::OnUpdateSceneSpace)
EVT_MENU(ID_SCENE_SAVE,			EnviroFrame::OnSceneSave)
EVT_MENU(ID_SCENE_EPHEMERIS,	EnviroFrame::OnSceneEphemeris)
#ifdef NVIDIA_PERFORMANCE_MONITORING
EVT_MENU(ID_SCENE_PERFMON,	    EnviroFrame::OnPerformanceMonitor)
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
EVT_MENU(ID_TERRAIN_FOUNDATIONS, EnviroFrame::OnToggleFoundations)
EVT_MENU(ID_TERRAIN_RESHADE,	EnviroFrame::OnTerrainReshade)
EVT_MENU(ID_TERRAIN_CHANGE_TEXTURE,	EnviroFrame::OnTerrainChangeTexture)
EVT_MENU(ID_TERRAIN_DISTRIB_VEHICLES,	EnviroFrame::OnTerrainDistribVehicles)
EVT_MENU(ID_TERRAIN_WRITE_ELEVATION,	EnviroFrame::OnTerrainWriteElevation)
EVT_MENU(ID_TERRAIN_ADD_CONTOUR,	EnviroFrame::OnTerrainAddContour)

EVT_UPDATE_UI(ID_TERRAIN_DYNAMIC,	EnviroFrame::OnUpdateDynamic)
EVT_UPDATE_UI(ID_TERRAIN_CULLEVERY, EnviroFrame::OnUpdateCullEvery)
EVT_UPDATE_UI(ID_TERRAIN_CULLONCE,	EnviroFrame::OnUpdateIsDynTerrain)
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
EVT_UPDATE_UI(ID_TERRAIN_RESHADE,	EnviroFrame::OnUpdateIsDynTerrain)
EVT_UPDATE_UI(ID_TERRAIN_CHANGE_TEXTURE, EnviroFrame::OnUpdateIsDynTerrain)
EVT_UPDATE_UI(ID_TERRAIN_WRITE_ELEVATION,	EnviroFrame::OnUpdateIsDynTerrain)

EVT_MENU(ID_EARTH_SHOWSHADING,	EnviroFrame::OnEarthShowShading)
EVT_MENU(ID_EARTH_SHOWAXES,		EnviroFrame::OnEarthShowAxes)
EVT_MENU(ID_EARTH_TILT,			EnviroFrame::OnEarthTilt)
EVT_MENU(ID_EARTH_FLATTEN,		EnviroFrame::OnEarthFlatten)
EVT_MENU(ID_EARTH_UNFOLD,		EnviroFrame::OnEarthUnfold)
EVT_MENU(ID_EARTH_CLOUDS,		EnviroFrame::OnEarthClouds)
EVT_MENU(ID_EARTH_CLOUDS2,		EnviroFrame::OnEarthClouds2)

EVT_UPDATE_UI(ID_EARTH_SHOWSHADING, EnviroFrame::OnUpdateEarthShowShading)
EVT_UPDATE_UI(ID_EARTH_SHOWAXES, EnviroFrame::OnUpdateEarthShowAxes)
EVT_UPDATE_UI(ID_EARTH_TILT,	EnviroFrame::OnUpdateEarthTilt)
EVT_UPDATE_UI(ID_EARTH_FLATTEN, EnviroFrame::OnUpdateEarthFlatten)
EVT_UPDATE_UI(ID_EARTH_UNFOLD,	EnviroFrame::OnUpdateEarthUnfold)
EVT_UPDATE_UI(ID_EARTH_CLOUDS,	EnviroFrame::OnUpdateEarthClouds)
EVT_UPDATE_UI(ID_EARTH_CLOUDS2,	EnviroFrame::OnUpdateEarthClouds)

EVT_MENU(ID_HELP_ABOUT, EnviroFrame::OnHelpAbout)
EVT_MENU(ID_HELP_DOC_LOCAL, EnviroFrame::OnHelpDocLocal)
EVT_MENU(ID_HELP_DOC_ONLINE, EnviroFrame::OnHelpDocOnline)

// Popup
EVT_MENU(ID_POPUP_PROPERTIES, EnviroFrame::OnPopupProperties)
EVT_MENU(ID_POPUP_FLIP, EnviroFrame::OnPopupFlip)
EVT_MENU(ID_POPUP_COPY_STYLE, EnviroFrame::OnPopupCopyStyle)
EVT_MENU(ID_POPUP_PASTE_STYLE, EnviroFrame::OnPopupPasteStyle)
EVT_MENU(ID_POPUP_RELOAD, EnviroFrame::OnPopupReload)
EVT_MENU(ID_POPUP_SHADOW, EnviroFrame::OnPopupShadow)
EVT_MENU(ID_POPUP_ADJUST, EnviroFrame::OnPopupAdjust)
EVT_MENU(ID_POPUP_START, EnviroFrame::OnPopupStart)
EVT_MENU(ID_POPUP_DELETE, EnviroFrame::OnPopupDelete)
EVT_MENU(ID_POPUP_URL, EnviroFrame::OnPopupURL)
EVT_MENU(ID_POPUP_VIA, EnviroFrame::OnPopupVIA)
EVT_UPDATE_UI(ID_POPUP_VIA, EnviroFrame::OnUpdatePopupVIA)
EVT_MENU(ID_POPUP_VIA_TARGET, EnviroFrame::OnPopupVIATarget)
EVT_UPDATE_UI(ID_POPUP_VIA_TARGET, EnviroFrame::OnUpdatePopupVIATarget)
END_EVENT_TABLE()

wxBitmap ToolsFunc( size_t index )
{
    if (index == 0)
    {
        /* XPM */
        static const char *xpm_data[] = {
        /* columns rows colors chars-per-pixel */
        "20 20 4 1",
        "  c None",
        "a c Black",
        "b c #FFFFFF",
        "c c #FFFF00",
        /* pixels */
        "acacacacacaca       ",
        "c           c       ",
        "a           a       ",
        "c           c       ",
        "a           a       ",
        "c           c       ",
        "a          aa       ",
        "c          aa       ",
        "acacacacacaaba      ",
        "           abba     ",
        "           abbba    ",
        "           abbbba   ",
        "           abbbbba  ",
        "           abbbbbba ",
        "           abbbbaaaa",
        "           abbabba  ",
        "           abaabba  ",
        "           aa  abba ",
        "           a   abba ",
        "                abba"
        };
        wxBitmap bitmap( xpm_data );
        return bitmap;
    }
    if (index == 1)
    {
        /* XPM */
        static const char *xpm_data[] = {
        /* columns rows colors chars-per-pixel */
        "20 20 4 1",
        "  c None",
        "a c Black",
        "b c #FFFFFF",
        "c c #808080",
        /* pixels */
        "             cc     ",
        "           caaac    ",
        "         caaaaaac   ",
        "            ac      ",
        "   c       ac   c   ",
        "  ca      ac    ac  ",
        " caaaaaaaaaaaaaaaac ",
        " caaccccccaaccccaac ",
        "  ca    acaba   ac  ",
        "   c   ac abba  c   ",
        "      ac  abbba     ",
        "   caaaaaaabbbba    ",
        "    caaac abbbbba   ",
        "     cc   abbbbbba  ",
        "          abbbbaaaa ",
        "          abbabba   ",
        "          abaabba   ",
        "          aa  abba  ",
        "          a   abba  ",
        "               abba "
        };
        wxBitmap bitmap( xpm_data );
        return bitmap;
    }
    return wxNullBitmap;
}

// Helper
class InstanceDlg3d: public InstanceDlg
{
public:
	InstanceDlg3d(EnviroFrame *frame, wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos, const wxSize& size, long style) :
		InstanceDlg( parent, id, title, pos, size, style )
	{
		m_pFrame = frame;
	}
	virtual void OnCreate()
	{
		m_pFrame->CreateInstance(m_pos, GetTagArray());
	}
	EnviroFrame *m_pFrame;
};


/////////////////////////////////////////////////////////////////////////////
//
// Frame constructor
//
EnviroFrame::EnviroFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style, bool bVerticalToolbar, bool bEnableEarth) :
		wxFrame(parent, -1, title, pos, size, style)
{
	VTLOG1("Frame constructor.\n");
	m_bCloseOnIdle = false;

	// tell wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);

	// Give the frame an icon
	VTLOG1(" Setting icon\n");
#ifdef ICON_NAME
	SetIcon(wxIcon(wxString(ICON_NAME, wxConvUTF8)));
#else
	SetIcon(wxICON(Enviro));
#endif

#if wxUSE_DRAG_AND_DROP
	SetDropTarget(new DnDFile);
#endif

	m_bCulleveryframe = true;
	m_bAlwaysMove = false;
	m_bFullscreen = false;
	m_bTopDown = false;
	m_ToggledMode = MM_SELECT;
	m_bEnableEarth = bEnableEarth;
	m_bEarthLines = false;
	m_bVerticalToolbar = bVerticalToolbar;

	m_pStatusBar = NULL;
	m_pToolbar = NULL;

	VTLOG1("Frame window: creating menus and toolbars.\n");
	CreateMenus();

	// Create StatusBar
	m_pStatusBar = new MyStatusBar(this);
	SetStatusBar(m_pStatusBar);
	m_pStatusBar->Show();
	m_pStatusBar->UpdateText();
	PositionStatusBar();

#ifdef NVIDIA_PERFORMANCE_MONITORING
	// Stop crash in update toolbar
	m_pCameraDlg = NULL;
	m_pLocationDlg = NULL;
	m_pLODDlg = NULL;
	m_pPerformanceMonitorDlg = NULL;
#endif

	// We definitely want full color and a 24-bit Z-buffer!
	int gl_attrib[8] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
		WX_GL_BUFFER_SIZE, 24, WX_GL_DEPTH_SIZE, 24, 0, 0 };
	if (g_Options.m_bStereo && g_Options.m_iStereoMode == 1)	// 1 = Quad-buffer stereo
	{
		gl_attrib[6] = WX_GL_STEREO;
		gl_attrib[7] = 0;
	}

	VTLOG("Frame window: creating view canvas.\n");
	m_canvas = new EnviroCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
			_T("vtGLCanvas"), gl_attrib);

	// Show the frame
	VTLOG("Showing the main frame\n");
	Show(true);

	VTLOG("Constructing dialogs\n");
	m_pBuildingDlg = new BuildingDlg3d(this, -1, _("Building Properties"));
	m_pCameraDlg = new CameraDlg(this, -1, _("Camera-View"));
	m_pDistanceDlg = new DistanceDlg3d(this, -1, _("Distance"));
	m_pEphemDlg = new EphemDlg(this, -1, _("Ephemeris"));
	m_pFenceDlg = new LinearStructureDlg3d(this, -1, _("Linear Structures"));
	m_pInstanceDlg = new InstanceDlg3d(this, this, -1, _("Instances"),
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pTagDlg = new TagDlg(this, -1, _("Tags"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pTagDlg->SetSize(440,80);
	m_pLODDlg = new LODDlg(this, -1, _("Terrain LOD Info"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	// m_pLODDlg->Show();	// Enable this to see the LOD dialog immediately

	m_pPlantDlg = new PlantDlg(this, -1, _("Plants"));
	m_pPlantDlg->ShowOnlyAvailableSpecies(g_Options.m_bOnlyAvailableSpecies);
	m_pPlantDlg->SetLang(wxGetApp().GetLanguageCode());

	m_pLocationDlg = new LocationDlg(this, -1, _("Locations"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg = new SceneGraphDlg(this, -1, _("Scene Graph"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(450, 600);
	m_pTimeDlg = new TimeDlg(this, -1, _("Time"));
	m_pUtilDlg = new UtilDlg(this, -1, _("Routes"));
	m_pScenarioSelectDialog = new ScenarioSelectDialog(this, -1, _("Scenarios"));
	m_pVehicleDlg = new VehicleDlg(this, -1, _("Vehicles"));
	m_pDriveDlg = new DriveDlg(this);
	m_pProfileDlg = NULL;
	#ifdef NVIDIA_PERFORMANCE_MONITORING
    m_pPerformanceMonitorDlg = new CPerformanceMonitorDialog(this, wxID_ANY, _("Performance Monitor"));
    #endif
	m_pVIADlg = new VIADlg(this);

#if wxVERSION_NUMBER < 2900		// before 2.9.0
	if (m_canvas)
		m_canvas->SetCurrent();
#else
	// Still need to do a "SetCurrent" here? It's more complicated now in wx 2.9.x,
	//  and it's probably already taken care of by GraphicsWindowWX?
#endif

	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().
				  Name(wxT("canvas")).Caption(wxT("Canvas")).
				  CenterPane());
	m_mgr.Update();

	m_pLayerDlg = new LayerDlg(this, -1, _("Layers"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pLayerDlg->SetSize(500, -1);

	m_mgr.AddPane(m_pLayerDlg, wxAuiPaneInfo().
				  Name(_T("layers")).Caption(_("Layers")).
				  Left().Hide());
	m_mgr.Update();
}

EnviroFrame::~EnviroFrame()
{
	VTLOG("Deleting Frame\n");

	m_mgr.UnInit();

	DeleteCanvas();

	delete m_pSceneGraphDlg;
	delete m_pPlantDlg;
	delete m_pFenceDlg;
	delete m_pTagDlg;
	delete m_pUtilDlg;
	delete m_pCameraDlg;
	delete m_pLocationDlg;
	delete m_pInstanceDlg;
	delete m_pLayerDlg;
	#ifdef NVIDIA_PERFORMANCE_MONITORING
    delete m_pPerformanceMonitorDlg;
    #endif
	delete m_pVIADlg;

	delete m_pStatusBar;
	delete m_pToolbar;
	SetStatusBar(NULL);
}

void EnviroFrame::DeleteCanvas()
{
	// Tell our graphics context that there is no canvas.
	GraphicsWindowWX *pGW = (GraphicsWindowWX*)vtGetScene()->GetGraphicsContext();
	if (pGW) {
		pGW->CloseOsgContext();
		pGW->SetCanvas(NULL);
	}
	if (m_canvas)
	{
		m_canvas->m_bRunning = false;
		delete m_canvas;
		m_canvas = NULL;
	}
}

void EnviroFrame::CreateMenus()
{
	// Make menu bar
	m_pMenuBar = new wxMenuBar;

	// Make menus
	m_pFileMenu = new wxMenu;
	m_pFileMenu->Append(ID_FILE_LAYERS, _("Layers"), _("Layers"));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(ID_FILE_LAYER_CREATE, _("Create Layer"), _("Create Layer"));
	m_pFileMenu->Append(ID_FILE_LAYER_LOAD, _("Load Layer"), _("Load Layer"));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(ID_FILE_SAVE_TERRAIN, _("Save Terrain State"));
	m_pFileMenu->Append(ID_FILE_SAVE_TERRAIN_AS, _("Save Terrain State As..."));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(wxID_EXIT, _("E&xit (Esc)"), _("Exit"));
	m_pMenuBar->Append(m_pFileMenu, _("&File"));

	m_pToolsMenu = new wxMenu;
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_SELECT, _("Select"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_SELECT_BOX, _("Select Box"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_SELECT_MOVE, _("Select and Move"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_FENCES, _("Fences"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_BUILDINGS, _("Buildings"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_ROUTES, _("Routes"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_PLANTS, _("Plants"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_POINTS, _("Points"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_INSTANCES, _("Instances"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_VEHICLES, _("Vehicles"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_MOVE, _("Move Objects"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_NAVIGATE, _("Navigate"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_MEASURE, _("Measure Distances\tCtrl+D"));
	m_pToolsMenu->AppendSeparator();
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_CONSTRAIN, _("Constrain building angles"));
#ifdef VISUAL_IMPACT_CALCULATOR
	wxMenu *pVIAMenu = new wxMenu;
	pVIAMenu->Append(ID_VIA_CALCULATE, _("&Calculate\tCtrl+C"), _("Calculate visual impact factor"));
	pVIAMenu->Append(ID_VIA_PLOT, _("&Plot\tCtrl+P"), _("Produce visual impact plot"));
	pVIAMenu->Append(ID_VIA_CLEAR, _("C&lear"), _("Clear visual impact target and all contributors"));
	m_pToolsMenu->AppendSeparator();
	m_pToolsMenu->Append(ID_VIA_MENU, _("Visual Impact.."), pVIAMenu);
#endif
	m_pMenuBar->Append(m_pToolsMenu, _("&Tools"));

	// shortcuts:
	// Ctrl+A Show Axes
	// Ctrl+D Measure Distances
	// Ctrl+E Flatten
	// Ctrl+F Fullscreen
	// Ctrl+G Go to Terrain
	// Ctrl+I Time
	// Ctrl+L Store/Recall Locations
	// Ctrl+N Save Numbered Snapshot
	// Ctrl+P ePhemeris
	// Ctrl+Q Terrain LOD Info
	// Ctrl+R Reset Camera
	// Ctrl+S Camera - View Settings
	// Ctrl+T Top-Down
	// Ctrl+U Unfold
	// Ctrl+W Wireframe
	//
	// A Maintain height above ground
	// D Toggle Grab-Pivot
	// F Faster
	// S Faster
	// W Navigate w/o mouse button

	m_pSceneMenu = new wxMenu;
	m_pSceneMenu->Append(ID_SCENE_SCENEGRAPH, _("Scene Graph"));
    #ifdef NVIDIA_PERFORMANCE_MONITORING
    m_pSceneMenu->Append(ID_SCENE_PERFMON, _("Performance Monitor"));
    #endif
	m_pSceneMenu->AppendSeparator();
	m_pSceneMenu->Append(ID_SCENE_TERRAIN, _("Go to Terrain...\tCtrl+G"));
	if (m_bEnableEarth)
	{
		m_pSceneMenu->Append(ID_SCENE_SPACE, _("Go to Space"));
	}
	m_pSceneMenu->AppendSeparator();
	m_pSceneMenu->Append(ID_SCENE_SAVE, _("Save scene graph to .osg"));
	m_pSceneMenu->AppendSeparator();
	m_pSceneMenu->Append(ID_SCENE_EPHEMERIS, _("Ephemeris...\tCtrl+P"));
	m_pSceneMenu->Append(ID_TIME_DIALOG, _("Time...\tCtrl+I"));
	m_pSceneMenu->Append(ID_TIME_STOP, _("Time Stop"));
	m_pSceneMenu->Append(ID_TIME_FASTER, _("Time Faster"));
    m_pMenuBar->Append(m_pSceneMenu, _("&Scene"));

	m_pViewMenu = new wxMenu;
	m_pViewMenu->AppendCheckItem(ID_VIEW_WIREFRAME, _("Wireframe\tCtrl+W"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_FULLSCREEN, _("Fullscreen\tCtrl+F"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_TOPDOWN, _("Top-Down Camera\tCtrl+T"));
	m_pViewMenu->Append(ID_VIEW_STATS, _("Rendering Statistics\tx"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_ELEV_LEGEND, _("Elevation Legend"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_COMPASS, _("Compass"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_MAP_OVERVIEW, _("Overview"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_DRIVE, _("Drive Vehicle"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->Append(ID_VIEW_SETTINGS, _("Camera - View Settings\tCtrl+S"));
	m_pViewMenu->Append(ID_VIEW_LOCATIONS, _("Store/Recall Locations\tCtrl+L"));
	m_pViewMenu->Append(ID_VIEW_RESET, _("Reset to Terrain Center\tCtrl+R"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->Append(ID_VIEW_SNAPSHOT, _("Save Window Snapshot"));
	m_pViewMenu->Append(ID_VIEW_SNAP_AGAIN, _("Save Numbered Snapshot\tCtrl+N"));
	m_pViewMenu->Append(ID_VIEW_SNAP_HIGH, _("High-resolution Snapshot"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Status Bar"));
	m_pViewMenu->Append(ID_VIEW_SCENARIOS, _("Scenarios"));
	m_pMenuBar->Append(m_pViewMenu, _("&View"));

		// submenu
		wxMenu *tbMenu = new wxMenu;
		tbMenu->AppendCheckItem(ID_VIEW_TOOLS_CULTURE, _("Culture"));
		tbMenu->AppendCheckItem(ID_VIEW_TOOLS_SNAPSHOT, _("Snapshot"));
		tbMenu->AppendCheckItem(ID_VIEW_TOOLS_TIME, _("Time"));
		m_pViewMenu->Append(0, _("Toolbars"), tbMenu);

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
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_CULLEVERY, _("Cull every frame"));
	m_pTerrainMenu->Append(ID_TERRAIN_CULLONCE, _("Cull once"));
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
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_FOUNDATIONS, _("Toggle Artificial Foundations"));
	m_pTerrainMenu->Append(ID_TERRAIN_RESHADE, _("&Recalculate Shading\tCtrl+H"));
	m_pTerrainMenu->Append(ID_TERRAIN_CHANGE_TEXTURE, _("&Change Texture"));
	m_pTerrainMenu->Append(ID_TERRAIN_DISTRIB_VEHICLES, _("&Distribute Vehicles (test)"));
	m_pTerrainMenu->Append(ID_TERRAIN_WRITE_ELEVATION, _("Write Elevation to BT"));
	m_pTerrainMenu->Append(ID_TERRAIN_ADD_CONTOUR, _("Add Contours"));
	m_pMenuBar->Append(m_pTerrainMenu, _("Te&rrain"));

	if (m_bEnableEarth)
	{
		m_pEarthMenu = new wxMenu;
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWSHADING, _("&Show Shading"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWAXES, _("Show &Axes\tCtrl+A"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_TILT, _("Seasonal &Tilt"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_FLATTEN, _("&Flatten\tCtrl+E"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_UNFOLD, _("&Unfold\tCtrl+U"));
		m_pEarthMenu->Append(ID_EARTH_CLOUDS, _T("Load Clouds..."));
#if SUPPORT_CURL
		m_pEarthMenu->Append(ID_EARTH_CLOUDS2, _T("Load Clouds from URL..."));
#endif
		m_pMenuBar->Append(m_pEarthMenu, _("&Earth"));
	}

	wxMenu *helpMenu = new wxMenu;
	wxString about = _("About");
	about += _T(" ");
	about += wxString(STRING_APPORG, wxConvUTF8);
	about += _T("...");
	helpMenu->Append(ID_HELP_ABOUT, about);
	helpMenu->Append(ID_HELP_DOC_LOCAL, _("Documentation (local)"));
	helpMenu->Append(ID_HELP_DOC_ONLINE, _("Documentation (online)"));
	m_pMenuBar->Append(helpMenu, _("&Help"));

	SetMenuBar(m_pMenuBar);
}

void EnviroFrame::CreateToolbar()
{
	long style = wxTB_FLAT | wxTB_NODIVIDER;	// wxTB_DOCKABLE is GTK-only
	if (m_bVerticalToolbar)
		style |= wxTB_VERTICAL;

	if (m_pToolbar != NULL)
		delete m_pToolbar;

	// Create
	m_pToolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
					               style);
	m_pToolbar->SetMargins(1, 1);
	m_pToolbar->SetToolBitmapSize(wxSize(20, 20));

	// populate the toolbar with buttons
	RefreshToolbar();

	wxAuiPaneInfo api;
	api.Name(_T("toolbar"));
	api.ToolbarPane();
	if (m_bVerticalToolbar)
	{
		api.GripperTop();
		api.Left();
		api.TopDockable(false);
		api.BottomDockable(false);
	}
	else
	{
		api.Top();
		api.LeftDockable(false);
		api.RightDockable(false);
	}
	wxSize best = m_pToolbar->GetBestSize();
	api.MinSize(best);
	api.Floatable(false);
	m_mgr.AddPane(m_pToolbar, api);
	m_mgr.Update();
}

void EnviroFrame::RefreshToolbar()
{
	if (!m_pToolbar)	// safety check
		return;

	// remove any existing buttons
	int count = m_pToolbar->GetToolsCount();
#ifdef __WXMAC__
	// Nino says: I spent a long time with this one, the issue was definitely
	// deep in wxMac, but I don't remember, nor want to repeat it :).  Can we
	// #ifdef __WXMAC__ for the time being to revisit it after Xcode is working?
	while (count >= 1)
	{
		m_pToolbar->DeleteToolByPos(count-1);
		count = m_pToolbar->GetToolsCount();
	}
	m_pToolbar->Realize();
	AddTool(ID_TOOLS_SELECT, wxBITMAP(select), _("Select"), true);
#else
	if (!count)
	{
		AddTool(ID_TOOLS_SELECT, wxBITMAP(select), _("Select"), true);
		count = 1;
	}
	while (count > 1)
	{
		m_pToolbar->DeleteToolByPos(count-1);
		count = m_pToolbar->GetToolsCount();
	}
#endif

	bool bEarth = (g_App.m_state == AS_Orbit);
	bool bTerr = (g_App.m_state == AS_Terrain);

	if (bTerr)
	{
		AddTool(ID_TOOLS_SELECT_BOX, wxBITMAP(select_box), _("Select Box"), true);
		if (g_Options.m_bShowToolsCulture)
		{
			AddTool(ID_TOOLS_SELECT_MOVE, wxBITMAP(select_move), _("Select and Move"), true);
			AddTool(ID_TOOLS_MOVE, wxBITMAP(move), _("Move Objects"), true);
			AddTool(ID_TOOLS_FENCES, wxBITMAP(fence), _("Create Fences"), true);
			AddTool(ID_TOOLS_BUILDINGS, wxBITMAP(building), _("Create Buildings"), true);
			AddTool(ID_TOOLS_ROUTES, wxBITMAP(route), _("Create Routes"), true);
			AddTool(ID_TOOLS_PLANTS, wxBITMAP(tree), _("Create Plants"), true);
		}
		AddTool(ID_TOOLS_POINTS, wxBITMAP(placemark), _("Create Points"), true);
		if (g_Options.m_bShowToolsCulture)
		{
			AddTool(ID_TOOLS_INSTANCES, wxBITMAP(instances), _("Create Instances"), true);
			AddTool(ID_TOOLS_VEHICLES, wxBITMAP(vehicles), _("Create Vehicles"), true);
		}
		AddTool(ID_TOOLS_NAVIGATE, wxBITMAP(nav), _("Navigate"), true);
	}
	if (bTerr || bEarth)
	{
		AddTool(ID_TOOLS_MEASURE, wxBITMAP(distance), _("Measure Distance"), true);
	}
	if (bTerr)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_VIEW_PROFILE, wxBITMAP(view_profile), _("Elevation Profile"), true);
		AddTool(ID_TOOLS_CONSTRAIN, wxBITMAP(bld_corner), _("Constrain Angles"), true);
	}
	if (g_Options.m_bShowToolsSnapshot)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_VIEW_SNAPSHOT, wxBITMAP(snap), _("Snapshot"), false);
		AddTool(ID_VIEW_SNAP_AGAIN, wxBITMAP(snap_num), _("Numbered Snapshot"), false);
	}
	if (bTerr || bEarth)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_FILE_LAYERS, wxBITMAP(layers), _("Show Layer Dialog"), false);
	}
	if (bTerr)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_VIEW_MAINTAIN, wxBITMAP(maintain), _("Maintain Height"), true);
		AddTool(ID_VIEW_FASTER, wxBITMAP(nav_fast), _("Fly Faster"), false);
		AddTool(ID_VIEW_SLOWER, wxBITMAP(nav_slow), _("Fly Slower"), false);
		AddTool(ID_VIEW_SETTINGS, wxBITMAP(nav_set), _("Camera Dialog"), false);
		AddTool(ID_VIEW_LOCATIONS, wxBITMAP(loc), _("Locations"), false);
	}
	if (m_bEnableEarth)
	{
		m_pToolbar->AddSeparator();
		if (bTerr) AddTool(ID_SCENE_SPACE, wxBITMAP(space), _("Go to Space"), false);
		AddTool(ID_SCENE_TERRAIN, wxBITMAP(terrain), _("Go to Terrain"), false);
		if (bEarth)
		{
			m_pToolbar->AddSeparator();
			AddTool(ID_EARTH_SHOWSHADING, wxBITMAP(sun), _("Show Sunlight"), true);
			AddTool(ID_EARTH_SHOWAXES, wxBITMAP(axes), _("Axes"), true);
			AddTool(ID_EARTH_TILT, wxBITMAP(tilt), _("Tilt"), true);
			AddTool(ID_EARTH_UNFOLD, wxBITMAP(unfold), _("Unfold"), true);
		}
	}
	if (g_Options.m_bShowToolsTime)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_TIME_DIALOG, wxBITMAP(time), _("Time"), false);
		AddTool(ID_TIME_FASTER, wxBITMAP(faster), _("Time Faster"), false);
		AddTool(ID_TIME_STOP, wxBITMAP(stop), _("Time Stop"), false);
	}

	m_pToolbar->AddSeparator();
	AddTool(ID_SCENE_SCENEGRAPH, wxBITMAP(sgraph), _("Scene Graph"), false);

	VTLOG1("Realize toolbar.\n");
	m_pToolbar->Realize();

	// "commit" all changes made to wxAuiManager
	wxAuiPaneInfo &api = m_mgr.GetPane(wxT("toolbar"));
	if (api.IsOk())
	{
		wxSize best = m_pToolbar->GetBestSize();
		api.MinSize(best);
		m_mgr.Update();
	}
}

void EnviroFrame::AddTool(int id, const wxBitmap &bmp, const wxString &tooltip, bool tog)
{
	m_pToolbar->AddTool(id, tooltip, bmp, tooltip,
		tog ? wxITEM_CHECK : wxITEM_NORMAL);
}

//
// Utility methods
//

void EnviroFrame::SetMode(MouseMode mode)
{
	// Show/hide the modeless dialogs as appropriate
	m_pUtilDlg->Show(mode == MM_ROUTES);
	m_pInstanceDlg->Show(mode == MM_INSTANCES);
	m_pDistanceDlg->Show(mode == MM_MEASURE);
	m_pVehicleDlg->Show(mode == MM_VEHICLES);

	g_App.SetMode(mode);

	if (mode == MM_FENCES)
		OpenFenceDialog();
	else
		m_pFenceDlg->Show(false);

	// Show/hide plant dialog
	if (mode == MM_PLANTS)
	{
		VTLOG1("Calling Plant dialog\n");

		// Make sure the species file and appearances are available
		g_App.LoadSpeciesList();
		GetCurrentTerrain()->SetPlantList(g_App.GetPlantList());

		m_pPlantDlg->SetPlantList(g_App.GetPlantList());
		m_pPlantDlg->SetDlgPlantOptions(g_App.GetPlantOptions());
	}
	m_pPlantDlg->Show(mode == MM_PLANTS);
}

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
		SetTerrainDetail(GetTerrainDetail()+1000);
		break;
	case '-':
		SetTerrainDetail(GetTerrainDetail()-1000);
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
		}
		break;
	case ']':
		{
			float exag = pTerr->GetVerticalExag();
			exag *= 1.01;
			pTerr->SetVerticalExag(exag);
		}
		break;

	case 'e':
		m_bEarthLines = !m_bEarthLines;
		g_App.ShowEarthLines(m_bEarthLines);
		break;

	case 'y':
		// Example code: modify the terrain by using the (slow) approach of using
		//  vtTerrain methods GetInitialGrid and UpdateElevation.
		{
			vtTerrain *pTerr = GetCurrentTerrain();
			if (pTerr && pTerr->GetParams().GetValueBool(STR_ALLOW_GRID_SCULPTING))
			{
				vtElevationGrid	*grid = pTerr->GetInitialGrid();
				if (grid)
				{
					clock_t t1 = clock();
					// Raise an area of the terrain
					int cols, rows;
					grid->GetDimensions(cols, rows);
					for (int i  = cols / 4; i < cols / 2; i++)
						for (int j = rows / 4; j < rows / 2; j++)
						{
							grid->SetFValue(i, j, grid->GetFValue(i, j) + 40);
						}
					pTerr->UpdateElevation();
					clock_t t2 = clock();
					VTLOG(" Modify1: %.3f sec\n", (float)(t2-t1)/CLOCKS_PER_SEC);

					// Update the shading and culture
					pTerr->RecreateTextures(vtGetTS()->GetSunLightTransform());
					DRECT area;
					area.Empty();
					pTerr->RedrapeCulture(area);
				}
			}
		}
		break;
	case 'Y':
		// Example code: modify the terrain by using the (fast) approach of using
		//  vtDynTerrainGeom::SetElevation.
		{
			vtTerrain *pTerr = GetCurrentTerrain();
			if (pTerr)
			{
				vtDynTerrainGeom *dyn = pTerr->GetDynTerrain();
				if (dyn)
				{
					clock_t t1 = clock();
					// Raise an area of the terrain
					int cols, rows;
					dyn->GetDimensions(cols, rows);
					for (int i  = cols / 4; i < cols / 2; i++)
						for (int j = rows / 4; j < rows / 2; j++)
						{
							dyn->SetElevation(i, j, dyn->GetElevation(i, j) + 40);
						}
					clock_t t2 = clock();
					VTLOG(" Modify2: %.3f sec\n", (float)(t2-t1)/CLOCKS_PER_SEC);

					// Update the shading and culture
					pTerr->RecreateTextures(vtGetTS()->GetSunLightTransform());
					DRECT area;
					area.Empty();
					pTerr->RedrapeCulture(area);
				}
			}
		}
		break;
	case 'u':
		// Example code: modify a small area of terrain around the mouse pointer.
		{
			vtTerrain *pTerr = GetCurrentTerrain();
			if (pTerr)
			{
				vtDynTerrainGeom *dyn = pTerr->GetDynTerrain();
				if (dyn)
				{
					// Get 3D cursor location in grid coordinates
					FPoint3 fpos;
					g_App.m_pTerrainPicker->GetCurrentPoint(fpos);
					IPoint2 ipos;
					dyn->WorldToGrid(fpos, ipos);
					for (int x  = -4; x < 4; x++)
						for (int y = -4; y < 4; y++)
						{
							float val = dyn->GetElevation(ipos.x + x, ipos.y + y);
							dyn->SetElevation(ipos.x + x, ipos.y + y, val + 40);
						}

					// Update the (entire) shading and culture
					pTerr->RecreateTextures(vtGetTS()->GetSunLightTransform());
					DRECT area;
					area.Empty();
					pTerr->RedrapeCulture(area);
				}
			}
		}
		break;
	case 'D':	// Shift-D
		// dump camera info
		g_App.DumpCameraInfo();
		break;

	case 2:	// Ctrl-B
		// toggle demo
		g_App.ToggleDemo();
		break;

	case WXK_F11:
		DoTestCode();
		break;

	case WXK_F12:
		m_pSceneGraphDlg->Show(true);
		break;

	case WXK_DELETE:
		DeleteAllSelected();
		break;

	default:
		event.Skip();
		break;
	}
}

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/ShadowTexture>
#include <osgDB/ReadFile>

void EnviroFrame::DoTestCode()
{
	SetMode(MM_SLOPE);

#if 0
	// Shadow tests
	const int ReceivesShadowTraversalMask = 0x1;
	const int CastsShadowTraversalMask = 0x2;

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;

	shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
	shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);

#if 0
	osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
	shadowedScene->setShadowTechnique(sm.get());
	int mapres = 1024;
	sm->setTextureSize(osg::Vec2s(mapres,mapres));
#else
	osg::ref_ptr<osgShadow::ShadowTexture> sm = new osgShadow::ShadowTexture;
	shadowedScene->setShadowTechnique(sm.get());
#endif

	osg::Group* cessna1 = (osg::Group*) osgDB::readNodeFile("cessna.osg");
	if (!cessna1)
		return;
	cessna1->setNodeMask(CastsShadowTraversalMask);
	cessna1->getChild(0)->setNodeMask(CastsShadowTraversalMask);

	osg::Group* cessna2 = (osg::Group*) osgDB::readNodeFile("cessna.osg");
	if (!cessna2)
		return;
	int flags_off = ~(CastsShadowTraversalMask | ReceivesShadowTraversalMask);
	cessna2->setNodeMask(flags_off);
	cessna2->getChild(0)->setNodeMask(flags_off);

	osg::MatrixTransform* positioned = new osg::MatrixTransform;
	positioned->setDataVariance(osg::Object::STATIC);
	positioned->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,1.0f,0.0f)
		*osg::Matrix::translate(40,40,0));
	positioned->addChild(cessna1);

//osg::ref_ptr<osg::Group> shadowedScene = new osg::Group;
	shadowedScene->addChild(positioned);
	shadowedScene->addChild(cessna2);

	//	osg::ref_ptr<osg::Group> container = new osg::Group;
	//	container->addChild(positioned);
	//    container->addChild(cessna2);

	vtGroup *vtg = GetCurrentTerrain()->GetTerrainGroup();
	vtg->GetOsgGroup()->addChild(shadowedScene.get());
	//	vtg->GetOsgGroup()->addChild(container.get());

	vtLogGraph(shadowedScene.get());
#endif
#if 0
		if (pTerr && g_App.m_bSelectedStruct)
		{
			vtStructureArray3d *sa = pTerr->GetStructureLayer();
			int i = 0;
			while (!sa->GetAt(i)->IsSelected())
				i++;
			vtBuilding3d *bld = sa->GetBuilding(i);
			// (Do something to the building as a test)
			sa->ConstructStructure(bld);
		}
#endif
#if 0
		{
			// Read points from a text file, create OBJ file with geometry at that locations
			FILE *fp = fopen("test.txt", "r");
			if (!fp) return;

			char buf[80];
			float depth, x, y;

			// Add the geometry and materials to the shape
			vtGeode *pGeode = new vtGeode;
			vtMaterialArray *pMats = new vtMaterialArray;
			pMats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 1.0f), false, false, false);
			pGeode->SetMaterials(pMats);

			vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLES, VT_Normals | VT_Colors, 4000);

			int line = 0;
			fgets(buf, 80, fp);	// skip first
			while (fgets(buf, 80, fp) != NULL)
			{
				sscanf(buf, "%f\t%f\t%f", &depth, &x, &y);
				int idx = mesh->GetNumVertices();
				for (int i = 0; i < 20; i++)
				{
					double angle = (double)i / 20.0 * PI2d;
					FPoint3 vec;
					vec.x = x/2 * cos(angle);
					vec.y = 0.0f;
					vec.z = y/2 * sin(angle);

					// normal
					FPoint3 norm = vec;
					norm.Normalize();

					// color
					RGBAf col(1.0f, 1.0f, 1.0f, 1.0f);
					if (x > y)
					{
						float frac = (x-y)/1.5f;	// typical: 0 - 1.2
						col.g -= frac;
						col.b -= frac;
					}
					else if (y > x)
					{
						float frac = (y-x)/1.5f;	// typical: 0 - 1.2
						col.r -= frac;
						col.g -= frac;
					}

					int add = mesh->AddVertexN(vec.x, /*650*/-depth, vec.z,
						norm.x, norm.y, norm.z);
					mesh->SetVtxColor(add, col);
				}
				if (line != 0)
				{
					for (int i = 0; i < 20; i++)
					{
						int next = (i+1)%20;
						mesh->AddTri(idx-20 + i, idx + i,    idx-20 + next);
						mesh->AddTri(idx    + i, idx + next, idx-20 + next);
					}
				}
				line++;
			}
			pGeode->AddMesh(mesh, 0);
			WriteGeomToOBJ(pGeode, "bore.obj");

			vtTransform *model = new vtTransform;
			model->addChild(pGeode);
			DPoint3 pos;
			g_App.m_pTerrainPicker->GetCurrentEarthPos(pos);
			GetCurrentTerrain()->AddNode(model);
			GetCurrentTerrain()->PlantModelAtPoint(model, DPoint2(pos.x, pos.y));
		}
#endif
}

void EnviroFrame::LoadClouds(const char *fname)
{
	osg::ref_ptr<osg::Image> input = osgDB::readImageFile(fname);
	if (input.valid())
	{
		int depth = input->getPixelSizeInBits();
		if (depth != 8)
			DisplayAndLog("That isn't an 8-bit cloud image.");
		else
		{
			// For transparency, convert the 8-bit (from black to white) to a
			//  32-bit (RGB is white, Alpha is 0-255)
			uint w = input->s();
			uint h = input->t();

			vtImagePtr img2 = new vtImage;
			img2->Create(w, h, 32);
			RGBAi rgba(255,255,255,0);
			for (uint i = 0; i < w; i++)
				for (uint j = 0; j < h; j++)
				{
					rgba.a = GetPixel8(input, i, j);
					img2->SetPixel32(i, j, rgba);
				}

			OpenProgressDialog(_T("Processing Images"), false, this);
			g_App.MakeOverlayGlobe(img2, progress_callback);
			CloseProgressDialog();
		}
	}
	else
		DisplayAndLog("Couldn't read input file.");
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

	// Also set spacenavigator speed.  Scaling from mouse to spacenav is
	//  approximate, based on the magnitude and number of INPUT events the
	//  spacenav appears to send, 100x seems to be rough correlation.
	m_canvas->SetSpaceNavSpeed(speed * factor / 100);
}

void EnviroFrame::SetTerrainDetail(int iMetric)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	vtDynTerrainGeom *pDyn = pTerr->GetDynTerrain();
	if (pDyn)
		return pDyn->SetPolygonTarget(iMetric);

	vtTiledGeom *pTiled = pTerr->GetTiledGeom();
	if (pTiled)
		return pTiled->SetVertexTarget(iMetric);
}

int EnviroFrame::GetTerrainDetail()
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return 0;

	vtDynTerrainGeom *pDyn = pTerr->GetDynTerrain();
	if (pDyn)
		return pDyn->GetPolygonTarget();

	vtTiledGeom *pTiled = pTerr->GetTiledGeom();
	if (pTiled)
		return pTiled->GetVertexTarget();

	return 0;
}

void EnviroFrame::ChangePagingRange(float prange)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr) return;
	vtTiledGeom *pTiled = pTerr->GetTiledGeom();
	if (pTiled)
		pTiled->SetPagingRange(prange);
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

void EnviroFrame::CreateInstance(const DPoint2 &pos, vtTagArray *tags)
{
	// Just pass it along to the app.
	g_App.CreateInstanceAt(pos, tags);
}


/////////////////////////////////////////////////////////////////////////////
//
// Handle the menu commands
//

void EnviroFrame::OnExit(wxCommandEvent& event)
{
	VTLOG("Got Exit event, shutting down.\n");
	DeleteCanvas();
	Destroy();
}

void EnviroFrame::OnClose(wxCloseEvent &event)
{
	VTLOG("Got Close event, shutting down.\n");
	DeleteCanvas();
	event.Skip();
}

void EnviroFrame::OnIdle(wxIdleEvent& event)
{
	// Check if we were requested to close on the next Idle event.
	if (m_bCloseOnIdle)
		Close();
	else
		event.Skip();

	// Test: exit after first terrain loaded
//	if (g_App.m_state == AS_Terrain)
//		Close();
}

#ifdef __WXMSW__
// Catch special events, or calls an appropriate default window procedure
WXLRESULT EnviroFrame::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	if (nMsg == WM_ENTERMENULOOP)
	{
		//VTLOG1("WM_ENTERMENULOOP\n");
		EnableContinuousRendering(false);
	}
	else if (nMsg == WM_EXITMENULOOP)
	{
		//VTLOG1("WM_EXITMENULOOP\n");
		EnableContinuousRendering(true);
	}
	else if (nMsg == WM_ENTERSIZEMOVE)
	{
		//VTLOG1("WM_ENTERSIZEMOVE\n");
		EnableContinuousRendering(false);
	}
	else if (nMsg == WM_EXITSIZEMOVE)
	{
		//VTLOG1("WM_EXITSIZEMOVE\n");
		EnableContinuousRendering(true);
	}
	return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
}
#endif


//////////////////// File menu //////////////////////////

void EnviroFrame::OnFileLayers(wxCommandEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pLayerDlg);
	info.Show(!info.IsShown());
	m_mgr.Update();
}

void EnviroFrame::OnLayerCreate(wxCommandEvent& event)
{
	m_pLayerDlg->OnLayerCreate(event);
}

void EnviroFrame::OnUpdateLayerCreate(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnLayerLoad(wxCommandEvent& event)
{
	m_pLayerDlg->OnLayerLoad(event);
}

void EnviroFrame::OnUpdateLayerLoad(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
}

void EnviroFrame::OnSaveTerrain(wxCommandEvent& event)
{
	// Copy as much state as possible from the active terrain to its parameters
	g_App.StoreTerrainParameters();

	vtTerrain *terr = GetCurrentTerrain();
	TParams &par = terr->GetParams();
	vtString fname = terr->GetParamFile();
	if (!par.WriteToXML(fname, STR_TPARAMS_FORMAT_NAME))
		DisplayAndLog("Couldn't write file '%s'", (const char *)fname);
}

void EnviroFrame::OnSaveTerrainAs(wxCommandEvent& event)
{
	// Copy as much state as possible from the active terrain to its parameters
	g_App.StoreTerrainParameters();

	vtTerrain *terr = GetCurrentTerrain();
	TParams &par = terr->GetParams();
	vtString pfile = terr->GetParamFile();

	wxString defDir(ExtractPath(pfile, false), wxConvUTF8);
	wxString defFile(StartOfFilename(pfile), wxConvUTF8);

	EnableContinuousRendering(false);
	wxFileDialog saveFile(NULL, _("Save Terrain State"), defDir, defFile,
		_("Terrain Files (*.xml)|*.xml"), wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
		return;

	// Give the user an opportunity to name the new terrain
	vtString tname = terr->GetName();
	wxString def(tname, wxConvUTF8);
	wxString name = wxGetTextFromUser(_("Name for new terrain:"), _("Query"), def, this);
	if (name == _T(""))
		return;

	vtString fname = (const char*)saveFile.GetPath().mb_str(wxConvUTF8);
	terr->SetParamFile(fname);
	terr->SetName((const char *) name.mb_str(wxConvUTF8));
	if (!par.WriteToXML(fname, STR_TPARAMS_FORMAT_NAME))
		DisplayAndLog("Couldn't write file '%s'", (const char *)fname);

	// Update title
	SetTitle(wxGetApp().MakeFrameTitle(terr));
}

void EnviroFrame::OnUpdateSaveTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
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

void EnviroFrame::OnViewStats(wxCommandEvent& event)
{
#ifdef USE_OSG_STATS
	// Yes, this is a hack, but it doesn't seem that StatsHandler can be cycled
	//  any other way than by key event.
	GraphicsWindowWX* pGW = (GraphicsWindowWX*)vtGetScene()->GetGraphicsContext();
	if ((NULL != pGW) && pGW->valid())
		pGW->getEventQueue()->keyPress('x');
#endif
}

void EnviroFrame::OnViewElevLegend(wxCommandEvent& event)
{
	g_App.ShowElevationLegend(!g_App.GetShowElevationLegend());
}

void EnviroFrame::OnUpdateViewElevLegend(wxUpdateUIEvent& event)
{
	// enable only for derived-color textured terrain
	bool bEnable = false;
	vtTerrain *curr = GetCurrentTerrain();
	if (curr)
		bEnable = (curr->GetParams().GetValueInt(STR_TEXTURE) == 3);
	event.Enable(g_App.m_state == AS_Terrain && bEnable);
	event.Check(g_App.GetShowElevationLegend());
}

void EnviroFrame::OnViewCompass(wxCommandEvent& event)
{
	g_App.ShowCompass(!g_App.GetShowCompass());
}

void EnviroFrame::OnUpdateViewCompass(wxUpdateUIEvent& event)
{
	// enable only for terrain view
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.GetShowCompass());
}

void EnviroFrame::OnViewMapOverView(wxCommandEvent& event)
{
	g_App.ShowMapOverview(!g_App.GetShowMapOverview());
}

void EnviroFrame::OnUpdateViewMapOverView(wxUpdateUIEvent& event)
{
	// Only supported in Terrain View for certain texture types
	bool bEnable = false;
	vtTerrain *curr = GetCurrentTerrain();
	if (curr)
	{
		TextureEnum eTex = curr->GetParams().GetTextureEnum();
		bEnable = (eTex == TE_SINGLE || eTex == TE_DERIVED);
	}
	event.Enable(bEnable);
	event.Check(g_App.GetShowMapOverview());
}

void EnviroFrame::OnViewDrive(wxCommandEvent& event)
{
	m_pDriveDlg->Show(!m_pDriveDlg->IsShown());
}

void EnviroFrame::OnUpdateViewDrive(wxUpdateUIEvent& event)
{
	// Only supported in Terrain View for certain texture types
	bool bEnable = (g_App.m_Vehicles.GetSelectedCarEngine() != NULL);
	event.Enable(bEnable);
	event.Check(m_pDriveDlg->IsShown());
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

void EnviroFrame::OnViewReset(wxCommandEvent& event)
{
	g_App.ResetCamera();
}

void EnviroFrame::OnUpdateViewLocations(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::Snapshot(bool bNumbered)
{
	VTLOG1("EnviroFrame::Snapshot\n");

	wxString use_name;
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
			filter, wxFD_SAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return;
		}
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
		// Append the number of the snapshot to the filename
		wxString start, number, extension;
		start = m_strSnapshotFilename.BeforeLast(_T('.'));
		extension = m_strSnapshotFilename.AfterLast(_T('.'));
		number.Printf(_T("_%03d."), m_iSnapshotNumber);
		m_iSnapshotNumber++;
		use_name = start + number + extension;
	}

	std::string Filename(use_name.mb_str(wxConvUTF8));
	CScreenCaptureHandler::SetupScreenCapture(Filename);
}

void EnviroFrame::OnViewSnapshot(wxCommandEvent& event)
{
	Snapshot(false); // do ask for explicit filename always
}

void EnviroFrame::OnViewSnapAgain(wxCommandEvent& event)
{
	Snapshot(true); // number, and don't ask for filename if we already have one
}

void EnviroFrame::OnViewSnapHigh(wxCommandEvent& event)
{
	VTLOG1("EnviroFrame::OnViewSnapHigh\n");

	vtScene *scene = vtGetScene();
	IPoint2 original_size = scene->GetWindowSize();

	SizeDlg dlg(this, -1, _("High-resolution Snapshot"));
	dlg.SetBase(original_size);
	dlg.SetRatioRange(1.0f, 4.0f);
	dlg.GetTextCtrl()->SetValue(_("If the size requested is greater than your graphics card supports, the size will be clamped to the card's limit"));
	if (dlg.ShowModal() != wxID_OK)
		return;

	int  aPixWidth = dlg.m_Current.x;
    int  aPixHeight = dlg.m_Current.y;
	VTLOG("\tSize: %d %d\n", aPixWidth, aPixHeight);

	// save current directory
	wxString path = wxGetCwd();

	wxString filter = _T("JPEG Files (*.jpg)|*.jpg");
	EnableContinuousRendering(false);
	wxFileDialog saveFile(NULL, _("Save View Snapshot"), _T(""), _T(""),
		filter, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
	{
		wxSetWorkingDirectory(path);	// restore
		return;
	}

	std::string Filename(saveFile.GetPath().mb_str(wxConvUTF8));
	if (!CSaveImageOSG::SaveImage(Filename, aPixWidth, aPixHeight))
		wxMessageBox(_T("Sorry - could not save image. Yuor graphics card software may need updating"),_T("ERROR"));
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

void EnviroFrame::OnViewProfile(wxCommandEvent& event)
{
	if (m_pProfileDlg && m_pProfileDlg->IsShown())
		m_pProfileDlg->Hide();
	else
	{
		// this might be the first time it's displayed, so we need to get
		//  the point values from the distance tool
		ProfileDlg *dlg = ShowProfileDlg();
		if (m_pDistanceDlg)
		{
			DPoint2 p1, p2;
			m_pDistanceDlg->GetPoints(p1, p2);
			dlg->SetPoints(p1, p2);
		}
	}
}

void EnviroFrame::OnUpdateViewProfile(wxUpdateUIEvent& event)
{
	event.Check(m_pProfileDlg && m_pProfileDlg->IsShown());
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnViewToolCulture(wxCommandEvent& event)
{
	g_Options.m_bShowToolsCulture = !g_Options.m_bShowToolsCulture;
	RefreshToolbar();
}
void EnviroFrame::OnUpdateViewToolCulture(wxUpdateUIEvent& event)
{
	event.Check(g_Options.m_bShowToolsCulture);
}
void EnviroFrame::OnViewToolSnapshot(wxCommandEvent& event)
{
	g_Options.m_bShowToolsSnapshot = !g_Options.m_bShowToolsSnapshot;
	RefreshToolbar();
}
void EnviroFrame::OnUpdateViewToolSnapshot(wxUpdateUIEvent& event)
{
	event.Check(g_Options.m_bShowToolsSnapshot);
}
void EnviroFrame::OnViewToolTime(wxCommandEvent& event)
{
	g_Options.m_bShowToolsTime = !g_Options.m_bShowToolsTime;
	RefreshToolbar();
}
void EnviroFrame::OnUpdateViewToolTime(wxUpdateUIEvent& event)
{
	event.Check(g_Options.m_bShowToolsTime);
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

void EnviroFrame::OnToolsSelectBox(wxCommandEvent& event)
{
	SetMode(MM_SELECTBOX);
}

void EnviroFrame::OnUpdateToolsSelectBox(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_SELECTBOX);
}

void EnviroFrame::OnToolsSelectMove(wxCommandEvent& event)
{
	SetMode(MM_SELECTMOVE);
}

void EnviroFrame::OnUpdateToolsSelectMove(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_SELECTMOVE);
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

void EnviroFrame::OnToolsBuildings(wxCommandEvent& event)
{
	SetMode(MM_BUILDINGS);
}

void EnviroFrame::OnUpdateToolsBuildings(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_BUILDINGS);
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

void EnviroFrame::OnToolsPoints(wxCommandEvent& event)
{
	vtAbstractLayer *alay = g_App.GetLabelLayer();
	if (!alay)
		alay = CreateNewAbstractPointLayer(GetCurrentTerrain());
	if (!alay)
		return;
	SetMode(MM_ADDPOINTS);
}

void EnviroFrame::OnUpdateToolsPoints(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_ADDPOINTS);
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

void EnviroFrame::OnToolsVehicles(wxCommandEvent& event)
{
	SetMode(MM_VEHICLES);
}

void EnviroFrame::OnUpdateToolsVehicles(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_VEHICLES);
}

void EnviroFrame::OnToolsMove(wxCommandEvent& event)
{
	SetMode(MM_MOVE);
}

void EnviroFrame::OnUpdateToolsMove(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
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

void EnviroFrame::OnToolsConstrain(wxCommandEvent& event)
{
	g_App.m_bConstrainAngles = !g_App.m_bConstrainAngles;
}

void EnviroFrame::OnUpdateToolsConstrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_bConstrainAngles);
}

// Visual impact submenu
void EnviroFrame::OnVIACalculate(wxCommandEvent& event)
{
#if VISUAL_IMPACT_CALCULATOR
	wxString Message = _("Your 3d driver does not support off screen rendering.\n");
	Message += _("If the main 3d window is obscured by any other windows\n");
	Message += _("(including popup dialogs). The accuracy of the visual impact\n");
	Message += _("calculation will be impaired.");

	if (vtGetScene()->GetVisualImpactCalculator().UsingLiveFrameBuffer())
		wxMessageBox(Message);
	m_pVIADlg->Show(true);
#else
	wxMessageBox(_("Not available."));
#endif
}

void EnviroFrame::OnUpdateVIACalculate(wxUpdateUIEvent& event)
{
	bool bFound = false;
	vtTerrain *pTerr = GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet& Layers = pTerr->GetLayers();

		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->GetSize();
				for (int j = 0; (j < iNumberOfStructures) && !bFound; j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					if (pStructure3d->GetVIAContributor())
						bFound = true;
				}
			}
		}
	}
	event.Enable(bFound);
}

void EnviroFrame::OnVIAPlot(wxCommandEvent& event)
{
#if VISUAL_IMPACT_CALCULATOR
	wxFileDialog RasterFileDialog(this,
								_T("Output raster file"),
								_T(""), _T("viaplot.tif"),
								_T(""), wxFD_SAVE);
	CVIAGDALOptionsDlg OptionsDlg(this);
	GDALDriverManager *pDriverManager;
	int iNumDrivers;
	int i;
	bool bFirst = true;
	wxString Filter;
	int *pDriverIndices = NULL;
	int iDriverIndexOffset = 0;
	int iDriverIndex;
	int iDefaultFilter = 0;

	g_GDALWrapper.RequestGDALFormats();

	pDriverManager = GetGDALDriverManager();
	iNumDrivers = pDriverManager->GetDriverCount();
	if (NULL == (pDriverIndices = new int[iNumDrivers]))
		return;
	for (i = 0; i < iNumDrivers; i++)
	{
		GDALDriver* pDriver = pDriverManager->GetDriver(i);
		char ** ppMetaData = pDriver->GetMetadata();
		const char *pExtension;
		const char *pLongname;

		if (CSLFetchBoolean(ppMetaData, GDAL_DCAP_CREATE, FALSE)
			&& (NULL != (pLongname = CSLFetchNameValue(ppMetaData, GDAL_DMD_LONGNAME))))
		{
			pExtension = CSLFetchNameValue(ppMetaData, GDAL_DMD_EXTENSION);
			if (bFirst)
				bFirst = false;
			else
				Filter += wxT("|");
			Filter += wxString(pLongname, wxConvUTF8);
			Filter += wxT("|*.");
			Filter += wxString(pExtension, wxConvUTF8);
			pDriverIndices[iDriverIndexOffset] = i;
#ifdef WIN32
			if (0 == strnicmp(pLongname, "GeoTIFF", 7))
#else
			if (0 == strncasecmp(pLongname, "GeoTIFF", 7))
#endif
				iDefaultFilter = i;
			iDriverIndexOffset++;
		}
	}

	RasterFileDialog.SetWildcard(Filter);
	RasterFileDialog.SetFilterIndex(iDefaultFilter);

	if (wxID_OK != RasterFileDialog.ShowModal())
	{
		delete pDriverIndices;
		return;
	}

	iDriverIndex = pDriverIndices[RasterFileDialog.GetFilterIndex()];
	delete pDriverIndices;

	OptionsDlg.Setup(iDriverIndex);

	if (wxID_OK != OptionsDlg.ShowModal())
		return;

	GDALDriver *pDriver = GetGDALDriverManager()->GetDriver(iDriverIndex);
	if (NULL == pDriver)
		return;

	int iNumArgs;
	int iNumChars;
	char **ppArgv = NULL;
	char *pArgs = NULL;
	int iType;

	float fXSampleInterval, fYSampleInterval, fScaleFactor;
	double Temp;
	OptionsDlg.m_XSampleInterval.ToDouble(&Temp);
	fXSampleInterval = Temp;
	OptionsDlg.m_YSampleInterval.ToDouble(&Temp);
	fYSampleInterval = Temp;
	OptionsDlg.m_ScaleFactor.ToDouble(&Temp);
	fScaleFactor = Temp;

	ParseCommandLine((const char *)OptionsDlg.m_CreationOptions.mb_str(wxConvUTF8), NULL, NULL, &iNumArgs, &iNumChars);
	ppArgv = new char*[iNumArgs];
	pArgs = new char[iNumChars];
	ParseCommandLine((const char *)OptionsDlg.m_CreationOptions.mb_str(wxConvUTF8), ppArgv, pArgs, &iNumArgs, &iNumChars);

	for (iType = 1; iType < GDT_TypeCount; iType++)
		if (GDALGetDataTypeName((GDALDataType)iType) != NULL
				&& EQUAL(GDALGetDataTypeName((GDALDataType)iType), (const char *)OptionsDlg.m_DataType.mb_str(wxConvUTF8)))
			break;

	DRECT EarthExtents = vtGetTS()->GetCurrentTerrain()->GetHeightField()->GetEarthExtents();

	int iXSize = (int)((EarthExtents.right - EarthExtents.left)/fXSampleInterval);
	int iYSize = (int)((EarthExtents.top - EarthExtents.bottom)/fYSampleInterval);

	GDALDataset *pDataset = pDriver->Create((const char *)RasterFileDialog.GetPath().mb_str(wxConvUTF8), iXSize, iYSize, 1, (GDALDataType)iType, ppArgv);
	if (NULL != ppArgv)
		delete ppArgv;
	if (NULL != pArgs)
		delete pArgs;
	if (NULL == pDataset)
		return;

	// Set up geo stuff;
	char *pWKT;
	double Transform[6] = {0.0};
	Transform[0] = EarthExtents.left;
	Transform[1] = fXSampleInterval;
	Transform[3] = EarthExtents.top;
	Transform[5] = -fYSampleInterval;
	pDataset->SetGeoTransform(Transform);
	GetCurrentTerrain()->GetProjection().exportToWkt(&pWKT);
	pDataset->SetProjection(pWKT);
	CPLFree(pWKT);
	GDALRasterBand *pRasterBand = pDataset->GetRasterBand(1);
	if (NULL == pRasterBand)
		return;

	EnableContinuousRendering(false);
	OpenProgressDialog(_T("Plotting Visual Impact Factor"), true);
	bool bRet = vtGetScene()->GetVisualImpactCalculator().Plot(pRasterBand,
													fScaleFactor,
													fXSampleInterval,
													fYSampleInterval,
													progress_callback);
	CloseProgressDialog();
	if (bRet)
		wxMessageBox(_("Visual Impact Plot successful"));
	else
		wxMessageBox(_("Visual Impact Plot unsuccessful"));

	EnableContinuousRendering(true);
	delete pDataset; // This flushes and closes the dataset
#endif  // VISUAL_IMPACT_CALCULATOR
}

void EnviroFrame::ParseCommandLine(const char *cmdstart, char **argv, char *args, int *numargs, int *numchars)
{
	static const char NULCHAR = '\0';
	static const char SPACECHAR = ' ';
	static const char TABCHAR = '\t';
	static const char DQUOTECHAR = '\"';
	static const char SLASHCHAR = '\\';
	const char *p;
	int inquote;                    // 1 = inside quotes
	int copychar;                   // 1 = copy char to *args
	unsigned numslash;              // num of backslashes seen

	*numchars = 0;
	*numargs = 0;

	p = cmdstart;

	inquote = 0;

	/* loop on each argument */
	for(;;)
	{

		if ( *p )
		{
			while (*p == SPACECHAR || *p == TABCHAR)
				++p;
		}

		if (*p == NULCHAR)
			break;              // end of args

		// scan an argument
		if (argv)
			*argv++ = args;     // store ptr to arg
		++*numargs;

		// loop through scanning one argument
		for (;;)
		{
			copychar = 1;
			// Rules: 2N backslashes + " ==> N backslashes and begin/end quote
			// 2N+1 backslashes + " ==> N backslashes + literal "
			// N backslashes ==> N backslashes
			numslash = 0;
			while (*p == SLASHCHAR)
			{
				// count number of backslashes for use below
				++p;
				++numslash;
			}
			if (*p == DQUOTECHAR)
			{
				// if 2N backslashes before, start/end quote, otherwise
				// copy literally
				if (numslash % 2 == 0)
				{
					if (inquote)
					{
						if (p[1] == DQUOTECHAR)
							p++;    // Double quote inside quoted string
						else        // skip first quote char and copy second
							copychar = 0;
					}
					else
						copychar = 0;       // don't copy quote

					inquote = !inquote;
				}
				numslash /= 2;          // divide numslash by two
			}

			// copy slashes
			while (numslash--)
			{
				if (args)
					*args++ = SLASHCHAR;
				++*numchars;
			}

			// if at end of arg, break loop
			if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR)))
				break;

			// copy character into argument
			if (copychar)
			{
				if (args)
					*args++ = *p;
				++*numchars;
			}
			++p;
		}

		// null-terminate the argument

		if (args)
			*args++ = NULCHAR;          // terminate string
		++*numchars;
	}

	/* We put one last argument in -- a null ptr */
	if (argv)
		*argv++ = NULL;
	++*numargs;
}

void EnviroFrame::OnUpdateVIAPlot(wxUpdateUIEvent& event)
{
	bool bFoundContributor = false;
	bool bFoundTarget = false;
	vtTerrain *pTerr = GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet &Layers = pTerr->GetLayers();
		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->GetSize();
				for (int j = 0; (j < iNumberOfStructures) && !(bFoundContributor && bFoundTarget); j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					if (pStructure3d->GetVIAContributor())
						bFoundContributor = true;
					if (pStructure3d->GetVIATarget())
						bFoundTarget = true;
				}
			}
		}
	}
	event.Enable(bFoundContributor && bFoundTarget);
}

void EnviroFrame::OnVIAClear(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet &Layers = pTerr->GetLayers();
		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->GetSize();
				for (int j = 0; j < iNumberOfStructures; j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					pStructure3d->SetVIATarget(false);
					pStructure3d->SetVIAContributor(false);
				}
			}
		}
	}
}

void EnviroFrame::OnUpdateVIAClear(wxUpdateUIEvent& event)
{
	bool bFoundContributor = false;
	bool bFoundTarget = false;
	vtTerrain *pTerr = GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet &Layers = pTerr->GetLayers();
		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->GetSize();
				for (int j = 0; (j < iNumberOfStructures) && !(bFoundContributor || bFoundTarget); j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					if (pStructure3d->GetVIAContributor())
						bFoundContributor = true;
					if (pStructure3d->GetVIATarget())
						bFoundTarget = true;
				}
			}
		}
	}
	event.Enable(bFoundContributor || bFoundTarget);
}


/////////////////////// Scene menu ///////////////////////////

void EnviroFrame::OnSceneGraph(wxCommandEvent& event)
{
	m_pSceneGraphDlg->Show(true);
}

#ifdef NVIDIA_PERFORMANCE_MONITORING
void EnviroFrame::OnPerformanceMonitor(wxCommandEvent& event)
{
	m_pPerformanceMonitorDlg->Show(true);
}
#endif

void EnviroFrame::OnSceneTerrain(wxCommandEvent& event)
{
	wxString str;

	// When switching terrains, highlight the current on
	vtTerrain *pTerr = GetCurrentTerrain();

	// Or, if in Earth view, highlight a terrain that's already been created
	if (!pTerr && g_App.m_state == AS_Orbit)
	{
		for (uint i = 0; i < vtGetTS()->NumTerrains(); i++)
		{
			vtTerrain *t = vtGetTS()->GetTerrain(i);
			if (t->IsCreated())
			{
				pTerr = t;
				break;
			}
		}
	}
	// Get the name from the terrain, if we found one
	if (pTerr)
		str = wxString(pTerr->GetName(), wxConvUTF8);

	if (wxGetApp().AskForTerrainName(this, str))
		g_App.SwitchToTerrain(str.mb_str(wxConvUTF8));
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
	vtGroup *pRoot = vtGetTS()->GetTop();
#if (OPENSCENEGRAPH_MAJOR_VERSION==2 && OPENSCENEGRAPH_MINOR_VERSION>=2) || OPENSCENEGRAPH_MAJOR_VERSION>2
	osgDB::Registry::instance()->writeNode(*pRoot, std::string("scene.osg"), NULL);
#else
	osgDB::Registry::instance()->writeNode(*pRoot, "scene.osg");
#endif
}

void EnviroFrame::OnSceneEphemeris(wxCommandEvent& event)
{
	vtTerrainScene *ts = vtGetTS();
	vtTerrain *terr = GetCurrentTerrain();
	TParams &param = terr->GetParams();
	vtSkyDome *sky = ts->GetSkyDome();

	m_pEphemDlg->m_bSky = sky->GetEnabled();
	m_pEphemDlg->m_strSkyTexture = wxString(param.GetValueString(STR_SKYTEXTURE), wxConvUTF8);
	m_pEphemDlg->m_bOceanPlane = terr->GetFeatureVisible(TFT_OCEAN);
	m_pEphemDlg->m_fOceanPlaneLevel = param.GetValueFloat(STR_OCEANPLANELEVEL);
	m_pEphemDlg->m_bFog = terr->GetFog();
	m_pEphemDlg->m_fFogDistance = param.GetValueFloat(STR_FOGDISTANCE) * 1000;
	RGBi col = terr->GetBgColor();
	// shadows
	m_pEphemDlg->m_bShadows = terr->GetShadows();
	vtShadowOptions opt;
	terr->GetShadowOptions(opt);
	m_pEphemDlg->m_fDarkness = opt.fDarkness;
	m_pEphemDlg->m_bShadowsEveryFrame = opt.bShadowsEveryFrame;
	m_pEphemDlg->m_bShadowLimit = opt.bShadowLimit;
	m_pEphemDlg->m_fShadowRadius = opt.fShadowRadius;

	m_pEphemDlg->m_BgColor.Set(col.r, col.g, col.b);
	m_pEphemDlg->m_iWindDir = param.GetValueInt("WindDirection");
	m_pEphemDlg->m_fWindSpeed = param.GetValueFloat("WindSpeed");
	m_pEphemDlg->ValuesToSliders();

	m_pEphemDlg->Show();
#if 0
	sky->SetEnabled(dlg.m_bSky);
	ts->UpdateSkydomeForTerrain(terr);
	terr->SetFeatureVisible(TFT_OCEAN, dlg.m_bOceanPlane);
	terr->SetWaterLevel(dlg.m_fOceanPlaneLevel);
	terr->SetFog(dlg.m_bFog);
	terr->SetFogDistance(dlg.m_fFogDistance);
	col.Set(dlg.m_BgColor.Red(), dlg.m_BgColor.Green(), dlg.m_BgColor.Blue());
	terr->SetBgColor(col);
	vtGetScene()->SetBgColor(col);
	g_App.SetWind(dlg.m_iWindDir, dlg.m_fWindSpeed);
#endif
}

void EnviroFrame::OnUpdateSceneEphemeris(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
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
	event.Check(t && t->GetFeatureVisible(TFT_TERRAINSURFACE));
	event.Enable(t != NULL);
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
	event.Enable(GetCurrentTerrain() != NULL);
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
	event.Enable(t != NULL);
}

void EnviroFrame::OnIncrease(wxCommandEvent& event)
{
	SetTerrainDetail(GetTerrainDetail()+1000);
}

void EnviroFrame::OnDecrease(wxCommandEvent& event)
{
	SetTerrainDetail(GetTerrainDetail()-1000);
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

static bool s_bBuilt = false;

void EnviroFrame::OnToggleFoundations(wxCommandEvent& event)
{
	s_bBuilt = !s_bBuilt;

	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructureLayer();

	if (s_bBuilt)
	{
		OpenProgressDialog(_("Adding Foundations"));
		sa->AddFoundations(pTerr->GetHeightField(), progress_callback);
	}
	else
	{
		OpenProgressDialog(_("Removing Foundations"));
		sa->RemoveFoundations();
	}
	int i, size = sa->GetSize(), selected = sa->NumSelected();
	for (i = 0; i < size; i++)
	{
		progress_callback(i * 99 / size);

		vtStructure *s = sa->GetAt(i);
		if (selected > 0 && !s->IsSelected())
			continue;
		if (s->GetType() != ST_BUILDING)
			continue;
		sa->ConstructStructure(i);
	}
	CloseProgressDialog();
}

void EnviroFrame::OnUpdateFoundations(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && t->GetStructureLayer() && t->GetStructureLayer()->GetSize() > 0);
	event.Check(s_bBuilt);
}

void EnviroFrame::OnTerrainReshade(wxCommandEvent& event)
{
	VTLOG1("EnviroFrame::OnTerrainReshade\n");

	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);
	OpenProgressDialog(_("Recalculating Shading"), false, this);
	pTerr->RecreateTextures(vtGetTS()->GetSunLightTransform(), progress_callback);
	CloseProgressDialog();
	EnableContinuousRendering(true);

	// Also update the overview, if there is one.
	if (g_App.GetShowMapOverview())
		g_App.TextureHasChanged();
}

void EnviroFrame::OnTerrainChangeTexture(wxCommandEvent& event)
{
	VTLOG1("EnviroFrame::OnTerrainChangeTexture\n");

	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);

	TextureDlg dlg(this, -1, _("Change Texture"));
	dlg.SetParams(pTerr->GetParams());
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetParams(pTerr->GetParams());

		OpenProgressDialog(_("Changing Texture"), false, this);
		pTerr->RecreateTextures(vtGetTS()->GetSunLightTransform(), progress_callback);
		CloseProgressDialog();

		// Also update the overview, if there is one.
		if (g_App.GetShowMapOverview())
			g_App.TextureHasChanged();
	}

	EnableContinuousRendering(true);
}

void EnviroFrame::OnUpdateIsTerrainView(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnTerrainDistribVehicles(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	if (pTerr->GetRoadMap() == NULL)
	{
		wxMessageBox(_("There are no roads to put the vehicles on.\n"));
		return;
	}

	int numv = 0;
	vtContentManager3d &con = vtGetContent();
	for (uint i = 0; i < con.NumItems(); i++)
	{
		vtItem *item = con.GetItem(i);
		const char *type = item->GetValueString("type");
		int wheels = item->GetValueInt("num_wheels");
		if (type && vtString(type) == "ground vehicle" && wheels == 4)
			numv++;
	}
	if (numv == 0)
	{
		wxMessageBox(_("Could not find any ground vehicles in the content file.\n"));
		return;
	}

	wxString msg;
	msg.Printf(_("There are %d types of ground vehicle available."), numv);
	int num = wxGetNumberFromUser(msg, _("Vehicles:"), _("Distribute Vehicles"), 10, 1, 99);
	if (num == -1)
		return;

	g_App.CreateSomeTestVehicles(pTerr);
}

void EnviroFrame::OnTerrainWriteElevation(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);
	wxFileDialog saveFile(NULL, _("Write Elevation to BT"), _T(""), _T(""),
		_T("BT Files (*.bt)|*.bt"), wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
		return;

	EnableContinuousRendering(false);
	OpenProgressDialog(_("Write Elevation to BT"), false, this);

	// Get the properties of the dynamic terrain, make an elevation grid like it
	vtDynTerrainGeom *dtg = pTerr->GetDynTerrain();
	int iCols, iRows;
	vtProjection proj = pTerr->GetProjection();
	dtg->GetDimensions(iCols, iRows);
	DRECT area = dtg->GetEarthExtents();
	bool bFloat = true;
	vtElevationGrid grid(area, iCols, iRows, bFloat, proj);

	// Copy the data to the (temporary) elevation grid
	for (int i = 0; i < iCols; i++)
	{
		progress_callback(i * 99 / iCols);
		for (int j = 0; j < iRows; j++)
		{
			float val = dtg->GetElevation(i, j, true);
			grid.SetFValue(i, j, val);
		}
	}

	// Write to disk, the temporary is freed when it goes out of scope
	vtString fname = (const char*)saveFile.GetPath().mb_str(wxConvUTF8);
	grid.SaveToBT(fname, progress_callback);

	CloseProgressDialog();
	EnableContinuousRendering(true);
}

void EnviroFrame::OnTerrainAddContour(wxCommandEvent& event)
{
#if SUPPORT_QUIKGRID
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);
	ContourDlg dlg(this, -1, _("Add Contours"));

	dlg.LayerChoice()->Clear();
	LayerSet &layers = pTerr->GetLayers();
	for (uint i = 0; i < layers.size(); i++)
	{
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(layers[i].get());
		if (!alay)
			continue;
		vtString vname = alay->GetLayerName();
		if (alay->GetFeatureSet()->GetGeomType() == wkbLineString)
			dlg.LayerChoice()->Append(wxString(vname, wxConvUTF8));
	}

	bool bResult = (dlg.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
		return;

	vtAbstractLayer *alay;
	if (dlg.m_bCreate)
	{
		// create new (abstract polyline) layer to receive contour lines
		alay = CreateNewAbstractLineLayer(pTerr, true);
	}
	else
	{
		// get the existing layer from the dialog's choice, by name
		wxString wname = dlg.m_strLayer;
		vtLayer *lay = layers.FindByName((const char *)wname.mb_str(wxConvUTF8));
		if (!lay) return;
		alay = dynamic_cast<vtAbstractLayer*>(lay);
	}
	if (!alay) return;
	vtFeatureSetLineString *pSet = (vtFeatureSetLineString *) alay->GetFeatureSet();

	vtContourConverter cc;
	if (!cc.Setup(pTerr, pSet))
		return;

	if (dlg.m_bSingle)
		cc.GenerateContour(dlg.m_fElevSingle);
	else
		cc.GenerateContours(dlg.m_fElevEvery);
	cc.Finish();

	// show the geometry
	alay->CreateStyledFeatures();

	// and show it in the layers dialog
	m_pLayerDlg->RefreshTreeContents();	// full refresh
#endif
}

void EnviroFrame::OnUpdateIsDynTerrain(wxUpdateUIEvent& event)
{
	vtTerrain *t = GetCurrentTerrain();
	event.Enable(t && t->GetDynTerrain());
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

void EnviroFrame::OnEarthClouds(wxCommandEvent& event)
{
	wxFileDialog loadFile(NULL, _("Load"), _T(""), _T(""),
		_("JPEG Files (*.jpg)|*.jpg"), wxFD_OPEN);
	if (loadFile.ShowModal() != wxID_OK)
		return;
	vtString fname = (const char *) loadFile.GetPath().mb_str(wxConvUTF8);
	LoadClouds(fname);
}

// Helper
vtString GetTempFolderName()
{
	vtString path;

	const char *temp = getenv("TEMP");
	if (temp)
		path = temp;
	else
#if WIN32
		path = "C:/TEMP";
#else
		path = "/tmp";
#endif
	return path;
}

void EnviroFrame::OnEarthClouds2(wxCommandEvent& event)
{
#if SUPPORT_CURL
	wxString defval = _T("http://xplanet.explore-the-world.net/clouds_2048.jpg");
	wxString str = wxGetTextFromUser(_T("Enter URL of overlay image"),
		_T("URL"), defval, this);

	if (str == _T(""))
		return;
	vtString url = (const char*)str.mb_str();

	vtBytes data;
	ReqContext cl;
	cl.SetProgressCallback(progress_callback);

	OpenProgressDialog(_T("Downloading"), false, this);
	bool success = cl.GetURL(url, data);
	CloseProgressDialog();

	if (!success)
		return;

	vtString tfile = GetTempFolderName() + "/temp.jpg";
	FILE *fp = vtFileOpen(tfile, "wb");
	if (!fp)
		return;
	fwrite(data.Get(), data.Len(), 1, fp);
	fclose(fp);

	LoadClouds(tfile);
#endif
}

void EnviroFrame::OnUpdateEarthClouds(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit && g_App.GetOverlayGlobe() == NULL);
}


//////////////////// Help menu //////////////////////////

void EnviroFrame::OnHelpAbout(wxCommandEvent& event)
{
	EnableContinuousRendering(false);

	wxString str(STRING_APPORG "\n\n", wxConvUTF8);
#ifdef ENVIRO_NATIVE
	str += _("The runtime environment for the Virtual Terrain Project.\n\n");
	str += _("Please read the HTML documentation and license.\n\n");
	str += _("Send feedback to: ben@vterrain.org\n");
	str += _T("\nVersion: ");
	str += _T(VTP_VERSION);
	str += _T("\n");
	str += _("Build date: ");
	str += wxString(__DATE__, *wxConvCurrent);
#else
	str += _T("Based on the Virtual Terrain Project 3D Runtime Environment.\n");
#endif

	str += _T("\n\n");
	str += wxString("Application framework: wxWidgets v" wxVERSION_NUM_DOT_STRING "\n", wxConvUTF8);

	wxString str2 = _("About ");
	str2 += wxString(STRING_APPORG, wxConvUTF8);
	wxMessageBox(str, str2);

	EnableContinuousRendering(true);
}

void EnviroFrame::OnHelpDocLocal(wxCommandEvent &event)
{
	vtString local_lang_code = (const char *) wxGetApp().GetLanguageCode().mb_str(wxConvUTF8);
	local_lang_code = local_lang_code.Left(2);

	// Launch default web browser with documentation pages
	LaunchAppDocumentation("Enviro", local_lang_code);
}

void EnviroFrame::OnHelpDocOnline(wxCommandEvent &event)
{
	// Launch default web browser with documentation pages
	wxLaunchDefaultBrowser(_T("http://vterrain.org/Doc/Enviro/"));
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

		// If there is paging involved, Inform the LOD dialog
		vtTiledGeom *geom = pTerrain->GetTiledGeom();
		if (geom && m_pLODDlg)
			m_pLODDlg->SetPagingRange(geom->prange_min, geom->prange_max);

		bool bAllowRoll = pTerrain->GetParams().GetValueBool(STR_ALLOW_ROLL);
		m_canvas->SetSpaceNavAllowRoll(bAllowRoll);
	}
	else
	{
		vtProjection geo;
		OGRErr err = geo.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
		if (err == OGRERR_NONE)
			m_pDistanceDlg->SetProjection(geo);
	}
	// Update Title
	SetTitle(wxGetApp().MakeFrameTitle(pTerrain));
}

//
// Called when the Earth View has been constructed
//
void EnviroFrame::SetTimeEngine(vtTimeEngine *pEngine)
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

void EnviroFrame::OnDrop(const wxString &str)
{
	if (!str.Right(3).CmpNoCase(_T("kml")))
	{
		g_App.ImportModelFromKML((const char *) str.ToUTF8());
	}
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
			geom->GetPagingRange());

		m_pLODDlg->DrawTilesetState(geom, vtGetScene()->GetCamera());
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
				sr->GetPolygonTarget(), sr->GetNumDrawnTriangles(),	-1);
		}
		SMTerrain *sm = dynamic_cast<SMTerrain*>(dyn);
		if (sm)
		{
			m_pLODDlg->Refresh(-1,
				log((sm->GetQualityConstant()-0.002f)*10000)*40, -1,
				sm->GetPolygonTarget(), sm->GetNumDrawnTriangles(),	-1);
		}
	}
	vtPagedStructureLodGrid *pPSLG = terr->GetStructureLodGrid();
	if (pPSLG)
		m_pLODDlg->DrawStructureState(pPSLG, terr->GetStructurePageOutDistance());
}

//
// Show the feature table dialog for a given feature set.
//
void EnviroFrame::ShowTable(vtAbstractLayer *alay)
{
	vtFeatureSet *set = alay->GetFeatureSet();
	FeatureTableDlg3d *table = NULL;
	for (uint i = 0; i < m_FeatureDlgs.size(); i++)
	{
		if (m_FeatureDlgs[i]->GetFeatureSet() == set)
			table = m_FeatureDlgs[i];
	}
	if (!table)
	{
		table = new FeatureTableDlg3d(this, -1, _T(""), wxDefaultPosition,
			wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		table->SetFeatureSet(set);
		table->SetLayer(alay);
		m_FeatureDlgs.push_back(table);
	}
	table->Show();

	// The dialog might have been already open, but hidden behind other
	//  windows.  We want to help the user see it.
	table->Raise();
}

//
// When a feature set is deleted, be sure to also remove the corresponding
//  feature table dialog.
//
void EnviroFrame::OnSetDelete(vtFeatureSet *set)
{
	for (uint i = 0; i < m_FeatureDlgs.size(); i++)
	{
		if (m_FeatureDlgs[i]->GetFeatureSet() == set)
		{
			delete m_FeatureDlgs[i];
			m_FeatureDlgs.erase(m_FeatureDlgs.begin()+i);
			return;
		}
	}
}
void EnviroFrame::DeleteAllSelected()
{
	vtTerrain *pTerr = GetCurrentTerrain();
	int structs = pTerr->DeleteSelectedStructures();
	int plants = pTerr->DeleteSelectedPlants();
	int points = pTerr->DeleteSelectedFeatures();

	// layer dialog needs to reflect the change
	if ((plants != 0 || points != 0) && structs == 0)
		m_pLayerDlg->UpdateTreeTerrain();		// we only need to update
	else if (structs != 0)
		m_pLayerDlg->RefreshTreeContents();		// we need full refresh
}


///////////////////////////////////////////////////////////////////

class EnviroProfileCallback : public ProfileCallback
{
public:
	EnviroProfileCallback() {}
	float GetElevation(const DPoint2 &p)
	{
		vtTerrain *terr = GetCurrentTerrain();
		if (terr)
		{
			FPoint3 w;
			terr->GetHeightField()->m_Conversion.ConvertFromEarth(p, w.x, w.z);
			terr->GetHeightField()->FindAltitudeAtPoint(w, w.y, true);
			return w.y;
		}
		return INVALID_ELEVATION;
	}
	float GetCultureHeight(const DPoint2 &p)
	{
		vtTerrain *terr = GetCurrentTerrain();
		if (terr)
		{
			FPoint3 w;
			terr->GetHeightField()->m_Conversion.ConvertFromEarth(p, w.x, w.z);
			bool success = terr->FindAltitudeOnCulture(w, w.y, true, CE_STRUCTURES);
			if (success)
				return w.y;
		}
		return INVALID_ELEVATION;
	}
	virtual bool HasCulture() { return true; }
};

ProfileDlg *EnviroFrame::ShowProfileDlg()
{
	if (!m_pProfileDlg)
	{
		// Create new Feature Info Dialog
		m_pProfileDlg = new ProfileDlg(this, wxID_ANY, _("Elevation Profile"),
				wxPoint(120, 80), wxSize(730, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

		EnviroProfileCallback *callback = new EnviroProfileCallback;
		m_pProfileDlg->SetCallback(callback);

		m_pProfileDlg->SetProjection(GetCurrentTerrain()->GetProjection());
	}
	m_pProfileDlg->Show(true);
	return m_pProfileDlg;
}

void EnviroFrame::OpenFenceDialog()
{
	// inform the dialog about the materials
	m_pFenceDlg->SetStructureMaterials(&vtStructure3d::GetMaterialDescriptors());
	m_pFenceDlg->Show(true);
}

///////////////////////////////////////////////////////////////////

void EnviroFrame::ShowPopupMenu(const IPoint2 &pos)
{
	// Currently, you can pop up properties on a single structure, or
	//  a single plant.  If both are selected, the structure is used.
	VTLOG1("Creating popup, ");

	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructureLayer();
	vtPlantInstanceArray3d &plants = pTerr->GetPlantInstances();

	wxMenu *popmenu = new wxMenu;

	int structures_selected = sa ? sa->NumSelected() : 0;
	int buildings_selected = sa ? sa->NumSelectedOfType(ST_BUILDING) : 0;
	int linears_selected = sa ? sa->NumSelectedOfType(ST_LINEAR) : 0;
	int instances_selected = sa ? sa->NumSelectedOfType(ST_INSTANCE) : 0;

	wxMenuItem *item = popmenu->Append(ID_POPUP_PROPERTIES, _("Properties"));
	if (structures_selected == 0 && plants.NumSelected() == 0)
		item->Enable(false);

	// Can't display properties for more than one structure
	if (buildings_selected == 1)
		popmenu->Append(ID_POPUP_COPY_STYLE, _("Copy Style"));

	if (buildings_selected > 0)
	{
		if (g_App.HaveBuildingStyle())
			popmenu->Append(ID_POPUP_PASTE_STYLE, _("Paste Style"));
		popmenu->Append(ID_POPUP_FLIP, _("Flip Footprint Direction"));
	}

	if (instances_selected > 0)
	{
		popmenu->Append(ID_POPUP_RELOAD, _("Reload from Disk"));
		popmenu->Append(ID_POPUP_ADJUST, _("Adjust Terrain Surface to Fit"));
	}

	if (structures_selected > 0)
		popmenu->Append(ID_POPUP_SHADOW, _("Toggle Shadow"));

	if (structures_selected == 1)
	{
		// It might have a URL, also
		vtTag *tag = sa->GetFirstSelectedStructure()->FindTag("url");
		if (tag)
		{
			popmenu->AppendSeparator();
			popmenu->Append(ID_POPUP_URL, _("URL"));
		}
#ifdef VISUAL_IMPACT_CALCULATOR
		// Visual Impact Assessment
        popmenu->AppendCheckItem(ID_POPUP_VIA,
			_("&Visual Impact Contributor\tCtrl+V"),
			_("Set this structure as a contributor to the VIA calculation"));
        popmenu->AppendCheckItem(ID_POPUP_VIA_TARGET,
			_("Visual Impact &Target\tCtrl+T"),
			_("Set this structure as the viewer target for VIA plots"));
#endif
	}
	if (plants.NumSelected() != 0)
	{
		// We could add some plant-specific commands here
	}

	if (g_App.m_Vehicles.GetSelected() != -1)
	{
		popmenu->Append(ID_POPUP_START, _("Start Driving"));
	}

	popmenu->AppendSeparator();
	popmenu->Append(ID_POPUP_DELETE, _("Delete"));

	VTLOG1("show.\n");
	m_canvas->PopupMenu(popmenu, pos.x, pos.y);
	delete popmenu;
}

void EnviroFrame::OnPopupProperties(wxCommandEvent& event)
{
	// Currently, you can pop up properties on a single structure, or
	//  a single plant.  If both are selected, the structure is used.

	VTLOG1("OnPopupProperties.\n");
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructureLayer();
	if (sa)
	{
		int sel = sa->GetFirstSelected();
		if (sel != -1)
		{
			vtBuilding3d *bld = sa->GetBuilding(sel);
			if (bld)
			{
				m_pBuildingDlg->Setup(bld);
				m_pBuildingDlg->Show(true);
			}
			vtFence3d *fen = sa->GetFence(sel);
			if (fen)
			{
				// Editing of fence properties
				m_pFenceDlg->SetOptions(fen->GetParams());
				OpenFenceDialog();
			}
			vtStructInstance3d *inst = sa->GetInstance(sel);
			if (inst)
			{
				// Display of instance properties
				m_pTagDlg->Show(true);
				m_pTagDlg->SetTags(inst);
			}
			return;
		}
	}

	vtPlantInstanceArray3d &plants = pTerr->GetPlantInstances();
	if (plants.NumSelected() != 0)
	{
		int found = -1;
		uint count = plants.GetNumEntities();
		for (uint i = 0; i < count; i++)
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
			m_pPlantDlg->SetDlgPlantOptions(opt);
			m_pPlantDlg->Show(true);
		}
	}
}

void EnviroFrame::OnPopupCopyStyle(wxCommandEvent& event)
{
	g_App.CopyBuildingStyle();
}

void EnviroFrame::OnPopupPasteStyle(wxCommandEvent& event)
{
	g_App.PasteBuildingStyle();
}

void EnviroFrame::OnPopupFlip(wxCommandEvent& event)
{
	g_App.FlipBuildingFooprints();
}

void EnviroFrame::OnPopupReload(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *structures = pTerr->GetStructureLayer();

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

void EnviroFrame::OnPopupShadow(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *structures = pTerr->GetStructureLayer();

	int count = structures->GetSize();
	vtStructure *str;
	for (int i = 0; i < count; i++)
	{
		str = structures->GetAt(i);
		if (!str->IsSelected())
			continue;

		// toggle
		bool bShow = !structures->GetStructure3d(i)->GetCastShadow();
		structures->GetStructure3d(i)->SetCastShadow(bShow);

		// remember state
		if (!bShow)
			str->SetValueBool("shadow", false);
		else
			// shadows are on by default, so don't store shadow=true
			str->RemoveTag("shadow");
	}
	pTerr->ForceShadowUpdate();
}

void EnviroFrame::CarveTerrainToFitNode(osg::Node *node)
{
	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;
	vtDynTerrainGeom *dyn = terr->GetDynTerrain();
	if (!dyn)
		return;

	FSphere sph;
	GetBoundSphere(node, sph, true);

	int changed = 0;
	int cols, rows;
	dyn->GetDimensions(cols, rows);
	for (int c = 0; c < cols; c++)
	{
		for (int r = 0; r < rows; r++)
		{
			FPoint3 wpos;
			dyn->GetWorldLocation(c, r, wpos);

			if (wpos.x < (sph.center.x - sph.radius))
				continue;
			if (wpos.x > (sph.center.x + sph.radius))
				continue;
			if (wpos.z < (sph.center.z - sph.radius))
				continue;
			if (wpos.z > (sph.center.z + sph.radius))
				continue;

			FPoint3 yvec(0,100,0);

			// Shoot a ray upwards through the terrain surface point
			vtHitList HitList;
			int iNumHits = vtIntersect(node, wpos - yvec, wpos + yvec, HitList);
			if (iNumHits)
			{
				FPoint3 pos = HitList.front().point;

				dyn->SetElevation(c, r, pos.y);
				changed++;
			}
		}
	}
	if (changed != 0)
	{
		wxString msg;
		msg.Printf(_T("Adjusted %d heixels.  Re-shade the terrain?"), changed);
		int res = wxMessageBox(msg, _T(""), wxYES_NO, this);

		if (res == wxYES)
		{
			// Update the (entire) shading and culture
			EnableContinuousRendering(false);
			OpenProgressDialog(_("Recalculating Shading"), false, this);

			terr->RecreateTextures(vtGetTS()->GetSunLightTransform(), progress_callback);
			DRECT area;
			area.Empty();
			terr->RedrapeCulture(area);

			CloseProgressDialog();
			EnableContinuousRendering(true);
		}
	}
}

void EnviroFrame::OnPopupAdjust(wxCommandEvent& event)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *structures = pTerr->GetStructureLayer();

	int count = structures->GetSize();
	for (int i = 0; i < count; i++)
	{
		vtStructure *str = structures->GetAt(i);
		if (str->IsSelected())
		{
			vtStructInstance3d *inst = structures->GetInstance(i);
			if (inst)
			{
				CarveTerrainToFitNode(inst->GetContainer());
			}
		}
	}
}

void EnviroFrame::OnPopupStart(wxCommandEvent& event)
{
	g_App.m_Vehicles.SetVehicleSpeed(g_App.m_Vehicles.GetSelected(), 1.0f);
}

void EnviroFrame::OnPopupDelete(wxCommandEvent& event)
{
	DeleteAllSelected();
}

void EnviroFrame::OnPopupURL(wxCommandEvent& event)
{
	vtStructureArray3d *sa = GetCurrentTerrain()->GetStructureLayer();
	vtStructure *struc = sa->GetAt(sa->GetFirstSelected());
	wxLaunchDefaultBrowser(wxString(struc->GetValueString("url"), wxConvUTF8));
}

void EnviroFrame::OnPopupVIA(wxCommandEvent& event)
{
#if VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *pStructures = pTerr->GetStructureLayer();
	vtStructure3d *pStructure3d;

	if (pStructures->NumSelected() != 1)
		return;

	pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());
	if (NULL == pStructure3d)
        return;
	if (pStructure3d->GetVIAContributor())
	{
		pStructure3d->SetVIAContributor(false);
		vtGetScene()->GetVisualImpactCalculator().RemoveVisualImpactContributor(pStructure3d->GetContainer());
	}
	else
	{
		pStructure3d->SetVIAContributor(true);
		vtGetScene()->GetVisualImpactCalculator().AddVisualImpactContributor(pStructure3d->GetContainer());
	}
#endif
}

void EnviroFrame::OnUpdatePopupVIA(wxUpdateUIEvent& event)
{
#if VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = GetCurrentTerrain();

	if (NULL == pTerr)
		event.Enable(false);
	else
	{
		vtStructureArray3d *pStructures = pTerr->GetStructureLayer();

		if (NULL == pStructures)
			event.Enable(false);
		else
		{
            vtStructure3d *pStructure3d;

			if (pStructures->NumSelected() != 1)
				event.Enable(false);
			else
			{
                pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());
				if ((NULL != pStructure3d) && pStructure3d->GetVIAContributor())
					event.Check(true);
				else
					event.Check(false);
/* Need to work out why this is in my original code
				if ((NULL == dynamic_cast<vtStructure3d*>(pStr)) || (NULL == dynamic_cast<vtStructure3d*>(pStr)->GetContainer()))
					event.Enable(false);
				else
					event.Enable(true);
*/
			}
		}
	}
#endif
}

void EnviroFrame::OnPopupVIATarget(wxCommandEvent& event)
{
#if VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *pStructures = pTerr->GetStructureLayer();
	int count = pStructures->GetSize();
	vtStructure3d *pStructure3d;
	vtTransform *pTransform;
	FSphere sphere;

	if (pStructures->NumSelected() != 1)
		return;

	pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());

	if (NULL == pStructure3d)
        return;

	if (pStructure3d->GetVIATarget())
	{
		pStructure3d->SetVIATarget(false);
		return;
	}

	for (int i = 0; i < count; i++)
	{
        vtStructure3d *pStructure3d = pStructures->GetStructure3d(i);
        if (NULL != pStructure3d)
            pStructure3d->SetVIATarget(false);
	}

	pTransform = pStructure3d->GetContainer();
	if (pTransform)
	{
		// Get the model centre
		pTransform->GetBoundSphere(sphere);
		vtGetScene()->GetVisualImpactCalculator().SetVisualImpactTarget(sphere.center + pTransform->GetTrans());
		pStructure3d->SetVIATarget(true);
	}
#endif
}

void EnviroFrame::OnUpdatePopupVIATarget(wxUpdateUIEvent& event)
{
#if VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = GetCurrentTerrain();

	if (NULL == pTerr)
		event.Enable(false);
	else
	{
		vtStructureArray3d *pStructures = pTerr->GetStructureLayer();

		if (NULL == pStructures)
			event.Enable(false);
		else
		{
			vtStructure3d *pStructure3d;

			if (pStructures->NumSelected() != 1)
				event.Enable(false);
			else
			{
				pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());
				if (NULL == pStructure3d)
					event.Enable(false);
				else
				{
					if (pStructure3d->GetVIATarget())
						event.Check(true);
					else
						event.Check(false);
/* Need to work out why this is in my original code
					if ((NULL == dynamic_cast<vtStructure3d*>(pStr)) || (NULL == dynamic_cast<vtStructure3d*>(pStr)->GetContainer()))
						event.Enable(false);
					else
						event.Enable(true);
*/
				}
			}
		}
	}
#endif
}


#if wxUSE_DRAG_AND_DROP
///////////////////////////////////////////////////////////////////////
// Drag-and-drop functionality
//
bool DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
	size_t nFiles = filenames.GetCount();
	EnviroFrame *frame = GetFrame();
	for ( size_t n = 0; n < nFiles; n++ )
	{
		wxString str = filenames[n];
		frame->OnDrop(str);
	}
	return TRUE;
}
#endif

