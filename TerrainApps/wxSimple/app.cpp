//
// Name:     app.cpp
// Purpose:  The application class for a wxWindows application.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
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

#include "app.h"
#include "frame.h"
#include "canvas.h"

IMPLEMENT_APP(vtApp);

//
// Initialize the app object
//
bool vtApp::OnInit(void)
{
	vtGetScene()->Init();

	// Create the main frame window
	m_frame = new vtFrame(NULL, _T("Simple vtlib example"), wxPoint(50, 50), wxSize(800, 600));

	return CreateScene();
}

//
// Create the 3d scene
//
bool vtApp::CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();

	// Look up the camera
	m_pCamera = pScene->GetCamera();
	m_pCamera->SetHither(10);
	m_pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	vtTerrainScene *ts = new vtTerrainScene;

	// Set the global data path
	vtStringArray paths;
	paths.push_back(vtString("Data/"));
	ts->SetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = ts->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile("Data/Simple.xml");

	// Add the terrain to the scene, and contruct it
	m_pTerrainScene->AppendTerrain(pTerr);
	if (!m_pTerrainScene->BuildTerrain(pTerr))
	{
		m_frame->m_canvas->m_bRunning = false;
		wxMessageBox(_T("Couldn't create the terrain.  Perhaps the elevation data file isn't in the expected location?"));
		return false;
	}
	m_pTerrainScene->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->SetTarget(m_pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->SetTarget(m_pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

	printf("Done creating scene.\n");
	return true;
}

int vtApp::OnExit()
{
	if (m_pTerrainScene)
	{
		m_pCamera->Release();

		// Clean up the scene
		vtGetScene()->SetRoot(NULL);
		m_pTerrainScene->CleanupScene();
		delete m_pTerrainScene;
		vtGetScene()->Shutdown();
	}
	return 0;
}
