
#include "vtlib/vtlib.h"
#include <GL/glut.h>

vtScene::vtScene() : vtSceneBase()
{
	m_pCamera = NULL;
	m_pSglRootNode = NULL;
}

void vtScene::SetBgColor(RGBf color)
{
	m_SglView.setClearColor(sglVec4f(color.r, color.g, color.b, 1));
}

void vtScene::SetAmbient(RGBf color)
{
	// TODO
}

//
// convert window coordinates (in pixels) to a ray from the camera
// in world coordinates
//
bool vtScene::CameraRay(IPoint2 win, FPoint3 &pos, FPoint3 &dir)
{
	// TODO
	return true;
}

float vtScene::GetFrameRate()
{
	// TODO
	return 1.0f;
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


void vtScene::Init()
{
	sgl::initialize();

	SetCamera(new vtCamera());

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
}

//sglCullf	 trav_state;
//sglFrustumf  frustum;	// Set from Camera?

deque<const sglStatelet*> override_statelets;

void vtScene::DoUpdate()
{
	DoEngines();

	// Set camera position to SGL
	float aspect = m_WindowSize.x / m_WindowSize.y;
	float fov_x = m_pCamera->GetFOV();
	float fov_y = fov_x / aspect;

	sglViewPlatform *view_platform  = m_pCamera->m_psglViewPlatform;
#if 0
	sglMat4f  view_matrix;	// could be single or double precision
	sglFrustumf  frustum;

	glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int state_mask = 0;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	view_platform->getCamera().applyProjection();

	view_platform->computeViewMatrix(view_matrix);
	frustum = view_platform->getCamera().getFrustum();

	m_pSglRootNode->preDraw();
	m_pSglRootNode->cull(trav_state,
			  fov_x, fov_y,
			  m_WindowSize.x, m_WindowSize.y,
			  frustum, view_matrix,
			  override_statelets, 0, state_mask);
	trav_state.draw();

	glPopMatrix();
#else
	m_SglView.setScene(m_pSglRootNode);
	m_SglView.setViewPlatform(view_platform);
	m_SglView.setViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
	m_SglView.cullAndDraw();
#endif
	glFlush();
}

void vtScene::SetRoot(vtRoot *pRoot)
{
	m_pSglRootNode = (sglScene *)pRoot;
	m_pRoot = pRoot;
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
	// TODO
	return NULL;
}

