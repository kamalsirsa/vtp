//
// In the PSM port, myWorld3D is the "world" which creates the scene.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "ogl/psgdev.h"
#include "vtlib/vtlib.h"

class myWorld3D : public PSWorld3D
{
public:
	myWorld3D() : PSWorld3D() { DoAsyncLoad = false; };
	PSScene*	NewScene(PSWindow = NULL);
#ifdef VTLIB_PSMDIB
	bool		OnInit();
#endif
};

void _psm_MakeWorld()
{
	VCoreInit();
	myWorld3D* world = new myWorld3D();
	world->IncUse();
}

class myScene : public PSDualScene<PSGLScene>
{
public:
	virtual void	DoSimulation();						//!< evaluate simulation engines for this frame.
};

void myScene::DoSimulation()
{
	vtGetScene()->DoUpdate();
}

//
// Replace PSCamera in display scene with a vtCamera
//
PSScene* myWorld3D::NewScene(PSWindow win)
{
	myScene*	scene;
	scene = new myScene;
	scene->SetNumThreads(0);

	scene->SetOptions(SCENE_ClearAll | SCENE_StateSort | SCENE_DoubleBuffer);
	if (win && !scene->SetWindow(win))
	{
		scene->Delete();
		return NULL;
	}

	vtCamera* vtcam = vtGetScene()->GetCamera();
	scene->SetCamera(vtcam);

	vtGetScene()->m_pPSS = scene;

	return scene;
}

#ifdef VTLIB_PSMDIB
/*
 * Cause it to load .BMP file using _psm_ReadDIB function.
 */
bool myWorld3D::OnInit()
{
	if (PSWorld3D::OnInit())
	{
		m_LoadQueue->SetFileFunc("bmp", &_psm_ReadDIB);
		return true;
	}
	return false;
}
#endif
