//
// The main Frame window of the VTBuilder application
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"
#include "vtdata/Unarchive.h"
#include "vtdata/boost/directory.h"

#include "Frame.h"
#include "SplitterWin.h"
#include "TreeView.h"
#include "MenuEnum.h"
#include "App.h"
#include "Helper.h"
#include "BuilderView.h"
// Layers
#include "ElevLayer.h"
#include "VegLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
// Dialogs
#include "ResampleDlg.h"
#include "FeatInfoDlg.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__)
#  include "bld_edit.xpm"
#  include "distance.xpm"
#  include "edit_crossing.xpm"
#  include "edit_delete.xpm"
#  include "edit_offset.xpm"
#  include "elev_box.xpm"

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

#  include "twr_edit.xpm"

#  include "view_hand.xpm"
#  include "view_mag.xpm"
#  include "view_minus.xpm"
#  include "view_plus.xpm"
#  include "view_zoomall.xpm"
#  include "view_zoomexact.xpm"

#	include "VTBuilder.xpm"
#endif

DECLARE_APP(MyApp)

// Window ids
#define WID_SPLITTER	100
#define WID_FRAME		101
#define WID_MAINVIEW	102
#define WID_FEATINFO	103


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
	// init app data
	m_pView = NULL;
	m_pActiveLayer = NULL;
	m_PlantListDlg = NULL;
	m_BioRegionDlg = NULL;
	m_pFeatInfoDlg = NULL;

	// frame icon
	SetIcon(wxICON(vtbuilder));
}

void MainFrame::CreateView()
{
	m_pView = new BuilderView(m_splitter, WID_MAINVIEW,
			wxPoint(0, 0), wxSize(200, 400), "MainView" );
}

void MainFrame::SetupUI()
{
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
	m_pTree->SetBackgroundColour(*wxLIGHT_GREY);

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

	vtProjection proj;
	proj.SetWellKnownGeogCS("WGS84");
	SetProjection(proj);
	RefreshStatusBar();
}

MainFrame::~MainFrame()
{
	WriteINI();
	DeleteContents();
}

void MainFrame::DeleteContents()
{
	m_Layers.Empty();
	m_pActiveLayer = NULL;
}

void MainFrame::OnSize(wxSizeEvent& event)
{
	int foo = 1;
	wxFrame::OnSize(event);
}

void MainFrame::CreateToolbar()
{
	// tool bar
	toolBar_main = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_DOCKABLE);
	toolBar_main->SetMargins(2, 2);
	toolBar_main->SetToolBitmapSize(wxSize(20, 20));

	RefreshToolbar();
}

#define NUM_MAIN_TOOLBARS 21

