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

#ifndef VTPLIB_SCENEH
#define VTPLIB_SCENEH

class vtScene : public vtSceneBase
{
public:
	vtScene();

	void SetBgColor(RGBf color);
	void SetAmbient(RGBf color);
	void SetRoot(vtGroup *pRoot);
	bool CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir);

	void SetGlobalWireframe(bool bWire);
	bool GetGlobalWireframe();

	void Init();
	void DoUpdate();
	float GetFrameRate();

	// SSG-specific implementation
//protected:
	ssgContext	*m_pContext;
	ssgRoot		*m_pSsgRoot;
	ulClock		m_Clock;		// for framerate
};

// global
vtScene *vtGetScene();
float vtGetTime();

#endif
