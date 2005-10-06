//
// Name:     app.cpp
// Purpose:  The application class the CManager application.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

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

// #include <osgDB/ReadFile>

static void Args(int argc, wxChar **argv)
{
   return;
}

IMPLEMENT_APP(vtApp)

class LogCatcher : public wxLog
{
    void DoLogString(const wxChar *szString, time_t t)
	{
		VTLOG(" wxLog: ");
		VTLOG(szString);
		VTLOG("\n");
	}
};


//
// Initialize the app object
//
bool vtApp::OnInit(void)
{
	// Redirect the wxWindows log messages to our own logging stream
	wxLog *logger = new LogCatcher();
	wxLog::SetActiveTarget(logger);

	Args(argc, argv);

	VTSTARTLOG("debug.txt");
	VTLOG("CManager\n");

	VTLOG("Setup scene\n");
	vtScene *pScene = vtGetScene();
	pScene->Init();
	pScene->SetBgColor(RGBf(0.5f, 0.5f, 0.5f));

	//
	// Create the main frame window
	//
	VTLOG("Creating frame\n");
	wxString title = _T("Content Manager");
	vtFrame *frame = new vtFrame(NULL, title,
		wxPoint(50, 50), wxSize(800, 600));

	VTLOG(" creating camera\n");
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetName2("Default Camera");

	m_pRoot = new vtGroup();
	m_pRoot->SetName2("Root");
	pScene->SetRoot(m_pRoot);

#if VTLIB_SGL
	CreateTestSGLScene();
#endif

	// make a simple directional light
	VTLOG(" creating light\n");
	vtLight *pLight = new vtLight();
	pLight->SetName2("Light");
	vtTransform *pMovLight = new vtTransform;
	pMovLight->AddChild(pLight);
	pMovLight->SetName2("Movable Light");
	pLight->SetAmbient(RGBf(1, 1, 1));
	pLight->SetDiffuse(RGBf(1, 1, 1));
	pMovLight->SetDirection(FPoint3(-0.2, -0.4, -0.9));
	m_pRoot->AddChild(pMovLight);

	frame->UseLight(pMovLight);

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
//	GetMainFrame()->AddModelFromFile("E:/3D/Sample FLT files/spitfire.flt");
//	vtNode *pNode = vtNode::LoadModel("E:/3D/Sample FLT files/spitfire.flt");
//	if (pNode)
//		m_pRoot->AddChild(pNode);
//	GetMainFrame()->AddNewItem();

//	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("E:/3D/Sample FLT files/spitfire.flt");
//	node = NULL;

	VTLOG(" end of OnInit\n");
	return TRUE;
}


int vtApp::OnExit(void)
{
	VTLOG("App OnExit\n");

	vtGetScene()->SetRoot(NULL);
	m_pRoot->Release();

	vtGetScene()->Shutdown();
	return 0;
}

