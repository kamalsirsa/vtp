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

	void SetBgColor(RGBf color);
	void SetAmbient(RGBf color);
	void SetRoot(vtRoot *pRoot);
	bool CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir);

	void SetGlobalWireframe(bool bWire);
	bool GetGlobalWireframe();

	bool Init();
	void DoUpdate();
	float GetFrameRate();
	void DrawFrameRateChart();

	// OSG-specific implementation
	void AddMovLight(vtMovLight *pTrans);
	osgUtil::SceneView	*m_pOsgSceneView;

#if WIN32
	bool vtScene::HasWinInfo() { return m_bWinInfo; }
	void vtScene::SetWinInfo(void *handle, void *context) { m_bWinInfo = true; }
#endif

protected:
	osg::Group			*m_pOsgSceneRoot;
	Array<vtMovLight*>	m_Lights;

	osg::Timer   _timer;
	osg::Timer_t _initialTick;
	osg::Timer_t _lastFrameTick;
	osg::Timer_t _frameTick;

	// time from the current frame update and the previous one in seconds.
	double frameSeconds() { return _timer.delta_s(_lastFrameTick,_frameTick); }
	double frameRate() { return 1.0/frameSeconds(); }

	bool	m_bWinInfo;
	bool	m_bInitialized;
	bool	m_bWireframe;
};

// global
vtScene *vtGetScene();
float vtGetTime();

#endif
