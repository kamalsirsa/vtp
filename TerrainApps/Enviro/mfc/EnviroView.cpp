//
// EnviroView.cpp : The 3d view window of the Enviro application
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/DynTerrain.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/TimeEngines.h"

#include "EnviroGUI.h"
#include "../Engines.h"

#include "EnviroFrame.h"
#include "EnviroView.h"
#include "EnviroApp.h"
#include "SimpleDoc.h"

/////////////////////////////////////////////////////////////////////////////
// EnviroView

IMPLEMENT_DYNCREATE(EnviroView, CView)

BEGIN_MESSAGE_MAP(EnviroView, CView)
	//{{AFX_MSG_MAP(EnviroView)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_COMMAND(ID_ENVIRO_TIMEON, OnEnviroTimeon)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_TIMEON, OnUpdateEnviroTimeon)
	ON_COMMAND(ID_ENVIRO_LODTERRAIN, OnEnviroLodterrain)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_LODTERRAIN, OnUpdateEnviroLodterrain)
	ON_COMMAND(ID_ENVIRO_CULLEVERYFRAME, OnEnviroCulleveryframe)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_CULLEVERYFRAME, OnUpdateEnviroCulleveryframe)
	ON_COMMAND(ID_ENVIRO_CULLONCE, OnEnviroCullonce)
	ON_COMMAND(ID_ENVIRO_INCREASE, OnEnviroIncrease)
	ON_COMMAND(ID_ENVIRO_DECREASE, OnEnviroDecrease)
	ON_COMMAND(ID_ENVIRO_DRIVEROADS, OnEnviroDriveroads)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_DRIVEROADS, OnUpdateEnviroDriveroads)
	ON_COMMAND(ID_ENVIRO_SHOWROADS, OnEnviroShowroads)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_SHOWROADS, OnUpdateEnviroShowroads)
	ON_COMMAND(ID_ENVIRO_SHOWOCEAN, OnEnviroShowocean)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_SHOWOCEAN, OnUpdateEnviroShowocean)
	ON_COMMAND(ID_ENVIRO_SHOWSKY, OnEnviroShowsky)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_SHOWSKY, OnUpdateEnviroShowsky)
	ON_COMMAND(ID_ENVIRO_SHOWFOG, OnEnviroShowfog)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_SHOWFOG, OnUpdateEnviroShowfog)
	ON_COMMAND(ID_ENVIRO_SHOWTREES, OnEnviroShowtrees)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_SHOWTREES, OnUpdateEnviroShowtrees)
	ON_COMMAND(ID_VIEW_TOPDOWN, OnEnviroTopdownview)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOPDOWN, OnUpdateEnviroTopdownview)
	ON_COMMAND(ID_ENVIRO_SHOWDETAILTEXTURE, OnShowdetailtexture)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_SHOWDETAILTEXTURE, OnUpdateShowdetailtexture)
	ON_COMMAND(ID_VIEW_FLYFASTER, OnViewFlyfaster)
	ON_COMMAND(ID_VIEW_FLYSLOWER, OnViewFlyslower)
	ON_COMMAND(ID_VIEW_MAINTAINHEIGHT, OnViewMaintainheight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAINTAINHEIGHT, OnUpdateViewMaintainheight)
	ON_COMMAND(ID_TOOLS_NAVIGATE_ORIGINALNAVENGIN, OnToolsNavigateOriginalnavengin)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_NAVIGATE_ORIGINALNAVENGIN, OnUpdateToolsNavigateOriginalnavengin)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_CULLONCE, OnUpdateEnviroCullonce)
	ON_COMMAND(ID_TOOLS_NAVIGATE_QUAKENAVENGINE, OnToolsNavigateQuakenavengine)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_NAVIGATE_QUAKENAVENGINE, OnUpdateToolsNavigateQuakenavengine)
	ON_COMMAND(ID_TOOLS_NAVIGATE_VELOCITYGRAVITY, OnToolsNavigateVelocitygravity)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_NAVIGATE_VELOCITYGRAVITY, OnUpdateToolsNavigateVelocitygravity)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FLYFASTER, OnUpdateViewFlyfaster)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FLYSLOWER, OnUpdateViewFlyslower)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

EnviroView::EnviroView()
{
	m_bTimeOn = true;
	m_bCulleveryframe = true;
	m_bDriveRoads = false;
	m_bTopDown = false;
	m_bMaintainHeight = false;

	m_bRunning = false;
}

EnviroView::~EnviroView()
{
}

