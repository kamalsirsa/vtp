//
// ScenePSM.h
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPSM_SCENEH
#define VTPSM_SCENEH

#include "../core/FrameTimer.h"

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
	void		SetBgColor(RGBf color)	{ if (m_pPSS) m_pPSS->SetBackColor(PSCol4(color.r, color.g, color.b)); }
	void		SetAmbient(RGBf color)	{ if (m_pPSS) m_pPSS->SetAmbient(PSCol4(color.r, color.g, color.b)); }
	void		SetRoot(vtGroup *pRoot)	{ if (m_pPSS) m_pPSS->SetDynamic(pRoot); }
	vtGroup*	GetRoot()				{ if (m_pPSS) return (vtGroup*) m_pPSS->GetDynamic(); else return NULL; }
	vtCamera*	GetCamera()				{ return m_pCamera; }
	float		GetFrameRate()			{ return 1.0f / (float) frameSeconds(); }
	void		SetCamera(vtCamera* c);
	void		SetGlobalWireframe(bool bWire);
	bool		GetGlobalWireframe();
	void		SetWindowSize(int w, int h);
	IPoint2		GetWindowSize();
	bool		Init();
	void		SetFog(bool bOn, float start = 0, float end = 10000, const RGBf &color = s_white, int iType = GL_LINEAR);
	bool		CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir);

	void		DoUpdate();
	void		AllocInternalScene();

	static RGBf s_white;

	PSScene	*m_pPSS;

#if WIN32
	bool vtScene::HasWinInfo();
	void vtScene::SetWinInfo(void *handle, void *context);
#endif

protected:
	// time from the current frame update and the previous one in seconds.
	double frameSeconds();
};

// global
vtScene *vtGetScene();

extern float vtGetTime();
#endif
