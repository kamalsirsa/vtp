//
// Name:     app.cpp
// Purpose:  The application class for a wxWindows application.
//
// Copyright (c) 2001 Virtual Terrain Project
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

// Header for the vtlib library
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"

#include "app.h"
#include "frame.h"

bool CreateScene();

IMPLEMENT_APP(vtApp)

//
// Initialize the app object
//
bool vtApp::OnInit(void)
{
	// Create the main frame window
	wxString title = "Simple vtlib example";
	vtFrame *frame = new vtFrame(NULL, title,
		wxPoint(50, 50), wxSize(800, 600));

	return CreateScene();
}

//
// Create the 3d scene
//
bool CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();
	pScene->Init();

	// Set the global data path
	StringArray paths;
	paths.Append(new vtString("Data/"));
	vtTerrain::SetDataPath(paths);

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// Create a new terrain scene.  This will contain all the terrain
	// that are created.
	vtTerrainScene *ts = new vtTerrainScene();
	vtRoot *pTopGroup = ts->BeginTerrainScene(false);

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain();
	pTerr->SetParamFile("Data/Simple.ini");

	// Add the terrain to the scene, and contruct it
	ts->AppendTerrain(pTerr);
	int iError;
	if (!pTerr->CreateScene(false, iError))
	{
		wxMessageBox("Terrain creation failed.");
		return false;
	}
	ts->Finish(paths);
	ts->SetTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Flight speed is 500 m/frame
	// Height over terrain is 100 m
	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(400, 100, true);
	pFlyer->SetTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	return true;
}


