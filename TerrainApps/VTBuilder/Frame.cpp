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

#include "wx/resource.h"

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
#include "ElevDlg.h"

#if defined(__WXMSW__)
static char *dialog1 = NULL;
static char *dialog2 = NULL;
static char *dialog3 = NULL;
static char *dialog4 = NULL;
static char *dialog5 = NULL;
#else
// Other platforms should have compilers that cope with long strings.
#include "dialog1.wxr"
#include "dialog2.wxr"
#include "dialog3.wxr"
#include "dialog4.wxr"
#include "dialog5.wxr"
#endif

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
#endif

DECLARE_APP(MyApp)

// Window ids
#define WID_SPLITTER	100
#define WID_FRAME		101
#define WID_MAINVIEW	102

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

#if defined(__WXMSW__)
	// Load the .wxr 'file' from a .rc resource, under Windows.
	dialog1 = wxLoadUserResource("dialog1", "TEXT");
	dialog2 = wxLoadUserResource("dialog2", "TEXT");
	dialog3 = wxLoadUserResource("dialog3", "TEXT");
	dialog4 = wxLoadUserResource("dialog4", "TEXT");
	dialog5 = wxLoadUserResource("dialog5", "TEXT");

	// prepare resources
	bool b1 = wxResourceParseString(dialog1);
	bool b2 = wxResourceParseString(dialog2);
	bool b3 = wxResourceParseString(dialog3);
	bool b4 = wxResourceParseString(dialog4);
	bool b5 = wxResourceParseString(dialog5);
#else
	// prepare resources
	bool b1 = wxResourceParseData(dialog1);	// merge and resample
	bool b2 = wxResourceParseData(dialog2);	// scale elevation
	bool b3 = wxResourceParseData(dialog3);	// specific projection
	bool b4 = wxResourceParseData(dialog4);	// node properties
	bool b5 = wxResourceParseData(dialog5);	// road properties
#endif

	// frame icon
	SetIcon(wxICON(vtbuilder));

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

	m_pTree = new MyTreeCtrl(m_splitter, TreeTest_Ctrl,
		wxPoint(0, 0), wxSize(200, 400),
//		wxTR_HAS_BUTTONS |
//		wxTR_EDIT_LABELS |
#ifndef NO_VARIABLE_HEIGHT
		wxTR_HAS_VARIABLE_ROW_HEIGHT |
#endif
		wxNO_BORDER);
	m_pTree->SetBackgroundColour(*wxLIGHT_GREY);

	m_pView = new BuilderView(m_splitter, WID_MAINVIEW,
		wxPoint(0, 0), wxSize(200, 400), "MainView" );
	m_pView->SetBackgroundColour(*wxLIGHT_GREY);
	m_pView->Show(FALSE);

	// Read INI file after creating the view
	ReadINI();

	m_splitter->Initialize(m_pTree);

	////////////////////////
	m_pTree->Show(TRUE);
	m_pView->Show(TRUE);
	m_splitter->SplitVertically( m_pTree, m_pView, 200);

	m_bShowMinutes = false;

	vtProjection proj;
	proj.SetWellKnownGeogCS("WGS84");
	SetProjection(proj);
}

MainFrame::~MainFrame()
{
	WriteINI();
	DeleteContents();
#if defined(__WXMSW__) && 0
	delete dialog1;		// not safe apparently, to delete memory allocated in the DLL
	delete dialog2;
	delete dialog3;
#endif
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
	}
	toolBar_main->Realize();

	menuBar->EnableTop(4, lt == LT_ROAD);
	menuBar->EnableTop(5, lt == LT_UTILITY);
	menuBar->EnableTop(6, lt == LT_ELEVATION);
//	menuBar->EnableTop(7, lt == LT_VEG);
	menuBar->EnableTop(8, lt == LT_STRUCTURE);
	menuBar->EnableTop(9, lt == LT_RAW);
}

////////////////////////////////////////////////////////////////
// Application Methods

