//
// The main Frame window of the VTBuilder application
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "xmlhelper/exception.hpp"
#include <fstream>

#include "Frame.h"
#include "SplitterWin.h"
#include "TreeView.h"
#include "MenuEnum.h"
#include "App.h"
#include "Helper.h"
#include "BuilderView.h"
#include "vtui/Helper.h"
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
#include "vtui/DistanceDlg.h"
#include "vtui/LinearStructDlg.h"
#include "vtui/InstanceDlg.h"

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

#  include "info.xpm"
#  include "instances.xpm"

#  include "layer_export.xpm"
#  include "layer_import.xpm"
#  include "layer_new.xpm"
#  include "layer_open.xpm"
#  include "layer_save.xpm"
#  include "layer_show.xpm"

#  include "loadimage.xpm"

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

#  include "table.xpm"
#  include "twr_edit.xpm"

#  include "view_hand.xpm"
#  include "view_mag.xpm"
#  include "view_minus.xpm"
#  include "view_plus.xpm"
#  include "view_zoomall.xpm"
#  include "view_zoomexact.xpm"
#  include "view_zoom_layer.xpm"

#	include "VTBuilder.xpm"
#endif

DECLARE_APP(BuilderApp)

// Window ids
#define WID_SPLITTER	100
#define WID_FRAME		101
#define WID_MAINVIEW	102
#define WID_FEATINFO	103
#define WID_DISTANCE	104

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
wxFrame(frame, WID_FRAME, title, pos, size)
{
	VTLOG("  MainFrame constructor: enter\n");

	// init app data
	m_pView = NULL;
	m_pActiveLayer = NULL;
	m_SpeciesListDlg = NULL;
	m_BioRegionDlg = NULL;
	m_pFeatInfoDlg = NULL;
	m_pDistanceDlg = NULL;
	m_pLinearStructureDlg = NULL;
	m_pInstanceDlg = NULL;
	m_szIniFilename = APPNAME ".ini";
	m_bDrawDisabled = false;
	m_bAdoptFirstCRS = true;

	// frame icon
	SetIcon(wxICON(vtbuilder));
	VTLOG("  MainFrame constructor: exit\n");
}

MainFrame::~MainFrame()
{
	VTLOG("Frame destructor\n");
	WriteINI();
	DeleteContents();
}

void MainFrame::CreateView()
{
	m_pView = new BuilderView(m_splitter, WID_MAINVIEW,
			wxPoint(0, 0), wxSize(200, 400), _T("") );
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

	SetDropTarget(new DnDFile());

	// splitter
	m_splitter = new MySplitterWindow(this, WID_SPLITTER);

	m_pTree = new MyTreeCtrl(m_splitter, LayerTree_Ctrl,
			wxPoint(0, 0), wxSize(200, 400),
#ifndef NO_VARIABLE_HEIGHT
			wxTR_HAS_VARIABLE_ROW_HEIGHT |
#endif
			wxNO_BORDER);

	// The following makes the views match, but it looks funny on Linux
//	m_pTree->SetBackgroundColour(*wxLIGHT_GREY);

	CreateView();
	m_pView->SetBackgroundColour(*wxLIGHT_GREY);
	m_pView->Show(FALSE);

	// Read INI file after creating the view
	ReadINI();

	m_splitter->Initialize(m_pTree);

	////////////////////////
	m_pTree->Show(TRUE);
	m_pView->Show(TRUE);
	m_splitter->SplitVertically( m_pTree, m_pView, 200);

	CheckForGDALAndWarn();

	vtProjection proj;
	proj.SetWellKnownGeogCS("WGS84");
	SetProjection(proj);
	RefreshStatusBar();

	// Load structure defaults
	ReadEnviroPaths(m_datapaths);
	bool foundmaterials = LoadGlobalMaterials(m_datapaths);
	if (!foundmaterials)
		DisplayAndLog("The building materials file (Culture/materials.xml) was not found\n"
			" on your Data Path.  Without this file, materials will not be handled\n"
			" correctly.  Please check your Data Paths to avoid this problem.");
	SetupDefaultStructures();

	// Load content files, which might be referenced by structure layers
	LookForContentFiles();

	SetStatusText(_("Ready"));
}

void MainFrame::DeleteContents()
{
	m_Layers.Empty();
	m_pActiveLayer = NULL;
	FreeGlobalMaterials();
	FreeContentFiles();
}

