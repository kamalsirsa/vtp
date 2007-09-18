//
// The main Frame window of the VTBuilder application
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "xmlhelper/exception.hpp"
#include <fstream>
#include <float.h>	// for FLT_MIN

#include "Frame.h"
#include "TreeView.h"
#include "MenuEnum.h"
#include "App.h"
#include "Helper.h"
#include "BuilderView.h"
#include "VegGenOptions.h"
#include "vtImage.h"
#include "Options.h"

#include "vtui/Helper.h"
#include "vtui/ProfileDlg.h"

// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
#include "VegLayer.h"
// Dialogs
#include "ResampleDlg.h"
#include "SampleImageDlg.h"
#include "FeatInfoDlg.h"
#include "DistanceDlg2d.h"
#include "vtui/InstanceDlg.h"
#include "vtui/LinearStructDlg.h"
#include "vtui/ProjectionDlg.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "bld_add_points.xpm"
#  include "bld_delete_points.xpm"
#  include "bld_edit.xpm"
#  include "bld_corner.xpm"
#  include "distance.xpm"
#  include "edit_crossing.xpm"
#  include "edit_delete.xpm"
#  include "edit_offset.xpm"
#  include "elev_box.xpm"
#  include "elev_resample.xpm"

#  include "image_resample.xpm"
#  include "info.xpm"
#  include "instances.xpm"

#  include "layer_export.xpm"
#  include "layer_import.xpm"
#  include "layer_new.xpm"
#  include "layer_open.xpm"
#  include "layer_save.xpm"
#  include "layer_show.xpm"
#  include "layer_up.xpm"

#  include "proj_new.xpm"
#  include "proj_open.xpm"
#  include "proj_save.xpm"

#  include "rd_direction.xpm"
#  include "rd_edit.xpm"
#  include "rd_select_node.xpm"
#  include "rd_select_road.xpm"
#  include "rd_select_whole.xpm"
#  include "rd_shownodes.xpm"

#  include "select.xpm"
#  include "str_add_linear.xpm"
#  include "str_edit_linear.xpm"
#  include "raw_add_point.xpm"
#  include "tin_trim.xpm"

#  include "table.xpm"
#  include "twr_edit.xpm"

#  include "view_hand.xpm"
#  include "view_mag.xpm"
#  include "view_minus.xpm"
#  include "view_plus.xpm"
#  include "view_zoomall.xpm"
#  include "view_zoomexact.xpm"
#  include "view_zoom_layer.xpm"
#  include "view_profile.xpm"
#  include "view_options.xpm"

#	include "VTBuilder.xpm"
#endif

// Singletons
DECLARE_APP(BuilderApp)
vtTagArray g_Options;

////////////////////////////////////////////////////////////////

#if 0
class vtScaleBar : public wxWindow
{
public:
	vtScaleBar(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);

	void OnPaint(wxPaintEvent& event);

protected:
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(vtScaleBar, wxWindow)
EVT_PAINT(OnPaint)
END_EVENT_TABLE()

vtScaleBar::vtScaleBar(wxWindow* parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name) :
  wxWindow(parent, id, pos, size, style, name)
{
	VTLOG1("vtScaleBar constructor\n");
}

void vtScaleBar::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	wxCoord w, h;
	GetClientSize(&w,&h);
	dc.DrawRoundedRectangle(0, 0, w, h, 5);
}
#endif

//////////////////////////////////////////////////////////////////

MainFrame *GetMainFrame()
{
	return (MainFrame *) wxGetApp().GetTopWindow();
}

//////////////////////////////////////////////////////////////////
// Frame constructor
//
MainFrame::MainFrame(wxFrame* frame, const wxString& title,
	const wxPoint& pos, const wxSize& size) :
wxFrame(frame, wxID_ANY, title, pos, size)
{
	VTLOG("  MainFrame constructor: enter\n");

    // tell wxAuiManager to manage this frame
    m_mgr.SetManagedWindow(this);

	// init app data
	m_pView = NULL;
	m_pActiveLayer = NULL;
	m_SpeciesListDlg = NULL;
	m_BioRegionDlg = NULL;
	m_pFeatInfoDlg = NULL;
	m_pDistanceDlg = NULL;
	m_pProfileDlg = NULL;
	m_pLinearStructureDlg = NULL;
	m_pInstanceDlg = NULL;
	m_bDrawDisabled = false;
	m_bAdoptFirstCRS = true;
	m_LSOptions.Defaults();
	m_pToolbar = NULL;
	m_pMapServerDlg = NULL;
	for (int i = 0; i < LAYER_TYPES; i++)
		m_pLayBar[i] = NULL;

	m_tileopts.cols = 4;
	m_tileopts.rows = 4;
	m_tileopts.numlods = 3;
	m_tileopts.lod0size = 512;
	m_tileopts.bCreateDerivedImages = false;
	m_tileopts.bUseTextureCompression = true;
	m_tileopts.eCompressionType = TC_OPENGL;

	// frame icon
	SetIcon(wxICON(vtbuilder));
	VTLOG1("  MainFrame constructor: exit\n");
}

MainFrame::~MainFrame()
{
	VTLOG1("Frame destructor\n");
	WriteXML(APPNAME ".xml");
	DeleteContents();

    m_mgr.UnInit();
}

void MainFrame::CreateView()
{
	VTLOG1("CreateView\n");
	m_pView = new BuilderView(this, wxID_ANY,
			wxPoint(0, 0), wxSize(200, 400), _T("") );

    m_mgr.AddPane(m_pView, wxAuiPaneInfo().
                  Name(wxT("view")).Caption(wxT("View")).
                  CenterPane().Show(true));
	m_mgr.Update();
	VTLOG1(" refreshing view\n");
	m_pView->Refresh();
}

void MainFrame::ZoomAll()
{
	VTLOG("Zoom All\n");
	m_pView->ZoomToRect(GetExtents(), 0.1f);
}

void MainFrame::SetupUI()
{
	// set up the datum list we will use
	SetupEPSGDatums();

	m_statbar = new MyStatusBar(this);
	SetStatusBar(m_statbar);
	m_statbar->Show();
	m_statbar->SetTexts(this);
	PositionStatusBar();

	CreateMenus();
	CreateToolbar();

#if wxUSE_DRAG_AND_DROP
	SetDropTarget(new DnDFile);
#endif

	m_pTree = new MyTreeCtrl(this, LayerTree_Ctrl,
			wxPoint(0, 0), wxSize(200, 400),
			wxTR_HIDE_ROOT | wxNO_BORDER);

    m_mgr.AddPane(m_pTree, wxAuiPaneInfo().
                  Name(_T("layers")).Caption(_("Layers")).
                  Left());
	m_mgr.Update();

	// The following makes the views match, but it looks funny on Linux
//	m_pTree->SetBackgroundColour(*wxLIGHT_GREY);

	CreateView();
	m_pView->SetBackgroundColour(*wxLIGHT_GREY);
	m_pView->Show(FALSE);

	// Read INI file after creating the view
	if (!ReadXML(APPNAME ".xml"))
	{
		// fall back on older ini file
		ReadINI(APPNAME ".ini");
	}

	RefreshToolbars();

	vtProjection proj;
	proj.SetWellKnownGeogCS("WGS84");
	SetProjection(proj);
	RefreshStatusBar();

	// Get datapaths from Enviro
	ReadEnviroPaths();
	VTLOG("Datapaths:\n");
	int i, n = m_datapaths.size();
	if (n == 0)
		VTLOG("   none.\n");
	for (i = 0; i < n; i++)
		VTLOG("   %s\n", (const char *) m_datapaths[i]);

	// Load structure defaults
	bool foundmaterials = LoadGlobalMaterials(m_datapaths);
	if (!foundmaterials)
		DisplayAndLog("The building materials file (Culture/materials.xml) was not found\n"
			" on your Data Path.  Without this file, materials will not be handled\n"
			" correctly.  Please check your Data Paths to avoid this problem.");
	SetupDefaultStructures(FindFileOnPaths(m_datapaths, "BuildingData/DefaultStructures.vtst"));

	// Load content files, which might be referenced by structure layers
	LookForContentFiles();

#if 0
	long style = 0;//wxCAPTION | wxCLOSE_BOX;
	m_pScaleBar = new vtScaleBar(this, wxID_ANY, wxPoint(0,0), wxSize(400,30), style,
		_T("ScaleBarPanel"));
    m_mgr.AddPane(m_pScaleBar, wxAuiPaneInfo().
                  Name(wxT("scalebar")).Caption(wxT("Scale Bar")).
                  Dockable(false).Float().MinSize(400,30).MaxSize(400,60).Resizable(false));
	m_mgr.Update();
#endif

	// Again, for good measure
	m_mgr.Update();

	SetStatusText(_("Ready"));
}

void MainFrame::DeleteContents()
{
	m_Layers.Empty();
	m_pActiveLayer = NULL;
	FreeGlobalMaterials();
	FreeContentFiles();
}

void MainFrame::OnClose(wxCloseEvent &event)
{
	VTLOG("Frame OnClose\n");
	int num = NumModifiedLayers();
	if (num > 0)
	{
		wxString str;
		str.Printf(_("There are %d layers modified but unsaved.\n Are you sure you want to exit?"), num);
		if (wxMessageBox(str, _("Warning"), wxYES_NO) == wxNO)
		{
			event.Veto();
			return;
		}
	}

	if (m_pFeatInfoDlg != NULL)
	{
		// For some reason, destroying the list control in the feature
		//  dialog is dangerous if allowed to occur naturally, but it is
		//  safe to do it at this point.
		m_pFeatInfoDlg->Clear();
	}

	Destroy();
}