BOOL EnviroView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// helper
int WindowsMouseFlagsToVT(UINT nFlags)
{
	int flags = 0;
	if (nFlags & MK_SHIFT)
		flags |= VT_SHIFT;
	if (nFlags & MK_CONTROL)
		flags |= VT_CONTROL;
	return flags;
}


/////////////////////////////////////////////////////////////////////////////
// EnviroView message handlers

void EnviroView::OnDraw(CDC* pDC)
{
	CSimpleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

void EnviroView::OnEnviroTimeon()
{
	m_bTimeOn = !m_bTimeOn;
	vtGetTS()->GetTimeEngine()->SetEnabled(m_bTimeOn);
}

void EnviroView::OnUpdateEnviroTimeon(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bTimeOn);
}

void EnviroView::OnEnviroLodterrain()
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->GetDynTerrain()) return;
	bool on = t->GetDynTerrain()->GetEnabled();

	t->GetDynTerrain()->SetEnabled(!on);
}

void EnviroView::OnUpdateEnviroLodterrain(CCmdUI* pCmdUI)
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->GetDynTerrain())
	{
		pCmdUI->Enable(false);
		return;
	}
	bool on = t->GetDynTerrain()->GetEnabled();

	pCmdUI->Enable(true);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroCulleveryframe()
{
	m_bCulleveryframe = !m_bCulleveryframe;
	GetCurrentTerrain()->GetDynTerrain()->SetCull(m_bCulleveryframe);
}

void EnviroView::OnUpdateEnviroCulleveryframe(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(m_bCulleveryframe);
}

void EnviroView::OnEnviroCullonce()
{
	GetCurrentTerrain()->GetDynTerrain()->CullOnce();
}

void EnviroView::OnUpdateEnviroCullonce(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
}

void EnviroView::OnEnviroIncrease()
{
	vtDynTerrainGeom *pTerr = GetCurrentTerrain()->GetDynTerrain();
	if (pTerr)
		pTerr->SetPolygonTarget(pTerr->GetPolygonTarget()+1000);
}

void EnviroView::OnEnviroDecrease()
{
	vtDynTerrainGeom *pTerr = GetCurrentTerrain()->GetDynTerrain();
	if (pTerr)
		pTerr->SetPolygonTarget(pTerr->GetPolygonTarget()-1000);
}

void EnviroView::OnEnviroDriveroads()
{
	m_bDriveRoads = !m_bDriveRoads;
}

void EnviroView::OnUpdateEnviroDriveroads(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(m_bDriveRoads);

	// not implemented
	pCmdUI->Enable(FALSE);
}

void EnviroView::OnEnviroShowroads()
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_ROADS, !t->GetFeatureVisible(TFT_ROADS));
}

void EnviroView::OnUpdateEnviroShowroads(CCmdUI* pCmdUI)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_ROADS);

	pCmdUI->Enable(t != NULL);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroShowocean()
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_OCEAN, !t->GetFeatureVisible(TFT_OCEAN));
}

void EnviroView::OnUpdateEnviroShowocean(CCmdUI* pCmdUI)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_OCEAN);

	pCmdUI->Enable(t != NULL);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroShowsky()
{
	vtSkyDome *sky = vtGetTS()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	sky->SetEnabled(!on);
}

void EnviroView::OnUpdateEnviroShowsky(CCmdUI* pCmdUI)
{
	vtSkyDome *sky = vtGetTS()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroShowfog()
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFog(!t->GetFog());
}

void EnviroView::OnUpdateEnviroShowfog(CCmdUI* pCmdUI)
{
	vtTerrain *t = GetCurrentTerrain();
	pCmdUI->SetCheck(t && t->GetFog());
}

void EnviroView::OnEnviroShowtrees()
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_VEGETATION, !t->GetFeatureVisible(TFT_VEGETATION));
}

void EnviroView::OnUpdateEnviroShowtrees(CCmdUI* pCmdUI)
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_VEGETATION);

	pCmdUI->Enable(t != NULL);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroTopdownview()
{
	m_bTopDown = !m_bTopDown;
	g_App.SetTopDown(m_bTopDown);
}

void EnviroView::OnUpdateEnviroTopdownview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(m_bTopDown);
}


void EnviroView::OnShowdetailtexture()
{
	vtDynTerrainGeom *pTerr = GetCurrentTerrain()->GetDynTerrain();
	pTerr->EnableDetail(!pTerr->GetDetail());
}

