//
// SceneOSG.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_SCENEH
#define VTOSG_SCENEH

#include <osg/Timer>

class CreateProjectedShadowTextureCullCallback;

/**
 * A Scene is the all-encompassing container for all 3D objects
 * that are to be managed and drawn by the scene graph / graphics
 * pipeline functionality of vtlib.
 *
 * A Scene currently encapsulates:
 *	- A scene graph
 *	- A set of engines (vtEngine)
 *  - A window
 *  - A current camera (vtCamera)
 */
class vtScene : public vtSceneBase
{
public:
	vtScene();
	~vtScene();

	/// Set the background color for the scene (the rendering window).
	void SetBgColor(RGBf color);

	/// Set the root node, the top node of the scene graph.
	void SetRoot(vtGroup *pRoot);

	/// Set global wireframe, which will force all objects to be drawn wireframe.
	void SetGlobalWireframe(bool bWire);
	/// Get the global wireframe state.
	bool GetGlobalWireframe();

	/// Call this method once before calling any other vtlib methods.
	bool Init();

	/// Call this method after all other vtlib methods, to free memory.
	void Shutdown();

	void DoUpdate();
	/// Return the instantaneous framerate in frames per seconds estimated.
	float GetFrameRate()
	{
		return 1.0 / _timer.delta_s(_lastFrameTick,_frameTick);
	}
	void DrawFrameRateChart();

	bool HasWinInfo() { return m_bWinInfo; }
	void SetWinInfo(void *handle, void *context) { m_bWinInfo = true; }

	float GetTime()
	{
		return _timer.delta_s(_initialTick,_frameTick);
	}
	float GetFrameTime()
	{
		return _timer.delta_s(_lastFrameTick,_frameTick);
	}

	// View methods
	bool CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir);
	FPlane *GetCullPlanes() { return m_cullPlanes; }

	// Experimental:
	// Object-terrain shadow casting, only for OSG
	void SetShadowedNode(vtTransform *pLight, vtNode *pShadowNode, vtTransform *pTransform, int iRez);
	void UnsetShadowedNode(vtTransform *pTransform);
	void UpdateShadowLightDirection(vtTransform *pLight);
	osg::ref_ptr<CreateProjectedShadowTextureCullCallback> m_pShadowVisitor;

protected:
	// OSG-specific implementation
	osg::ref_ptr<osgUtil::SceneView>	m_pOsgSceneView;

	// for culling
	void CalcCullPlanes();
	FPlane		m_cullPlanes[6];

	osg::ref_ptr<osg::Group>	m_pOsgSceneRoot;

	osg::Timer   _timer;
	osg::Timer_t _initialTick;
	osg::Timer_t _lastFrameTick;
	osg::Timer_t _frameTick;

	bool	m_bWinInfo;
	bool	m_bInitialized;
	bool	m_bWireframe;
};

// global
vtScene *vtGetScene();
float vtGetTime();
float vtGetFrameTime();

#endif	// VTOSG_SCENEH