void MainFrame::ManageToolbar(const wxString &name, wxToolBar *bar, bool show)
{
	wxAuiPaneInfo api;
	api.Name(name);
	api.ToolbarPane();
	api.Top();
	api.LeftDockable(false);
	api.RightDockable(false);
	api.Show(show);
    m_mgr.AddPane(bar, api);
}

wxToolBar *MainFrame::NewToolbar()
{
	int style = (wxTB_FLAT | wxTB_NODIVIDER);
	wxToolBar *bar = new wxToolBar(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, style);
	bar->SetMargins(1, 1);
	bar->SetToolBitmapSize(wxSize(20, 20));
	return bar;
}

void MainFrame::CreateToolbar()
{
	// tool bar
	m_pToolbar = NewToolbar();
	m_pLayBar[LT_RAW] = NewToolbar();
	m_pLayBar[LT_ELEVATION] = NewToolbar();
	m_pLayBar[LT_IMAGE] = NewToolbar();
	m_pLayBar[LT_ROAD] = NewToolbar();
	m_pLayBar[LT_STRUCTURE] = NewToolbar();
	m_pLayBar[LT_VEG] = NewToolbar();
	m_pLayBar[LT_UTILITY] = NewToolbar();

	AddMainToolbars();

	ManageToolbar(_T("toolbar"), m_pToolbar, true);
	ManageToolbar(_T("toolbar_raw"), m_pLayBar[LT_RAW], false);
	ManageToolbar(_T("toolbar_elev"), m_pLayBar[LT_ELEVATION], false);
	ManageToolbar(_T("toolbar_image"), m_pLayBar[LT_IMAGE], false);
	ManageToolbar(_T("toolbar_road"), m_pLayBar[LT_ROAD], false);
	ManageToolbar(_T("toolbar_struct"), m_pLayBar[LT_STRUCTURE], false);
	ManageToolbar(_T("toolbar_veg"), m_pLayBar[LT_VEG], false);
	ManageToolbar(_T("toolbar_util"), m_pLayBar[LT_UTILITY], false);
	m_mgr.Update();
}

void MainFrame::RefreshToolbars()
{
	vtLayer *lay = GetActiveLayer();
	LayerType lt = LT_UNKNOWN;
	if (lay)
		lt = lay->GetType();

	m_pMenuBar->EnableTop(m_iLayerMenu[LT_ELEVATION], lt == LT_ELEVATION);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_IMAGE], lt == LT_IMAGE);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_ROAD], lt == LT_ROAD);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_UTILITY], lt == LT_UTILITY);
//	m_pMenuBar->EnableTop(m_iLayerMenu[LT_VEG], lt == LT_VEG);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_STRUCTURE], lt == LT_STRUCTURE);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_RAW], lt == LT_RAW);

	for (int i = 0; i < LAYER_TYPES; i++)
	{
		wxToolBar *bar = m_pLayBar[i];
		if (bar)
		{
			wxAuiPaneInfo &info = m_mgr.GetPane(bar);
			info.Show(lt == i);
		}
	}
	m_mgr.Update();
}