void MainFrame::CheckForGDALAndWarn()
{
	// check for correctly set up environment variables and locatable files
	bool has1 = true, has2 = true, has3 = true;

	const char *gdal = getenv("GEOTIFF_CSV");
	if (!gdal)
	{
		has1 = false;
		has2 = false;
	}
	else
	{
		vtString fname = gdal;
		fname += "/pcs.csv";	// this should always be there
		FILE *fp = fopen((const char *)fname, "rb");
		if (fp)
			fclose(fp);
		else
			has1 = false;
		fname = gdal;
		fname += "/gdal_datum.csv";	// this should be there if data is current
		fp = fopen((const char *)fname, "rb");
		if (fp)
			fclose(fp);
		else
			has2 = false;
	}
	const char *proj4 = getenv("PROJ_LIB");
	if (!proj4)
		has3 = true;
	else
	{
		vtString fname = proj4;
		fname += "/nad83";		// this should always be there
		FILE *fp = fopen((const char *)fname, "rb");
		if (fp)
			fclose(fp);
		else
			has3 = false;
	}
	if (has1 && !has2)
	{
		DisplayAndLog("The GDAL-data on your computer is out of date.  You will need\n"
			" the latest files in order for full coordinate system support.\n"
			" If you don't need full support for coordinate systems\n"
			" including converting between different projections, you can\n"
			" ignore this warning.  Otherwise, get the latest (gdal-data-119.zip)\n"
			" from the VTP website or CD.");
	}
	else if (!has1 || !has3)
	{
		DisplayAndLog("Unable to locate the necessary files for full coordinate\n"
			" system support.  Check that the environment variables GEOTIFF_CSV\n"
			" and PROJ_LIB are set and contain correct paths to the GDAL and PROJ.4\n"
			" data files.  If you don't need full support for coordinate systems\n"
			" including converting between different projections, you can ignore\n"
			" this warning.");
	}
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

void MainFrame::CreateToolbar()
{
	// tool bar
	toolBar_main = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_DOCKABLE);
	toolBar_main->SetMargins(2, 2);
	toolBar_main->SetToolBitmapSize(wxSize(20, 20));

	RefreshToolbar();
}

