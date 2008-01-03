//
// SceneOSG.cpp
//
// Implementation of vtScene for the OpenSG library
//
// Copyright (c) 2006-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include <time.h>		// clock() & CLOCKS_PER_SEC

#ifdef __FreeBSD__
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/resource.h>
#endif

#include <iostream>
#include "vtdata/vtLog.h"

#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGIntersectAction.h>	// the ray intersect traversal


#include <math.h>

//passive windows do not handle OpenGL contexts and are designed to fit easily into existing applications.

#include <OpenSG/OSGPassiveWindow.h>
///////////////////////////////////////////////////////////////

// There is always and only one global vtScene object
vtScene g_Scene;

vtScene::vtScene() : vtSceneBase()
{
	m_bInitialized = false;
	m_bWireframe = false;
	m_bWinInfo = false;
	m_pHUD = NULL;
}

vtScene::~vtScene()
{
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
 * \param bStereo True for a stereo display output.
 * \param iStereoMode Currently for vtosg, supported values are 0 for
 *		Anaglyphic (red-blue) and 1 for Quad-buffer (shutter glasses).
 */
bool vtScene::Init(bool bStereo, int iStereoMode)
{
	osg::osgInit(0,NULL);

	m_pSceneViewOSG = new SceneViewOSG( bStereo, iStereoMode );

	m_pDefaultCamera = new vtCamera;
	m_pDefaultWindow = new vtWindow;
	SetCamera(m_pDefaultCamera);
	AddWindow(m_pDefaultWindow);

	//not in sceneviewosg... crashes ?
	//no stereo or passive stereo  have only ONE camera
	if (!bStereo || GetSceneView()->GetStereoMode() == 2) 
		GetSceneView()->GetCamera()->setBeacon( m_pCamera->GetOsgNode() );
	else {
		GetSceneView()->GetLeftCamera()->setBeacon( m_pCamera->GetOsgNode() );
		GetSceneView()->GetRightCamera()->setBeacon( m_pCamera->GetOsgNode() );
	}

	_initialTick = clock();
	_frameTick = _initialTick;

	m_bInitialized = true;

	return true;
}

/**
 * Initialize the vtlib library, including the display and scene graph.
 * You should call this function only once, before any other vtlib calls.
 *
 * \param SceneViewOSG the wrapper for the rendering components.
 * \return true, if succeeded.
 */
bool vtScene::CustomInit( SceneViewOSG * sceneview )
{
	assert( sceneview != NULL );

	m_pSceneViewOSG = sceneview;

	m_pDefaultCamera = new vtCamera;
	m_pDefaultWindow = new vtWindow;
	SetCamera(m_pDefaultCamera);
	AddWindow(m_pDefaultWindow);

	//the external camera controls now the vtp cam
	//dont set the beacon...
	if (!GetSceneView()->IsCustomSceneView() ) {
		if (!GetSceneView()->IsStereo() || GetSceneView()->GetStereoMode() == 2) 
			GetSceneView()->GetCamera()->setBeacon( m_pCamera->GetOsgNode() );
		else {
			GetSceneView()->GetLeftCamera()->setBeacon( m_pCamera->GetOsgNode() );
			GetSceneView()->GetRightCamera()->setBeacon( m_pCamera->GetOsgNode() );
		}
	}

	_initialTick = clock();
	_frameTick = _initialTick;

	m_bInitialized = true;

	return true;
}

void vtScene::Shutdown()
{
	VTLOG("vtScene::Shutdown\n");
	m_pDefaultCamera->Release();
	delete m_pDefaultWindow;
	vtNode::ClearOsgModelCache();
	vtImageCacheClear();
	delete m_pSceneViewOSG;
}

void vtScene::TimerRunning(bool bRun)
{
	if ( !bRun ) {
		// stop timer, count how much running time has already elapsed
		m_fAccumulatedFrameTime = (float)(_lastRunningTick - clock() ) / CLOCKS_PER_SEC;
		//VTLOG("partial frame: %lf seconds\n", m_fAccumulatedFrameTime);
	} else
		// start again
		_lastRunningTick =  clock();
}

void vtScene::UpdateBegin()
{
	_lastFrameTick = _frameTick;
	_frameTick = clock();

	// finish counting the split frame's elapsed time
	if ( _lastRunningTick != _lastFrameTick ) {
		m_fAccumulatedFrameTime += (float)(_lastRunningTick - _frameTick) / CLOCKS_PER_SEC;
		//VTLOG("   full frame: %lf seconds\n", m_fAccumulatedFrameTime);
		m_fLastFrameTime = m_fAccumulatedFrameTime;
	} else
		m_fLastFrameTime = (float)(_lastFrameTick - _frameTick) / CLOCKS_PER_SEC;

	_lastRunningTick = _frameTick;

}

void vtScene::UpdateEngines()
{
	if ( !m_bInitialized ) return;
	DoEngines(m_pRootEngine);
}

void vtScene::UpdateWindow(vtWindow *pWindow)
{
	if ( !m_bInitialized )
		return;

	osg::Color3f color3;
	v2s(pWindow->GetBgColor(), color3);
	GetSceneView()->SetBackgroundColor( color3 );   

	if ( m_pCamera->IsOrtho() ) { //no ortho for now
		//we need to have a ortho cam in order to do this in opensg
		// Arguments are left, right, bottom, top, zNear, zFar
		//float w2 = m_pCamera->GetWidth() /2;
		//float h2 = w2 / aspect;
		//m_pOsgSceneView->setProjectionMatrixAsOrtho(-w2, w2, -h2, h2,
		//	m_pCamera->GetHither(), m_pCamera->GetYon());
	} else {
		// window size
		IPoint2 winsize = pWindow->GetSize();
		if ( winsize.x == 0 || winsize.y == 0 )
			VTLOG("Warning: winsize %d %d\n", winsize.x, winsize.y);
		float winaspect;
		if ( winsize.x == 0 || winsize.y == 0 )
			winaspect = 1.0f;
		else
			winaspect = (float) winsize.y / winsize.x;

		// vtlib has horizontal FOV, but OpenSG wants vertical FOV
		float fov_x = m_pCamera->GetFOV();
		float a = tan (fov_x/2);
		float b = a * winaspect;
		float fov_y_div2 = atan(b);
		float fov_y = fov_y_div2 * 2;

		// 'Aspect ratio' in OpenSG is a real pixel aspect ratio (1.0 is normal),
		//   not a compensation factor for window size as with the other OSG.
		float aspect = 1.0f;
		// Not: aspect = (float) winsize.y / winsize.x;

		GetSceneView()->UpdateCamera(
									aspect, 
									fov_y, 
									m_pCamera->GetHither(), 
									m_pCamera->GetYon()
									);
	}

	CalcCullPlanes();

	GetSceneView()->Redraw();
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
	osg::Matrix result;
	GetSceneView()->GetCamera()->getViewing( result, 
											 GetSceneView()->GetWindow()->getWidth(), 
											 GetSceneView()->GetWindow()->getHeight() 
										   );

	ConvertMatrix4( &result, &mat );
}

void vtScene::DoUpdate()
{
	UpdateBegin();
	UpdateEngines();
	UpdateWindow(GetWindow(0));
}

//sets the GLOBAL scene root for SimpleSceneManager
void vtScene::SetRoot(vtGroup *pRoot)
{
	if ( pRoot ) {
		beginEditCP(m_pOsgSceneRoot);
		m_pOsgSceneRoot = pRoot->GetOsgNode();      
		endEditCP(m_pOsgSceneRoot); 
	} else
		m_pOsgSceneRoot	= osg::NullFC;

	if ( GetSceneView() != NULL ) {
		GetSceneView()->SetRoot(m_pOsgSceneRoot);
	}

	m_pRoot = pRoot;
}


bool vtScene::IsStereo() const
{
	return GetSceneView()->IsStereo();
}

void vtScene::SetStereoSeparation(float fSep)
{
	/*osg::DisplaySettings* displaySettings = m_pOsgSceneView->getDisplaySettings();
	displaySettings->setEyeSeparation(fSep);*/
}

float vtScene::GetStereoSeparation() const
{
	/*const osg::DisplaySettings* displaySettings = m_pOsgSceneView->getDisplaySettings();
	return displaySettings->getEyeSeparation();*/
	return 0.f;
}



/**
 * Convert window coordinates (in pixels) to a ray from the camera
 * in world coordinates.  Pixel coordinates are measured from the
 * top left corner of the window: X right, Y down.
 */
bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir, vtWindow *pWindow)
{
	//we don't actually need the window since it is managed by the SimpleSceneManager
	osg::Line  l = GetSceneView()->CalcViewRay(win.x, win.y);

	s2v(l.getPosition(), pos);
	s2v(l.getDirection(), dir);

	//do we really need to norm. ?
	dir.Normalize();

	return true;
}

