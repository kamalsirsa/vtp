//
// SceneOSG.cpp
//
// Implementation of vtScene for the OSG library
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <osg/LightSource>
#include <osg/PolygonMode>
#include <osg/Switch>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <time.h>		// clock() & CLOCKS_PER_SEC

#ifdef __FreeBSD__
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

///////////////////////////////////////////////////////////////
// Override the CullVisitor class to allow turning off nodes
// based on their NodeMask.  Eventually this capability will
// probably be in the OSG core.
//
class CullVisitor2 : public osgUtil::CullVisitor
{
public:
	void apply(osg::Node& node);
	void apply(osg::Geode& node);
	void apply(osg::Billboard& node);
	void apply(osg::LightSource& node);

	void apply(osg::Group& node);
	void apply(osg::Transform& node);
	void apply(osg::Switch& node);
	void apply(osg::LOD& node);
};

typedef osgUtil::CullVisitor ourv;

void CullVisitor2::apply(osg::Node& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}
void CullVisitor2::apply(osg::Geode& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}
void CullVisitor2::apply(osg::Billboard& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}
void CullVisitor2::apply(osg::LightSource& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}
void CullVisitor2::apply(osg::Group& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}
void CullVisitor2::apply(osg::Transform& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}
void CullVisitor2::apply(osg::Switch& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}
void CullVisitor2::apply(osg::LOD& node)
{
	if (node.getNodeMask())
		ourv::apply(node);
}

///////////////////////////////////////////////////////////////

// There is always and only one global vtScene object
vtScene g_Scene;

vtScene::vtScene() : vtSceneBase()
{
	m_bInitialized = false;
	m_bWireframe = false;
}

vtScene *vtGetScene()
{
	return &g_Scene;
}

float vtGetTime()
{
	float sec;

	// clock(), while ANSI C, is generally a low-res timer on most systems.  
	//   Use a high-res timer if one exists.
#ifdef __FreeBSD__
	struct rusage ru;
	if (getrusage(RUSAGE_SELF, &ru))
		sec = -1.0 / 1000000;
	else
		sec = (ru.ru_utime.tv_sec  + ru.ru_stime.tv_sec ) +
			  (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) / 1000000.0;
#else
	sec = (float)clock() / CLOCKS_PER_SEC;
#endif

	return sec;
}

void vtScene::SetBgColor(RGBf color)
{
	osg::Vec4 color2;
	v2s(color, color2);
	m_pOsgSceneView->setBackgroundColor(color2);
}

void vtScene::SetAmbient(RGBf color)
{
	// TODO?  Or should applications simply set the ambient component of
	// their lights.
}

float vtScene::GetFrameRate()
{
//	return m_FrameTimer.frameRate();
	return m_FrameTimer.frameRateAverge();
}

void vtScene::Init()
{
	SetCamera(new vtCamera());

	m_pOsgSceneView = new osgUtil::SceneView();
	m_pOsgSceneView->setDefaults();
	m_pOsgSceneView->setCalcNearFar(false);

//	osgUtil::SceneView::LightingMode lm= m_pOsgSceneView->getLightingMode();
	m_pOsgSceneView->setLightingMode(osgUtil::SceneView::SKY_LIGHT);
//	m_pOsgSceneView->setLightingMode(osgUtil::SceneView::NO_SCENEVIEW_LIGHT);

	CullVisitor2 *cv = new CullVisitor2;
	cv->setRenderGraph(m_pOsgSceneView->getRenderGraph());
	cv->setRenderStage(m_pOsgSceneView->getRenderStage());
	m_pOsgSceneView->setCullVisitor(cv);

	m_bInitialized = true;

	m_FrameTimer.Init();
}

void vtScene::AddMovLight(vtMovLight *pML)
{
	m_Lights.Append(pML);
	m_pOsgSceneView->setLight(pML->GetLight()->m_pLight);
}

void vtScene::DoUpdate()
{
	if (!m_bInitialized)
		return;

	DoEngines();

	// Copy the light locations from their transform nodes
	// to the real lights
	for (int i = 0; i < m_Lights.GetSize(); i++)
	{
		vtMovLight *m = m_Lights[i];
		vtLight *l = m->GetLight();
		FMatrix4 mat;
		m->GetTransform1(mat);

		// get position
		FPoint3 pos = mat.GetTrans();

		// get direction
		FPoint3 dir1(0.0f, 0.0f, -1.0f), dir;
		mat.TransformVector(dir1, dir);

		pos = -dir;
		l->m_pLight->setPosition(v2s2(pos));
//		l->m_pLight->setDirection(v2s(dir));
	}

	//
	// Copy the camera location from it's transform node
	// to the real camera
	//
	osg::Transform *pTransform = m_pCamera->m_pTransform;
	const osg::Matrix &pCameraMatrix = pTransform->getMatrix();
	osg::Camera *pOsgCam = m_pCamera->m_pOsgCamera;
	osg::Vec3 eye(0.0f, 0.0f, 0.0f), look(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f);
	pOsgCam->setView(eye, look, up);
// osg 8.40
//	pOsgCam->mult(*pOsgCam, pCameraMatrix);
// osg 8.41
	pOsgCam->transformLookAt(pCameraMatrix);

	assert(( m_WindowSize.x > 0 ) && ( m_WindowSize.y > 0 ));
	m_pOsgSceneView->setCamera(pOsgCam);
	m_pOsgSceneView->setViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
	m_pOsgSceneView->cull();
	m_pOsgSceneView->draw();

	m_FrameTimer.updateFrameTick();
}

void vtScene::SetRoot(vtRoot *pRoot)
{
	m_pOsgSceneRoot = pRoot->m_pOsgRoot;
	m_pOsgSceneView->setSceneData(m_pOsgSceneRoot);
	m_pRoot = pRoot;
}

//
// convert window coordinates (in pixels) to a ray from the camera
// in world coordinates
//
bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir)
{
	osg::Vec3 near_point, far_point, diff;

	// call the handy OSG function
	m_pOsgSceneView->projectWindowXYIntoObject(win.x, m_WindowSize.y-win.y, near_point, far_point);

	diff = far_point - near_point;
	diff.normalize();

	s2v(near_point, pos);
	s2v(diff, dir);

	return true;
}

void vtScene::DrawFrameRateChart()
{
	static float fps[100];
	static int s = 0;
	fps[s] = GetFrameRate();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 0.0f);

	glBegin(GL_LINE_STRIP);
	for (int i = 1; i <= 100; i++)
	{
		glVertex3f(-1.0 + i/200.0f, -1.0f + fps[(s+i)%100]/200.0f, 0.0f);
	}
	glEnd();

	s++;
	if (s == 100) s = 0;
}

void vtScene::SetGlobalWireframe(bool bWire)
{
	m_bWireframe = bWire;

	// Set the scene's global PolygonMode attribute, which will affect all
	// other materials in the scene, except those which explicitly override
	// the attribute themselves.
	osg::StateSet *global_state = m_pOsgSceneView->getGlobalState();
	osg::PolygonMode *npm = new osg::PolygonMode();
	if (m_bWireframe)
		npm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	else
		npm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
	global_state->setAttributeAndModes(npm, osg::StateAttribute::ON);
}

bool vtScene::GetGlobalWireframe()
{
	return m_bWireframe;
}

////////////////////////////////////////

vtNode *vtLoadModel(const char *filename)
{
	osg::Node *node = osgDB::readNodeFile(filename);
	if (node)
	{
		vtGroup *pGroup = new vtGroup();
		pGroup->SetName2(filename);
		pGroup->GetOsgGroup()->addChild(node);
		return pGroup;
	}
	else
		return NULL;
}