void MainFrame::AddMainToolbars()
{
	ADD_TOOL(m_pToolbar, ID_FILE_NEW, wxBITMAP(proj_new), _("New Project"));
	ADD_TOOL(m_pToolbar, ID_FILE_OPEN, wxBITMAP(proj_open), _("Open Project"));
	ADD_TOOL(m_pToolbar, ID_FILE_SAVE, wxBITMAP(proj_save), _("Save Project"));
	ADD_TOOL(m_pToolbar, ID_VIEW_OPTIONS, wxBITMAP(view_options), _("View Options"));
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_LAYER_NEW, wxBITMAP(layer_new), _("New Layer"));
	ADD_TOOL(m_pToolbar, ID_LAYER_OPEN, wxBITMAP(layer_open), _("Open Layer"));
	ADD_TOOL(m_pToolbar, ID_LAYER_SAVE, wxBITMAP(layer_save), _("Save Layer"));
	ADD_TOOL(m_pToolbar, ID_LAYER_IMPORT, wxBITMAP(layer_import), _("Import Data"));
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_EDIT_DELETE, wxBITMAP(edit_delete), _("Delete"));
	ADD_TOOL(m_pToolbar, ID_EDIT_OFFSET, wxBITMAP(edit_offset), _("Offset"));
	ADD_TOOL2(m_pToolbar, ID_VIEW_SHOWLAYER, wxBITMAP(layer_show), _("Layer Visibility"), wxITEM_CHECK);
	ADD_TOOL(m_pToolbar, ID_VIEW_LAYER_UP, ToolBitmapsFunc( 0 ), _("Layer Up"));
	ADD_TOOL(m_pToolbar, ID_VIEW_LAYER_DOWN, ToolBitmapsFunc( 1 ), _("Layer Down"));
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOMIN, wxBITMAP(view_plus), _("Zoom In"));
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOMOUT, wxBITMAP(view_minus), _("Zoom Out"));
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOMALL, wxBITMAP(view_zoomall), _("Zoom All"));
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOM_LAYER, wxBITMAP(view_zoom_layer), _("Zoom To Layer"));
	m_pToolbar->AddSeparator();
	ADD_TOOL2(m_pToolbar, ID_VIEW_MAGNIFIER, wxBITMAP(view_mag), _("Magnifier"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_PAN, wxBITMAP(view_hand), _("Pan"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_DISTANCE, wxBITMAP(distance), _("Distance"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_SETAREA, wxBITMAP(elev_box), _("Area Tool"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_PROFILE, wxBITMAP(view_profile), _("Elevation Profile"), wxITEM_CHECK);
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_AREA_EXPORT_ELEV, wxBITMAP(elev_resample), _("Merge/Resample Elevation"));
	ADD_TOOL(m_pToolbar, ID_AREA_EXPORT_IMAGE, wxBITMAP(image_resample), _("Merge/Resample Imagery"));
	m_pToolbar->Realize();

	// Raw
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_FEATURE_SELECT, wxBITMAP(select), _("Select Features"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_FEATURE_PICK, wxBITMAP(info), _("Pick Features"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_FEATURE_TABLE, wxBITMAP(table), _("Table"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_RAW_ADDPOINTS, wxBITMAP(raw_add_point), _("Add Points with Mouse"), wxITEM_CHECK);
	m_pLayBar[LT_RAW]->Realize();

	// Elevation
	ADD_TOOL2(m_pLayBar[LT_ELEVATION], ID_ELEV_SELECT, wxBITMAP(select), _("Select Elevation"), wxITEM_CHECK);
	ADD_TOOL(m_pLayBar[LT_ELEVATION], ID_VIEW_FULLVIEW, wxBITMAP(view_zoomexact), _("Zoom to Full Detail"));
	ADD_TOOL2(m_pLayBar[LT_ELEVATION], ID_ELEV_TRIMTIN, wxBITMAP(tin_trim), _("Trim TIN triangles by line segment"), wxITEM_CHECK);
	m_pLayBar[LT_ELEVATION]->Realize();

	// Image
	ADD_TOOL(m_pLayBar[LT_IMAGE], ID_VIEW_FULLVIEW, wxBITMAP(view_zoomexact), _("Zoom to Full Detail"));
	m_pLayBar[LT_IMAGE]->Realize();

	// Road
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SELECTROAD, wxBITMAP(rd_select_road), _("Select Roads"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SELECTNODE, wxBITMAP(rd_select_node), _("Select Nodes"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SELECTWHOLE, wxBITMAP(rd_select_whole), _("Select Whole Roads"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_DIRECTION, wxBITMAP(rd_direction), _("Set Road Direction"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_EDIT, wxBITMAP(rd_edit), _("Edit Road Points"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SHOWNODES, wxBITMAP(rd_shownodes), _("Show Nodes"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_EDIT_CROSSINGSELECTION, wxBITMAP(edit_crossing), _("Crossing Selection"), wxITEM_CHECK);
	m_pLayBar[LT_ROAD]->Realize();

	// Structure
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_FEATURE_SELECT, wxBITMAP(select), _("Select Features"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_EDIT_BLD, wxBITMAP(bld_edit), _("Edit Buildings"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_ADD_POINTS, wxBITMAP(bld_add_points), _("Add points to building footprints"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_DELETE_POINTS, wxBITMAP(bld_delete_points), _("Delete points from building footprints"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_ADD_LINEAR, wxBITMAP(str_add_linear), _("Add Linear Structures"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_EDIT_LINEAR, wxBITMAP(str_edit_linear), _("Edit Linear Structures"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_CONSTRAIN, wxBITMAP(bld_corner), _("Constrain Angles"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_ADD_INST, wxBITMAP(instances), _("Add Instances"), wxITEM_CHECK);
	m_pLayBar[LT_STRUCTURE]->Realize();

	// Veg
	ADD_TOOL2(m_pLayBar[LT_VEG], ID_FEATURE_SELECT, wxBITMAP(select), _("Select Plants"), wxITEM_CHECK);
	m_pLayBar[LT_VEG]->Realize();

	// Utility
	ADD_TOOL2(m_pLayBar[LT_UTILITY], ID_TOWER_ADD,wxBITMAP(rd_select_node), _("Add Tower"), wxITEM_CHECK);
	m_pLayBar[LT_UTILITY]->AddSeparator();
	ADD_TOOL2(m_pLayBar[LT_UTILITY], ID_TOWER_SELECT,wxBITMAP(select),_("Select Towers"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_UTILITY], ID_TOWER_EDIT, wxBITMAP(twr_edit), _("Edit Towers"), wxITEM_CHECK);
	m_pLayBar[LT_UTILITY]->Realize();
}


////////////////////////////////////////////////////////////////
// Application Methods

//
// Load a layer from a file without knowing its type
//
void MainFrame::LoadLayer(const wxString &fname_in)
{
	LayerType ltype = LT_UNKNOWN;

	// check file extension
	wxString fname = fname_in;
	wxString ext = fname.AfterLast('.');

	vtLayer *pLayer = NULL;
	if (ext.CmpNoCase(_T("rmf")) == 0)
	{
		vtRoadLayer *pRL = new vtRoadLayer;
		if (pRL->Load(fname))
			pLayer = pRL;
		else
			delete pRL;
	}
	if (ext.CmpNoCase(_T("bt")) == 0 ||
		ext.CmpNoCase(_T("tin")) == 0 ||
		ext.CmpNoCase(_T("itf")) == 0 ||
		fname.Right(6).CmpNoCase(_T(".bt.gz")) == 0)
	{
		vtElevLayer *pEL = new vtElevLayer;
		if (pEL->Load(fname))
			pLayer = pEL;
		else
			delete pEL;
	}
#if SUPPORT_TRANSIT
	if (ext.CmpNoCase(_T("xml")) == 0)
	{
		vtTransitLayer *pTL = new vtTransitLayer;
		if (pTL->Load(fname))
			pLayer = pTL;
	}
#endif
	if (ext.CmpNoCase(_T("vtst")) == 0 ||
		fname.Right(8).CmpNoCase(_T(".vtst.gz")) == 0)
	{
		vtStructureLayer *pSL = new vtStructureLayer;
		if (pSL->Load(fname))
			pLayer = pSL;
		else
			delete pSL;
	}
	if (ext.CmpNoCase(_T("vf")) == 0)
	{
		vtVegLayer *pVL = new vtVegLayer;
		if (pVL->Load(fname))
			pLayer = pVL;
		else
			delete pVL;
	}
	if (ext.CmpNoCase(_T("utl")) == 0)
	{
		vtUtilityLayer *pTR = new vtUtilityLayer;
		if(pTR->Load(fname))
			pLayer = pTR;
		else
			delete pTR;
	}
	if (ext.CmpNoCase(_T("shp")) == 0 ||
		ext.CmpNoCase(_T("gml")) == 0 ||
		ext.CmpNoCase(_T("xml")) == 0 ||
		ext.CmpNoCase(_T("igc")) == 0)
	{
		vtRawLayer *pRL = new vtRawLayer;
		if (pRL->Load(fname))
			pLayer = pRL;
		else
			delete pRL;
 	}
	if (ext.CmpNoCase(_T("img")) == 0)
	{
		vtImageLayer *pIL = new vtImageLayer;
		if (pIL->Load(fname))
			pLayer = pIL;
		else
			delete pIL;
	}
	if (ext.CmpNoCase(_T("tif")) == 0)
	{
		// If it's a 8-bit or 24-bit TIF, then it's likely to be an image.
		// If it's a 16-bit TIF, then it's likely to be elevation.
		int depth = GetBitDepthUsingGDAL(fname_in.mb_str(wxConvUTF8));
		if (depth == 8 || depth == 24 || depth == 32)
		{
			vtImageLayer *pIL = new vtImageLayer;
			if (pIL->Load(fname))
				pLayer = pIL;
			else
				delete pIL;
		}
		else if (depth == 16)
			ltype = LT_ELEVATION;
	}
	if (pLayer)
	{
		bool success = AddLayerWithCheck(pLayer, true);
		if (!success)
			delete pLayer;
	}
	else
	{
		// try importing
		ImportDataFromArchive(ltype, fname, true);
	}

}

void MainFrame::AddLayer(vtLayer *lp)
{
	m_Layers.Append(lp);
}

bool MainFrame::AddLayerWithCheck(vtLayer *pLayer, bool bRefresh)
{
	vtProjection proj;
	pLayer->GetProjection(proj);

	bool bFirst = (m_Layers.GetSize() == 0);
	if (bFirst && m_bAdoptFirstCRS)
	{
		// if this is our first layer, adopt its projection
		SetProjection(proj);
	}
	else
	{
		// check for Projection conflict
		if (!(m_proj == proj))
		{
			char *str1, *str2;
			m_proj.exportToProj4(&str1);
			proj.exportToProj4(&str2);

			bool keep = false;
			wxString msg;
			msg.Printf(_("The data already loaded is in:\n   %hs\n but the layer you are attempting to add:\n   %s\n is using:\n   %hs\n Would you like to attempt to convert it now to the existing projection?"),
				str1,
				pLayer->GetLayerFilename().c_str(),
				str2);
			OGRFree(str1);
			OGRFree(str2);
			int ret = wxMessageBox(msg, _("Convert Coordinate System?"), wxYES_NO | wxCANCEL);
			if (ret == wxNO)
				keep = true;
			if (ret == wxYES)
			{
				OpenProgressDialog(_("Reprojecting"), false, this);
				bool success = pLayer->TransformCoords(m_proj);
				CloseProgressDialog();
				if (success)
					keep = true;
				else
				{
					ret = wxMessageBox(_("Couldn't convert projection.\n Proceed anyway?"),
						_("Warning"), wxYES_NO);
					if (ret == wxYES)
						keep = true;
				}
			}
			if (!keep)
				return false;
		}
	}
	AddLayer(pLayer);
	SetActiveLayer(pLayer, false);
	if (bRefresh)
	{
		// refresh the view
		ZoomAll();
		RefreshToolbars();
		RefreshTreeView();
		RefreshStatusBar();
	}
	return true;
}

void MainFrame::RemoveLayer(vtLayer *lp)
{
	if (!lp)
		return;

	// check the type of the layer we're deleting
	LayerType lt = lp->GetType();

	// remove and delete the layer
	m_Layers.RemoveAt(m_Layers.Find(lp));

	// if it was the active layer, select another layer of the same type
	if (GetActiveLayer() == lp)
	{
		vtLayer *lp_new = FindLayerOfType(lt);
		SetActiveLayer(lp_new, true);
	}

	// if it was being shown in the feature info dialog, reset that dialog
	if (m_pFeatInfoDlg && m_pFeatInfoDlg->GetLayer() == lp)
	{
		m_pFeatInfoDlg->SetLayer(NULL);
		m_pFeatInfoDlg->SetFeatureSet(NULL);
	}

	DeleteLayer(lp);
	m_pView->Refresh();
	m_pTree->RefreshTreeItems(this);
	RefreshToolbars();
}

void MainFrame::DeleteLayer(vtLayer *lp)
{
	delete lp;
}

void MainFrame::SetActiveLayer(vtLayer *lp, bool refresh)
{
	LayerType last = m_pActiveLayer ? m_pActiveLayer->GetType() : LT_UNKNOWN;

	m_pActiveLayer = lp;
	if (refresh)
		m_pTree->RefreshTreeStatus(this);

	// change mouse mode based on layer type
	if (lp == NULL)
		m_pView->SetMode(LB_Mag);

	if (lp != NULL)
	{
		if (lp->GetType() == LT_ELEVATION && last != LT_ELEVATION)
			m_pView->SetMode(LB_TSelect);

		if (lp->GetType() == LT_ROAD && last != LT_ROAD)
			m_pView->SetMode(LB_Link);

		if (lp->GetType() == LT_STRUCTURE && last != LT_STRUCTURE)
			m_pView->SetMode(LB_FSelect);

		if (lp->GetType() == LT_UTILITY && last != LT_UTILITY)
			m_pView->SetMode(LB_FSelect);

		if (lp->GetType() == LT_RAW && last != LT_RAW)
			m_pView->SetMode(LB_FSelect);
	}
}

//
// Returns the number of layers present of a given type.
//
int MainFrame::LayersOfType(LayerType lt)
{
	int count = 0;
	int layers = m_Layers.GetSize();
	for (int l = 0; l < layers; l++)
	{
		if (m_Layers.GetAt(l)->GetType() == lt)
			count++;
	}
	return count;
}

int MainFrame::NumModifiedLayers()
{
	int count = 0;
	int layers = m_Layers.GetSize();
	for (int l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers[l];
		if (lp->GetModified() && lp->CanBeSaved())
			count++;
	}
	return count;
}

vtLayer *MainFrame::FindLayerOfType(LayerType lt)
{
	int layers = m_Layers.GetSize();
	for (int l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == lt)
			return lp;
	}
	return NULL;
}

int MainFrame::LayerNum(vtLayer *lp)
{
	int layers = m_Layers.GetSize();
	for (int i = 0; i < layers; i++)
		if (lp == m_Layers[i])
			return i;
	return -1;
}

void MainFrame::SwapLayerOrder(int n0, int n1)
{
	vtLayer *lp0 = m_Layers[n0];
	vtLayer *lp1 = m_Layers[n1];
	m_Layers[n0] = lp1;
	m_Layers[n1] = lp0;
}

void MainFrame::RefreshLayerInView(vtLayer *pLayer)
{
	DRECT r;
	pLayer->GetExtent(r);
	wxRect sr = m_pView->WorldToWindow(r);
	IncreaseRect(sr, 5);
	m_pView->Refresh(TRUE, &sr);
}


//
// read / write ini file
//
bool MainFrame::ReadINI(const char *fname)
{
	FILE *fpIni = vtFileOpen(fname, "rb+");

	if (fpIni)
	{
		int ShowMap, ShowElev, ShadeQuick, DoMask, DoUTM, ShowPaths, DrawWidth,
			CastShadows, ShadeDot=0, Angle=30, Direction=45;
		float Ambient = 0.1f;
		float Gamma = 0.8f;
		fscanf(fpIni, "%d %d %d %d %d %d %d %d %d %d %d %f %f", &ShowMap,
			&ShowElev, &ShadeQuick, &DoMask, &DoUTM, &ShowPaths, &DrawWidth,
			&CastShadows, &ShadeDot, &Angle, &Direction, &Ambient, &Gamma);

		vtElevLayer::m_draw.m_bShowElevation = (ShowElev != 0);
		vtElevLayer::m_draw.m_bShadingQuick = (ShadeQuick != 0);
		vtElevLayer::m_draw.m_bShadingDot = (ShadeDot != 0);
		vtElevLayer::m_draw.m_bDoMask = (DoMask != 0);
		vtElevLayer::m_draw.m_bCastShadows = (CastShadows != 0);
		vtElevLayer::m_draw.m_iCastAngle = Angle;
		vtElevLayer::m_draw.m_iCastDirection = Direction;
		vtElevLayer::m_draw.m_fAmbient = Ambient;
		vtElevLayer::m_draw.m_fGamma = Gamma;
		m_pView->SetShowMap(ShowMap != 0);
		m_pView->m_bShowUTMBounds = (DoUTM != 0);
		m_pTree->SetShowPaths(ShowPaths != 0);
		vtRoadLayer::SetDrawWidth(DrawWidth != 0);

		char buf[4000];
		if (fscanf(fpIni, "\n%s\n", buf) == 1)
		{
			wxString str(buf, wxConvUTF8);
			m_mgr.LoadPerspective(str, false);
		}

		return true;
	}
	return false;
}

#if 0
bool MainFrame::WriteINI()
{
	fpIni = vtFileOpen(m_szIniFilename, "wb");
	if (fpIni)
	{
		wxString str = m_mgr.SavePerspective();
		vtString vs = (const char *) str.mb_str(wxConvUTF8);

		rewind(fpIni);
		fprintf(fpIni, "%d %d %d %d %d %d %d %d %d %d %d %f %f\n", m_pView->GetShowMap(),
			vtElevLayer::m_draw.m_bShowElevation,
			vtElevLayer::m_draw.m_bShadingQuick, vtElevLayer::m_draw.m_bDoMask,
			m_pView->m_bShowUTMBounds, m_pTree->GetShowPaths(),
			vtRoadLayer::GetDrawWidth(), vtElevLayer::m_draw.m_bCastShadows,
			vtElevLayer::m_draw.m_bShadingDot, vtElevLayer::m_draw.m_iCastAngle,
			vtElevLayer::m_draw.m_iCastDirection, vtElevLayer::m_draw.m_fAmbient,
			vtElevLayer::m_draw.m_fGamma);
		fprintf(fpIni, "%s\n", (const char *) vs);
		fclose(fpIni);
		return true;
	}
	return false;
}
#endif

//
// read / write ini file
//
bool MainFrame::ReadXML(const char *fname)
{
	if (!g_Options.LoadFromXML(fname))
		return false;

	// Safety check
	if (g_Options.GetValueInt(TAG_SAMPLING_N) < 1)
		g_Options.SetValueInt(TAG_SAMPLING_N, 1);
	if (g_Options.GetValueInt(TAG_SAMPLING_N) > 32)
		g_Options.SetValueInt(TAG_SAMPLING_N, 32);

	// Apply all the options, from g_Options the rest of the application
	m_pView->SetShowMap(g_Options.GetValueBool(TAG_SHOW_MAP));
	m_pView->m_bShowUTMBounds = g_Options.GetValueBool(TAG_SHOW_UTM);
	m_pTree->SetShowPaths(g_Options.GetValueBool(TAG_SHOW_PATHS));
	vtRoadLayer::SetDrawWidth(g_Options.GetValueBool(TAG_ROAD_DRAW_WIDTH));

	vtElevLayer::m_draw.SetFromTags(g_Options);

	vtString str;
	if (g_Options.GetValueString("UI_Layout", str))
	{
		wxString str2(str, wxConvUTF8);
		m_mgr.LoadPerspective(str2, false);
	}

	return true;
}

bool MainFrame::WriteXML(const char *fname)
{
	// Gather all the options into g_Options
	g_Options.SetValueBool(TAG_SHOW_MAP, m_pView->GetShowMap());
	g_Options.SetValueBool(TAG_SHOW_UTM, m_pView->m_bShowUTMBounds);
	g_Options.SetValueBool(TAG_SHOW_PATHS, m_pTree->GetShowPaths());
	g_Options.SetValueBool(TAG_ROAD_DRAW_WIDTH, vtRoadLayer::GetDrawWidth());

	vtElevLayer::m_draw.SetToTags(g_Options);

	wxString str = m_mgr.SavePerspective();
	g_Options.SetValueString("UI_Layout", (const char *) str.mb_str(wxConvUTF8));

	// Write it to XML
	return g_Options.WriteToXML(fname, "Options");
}

DRECT MainFrame::GetExtents()
{
	DRECT rect(1E9,-1E9,-1E9,1E9);

	bool has_bounds = false;

	// Acculumate the extents of all the layers
	DRECT rect2;
	int iLayers = m_Layers.GetSize();

	for (int i = 0; i < iLayers; i++)
	{
		if (m_Layers.GetAt(i)->GetExtent(rect2))
		{
			rect.GrowToContainRect(rect2);
			has_bounds = true;
		}
	}
	if (has_bounds)
		return rect;
	else if (m_proj.IsDymaxion())
	{
		return DRECT(0, 1.5*sqrt(3.0), 5.5, 0);
	}
	else
		return DRECT(-180,90,180,-90);	// geo extents of whole planet
}

//
// Pick a point, in geographic coords, which is roughly in the middle
//  of the data that the user is working with.
//
DPoint2 MainFrame::EstimateGeoDataCenter()
{
	DRECT rect = GetExtents();
	DPoint2 pos = rect.GetCenter();

	if (!m_proj.IsGeographic())
	{
		vtProjection geo;
		CreateSimilarGeographicProjection(m_proj, geo);
		OCT *trans = CreateConversionIgnoringDatum(&m_proj, &geo);
		if (trans)
			trans->Transform(1, &pos.x, &pos.y);
		delete trans;
	}
	return pos;
}

void MainFrame::RefreshTreeView()
{
	if (m_pTree)
		m_pTree->RefreshTreeItems(this);
}

void MainFrame::RefreshTreeStatus()
{
	if (m_pTree)
		m_pTree->RefreshTreeStatus(this);
}

void MainFrame::RefreshStatusBar()
{
	m_statbar->SetTexts(this);
}

LayerType MainFrame::AskLayerType()
{
	wxString choices[LAYER_TYPES];
	for (int i = 0; i < LAYER_TYPES; i++)
		choices[i] = vtLayer::LayerTypeNames[i];

	int n = LAYER_TYPES;
	static int cur_type = 0;	// remember the choice for next time

	wxSingleChoiceDialog dialog(this, _("These are your choices"),
		_("Please indicate layer type"), n, (const wxString *)choices);

	dialog.SetSelection(cur_type);

	if (dialog.ShowModal() == wxID_OK)
	{
		cur_type = dialog.GetSelection();
		return (LayerType) cur_type;
	}
	else
		return LT_UNKNOWN;
}

vtFeatureSet *MainFrame::GetActiveFeatureSet()
{
	if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_RAW)
		return ((vtRawLayer *)m_pActiveLayer)->GetFeatureSet();
	return NULL;
}

FeatInfoDlg	*MainFrame::ShowFeatInfoDlg()
{
	if (!m_pFeatInfoDlg)
	{
		// Create new Feature Info Dialog
		m_pFeatInfoDlg = new FeatInfoDlg(this, wxID_ANY, _("Feature Info"),
				wxPoint(120, 80), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pFeatInfoDlg->SetView(GetView());
	}
	m_pFeatInfoDlg->Show(true);
	return m_pFeatInfoDlg;
}


DistanceDlg2d *MainFrame::ShowDistanceDlg()
{
	if (!m_pDistanceDlg)
	{
		// Create new Distance Dialog
		m_pDistanceDlg = new DistanceDlg2d(this, wxID_ANY, _("Distance Tool"),
				wxPoint(200, 200), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pDistanceDlg->SetProjection(m_proj);
		m_pDistanceDlg->SetFrame(this);
	}
	m_pDistanceDlg->Show(true);
	return m_pDistanceDlg;
}

void MainFrame::UpdateDistance(const DPoint2 &p1, const DPoint2 &p2)
{
	DistanceDlg2d *pDlg = ShowDistanceDlg();
	if (pDlg)
	{
		pDlg->SetPoints(p1, p2, true);
		float h1 = GetHeightFromTerrain(p1);
		float h2 = GetHeightFromTerrain(p2);
		float diff = FLT_MIN;
		if (h1 != INVALID_ELEVATION && h2 != INVALID_ELEVATION)
			diff = h2 - h1;
		if (pDlg)
			pDlg->SetGroundAndVertical(FLT_MIN, diff, false);
	}

	ProfileDlg *pDlg2 = m_pProfileDlg;
	if (pDlg2)
		pDlg2->SetPoints(p1, p2);
}

void MainFrame::UpdateDistance(const DLine2 &path)
{
	DistanceDlg2d *pDlg = ShowDistanceDlg();
	if (pDlg)
	{
		pDlg->SetPath(path, true);
		//float h1 = GetHeightFromTerrain(p1);
		//float h2 = GetHeightFromTerrain(p2);
		//float diff = FLT_MIN;
		//if (h1 != INVALID_ELEVATION && h2 != INVALID_ELEVATION)
		//	diff = h2 - h1;
		//if (pDlg)
		//	pDlg->SetGroundAndVertical(FLT_MIN, diff, false);
	}

	ProfileDlg *pDlg2 = m_pProfileDlg;
	if (pDlg2)
		pDlg2->SetPath(path);
}

void MainFrame::ClearDistance()
{
	UpdateDistance(DPoint2(0,0), DPoint2(0,0));
	UpdateDistance(DLine2());

	// erase previous
	GetView()->ClearDistanceTool();
}


class LinearStructureDlg2d: public LinearStructureDlg
{
public:
	LinearStructureDlg2d(wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos, const wxSize& size, long style) :
	LinearStructureDlg(parent, id, title, pos, size, style) {}
	void OnSetOptions(const vtLinearParams &opt)
	{
		m_pFrame->m_LSOptions = opt;
	}
	MainFrame *m_pFrame;
};

LinearStructureDlg *MainFrame::ShowLinearStructureDlg(bool bShow)
{
	if (bShow && !m_pLinearStructureDlg)
	{
		// Create new Distance Dialog
		m_pLinearStructureDlg = new LinearStructureDlg2d(this, -1,
			_("Linear Structures"), wxPoint(120, 80), wxSize(600, 200),
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pLinearStructureDlg->m_pFrame = this;
	}
	if (m_pLinearStructureDlg)
		m_pLinearStructureDlg->Show(bShow);
	return m_pLinearStructureDlg;
}


InstanceDlg *MainFrame::ShowInstanceDlg(bool bShow)
{
	if (bShow && !m_pInstanceDlg)
	{
		// Create new Distance Dialog
		m_pInstanceDlg = new InstanceDlg(this, -1,
			_("Structure Instances"), wxPoint(120, 80), wxSize(600, 200));

		for (unsigned int i = 0; i < m_contents.size(); i++)
			m_pInstanceDlg->AddContent(m_contents[i]);
		m_pInstanceDlg->SetProjection(m_proj);
	}
	if (m_pInstanceDlg)
		m_pInstanceDlg->Show(bShow);
	return m_pInstanceDlg;
}

void MainFrame::LookForContentFiles()
{
	VTLOG1("Searching data paths for content files (.vtco)\n");
	for (unsigned int i = 0; i < m_datapaths.size(); i++)
	{
		vtStringArray array;
		AddFilenamesToStringArray(array, m_datapaths[i], "*.vtco");

		for (unsigned int j = 0; j < array.size(); j++)
		{
			vtString path = m_datapaths[i];
			path += array[j];

			bool success = true;
			vtContentManager *mng = new vtContentManager;
			try
			{
				mng->ReadXML(path);
			}
			catch (xh_io_exception &ex)
			{
				// display (or at least log) error message here
				VTLOG("XML error:");
				VTLOG(ex.getFormattedMessage().c_str());
				success = false;
				delete mng;
			}
			if (success)
				m_contents.push_back(mng);
		}
	}
	VTLOG(" found %d files on %d paths\n", m_contents.size(), m_datapaths.size());
}

void MainFrame::FreeContentFiles()
{
	for (unsigned int i = 0; i < m_contents.size(); i++)
		delete m_contents[i];
	m_contents.clear();
}

void MainFrame::ResolveInstanceItem(vtStructInstance *inst)
{
	vtString name;
	if (!inst->GetValueString("itemname", name))
		return;
	for (unsigned int j = 0; j < m_contents.size(); j++)
	{
		vtItem *item = m_contents[j]->FindItemByName(name);
		if (item)
		{
			inst->SetItem(item);
			break;
		}
	}
}

class BuildingProfileCallback : public ProfileCallback
{
public:
	void Begin()
	{
		m_elevs.clear();
		m_frame->ElevLayerArray(m_elevs);
	}
	float GetElevation(const DPoint2 &p)
	{
		return ElevLayerArrayValue(m_elevs, p);
	}
	float GetCultureHeight(const DPoint2 &p)
	{
		return INVALID_ELEVATION;
	}
	MainFrame *m_frame;
	std::vector<vtElevLayer*> m_elevs;
};

ProfileDlg *MainFrame::ShowProfileDlg()
{
	if (!m_pProfileDlg)
	{
		// Create new Feature Info Dialog
		m_pProfileDlg = new ProfileDlg(this, wxID_ANY, _("Elevation Profile"),
				wxPoint(120, 80), wxSize(730, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		BuildingProfileCallback *callback = new BuildingProfileCallback;
		callback->m_frame = this;
		m_pProfileDlg->SetCallback(callback);
		m_pProfileDlg->SetProjection(m_proj);
	}
	m_pProfileDlg->Show(true);

	// This might be the first time it's displayed, so we need to get
	//  the point values from the distance tool
	GetView()->UpdateDistance();

	return m_pProfileDlg;
}

unsigned int MainFrame::ElevLayerArray(std::vector<vtElevLayer*> &elevs)
{
	for (int l = 0; l < NumLayers(); l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_ELEVATION && lp->GetVisible())
			elevs.push_back((vtElevLayer *)lp);
	}
	return elevs.size();
}

bool MainFrame::FillElevGaps(vtElevLayer *el)
{
	// Create progress dialog for the slow part
	OpenProgressDialog(_("Filling Gaps"), true);

	bool bGood;
	if (g_Options.GetValueBool(TAG_SLOW_FILL_GAPS))
		// slow and smooth
		bGood = el->m_pGrid->FillGapsSmooth(progress_callback);
	else
		// fast
		bGood = el->m_pGrid->FillGaps(progress_callback);

	CloseProgressDialog();
	return bGood;
}

/**
 * From a set of elevation layers, pick the valid elevation that occurs latest
 *  in the set.
 */
float ElevLayerArrayValue(std::vector<vtElevLayer*> &elevs, const DPoint2 &p)
{
	float fData, fBestData = INVALID_ELEVATION;
	for (unsigned int g = 0; g < elevs.size(); g++)
	{
		vtElevLayer *elev = elevs[g];

		vtElevationGrid *grid = elev->m_pGrid;
		vtTin2d *tin = elev->m_pTin;
		if (grid)
		{
			fData = grid->GetFilteredValue(p);
			if (fData != INVALID_ELEVATION)
				fBestData = fData;
		}
		else if (tin)
		{
			if (tin->FindAltitudeOnEarth(p, fData))
				fBestData = fData;
		}
	}
	return fBestData;
}

/**
 * From a set of elevation layers, pick the average of all the valid elevations
 *  in the set.
 */
float ElevLayerArrayValueAverage(std::vector<vtElevLayer*> &elevs, const DPoint2 &p)
{
	float fData, fSum = 0.0f;
	int count = 0;

	for (unsigned int g = 0; g < elevs.size(); g++)
	{
		vtElevLayer *elev = elevs[g];

		vtElevationGrid *grid = elev->m_pGrid;
		vtTin2d *tin = elev->m_pTin;
		if (grid)
		{
			fData = grid->GetFilteredValue(p);
			if (fData != INVALID_ELEVATION)
			{
				fSum += fData;
				count++;
			}
		}
		else if (tin)
		{
			if (tin->FindAltitudeOnEarth(p, fData))
			{
				fSum += fData;
				count++;
			}
		}
	}
	if (count)
		return fSum / count;
	else
		return INVALID_ELEVATION;
}

void ElevLayerArrayRange(std::vector<vtElevLayer*> &elevs,
						 float &minval, float &maxval)
{
	float fMin = 1E9;
	float fMax = -1E9;
	for (unsigned int g = 0; g < elevs.size(); g++)
	{
		float LayerMin, LayerMax;
		elevs[g]->GetHeightField()->GetHeightExtents(LayerMin, LayerMax);

		if (LayerMin != INVALID_ELEVATION && LayerMin < fMin)
			fMin = LayerMin;
		if (LayerMax != INVALID_ELEVATION && LayerMax  > fMax)
			fMax = LayerMax;
	}
	minval = fMin;
	maxval = fMax;
}

//
// sample all elevation layers into this one
//
bool MainFrame::SampleCurrentTerrains(vtElevLayer *pTarget)
{
	VTLOG1(" SampleCurrentTerrains\n");
	// measure time
	clock_t tm1 = clock();

	DRECT area;
	pTarget->GetExtent(area);
	DPoint2 step = pTarget->m_pGrid->GetSpacing();

	int i, j, layers = m_Layers.GetSize();
	float fData=0, fBestData;
	int iColumns, iRows;
	pTarget->m_pGrid->GetDimensions(iColumns, iRows);

	// Create progress dialog for the slow part
	OpenProgressDialog(_("Merging and Resampling Elevation Layers"), true);

	std::vector<vtElevLayer*> elevs;
	ElevLayerArray(elevs);

	// Setup TINs for speedy picking
	for (int l = 0; l < NumLayers(); l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_ELEVATION && lp->GetVisible())
		{
			vtElevLayer *el = (vtElevLayer *)lp;
			if (el->m_pTin)
			{
				int tris = el->m_pTin->NumTris();
				// Aim for no more than 50 triangles in a bin
				int bins = (int) sqrt((double) tris / 50);
				if (bins < 10)
					bins = 10;
				el->m_pTin->SetupTriangleBins(bins, progress_callback);
			}
		}
	}

	// iterate through the vertices of the new terrain
	DPoint2 p;
	wxString str;
	for (i = 0; i < iColumns; i++)
	{
		if ((i % 5) == 0)
		{
			str.Printf(_T("%d / %d"), i, iColumns);
			if (UpdateProgressDialog(i*100/iColumns, str))
			{
				CloseProgressDialog();
				return false;
			}
		}
		p.x = area.left + (i * step.x);
		for (j = 0; j < iRows; j++)
		{
			p.y = area.bottom + (j * step.y);

			// find some data for this point
			fBestData = ElevLayerArrayValue(elevs, p);
			pTarget->m_pGrid->SetFValue(i, j, fBestData);
		}
	}
	CloseProgressDialog();

	clock_t tm2 = clock();
	float time = ((float)tm2 - tm1)/CLOCKS_PER_SEC;
	VTLOG(" SampleCurrentTerrains: %.3f seconds.\n", time);

	return true;
}


//
// sample all image data into this one
//
bool MainFrame::SampleCurrentImages(vtImageLayer *pTargetLayer)
{
	vtImage *pTarget = pTargetLayer->GetImage();

	DRECT area;
	pTarget->GetExtent(area);
	DPoint2 step = pTarget->GetSpacing();

	int i, j, l, layers = m_Layers.GetSize();
	int iColumns, iRows;
	pTarget->GetDimensions(iColumns, iRows);

	// Create progress dialog for the slow part
	OpenProgressDialog(_("Merging and Resampling Image Layers"), true);

	vtImage **images = new vtImage *[LayersOfType(LT_IMAGE)];
	int g, num_image = 0;
	for (l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_IMAGE)
			images[num_image++] = ((vtImageLayer *)lp)->GetImage();
	}

	// Get ready to multisample
	DLine2 offsets;
	MakeSampleOffsets(step, g_Options.GetValueInt(TAG_SAMPLING_N), offsets);

	// iterate through the pixels of the new image
	DPoint2 p;
	RGBi rgb, sampled;
	int count;
	for (j = 0; j < iRows; j++)
	{
		if (UpdateProgressDialog(j*100/iRows))
		{
			// Cancel
			CloseProgressDialog();
			return false;
		}

		// Sample at the pixel centers, which are 1/2 pixel in from extents
		p.y = area.bottom + (step.y/2) + (j * step.y);

		for (i = 0; i < iColumns; i++)
		{
			p.x = area.left + (step.x/2) + (i * step.x);

			// find some data for this point
			count = 0;
			for (g = 0; g < num_image; g++)
			{
				// take image that's on top (last in list)
				if (images[g]->GetMultiSample(p, offsets, sampled))
				{
					rgb = sampled;
					count++;
				}
			}
			if (count)
				pTarget->SetRGB(i, iRows-1-j, rgb.r, rgb.g, rgb.b);
			else
				// write NODATA (black, for now)
				pTarget->SetRGB(i, iRows-1-j, 0, 0, 0);
		}
	}
	CloseProgressDialog();
	delete images;
	return true;
}


float MainFrame::GetHeightFromTerrain(const DPoint2 &p)
{
	float height = INVALID_ELEVATION;

	int layers = m_Layers.GetSize();
	for (int i = 0; i < layers; i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() != LT_ELEVATION || !l->GetVisible()) continue;
		vtElevLayer *pEL = (vtElevLayer *)l;
		height = pEL->GetElevation(p);
		if (height != INVALID_ELEVATION)
			break;
	}
	return height;
}

bool MainFrame::GetRGBUnderCursor(const DPoint2 &p, RGBi &rgb)
{
	bool success = false;
	int layers = m_Layers.GetSize();
	for (int i = 0; i < layers; i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() != LT_IMAGE || !l->GetVisible()) continue;
		vtImageLayer *pIL = (vtImageLayer *)l;
		if (pIL->GetImage()->GetColorSolid(p, rgb))
			success = true;
	}
	return success;
}

void MainFrame::SetProjection(const vtProjection &p)
{
	char type[7], value[4000];
	p.GetTextDescription(type, value);
	VTLOG("Setting main projection to: %s, %s\n", type, value);

	m_proj = p;

	// inform the world map view
	GetView()->SetWMProj(p);

	// inform the dialogs that care, if they're open
	if (m_pDistanceDlg)
		m_pDistanceDlg->SetProjection(m_proj);
	if (m_pInstanceDlg)
		m_pInstanceDlg->SetProjection(m_proj);
	if (m_pProfileDlg)
		m_pProfileDlg->SetProjection(m_proj);
}

void MainFrame::OnSelectionChanged()
{
	if (m_pFeatInfoDlg && m_pFeatInfoDlg->IsShown())
	{
		vtRawLayer *pRL = GetActiveRawLayer();
		m_pFeatInfoDlg->SetFeatureSet(pRL->GetFeatureSet());
		m_pFeatInfoDlg->ShowSelected();
	}
}


////////////////////////////////////////////////////////////////
// Project operations

void trim_eol(char *buf)
{
	int len = strlen(buf);
	if (len && buf[len-1] == 10) buf[len-1] = 0;
	len = strlen(buf);
	if (len && buf[len-1] == 13) buf[len-1] = 0;
}

void MainFrame::LoadProject(const wxString &strPathName)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	vtString fname = (const char *) strPathName.mb_str(wxConvUTF8);
	VTLOG("Loading project: '%s'\n", (const char *) fname);

	// read project file
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
	{
		DisplayAndLog("Couldn't open project file: '%s'", (const char *) fname);
		return;
	}

	// even the first layer must match the project's CRS
	m_bAdoptFirstCRS = false;

	// avoid trying to draw while we're loading the project
	m_bDrawDisabled = true;

	char buf[2000];
	bool bHasView = false;
	while (fgets(buf, 2000, fp) != NULL)
	{
		if (!strncmp(buf, "Projection ", 11))
		{
			// read projection info
			vtProjection proj;
			char *wkt = buf + 11;
			OGRErr err = proj.importFromWkt(&wkt);
			if (err != OGRERR_NONE)
			{
				DisplayAndLog("Had trouble parsing the projection information"
					"from that file.");
				fclose(fp);
				return;
			}
			SetProjection(proj);
		}
		if (!strncmp(buf, "PlantList ", 10))
		{
			trim_eol(buf);
			LoadSpeciesFile(buf+10);
		}
		if (!strncmp(buf, "BioTypes ", 9))
		{
			trim_eol(buf);
			LoadBiotypesFile(buf+9);
		}
		if (!strncmp(buf, "area ", 5))
		{
			sscanf(buf+5, "%lf %lf %lf %lf\n", &m_area.left, &m_area.top,
				&m_area.right, &m_area.bottom);
		}
		if (!strncmp(buf, "view ", 5))
		{
			DRECT rect;
			sscanf(buf+5, "%lf %lf %lf %lf\n", &rect.left, &rect.top,
				&rect.right, &rect.bottom);
			m_pView->ZoomToRect(rect, 0.0f);
			bHasView = true;
		}
		if (!strncmp(buf, "layers", 6))
		{
			int count = 0;
			LayerType ltype;

			sscanf(buf+7, "%d\n", &count);
			for (int i = 0; i < count; i++)
			{
				bool bShow = true, bImport = false;

				char buf2[200], buf3[200];
				fgets(buf, 200, fp);
				int num = sscanf(buf, "type %d, %s %s", &ltype, buf2, buf3);

				if (!strcmp(buf2, "import"))
					bImport = true;
				if (num > 2 && !strcmp(buf3, "hidden"))
					bShow = false;

				// next line is the path
				fgets(buf, 200, fp);

				// trim trailing LF character
				trim_eol(buf);
				wxString fname(buf, wxConvUTF8);

				int numlayers = NumLayers();
				if (bImport)
					ImportDataFromArchive(ltype, fname, false);
				else
				{
					vtLayer *lp = vtLayer::CreateNewLayer(ltype);
					if (lp && lp->Load(fname))
						AddLayer(lp);
					else
						delete lp;
				}

				// Hide any layers created, if desired
				int newlayers = NumLayers();
				for (int j = numlayers; j < newlayers; j++)
					GetLayer(j)->SetVisible(bShow);
			}
		}
	}
	fclose(fp);

	// reset to default behavior
	m_bAdoptFirstCRS = true;

	// refresh the view
	m_bDrawDisabled = false;
	if (!bHasView)
		ZoomAll();
	RefreshTreeView();
	RefreshToolbars();
}

void MainFrame::SaveProject(const wxString &strPathName) const
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// write project file
	FILE *fp = vtFileOpen(strPathName.mb_str(wxConvUTF8), "wb");
	if (!fp)
		return;

	// write projection info
	char *wkt;
	m_proj.exportToWkt(&wkt);
	fprintf(fp, "Projection %s\n", wkt);
	OGRFree(wkt);

	if (m_strSpeciesFilename != "")
	{
		fprintf(fp, "PlantList %s\n", (const char *) m_strSpeciesFilename);
	}

	if (m_strBiotypesFilename != "")
	{
		fprintf(fp, "BioTypes %s\n", (const char *) m_strBiotypesFilename);
	}

	// write list of layers
	int iLayers = m_Layers.GetSize();
	fprintf(fp, "layers: %d\n", iLayers);

	vtLayer *lp;
	for (int i = 0; i < iLayers; i++)
	{
		lp = m_Layers.GetAt(i);

		bool bNative = lp->IsNative();

		fprintf(fp, "type %d, %s", lp->GetType(), bNative ? "native" : "import");
		if (!lp->GetVisible())
			fprintf(fp, " hidden");
		fprintf(fp, "\n");

		wxString fname = lp->GetLayerFilename();
		if (!bNative)
		{
			if (lp->GetImportedFrom() != _T(""))
				fname = lp->GetImportedFrom();
		}
		fprintf(fp, "%s\n", (const char *) fname.mb_str(wxConvUTF8));
	}

	// write area
	fprintf(fp, "area %lf %lf %lf %lf\n", m_area.left, m_area.top,
		m_area.right, m_area.bottom);

	// write view rectangle
	DRECT rect = m_pView->GetWorldRect();
	fprintf(fp, "view %lf %lf %lf %lf\n", rect.left, rect.top,
		rect.right, rect.bottom);

	// done
	fclose(fp);
}

bool MainFrame::LoadSpeciesFile(const char *fname)
{
	if (!GetPlantList()->ReadXML(fname))
	{
		DisplayAndLog("Couldn't read plant list from file '%s'.", fname);
		return false;
	}
	m_strSpeciesFilename = fname;
	return true;
}

bool MainFrame::LoadBiotypesFile(const char *fname)
{
	if (!m_BioRegion.Read(fname, m_PlantList))
	{
		DisplayAndLog("Couldn't read bioregion list from file '%s'.", fname);
		return false;
	}
	m_strBiotypesFilename = fname;
	return true;
}


#if wxUSE_DRAG_AND_DROP
///////////////////////////////////////////////////////////////////////
// Drag-and-drop functionality
//

bool DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
	size_t nFiles = filenames.GetCount();
	for ( size_t n = 0; n < nFiles; n++ )
	{
		wxString str = filenames[n];
		if (!str.Right(3).CmpNoCase(_T("vtb")))
			GetMainFrame()->LoadProject(str);
		else
			GetMainFrame()->LoadLayer(str);
	}
	return TRUE;
}
#endif

//////////////////////////
// Elevation ops

void MainFrame::ScanElevationLayers(int &count, int &floating, int &tins, DPoint2 &spacing)
{
	count = floating = tins = 0;
	spacing.Set(0,0);
	for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() != LT_ELEVATION)
			continue;

		count++;
		vtElevLayer *el = (vtElevLayer *)l;
		if (el->IsGrid())
		{
			vtElevationGrid *grid = el->m_pGrid;
			if (grid->IsFloatMode() || grid->GetScale() != 1.0f)
				floating++;

			spacing = grid->GetSpacing();
		}
		else
		{
			tins++;
			// All TINs have floating-point precision
			floating++;
		}
	}
}

void MainFrame::MergeResampleElevation()
{
	VTLOG1("MergeResampleElevation\n");

	// If any of the input terrain are floats, then recommend to the user
	// that the output should be float as well.
	bool floatmode = false;

	// sample spacing in meters/heixel or degrees/heixel
	DPoint2 spacing(0, 0);
	int count = 0, floating = 0, tins = 0;
	ScanElevationLayers(count, floating, tins, spacing);
	VTLOG(" Layers: %d, Elevation layers: %d, %d are floating point\n",
		NumLayers(), count, floating);

	if (count == 0)
	{
		DisplayAndLog("Sorry, you must have some elevation grid layers\n"
					  "to perform a sampling operation on them.");
		return;
	}
	if (floating > 0)
		floatmode = true;

	// Always recommend sub-meter precision when sampling TINs
	if (tins > 0)
		floatmode = true;

	if (spacing == DPoint2(0, 0))
	{
		// There were no elevation grids to estimate spacing, so just give 
		//  a default value.
		spacing.Set(1,1);
	}

	// Open the Resample dialog
	ResampleDlg dlg(this, -1, _("Merge and Resample Elevation"));
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.m_area = m_area;
	dlg.m_bFloats = floatmode;
	dlg.m_tileopts = m_tileopts;
	dlg.SetView(GetView());
	dlg.FormatTilingString();

	int ret = dlg.ShowModal();
	GetView()->HideGridMarks();
	if (ret == wxID_CANCEL)
		return;

	// Make new terrain
	vtElevLayer *pOutput = new vtElevLayer(dlg.m_area, dlg.m_iSizeX,
			dlg.m_iSizeY, dlg.m_bFloats, dlg.m_fVUnits, m_proj);

	if (!pOutput->m_pGrid->HasData())
	{
		wxString str;
		str.Printf(_("Failed to initialize %d x %d elevation grid"), dlg.m_iSizeX, dlg.m_iSizeY);
		wxMessageBox(str);
		return;
	}

	// fill in the value for pOutput by merging samples from all other terrain
	if (!SampleCurrentTerrains(pOutput))
	{
		delete pOutput;
		return;
	}
	pOutput->m_pGrid->ComputeHeightExtents();

	if (dlg.m_bFillGaps)
	{
		if (!FillElevGaps(pOutput))
		{
			delete pOutput;
			return;
		}
	}

	if (dlg.m_bNewLayer)
		AddLayerWithCheck(pOutput);
	else if (dlg.m_bToFile)
	{
		OpenProgressDialog(_T("Writing file"), true);

		wxString fname = dlg.m_strToFile;
		bool gzip = (fname.Right(3).CmpNoCase(_T(".gz")) == 0);
		vtString fname_utf8 = (const char *) fname.mb_str(wxConvUTF8);

		bool success = pOutput->m_pGrid->SaveToBT(fname_utf8, progress_callback, gzip);
		delete pOutput;
		CloseProgressDialog();

		if (success)
			DisplayAndLog("Successfully wrote to '%s'", (const char *) fname_utf8);
		else
			DisplayAndLog("Did not successfully write to '%s'", (const char *) fname_utf8);
	}
	else if (dlg.m_bToTiles)
	{
		OpenProgressDialog2(_T("Writing tiles"), true);
		bool success = pOutput->WriteGridOfElevTilePyramids(dlg.m_tileopts, GetView());
		GetView()->HideGridMarks();
		delete pOutput;
		CloseProgressDialog2();
		if (success)
			DisplayAndLog("Successfully wrote to '%s'", (const char *) dlg.m_tileopts.fname);
		else
			DisplayAndLog("Did not successfully write to '%s'", (const char *) dlg.m_tileopts.fname);
	}
}

//////////////////////////////////////////////////////////
// Image ops

void MainFrame::ExportImage()
{
	// sample spacing in meters/heixel or degrees/heixel
	DPoint2 spacing(0, 0);
	for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() == LT_IMAGE)
		{
			vtImageLayer *im = (vtImageLayer *)l;
			spacing = im->GetSpacing();
		}
	}
	if (spacing == DPoint2(0, 0))
	{
		DisplayAndLog(_("Sorry, you must have some image layers to\n perform a sampling operation on them."));
		return;
	}

	// Open the Resample dialog
	SampleImageDlg dlg(this, -1, _("Merge and Resample Imagery"));
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.m_area = m_area;
	dlg.SetView(GetView());
	dlg.m_tileopts = m_tileopts;
	dlg.FormatTilingString();

	int ret = dlg.ShowModal();
	GetView()->HideGridMarks();
	if (ret == wxID_CANCEL)
		return;

	// Make new image
	vtImageLayer *pOutputLayer = new vtImageLayer(dlg.m_area, dlg.m_iSizeX,
			dlg.m_iSizeY, m_proj);
	vtImage *pOutput = pOutputLayer->GetImage();

	if (!pOutput->IsAllocated())
	{
		DisplayAndLog(_("Sorry, could not allocate an image of that size."));
		delete pOutputLayer;
		return;
	}

	// fill in the value for pBig by merging samples from all other terrain
	bool success = SampleCurrentImages(pOutputLayer);
	if (!success)
	{
		delete pOutputLayer;
		return;
	}

	if (dlg.m_bNewLayer)
		AddLayerWithCheck(pOutputLayer);
	else if (dlg.m_bToFile)
	{
		OpenProgressDialog(_T("Writing file"), true);
		vtString fname = (const char *) dlg.m_strToFile.mb_str(wxConvUTF8);
		success = pOutput->SaveToFile(fname);
		delete pOutput;
		CloseProgressDialog();
		if (success)
			DisplayAndLog("Successfully wrote to '%s'", (const char *) fname);
		else
			DisplayAndLog(("Did not successfully write to '%s'."), (const char *) fname);
	}
	else if (dlg.m_bToTiles)
	{
		OpenProgressDialog(_T("Writing tiles"), true);
		bool success = pOutput->WriteGridOfTilePyramids(dlg.m_tileopts, GetView());
		GetView()->HideGridMarks();
		delete pOutput;
		CloseProgressDialog();
		if (success)
			DisplayAndLog("Successfully wrote to '%s'", (const char *) dlg.m_tileopts.fname);
		else
			DisplayAndLog("Did not successfully write to '%s'", (const char *) dlg.m_tileopts.fname);
	}
}


//////////////////////////
// Vegetation ops

/**
 * Generate vegetation in a given area, and writes it to a VF file.
 * All options are given in the VegGenOptions object passed in.
 *
 */
void MainFrame::GenerateVegetation(const char *vf_file, DRECT area,
	VegGenOptions &opt)
{
	OpenProgressDialog(_("Generating Vegetation"), true);

	clock_t time1 = clock();

	vtBioType SingleBiotype;
	if (opt.m_iSingleSpecies != -1)
	{
		// simply use a single species
		vtPlantSpecies *ps = m_PlantList.GetSpecies(opt.m_iSingleSpecies);
		SingleBiotype.AddPlant(ps, opt.m_fFixedDensity);
		opt.m_iSingleBiotype = m_BioRegion.AddType(&SingleBiotype);
	}

	// Create some optimization indices to speed it up
	if (opt.m_pBiotypeLayer)
		opt.m_pBiotypeLayer->CreateIndex(10);
	if (opt.m_pDensityLayer)
		opt.m_pDensityLayer->CreateIndex(10);

	GenerateVegetationPhase2(vf_file, area, opt);

	// Clean up the optimization indices
	if (opt.m_pBiotypeLayer)
		opt.m_pBiotypeLayer->FreeIndex();
	if (opt.m_pDensityLayer)
		opt.m_pDensityLayer->FreeIndex();

	// clean up temporary biotype
	if (opt.m_iSingleSpecies != -1)
	{
		m_BioRegion.m_Types.RemoveAt(opt.m_iSingleBiotype);
	}

	clock_t time2 = clock();
	float time = ((float)time2 - time1)/CLOCKS_PER_SEC;
	VTLOG("GenerateVegetation: %.3f seconds.\n", time);
}

void MainFrame::GenerateVegetationPhase2(const char *vf_file, DRECT area,
	VegGenOptions &opt)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	unsigned int i, j, k;
	DPoint2 p, p2;

	unsigned int x_trees = (unsigned int)(area.Width() / opt.m_fSampling);
	unsigned int y_trees = (unsigned int)(area.Height() / opt.m_fSampling);

	int bio_type=0;
	float density_scale;

	vtPlantInstanceArray pia;
	vtPlantDensity *pd;
	vtBioType *bio;
	vtPlantSpecies *ps;

	// inherit projection from the main frame
	vtProjection proj;
	GetProjection(proj);
	pia.SetProjection(proj);

	m_BioRegion.ResetAmounts();
	pia.SetPlantList(&m_PlantList);

	// Iterate over the whole area, creating plant instances
	for (i = 0; i < x_trees; i ++)
	{
		wxString str;
		str.Printf(_("column %d/%d, plants: %d"), i, x_trees, pia.GetNumEntities());
		if (UpdateProgressDialog(i * 100 / x_trees, str))
		{
			// user cancel
			CloseProgressDialog();
			return;
		}

		p.x = area.left + (i * opt.m_fSampling);
		for (j = 0; j < y_trees; j ++)
		{
			p.y = area.bottom + (j * opt.m_fSampling);

			// randomize the position slightly
			p2.x = p.x + random_offset(opt.m_fSampling * 0.5f);
			p2.y = p.y + random_offset(opt.m_fSampling * 0.5f);

			// Density
			if (opt.m_pDensityLayer)
			{
				density_scale = opt.m_pDensityLayer->FindDensity(p2);
				if (density_scale == 0.0f)
					continue;
			}
			else
				density_scale = 1.0f;

			// Species
			if (opt.m_iSingleSpecies != -1)
			{
				// use our single species biotype
				bio_type = opt.m_iSingleBiotype;
			}
			else
			{
				if (opt.m_iSingleBiotype != -1)
				{
					bio_type = opt.m_iSingleBiotype;
				}
				else if (opt.m_pBiotypeLayer != NULL)
				{
					bio_type = opt.m_pBiotypeLayer->FindBiotype(p2);
					if (bio_type == -1)
						continue;
				}
			}
			// look at veg_type to decide which BioType to use
			bio = m_BioRegion.m_Types[bio_type];

			float square_meters = opt.m_fSampling * opt.m_fSampling;
			float factor = density_scale * square_meters * opt.m_fScarcity;

			// the amount of each species present accumulates until it
			//  exceeds 1, at which time we produce a plant instance
			for (k = 0; k < bio->m_Densities.GetSize(); k++)
			{
				pd = bio->m_Densities[k];

				pd->m_amount += (pd->m_plant_per_m2 * factor);
			}

			ps = NULL;
			for (k = 0; k < bio->m_Densities.GetSize(); k++)
			{
				pd = bio->m_Densities[k];
				if (pd->m_amount > 1.0f)	// time to plant
				{
					pd->m_amount -= 1.0f;
					pd->m_iNumPlanted++;
					ps = pd->m_pSpecies;
					break;
				}
			}
			if (ps == NULL)
				continue;

			// Now determine size
			float size;
			if (opt.m_fFixedSize != -1.0f)
			{
				size = opt.m_fFixedSize;
			}
			else
			{
				float range = opt.m_fRandomTo - opt.m_fRandomFrom;
				size = (opt.m_fRandomFrom + random(range)) * ps->GetMaxHeight();
			}

			// Finally, add the plant
			pia.AddPlant(p2, size, ps);
		}
	}
	pia.WriteVF(vf_file);
	CloseProgressDialog();

	// display a useful message informing the user what was planted
	int unplanted = 0;
	wxString msg, str;
	msg = _("Vegetation distribution results:\n");
	for (i = 0; i < m_BioRegion.m_Types.GetSize(); i++)
	{
		bio = m_BioRegion.m_Types[i];

		int total_this_type = 0;
		for (k = 0; k < bio->m_Densities.GetSize(); k++)
		{
			pd = bio->m_Densities[k];
			total_this_type += pd->m_iNumPlanted;
			unplanted += (int) (pd->m_amount);
		}
		str.Printf(_("  BioType %d"), i);
		msg += str;

		if (total_this_type != 0)
		{
			msg += _T("\n");
			for (k = 0; k < bio->m_Densities.GetSize(); k++)
			{
				pd = bio->m_Densities[k];
				str.Printf(_("    Plant %d: %hs: %d generated.\n"), k,
					(const char *) pd->m_pSpecies->GetCommonName().m_strName, pd->m_iNumPlanted);
				msg += str;
			}
		}
		else
			msg += _(": None.\n");
	}
	DisplayAndLog(msg.mb_str(wxConvUTF8));

	if (unplanted > 0)
	{
		msg.Printf(_T("%d plants were generated that could not be placed.\n"), unplanted);
		msg += _T("Try to decrease your spacing or scarcity, so that\n");
		msg += _T("there are enough places to plant.");
		wxMessageBox(msg, _("Warning"));
	}
}


//////////////////
// Keyboard shortcuts

void MainFrame::OnChar(wxKeyEvent& event)
{
	m_pView->OnChar(event);
}

void MainFrame::OnKeyDown(wxKeyEvent& event)
{
	m_pView->OnChar(event);
}

void MainFrame::OnMouseWheel(wxMouseEvent& event)
{
	m_pView->OnMouseWheel(event);
}

//////////////////////////////

using namespace std;

void MainFrame::ReadEnviroPaths()
{
	VTLOG("Getting data paths from Enviro.\n");
	wxString cwd1 = wxGetCwd();
	vtString cwd = (const char *) cwd1.mb_str(wxConvUTF8);
	VTLOG("  Current directory: '%s'\n", (const char *) cwd);

	vtString fname = cwd + "/Enviro.xml";
	VTLOG("  Looking for '%s'\n", (const char *) fname);
	ifstream input;
	input.open(fname, ios::in | ios::binary);
	if (!input.is_open())
	{
		input.clear();
		fname = cwd + "/../Enviro/Enviro.xml";
		VTLOG("  Not there.  Looking for '%s'\n", (const char *) fname);
		input.open(fname, ios::in | ios::binary);
	}
	if (input.is_open())
	{
		VTLOG1(" found it.\n");
		ReadDatapathsFromXML(input, fname);
		return;
	}
	VTLOG1("  Not found, using default of '../Data/'.\n");
	m_datapaths.push_back(vtString("../Data/"));
}


///////////////////////////////////////////////////////////////////////
// XML format

class EnviroOptionsVisitor : public XMLVisitor
{
public:
	EnviroOptionsVisitor(vtStringArray &paths) : m_paths(paths) {}
	void startElement(const char *name, const XMLAttributes &atts) { m_data = ""; }
	void endElement (const char *name);
	void data(const char *s, int length) { m_data += vtString(s, length); }
protected:
	vtStringArray &m_paths;
	vtString m_data;
};

void EnviroOptionsVisitor::endElement(const char *name)
{
	if (!strcmp(name, "DataPath"))
		m_paths.push_back(m_data);
}

void MainFrame::ReadDatapathsFromXML(ifstream &input, const char *path)
{
	EnviroOptionsVisitor visitor(m_datapaths);
	try
	{
		readXML(input, visitor, path);
	}
	catch (xh_io_exception &ex)
	{
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
	}
}

bool MainFrame::ConfirmValidCRS(vtProjection *pProj)
{
	if (!pProj->GetRoot())
	{
		// No projection.
		int res;
		if (g_Options.GetValueBool(TAG_USE_CURRENT_CRS))
			res = wxNO;		// Don't ask user, just use current CRS
		else
		{
			wxString msg = _("File lacks a projection.\n Would you like to specify one?\n Yes - specify projection\n No - use current projection\n");
			res = wxMessageBox(msg, _("Coordinate Reference System"), wxYES_NO | wxCANCEL);
		}
		if (res == wxYES)
		{
			ProjectionDlg dlg(NULL, -1, _("Please indicate projection"));
			dlg.SetProjection(m_proj);

			if (dlg.ShowModal() == wxID_CANCEL)
				return false;
			dlg.GetProjection(*pProj);
		}
		else if (res == wxNO)
			*pProj = m_proj;
		else if (res == wxCANCEL)
			return false;
	}
	return true;
}

