//
// SceneOSG.cpp
//
// Implementation of vtScene for the OSG library
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#if OLD_OSG_SHADOWS
#include "StructureShadowsOSG.h"
#endif

#include <osgViewer/ViewerEventHandlers>

#include <osg/PolygonMode>	// SetGlobalWireframe
#include <osgDB/Registry>	// for clearObjectCache

#ifdef __FreeBSD__
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <iostream>			// For redirecting OSG's stdout messages
#include "vtdata/vtLog.h"	// to the VTP log.

/** A way to catch OSG messages */
class OsgMsgTrap : public std::streambuf
{
public:
	inline virtual int_type overflow(int_type c = std::streambuf::traits_type::eof())
	{
		if (c == std::streambuf::traits_type::eof()) return std::streambuf::traits_type::not_eof(c);
		VTLOG1((char) c);
		return c;
	}
} g_Trap;

// preserve and restore
static std::streambuf *previous_cout;
static std::streambuf *previous_cerr;

///////////////////////////////////////////////////////////////

/// There one and only global vtScene object
vtScene g_Scene;


///////////////////////////////////////////////////////////////////////

vtScene::vtScene()
{
	m_pCamera = NULL;
	m_pRoot = NULL;
	m_pRootEngine = NULL;
	m_pRootEnginePostDraw = NULL;
	m_piKeyState = NULL;
	m_pDefaultCamera = NULL;
	m_pDefaultWindow = NULL;

	m_bInitialized = false;
	m_bWireframe = false;
	m_bWinInfo = false;
	m_pHUD = NULL;
}

vtScene::~vtScene()
{
	// Cleanup engines.  They are in a tree, connected by ref_ptr, so we only need
	//  to release the top of the tree.
	m_pRootEngine = NULL;

	m_pOsgViewer = NULL;	// derefs

	// Do not release camera or window, that is left for the application.
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

int vtGetMaxTextureSize()
{
	GLint tmax = 0;	// TODO: cannot make direct GL calls in threaded environment
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tmax);
	return tmax;
}

/**
 * Initialize the vtlib library, including the display and scene graph.
 * You should call this function only once, before any other vtlib calls.
 *
 * \param argc, argv Command-line arguments.
 * \param bStereo True for a stereo display output.
 * \param iStereoMode Currently for vtosg, supported values are 0 for
 *		Anaglyphic (red-blue) and 1 for Quad-buffer (shutter glasses).
 */
bool vtScene::Init(int argc, char** argv, bool bStereo, int iStereoMode)
{
	VTLOG1("vtScene::Init\n");

	// Redirect cout messages (where OSG sends its messages) to our own log
	previous_cout =  std::cout.rdbuf(&g_Trap);
	previous_cerr = std::cerr.rdbuf(&g_Trap);

#if 0
	// If you encounter trouble in OSG that you want to debug, enable this
	//  to get a LOT of diagnostic messages from OSG.
	osg::setNotifyLevel(osg::INFO);
#endif

	m_pDefaultCamera = new vtCamera;
	m_pDefaultWindow = new vtWindow;
	SetCamera(m_pDefaultCamera);
	AddWindow(m_pDefaultWindow);

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

	m_pOsgViewer = new osgViewer::Viewer(arguments);

	m_pOsgViewer->setDisplaySettings(osg::DisplaySettings::instance());
	if (bStereo)
	{
		osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
		displaySettings->setStereo(true);
		osg::DisplaySettings::StereoMode mode;
		if (iStereoMode == 0) mode = osg::DisplaySettings::ANAGLYPHIC;
		if (iStereoMode == 1) mode = osg::DisplaySettings::QUAD_BUFFER;
		displaySettings->setStereoMode(mode);
	}
#ifdef __DARWIN_OSX__
	// Kill multi-threading on OSX until wxGLContext properly implemented on that platform
	m_pOsgViewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
#endif
#ifdef USE_OSG_STATS
	osgViewer::StatsHandler* pStatsHandler = new osgViewer::StatsHandler;
	pStatsHandler->setKeyEventPrintsOutStats(0);
	pStatsHandler->setKeyEventTogglesOnScreenStats('x'); // I dont think this is used for anything else at the moment
	m_pOsgViewer->addEventHandler(pStatsHandler);
#endif

	// Kill the event visitor (saves a scenegraph traversal)
	// This will need to be restored if we need to use FRAME events etc. in the scenegraph
	m_pOsgViewer->setEventVisitor(NULL);

	if (bStereo)
	{
		// displaySettings->getScreenDistance(); default is 0.5
		// m_pOsgSceneView->getFusionDistanceMode(); default is PROPORTIONAL_TO_SCREEN_DISTANCE
		// m_pOsgSceneView->getFusionDistanceValue(); default is 1.0
		// The FusionDistanceValue is only used for USE_FUSION_DISTANCE_VALUE & PROPORTIONAL_TO_SCREEN_DISTANCE modes.

		// We use real-world units for fusion distance value
		m_pOsgViewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, 100.0f);
	}

	// From the OSG mailing list: You must specify the lighting mode in
	// setDefaults() and override the default options. If you call
	// setDefaults() with the default options, a headlight is added to the
	// global state set of the SceneView.  With the default options applied,
	// I have tried subsequently calling setLightingMode(NO_SCENE_LIGHT)
	// and setLight(NULL), but I still get a headlight.
	m_pOsgViewer->setLightingMode(osg::View::NO_LIGHT);
	m_pOsgViewer->getCamera()->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
	m_pOsgViewer->getCamera()->setCullingMode(m_pOsgViewer->getCamera()->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING);

	// We maintain a node between OSG's viewer/camera and the vtlib Root, to
	//  control global state
	m_StateRoot = new osg::Group;
	m_pOsgViewer->setSceneData(m_StateRoot);

	m_bInitialized = true;

	_initialTick = _timer.tick();
	_frameTick = _initialTick;

	return true;
}

