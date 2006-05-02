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
#define	   USE_SSM_WHEN_POSSIBLE		0
const bool USE_RENDER_STATISTICS    (false);
const bool USE_OPENSG_LOGO          (true);
const bool USE_HEADLIGHT            (false);

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
 * \param iStereoMode Currently for vtosg, supported values are:
 * 0 for Anaglyphic (red-blue)
 * 1 for Quad-buffer (shutter glasses)
 * 2 for Passive Stereo (2 projectors)
 * 
 * @param bStereo
 * @param iStereoMode
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
		/* M O N O  W/  S I M P L E S C E N E M A N A G E R */
#if USE_SSM_WHEN_POSSIBLE
		m_bUsesSSM = true;

		OSG::PassiveWindowPtr gwin = osg::PassiveWindow::create();
		gwin->init();

		m_pSimpleSceneManager = new OSG::SimpleSceneManager;

		GetSSM()->setWindow(gwin);
		GetSSM()->setHeadlight( USE_HEADLIGHT );
		GetSSM()->turnHeadlightOff();
		GetSSM()->setStatistics( USE_RENDER_STATISTICS );
		if ( USE_OPENSG_LOGO ) {
			GetSSM()->useOpenSGLogo();
		}
		m_pRenderAction = static_cast<OSG::RenderAction*> (GetSSM()->getAction());
		//raction->setLocalLights( true );
		//raction->setZWriteTrans(true);
		//raction->setFrustumCulling( true );
		//raction->setAutoFrustum( true );

#else   /* M O N O  W/O  S I M P L E S C E N E M A N A G E R */
		m_bUsesSSM = false;
		m_bStereo = false;

		//only create rudimentary components
		//fields are set in vtScene
		//note that NO lights will be created here

		//create a perspective camera
		m_LeftCameraPtr = OSG::PerspectiveCamera::create();

		//and a viewport
		m_LeftViewportPtr = OSG::Viewport::create();
		beginEditCP( m_LeftViewportPtr );
		m_LeftViewportPtr->setCamera( m_LeftCameraPtr );
		m_LeftViewportPtr->setSize( 0,0,1,1 );
		endEditCP( m_LeftViewportPtr );

		//the one and only window
		m_WindowPtr = OSG::PassiveWindow::create();
		m_WindowPtr->addPort( m_LeftViewportPtr );
		m_WindowPtr->init();
		//probably not necessary
		m_WindowPtr->setSize( 800, 600 );

		//and at last a renderaction
		m_pRenderAction = OSG::RenderAction::create();

		//currently, vtp only uses one directional light
		//m_pRenderAction->setLocalLights( true );
		//m_pRenderAction->setZWriteTrans(true);
		//m_pRenderAction->setFrustumCulling( true );
		//m_pRenderAction->setAutoFrustum(true );
#endif
	}
}

void SceneViewOSG::UpdateCamera(
							   float aspect, 
							   float fov_y,
							   float hither, 
							   float yon, 
							   OSG::NodePtr node )
{

	if ( !IsStereo() ||  GetStereoMode() == PASSIVE ) {
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
	return UsesSSM() ? GetSSM()->getWindow() : m_WindowPtr;
}

//for mono only, and for passive stereo
OSG::ViewportPtr SceneViewOSG::GetViewport() const
{
	return UsesSSM() ? GetSSM()->getWindow()->getPort(0) : GetLeftViewport() ;
}

OSG::PerspectiveCameraPtr SceneViewOSG::GetCamera() const
{
	return UsesSSM() ? 
	OSG::PerspectiveCameraPtr::dcast( GetSSM()->getCamera() ) 
	: 
	OSG::PerspectiveCameraPtr::dcast(GetLeftCamera());
}

void SceneViewOSG::Redraw()
{
	if ( UsesSSM() ) {
		GetSSM()->redraw();
	} else {
		m_WindowPtr->render(m_pRenderAction);
	}
}

void SceneViewOSG::SetRoot( OSG::NodePtr root )
{
	if ( !IsStereo() ) {
		if ( UsesSSM() ) {
			GetSSM()->setRoot( root );
		} else {
			beginEditCP( m_LeftViewportPtr );
			m_LeftViewportPtr->setRoot( root ); 
			endEditCP( m_LeftViewportPtr );
		}
	} else {
		beginEditCP( m_LeftViewportPtr );
		m_LeftViewportPtr->setRoot(root);
		endEditCP( m_LeftViewportPtr );

		beginEditCP( m_RightViewportPtr );
		m_RightViewportPtr->setRoot(root);
		endEditCP( m_RightViewportPtr );
	}
}

OSG::DrawActionBase *SceneViewOSG::GetAction() const 
{
	if ( UsesSSM() ) {
		return GetSSM()->getAction();
	} else {
		return m_pRenderAction;
	} 
}

OSG::Line SceneViewOSG::CalcViewRay(OSG::UInt32 x, OSG::UInt32 y)
{
	if ( !IsStereo() ) {
		if ( UsesSSM() ) {
			return m_pSimpleSceneManager->calcViewRay( x, y );
		} else {
			osg::Line l;
			m_LeftCameraPtr->calcViewRay(l, x, y, *m_LeftViewportPtr);
			return l;
		}
	} 

	//shouldn't come here, but to make compiler happy
	return OSG::Line( OSG::Pnt3f(0,0,0), OSG::Pnt3f(0,0,0) );
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