void MainFrame::RefreshToolbar()
{
	int count = toolBar_main->GetToolsCount();

	// the first time, add the original buttons
	if (count == 0)
	{
		AddMainToolbars();
		m_iMainButtons = toolBar_main->GetToolsCount();
	}

	// remove any existing extra buttons
	while (count > m_iMainButtons)
	{
		toolBar_main->DeleteToolByPos(m_iMainButtons);
		count = toolBar_main->GetToolsCount();
	}

	vtLayer *pLayer = GetActiveLayer();
	LayerType lt = LT_UNKNOWN;
	if (pLayer)
		lt = pLayer->GetType();

	switch (lt)
	{
		case LT_ROAD:
			toolBar_main->AddSeparator();
			ADD_TOOL(ID_ROAD_SELECTROAD, wxBITMAP(rd_select_road), _("Select Roads"), true);
			ADD_TOOL(ID_ROAD_SELECTNODE, wxBITMAP(rd_select_node), _("Select Nodes"), true);
			ADD_TOOL(ID_ROAD_SELECTWHOLE, wxBITMAP(rd_select_whole), _("Select Whole Roads"), true);
			ADD_TOOL(ID_ROAD_DIRECTION, wxBITMAP(rd_direction), _("Set Road Direction"), true);
			ADD_TOOL(ID_ROAD_EDIT, wxBITMAP(rd_edit), _("Edit Road Points"), true);
			ADD_TOOL(ID_ROAD_SHOWNODES, wxBITMAP(rd_shownodes), _("Show Nodes"), true);
			ADD_TOOL(ID_EDIT_CROSSINGSELECTION, wxBITMAP(edit_crossing), _("Crossing Selection"), true);
			break;
		case LT_ELEVATION:
			toolBar_main->AddSeparator();
			ADD_TOOL(ID_ELEV_SELECT, wxBITMAP(select), _("Select Elevation"), true);
			ADD_TOOL(ID_VIEW_FULLVIEW, wxBITMAP(view_zoomexact), _("Zoom to Full Detail"), false);
			ADD_TOOL(ID_AREA_EXPORT_ELEV, wxBITMAP(layer_export), _("Export Elevation"), false);
			break;
		case LT_IMAGE:
			toolBar_main->AddSeparator();
			ADD_TOOL(ID_VIEW_FULLVIEW, wxBITMAP(view_zoomexact), _("Zoom to Full Detail"), false);
			break;
		case LT_STRUCTURE:
			toolBar_main->AddSeparator();
			ADD_TOOL(ID_FEATURE_SELECT, wxBITMAP(select), _("Select Features"), true);
			ADD_TOOL(ID_STRUCTURE_EDIT_BLD, wxBITMAP(bld_edit), _("Edit Buildings"), true);
			ADD_TOOL(ID_STRUCTURE_ADD_POINTS, wxBITMAP(bld_add_points), _("Add points to building footprints"), true);
			ADD_TOOL(ID_STRUCTURE_DELETE_POINTS, wxBITMAP(bld_delete_points), _("Delete points from building footprints"), true);
			ADD_TOOL(ID_STRUCTURE_ADD_LINEAR, wxBITMAP(str_add_linear), _("Add Linear Structures"), true);
			ADD_TOOL(ID_STRUCTURE_EDIT_LINEAR, wxBITMAP(str_edit_linear), _("Edit Linear Structures"), true);
			ADD_TOOL(ID_STRUCTURE_CONSTRAIN, wxBITMAP(bld_corner), _("Constrain Angles"), true);
			ADD_TOOL(ID_STRUCTURE_ADD_INST, wxBITMAP(instances), _("Add Instances"), true);
			break;
		case LT_UTILITY:
			toolBar_main->AddSeparator();
			ADD_TOOL(ID_TOWER_ADD,wxBITMAP(rd_select_node), _("Add Tower"), true);
			toolBar_main->AddSeparator();
			ADD_TOOL(ID_TOWER_SELECT,wxBITMAP(select),_("Select Towers"), true);
			ADD_TOOL(ID_TOWER_EDIT, wxBITMAP(twr_edit), _("Edit Towers"),true);
			break;
		case LT_RAW:
			toolBar_main->AddSeparator();
			ADD_TOOL(ID_FEATURE_SELECT, wxBITMAP(select), _("Select Features"), true);
			ADD_TOOL(ID_FEATURE_PICK, wxBITMAP(info), _("Pick Features"), true);
			ADD_TOOL(ID_FEATURE_TABLE, wxBITMAP(table), _("Table"), true);
			ADD_TOOL(ID_RAW_ADDPOINTS, wxBITMAP(raw_add_point), _("Add Points with Mouse"), true);
	}
	toolBar_main->Realize();

	m_pMenuBar->EnableTop(m_iLayerMenu[LT_ELEVATION], lt == LT_ELEVATION);
#ifndef ELEVATION_ONLY
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_ROAD], lt == LT_ROAD);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_UTILITY], lt == LT_UTILITY);
//	m_pMenuBar->EnableTop(m_iLayerMenu[LT_VEG], lt == LT_VEG);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_STRUCTURE], lt == LT_STRUCTURE);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_RAW], lt == LT_RAW);
#endif
}

