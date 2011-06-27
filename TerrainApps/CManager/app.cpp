//
// Name:     app.cpp
// Purpose:  The application class the CManager application.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Header for the vtlib librarys
#include "vtlib/vtlib.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/vtSOG.h"
#include "vtui/Helper.h"	// for ConvertArgcArgv
#include "wxosg/GraphicsWindowWX.h"
#include "vtdata/vtLog.h"

#include "app.h"
#include "frame.h"
#include "canvas.h"

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
    m_pTrackball = NULL;

	// Redirect the wxWindows log messages to our own logging stream
	wxLog *logger = new LogCatcher;
	wxLog::SetActiveTarget(logger);

	VTSTARTLOG("debug.txt");
	VTLOG("CManager\n");

	VTLOG("Setup scene\n");
	vtScene *pScene = vtGetScene();

	//
	// Create the main frame window
	//
	VTLOG("Creating frame\n");
	vtFrame *frame = new vtFrame(NULL, _T("Content Manager"), wxPoint(50, 50), wxSize(800, 600));

	int MyArgc;
	char** MyArgv;
	ConvertArgcArgv(wxApp::argc, wxApp::argv, &MyArgc, &MyArgv);
	pScene->Init(MyArgc, MyArgv);

	pScene->SetGraphicsContext(new GraphicsWindowWX(frame->m_canvas));

	pScene->SetBgColor(RGBf(0.5f, 0.5f, 0.5f));		// grey

	// Make sure the scene knows the size of the canvas
	//  (on wxGTK, the first size events arrive too early before the Scene exists)
	wxSize canvas_size = frame->m_canvas->GetClientSize();
	pScene->SetWindowSize(canvas_size.x, canvas_size.y);

	VTLOG(" getting camera\n");
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->setName("Default Camera");

	m_pRoot = new vtGroup;
	m_pRoot->setName("Root");
	pScene->SetRoot(m_pRoot);

	// make a simple directional light
	VTLOG(" creating light\n");
	vtLightSource *pLight = new vtLightSource(0);
	pLight->setName("Light");
	vtTransform *pMovLight = new vtTransform;
	pMovLight->addChild(pLight);
	pMovLight->setName("Movable Light");
	pLight->SetAmbient(RGBf(1, 1, 1));
	pLight->SetDiffuse(RGBf(1, 1, 1));
	pLight->SetSpecular(RGBf(1, 1, 1));
	pMovLight->SetDirection(FPoint3(-0.2, -0.4, -0.9));
	m_pRoot->addChild(pMovLight);

	frame->UseLight(pMovLight);

	// SOG testing, currently disabled
#if 0
#if 0
	// make a yellow sphere
	vtMaterialArray *pMats = new vtMaterialArray;
	pMats->AddRGBMaterial(RGBf(1.0f, 1.0f, 0.0f), RGBf(0.0f, 0.0f, 1.0f));
	vtGeode *pGeode = CreateSphereGeom(pMats, 0, VT_Normals, 0.5, 16);
	pGeode->setName("Yellow Sphere");
	pMats->Release();

	OutputSOG osog;

	FILE *fp = fopen("output.sog", "wb");
	osog.WriteHeader(fp);
	osog.WriteSingleGeometry(fp, pGeode);
	fclose(fp);
	m_pRoot->addChild(pGeode);
#else
	InputSOG isog;

	FILE *fp = fopen("output.sog", "rb");
	vtGroup *pGroup = new vtGroup;
	bool success = isog.ReadContents(fp, pGroup);
	fclose(fp);
	m_pRoot->addChild(pGroup);
#endif
#endif

	// make a trackball controller for the camera
	VTLOG(" creating trackball\n");
	m_pTrackball = new vtTrackball(3.0f);
	m_pTrackball->SetTarget(pScene->GetCamera());
	m_pTrackball->setName("Trackball");
	m_pTrackball->SetRotateButton(VT_LEFT, 0);
	m_pTrackball->SetZoomButton(VT_LEFT|VT_RIGHT, 0);
	m_pTrackball->SetZoomScale(3000.0f);
	m_pTrackball->SetTranslateButton(VT_RIGHT, 0);
	pScene->AddEngine(m_pTrackball);

	// Memleak Testing
//	GetMainFrame()->AddModelFromFile("E:/3D/Sample FLT files/spitfire.flt");
//	osg::Node *pNode = vtLoadModel("E:/3D/Sample FLT files/spitfire.flt");
//	if (pNode)
//		m_pRoot->addChild(pNode);
//	GetMainFrame()->AddNewItem();

//	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("E:/3D/Sample FLT files/spitfire.flt");
//	node = NULL;

	VTLOG(" end of OnInit\n");
	return true;
}


int vtApp::OnExit(void)
{
	VTLOG("App OnExit\n");

	vtGetScene()->SetRoot(NULL);
	m_pRoot = NULL;

	vtGetScene()->Shutdown();
	return 0;
}

