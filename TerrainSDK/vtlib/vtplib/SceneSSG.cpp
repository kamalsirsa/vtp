//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <GL/glut.h>
#include <time.h>

vtScene::vtScene() : vtSceneBase()
{
	SetCamera(new vtCamera());
}

void vtScene::Init()
{
	// Initialise SSG
	glColorMaterial(GL_FRONT, GL_DIFFUSE);	// hack for SSG
	ssgInit();
	glDisable(GL_COLOR_MATERIAL);	// hack for SSG

	// Some basic OpenGL setup
	glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Set up the viewing parameters
	ssgSetFOV(60.0f, 0.0f);
	ssgSetNearFar(1.0f, 70.0f);

	// Set up the Sun.
	sgVec3 sunposn;
	sgSetVec3(sunposn, 0.2f, -0.5f, 0.5f);
	ssgGetLight(0 )->setPosition(sunposn);
}

void vtScene::SetBgColor(RGBf color)
{
	// TODO
}

void vtScene::SetAmbient(RGBf color)
{
	// TODO
}

void vtScene::DoUpdate()
{
	m_Clock.update();

	DoEngines();

#if 0
	// debug
	FPoint3 pos = m_pCamera->GetTrans();
	vtString str;
	str.Format("cam: %f, %f, %f\n", pos.x, pos.y, pos.z);
	OutputDebugString(str);
#endif

	// Set camera position to SSG
	sgMat4 xform;
	ssgTransform *pTrans = m_pCamera->GetSsgTransform();
	pTrans->getTransform(xform);
	ssgSetCamera(xform);

	// Cull and draw
	ssgCullAndDraw(m_pSsgRoot);
}

float vtScene::GetFrameRate()
{
	return m_Clock.getFrameRate();
}

void vtScene::SetGlobalWireframe(bool bWire)
{
	// TODO
}

bool vtScene::GetGlobalWireframe()
{
	// TODO
	return false;
}

void vtScene::SetRoot(vtGroup *pRoot)
{
	m_pRoot = pRoot;
	m_pSsgRoot = pRoot->m_pSsgRoot;
}

bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir)
{
	vtCamera *pCam = GetCamera();

	FPoint2 hsize;

	ssgContext *con = ssgGetCurrentContext();
	sgFrustum *frust = con->getFrustum();

	hsize.x = frust->getLeft() * 2.0f;
	hsize.y = frust->getBot() * 2.0f;

	IPoint2	WinSize = GetWindowSize();
	float	mx = (float) win.x / WinSize.x;
	float	my = (float) win.y / WinSize.y;

	// find the direction the pointer is pointing (in camera coordinates)
	FPoint3 cam_dir;
	cam_dir.x = (mx - 0.5f) * hsize.x;
	cam_dir.y = -(my - 0.5f) * hsize.y;
	cam_dir.z = -pCam->GetHither();

	// convert from camera to world coordinates
	FMatrix4	camtrans;
	pCam->GetTransform1(camtrans);
	camtrans.Transform(FPoint3(0, 0, 0), pos);
	camtrans.TransformVector(cam_dir, dir);
	dir.Normalize();
	return true;
}

vtScene g_Scene;

vtScene *vtGetScene()
{
	return &g_Scene;
}

float vtGetTime()
{
	return (float)clock() / CLOCKS_PER_SEC;
}

////////////////////////////////////////

vtNode *vtLoadModel(const char *filename)
{
	ssgEntity *model = ssgLoadAC((char *)filename);

	if (model)	// can read file?
	{
		vtGroup *pGroup = new vtGroup();
		pGroup->GetBranch()->addKid(model);
		return pGroup;
	}
	else
		return NULL;
}