void vtScene::WorldToScreen(const FPoint3 &point, IPoint2 &result)
{
	osg::Matrix to_screen;
	//TODO don't use index but cast to correct type of viewport, e.g. no shadowmapviewport...
	GetSceneView()->GetCamera()->getWorldToScreen(to_screen, 
												  *GetSceneView()->GetViewport());
	osg::Pnt3f p; 
	v2s(point, p);
	to_screen.multMatrixPnt( p );
	result.x = (int) p.x();
	result.y = (int) p.y();
}


// Debugging helper
void LogCullPlanes(FPlane *planes)
{
	/*for (int i = 0; i < 4; i++)
		VTLOG(" plane %d: %.3f %.3f %.3f %.3f\n", i, planes[i].x, planes[i].y, planes[i].z, planes[i].w);
	VTLOG("\n");*/
}

void vtScene::CalcCullPlanes()
{
#if 1 
	// Non-API-Specific code - will work correctly as long as the Camera
	// methods are fully functional.
	FMatrix4 mat;
	m_pCamera->GetTransform1(mat);

	IPoint2 m_WindowSize = GetWindow(0)->GetSize();
	assert(( m_WindowSize.x > 0 ) && ( m_WindowSize.y > 0 ));

	double fov = m_pCamera->GetFOV();

	double aspect = (float)m_WindowSize.y / m_WindowSize.x;
	float hither = m_pCamera->GetHither();

	float a = (float) (hither * tan( fov / 2 ));
	float b = (float) (a * aspect);

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
	FrustrumVolume f;
	GetSceneView()->getCamera()->getFrustrum( f, GetSceneView()->getWindow()->getPort(0) );

	// Get the view frustum clipping planes directly from OSG.
	// We can't get the planes from the state, because the state
	//  includes the funny modelview matrix used to scale the
	//  heightfield.  We must get it from the 'scene' instead.

	const osg::Matrixd &_projection = m_pOsgSceneView->getProjectionMatrix();
	const osg::Matrixd &_modelView = m_pOsgSceneView->getViewMatrix();

	Polytope tope;
	tope.setToUnitFrustum();
	tope.transformProvidingInverse((_modelView)*(_projection));

	const Polytope::PlaneList &planes = tope.getPlaneList();

	int i = 0;
	for ( Polytope::PlaneList::const_iterator itr=planes.begin();
		itr!=planes.end(); ++itr ) {
		// make a copy of the clipping plane
		Plane plane = *itr;

		// extract the OSG plane to our own structure
		Vec4 pvec = plane.asVec4();
		m_cullPlanes[i++].Set(-pvec.x(), -pvec.y(), -pvec.z(), -pvec.w());
	}
#endif //1 
	// For debugging
	LogCullPlanes(m_cullPlanes);
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
	for ( int i = 1; i <= 100; i++ ) {
		glVertex3f(-1.0f + i/200.0f, -1.0f + fps[(s+i)%100]/200.0f, 0.0f);
	}
	glEnd();

	s++;
	if ( s == 100 )	s = 0;
}

