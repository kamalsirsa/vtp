// EnviroFrame.cpp : implementation of the EnviroFrame class
//

#include "StdAfx.h"
#include "Charset.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/TerrainScene.h"
#include "vtdata/vtLog.h"

#include "EnviroApp.h"
#include "EnviroFrame.h"
#include "ChooseDlg.h"

#include "HtmlVw.h"
#include "../Options.h"
#include "EnviroGUI.h"

/////////////////////////////////////////////////////////////////////////////
// EnviroFrame

IMPLEMENT_DYNCREATE(EnviroFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(EnviroFrame, CFrameWnd)
	//{{AFX_MSG_MAP(EnviroFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FLYTO_SPACE, OnFlytoSpace)
	ON_COMMAND(ID_FLYTO_TERRAIN, OnFlytoTerrain)
	ON_UPDATE_COMMAND_UI(ID_FLYTO_SPACE, OnUpdateFlytoSpace)
	ON_UPDATE_COMMAND_UI(ID_FLYTO_TERRAIN, OnUpdateFlytoTerrain)
	ON_COMMAND(ID_SPACE_SHOWTIME, OnSpaceShowtime)
	ON_UPDATE_COMMAND_UI(ID_SPACE_SHOWTIME, OnUpdateSpaceShowtime)
	ON_COMMAND(ID_EARTH_FLATUNFOLD, OnEarthFlatunfold)
	ON_UPDATE_COMMAND_UI(ID_EARTH_FLATUNFOLD, OnUpdateEarthFlatunfold)
	ON_COMMAND(ID_EARTH_ADDPOINTDATA, OnEarthAddpointdata)
	ON_COMMAND(ID_TOOLS_TREES, OnToolsTrees)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TREES, OnUpdateToolsTrees)
	ON_COMMAND(ID_TOOLS_FENCES, OnToolsFences)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_FENCES, OnUpdateToolsFences)
	ON_COMMAND(ID_TOOLS_NAVIGATE, OnToolsNavigate)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_NAVIGATE, OnUpdateToolsNavigate)
	ON_COMMAND(ID_TOOLS_MOVE, OnToolsMove)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_MOVE, OnUpdateToolsMove)
	ON_UPDATE_COMMAND_UI(ID_EARTH_ADDPOINTDATA, OnUpdateEarthAddpointdata)
	ON_COMMAND(ID_TOOLS_SELECT, OnToolsSelect)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_SELECT, OnUpdateToolsSelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,	// status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


/////////////////////////////////////////////////////////////////////////////
// EnviroFrame construction/destruction

EnviroFrame::EnviroFrame() : CFrameWnd()
{
	// test code
	CString str;
	str.Delete(3, 1);
}

EnviroFrame::~EnviroFrame()
{
}

int EnviroFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		VTLOG("Failed to create toolbar\n");
		return -1;	// fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		VTLOG("Failed to create status bar\n");
		return -1;	// fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);

	if (g_Options.m_bFloatingToolbar)
		FloatControlBar(&m_wndToolBar, CPoint(100, 100));
	else
		DockControlBar(&m_wndToolBar);

	m_wndPlants.Create(IDD_PLANTDLG, this);
	m_wndPlants.SetWindowPos(NULL, 600, 200, -1, -1, SWP_NOREPOSITION | SWP_NOSIZE);

	m_wndFences.Create(IDD_FENCEDLG, this);
	m_wndFences.SetWindowPos(NULL, 600, 200, -1, -1, SWP_NOREPOSITION | SWP_NOSIZE);

	return 0;
}

BOOL EnviroFrame::OnCreateClient( LPCREATESTRUCT lpcs,
	CCreateContext* pContext)
{
	if (g_Options.m_bHtmlpane)
	{
		SIZE s;
		s.cx = 500;
		s.cy = 600;

		m_wndSplitter.CreateStatic( this, 1, 2); // the number of rows, columns
		BOOL v1 = m_wndSplitter.CreateView(0, 0, pContext->m_pNewViewClass, s, pContext);
		BOOL v2 = m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CMfcieView), s, pContext);
	}
	else
	{
		if (pContext != NULL && pContext->m_pNewViewClass != NULL)
		{
			if (CreateView(pContext, AFX_IDW_PANE_FIRST) == NULL)
				return FALSE;
		}
	}
	return TRUE;
}

void EnviroFrame::UpdateStatusBar()
{
	vtString vs = g_App.GetStatusString(0);
	vs += "  ";
	vs += g_App.GetStatusString(1);
	vs += "  ";
	vs += g_App.GetStatusString(2);

	CString str = FromUTF8(vs);
	m_wndStatusBar.SetPaneText(0, str);
}