void vtScene::Shutdown()
{
	VTLOG("vtScene::Shutdown\n");
	m_pDefaultCamera = NULL;
	m_pCamera = NULL;

	delete m_pDefaultWindow;
	m_pDefaultWindow = NULL;
	m_Windows.Empty();

	// Also clear the OSG cache
	osgDB::Registry::instance()->clearObjectCache();

	m_pRoot = NULL;
	m_pRootEnginePostDraw = NULL;
	m_piKeyState = NULL;

	// remove our hold on refcounted objects
	m_pOsgViewer = NULL;
	m_pGraphicsContext = NULL;

	// restore
	std::cout.rdbuf(previous_cout);
	std::cerr.rdbuf(previous_cerr);
}

void vtScene::OnMouse(vtMouseEvent &event, vtWindow *pWindow)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnMouse(event);
	}
}

void vtScene::OnKey(int key, int flags, vtWindow *pWindow)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnKey(key, flags);
	}
}

bool vtScene::GetKeyState(int key)
{
	if (m_piKeyState)
		return m_piKeyState[key];
	else
		return false;
}

IPoint2 vtScene::GetWindowSize(vtWindow *pWindow)
{
	if (!pWindow)
		pWindow = GetWindow(0);
	return pWindow->GetSize();
}

void vtScene::DoEngines(vtEngine *eng)
{
	// Evaluate Engines
	vtEngineArray list(eng);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled())
			pEng->Eval();
	}
}

// (for backward compatibility only)
void vtScene::AddEngine(vtEngine *ptr)
{
	if (m_pRootEngine)
		m_pRootEngine->AddChild(ptr);
	else
		m_pRootEngine = ptr;
}

void vtScene::TargetRemoved(osg::Referenced *tar)
{
	// Look at all Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		// If this engine targets something that is no longer there
		vtEngine *pEng = list[i];
		for (unsigned int j = 0; j < pEng->NumTargets(); j++)
		{
			// Then remove it
			if (pEng->GetTarget(j) == tar)
				pEng->RemoveTarget(tar);
		}
	}
}

void vtScene::TimerRunning(bool bRun)
{
	if (!bRun)
	{
		// stop timer, count how much running time has already elapsed
		m_fAccumulatedFrameTime = _timer.delta_s(_lastRunningTick,_timer.tick());
		//VTLOG("partial frame: %lf seconds\n", m_fAccumulatedFrameTime);
	}
	else
		// start again
		_lastRunningTick = _timer.tick();
}

void vtScene::UpdateBegin()
{
	_lastFrameTick = _frameTick;
	_frameTick = _timer.tick();

	// finish counting the split frame's elapsed time
	if (_lastRunningTick != _lastFrameTick)
	{
		m_fAccumulatedFrameTime += _timer.delta_s(_lastRunningTick,_frameTick);
		//VTLOG("   full frame: %lf seconds\n", m_fAccumulatedFrameTime);
		m_fLastFrameTime = m_fAccumulatedFrameTime;
	}
	else
		m_fLastFrameTime = _timer.delta_s(_lastFrameTick,_frameTick);

	_lastRunningTick = _frameTick;
}

