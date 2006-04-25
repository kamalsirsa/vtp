//
// SceneViewOSG.cpp
//
// SceneView class for OpenSG.
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "SceneViewOSG.h"

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#include <OpenSG/OSGPassiveWindow.h>
#include <OpenSG/OSGPassiveViewport.h>
#include <OpenSG/OSGSolidBackground.h>
#include <OpenSG/OSGShearedStereoCameraDecorator.h>

//change this for creating the rendering components manually
#define	   USE_SSM_WHEN_POSSIBLE		1
const bool USE_RENDER_STATISTICS	(true);
const bool USE_OPENSG_LOGO			(true);
const bool USE_HEADLIGHT	   		(true);

#define ANAGLYPHIC 0  //hmm i don't need the enum hack
#define QUADBUFFER 1
#define PASSIVE    2 


//takes mono components for usage within VTP
SceneViewOSG::SceneViewOSG(
						  OSG::WindowPtr window, 
						  OSG::ViewportPtr leftViewport,
						  OSG::RenderAction renderAction ) :  m_bUsesSSM (false), m_bStereo (false)
{

}

/**
 * Initialize the Scene Viewer.
 *
 * \param bStereo True for a stereo display output.
 * \param iStereoMode Currently for vtosg, supported values are 
 * 0 for Anaglyphic (red-blue)
 * 1 for Quad-buffer (shutter glasses)
 * 2 for Passive Stereo (2 projectors).    
 *
 */