/////////////////////////////////////////////////////////////////////////////
// EnviroFrame message handlers

void EnviroFrame::OnFlytoSpace()
{
	g_App.FlyToSpace();
}

void EnviroFrame::OnFlytoTerrain()
{
	CChooseDlg dlg;
	dlg.m_strTName = _T("none");
	if (dlg.DoModal() == IDOK)
	{
		vtString fname = ToUTF8(dlg.m_strTName);
		g_App.SwitchToTerrain(fname);
	}
}


void EnviroFrame::OnUpdateFlytoSpace(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnUpdateFlytoTerrain(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
}

void EnviroFrame::OnSpaceShowtime()
{
	// TODO: Add your command handler code here
	g_App.SetEarthShading(!g_App.GetEarthShading());
}

void EnviroFrame::OnUpdateSpaceShowtime(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Orbit);
	pCmdUI->SetCheck(g_App.GetEarthShading());
}

void EnviroFrame::OnEarthFlatunfold()
{
	g_App.SetEarthShape(!g_App.GetEarthShape());
}

void EnviroFrame::OnUpdateEarthFlatunfold(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Orbit);
	pCmdUI->SetCheck(g_App.GetEarthShape());
}

void EnviroFrame::OnEarthAddpointdata()
{
	TCHAR path[256];
	// save
	GetCurrentDirectory(256, path);

	CFileDialog openDialog(TRUE);
	openDialog.m_ofn.lpstrFilter = _T("Point Data Sources (*.shp)\0*.shp\0");
	openDialog.m_ofn.lpstrInitialDir = _T(".");
	if (openDialog.DoModal() != IDOK)
	{
		// restore
		SetCurrentDirectory(path);
		return;
	}
	CString str = openDialog.GetPathName();

	int ret = g_App.AddGlobeAbstractLayer(ToUTF8(str));
	if (ret == -1)
		::AfxMessageBox(_T("Couldn't Open"));
	if (ret == -2)
		::AfxMessageBox(_T("That file isn't point data."));

	// restore
	SetCurrentDirectory(path);
}

void EnviroFrame::OnUpdateEarthAddpointdata(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Orbit);
}

void EnviroFrame::SetMode(MouseMode mode)
{
	// Show/hide plant dialog
	if (mode == MM_PLANTS)
	{
		m_wndPlants.SetPlantList(g_App.GetPlantList());
		m_wndPlants.SetPlantOptions(g_App.GetPlantOptions());
		m_wndPlants.ShowWindow(SW_SHOW);
	}
	else
		m_wndPlants.ShowWindow(SW_HIDE);

	// Show/hide fence dialog
	m_wndFences.ShowWindow(mode == MM_FENCES ? SW_SHOW : SW_HIDE);

	g_App.SetMode(mode);
}

void EnviroFrame::OnToolsTrees()
{
	SetMode(MM_PLANTS);
	g_App.EnableFlyerEngine(false);
}

void EnviroFrame::OnUpdateToolsTrees(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.m_mode == MM_PLANTS);
}

void EnviroFrame::OnToolsFences()
{
	SetMode(MM_FENCES);
	g_App.EnableFlyerEngine(false);
}

void EnviroFrame::OnUpdateToolsFences(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.m_mode == MM_FENCES);
}

void EnviroFrame::OnToolsNavigate()
{
	SetMode(MM_NAVIGATE);
	g_App.EnableFlyerEngine(true);
}

void EnviroFrame::OnUpdateToolsNavigate(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.m_mode == MM_NAVIGATE);
}

void EnviroFrame::OnToolsMove()
{
	SetMode(MM_MOVE);
	g_App.EnableFlyerEngine(false);
}

void EnviroFrame::OnUpdateToolsMove(CCmdUI* pCmdUI)
{
	// not yet implemented
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(g_App.m_mode == MM_MOVE);
}

void EnviroFrame::OnToolsSelect()
{
	SetMode(MM_SELECT);
	g_App.EnableFlyerEngine(false);
}

void EnviroFrame::OnUpdateToolsSelect(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(g_App.m_mode == MM_SELECT);
}


/////////////////////////////////////////////////////

void EnviroFrame::ShowPopupMenu(const IPoint2 &pos)
{
	// TODO
}

// Called by Enviro when the GUI needs to be informed of a new terrain
void EnviroFrame::SetTerrainToGUI(vtTerrain *pTerrain)
{
	// TODO
}

void EnviroFrame::EarthPosUpdated(const DPoint3 &pos)
{
}

