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
#include <time.h>		// clock() & CLOCKS_PER_SEC

#ifdef __FreeBSD__
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <iostream>
#include "vtdata/vtLog.h"

using namespace osg;

///////////////////////////////////////////////////////////////
// Trap for OSG messages
//

class OsgMsgTrap : public std::streambuf
{
public:
	inline virtual int_type overflow(int_type c = std::streambuf::traits_type::eof())
	{
		if (c == std::streambuf::traits_type::eof()) return std::streambuf::traits_type::not_eof(c);
		g_Log._Log((char) c);
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
	// Do not release camera, that is left for the application.
}

vtScene *vtGetScene()
{
	return &g_Scene;
}

float vtGetTime()
{
	return g_Scene.GetTime();
}

float vtGetFrameTime()
{
	return g_Scene.GetFrameTime();
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

bool vtScene::Init()
{
	// Redirect cout messages (where OSG sends its messages) to our own log
	std::cout.rdbuf(&g_Trap);
	std::cerr.rdbuf(&g_Trap);

	SetCamera(new vtCamera());

	m_pOsgSceneView = new osgUtil::SceneView();
	m_pOsgSceneView->setDefaults();

	// OSG 0.9.0 and newer
	m_pOsgSceneView->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

	m_pOsgSceneView->setLightingMode(osgUtil::SceneView::NO_SCENEVIEW_LIGHT);

	// OSG 0.9.2 and newer
	m_pOsgSceneView->setCullingMode( m_pOsgSceneView->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);

	m_bInitialized = true;

	_initialTick = _timer.tick();
	_frameTick = _initialTick;

	return true;
}

void vtScene::Shutdown()
{
	osgDB::Registry::instance()->clearObjectCache();
}

void vtScene::DoUpdate()
{
	if (!m_bInitialized)
		return;

	_lastFrameTick = _frameTick;
	_frameTick = _timer.tick();

	DoEngines();

	// As of OSG 0.9.5, we need to store our own camera params and recreate
	//  the projection matrix each frame.
	float aspect;
	if (m_WindowSize.x == 0 || m_WindowSize.y == 0)		// safety
		aspect = 1.0;
	else
		aspect = (float) m_WindowSize.x / m_WindowSize.y;

	if (m_pCamera->IsOrtho())
	{
		// Arguments are left, right, bottom, top, zNear, zFar
		float w2 = m_pCamera->GetWidth() /2;
		float h2 = w2 / aspect;
		m_pOsgSceneView->setProjectionMatrixAsOrtho(-w2, w2, -h2, h2,
			m_pCamera->GetHither(), m_pCamera->GetYon());
	}
	else
	{
		float fov_x = m_pCamera->GetFOV();
		float a = tan (fov_x/2);
		float b = a / aspect;
		float fov_y_div2 = atan(b);
		float fov_y_deg = RadiansToDegrees(fov_y_div2 * 2);

		m_pOsgSceneView->setProjectionMatrixAsPerspective(fov_y_deg,
			aspect, m_pCamera->GetHither(), m_pCamera->GetYon());
	}

	// And apply the rotation and translation of the camera itself
	osg::Matrix &mat2 = m_pCamera->m_pTransform->getMatrix();
	osg::Matrix imat;
	imat.invert(mat2);
	m_pOsgSceneView->setViewMatrix(imat);

	CalcCullPlanes();

	m_pOsgSceneView->setViewport(0, 0, m_WindowSize.x, m_WindowSize.y);
	m_pOsgSceneView->cull();
	m_pOsgSceneView->draw();
}

void vtScene::SetRoot(vtGroup *pRoot)
{
	m_pOsgSceneRoot = pRoot->GetOsgGroup();
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

// Debugging helper
void LogCullPlanes(FPlane *planes)
{
	for (int i = 0; i < 4; i++)
		VTLOG(" plane %d: %.3f %.3f %.3f %.3f\n", i, planes[i].x, planes[i].y, planes[i].z, planes[i].w);
	VTLOG("\n");
}

void vtScene::CalcCullPlanes()
{
#if 0
	// Non-API-Specific code - will work correctly as long as the Camera
	// methods are fully functional.
	FMatrix4 mat;
	m_pCamera->GetTransform1(mat);

	assert(( m_WindowSize.x > 0 ) && ( m_WindowSize.y > 0 ));

	double fov = m_pCamera->GetFOV();

	double aspect = (float)m_WindowSize.y / m_WindowSize.x;
	double hither = m_pCamera->GetHither();

	double a = hither * tan(fov / 2);
	double b = a * aspect;

	FPoint3 vec_l(-a, 0, -hither);
	FPoint3 vec_r(a, 0, -hither);
	FPoint3 vec_t(0, b, -hither);
	FPoint3 vec_b(0, -b, -hither);

	vec_l.Normalize();
	vec_r.Normalize();
	vec_t.Normalize();
	vec_b.Normalize();

	FPoint3 up(0.0f, 1.0f, 0.0f);
	FPoint3 right(1.0f, 0.0f, 0.0f);

	FPoint3 temp;

	FPoint3 center;
	FPoint3 norm_l, norm_r, norm_t, norm_b;

	temp = up.Cross(vec_l);
	mat.TransformVector(temp, norm_l);

	temp = vec_r.Cross(up);
	mat.TransformVector(temp, norm_r);

	temp = right.Cross(vec_t);
	mat.TransformVector(temp, norm_t);

	temp = vec_b.Cross(right);
	mat.TransformVector(temp, norm_b);

	mat.Transform(FPoint3(0.0f, 0.0f, 0.0f), center);

	// set up m_cullPlanes in world coordinates!
	m_cullPlanes[0].Set(center, norm_l);
	m_cullPlanes[1].Set(center, norm_r);
	m_cullPlanes[2].Set(center, norm_t);
	m_cullPlanes[3].Set(center, norm_b);
#else
	// Get the view frustum clipping planes directly from OSG

	// OSG 0.8.44
//	const ClippingVolume &clipvol = pCam->m_pOsgCamera->getClippingVolume();
	// OSG 0.8.45
//	const ClippingVolume &clipvol = hack_global_state->getClippingVolume();
	// OSG 0.9.0
	// clipvol1 is the global camera frustum (in world coordinates)
//	const Polytope &clipvol1 = pCam->m_pOsgCamera->getViewFrustum();
	// OSG 0.9.6
	// clipvol2 is the camera's frustum after it's been transformed to the
	//		local coordinates.
//	const Polytope &clipvol2 = hack_global_state->getViewFrustum();
//	const Polytope::PlaneList &planes = clipvol2.getPlaneList();
	// Actually no - we can't get the planes from the state, because
	//  the state includes the funny modelview matrix used to scale
	//  the heightfield.  We must get it from the 'scene' instead.

	osg::Matrixd &_projection = m_pOsgSceneView->getProjectionMatrix();
	osg::Matrixd &_modelView = m_pOsgSceneView->getViewMatrix();

	Polytope tope;
	tope.setToUnitFrustum();
	tope.transformProvidingInverse((_modelView)*(_projection));

	const Polytope::PlaneList &planes = tope.getPlaneList();

	int i = 0;
	for (Polytope::PlaneList::const_iterator itr=planes.begin();
		itr!=planes.end(); ++itr)
	{
		// make a copy of the clipping plane
		Plane plane = *itr;

		// extract the OSG plane to our own structure
		Vec4 pvec = plane.asVec4();
		m_cullPlanes[i++].Set(-pvec.x(), -pvec.y(), -pvec.z(), -pvec.w());
	}
#endif

	// For debugging
//	LogCullPlanes(m_cullPlanes);
}


void vtScene::DrawFrameRateChart()
{
	static float fps[100];
	static int s = 0;
	fps[s] = GetFrameRate();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();

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

