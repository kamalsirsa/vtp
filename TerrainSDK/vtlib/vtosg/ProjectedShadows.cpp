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
#include <osg/PolygonOffset>
#include <osg/CullFace>

#include <osgUtil/CullVisitor>

#include "ProjectedShadows.h"

#define SHADOW_UPDATE_ANGLE 5.0f
#define SA_OVERRIDE	osg::StateAttribute::OVERRIDE
#define SA_OFF		osg::StateAttribute::OFF
#define SA_ON		osg::StateAttribute::ON

CreateProjectedShadowTextureCullCallback::CreateProjectedShadowTextureCullCallback(
	osg::Node *shadower, int iRez, const osg::Vec3& position,
	const osg::Vec4 &ambientLightColor, unsigned int textureUnit) :
		m_shadower(shadower),
		m_position(position),
			m_ambientLightColor(ambientLightColor),
		m_unit(textureUnit),
		m_shadowState(new osg::StateSet),
		m_shadowedState(new osg::StateSet)
{
	int contextID = 0;
	osg::ref_ptr<osg::Texture::Extensions> pExtensions = new osg::Texture::Extensions(contextID);
	pExtensions->setupGLExtensions(contextID);
	if (pExtensions->isShadowSupported())
		m_bDepthShadow = true;
	else
		m_bDepthShadow = false;

	// BD note: the main effect i see from DepthShadow=true is that shadows lose
	//	their soft edges.
	m_bDepthShadow = false;

#ifdef _DEBUG
	m_texture = new MyTexture2D;
#else
	m_texture = new osg::Texture2D;
#endif
	if (m_bDepthShadow)
	{
		m_texture->setInternalFormat(GL_DEPTH_COMPONENT);
		m_texture->setShadowComparison(true);
		m_texture->setShadowTextureMode(osg::Texture::LUMINANCE);

		osg::ref_ptr<osg::PolygonOffset> pPolygonOffset = new osg::PolygonOffset;
		pPolygonOffset->setFactor(1.1f);
		pPolygonOffset->setUnits(4.0f);
		m_shadowState->setAttribute(pPolygonOffset.get(), SA_ON|SA_OVERRIDE);
		m_shadowState->setMode(GL_POLYGON_OFFSET_FILL, SA_ON|SA_OVERRIDE);
	}
	else
	{
		m_texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		m_texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
	}

	osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace;
	pCullFace->setMode(osg::CullFace::FRONT);
	m_shadowState->setAttribute(pCullFace.get(), SA_ON|SA_OVERRIDE);
	m_shadowState->setMode(GL_CULL_FACE, SA_ON|SA_OVERRIDE);
	m_texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
	m_texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
	m_texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
	m_bRecomputeShadows = false;
	m_iRez = iRez;
	m_fShadowDarkness = 0.8f;

	shadow_ignore_nodes = new Array<osg::Node *>;
}

void CreateProjectedShadowTextureCullCallback::doPreRender(osg::Node &node,
														   osgUtil::CullVisitor &cv)
{
	if (m_bRecomputeShadows)
		DoRecomputeShadows(node, cv);
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

	m_shadowedState->setTextureAttributeAndModes(m_unit,m_texture.get(),SA_OVERRIDE|SA_ON);
	m_shadowedState->setTextureAttribute(m_unit,texgen, SA_OVERRIDE);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_S,SA_OVERRIDE|SA_ON);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_T,SA_OVERRIDE|SA_ON);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_R,SA_OVERRIDE|SA_ON);
	m_shadowedState->setTextureMode(m_unit,GL_TEXTURE_GEN_Q,SA_OVERRIDE|SA_ON);

	cv.pushStateSet(m_shadowedState.get());

		// must traverse the shadowed
		traverse(&node,&cv);

	cv.popStateSet();
}