void EnviroView::OnUpdateShowdetailtexture(CCmdUI* pCmdUI)
{
	bool bHasIt = (GetCurrentTerrain() && GetCurrentTerrain()->GetDynTerrain() != NULL);

	pCmdUI->Enable(bHasIt && g_App.m_state == AS_Terrain);
	if (bHasIt)
	{
		vtDynTerrainGeom *pTerr = GetCurrentTerrain()->GetDynTerrain();
		pCmdUI->SetCheck(pTerr->GetDetail());
	}
	else
		pCmdUI->Enable(false);
}

void EnviroView::OnMouseMove(UINT nFlags, CPoint point)
{
	vtMouseEvent event;
	event.type = VT_MOVE;
	event.button = VT_NONE;
	event.flags = WindowsMouseFlagsToVT(nFlags);
	event.pos.Set(point.x, point.y);

	g_App.OnMouse(event);			// inform Enviro app
	vtGetScene()->OnMouse(event);	// inform vtlib
}

void EnviroView::OnViewFlyfaster()
{
	float speed = g_App.GetFlightSpeed();
	g_App.SetFlightSpeed(speed * 1.8f);
}

void EnviroView::OnViewFlyslower()
{
	float speed = g_App.GetFlightSpeed();
	g_App.SetFlightSpeed(speed / 1.8f);
}

void EnviroView::OnViewMaintainheight()
{
	g_App.SetMaintain(!g_App.GetMaintain());
}

void EnviroView::OnUpdateViewMaintainheight(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.GetMaintain());
}

void EnviroView::OnToolsNavigateOriginalnavengin()
{
	g_App.m_nav = NT_Normal;
	g_App.EnableFlyerEngine(true);
}

void EnviroView::OnUpdateToolsNavigateOriginalnavengin(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.m_nav == NT_Normal);
}

void EnviroView::OnToolsNavigateQuakenavengine()
{
//	g_App.m_nav = NT_Quake;
	g_App.EnableFlyerEngine(true);
}

void EnviroView::OnUpdateToolsNavigateQuakenavengine(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
//	pCmdUI->SetCheck(g_App.m_nav == NT_Quake);
}

void EnviroView::OnToolsNavigateVelocitygravity()
{
	g_App.m_nav = NT_Velo;
	g_App.EnableFlyerEngine(true);
}

void EnviroView::OnUpdateToolsNavigateVelocitygravity(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.m_nav == NT_Velo);
}


void EnviroView::OnUpdateViewFlyfaster(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
}

void EnviroView::OnUpdateViewFlyslower(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
}

void EnviroView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// turn MFC mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_DOWN;
	event.button = VT_LEFT;
	event.flags = WindowsMouseFlagsToVT(nFlags);
	event.pos.Set(point.x, point.y);

	g_App.OnMouse(event);			// inform Enviro app
	vtGetScene()->OnMouse(event);	// inform vtlib
}

void EnviroView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// turn MFC mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_UP;
	event.button = VT_LEFT;
	event.flags = WindowsMouseFlagsToVT(nFlags);
	event.pos.Set(point.x, point.y);

	g_App.OnMouse(event);			// inform Enviro app
	vtGetScene()->OnMouse(event);	// inform vtlib
}

void EnviroView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// turn MFC mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_DOWN;
	event.button = VT_RIGHT;
	event.flags = WindowsMouseFlagsToVT(nFlags);
	event.pos.Set(point.x, point.y);

	g_App.OnMouse(event);			// inform Enviro app
	vtGetScene()->OnMouse(event);	// inform vtlib
}

void EnviroView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// turn MFC mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_UP;
	event.button = VT_RIGHT;
	event.flags = WindowsMouseFlagsToVT(nFlags);
	event.pos.Set(point.x, point.y);

	g_App.OnMouse(event);			// inform Enviro app
	vtGetScene()->OnMouse(event);	// inform vtlib
}

void EnviroView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// turn MFC mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_DOWN;
	event.button = VT_MIDDLE;
	event.flags = WindowsMouseFlagsToVT(nFlags);
	event.pos.Set(point.x, point.y);

	g_App.OnMouse(event);			// inform Enviro app
	vtGetScene()->OnMouse(event);	// inform vtlib
}

