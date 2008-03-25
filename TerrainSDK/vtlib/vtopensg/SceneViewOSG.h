//
// SceneViewOSG.h
//
// SceneView class for OpenSG.
//
// Copyright (c) 2006-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef _SCENEVIEWOSG_
#define _SCENEVIEWOSG_

//OpenSG includes
#include <OpenSG/OSGPerspectiveCamera.h>
#include <OpenSG/OSGWindow.h>
#include <OpenSG/OSGViewport.h>
#include <OpenSG/OSGShadowMapViewport.h>
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGSolidBackground.h>
#include <OpenSG/OSGShearedStereoCameraDecorator.h>

/**
 * a scene viewer wrapper for dealing with vtp.
 */
class SceneViewOSG {
public:
	SceneViewOSG( OSG::WindowPtr window,
			   OSG::ViewportPtr viewport,
			   OSG::PerspectiveCameraPtr camera,
			   OSG::RenderAction *renderAction,
			   int stereoMode=0,
			   OSG::SolidBackgroundPtr background=OSG::NullFC
			   );
	~SceneViewOSG();

	//from vtScene::Init(bStereo, iStereoMode)
	SceneViewOSG( bool bStereo, int iStereoMode );

	void SetRoot( OSG::NodePtr root );
	OSG::Line CalcViewRay(OSG::UInt32 x, OSG::UInt32 y);
	void UpdateCamera( float aspect, float fov_y, float hither, float yon);

	OSG::DrawActionBase *GetAction() const;
	OSG::ViewportPtr/*OSG::ShadowMapViewportPtr*/ GetLeftViewport() const {return m_LeftViewportPtr; };
	OSG::ViewportPtr/*OSG::ShadowMapViewportPtr*/ GetRightViewport() const {return m_RightViewportPtr;};
	OSG::ViewportPtr/*OSG::ShadowMapViewportPtr*/ GetViewport() const;
	OSG::PerspectiveCameraPtr GetCamera() const;
	OSG::PerspectiveCameraPtr GetLeftCamera() const {return m_LeftCameraPtr;};
	OSG::PerspectiveCameraPtr GetRightCamera() const {return m_RightCameraPtr;};
	OSG::WindowPtr GetWindow() const;
	void SetBackgroundColor ( osg::Color3f backColor );
	bool IsCustomSceneView() const { return m_bUsesCustomSceneView;};
	bool IsStereo() const {return m_bStereo;};

	//shadows
	void SetShadowOn( bool bOn );
	bool UsesShadow () const {return m_bUsesShadow;};

	int  GetStereoMode() const {return m_iStereoMode;};
	void Redraw();


protected:
	SceneViewOSG(){};

private:
	bool m_bStereo;
	bool m_bUsesShadow;
	bool m_bUsesCustomSceneView;
	int m_iStereoMode;
	OSG::RefPtr<OSG::PerspectiveCameraPtr> m_LeftCameraPtr, m_RightCameraPtr;
	OSG::ViewportPtr/*OSG::ShadowMapViewportPtr*/ m_LeftViewportPtr,  m_RightViewportPtr;
	OSG::RefPtr<OSG::WindowPtr> m_WindowPtr;
	OSG::RenderAction *m_pRenderAction;
	//wrap also SSM
	OSG::RefPtr<OSG::SolidBackgroundPtr> m_SolidBackgroundPtr;
	OSG::RefPtr<OSG::ShearedStereoCameraDecoratorPtr> m_SSCD;
};

#endif