//
// Load a layer from a file without knowing its type
//
void MainFrame::LoadLayer(const wxString &fname)
{
	// check file extension
	wxString ext = fname.AfterLast('.');

	bool bFirst = (m_Layers.GetSize() == 0);

	vtLayer *pLayer = NULL;
	if (ext.CmpNoCase("rmf") == 0)
	{
		vtRoadLayer *pRL = new vtRoadLayer();
		if (pRL->Load(fname))
			pLayer = pRL;
	}
	if (ext.CmpNoCase("bt") == 0)
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
	if (!pLayer)
	{
		// try importing
		ImportDataFromFile(LT_UNKNOWN, fname, true);
		return;
	}

	bool success = AddLayerWithCheck(pLayer, true);
	if (!success)
		delete pLayer;
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
				"but the file you are attempting to load:\n     %s\n"
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

void MainFrame::AddLayer(vtLayer *lp)
{
	m_Layers.Append(lp);
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

//
// merge all terrain data into this one
//
void MainFrame::SampleCurrentTerrains(vtElevLayer *pTarget)
{
	DRECT area = pTarget->GetExtents();
	DPoint2 step = pTarget->m_pGrid->GetSpacing();

	double x, y;
	int i, j, l, layers = m_Layers.GetSize();
	double fData, fBestData;
	int iColumns, iRows;
	pTarget->m_pGrid->GetDimensions(iColumns, iRows);

	// Create progress dialog for the slow part
	OpenProgressDialog("Merging and Resampling Elevation Layers");

	int num_elev = LayersOfType(LT_ELEVATION);
	vtElevationGrid **grids = new vtElevationGrid *[num_elev];
	int g, num_grids = 0;
	for (l = 0; l < layers; l++)
	{
		vtLayer *lp = m_Layers.GetAt(l);
		if (lp->GetType() == LT_ELEVATION)
			grids[num_grids++] = ((vtElevLayer *)lp)->m_pGrid;
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
			for (g = 0; g < num_grids; g++)
			{
				vtElevationGrid *grid = grids[g];

				// is our point inside the extents of this terrain?
				if (! grid->ContainsPoint(x, y))
					continue;

				fData = grid->GetFilteredValue(x, y);
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
}


double MainFrame::GetHeightFromTerrain(double lx, double ly)
{
	double height = INVALID_ELEVATION;

	int layers = m_Layers.GetSize();
	for (int i = 0; i < layers; i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() != LT_ELEVATION) continue;
		vtElevLayer *pTerr = (vtElevLayer *)l;
		if (!pTerr->m_pGrid) continue;

		height = pTerr->m_pGrid->GetFilteredValue(lx, ly);
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
	// If any of the input terrain are floats, the output should be float
	bool floatmode = false;

	// sample spacing in meters/heixel or degrees/heixel
	DPoint2 spacing(1.0f, 1.0f);
	for (int i = 0; i < m_Layers.GetSize(); i++)
	{
		vtLayer *l = m_Layers.GetAt(i);
		if (l->GetType() == LT_ELEVATION)
		{
			vtElevLayer *el = (vtElevLayer *)l;
			if (el->m_pGrid->IsFloatMode())
				floatmode = true;
			spacing = el->m_pGrid->GetSpacing();
		}
	}

	// Open the Resample dialog
	ResampleDlg dlg(!m_proj.IsGeographic(), spacing.x);
	dlg.LoadFromResource(this, "dialog1");
	dlg.m_area = m_area;

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
	vtElevLayer *pBig = new vtElevLayer(dlg.m_area, dlg.m_iXSamples,
		dlg.m_iYSamples, floatmode, m_proj);
	pBig->SetFilename(strPathName);

	// fill in the value for pBig by merging samples from all other terrain
	SampleCurrentTerrains(pBig);
	pBig->FillGaps();

	bool success = pBig->m_pGrid->SaveToBT(strPathName);
	if (!success)
	{
		wxMessageBox("Couldn't open file for writing.");
		delete pBig;
		return;
	}

	wxString str = "Successfully wrote BT file ";
	str += strPathName;
	wxMessageBox(str);
	delete pBig;
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
