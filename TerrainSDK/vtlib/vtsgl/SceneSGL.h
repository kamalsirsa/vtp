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
	void SetRoot(vtRoot *pRoot);
	bool CameraRay(IPoint2 win, FPoint3 &pos, FPoint3 &dir);
	float GetFrameRate();
	void SetGlobalWireframe(bool bWire);
	bool GetGlobalWireframe();

	void Init();
	void DoUpdate();

//protected:
	sglView		 m_SglView;
	sglScene	*m_pSglRootNode;
};

// global
vtScene *vtGetScene();
float vtGetTime();

#endif // VTSGL_SCENEH
