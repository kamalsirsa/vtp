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
const bool USE_RENDER_STATISTICS    (false);
const bool USE_OPENSG_LOGO          (true);
const bool USE_HEADLIGHT            (false);

#define ANAGLYPHIC 0  //hmm i don't need the enum hack
#define QUADBUFFER 1
#define PASSIVE    2 


//takes mono components for usage within VTP
SceneViewOSG::SceneViewOSG(
						  OSG::WindowPtr window, 
						  OSG::ViewportPtr viewport,
						  OSG::PerspectiveCameraPtr camera,
						  OSG::RenderAction *renderAction,
						  int stereoMode,
						  OSG::SolidBackgroundPtr background
						   ) :  
m_bStereo ( stereoMode>0 ),
m_bUsesCustomSceneView (true), 
m_WindowPtr ( window ),
m_LeftViewportPtr ( /*OSG::ShadowMapViewportPtr::dcast(*/viewport ),
m_LeftCameraPtr ( camera ),
m_pRenderAction ( renderAction ),
m_iStereoMode ( stereoMode ),
m_SolidBackgroundPtr ( background )
{
	renderAction->setFrustumCulling( true );
    renderAction->setAutoFrustum( true );
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
SceneViewOSG::SceneViewOSG( bool bStereo, int iStereoMode ) : 
m_bStereo(bStereo), 
m_iStereoMode (iStereoMode),
m_bUsesCustomSceneView (false),
m_bUsesShadow(false)
{
	if ( IsStereo() ) {
		switch ( m_iStereoMode ) {
		case ANAGLYPHIC: break;
		case QUADBUFFER: break;
			/* P A S S I V E  S T E R E O */
		case PASSIVE   : 

			break;
		default: ;
		}

	} else {
		/* M O N O  W/O  S I M P L E S C E N E M A N A G E R */
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
		
		// Shadow viewport
		/*m_LeftViewportPtr = OSG::ShadowMapViewport::create();
		beginEditCP(m_LeftViewportPtr);
		m_LeftViewportPtr->setSize(0,0,1,1);
		m_LeftViewportPtr->setOffFactor(10.0);
		m_LeftViewportPtr->setOffBias(4.0);
		m_LeftViewportPtr->setShadowColor(osg::Color4f(0.1, 0.1, 0.1, 1.0));
		m_LeftViewportPtr->setMapSize(256);
		// you can add the light sources here, as default all light source in
		// the scenegraph are used.
		m_LeftViewportPtr->setCamera( m_LeftCameraPtr );

		endEditCP(m_LeftViewportPtr);*/


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
	}
}

void SceneViewOSG::UpdateCamera(
							   float aspect, 
							   float fov_y,
							   float hither, 
							   float yon )
{

	if ( !IsStereo() ||  GetStereoMode() == PASSIVE ) {
		osg::PerspectiveCameraPtr cam = GetCamera();
		beginEditCP(cam);
		cam->setAspect(aspect);
		cam->setFov(fov_y);
		cam->setNear(hither);
		cam->setFar(yon);
		endEditCP(cam);
	}
}

OSG::WindowPtr SceneViewOSG::GetWindow() const
{
	return m_WindowPtr;
}

//for mono only, and for passive stereo
OSG::ViewportPtr/*OSG::ShadowMapViewportPtr*/ SceneViewOSG::GetViewport() const
{
	return  GetLeftViewport() ;
}

OSG::PerspectiveCameraPtr SceneViewOSG::GetCamera() const
{
	return OSG::PerspectiveCameraPtr::dcast(GetLeftCamera());
}

void SceneViewOSG::Redraw()
{
	//we don't need to apply the renderaction within vtp,
	//this should be done within the custom framework.
	if (!m_bUsesCustomSceneView) {
		m_WindowPtr->render(m_pRenderAction);
	}
}

void SceneViewOSG::SetShadowOn( bool bOn )
{
	/*if ( !IsStereo() ) {
		beginEditCP( m_LeftViewportPtr );
		m_LeftViewportPtr->setShadowOn(bOn); 
		endEditCP( m_LeftViewportPtr );
	} else {
		beginEditCP( m_LeftViewportPtr );
		m_LeftViewportPtr->setShadowOn(bOn); 
		endEditCP( m_LeftViewportPtr );

		beginEditCP( m_RightViewportPtr );
		m_RightViewportPtr->setShadowOn(bOn); 
		endEditCP( m_RightViewportPtr );
	}*/

	m_bUsesShadow = bOn;
}

void SceneViewOSG::SetRoot( OSG::NodePtr root )
{
	if ( !IsStereo() ) {
		beginEditCP( m_LeftViewportPtr );
		m_LeftViewportPtr->setRoot( root ); 
		endEditCP( m_LeftViewportPtr );
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
	return m_pRenderAction;
}

OSG::Line SceneViewOSG::CalcViewRay(OSG::UInt32 x, OSG::UInt32 y)
{
	//if ( !IsStereo() ) {
		osg::Line l;
		m_LeftCameraPtr->calcViewRay(l, x, y, *m_LeftViewportPtr);
		return l;
	//} 

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



