//
// EnviroView.cpp : The 3d view window of the Enviro application
//

#include "StdAfx.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/DynTerrain.h"
#include "vtlib/core/TerrainSurface.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/TimeEngines.h"

#include "../TerrainSceneWP.h"
#include "../Enviro.h"
#include "../Engines.h"

#include "EnviroView.h"
#include "EnviroApp.h"
#include "SimpleDoc.h"

#define LONGEST_FENCE 2000

/////////////////////////////////////////////////////////////////////////////
// EnviroView

IMPLEMENT_DYNCREATE(EnviroView, CView)

BEGIN_MESSAGE_MAP(EnviroView, CView)
	//{{AFX_MSG_MAP(EnviroView)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_COMMAND(ID_ENVIRO_TIMEON, OnEnviroTimeon)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_TIMEON, OnUpdateEnviroTimeon)
	ON_COMMAND(ID_ENVIRO_REGULARTERRAIN, OnEnviroRegularterrain)
	ON_COMMAND(ID_ENVIRO_LODTERRAIN, OnEnviroLodterrain)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_LODTERRAIN, OnUpdateEnviroLodterrain)
	ON_UPDATE_COMMAND_UI(ID_ENVIRO_REGULARTERRAIN, OnUpdateEnviroRegularterrain)
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
	GetTerrainScene().m_pTime->SetEnabled(m_bTimeOn);
}

void EnviroView::OnUpdateEnviroTimeon(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bTimeOn);
}

void EnviroView::OnEnviroRegularterrain() 
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pTerrainGeom) return;
	bool on = t->m_pTerrainGeom->GetEnabled();

	t->m_pTerrainGeom->SetEnabled(!on);
}

void EnviroView::OnUpdateEnviroRegularterrain(CCmdUI* pCmdUI) 
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pTerrainGeom)
	{
		pCmdUI->Enable(false);
		return;
	}
	bool on = t->m_pTerrainGeom->GetEnabled();

	pCmdUI->Enable(true);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroLodterrain() 
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pDynGeom) return;
	bool on = t->m_pDynGeom->GetEnabled();

	t->m_pDynGeom->SetEnabled(!on);
}

void EnviroView::OnUpdateEnviroLodterrain(CCmdUI* pCmdUI) 
{
	vtTerrain *t = GetCurrentTerrain();
	if (!t || !t->m_pDynGeom)
	{
		pCmdUI->Enable(false);
		return;
	}
	bool on = t->m_pDynGeom->GetEnabled();

	pCmdUI->Enable(true);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroCulleveryframe() 
{
	m_bCulleveryframe = !m_bCulleveryframe;
	GetCurrentTerrain()->m_pDynGeom->SetCull(m_bCulleveryframe);
}

void EnviroView::OnUpdateEnviroCulleveryframe(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(m_bCulleveryframe);
}

void EnviroView::OnEnviroCullonce() 
{
	GetCurrentTerrain()->m_pDynGeom->CullOnce();
}

void EnviroView::OnUpdateEnviroCullonce(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
}

void EnviroView::OnEnviroIncrease() 
{
	vtDynTerrainGeom *pTerr = GetCurrentTerrain()->m_pDynGeom;
	if (pTerr)
	{
		pTerr->SetPixelError(pTerr->GetPixelError()+0.1f);
		pTerr->SetPolygonCount(pTerr->GetPolygonCount()+1000);
	}
}

void EnviroView::OnEnviroDecrease() 
{
	vtDynTerrainGeom *pTerr = GetCurrentTerrain()->m_pDynGeom;
	if (pTerr)
	{
		pTerr->SetPixelError(pTerr->GetPixelError()-0.1f);
		pTerr->SetPolygonCount(pTerr->GetPolygonCount()-1000);
	}
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
	if (t) t->SetFeatureVisible(ROADS, !t->GetFeatureVisible(ROADS));
}

void EnviroView::OnUpdateEnviroShowroads(CCmdUI* pCmdUI) 
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(ROADS);

	pCmdUI->Enable(t != NULL);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroShowocean() 
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(OCEAN, !t->GetFeatureVisible(OCEAN));
}

void EnviroView::OnUpdateEnviroShowocean(CCmdUI* pCmdUI) 
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(OCEAN);

	pCmdUI->Enable(t != NULL);
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroShowsky() 
{
	vtSkyDome *sky = GetTerrainScene().m_pSkyDome;
	if (!sky) return;
	bool on = sky->GetEnabled();
	sky->SetEnabled(!on);
}