void EnviroView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// turn MFC mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_UP;
	event.button = VT_MIDDLE;
	event.flags = WindowsMouseFlagsToVT(nFlags);
	event.pos.Set(point.x, point.y);

	g_App.OnMouse(event);			// inform Enviro app
	vtGetScene()->OnMouse(event);	// inform vtlib
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleView message handlers
// set Windows Pixel Format
BOOL EnviroView::SetWindowPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pixelDesc;

	pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion = 1;

	pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER | PFD_STEREO_DONTCARE;

	pixelDesc.iPixelType = PFD_TYPE_RGBA;
	pixelDesc.cColorBits = 32;
	pixelDesc.cRedBits = 8;
	pixelDesc.cRedShift = 16;
	pixelDesc.cGreenBits = 8;
	pixelDesc.cGreenShift = 8;
	pixelDesc.cBlueBits = 8;
	pixelDesc.cBlueShift = 0;
	pixelDesc.cAlphaBits = 0;
	pixelDesc.cAlphaShift = 0;
	pixelDesc.cAccumBits = 64;
	pixelDesc.cAccumRedBits = 16;
	pixelDesc.cAccumGreenBits = 16;
	pixelDesc.cAccumBlueBits = 16;
	pixelDesc.cAccumAlphaBits = 0;
	pixelDesc.cDepthBits = 32;
	pixelDesc.cStencilBits = 8;
	pixelDesc.cAuxBuffers = 0;
	pixelDesc.iLayerType = PFD_MAIN_PLANE;
	pixelDesc.bReserved = 0;
	pixelDesc.dwLayerMask = 0;
	pixelDesc.dwVisibleMask = 0;
	pixelDesc.dwDamageMask = 0;

	m_GLPixelIndex = ChoosePixelFormat(hDC,&pixelDesc);
	if (m_GLPixelIndex == 0) // Choose default
	{
		m_GLPixelIndex = 1;
		if (DescribePixelFormat(hDC,m_GLPixelIndex,
			sizeof(PIXELFORMATDESCRIPTOR),&pixelDesc)==0)
			return FALSE;
	}

	if (!SetPixelFormat(hDC,m_GLPixelIndex,&pixelDesc))
		return FALSE;

	return TRUE;
}

//********************************************
// CreateViewGLContext
// Create an OpenGL rendering context
//********************************************
BOOL EnviroView::CreateViewGLContext(HDC hDC)
{
	m_hGLContext = wglCreateContext(hDC);

	if (m_hGLContext==NULL)
		return FALSE;

	if (wglMakeCurrent(hDC,m_hGLContext)==FALSE)
		return FALSE;

	return TRUE;
}


void EnviroView::OnDestroy()
{
	if (wglGetCurrentContext() != NULL)
		wglMakeCurrent(NULL,NULL);
	if (m_hGLContext != NULL)
	{
		wglDeleteContext(m_hGLContext);
		m_hGLContext = NULL;
	}
	CView::OnDestroy();
}

int EnviroView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	HWND hWnd = GetSafeHwnd();
	HDC hDC = ::GetDC(hWnd);

	if (SetWindowPixelFormat(hDC)==FALSE)
		return 0;

	if (CreateViewGLContext(hDC)==FALSE)
		return 0;

	// We use a combination of paint-on-idle and a timer to keep the
	//  rendering continuous.  Idle events drive the painting, and these
	//  timer events (which we ignore) keep the event queue moving.
	// Interval of 10ms is 100fps.
	int timer = SetTimer(1, 10, 0);

	return 1;
}

void EnviroView::OnSize(UINT nType, int cx, int cy)
{
	vtGetScene()->SetWindowSize(cx, cy);
}

BOOL EnviroView::OnEraseBkgnd(CDC *pDC)
{
    UNUSED_ALWAYS(pDC);
    return TRUE;
}

void EnviroView::OnPaint()
{
	if (1)
	{
		CPaintDC dc(this); // device context for painting

		// Render the scene
		vtGetScene()->DoUpdate();

		SwapBuffers(dc.m_ps.hdc);
	}

	// update the status bar every 1/10 of a second
	static float last_stat = 0.0f;
	static vtString last_msg;
	float cur = vtGetTime();
	if (cur - last_stat > 0.1f || g_App.GetMessage() != last_msg)
	{
		last_msg = g_App.GetMessage();
		last_stat = cur;

		EnviroFrame *pFrame = (EnviroFrame *) AfxGetMainWnd();
		pFrame->UpdateStatusBar();
	}

	// We are now using repain on idle, so don't repaint on paint
#if 0
	if (m_bRunning)
		InvalidateRect(NULL,FALSE);	//for Continuous Rendering
#endif
}

void EnviroView::PaintOnIdle()
{
	if (m_bRunning)
		InvalidateRect(NULL,FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// EnviroView diagnostics

#ifdef _DEBUG
CSimpleDoc* EnviroView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpleDoc)));
	return (CSimpleDoc*)m_pDocument;
}
#endif //_DEBUG