void vtScene::UpdateEngines()
{
	if (!m_bInitialized) return;
	DoEngines(m_pRootEngine);
}

void vtScene::PostDrawEngines()
{
	if (!m_bInitialized) return;
	DoEngines(m_pRootEnginePostDraw);
}

void vtScene::UpdateWindow(vtWindow *pWindow)
{
	if (!m_bInitialized) return;

	// window background color
	osg::Vec4 color2;
	v2s(pWindow->GetBgColor(), color2);
	m_pOsgViewer->getCamera()->setClearColor(color2);

	// window size
	IPoint2 winsize = pWindow->GetSize();
	if (winsize.x == 0 || winsize.y == 0)
	{
		VTLOG("Warning: winsize %d %d\n", winsize.x, winsize.y);
		return;
	}
	m_pOsgViewer->getCamera()->setViewport(0, 0, winsize.x, winsize.y);

	// As of OSG 0.9.5, we need to store our own camera params and recreate
	//  the projection matrix each frame.
	float aspect;
	if (winsize.x == 0 || winsize.y == 0)		// safety
		aspect = 1.0;
	else
		aspect = (float) winsize.x / winsize.y;

	if (m_pCamera->IsOrtho())
	{
		// Arguments are left, right, bottom, top, zNear, zFar
		float w2 = m_pCamera->GetWidth() /2;
		float h2 = w2 / aspect;
		m_pOsgViewer->getCamera()->setProjectionMatrixAsOrtho(-w2, w2, -h2, h2,
			m_pCamera->GetHither(), m_pCamera->GetYon());
	}
	else
	{
		float fov_x = m_pCamera->GetFOV();
		float a = tan (fov_x/2);
		float b = a / aspect;
		float fov_y_div2 = atan(b);
		float fov_y_deg = osg::RadiansToDegrees(fov_y_div2 * 2);

		m_pOsgViewer->getCamera()->setProjectionMatrixAsPerspective(fov_y_deg,
			aspect, m_pCamera->GetHither(), m_pCamera->GetYon());
	}

	// And apply the rotation and translation of the camera itself
	const osg::Matrix &mat2 = m_pCamera->getMatrix();
	osg::Matrix imat;
	imat.invert(mat2);
	m_pOsgViewer->getCamera()->setViewMatrix(imat);

	CalcCullPlanes();

	m_pOsgViewer->getCamera()->setCullMask(0x3);
	m_pOsgViewer->frame();
}

/**
 * Compute the full current view transform as a matrix, which includes
 * the projection of the camera and the transform to window coordinates.
 *
 * This transform is the one used to convert XYZ points in world coodinates
 * into XY window coordinates.
 *
 * By inverting this matrix, you can "un-project" window coordinates back
 * into the world.
 *
 * \param mat This matrix will receive the current view transform.
 */
void vtScene::ComputeViewMatrix(FMatrix4 &mat)
{
	osg::Matrix _viewMatrix = m_pOsgViewer->getCamera()->getViewMatrix();
	osg::Matrix _projectionMatrix = m_pOsgViewer->getCamera()->getProjectionMatrix();
	osg::Viewport *_viewport = m_pOsgViewer->getCamera()->getViewport();

	osg::Matrix matrix( _viewMatrix * _projectionMatrix);

	if (_viewport != NULL)
		matrix.postMult(_viewport->computeWindowMatrix());

	ConvertMatrix4(&matrix, &mat);
}

void vtScene::DoUpdate()
{
	UpdateBegin();
	UpdateEngines();
	UpdateWindow(GetWindow(0));

	// Some engines need to run after the cull-draw phase
	PostDrawEngines();
}

void vtScene::SetRoot(vtGroup *pRoot)
{
	// Remove previous root, add this one
	m_StateRoot->removeChildren(0, m_StateRoot->getNumChildren());
	if (pRoot)
		m_StateRoot->addChild(pRoot);

#if OLD_OSG_SHADOWS
	// Clear out any shadow stuff
	m_pStructureShadowsOSG = NULL;
#endif

	// Remember it
	m_pRoot = pRoot;
}

bool vtScene::IsStereo() const
{
	const osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
	return displaySettings->getStereo();
}

void vtScene::SetStereoSeparation(float fSep)
{
	osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
	displaySettings->setEyeSeparation(fSep);
}