void vtScene::SetGlobalWireframe(bool bWire)
{
#if EXCEPT
	m_bWireframe = bWire;

	// Set the scene's global PolygonMode attribute, which will affect all
	// other materials in the scene, except those which explicitly override
	// the attribute themselves.
	StateSet *global_state = m_pOsgSceneView->getGlobalStateSet();
	PolygonMode *npm = new PolygonMode();
	if ( m_bWireframe )
		npm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
	else
		npm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::FILL);
	global_state->setAttributeAndModes(npm, StateAttribute::ON);
#endif //EXCEPT
}

void vtScene::SetShadowOn (bool bOn)
{
	GetSceneView()->SetShadowOn(bOn);
}

bool vtScene::GetGlobalWireframe()
{
	return m_bWireframe;
}

void vtScene::SetWindowSize(int w, int h, vtWindow *pWindow)
{
	if ( !m_bInitialized ) return;

	GetSceneView()->GetWindow()->resize(w, h);

	if ( m_pHUD )
		m_pHUD->SetWindowSize(w, h);

	if (!pWindow) pWindow = vtGetScene()->GetWindow(0);
	vtSceneBase::SetWindowSize(w, h, pWindow);
} 


////////////////////////////////////////
// Shadow methods

void vtScene::ComputeShadows()
{
	// TODO
}
