//
// A "scene" currently encapsulates:
//
// 1. a scene graph
// 2. a set of engines
// 3. a window
// 4. a current camera
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_SCENEH
#define VTOSG_SCENEH

#include "../core/FrameTimer.h"

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

	void Init();
	void DoUpdate();
	float GetFrameRate();
	void DrawFrameRateChart();

	// OSG-specific implementation
	void AddMovLight(vtMovLight *pTrans);
	osgUtil::SceneView	*m_pOsgSceneView;

protected:
	osg::Group			*m_pOsgSceneRoot;
	Array<vtMovLight*>	m_Lights;

	FrameTimer m_FrameTimer;

	bool	m_bInitialized;
	bool	m_bWireframe;
};

// global
vtScene *vtGetScene();
float vtGetTime();

#endif
