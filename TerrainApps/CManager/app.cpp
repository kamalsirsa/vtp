//
// Name:     app.cpp
// Purpose:  The application class the CManager application.
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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/vtSOG.h"
#include "vtdata/vtLog.h"

#include "app.h"
#include "frame.h"

static void Args(int argc, wxChar **argv)
{
   return;
}

IMPLEMENT_APP(vtApp)

//
// Initialize the app object
//
bool vtApp::OnInit(void)
{
	Args(argc, argv);

	g_Log._StartLog("debug.txt");
	VTLOG("CManager\n");

	//
	// Create the main frame window
	//
	VTLOG("Creating frame\n");
	wxString title = _T("Content Manager");
	vtFrame *frame = new vtFrame(NULL, title,
		wxPoint(50, 50), wxSize(800, 600));

	VTLOG("Setup scene\n");
	vtScene *pScene = vtGetScene();
	pScene->Init();
	pScene->SetBgColor(RGBf(0.5f, 0.5f, 0.5f));

	VTLOG(" creating camera\n");
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetName2("Default Camera");

	m_pRoot = new vtGroup();
	pScene->SetRoot(m_pRoot);

#if VTLIB_SGL
	CreateTestSGLScene();
#endif

	// make a simple directional light
	VTLOG(" creating light\n");
	vtLight *pLight = new vtLight();
	pLight->SetName2("Light");
	vtMovLight *pMovLight = new vtMovLight(pLight);
	pMovLight->SetName2("Movable Light");
	pLight->SetAmbient(RGBf(1, 1, 1));
	pMovLight->SetTrans(FPoint3(0.0f, 0.0f, 5.0f));
	m_pRoot->AddChild(pMovLight);

	// SOG testing, currently disabled
#if 0
#if 0
	// make a yellow sphere
	vtMaterialArray *pMats = new vtMaterialArray();
	pMats->AddRGBMaterial(RGBf(1.0f, 1.0f, 0.0f), RGBf(0.0f, 0.0f, 1.0f));
	vtGeom *pGeom = CreateSphereGeom(pMats, 0, VT_Normals, 0.5, 16);
	pGeom->SetName2("Yellow Sphere");
	pMats->Release();

	OutputSOG osog;

	FILE *fp = fopen("output.sog", "wb");
	osog.WriteHeader(fp);
	osog.WriteSingleGeometry(fp, pGeom);
	fclose(fp);
	m_pRoot->AddChild(pGeom);
#else
	InputSOG isog;

	FILE *fp = fopen("output.sog", "rb");
	vtGroup *pGroup = new vtGroup;
	bool success = isog.ReadContents(fp, pGroup);
	fclose(fp);
	m_pRoot->AddChild(pGroup);
#endif
#endif

	// make a trackball controller for the camera
	VTLOG(" creating trackball\n");
	m_pTrackball = new vtTrackball(3.0f);
	m_pTrackball->SetTarget(pScene->GetCamera());
	m_pTrackball->SetName2("Trackball");
	m_pTrackball->SetRotateButton(VT_LEFT, 0);
	m_pTrackball->SetZoomButton(VT_LEFT|VT_RIGHT, 0);
	m_pTrackball->SetZoomScale(3000.0f);
	m_pTrackball->SetTranslateButton(VT_RIGHT, 0);
	pScene->AddEngine(m_pTrackball);

	// Memleak Testing
//	GetMainFrame()->AddModelFromFile("E:/3D/IDA Free Models/schoolbus.flt");
//	GetMainFrame()->AddNewItem();

	VTLOG(" end of OnInit\n");
	return TRUE;
}


int vtApp::OnExit(void)
{
	vtCamera *pCamera = vtGetScene()->GetCamera();
	pCamera->Release();

	m_pRoot->Release();
	return 0;
}