float vtScene::GetStereoSeparation() const
{
	const osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
	return displaySettings->getEyeSeparation();
}

void vtScene::SetStereoFusionDistance(float fDist)
{
	m_pOsgViewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, fDist);
}

float vtScene::GetStereoFusionDistance()
{
	return m_pOsgViewer->getFusionDistanceValue();
}

/**
 * Convert window coordinates (in pixels) to a ray from the camera
 * in world coordinates.  Pixel coordinates are measured from the
 * top left corner of the window: X right, Y down.
 */
bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir, vtWindow *pWindow)
{
	if (!pWindow)
		pWindow = GetWindow(0);

	osg::Vec3 near_point, far_point, diff;

	// There used to be a handy function for this
	osg::Matrix MVPW;
	osg::Matrix InverseMVPW;
	osg::Camera *pCamera = m_pOsgViewer->getCamera();
	MVPW = pCamera->getViewMatrix() * pCamera->getProjectionMatrix();
	if (pCamera->getViewport())
		MVPW.postMult(pCamera->getViewport()->computeWindowMatrix());
	InverseMVPW.invert(MVPW);

	IPoint2 winsize = pWindow->GetSize();

	near_point = osg::Vec3d(win.x, winsize.y - 1 - win.y, 0.0) * InverseMVPW;
	far_point = osg::Vec3d(win.x, winsize.y - 1 - win.y, 1.0) * InverseMVPW;

	diff = far_point - near_point;
	diff.normalize();

	s2v(near_point, pos);
	s2v(diff, dir);

	return true;
}

/**
 * Convert a point in world coordinates to a window pixel coordinate, measured
 * from the top left corner of the window: X right, Y down.
 */
