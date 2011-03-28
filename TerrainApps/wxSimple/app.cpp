//
// Name:     app.cpp
// Purpose:  The application class for a wxWindows application.
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Header for the vtlib library
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/vtLog.h"

#include "app.h"
#include "frame.h"

#include "vtui/Helper.h"
#include "vtui/GraphicsWindowWX.h"
#include "canvas.h"

IMPLEMENT_APP(vtApp);

// An engine to wait for frame 5 before building the 3D scene
struct StartupEngine : public vtEngine
{
	StartupEngine(vtApp *a) { app = a; count = 0; }
	void Eval()
	{
		count ++;
		if (count <= 5)
			VTLOG("%d\n", count);
		if (count == 5)
		{
		    const char* version = (const char*) glGetString( GL_VERSION );
			app->CreateScene();
		}
	}
	int count;
	vtApp *app;
};

//
// Initialize the app object
//
bool vtApp::OnInit(void)
{
	m_pTerrainScene = NULL;

	// Create the main frame window
	m_pFrame = new vtFrame(NULL, _T("Simple vtlib example"), wxPoint(50, 50), wxSize(800, 600));

	int MyArgc;
	char** MyArgv;
	ConvertArgcArgv(wxApp::argc, wxApp::argv, &MyArgc, &MyArgv);
	vtGetScene()->Init(MyArgc, MyArgv);

	vtGetScene()->SetGraphicsContext(new GraphicsWindowWX(m_pFrame->m_canvas));

	// Make sure the scene knows the size of the canvas
	//  (on wxGTK, the first size events arrive too early before the Scene exists)
	wxSize canvas_size = m_pFrame->m_canvas->GetClientSize();
	vtGetScene()->SetWindowSize(canvas_size.x, canvas_size.y);

#if 0
	// Force graphics context to be realised
	vtGetScene()->DoUpdate();

	// Need a valid graphics context to for the the texture unit manager to initialise properly
	vtGetScene()->GetGraphicsContext()->makeCurrent();
    const char* version = (const char*) glGetString( GL_VERSION );
	bool bReturn = CreateScene();
	vtGetScene()->GetGraphicsContext()->releaseContext();
	return bReturn;
#else
	vtGetScene()->AddEngine(new StartupEngine(this));
	return true;
#endif
}

//
// Create the 3d scene
//
bool vtApp::CreateScene()
{
	VTSTARTLOG("debug.txt");

	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	m_pTerrainScene = new vtTerrainScene;

	// Set the global data path
	vtStringArray paths;
	paths.push_back(vtString("../Data/"));
	paths.push_back(vtString("Data/"));
	pScene->SetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = m_pTerrainScene->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile("Data/Simple.xml");
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	m_pTerrainScene->AppendTerrain(pTerr);
	if (!m_pTerrainScene->BuildTerrain(pTerr))
	{
		wxMessageBox(_T("Couldn't create the terrain.  Perhaps the elevation data file isn't in the expected location?"));
		return false;
	}
	m_pTerrainScene->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->SetTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->SetTarget(pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

	printf("Done creating scene.\n");
	return true;
}

int vtApp::OnExit()
{
	if (m_pTerrainScene)
	{
		// Clean up the scene
		vtGetScene()->SetRoot(NULL);
		m_pTerrainScene->CleanupScene();
		delete m_pTerrainScene;
		vtGetScene()->Shutdown();
	}
	return 0;
}
