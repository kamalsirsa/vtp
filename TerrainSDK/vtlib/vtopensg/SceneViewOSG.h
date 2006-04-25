//
// SceneViewOSG.h
//
// SceneView class for OpenSG.
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef _SCENEVIEWOSG_
#define _SCENEVIEWOSG_

#pragma warning(disable:4251)

//OpenSG includes
#include <OpenSG/OSGPerspectiveCamera.h>
#include <OpenSG/OSGSimpleSceneManager.h>
#include <OpenSG/OSGWindow.h>
#include <OpenSG/OSGViewport.h>
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGSolidBackground.h>

/**
 * a scene viewer wrapper for dealing with vtp.
 */
class SceneViewOSG {
public:
	//takes mono components for usage within VTP
	SceneViewOSG( OSG::WindowPtr window, 
			   OSG::ViewportPtr leftViewport,
			   OSG::RenderAction renderAction );

	//from vtScene::Init(bStereo, iStereoMode)
	SceneViewOSG( bool bStereo, int iStereoMode );

	void SetRoot( OSG::NodePtr root );
	OSG::Line CalcViewRay(OSG::UInt32 x, OSG::UInt32 y);
	void UpdateCamera( float aspect, float fov_y, float hither, float yon, OSG::NodePtr node);
				
	OSG::DrawActionBase *GetAction() const;
	OSG::ViewportPtr GetLeftViewport() const {return m_LeftViewportPtr; };
	OSG::ViewportPtr GetRightViewport() const {return m_RightViewportPtr;};
	OSG::ViewportPtr GetViewport() const;
	OSG::PerspectiveCameraPtr GetCamera() const;
	OSG::PerspectiveCameraPtr GetLeftCamera() const {return m_LeftCameraPtr;}; 
    OSG::PerspectiveCameraPtr GetRightCamera() const {return m_RightCameraPtr;}; 
	OSG::WindowPtr GetWindow() const; 
	void SetBackgroundColor ( osg::Color3f backColor );
	OSG::SimpleSceneManager *GetSSM() const;
	bool UsesSSM() const {return m_bUsesSSM;};
	bool IsStereo() const {return m_bStereo;};
	int  GetStereoMode() const {return m_iStereoMode;};
	void Redraw();
	

protected:
	SceneViewOSG(){};

private:
	bool m_bStereo;
	bool m_bUsesSSM;
	int m_iStereoMode;
	OSG::PerspectiveCameraPtr m_LeftCameraPtr, m_RightCameraPtr;
	OSG::ViewportPtr m_LeftViewportPtr,  m_RightViewportPtr;
	OSG::WindowPtr m_WindowPtr;
    OSG::RenderAction *m_pRenderAction;
	//wrap also SSM
	OSG::SimpleSceneManager *m_pSimpleSceneManager;
	OSG::SolidBackgroundPtr m_SolidBackgroundPtr;
};

#endif
