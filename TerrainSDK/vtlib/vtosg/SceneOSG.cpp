//
// SceneOSG.cpp
//
// Implementation of vtScene for the OSG library
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <osg/LightSource>
#include <osg/PolygonMode>
#include <osg/Switch>
#include <osg/Fog>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <time.h>		// clock() & CLOCKS_PER_SEC

#ifdef __FreeBSD__
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <streambuf>
#include "vtdata/vtLog.h"

using namespace osg;

///////////////////////////////////////////////////////////////
// Trap for OSG messages
//

class OsgMsgTrap : public std::streambuf
{
public:
	inline virtual int_type overflow(int_type c = traits_type::eof())
	{
		if (c == traits_type::eof()) return traits_type::not_eof(c);
		char str[2];
		str[0] = c;
		str[1] = 0;
		g_Log._Log(str);
		return c;
	}
} g_Trap;

///////////////////////////////////////////////////////////////

// There is always and only one global vtScene object
vtScene g_Scene;

vtScene::vtScene() : vtSceneBase()
{
	m_bInitialized = false;
	m_bWireframe = false;
	m_bWinInfo = false;
}

vtScene::~vtScene()
{
	if (m_pCamera)
		m_pCamera->Destroy();
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
	Vec4 color2;
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
	return (float) frameRate();
}

bool vtScene::Init()
{
	// Redirect cout messages (where OSG sends its messages) to our own log
	std::cout.rdbuf(&g_Trap);
	std::cerr.rdbuf(&g_Trap);

	SetCamera(new vtCamera());

	m_pOsgSceneView = new osgUtil::SceneView();
	m_pOsgSceneView->setDefaults();

	// OSG 0.8.45 and before
//	m_pOsgSceneView->setCalcNearFar(false);

	// OSG 0.9.0 and newer
	m_pOsgSceneView->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

//	m_pOsgSceneView->setLightingMode(osgUtil::SceneView::SKY_LIGHT);
	m_pOsgSceneView->setLightingMode(osgUtil::SceneView::NO_SCENEVIEW_LIGHT);

	// OSG 0.8.45
//	osgUtil::CullVisitor *cvis = m_pOsgSceneView->getCullVisitor();
//	osgUtil::CullVisitor::CullingMode mode = cvis->getCullingMode();
//	mode &= ~(osgUtil::CullVisitor::SMALL_FEATURE_CULLING);
//	cvis->setCullingMode(mode);

	// OSG 0.9.2
	m_pOsgSceneView->setCullingMode( m_pOsgSceneView->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);

	m_bInitialized = true;

	_initialTick = _timer.tick();
	_frameTick = _initialTick;

	return true;
}

void vtScene::DoUpdate()
{
	if (!m_bInitialized)
		return;

	_lastFrameTick = _frameTick;
	_frameTick = _timer.tick();

	DoEngines();

	Camera *pOsgCam = m_pCamera->m_pOsgCamera.get();

	// let the OSG Camera know that its transform has (probably) changed
	// apparently no longer needed as of OSG 0.8.45
//	pOsgCam->dirtyTransform();

	// Copy the camera transform position to camera.  With OSG 0.9.2, we
	// used to be able to do this one call to Camera::attachTransform.
	// With 0.9.3, we have to do it each frame:
	static RefMatrix *cmat = NULL;
	if (cmat == NULL) cmat = new RefMatrix;
	Matrix &rcmat = *cmat;
	rcmat = m_pCamera->m_pTransform->getMatrix();
	pOsgCam->attachTransform(Camera::EYE_TO_MODEL, cmat);

	m_pOsgSceneView->setCamera(pOsgCam);
	m_pOsgSceneView->setViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
	m_pOsgSceneView->cull();
	m_pOsgSceneView->draw();
}

void vtScene::SetRoot(vtRoot *pRoot)
{
	m_pOsgSceneRoot = pRoot->m_pOsgRoot;
	m_pOsgSceneView->setSceneData(m_pOsgSceneRoot.get());
	m_pRoot = pRoot;
}

//
// convert window coordinates (in pixels) to a ray from the camera
// in world coordinates
//
bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir)
{
	Vec3 near_point, far_point, diff;

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
	StateSet *global_state = m_pOsgSceneView->getGlobalStateSet();
	PolygonMode *npm = new PolygonMode();
	if (m_bWireframe)
		npm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
	else
		npm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::FILL);
	global_state->setAttributeAndModes(npm, StateAttribute::ON);
}

bool vtScene::GetGlobalWireframe()
{
	return m_bWireframe;
}


////////////////////////////////////////

vtNodeBase *vtLoadModel(const char *filename)
{
	// Temporary workaround for OSG OBJ-MTL reader which doesn't like
	// backslashes in the version we're using
	char newname[500];
	strcpy(newname, filename);
	for (unsigned int i = 0; i < strlen(filename); i++)
	{
		if (newname[i] == '\\') newname[i] = '/';
	}

	// We must insert a 'Normalize' state above the geometry objets
	// that we load, otherwise when they are scaled, the vertex normals
	// will cause strange lighting.  Fortunately, we only need to create
	// a single State object which is shared by all loaded models.
	static 	StateSet *normstate = NULL;
	if (!normstate)
	{
		normstate = new StateSet;
		normstate->setMode(GL_NORMALIZE, StateAttribute::ON);
	}

	Node *node = osgDB::readNodeFile(newname);
	if (node)
	{
		vtGroup *pGroup = new vtGroup();
		pGroup->SetName2(newname);
		pGroup->GetOsgGroup()->addChild(node);
		pGroup->GetOsgNode()->setStateSet(normstate);
		return pGroup;
	}
	else
		return NULL;
}

RGBf vtNodeBase::s_white(1, 1, 1);

/**
 * Set the Fog state for a node.
 *
 * You can turn fog on or off.  When you turn fog on, it affects this node
 * and all others below it in the scene graph.
 *
 * \param bOn True to turn fog on, false to turn it off.
 * \param start The distance from the camera at which fog starts, in meters.
 * \param end The distance from the camera at which fog end, in meters.  This
 *		is the point at which it becomes totally opaque.
 * \param color The color of the fog.  All geometry will be faded toward this
 *		color.
 * \param iType Can be GL_LINEAR, GL_EXP or GL_EXP2 for linear or exponential
 *		increase of the fog density.
 */
void vtNode::SetFog(bool bOn, float start, float end, const RGBf &color, int iType)
{
	osg::StateSet *set = GetOsgNode()->getStateSet();
	if (!set)
	{
		set = new osg::StateSet;
		GetOsgNode()->setStateSet(set);
	}

	if (bOn)
	{
		Fog::Mode eType;
		switch (iType)
		{
		case GL_LINEAR: eType = Fog::LINEAR; break;
		case GL_EXP: eType = Fog::EXP; break;
		case GL_EXP2: eType = Fog::EXP2; break;
		default: return;
		}
		Fog *fog = new Fog;
		fog->setMode(eType);
		fog->setDensity(0.25f);	// not used for linear
		fog->setStart(start);
		fog->setEnd(end);
		fog->setColor(osg::Vec4(color.r, color.g, color.b, 1));

		set->setAttributeAndModes(fog, StateAttribute::OVERRIDE | StateAttribute::ON);
	}
	else
	{
		// turn fog off
		set->setModeToInherit(GL_FOG);
	}
}