void MainFrame::AddMainToolbars()
{
	ADD_TOOL(ID_FILE_NEW, wxBITMAP(proj_new), _("New Project"), false);
	ADD_TOOL(ID_FILE_OPEN, wxBITMAP(proj_open), _("Open Project"), false);
	ADD_TOOL(ID_FILE_SAVE, wxBITMAP(proj_save), _("Save Project"), false);
	toolBar_main->AddSeparator();
	ADD_TOOL(ID_LAYER_NEW, wxBITMAP(layer_new), _("New Layer"), false);
	ADD_TOOL(ID_LAYER_OPEN, wxBITMAP(layer_open), _("Open Layer"), false);
	ADD_TOOL(ID_LAYER_SAVE, wxBITMAP(layer_save), _("Save Layer"), false);
	ADD_TOOL(ID_LAYER_IMPORT, wxBITMAP(layer_import), _("Import Data"), false);
	toolBar_main->AddSeparator();
	ADD_TOOL(ID_EDIT_DELETE, wxBITMAP(edit_delete), _("Delete"), false);
	ADD_TOOL(ID_EDIT_OFFSET, wxBITMAP(edit_offset), _("Offset"), false);
	ADD_TOOL(ID_VIEW_SHOWLAYER, wxBITMAP(layer_show), _("Layer Visibility"), true);
	toolBar_main->AddSeparator();
	ADD_TOOL(ID_VIEW_ZOOMIN, wxBITMAP(view_plus), _("Zoom In"), false);
	ADD_TOOL(ID_VIEW_ZOOMOUT, wxBITMAP(view_minus), _("Zoom Out"), false);
	ADD_TOOL(ID_VIEW_ZOOMALL, wxBITMAP(view_zoomall), _("Zoom All"), false);
	ADD_TOOL(ID_VIEW_ZOOM_LAYER, wxBITMAP(view_zoom_layer), _("Zoom To Layer"), false);
	toolBar_main->AddSeparator();
	ADD_TOOL(ID_VIEW_MAGNIFIER, wxBITMAP(view_mag), _("Magnifier"), true);
	ADD_TOOL(ID_VIEW_PAN, wxBITMAP(view_hand), _("Pan"), true);
	ADD_TOOL(ID_VIEW_DISTANCE, wxBITMAP(distance), _("Distance"), true);
	ADD_TOOL(ID_VIEW_SETAREA, wxBITMAP(elev_box), _("Area Tool"), true);
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

	bool bFirst = (m_Layers.GetSize() == 0);

	vtLayer *pLayer = NULL;
	if (ext.CmpNoCase(_T("rmf")) == 0)
	{
		vtRoadLayer *pRL = new vtRoadLayer();
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
		vtElevLayer *pEL = new vtElevLayer();
		if (pEL->Load(fname))
			pLayer = pEL;
		else
			delete pEL;
	}
#if SUPPORT_TRANSIT
	if (ext.CmpNoCase(_T("xml")) == 0)
	{
		vtTransitLayer *pTL = new vtTransitLayer();
		if (pTL->Load(fname))
			pLayer = pTL;
	}
#endif
	if (ext.CmpNoCase(_T("vtst")) == 0 ||
		fname.Right(8).CmpNoCase(_T(".vtst.gz")) == 0)
	{
		vtStructureLayer *pSL = new vtStructureLayer();
		if (pSL->Load(fname))
			pLayer = pSL;
		else
			delete pSL;
	}
	if (ext.CmpNoCase(_T("vf")) == 0)
	{
		vtVegLayer *pVL = new vtVegLayer();
		if (pVL->Load(fname))
			pLayer = pVL;
		else
			delete pVL;
	}
	if (ext.CmpNoCase(_T("utl")) == 0)
	{
		vtUtilityLayer *pTR = new vtUtilityLayer();
		if(pTR->Load(fname))
			pLayer = pTR;
		else
			delete pTR;
	}
	if (ext.CmpNoCase(_T("shp")) == 0 ||
			ext.CmpNoCase(_T("gml")) == 0 ||
			ext.CmpNoCase(_T("xml")) == 0)
	{
		vtRawLayer *pRL = new vtRawLayer();
		if (pRL->Load(fname))
			pLayer = pRL;
		else
			delete pRL;
 	}
	if (ext.CmpNoCase(_T("tif")) == 0 ||
		ext.CmpNoCase(_T("img")) == 0)
	{
		vtImageLayer *pIL = new vtImageLayer();
		if (pIL->Load(fname))
			pLayer = pIL;
		else
			delete pIL;
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
			msg.Printf(_("The data already loaded is in:\n     %hs\n but the layer you are attempting to add:\n     %s\n is using:\n     %hs\n Would you like to attempt to convert it now to the existing projection?"),
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
				bool success = pLayer->TransformCoords(m_proj);
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
		RefreshToolbar();
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
	DeleteLayer(lp);
	m_pView->Refresh();
	m_pTree->RefreshTreeItems(this);
	RefreshToolbar();
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

//
// read / write ini file
//
bool MainFrame::ReadINI()
{
	m_fpIni = fopen(m_szIniFilename, "rb+");

	if (m_fpIni)
	{
		int ShowMap, ShowElev, Shading, DoMask, DoUTM, ShowPaths, DrawWidth, CastShadows;
		fscanf(m_fpIni, "%d %d %d %d %d %d %d %d", &ShowMap, &ShowElev, &Shading,
			&DoMask, &DoUTM, &ShowPaths, &DrawWidth, &CastShadows);

		m_pView->SetShowMap(ShowMap != 0);
		vtElevLayer::m_draw.m_bShowElevation = (ShowElev != 0);
		vtElevLayer::m_draw.m_bShading = (Shading != 0);
		vtElevLayer::m_draw.m_bDoMask = (DoMask != 0);
		vtElevLayer::m_draw.m_bCastShadows = (CastShadows != 0);
		m_pView->m_bShowUTMBounds = (DoUTM != 0);
		m_pTree->SetShowPaths(ShowPaths != 0);
		vtRoadLayer::SetDrawWidth(DrawWidth != 0);

		return true;
	}
	m_fpIni = fopen(m_szIniFilename, "wb");
	return false;
}

bool MainFrame::WriteINI()
{
	if (m_fpIni)
	{
		rewind(m_fpIni);
		fprintf(m_fpIni, "%d %d %d %d %d %d %d %d", m_pView->GetShowMap(),
			vtElevLayer::m_draw.m_bShowElevation,
			vtElevLayer::m_draw.m_bShading, vtElevLayer::m_draw.m_bDoMask,
			m_pView->m_bShowUTMBounds, m_pTree->GetShowPaths(),
			vtRoadLayer::GetDrawWidth(), vtElevLayer::m_draw.m_bCastShadows);
		fclose(m_fpIni);
		m_fpIni = NULL;
		return true;
	}
	return false;
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

void MainFrame::StretchArea()
{
	m_area = GetExtents();
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
		m_pFeatInfoDlg = new FeatInfoDlg(this, WID_FEATINFO, _("Feature Info"),
				wxPoint(120, 80), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pFeatInfoDlg->SetView(GetView());
	}
	m_pFeatInfoDlg->Show(true);
	return m_pFeatInfoDlg;
}


DistanceDlg	*MainFrame::ShowDistanceDlg()
{
	if (!m_pDistanceDlg)
	{
		// Create new Distance Dialog
		m_pDistanceDlg = new DistanceDlg(this, WID_DISTANCE, _("Distance Tool"),
				wxPoint(120, 80), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pDistanceDlg->SetProjection(m_proj);
	}
	m_pDistanceDlg->Show(true);
	return m_pDistanceDlg;
}

class LinearStructureDlg2d: public LinearStructureDlg
{
public:
	LinearStructureDlg2d(wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos, const wxSize& size, long style) :
	LinearStructureDlg(parent, id, title, pos, size, style) {}
	void OnSetOptions(LinStructOptions &opt)
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
			inst->m_pItem = item;
			break;
		}
	}
}


//
// sample all elevation layers into this one
//
bool MainFrame::SampleCurrentTerrains(vtElevLayer *pTarget)
{
	// measure time
	clock_t tm1 = clock();

	DRECT area;
	pTarget->GetExtent(area);
	DPoint2 step = pTarget->m_pGrid->GetSpacing();

	int i, j, l, layers = m_Layers.GetSize();
	float fData, fBestData;
	int iColumns, iRows;
	pTarget->m_pGrid->GetDimensions(iColumns, iRows);

	// Create progress dialog for the slow part
	OpenProgressDialog(_("Merging and Resampling Elevation Layers"), true);

	int num_elev = LayersOfType(LT_ELEVATION);
	vtElevLayer **elevs = new vtElevLayer *[num_elev];
	int g, num_elevs = 0;
	for (l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_ELEVATION)
			elevs[num_elevs++] = (vtElevLayer *)lp;
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
			fBestData = INVALID_ELEVATION;
			for (g = 0; g < num_elevs; g++)
			{
				vtElevLayer *elev = elevs[g];

				vtElevationGrid *grid = elev->m_pGrid;
				vtTin2d *tin = elev->m_pTin;
				if (grid)
					fData = grid->GetFilteredValue2(p);
				else if (tin)
					tin->FindAltitudeAtPoint2(p, fData);

				if (fData != INVALID_ELEVATION)
					fBestData = fData;
			}
			pTarget->m_pGrid->SetFValue(i, j, fBestData);
		}
	}
	CloseProgressDialog();
	delete [] elevs;

	clock_t tm2 = clock();
	float time = ((float)tm2 - tm1)/CLOCKS_PER_SEC;
	VTLOG("SampleCurrentTerrains: %.3f seconds.\n", time);

	return true;
}


//
// sample all image data into this one
//
bool MainFrame::SampleCurrentImages(vtImageLayer *pTarget)
{
	DRECT area;
	pTarget->GetExtent(area);
	DPoint2 step = pTarget->GetSpacing();

	double x, y;
	int i, j, l, layers = m_Layers.GetSize();
	int iColumns, iRows;
	pTarget->GetDimensions(iColumns, iRows);

	// Create progress dialog for the slow part
	OpenProgressDialog(_("Merging and Resampling Image Layers"), true);

	vtImageLayer **images = new vtImageLayer *[LayersOfType(LT_IMAGE)];
	int g, num_image = 0;
	for (l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_IMAGE)
			images[num_image++] = (vtImageLayer *)lp;
	}

	// iterate through the pixels of the new image
	RGBi rgb;
	bool bHit;
	for (j = 0; j < iRows; j++)
	{
		if (UpdateProgressDialog(j*100/iRows))
		{
			// Cancel
			CloseProgressDialog();
			return false;
		}
		y = area.bottom + (j * step.y);

		for (i = 0; i < iColumns; i++)
		{
			x = area.left + (i * step.x);

			// find some data for this point
			rgb.Set(0,0,0);
			for (g = 0; g < num_image; g++)
			{
				vtImageLayer *image = images[g];

				bHit = image->GetFilteredColor(x, y, rgb);
				if (bHit)
					break;
			}
			pTarget->SetRGB(i, iRows-1-j, rgb.r, rgb.g, rgb.b);
		}
	}
	CloseProgressDialog();
	delete images;
	return true;
}


