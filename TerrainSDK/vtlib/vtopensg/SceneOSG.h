//
// SceneOSG.h
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_SCENEH
	#define VTOSG_SCENEH

// A little helper to simplify scene management and interaction
	#include <OpenSG/OSGSimpleSceneManager.h>

	#include <time.h>

	#include <OpenSG/OSGNode.h>
	#include <OpenSG/OSGGroup.h>
	#include <OpenSG/OSGRefPtr.h>
	#include <OpenSG/OSGSolidBackground.h>
	#include <OpenSG/OSGShadowMapViewport.h>


//EXCEPT class CreateProjectedShadowTextureCullCallback;

/** \addtogroup sg */
/*@{*/

/**
 * A Scene is the all-encompassing container for all 3D objects
 * that are to be managed and drawn by the scene graph / graphics
 * pipeline functionality of vtlib.
 *
 * A Scene currently encapsulates:
	- A scene graph
	- A set of engines (vtEngine)
	- A window
	- A current camera (vtCamera)
 */
class vtScene : public vtSceneBase {
public:
	vtScene();
	~vtScene();

	/// Set the root node, the top node of the scene graph.
	void SetRoot(vtGroup *pRoot);

	/// Set global wireframe, which will force all objects to be drawn wireframe.
	void SetGlobalWireframe(bool bWire);
	/// Get the global wireframe state.
	bool GetGlobalWireframe();

	/// Call this method once before calling any other vtlib methods.
	bool Init(bool bStereo = false, int iStereoMode = 0);

	/// Call this method after all other vtlib methods, to free memory.
	void Shutdown();

	void TimerRunning(bool bRun);
	void UpdateBegin();
	void UpdateEngines();
	void UpdateWindow(vtWindow *window);

	// backward compatibility
	void DoUpdate();

	/// Return the instantaneous framerate in frames per seconds estimated.
	float GetFrameRate()
	{
		return 1.f /( ( (float)(_frameTick - _lastFrameTick))/CLOCKS_PER_SEC );
	}

	void DrawFrameRateChart();

	float GetTime()
	{
		//EXCEPT return _timer.delta_s(_initialTick,_frameTick);
		return( (float)(_frameTick - _initialTick))/CLOCKS_PER_SEC;
	}
	float GetFrameTime()
	{
		//EXCEPT return _timer.delta_s(_lastFrameTick,_frameTick);
		//std::cout << "frametime " <<_lastframeTick - _initialTick;
		return( (float)(_frameTick - _lastFrameTick))/CLOCKS_PER_SEC;  
	}

	// View methods
	bool CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir, vtWindow *pWindow = NULL);
	void WorldToScreen(const FPoint3 &point, IPoint2 &result);
	FPlane *GetCullPlanes() { return m_cullPlanes;}

	// Experimental:
	// Object-terrain shadow casting, only for OSG
	void SetShadowedNode(vtTransform *pLight, vtNode *pShadowNode, vtTransform *pTransform, int iRez) {};
	void UnsetShadowedNode(vtTransform *pTransform) {};
	void UpdateShadowLightDirection(vtTransform *pLight) {};
	void SetShadowDarkness(float fDarkness) {};
	//osg::ref_ptr<CreateProjectedShadowTextureCullCallback> m_pShadowVisitor;
	void ShadowVisibleNode(vtNode *node, bool bVis){};


	void SetHUD(vtHUD *hud) { m_pHUD = hud;};
	vtHUD *GetHUD() { return m_pHUD;};
	void SetWindowSize(int w, int h, vtWindow *pWindow = NULL);

	// For backward compatibility
	void SetBgColor(const RGBf &color) {
		if( GetWindow(0) )
			GetWindow(0)->SetBgColor(color);
	};


	bool IsStereo() const;
	void SetStereoSeparation(float fSep);
	float GetStereoSeparation() const; 
	void ComputeViewMatrix(FMatrix4 &mat);

	// OpenSG access
	osg::SimpleSceneManager *GetSceneView() { return m_pOsgSceneView;}
	void SetSceneView(osg::SimpleSceneManager *mgr) { 
		m_pOsgSceneView = mgr;
	}

protected:
	// OpenSG-specific implementation
	osg::SimpleSceneManager *m_pOsgSceneView;
	osg::WindowPtr m_pOsgWindow;


	// for culling
	void CalcCullPlanes();
	FPlane      m_cullPlanes[6];

	//{
	//osg::GroupPtr m_pOsgSceneRootCore;
	osg::RefPtr<osg::NodePtr> m_pOsgSceneRoot;
	//preliminary, until opensg gets real shadow as discussed (soon).
	osg::RefPtr<osg::ShadowMapViewportPtr> m_pShadowMapViewport;
	//osg::NodePtr m_pOsgSceneRoot;
	//}
	osg::NodePtr m_pCameraTransformNode;

	clock_t _timer;
	clock_t _initialTick;
	clock_t _lastFrameTick;
	clock_t _lastRunningTick;
	clock_t _frameTick;
	double m_fAccumulatedFrameTime, m_fLastFrameTime;

	bool    m_bWinInfo;
	bool    m_bInitialized;
	bool    m_bWireframe;
	vtHUD   *m_pHUD;

	osg::SolidBackgroundPtr m_Background;
};

// global
vtScene *vtGetScene();
float vtGetTime();
float vtGetFrameTime();
int vtGetMaxTextureSize();

/*@}*/	// Group sg

#endif	// VTOSG_SCENEH

