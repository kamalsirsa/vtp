//
// SceneOSG.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_SCENEH
#define VTOSG_SCENEH

#include "../core/FrameTimer.h"
#include <osg/Timer>

/*
 * A "scene" currently encapsulates:
 *
 * 1. a scene graph
 * 2. a set of engines
 * 3. a window
 * 4. a current camera
 */
class vtScene : public vtSceneBase
{
public:
	vtScene();
	~vtScene();

	void SetBgColor(RGBf color);
	void SetAmbient(RGBf color);
	void SetRoot(vtRoot *pRoot);
	bool CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir);

	void SetGlobalWireframe(bool bWire);
	bool GetGlobalWireframe();

	bool Init();
	void Shutdown();

	void DoUpdate();
	float GetFrameRate()
	{
		return 1.0 / _timer.delta_s(_lastFrameTick,_frameTick);
	}
	void DrawFrameRateChart();

	// OSG-specific implementation
	osg::ref_ptr<osgUtil::SceneView>	m_pOsgSceneView;

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

protected:
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