float MainFrame::GetHeightFromTerrain(DPoint2 &p)
{
	float height = INVALID_ELEVATION;

	int layers = m_Layers.GetSize();
	for (int i = 0; i < layers; i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() != LT_ELEVATION) continue;
		vtElevLayer *pEL = (vtElevLayer *)l;
		height = pEL->GetElevation(p);
		if (height != INVALID_ELEVATION)
			break;
	}
	return height;
}

void MainFrame::SetProjection(const vtProjection &p)
{
	char type[7], value[4000];
	p.GetTextDescription(type, value);
	VTLOG("Setting main projection to: %s, %s\n", type, value);

	m_proj = p;
	GetView()->SetWMProj(p);
	if (m_pDistanceDlg)
		m_pDistanceDlg->SetProjection(m_proj);
	if (m_pInstanceDlg)
		m_pInstanceDlg->SetProjection(m_proj);
}

void MainFrame::OnSelectionChanged()
{
	if (m_pFeatInfoDlg && m_pFeatInfoDlg->IsShown())
		m_pFeatInfoDlg->ShowSelected();
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

void MainFrame::LoadProject(const wxString2 &strPathName)
{
	VTLOG("Loading project: '%s'\n", strPathName.mb_str());

	// read project file
	FILE *fp = fopen(strPathName.mb_str(), "rb");
	if (!fp)
	{
		DisplayAndLog("Couldn't open project file: '%s'", strPathName.mb_str());
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
				wxString2 fname = buf;

				int numlayers = NumLayers();
				if (bImport)
					ImportDataFromArchive(ltype, fname, false);
				else
				{
					vtLayer *lp = vtLayer::CreateNewLayer(ltype);
					if (lp->Load(fname))
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
	RefreshToolbar();
}

void MainFrame::SaveProject(const wxString2 &strPathName)
{
	// write project file
	wxString2 str = strPathName;
	FILE *fp = fopen(str.mb_str(), "wb");
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

		wxString2 fname = lp->GetLayerFilename();
		if (!bNative)
		{
			if (lp->GetImportedFrom() != _T(""))
				fname = lp->GetImportedFrom();
		}
		fprintf(fp, "%s\n", fname.mb_str());
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
	if (!m_BioRegions.Read(fname))
	{
		DisplayAndLog("Couldn't read bioregion list from file '%s'.", fname);
		return false;
	}
	m_strBiotypesFilename = fname;
	return true;
}


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


//////////////////////////
// Elevation ops

void MainFrame::ExportElevation()
{
	// If any of the input terrain are floats, then recommend to the user
	// that the output should be float as well.
	bool floatmode = false;

	// sample spacing in meters/heixel or degrees/heixel
	DPoint2 spacing(0, 0);
	for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() == LT_ELEVATION)
		{
			vtElevLayer *el = (vtElevLayer *)l;
			if (el->IsGrid())
			{
				vtElevationGrid *grid = el->m_pGrid;
				if (grid->IsFloatMode() || grid->GetScale() != 1.0f)
					floatmode = true;
				spacing = grid->GetSpacing();
			}
		}
	}
	if (spacing == DPoint2(0, 0))
	{
		DisplayAndLog("Sorry, you must have some elevation grid layers\n"
					  "to perform a sampling operation on them.");
		return;
	}

	// Open the Resample dialog
	ResampleDlg dlg(this, -1, _("Merge and Resample Elevation"));
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.m_area = m_area;
	dlg.m_bFloats = floatmode;

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	// Make new terrain
	vtElevLayer *pOutput = new vtElevLayer(dlg.m_area, dlg.m_iSizeX,
			dlg.m_iSizeY, dlg.m_bFloats, dlg.m_fVUnits, m_proj);

	// fill in the value for pBig by merging samples from all other terrain
	if (!SampleCurrentTerrains(pOutput))
	{
		delete pOutput;
		return;
	}
	pOutput->m_pGrid->ComputeHeightExtents();
	if (!pOutput->FillGaps())
	{
		delete pOutput;
		return;
	}

	if (dlg.m_bToFile)
	{
		wxString2 fname = dlg.m_strToFile;
		bool gzip = (fname.Right(3).CmpNoCase(_T(".gz")) == 0);
		bool success = pOutput->m_pGrid->SaveToBT(fname.mb_str(), NULL, gzip);
		if (success)
			DisplayAndLog("Successfully wrote to file '%s'", fname.mb_str());
		else
			DisplayAndLog("Couldn't open file for writing.");
		delete pOutput;
	}
	else
		AddLayerWithCheck(pOutput);
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

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString filter = _("All Files|*.*|");
	AddType(filter, FSTRING_TIF);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Export Image"), _T(""), _T(""), filter, wxSAVE);
	saveFile.SetFilterIndex(1);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString2 strPathName = saveFile.GetPath();

	// Make new image
	vtImageLayer *pOutput = new vtImageLayer(dlg.m_area, dlg.m_iSizeX,
			dlg.m_iSizeY, m_proj);
	pOutput->SetLayerFilename(strPathName);

	// fill in the value for pBig by merging samples from all other terrain
	bool success = SampleCurrentImages(pOutput);
	if (success)
	{
		success = pOutput->SaveToFile(strPathName.mb_str());
		if (success)
			DisplayAndLog("Successfully wrote image file '%s'", strPathName.mb_str());
		else
			DisplayAndLog(("Couldn't write image file."));
	}
	delete pOutput;
}


//////////////////////////
// Vegetation ops

void MainFrame::FindVegLayers(vtVegLayer **Density, vtVegLayer **BioMap)
{
	for (unsigned int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *lp = m_Layers.GetAt(i);
		if (lp->GetType() == LT_VEG)
		{
			vtVegLayer *veg = (vtVegLayer *)lp;
			VegLayerType vltype = veg->GetVegType();

			if (vltype == VLT_Density)
				*Density = veg;
			if (vltype == VLT_BioMap)
				*BioMap = veg;
		}
	}
}

/**
 * Generate vegetation in a given area, given input layers for biotypes
 * and density.
 * Density is now optional, defaults to 1 if there is no density layer.
 */
void MainFrame::GenerateVegetation(const char *vf_file, DRECT area, 
	vtVegLayer *pDensity, vtVegLayer *pVegType,
	float fTreeSpacing, float fTreeScarcity)
{
	unsigned int i, j, k;
	DPoint2 p, p2;

	vtSpeciesList *pl = GetPlantList();
	if (!pl)
	{
		wxMessageBox(_("No plant list."));
		return;
	}

	for (i = 0; i < m_BioRegions.m_Types.GetSize(); i++)
		m_PlantList.LookupPlantIndices(m_BioRegions.m_Types[i]);

	OpenProgressDialog(_("Generating Vegetation"), true);

	int tree_count = 0;

	float x_size = (area.right - area.left);
	float y_size = (area.top - area.bottom);
	unsigned int x_trees = (unsigned int)(x_size / fTreeSpacing);
	unsigned int y_trees = (unsigned int)(y_size / fTreeSpacing);

	int bio_type;
	float density_scale;

	vtPlantInstanceArray pia;
	vtPlantDensity *pd;
	vtBioType *bio;

	m_BioRegions.ResetAmounts();
	pia.SetPlantList(pl);

	// inherit projection from the main frame
	vtProjection proj;
	GetProjection(proj);
	pia.SetProjection(proj);

	// all vegetation
	for (i = 0; i < x_trees; i ++)
	{
		wxString str;
		str.Printf(_("plants: %d"), pia.GetNumEntities());
		if (UpdateProgressDialog(i * 100 / x_trees, str))
		{
			// user cancel
			CloseProgressDialog();
			return;
		}

		p.x = area.left + (i * fTreeSpacing);
		for (j = 0; j < y_trees; j ++)
		{
			p.y = area.bottom + (j * fTreeSpacing);

			// randomize the position slightly
			p2.x = p.x + random_offset(fTreeSpacing * 0.5f);
			p2.y = p.y + random_offset(fTreeSpacing * 0.5f);

			// Get Land Use Attribute
			if (pDensity)
			{
				density_scale = pDensity->FindDensity(p2);
				if (density_scale == 0.0f)
					continue;
			}
			else
				density_scale = 1.0f;

			bio_type = pVegType->FindBiotype(p2);
			if (bio_type == -1)
				continue;

			float square_meters = fTreeSpacing * fTreeSpacing;

			// look at veg_type to decide which BioType to use
			bio = m_BioRegions.m_Types[bio_type];

			float factor = density_scale * square_meters * fTreeScarcity;

			for (k = 0; k < bio->m_Densities.GetSize(); k++)
			{
				pd = bio->m_Densities[k];

				pd->m_amount += (pd->m_plant_per_m2 * factor);
			}
			int species_num = -1;
			for (k = 0; k < bio->m_Densities.GetSize(); k++)
			{
				pd = bio->m_Densities[k];
				if (pd->m_amount > 1.0f)	// time to plant
				{
					pd->m_amount -= 1.0f;
					pd->m_iNumPlanted++;
					species_num = pd->m_list_index;
					break;
				}
			}
			if (species_num != -1)
			{
				vtPlantSpecies *ps = GetPlantList()->GetSpecies(species_num);
				pia.AddPlant(p2, random(ps->GetMaxHeight()), species_num);
			}
		}
	}
	pia.WriteVF(vf_file);
	CloseProgressDialog();

	// display a useful message informing the user what was planted
	wxString2 msg, str;
	msg = _("Vegetation distribution results:\n");
	for (i = 0; i < m_BioRegions.m_Types.GetSize(); i++)
	{
		bio = m_BioRegions.m_Types[i];

		int total_this_type = 0;
		for (k = 0; k < bio->m_Densities.GetSize(); k++)
		{
			pd = bio->m_Densities[k];
			total_this_type += pd->m_iNumPlanted;
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
					(const char *) pd->m_common_name, pd->m_iNumPlanted);
				msg += str;
			}
		}
		else
			msg += _(": None.\n");
	}
	DisplayAndLog(msg.mb_str());
}