void vtScene::WorldToScreen(const FPoint3 &point, IPoint2 &result)
{
	osg::Vec3 object;
	v2s(point, object);
	osg::Vec3 window;

	osg::Matrix MVPW;
	osg::Camera *pCamera = m_pOsgViewer->getCamera();
	MVPW = pCamera->getViewMatrix() * pCamera->getProjectionMatrix();
	if (pCamera->getViewport())
		MVPW.postMult(pCamera->getViewport()->computeWindowMatrix());

	window = object * MVPW;

	result.x = (int) window.x();
	result.y = (int) window.y();
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
	// Get the view frustum clipping planes directly from OSG.
	// We can't get the planes from the state, because the state
	//  includes the funny modelview matrix used to scale the
	//  heightfield.  We must get it from the 'scene' instead.

	const osg::Matrixd &_projection = m_pOsgViewer->getCamera()->getProjectionMatrix();
	const osg::Matrixd &_modelView = m_pOsgViewer->getCamera()->getViewMatrix();

	osg::Polytope tope;
	tope.setToUnitFrustum();
	tope.transformProvidingInverse((_modelView)*(_projection));

	const osg::Polytope::PlaneList &planes = tope.getPlaneList();

	int i = 0;
	for (osg::Polytope::PlaneList::const_iterator itr=planes.begin();
		itr!=planes.end(); ++itr)
	{
		// make a copy of the clipping plane
		osg::Plane plane = *itr;

		// extract the OSG plane to our own structure
		osg::Vec4 pvec = plane.asVec4();
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
	osg::PolygonMode *npm = new osg::PolygonMode;
	if (m_bWireframe)
		npm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	else
		npm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
	GetRootState()->setAttributeAndModes(npm, osg::StateAttribute::ON);
}

bool vtScene::GetGlobalWireframe()
{
	return m_bWireframe;
}

void vtScene::SetWindowSize(int w, int h, vtWindow *pWindow)
{
	if (!m_bInitialized) return;
	if (m_pHUD)
		m_pHUD->SetWindowSize(w, h);

	if (!pWindow)
		pWindow = GetWindow(0);
	pWindow->SetSize(w, h);

	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (unsigned int i = 0; i < list.GetSize(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnWindowSize(w, h);
	}

	osgViewer::GraphicsWindow* pGW = (osgViewer::GraphicsWindow*)GetGraphicsContext();
	if ((NULL != pGW) && pGW->valid())
	{
		pGW->getEventQueue()->windowResize(0, 0, w, h);
		pGW->resized(0, 0, w, h);
	}
}

bool vtScene::GetWindowSizeFromOSG()
{
	bool bNeedSize = true;

	// Get initial size from OSG?
	osg::Camera *cam = m_pOsgViewer->getCamera();
	osg::Viewport *vport = cam->getViewport();
	if (vport)
	{
		vtGetScene()->SetWindowSize(vport->width(), vport->height());
		bNeedSize = false;
	}

	if (bNeedSize)
	{
		// or another way
		osgViewer::ViewerBase::Contexts contexts;
		m_pOsgViewer->getContexts(contexts);
		if (contexts.size() > 0)
		{
			const osg::GraphicsContext::Traits *traits = contexts[0]->getTraits();
			vtGetScene()->SetWindowSize(traits->width, traits->height);
			bNeedSize = false;
		}
	}

	//// or another way
	//osgViewer::ViewerBase::Windows windows;
	//m_pOsgViewer->getWindows(windows);

	//// or another
	//osgViewer::ViewerBase::Views views;
	//m_pOsgViewer->getViews(views);

	return !bNeedSize;
}

////////////////////////////////////////
// Shadow methods

#if OLD_OSG_SHADOWS
void vtScene::ShadowVisibleNode(osg::Node *node, bool bVis)
{
	if (m_pStructureShadowsOSG.valid())
		if (bVis)
			m_pStructureShadowsOSG->ExcludeFromShadower(node, false);
		else
			m_pStructureShadowsOSG->ExcludeFromShadower(node, true);
}

bool vtScene::IsShadowVisibleNode(osg::Node *node)
{
	if (m_pStructureShadowsOSG.valid())
		return (m_pStructureShadowsOSG->IsExcludedFromShadower(node) == false);
	return false;
}

void vtScene::SetShadowedNode(vtTransform *pLight, osg::Node *pShadowerNode,
							  osg::Node *pShadowed, int iRez, float fDarkness,
							  int iTextureUnit, const FSphere &ShadowSphere)
{
	m_pStructureShadowsOSG = new CStructureShadowsOSG;
	m_pStructureShadowsOSG->Initialise(m_pOsgSceneView.get(), pShadowerNode,
		pShadowed, iRez, fDarkness, iTextureUnit, ShadowSphere);
	m_pStructureShadowsOSG->SetSunDirection(v2s(-pLight->GetDirection()));
	m_pStructureShadowsOSG->ComputeShadows();
}

void vtScene::UnsetShadowedNode(vtTransform *pTransform)
{
	m_pStructureShadowsOSG = NULL;
}

void vtScene::UpdateShadowLightDirection(vtTransform *pLight)
{
	if (m_pStructureShadowsOSG.valid())
		m_pStructureShadowsOSG->SetSunDirection(v2s(-pLight->GetDirection()));
}

void vtScene::SetShadowDarkness(float fDarkness)
{
	if (m_pStructureShadowsOSG.valid())
		m_pStructureShadowsOSG->SetShadowDarkness(fDarkness);
}

void vtScene::SetShadowSphere(const FSphere &ShadowSphere, bool bForceRedraw)
{
	if (m_pStructureShadowsOSG.valid())
		m_pStructureShadowsOSG->SetShadowSphere(ShadowSphere, bForceRedraw);
}

void vtScene::ComputeShadows()
{
	if (m_pStructureShadowsOSG.valid())
		m_pStructureShadowsOSG->ComputeShadows();
}
#endif // OLD_OSG_SHADOWS

void vtScene::SetGraphicsContext(osg::GraphicsContext* pGraphicsContext)
{
	m_pGraphicsContext = pGraphicsContext;
	m_pOsgViewer->getCamera()->setGraphicsContext(pGraphicsContext);
}

osg::GraphicsContext* vtScene::GetGraphicsContext()
{
	return m_pGraphicsContext.get();
}

////////////////////////////////////////

// Helper fn for dumping an OSG scenegraph
void printnode(osg::Node *node, int tab)
{
	for (int i = 0; i < tab*2; i++) {
	   osg::notify(osg::WARN) << " ";
	}
	osg::notify(osg::WARN) << node->className() << " - " << node->getName() << " @ " << node << std::endl;
	osg::Group *group = node->asGroup();
	if (group) {
		for (unsigned int i = 0; i < group->getNumChildren(); i++) {
			printnode(group->getChild(i), tab+1);
		}
	}
}

//////////////////////////////////////////////
// Window

vtWindow::vtWindow()
{
	m_BgColor.Set(0.2f, 0.2f, 0.4f);
	m_Size.Set(0, 0);
}

void vtWindow::SetSize(int w, int h)
{
    VTLOG("vtWindow::SetSize(%d, %d)\n", w, h);
	m_Size.Set(w,h);
}