void CreateProjectedShadowTextureCullCallback::DoRecomputeShadows(osg::Node& node,
																  osgUtil::CullVisitor &cv)
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
	if (m_bDepthShadow)
	{
		m_pRtts->setClearMask(GL_DEPTH_BUFFER_BIT);
		m_pRtts->setColorMask(new osg::ColorMask(false, false, false, false));
	}
	else
	{
		// set up the background color and clear mask.
		m_pRtts->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		m_pRtts->setClearMask(previous_stage->getClearMask());
	}

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

	if (!m_bDepthShadow)
	{
		// make the material black for a shadow.
		m_material = new osg::Material;
		m_material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
		m_material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,m_fShadowDarkness));
//		material->setEmission(osg::Material::FRONT_AND_BACK,m_ambientLightColor);
		// Do not pick up the emmissive colour from the sun light ambient or else
		// we will lose shadows when the ambient peaks in the middle of the day
		m_material->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
		m_material->setShininess(osg::Material::FRONT_AND_BACK,0.0f);
		m_shadowState->setAttribute(m_material.get(), SA_OVERRIDE);

#if 1
		// Roger suggested this code on 2004.05.27, to avoid the shadows cast by
		//  the front faces of the buildings.  Seems to work well!
		osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace;
		pCullFace->setMode(osg::CullFace::FRONT);
		m_shadowState->setAttribute(pCullFace.get(), SA_ON|SA_OVERRIDE);
		m_shadowState->setMode(GL_CULL_FACE, SA_ON|SA_OVERRIDE);
#endif

#if 1
		// This blend function lets us vary the darkness of the shadow with the
		//  alpha component of the diffure color of the material.
		osg::ref_ptr<osg::BlendFunc>	pBlendFunc = new osg::BlendFunc;
		m_shadowState->setAttributeAndModes(pBlendFunc.get(), SA_ON|SA_OVERRIDE);
#endif

		// Kill any textures
		m_shadowState->setTextureMode(0,GL_TEXTURE_2D,SA_OVERRIDE|SA_OFF);
	}

	cv.pushStateSet(m_shadowState.get());

	// No shadows after sunset
	if (m_position.y() > 0.0f)
	{
		osg::Node::NodeMask  NodeMask;
	
		if (!m_bDepthShadow)
		{
			// Stop the shadowed node being included in the shadow cull
			NodeMask = node.getNodeMask();
			node.setNodeMask(0);
		}

		// Need to turn off LOD culling temporarily
		// shadower node should be a lod grid
		// this needs more work if other lod grids further down the tree
		vtLodGrid *pGrid = dynamic_cast<vtLodGrid*>(m_shadower->getUserData());
		float fOldDistance=0;

		if (NULL != pGrid)
		{
			fOldDistance = pGrid->GetDistance();
			pGrid->SetDistance(1E9);
		}

		Array<osg::Node::NodeMask> nodemasks(shadow_ignore_nodes->GetSize());
#define TOGGLE_SHADOWS 1
#ifdef TOGGLE_SHADOWS
		/* Set ignore node masks */
		for (unsigned int i = 0; i < shadow_ignore_nodes->GetSize(); i++) {
			osg::Node *n = (*shadow_ignore_nodes)[i];
			nodemasks[i] = n->getNodeMask();
			n->setNodeMask(0);
		}
#endif

		// traverse the shadower
		m_shadower->accept(cv);

		/* Restore ignore node masks */
#if TOGGLE_SHADOWS
		for (unsigned int i = 0; i < shadow_ignore_nodes->GetSize(); i++) {
			osg::Node *n = (*shadow_ignore_nodes)[i];
			n->setNodeMask(nodemasks[i]);
		}
#endif
	
 		if (NULL != pGrid)
			pGrid->SetDistance(fOldDistance);

		if (!m_bDepthShadow)
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

void CreateProjectedShadowTextureCullCallback::SetShadowDarkness(float fDarkness)
{
	m_fShadowDarkness = fDarkness;
	if (m_material.valid())
		m_material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,m_fShadowDarkness));
}