SceneViewOSG::SceneViewOSG( bool bStereo, int iStereoMode ) : m_bStereo(bStereo), m_iStereoMode (iStereoMode) 
{
	if ( IsStereo() ) {
		switch ( m_iStereoMode ) {
		case ANAGLYPHIC: break;
		case QUADBUFFER: break;
			/* P A S S I V E  S T E R E O */
		case PASSIVE   : 
#if USE_SSM_WHEN_POSSIBLE
			{
				m_bUsesSSM = true;

				OSG::PassiveWindowPtr gwin = osg::PassiveWindow::create();
				gwin->init();
				m_WindowPtr = gwin;

				m_pSimpleSceneManager = new OSG::SimpleSceneManager;

				GetSSM()->setWindow(gwin);
				GetSSM()->setHeadlight( USE_HEADLIGHT );
				GetSSM()->setStatistics( USE_RENDER_STATISTICS );
				if ( USE_OPENSG_LOGO ) {
					GetSSM()->useOpenSGLogo();
				}

				//get left viewport from default viewport
				m_LeftViewportPtr = GetSSM()->getWindow()->getPort(0);
		
				OSG::PerspectiveCameraPtr cam = GetCamera();
				//left = right camera
				m_LeftCameraPtr = cam;
				m_RightCameraPtr = cam;			

				//decorate the camera
				OSG::ShearedStereoCameraDecoratorPtr rightCameraDecorator = OSG::ShearedStereoCameraDecorator::create();
				beginEditCP(rightCameraDecorator);
				rightCameraDecorator->setLeftEye(false);
				rightCameraDecorator->setEyeSeparation(6);
				rightCameraDecorator->setDecoratee( GetCamera() );
				rightCameraDecorator->setZeroParallaxDistance(200);
				m_RightViewportPtr = OSG::Viewport::create();
				beginEditCP(m_RightViewportPtr);
				m_RightViewportPtr->setCamera    ( rightCameraDecorator );
				m_RightViewportPtr->setSize      ( .5,0,1,1 );
				endEditCP(m_RightViewportPtr);
				endEditCP(rightCameraDecorator);

				beginEditCP(m_WindowPtr);
				m_WindowPtr->addPort ( m_RightViewportPtr );
				endEditCP(m_WindowPtr);

				//decorate the camera
				OSG::ShearedStereoCameraDecoratorPtr leftCameraDecorator  
					 = OSG::ShearedStereoCameraDecorator::create();
				beginEditCP(leftCameraDecorator);
				leftCameraDecorator->setLeftEye(true);
				leftCameraDecorator->setEyeSeparation(6);
				leftCameraDecorator->setDecoratee( GetCamera() );
				leftCameraDecorator->setZeroParallaxDistance(200);
				beginEditCP(m_LeftViewportPtr);
				m_LeftViewportPtr->setCamera    ( leftCameraDecorator );
				m_LeftViewportPtr->setSize      ( 0,0,.5f,1 );
				endEditCP(m_LeftViewportPtr);
				endEditCP(leftCameraDecorator);
			}
#else  //NO SSM

#endif //USE_SSM_WHEN_POSSIBLE

			break;
		default: ;
		}

	} else {
#if USE_SSM_WHEN_POSSIBLE
		m_bUsesSSM = true;

		OSG::PassiveWindowPtr gwin = osg::PassiveWindow::create();
		gwin->init();

		m_pSimpleSceneManager = new OSG::SimpleSceneManager;

		GetSSM()->setWindow(gwin);
		GetSSM()->setHeadlight( USE_HEADLIGHT );
		GetSSM()->setStatistics( USE_RENDER_STATISTICS );
		if ( USE_OPENSG_LOGO ) {
			GetSSM()->useOpenSGLogo();
		}
		OSG::RenderAction *raction = static_cast<OSG::RenderAction*> (GetSSM()->getAction());
		//raction->setZWriteTrans(true);
		raction->setFrustumCulling(true);
		raction->setAutoFrustum(true);

	}
#else
	;
#endif
	}

	void SceneViewOSG::UpdateCamera(
									float aspect, 
									float fov_y,
									float hither, 
									float yon, 
									OSG::NodePtr node )
	{

		if ( !IsStereo() ||  GetStereoMode() == PASSIVE) {
			osg::PerspectiveCameraPtr cam = GetCamera();
			beginEditCP(cam);
			cam->setAspect(aspect);
			cam->setFov(fov_y);
			cam->setNear(hither);
			cam->setFar(yon);
			cam->setBeacon(node);
			endEditCP(cam);
		}
	}

	OSG::SimpleSceneManager *SceneViewOSG::GetSSM() const 
	{ 
		assert (m_pSimpleSceneManager != NULL);
		return m_pSimpleSceneManager; 
	}

	OSG::WindowPtr SceneViewOSG::GetWindow() const
	{
		return UsesSSM() ? m_pSimpleSceneManager->getWindow() : m_WindowPtr;
	}

	//for mono only, and for passive stereo
	OSG::ViewportPtr SceneViewOSG::GetViewport() const
	{
		return UsesSSM() ? m_pSimpleSceneManager->getWindow()->getPort(0) : GetLeftViewport() ;
	}

	OSG::PerspectiveCameraPtr SceneViewOSG::GetCamera() const
	{
		return UsesSSM() ? 
		OSG::PerspectiveCameraPtr::dcast(m_pSimpleSceneManager->getCamera()) 
		: 
		OSG::PerspectiveCameraPtr::dcast(GetLeftCamera());
	}

	void SceneViewOSG::Redraw()
	{
		if ( UsesSSM() ) {
			m_pSimpleSceneManager->redraw();
		} else {

		}
	}

	void SceneViewOSG::SetRoot( OSG::NodePtr root )
	{
		if ( UsesSSM() && !IsStereo()) {
			m_pSimpleSceneManager->setRoot( root );
		} else if (IsStereo()) {
			beginEditCP(m_LeftViewportPtr);
			m_LeftViewportPtr->setRoot(root);
			endEditCP(m_LeftViewportPtr);

			beginEditCP(m_RightViewportPtr);
			m_RightViewportPtr->setRoot(root);
			endEditCP(m_RightViewportPtr);
		}
	}

	OSG::DrawActionBase *SceneViewOSG::GetAction() const 
	{
		if ( UsesSSM() ) {
			return m_pSimpleSceneManager->getAction();
		} else {
			return m_pRenderAction;
		} 
	}

	OSG::Line SceneViewOSG::CalcViewRay(OSG::UInt32 x, OSG::UInt32 y)
	{
		if ( UsesSSM() ) {
			return m_pSimpleSceneManager->calcViewRay( x, y );
		} else {
			//TODO
			return OSG::Line(OSG::Pnt3f(0,0,0) ,OSG::Pnt3f(0,0,1) );//depends if it is externally created... later
		}
	}

//this should be called only during init of the app due to the many begins/ends
//TODO set the field bitmasks
	void SceneViewOSG::SetBackgroundColor( OSG::Color3f backColor )
	{
		//init 
		if ( m_SolidBackgroundPtr==osg::NullFC ) {
			m_SolidBackgroundPtr = OSG::SolidBackground::create();
			if ( !IsStereo() ) {
				//GetViewport defaults to the left vp
				OSG::ViewportPtr &vport = GetViewport();
				beginEditCP( vport );
				vport->setBackground(m_SolidBackgroundPtr);
				endEditCP( vport );
			} else {
				OSG::ViewportPtr &lvport = GetLeftViewport();
				beginEditCP( lvport );
				lvport->setBackground(m_SolidBackgroundPtr);
				endEditCP( lvport );

				OSG::ViewportPtr &rvport = GetRightViewport();
				beginEditCP( rvport );
				rvport->setBackground(m_SolidBackgroundPtr);
				endEditCP( rvport );
			}
		}

		//use bitmask to speed it up
		beginEditCP(m_SolidBackgroundPtr, OSG::SolidBackground::ColorFieldMask);
		m_SolidBackgroundPtr->setColor( backColor );
		endEditCP(m_SolidBackgroundPtr, OSG::SolidBackground::ColorFieldMask);


	}



	