void MainFrame::RefreshToolbar()
{
	int count = toolBar_main->GetToolsCount();
	// remove any existing buttons
	while (count > NUM_MAIN_TOOLBARS)
	{
		toolBar_main->DeleteToolByPos(NUM_MAIN_TOOLBARS);
		count = toolBar_main->GetToolsCount();
	}

	// now add new buttons
	if (count < NUM_MAIN_TOOLBARS)
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
		toolBar_main->AddSeparator();
		ADD_TOOL(ID_VIEW_MAGNIFIER, wxBITMAP(view_mag), _("Magnifier"), true);
		ADD_TOOL(ID_VIEW_PAN, wxBITMAP(view_hand), _("Pan"), true);
		ADD_TOOL(ID_VIEW_DISTANCE, wxBITMAP(distance), _("Distance"), true);
		ADD_TOOL(ID_VIEW_SETAREA, wxBITMAP(elev_box), _("Set Export Area"), true);
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
			ADD_TOOL(ID_STRUCTURE_ADD_LINEAR, wxBITMAP(str_add_linear), _("Add Linear Features"), true);
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
			ADD_TOOL(ID_FEATURE_INFO, wxBITMAP(info), _("Feature Info"), true);
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


// Helper
void GetTempFolderName(char *path, const char *base)
{
	const char *tmp = base;
	const char *tmp1 = strrchr(base, '/');
	if (tmp1)
		tmp = tmp1+1;
	const char *tmp2 = strrchr(base, '\\');
	if (tmp2 && tmp2 > tmp)
		tmp = tmp2+1;
	const char *tmp3 = strrchr(base, ':');
	if (tmp3 && tmp3 > tmp)
		tmp = tmp3+1;

	const char *temp = getenv("TEMP");
	if (temp)
		strcpy(path, temp);
	else
		strcpy(path, "C:/TEMP");
	strcat(path, "/");
	strcat(path, tmp);
	strcat(path, "_temp");
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

	using namespace boost::filesystem;

	// check if it's an archive
	bool bExpandedArchive = false;
	char prepend_path[1024];
	GetTempFolderName(prepend_path, fname_in);

	if (ext.CmpNoCase("gz") == 0 || ext.CmpNoCase("tgz") == 0 ||
		ext.CmpNoCase("tar") == 0)
	{
		// try to uncompress
		const char *input_filename = fname;

		int result;
		result = vtCreateDir(prepend_path);
		if (result == 0 && errno != EEXIST)
		{
			wxMessageBox("Couldn't create temporary directory to hold contents of archive.");
			return;
		}
		strcat(prepend_path, "/");

		result = ExpandTGZ(input_filename, prepend_path);
		if (result < 1)
		{
			wxMessageBox("Couldn't expand archive.");
			return;
		}
		else if (result == 1)
		{
			// the archive contained a single file
			for (dir_it it(prepend_path); it != dir_it(); ++it)
			{
				if (get<is_directory>(it))
					continue;
				std::string name1 = *it;
				fname = prepend_path;
				fname += name1.c_str();
				break;
			}
		}
		else if (result > 1)
		{
			// probably SDTS
			// try to guess layer type from original file name
			if (fname.Contains(".hy") || fname.Contains(".HY"))
				ltype = LT_WATER;
			if (fname.Contains(".rd") || fname.Contains(".RD"))
				ltype = LT_ROAD;
			if (fname.Contains(".dem") || fname.Contains(".DEM"))
				ltype = LT_ELEVATION;
			if (fname.Contains(".ms") || fname.Contains(".MS"))
				ltype = LT_STRUCTURE;

			// look for the catalog file
			bool found = false;
			for (dir_it it(prepend_path); it != dir_it(); ++it)
			{
				std::string name1 = *it;
				wxString fname2 = name1.c_str();
				if (fname2.Right(8).CmpNoCase("catd.ddf") == 0)
				{
					fname = prepend_path;
					fname += fname2;
					found= true;
					break;
				}
			}
			if (!found)
			{
				wxMessageBox("Don't know what to do with contents of archive.");
				return;
			}
		}

		// extension has certainly changed
		wxString ext = fname.AfterLast('.');

		bExpandedArchive = true;
	}

	vtLayer *pLayer = NULL;
	if (ext.CmpNoCase("rmf") == 0)
	{
		vtRoadLayer *pRL = new vtRoadLayer();
		if (pRL->Load(fname))
			pLayer = pRL;
	}
	if (ext.CmpNoCase("bt") == 0 || ext.CmpNoCase("tin") == 0)
	{
		vtElevLayer *pEL = new vtElevLayer();
		if (pEL->Load(fname))
			pLayer = pEL;
	}
#if SUPPORT_TRANSIT
	if (ext.CmpNoCase("xml") == 0)
	{
		vtTransitLayer *pTL = new vtTransitLayer();
		if (pTL->Load(fname))
			pLayer = pTL;
	}
#endif
	if (ext.CmpNoCase("vtst") == 0)
	{
		vtStructureLayer *pSL = new vtStructureLayer();
		if (pSL->Load(fname))
			pLayer = pSL;
	}
	if (ext.CmpNoCase("vf") == 0)
	{
		vtVegLayer *pVL = new vtVegLayer();
		if (pVL->Load(fname))
			pLayer = pVL;
	}
	if (ext.CmpNoCase("utl") == 0)
	{
		vtUtilityLayer *pTR = new vtUtilityLayer();
		if(pTR->Load(fname))
			pLayer = pTR;
	}
	if (ext.CmpNoCase("shp") == 0)
	{
		vtRawLayer *pRL = new vtRawLayer();
		if (pRL->Load(fname))
			pLayer = pRL;
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
		ImportDataFromFile(ltype, fname, true);
	}

	if (bExpandedArchive)
	{
		// clean up after ourselves
		GetTempFolderName(prepend_path, fname_in);
		vtDestroyDir(prepend_path);
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
	if (bFirst)
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
			msg.Printf("The data already loaded is in:\n     %s\n"
					"but the layer you are attempting to add:\n     %s\n"
					"is using:\n     %s\n"
					"Would you like to attempt to convert it now to the existing projection?",
				str1,
				(const char *) pLayer->GetFilename(),
				str2);
			int ret = wxMessageBox(msg, "Warning", wxYES_NO | wxCANCEL);
			if (ret == wxNO)
				keep = true;
			if (ret == wxYES)
			{
				bool success = pLayer->ConvertProjection(m_proj);
				if (success)
					keep = true;
				else
				{
					ret = wxMessageBox("Couldn't convert projection.\n"
							"Proceed anyway?", "Warning", wxYES_NO);
					if (ret == wxYES)
						keep = true;
				}
			}
			if (!keep)
			{
				return false;
			}
		}
	}
	AddLayer(pLayer);
	SetActiveLayer(pLayer, false);
	if (bRefresh)
	{
		// refresh the view
		m_pView->ZoomAll();
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
			m_pView->SetMode(LB_Road);

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
	m_fpIni = fopen("VTBuilder.ini", "rb+");

	if (m_fpIni)
	{
		int ShowMap, ShowElev, Shading, DoMask, DoUTM;
		fscanf(m_fpIni, "%d %d %d %d %d", &ShowMap, &ShowElev, &Shading,
			&DoMask, &DoUTM);

		m_pView->m_bShowMap = (ShowMap != 0);
		vtElevLayer::m_bShowElevation = (ShowElev != 0);
		vtElevLayer::m_bShading = (Shading != 0);
		vtElevLayer::m_bDoMask = (DoMask != 0);
		m_pView->m_bShowUTMBounds = (DoUTM != 0);

		return true;
	}
	m_fpIni = fopen("VTBuilder.ini", "wb");
	return false;
}

bool MainFrame::WriteINI()
{
	if (m_fpIni)
	{
		rewind(m_fpIni);
		fprintf(m_fpIni, "%d %d %d %d %d", m_pView->m_bShowMap,
			vtElevLayer::m_bShowElevation,
			vtElevLayer::m_bShading, vtElevLayer::m_bDoMask,
			m_pView->m_bShowUTMBounds);
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
		choices[i] = vtLayer::LayerTypeName[i];

	int n = LAYER_TYPES;
	static int cur_type = 0;	// remember the choice for next time

	wxSingleChoiceDialog dialog(this, "These are your choices",
		"Please indicate layer type", n, (const wxString *)choices);

	dialog.SetSelection(cur_type);

	if (dialog.ShowModal() == wxID_OK)
	{
		cur_type = dialog.GetSelection();
		return (LayerType) cur_type;
	}
	else
		return LT_UNKNOWN;
}


FeatInfoDlg	*MainFrame::ShowFeatInfoDlg()
{
	if (!m_pFeatInfoDlg)
	{
		// Create new Bioregion Dialog
		m_pFeatInfoDlg = new FeatInfoDlg(this, WID_FEATINFO, "Feature Info",
				wxPoint(120, 80), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	}
	m_pFeatInfoDlg->Show(true);
	return m_pFeatInfoDlg;
}

//
// merge all terrain data into this one
//
void MainFrame::SampleCurrentTerrains(vtElevLayer *pTarget)
{
	DRECT area;
	pTarget->GetExtent(area);
	DPoint2 step = pTarget->m_pGrid->GetSpacing();

	double x, y;
	int i, j, l, layers = m_Layers.GetSize();
	double fData, fBestData;
	int iColumns, iRows;
	pTarget->m_pGrid->GetDimensions(iColumns, iRows);

	// Create progress dialog for the slow part
	OpenProgressDialog("Merging and Resampling Elevation Layers");

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
	for (i = 0; i < iColumns; i++)
	{
		UpdateProgressDialog(i*100/iColumns);
		x = area.left + (i * step.x);
		for (j = 0; j < iRows; j++)
		{
			y = area.bottom + (j * step.y);

			// find some data for this point
			fBestData = INVALID_ELEVATION;
			for (g = 0; g < num_elevs; g++)
			{
				vtElevLayer *elev = elevs[g];
				vtElevationGrid *grid = elev->m_pGrid;

				fData = grid->GetFilteredValue2(x, y);
				if (fData != INVALID_ELEVATION &&
						(fBestData == INVALID_ELEVATION || fBestData == 0.0f))
					fBestData = fData;
				if (fBestData != INVALID_ELEVATION && fBestData != 0)
					break;
			}
			pTarget->m_pGrid->SetFValue(i, j, fBestData);
		}
	}
	CloseProgressDialog();
	delete elevs;
}


double MainFrame::GetHeightFromTerrain(DPoint2 &p)
{
	double height = INVALID_ELEVATION;

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

void MainFrame::SetProjection(vtProjection &p)
{
	m_proj = p;
	GetView()->SetWMProj(p);
}

////////////////////////////////////////////////////////////////
// Project ops

void MainFrame::LoadProject(const wxString &strPathName)
{
	// read project file
	wxString str;
	FILE *fp = fopen(strPathName, "rb");
	if (!fp)
	{
		str = "Couldn't open project file: ";
		str += strPathName;
		wxMessageBox(str);
		return;
	}

	// read projection info
	char buf[900];
	fgets(buf, 900, fp);
	char *wkt = buf + 11;
	OGRErr err = m_proj.importFromWkt(&wkt);
	if (err != OGRERR_NONE)
	{
		wxMessageBox("Had trouble parsing the projection information from that file.");
		fclose(fp);
		return;
	}

	int count = 0;
	LayerType ltype;

	fscanf(fp, "layers: %d\n", &count);
	if (count < 1)
	{
		fclose(fp);
		str = "Empty or invalid project file: ";
		str += strPathName;
		wxMessageBox(str);
		return;
	}
	for (int i = 0; i < count; i++)
	{
		char buf2[80];
		fscanf(fp, "type %d, %s\n", &ltype, buf2);
		fgets(buf, 160, fp);

		// trim trailing LF character
		int len = strlen(buf);
		if (len && buf[len-1] == 10) buf[len-1] = 0;
		len = strlen(buf);
		if (len && buf[len-1] == 13) buf[len-1] = 0;
		wxString fname = buf;

		if (!strcmp(buf2, "import"))
		{
			ImportDataFromFile(ltype, fname, false);
		}
		else
		{
			vtLayer *lp = vtLayer::CreateNewLayer(ltype);
			if (lp->Load(fname))
				AddLayer(lp);
			else
				delete lp;
		}
	}

	fscanf(fp, "%s", buf);
	if (!strcmp(buf, "area"))
	{
		fscanf(fp, "%lf %lf %lf %lf\n", &m_area.left, &m_area.top,
			&m_area.right, &m_area.bottom);
	}

	bool bHasView = false;
	fscanf(fp, "%s", buf);
	if (!strcmp(buf, "view"))
	{
		DRECT rect;
		fscanf(fp, "%lf %lf %lf %lf\n", &rect.left, &rect.top,
			&rect.right, &rect.bottom);
		m_pView->ZoomToRect(rect, 0.0f);
		bHasView = true;
	}
	fclose(fp);

	// refresh the view
	if (!bHasView)
		m_pView->ZoomAll();
	RefreshTreeView();
	RefreshToolbar();
}

void MainFrame::SaveProject(const wxString &strPathName)
{
	// write project file
	FILE *fp = fopen(strPathName, "wb");
	if (!fp)
		return;

	// write projection info
	char *wkt;
	m_proj.exportToWkt(&wkt);
	fprintf(fp, "Projection %s\n", wkt);
	OGRFree(wkt);

	// write list of layers
	int iLayers = m_Layers.GetSize();
	fprintf(fp, "layers: %d\n", iLayers);

	vtLayer *lp;
	for (int i = 0; i < iLayers; i++)
	{
		lp = m_Layers.GetAt(i);

		fprintf(fp, "type %d, %s\n", lp->GetType(), lp->IsNative() ? "native" : "import");
		fprintf(fp, "%s\n", (const char *) lp->GetFilename());
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


///////////////////////////////////////////////////////////////////////
// Drag-and-drop functionality
//

bool DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
	size_t nFiles = filenames.GetCount();
	for ( size_t n = 0; n < nFiles; n++ )
	{
		wxString str = filenames[n];
		if (!str.Right(3).CmpNoCase("vtb"))
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
	DPoint2 spacing(1.0f, 1.0f);
	for (int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() == LT_ELEVATION)
		{
			vtElevLayer *el = (vtElevLayer *)l;
			if (el->IsGrid())
			{
				if (el->m_pGrid->IsFloatMode())
					floatmode = true;
				spacing = el->m_pGrid->GetSpacing();
			}
		}
	}
	if (spacing == DPoint2(1.0f, 1.0f))
	{
		wxMessageBox("Sorry, you must have some elevation grid layers to\n"
			"perform a sampling operation on them.", "Info");
		return;
	}

	// Open the Resample dialog
	ResampleDlg dlg(this, -1, "Merge and Resample Elevation");
	dlg.m_fEstX = spacing.x;
	dlg.m_fEstY = spacing.y;
	dlg.m_area = m_area;
	dlg.m_bFloats = floatmode;

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString filter = "All Files|*.*|";
	AddType(filter, FSTRING_BT);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, "Export Elevation", "", "", filter, wxSAVE);
	saveFile.SetFilterIndex(1);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strPathName = saveFile.GetPath();

	// Make new terrain
	vtElevLayer *pOutput = new vtElevLayer(dlg.m_area, dlg.m_iSizeX,
			dlg.m_iSizeY, dlg.m_bFloats, dlg.m_fVUnits, m_proj);
	pOutput->SetFilename(strPathName);

	// fill in the value for pBig by merging samples from all other terrain
	SampleCurrentTerrains(pOutput);
#if 1
	pOutput->FillGaps();
#endif

	bool success = pOutput->m_pGrid->SaveToBT(strPathName);
	if (!success)
	{
		wxMessageBox("Couldn't open file for writing.");
		delete pOutput;
		return;
	}

	wxString str = "Successfully wrote BT file ";
	str += strPathName;
	wxMessageBox(str);
	delete pOutput;
}


//////////////////////////
// Vegetation ops

void MainFrame::FindVegLayers(vtVegLayer **Density, vtVegLayer **BioMap)
{
	for (int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *lp = m_Layers.GetAt(i);
		if (lp->GetType() == LT_VEG)
		{
			vtVegLayer *veg = (vtVegLayer *)lp;
			VegLayerType vltype = veg->m_VLType;

			if (vltype == VLT_Density)
				*Density = veg;
			if (vltype == VLT_BioMap)
				*BioMap = veg;
		}
	}
}

void MainFrame::GenerateVegetation(const char *vf_file, DRECT area, 
	vtVegLayer *pLandUse, vtVegLayer *pVegType,
	float fTreeSpacing, float fTreeScarcity)
{
	int i, j;
	DPoint2 p, p2;

	for (i = 0; i < m_BioRegions.m_Types.GetSize(); i++)
		m_PlantList.LookupPlantIndices(m_BioRegions.m_Types[i]);

	OpenProgressDialog("Generating Vegetation");

	int tree_count = 0;

	float x_size = (area.right - area.left);
	float y_size = (area.top - area.bottom);
	int x_trees = (int)(x_size / fTreeSpacing);
	int y_trees = (int)(y_size / fTreeSpacing);

	int bio_type;
	float density_scale;

	vtPlantInstanceArray pia;
//	pia.m_proj.SetUTM(true);

	// all vegetation
	for (i = 0; i < x_trees; i ++)
	{
		wxString str;
		str.Printf("plants: %d", pia.GetSize());
		UpdateProgressDialog(i * 100 / x_trees, str);

		p.x = area.left + (i * fTreeSpacing);
		for (j = 0; j < y_trees; j ++)
		{
			p.y = area.bottom + (j * fTreeSpacing);

			// randomize the position slightly
			p2.x = p.x + random_offset(fTreeSpacing * 0.5f);
			p2.y = p.y + random_offset(fTreeSpacing * 0.5f);

			// Get Land Use Attribute
			density_scale = pLandUse->FindDensity(p2);

			if (density_scale == 0.0f)
				continue;

			bio_type = pVegType->FindBiotype(p2);
			if (bio_type == -1)
				continue;

			float square_meters = fTreeSpacing * fTreeSpacing;

			// look at veg_type to decide which BioType to use
			vtBioType *bio = m_BioRegions.m_Types[bio_type];

			float factor = density_scale * square_meters * fTreeScarcity;

			int i;
			for (i = 0; i < bio->m_Densities.GetSize(); i++)
			{
				vtPlantDensity *pd = bio->m_Densities[i];

				pd->m_amount += (pd->m_plant_per_m2 * factor);
			}
			int species_num = -1;
			for (i = 0; i < bio->m_Densities.GetSize(); i++)
			{
				vtPlantDensity *pd = bio->m_Densities[i];
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
				pia.AddInstance(p2, random(ps->GetMaxHeight()), species_num);
			}
		}
	}
	pia.WriteVF(vf_file);
	CloseProgressDialog();
}


//////////////////
// Keyboard shortcuts

void MainFrame::OnChar(wxKeyEvent& event)
{
	m_pView->OnChar(event);
}