//////////////////
// Keyboard shortcuts

void MainFrame::OnChar(wxKeyEvent& event)
{
	m_pView->OnChar(event);
}


//////////////////////////////

using namespace std;

#define STR_DATAPATH "DataPath"

void MainFrame::ReadEnviroPaths(vtStringArray &paths)
{
	VTLOG("Getting data paths from Enviro.\n");
	wxString2 IniPath = wxGetCwd();

	ifstream input;
	IniPath += _T("/Enviro.ini");
	VTLOG("  Looking for '%s'\n", IniPath.mb_str());
	input.open(IniPath.mb_str(), ios::in | ios::binary);
	if (!input.is_open())
	{
		input.clear();
		IniPath = wxGetCwd() + _T("/../Enviro/Enviro.ini");
		VTLOG("  Not there.  Looking for '%s'\n", IniPath.mb_str());
		input.open(IniPath.mb_str(), ios::in | ios::binary);
	}
	if (!input.is_open())
	{
		VTLOG("  Not found.\n");
		return;
	}
	VTLOG(" found it.\n");

	char buf[80];
	while (!input.eof())
	{
		if (input.peek() == '\n')
			input.ignore();
		input >> buf;

		// data value should been separated by a tab or space
		int next = input.peek();
		if (next != '\t' && next != ' ')
			continue;
		while (input.peek() == '\t' || input.peek() == ' ')
			input.ignore();

		if (strcmp(buf, STR_DATAPATH) == 0)
		{
			vtString string = get_line_from_stream(input);
			paths.push_back(vtString(string));
		}
	}
	VTLOG("Datapaths:\n");
	int i, n = paths.size();
	if (n == 0)
		VTLOG("   none.\n");
	for (i = 0; i < n; i++)
	{
		VTLOG("   %s\n", (const char *) paths[i]);
	}
}