void EnviroView::OnUpdateEnviroShowsky(CCmdUI* pCmdUI) 
{
	vtSkyDome *sky = GetTerrainScene().m_pSkyDome;
	if (!sky) return;
	bool on = sky->GetEnabled();
	pCmdUI->SetCheck(on);
}

void EnviroView::OnEnviroShowfog() 
{
	GetTerrainScene().ToggleFog();
}

void EnviroView::OnUpdateEnviroShowfog(CCmdUI* pCmdUI) 
{
#if 1
	// not really implemented yet
	pCmdUI->Enable(false);
#else
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	if (vtGetScene()->GetFog() == NULL)
	{
		pCmdUI->SetCheck(false);
		pCmdUI->Enable(false);
	}
	else
		pCmdUI->SetCheck(!GetTerrainScene().m_bFog);
#endif
}

void EnviroView::OnEnviroShowtrees() 
{
	vtTerrain *t = GetCurrentTerrain();
	if (t) t->SetFeatureVisible(VEGETATION, !t->GetFeatureVisible(VEGETATION));
}

void EnviroView::OnUpdateEnviroShowtrees(CCmdUI* pCmdUI) 
{
	vtTerrain *t = GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(VEGETATION);

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
	vtDynTerrainGeom *pTerr = GetCurrentTerrain()->m_pDynGeom;
	pTerr->EnableDetail(!pTerr->GetDetail());
}

void EnviroView::OnUpdateShowdetailtexture(CCmdUI* pCmdUI) 
{
	bool bHasIt = (GetCurrentTerrain() && GetCurrentTerrain()->m_pDynGeom != NULL);

	pCmdUI->Enable(bHasIt && g_App.m_state == AS_Terrain);
	if (bHasIt)
	{
		vtDynTerrainGeom *pTerr = GetCurrentTerrain()->m_pDynGeom;
		pCmdUI->SetCheck(pTerr->GetDetail());
	}
	else
		pCmdUI->Enable(false);
}

void EnviroView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ((g_App.m_mode == MM_NAVIGATE || g_App.m_mode == MM_FENCES) &&
		(g_App.m_pTerrainPicker != NULL))
	{
		vtTerrain *ter = GetCurrentTerrain();
		if (ter && ter->GetShowPOI())
		{
			ter->HideAllPOI();
			DPoint2 epos(g_App.m_EarthPos.x, g_App.m_EarthPos.y);
			vtPointOfInterest *poi = ter->FindPointOfInterest(epos);
			if (poi)
				ter->ShowPOI(poi, true);
		}

	}
//	CView::OnMouseMove(nFlags, point);
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
	m_bMaintainHeight = !m_bMaintainHeight;

	if (g_App.m_pTFlyer != NULL)
	{
		g_App.m_pTFlyer->MaintainHeight(m_bMaintainHeight);
		g_App.m_pTFlyer->SetMaintainHeight(0);
	}
}

void EnviroView::OnUpdateViewMaintainheight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(m_bMaintainHeight);
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
	g_App.m_nav = NT_Quake;
	g_App.EnableFlyerEngine(true);
}

void EnviroView::OnUpdateToolsNavigateQuakenavengine(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.m_nav == NT_Quake);
}

void EnviroView::OnToolsNavigateVelocitygravity() 
{
	g_App.m_nav = NT_Gravity;
	g_App.EnableFlyerEngine(true);
}

void EnviroView::OnUpdateToolsNavigateVelocitygravity(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_App.m_state == AS_Terrain);
	pCmdUI->SetCheck(g_App.m_nav == NT_Gravity);
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
	return 1;
}

void EnviroView::OnSize(UINT nType, int cx, int cy) 
{
	vtGetScene()->SetWindowSize(cx, cy);
}

void EnviroView::OnPaint() 
{
    // try to let MFC do some of its idle processing
    LONG lIdle = 0;
    while ( AfxGetApp()->OnIdle(lIdle++ ) );

	if (1)
	{
		CPaintDC dc(this); // device context for painting
		
		// Render the scene
		vtGetScene()->DoUpdate();

		SwapBuffers(dc.m_ps.hdc);
	}

    // try to let MFC do some of its idle processing
    lIdle = 0;
    while ( AfxGetApp()->OnIdle(lIdle++ ) );

	if (m_bRunning)
		InvalidateRect(NULL,FALSE);	//for Continuous Rendering

    // try to let MFC do some of its idle processing
	lIdle = 0;
    while ( AfxGetApp()->OnIdle(lIdle++ ) );
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


