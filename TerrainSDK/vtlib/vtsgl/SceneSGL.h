//
// A "scene" currently encapsulates:
//
// 1. a scene graph
// 2. a set of engines
// 3. a window
// 4. a current camera
//

#ifndef VTSGL_SCENEH
#define VTSGL_SCENEH

class vtScene : public vtSceneBase
{
public:
	vtScene();

	void SetBgColor(RGBf color);
	void SetAmbient(RGBf color);
	void SetRoot(vtGroup *pRoot);
	bool CameraRay(IPoint2 win, FPoint3 &pos, FPoint3 &dir);
	float GetFrameRate();
	void SetGlobalWireframe(bool bWire);
	bool GetGlobalWireframe();

	bool Init();
	void DoUpdate();

	bool HasWinInfo() { return m_bWinInfo; }
	void SetWinInfo(void *handle, void *context) { m_bWinInfo = true; }

//protected:
	sglView		 m_SglView;
	sglScene	*m_pSglRootNode;
	bool		m_bWinInfo;
	sglTimespec	m_TimeCurrent, m_TimePrevious;
	float		m_fFrameRate;
};

// global
vtScene *vtGetScene();
float vtGetTime();
float vtGetFrameTime();

#endif // VTSGL_SCENEH
