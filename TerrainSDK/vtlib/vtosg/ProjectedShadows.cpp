//
// ProjectedShadows.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/HeightField.h"
#include "vtlib/core/LodGrid.h"

#include <osg/Texture2D>
#include <osg/Material>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/ShapeDrawable>

#include <osgUtil/CullVisitor>

#include "ProjectedShadows.h"

using namespace osg;

#define SHADOW_UPDATE_ANGLE 5.0f

void CreateProjectedShadowTextureCullCallback::doPreRender(osg::Node& node,
														   osgUtil::CullVisitor &cv)
{
	if (m_bRecomputeShadows)
	{
		m_bRecomputeShadows = false;

		const osg::BoundingSphere& bsShadowed = node.getBound();
		if (!bsShadowed.valid())
		{
			osg::notify(osg::WARN) << "bb invalid"<<&node<<std::endl;
			return;
		}
		const osg::BoundingSphere& bsShadower = m_shadower->getBound();
		if (!bsShadower.valid())
		{
			osg::notify(osg::WARN) << "bb invalid"<<m_shadower.get()<<std::endl;
			return;
		}
		float centerDistanceShadowed = (m_position-bsShadowed.center()).length();
		float centerDistanceShadower = (m_position-bsShadower.center()).length();
		float znear;
		float zfar;
		// Set ortho near to front of shadowed
		// Set ortho far to rear of shadower
		znear = centerDistanceShadower - bsShadower.radius();
		zfar  = centerDistanceShadowed + bsShadowed.radius();
		if (zfar <= znear)
		{
			osg::notify(osg::WARN) << "near far invalid"<<std::endl;
			return;
		}
		float zNearRatio = 0.001f;
		if (znear<zfar*zNearRatio)
			znear = zfar*zNearRatio;

		float top;
		top = bsShadower.radius();
		float right = top;

		// set up projection.
		osg::RefMatrix* ProjectionMatrix = new osg::RefMatrix;
		ProjectionMatrix->makeOrtho(-right,right,-top,top,znear,zfar);

		// Set up the view matrix
		osg::RefMatrix* ViewMatrix = new osg::RefMatrix;
		ViewMatrix->makeLookAt(m_position,bsShadower.center(),osg::Vec3(0.0f,1.0f,0.0f));
		
		// create the render to texture stage.
		if (!m_pRtts.valid())
		{
#ifdef _DEBUG
			m_pRtts = new MyRenderStage;
#else
			m_pRtts = new osgUtil::RenderToTextureStage;
#endif
		}
		else
			m_pRtts->reset();

		// set up lighting.
		// currently ignore lights in the scene graph itself..
		// will do later.
		osgUtil::RenderStage* previous_stage = cv.getCurrentRenderBin()->getStage();

		// set up the background color and clear mask.
		m_pRtts->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		m_pRtts->setClearMask(previous_stage->getClearMask());

		// set up to charge the same RenderStageLighting is the parent previous stage.
		m_pRtts->setRenderStageLighting(previous_stage->getRenderStageLighting());

		// record the render bin, to be restored after creation
		// of the render to text
		osgUtil::RenderBin* previousRenderBin = cv.getCurrentRenderBin();

		// set the current renderbin to be the newly created stage.
		cv.setCurrentRenderBin(m_pRtts.get());

		cv.pushModelViewMatrix(ViewMatrix);

		cv.pushProjectionMatrix(ProjectionMatrix);

		// compute the matrix which takes a vertex from local coords into tex coords
		// will use this later to specify osg::TexGen..
		m_VPOSTransform = *ViewMatrix * 
				*ProjectionMatrix *
				osg::Matrix::translate(1.0f,1.0f,1.0f) *
				osg::Matrix::scale(0.5f,0.5f,0.5f);

		// make the material black for a shadow.
		osg::Material* material = new osg::Material;
		material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
		material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
//		material->setEmission(osg::Material::FRONT_AND_BACK,m_ambientLightColor);
		// Do not pick up the emmissive colour from the sun light ambient or else
		// we will lose shadows when the ambient peaks in the middle of the day
		material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
		material->setShininess(osg::Material::FRONT_AND_BACK,0.0f);
		m_shadowState->setAttribute(material,osg::StateAttribute::OVERRIDE);

		// Kill any textures
		m_shadowState->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);

		cv.pushStateSet(m_shadowState.get());

		// No shadows after sunset
		if (m_position.y() > 0.0f)
		{
			// Stop the shadowed node being included in the shadow cull
			osg::Node::NodeMask  NodeMask = node.getNodeMask();
			node.setNodeMask(0);

			// Need to turn off LOD culling temporarily
			// shadower node should be a lod grid
			// this needs more work if other lod grids further down the tree
			vtLodGrid *pGrid = dynamic_cast<vtLodGrid*>(m_shadower->getUserData());
			float fOldDistance;

			if (NULL != pGrid)
			{
				fOldDistance = pGrid->GetDistance();
				pGrid->SetDistance(1E9);
			}

			// traverse the shadower
			m_shadower->accept(cv);

 			if (NULL != pGrid)
				pGrid->SetDistance(fOldDistance);

			// Restore the shadowed to the subgraph
			node.setNodeMask(NodeMask);
		}

		cv.popStateSet();

		// restore the previous model view matrix.
		cv.popModelViewMatrix();

		// restore the previous model view matrix.
		cv.popProjectionMatrix();

		// restore the previous renderbin.
		cv.setCurrentRenderBin(previousRenderBin);

		int height = m_iRez;
		int width  = m_iRez;

		const osg::Viewport& viewport = *cv.getViewport();

		// offset the impostor viewport from the center of the main window
		// viewport as often the edges of the viewport might be obscured by
		// other windows, which can cause image/reading writing problems.
		int center_x = viewport.x()+viewport.width()/2;
		int center_y = viewport.y()+viewport.height()/2;

		osg::Viewport* new_viewport = new osg::Viewport;
		new_viewport->setViewport(center_x-width/2,center_y-height/2,width,height);
		m_pRtts->setViewport(new_viewport);
		
		m_shadowState->setAttribute(new_viewport);

		// add the render to texture stage to the current stages
		// dependancy list.
		cv.getCurrentRenderBin()->getStage()->addToDependencyList(m_pRtts.get());

		// if one exists attach texture to the RenderToTextureStage.
		if (m_texture.valid())
			m_pRtts->setTexture(m_texture.get());
	}
	else if (!m_pRtts.valid())
		return;

	// set up the stateset to decorate the shadower with the shadow texture
	// with the appropriate tex gen coords.
	MyTexGen* texgen = new MyTexGen;
	texgen->setMatrix(cv.getState()->getInitialViewMatrix());
	texgen->setMode(osg::TexGen::EYE_LINEAR);
	texgen->setPlane(osg::TexGen::S,osg::Plane(m_VPOSTransform(0,0),m_VPOSTransform(1,0),m_VPOSTransform(2,0),m_VPOSTransform(3,0)));
	texgen->setPlane(osg::TexGen::T,osg::Plane(m_VPOSTransform(0,1),m_VPOSTransform(1,1),m_VPOSTransform(2,1),m_VPOSTransform(3,1)));
	texgen->setPlane(osg::TexGen::R,osg::Plane(m_VPOSTransform(0,2),m_VPOSTransform(1,2),m_VPOSTransform(2,2),m_VPOSTransform(3,2)));
	texgen->setPlane(osg::TexGen::Q,osg::Plane(m_VPOSTransform(0,3),m_VPOSTransform(1,3),m_VPOSTransform(2,3),m_VPOSTransform(3,3)));

	m_shadowedState->setTextureAttributeAndModes(m_unit,m_texture.get(),osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
	m_shadowedState->setTextureAttribute(m_unit,texgen, osg::StateAttribute::OVERRIDE);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_S,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_T,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_R,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_Q,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

	cv.pushStateSet(m_shadowedState.get());

		// must traverse the shadowed           
		traverse(&node,&cv);

	cv.popStateSet();
}

void CreateProjectedShadowTextureCullCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgUtil::CullVisitor* cullVisitor = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (cullVisitor && (m_texture.valid() && m_shadower.valid()))
    {
        doPreRender(*node,*cullVisitor);
    }
    else
    {
        // must traverse the shadower            
        traverse(node,nv);
    }
}

void CreateProjectedShadowTextureCullCallback::SetLightPosition(const osg::Vec3& position)
{
	osg::Vec3 OldPosition = m_position;
	osg::Vec3 NewPosition = position;

	OldPosition.normalize();
	NewPosition.normalize();

	if (acos(OldPosition * NewPosition) > PIf * SHADOW_UPDATE_ANGLE / 180.0f)
	{
		m_position = position;
		m_bRecomputeShadows = true;
	}
}

