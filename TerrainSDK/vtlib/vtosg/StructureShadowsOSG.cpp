//
// StructureShadowsOSG.cpp
//
// Copyright (c) 2005-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#if OLD_OSG_SHADOWS

#include "vtdata/HeightField.h"
#include "vtlib/core/LodGrid.h"
#include "vtlib/core/PagedLodGrid.h"
#include "vtdata/vtLog.h"

#include "StructureShadowsOSG.h"
#include <osg/CullFace>
#include <osgDB/WriteFile>
#include <osg/GLExtensions>
#include <osg/FragmentProgram>
#include <osg/Version>

#define USE_SHADOW_HUD		0
#define USE_FRAGMENT_SHADER	0

#define SHADOW_UPDATE_ANGLE 5.0f

// These must match
#define MAIN_SCENE_TEXTURE_UNIT 0
#define MAIN_SCENE_TEXTURE_UNIT_STR "0"

#define SA_OVERRIDE	osg::StateAttribute::OVERRIDE
#define SA_OFF		osg::StateAttribute::OFF
#define SA_ON		osg::StateAttribute::ON

osg::TexGenNode *CStructureShadowsOSG::m_pStructureTexGenNode = NULL;

CStructureShadowsOSG::CStructureShadowsOSG(bool bDepthShadow, bool bStructureOnStructureShadows)
{
//	bDepthShadow = true;
//	bStructureOnStructureShadows = true;

	// Check if depth shadows supported
#if USE_FRAGMENT_SHADER
	osg::ref_ptr<osg::FragmentProgram::Extensions> pFragmentExtensions
		= osg::FragmentProgram::getExtensions(vtGetScene()->GetGraphicsContext()->getState()->getContextID(), true);
	if (bDepthShadow && pTextureExtensions->isShadowSupported() && pFragmentExtensions->isFragmentProgramSupported())
#else
	osg::ref_ptr<osg::Texture::Extensions> pTextureExtensions =
		osg::Texture::getExtensions(vtGetScene()->GetGraphicsContext()->getState()->getContextID(), true);
	if (bDepthShadow && pTextureExtensions->isShadowSupported())
#endif
		m_bDepthShadow = true;
	else
		m_bDepthShadow = false;
	if (m_bDepthShadow && bStructureOnStructureShadows)
		m_bStructureOnStructureShadows = true;
	else
		m_bStructureOnStructureShadows = false;
	m_fShadowDarkness = 0.8f;
	m_fPolygonOffsetFactor = 1.5f;
	m_fPolygonOffsetUnits = 1.0f;
}

CStructureShadowsOSG::~CStructureShadowsOSG()
{
	m_pShadowed = NULL;
	m_pTexture = NULL;
	m_pMaterial = NULL;
	m_pSceneView = NULL;
	m_pAmbientBias = NULL;
	m_pPolygonOffset = NULL;
	if (m_pCameraNode.valid())
	{
		m_pCameraNode->removeChild(0, m_pCameraNode->getNumChildren());
		m_pCameraNode = NULL;
	}
	if (m_pTop.valid())
	{
		if (m_pTerrainTexGenNode.valid())
		{
			m_pTop->removeChild(m_pTerrainTexGenNode.get());
			m_pTerrainTexGenNode = NULL;
		}
		m_pTop = NULL;
	}
}

bool CStructureShadowsOSG::Initialise(osgUtil::SceneView *pSceneView,
	osg::Node *pStructures, osg::Node *pShadowed, const int iResolution,
	float fDarkness, int iTextureUnit, const FSphere &ShadowSphere)
{
	m_fShadowDarkness = fDarkness;
	m_pSceneView = pSceneView;
	m_iTargetResolution = m_iCurrentResolution = iResolution;
	m_pTexture = new osg::Texture2D;
	m_ShadowSphere = ShadowSphere;
	if (!m_pTexture.valid())
		return false;

	// Texture size and viewport are place holders at the moment
	// if I am using the real frame buffer I will need to clamp
	// it to the actual current window size
	m_pTexture->setTextureSize(m_iCurrentResolution, m_iCurrentResolution);
	m_pTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	m_pTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

	if (m_bDepthShadow)
	{
		m_pTexture->setInternalFormat(GL_DEPTH_COMPONENT);
		m_pTexture->setShadowComparison(true);
		m_pTexture->setShadowTextureMode(osg::Texture::LUMINANCE);
		m_pTexture->setShadowAmbient(1.0f - m_fShadowDarkness);
	}
	else
	{
		m_pTexture->setInternalFormat(GL_RGB);
		m_pTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
		m_pTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
		m_pTexture->setBorderColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	m_pCameraNode = new osg::CameraNode;
	if (!m_pCameraNode.valid())
	{
		m_pTexture = NULL;
		return false;
	}

	m_pCameraNode->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	m_pCameraNode->setViewport(0, 0, m_iCurrentResolution, m_iCurrentResolution);
	m_pCameraNode->setRenderOrder(osg::CameraNode::PRE_RENDER);
	m_pCameraNode->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT,
		osg::CameraNode::FRAME_BUFFER);
	m_pCameraNode->setComputeNearFarMode(osg::CameraNode::DO_NOT_COMPUTE_NEAR_FAR);
	if (m_bDepthShadow)
	{
		osg::ref_ptr<osg::Group> pGroup = new osg::Group;
		if (!pGroup.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		pGroup->addChild(pStructures);
		pGroup->addChild(pShadowed);
		m_pCameraNode->setClearMask(GL_DEPTH_BUFFER_BIT);
		m_pCameraNode->attach(osg::CameraNode::DEPTH_BUFFER, m_pTexture.get());
		m_pCameraNode->addChild(pGroup.get());
	}
	else
	{
		m_pCameraNode->attach(osg::CameraNode::COLOR_BUFFER, m_pTexture.get());
		m_pCameraNode->addChild(pStructures);
	}

	osg::StateSet *pCameraStateSet = m_pCameraNode->getOrCreateStateSet();;
	if (NULL == pCameraStateSet)
	{
		m_pTexture = NULL;
		m_pCameraNode = NULL;
		return false;
	}

	// Kill any textures
	pCameraStateSet->setTextureMode(MAIN_SCENE_TEXTURE_UNIT, GL_TEXTURE_2D, SA_OVERRIDE | SA_OFF);

	// Avoid the shadows cast by the front faces of the buildings.
	osg::ref_ptr<osg::CullFace> pCullFace = new osg::CullFace;
	if (!pCullFace.valid())
	{
		m_pTexture = NULL;
		m_pCameraNode = NULL;
		m_pMaterial = NULL;
		return false;
	}
	pCullFace->setMode(osg::CullFace::FRONT);
	pCameraStateSet->setAttribute(pCullFace.get(), SA_ON|SA_OVERRIDE);
	pCameraStateSet->setMode(GL_CULL_FACE, SA_ON|SA_OVERRIDE);
	if (m_bDepthShadow)
	{
		pCameraStateSet->setMode(GL_LIGHTING, SA_OFF);
		// Polygon offset
		m_pPolygonOffset = new osg::PolygonOffset;
		if (!m_pPolygonOffset.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		m_pPolygonOffset->setFactor(m_fPolygonOffsetFactor);
		m_pPolygonOffset->setUnits(m_fPolygonOffsetUnits);
		pCameraStateSet->setAttribute(m_pPolygonOffset.get(), SA_ON | SA_OVERRIDE);
		pCameraStateSet->setMode(GL_POLYGON_OFFSET_FILL, SA_ON | SA_OVERRIDE);
	}
	else
	{
		m_pMaterial = new osg::Material;
		if (!m_pMaterial.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		m_pMaterial->setAmbient(FAB, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		m_pMaterial->setDiffuse(FAB, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
		m_pMaterial->setEmission(FAB, osg::Vec4(1.0f - m_fShadowDarkness, 1.0f - m_fShadowDarkness, 1.0f - m_fShadowDarkness, 1.0f));
		m_pMaterial->setShininess(FAB, 0.0f);
		pCameraStateSet->setAttribute(m_pMaterial.get(), SA_OVERRIDE);
	}

	m_pShadowed = pShadowed;

	// Set up texgen node
	m_pTerrainTexGenNode = new osg::TexGenNode;
	if (!m_pTerrainTexGenNode.valid())
	{
		m_pTexture = NULL;
		m_pCameraNode = NULL;
		m_pMaterial = NULL;
		m_pPolygonOffset = NULL;
		return false;
	}
	m_pTerrainTexGenNode->setTextureUnit(iTextureUnit);
	m_pTop = dynamic_cast<osg::Group*>(m_pSceneView->getSceneData());
	m_pTop->addChild(m_pTerrainTexGenNode.get());

	// Decorate the shadowed node
	osg::StateSet *pShadowedStateSet = m_pShadowed->getOrCreateStateSet();
	pShadowedStateSet->setTextureAttributeAndModes(iTextureUnit, m_pTexture.get(), SA_OVERRIDE|SA_ON);
	pShadowedStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_S, SA_OVERRIDE|SA_ON);
	pShadowedStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_T, SA_OVERRIDE|SA_ON);
	pShadowedStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_R, SA_OVERRIDE|SA_ON);
	pShadowedStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_Q, SA_OVERRIDE|SA_ON);

	if (m_bDepthShadow)
	{
#if USE_FRAGMENT_SHADER
		static char FragmentShaderSource[] =
			"uniform sampler2D MainSceneTexture; \n"
			"uniform sampler2DShadow ShadowTexture; \n"
			"uniform float ShadowDarkness; \n"
			"\n"
			"void main(void) \n"
			"{ \n"
			"	vec4 colour = texture2D( MainSceneTexture, gl_TexCoord[" MAIN_SCENE_TEXTURE_UNIT_STR "].xy ); \n"
			"   vec4 shadow = shadow2DProj( ShadowTexture, gl_TexCoord[" iTextureUnit_STR "] ); \n"
				// Check if a shadow is to be applied
			"   if (shadow.r == 1.0) \n"
				   // texture2D seems to return vec4(0,0,0,0) when there is no texture assigned.
				   // this will not normally be the case for terrain shadows
				   // but may well occur with shadows on buildings. I am not sure If I can rely on this
				   // it may well be GPU implementation dependent.
			"      if (colour.a) \n"
			"	      gl_FragColor = gl_Color * colour; \n"
			"      else \n"
			"	      gl_FragColor = gl_Color; \n"
			"   else \n"
			"   { \n"
			"      if (colour.a) \n"
			"      { \n"
			"         colour = gl_Color * colour; \n"
			"	      gl_FragColor = colour * (1 - ShadowDarkness + shadow * ShadowDarkness); \n"
			"      } \n"
			"      else \n"
			"	      gl_FragColor = gl_Color * (1 - ShadowDarkness + shadow * ShadowDarkness); \n"
			"   } \n"
			"} \n";

		osg::ref_ptr<osg::Program> pProgram = new osg::Program;
		if (!pProgram.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		osg::ref_ptr<osg::Shader> pFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, FragmentShaderSource);
		if (!pFragmentShader.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		osg::ref_ptr<osg::Uniform> pShadowTexture = new osg::Uniform("ShadowTexture", iTextureUnit);
		if (!pShadowTexture.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		osg::ref_ptr<osg::Uniform> pMainTexture = new osg::Uniform("MainSceneTexture", MAIN_SCENE_TEXTURE_UNIT);
		if (!pMainTexture.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		m_pAmbientBias = new osg::Uniform("ShadowDarkness", m_fShadowDarkness);
		if (!m_pAmbientBias.valid())
		{
			m_pTexture = NULL;
			m_pCameraNode = NULL;
			return false;
		}
		pProgram->addShader(pFragmentShader.get());
		pShadowedStateSet->setAttribute(pProgram.get());
		pShadowedStateSet->addUniform(pMainTexture.get());
		pShadowedStateSet->addUniform(pShadowTexture.get());
		pShadowedStateSet->addUniform(m_pAmbientBias.get());
#endif
		// Structure on structure shadows
		if (m_bStructureOnStructureShadows)
		{
			osg::StateSet *pStructuresStateSet = pStructures->getOrCreateStateSet();
			pStructuresStateSet->setTextureAttributeAndModes(iTextureUnit, m_pTexture.get(), SA_OVERRIDE|SA_ON);
			pStructuresStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_S, SA_OVERRIDE|SA_ON);
			pStructuresStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_T, SA_OVERRIDE|SA_ON);
			pStructuresStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_R, SA_OVERRIDE|SA_ON);
			pStructuresStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_Q, SA_OVERRIDE|SA_ON);

#if USE_FRAGMENT_SHADER
			pStructuresStateSet->setAttribute(pProgram.get());
			pStructuresStateSet->addUniform(pMainTexture.get());
			pStructuresStateSet->addUniform(pShadowTexture.get());
			pStructuresStateSet->addUniform(m_pAmbientBias.get());
#endif
		}
	}

#if USE_SHADOW_HUD
	if (!m_bDepthShadow)
	{
		// Create a little floating HUD which shows the shadow texture
		osg::Geode* pGeode = new osg::Geode;
		osg::Geometry* pGeom = osg::createTexturedQuadGeometry(osg::Vec3(0,0,0), osg::Vec3(100.0,0.0,0.0), osg::Vec3(0.0,100.0,0.0));
		pGeom->getOrCreateStateSet()->setTextureAttributeAndModes(0, m_pTexture.get(), osg::StateAttribute::ON);
		pGeom->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pGeode->addDrawable(pGeom);
		osg::CameraNode* pCamera = new osg::CameraNode;
		pCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,100,0,100));
		pCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		pCamera->setViewMatrix(osg::Matrix::identity());
		pCamera->setViewport(20,20,200,200);
		pCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
		pCamera->setRenderOrder(osg::CameraNode::POST_RENDER);
		pCamera->addChild(pGeode);
		if (m_pTop.valid())
			m_pTop->addChild(pCamera);
	}
#endif

#if OSG_VERSION_MAJOR == 1 && OSG_VERSION_MINOR > 0 || OSG_VERSION_MAJOR > 1
	// We are probably OSG 1.1 or newer
	osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(0, true);
#else
	osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(0);
#endif
	if ((fbo_ext && fbo_ext->isSupported()) || osg::isGLExtensionSupported(0, "ARB_render_texture"))
		m_bUsingLiveFrameBuffer = false;
	else
		m_bUsingLiveFrameBuffer = true;

	return true;
}

void CStructureShadowsOSG::SetSunDirection(osg::Vec3 SunDirection)
{
	SunDirection.normalize();

	osg::Vec3 previous = m_SunDirection;
	m_SunDirection = SunDirection;

	// Recompute shadows if position differs significantly from the previous
	if (acos(previous * SunDirection) > (PIf * SHADOW_UPDATE_ANGLE / 180.0f))
		ComputeShadows();
}

void CStructureShadowsOSG::ComputeShadows()
{
	// We must have a valid shadow sphere in order to do anything
	if (m_ShadowSphere.radius < 0)
		return;

	const osg::Viewport *pCurrentMainViewport = m_pSceneView->getViewport();

	// Constrain the texture size to the current viewport
	// this may be one frame behind
	// I may not need to do this when we are using a FBO or a pBuffer
	if (m_bUsingLiveFrameBuffer)
	{
		int iNewResolution = m_iTargetResolution;
		if (iNewResolution > pCurrentMainViewport->width())
			iNewResolution = int( pCurrentMainViewport->width() );
		if (iNewResolution > pCurrentMainViewport->height())
			iNewResolution = int( pCurrentMainViewport->height() );
		int iTemp = 1;
		while ((iTemp = iTemp << 1) <= iNewResolution);
		iNewResolution  = iTemp / 2;
		if (iNewResolution != m_iCurrentResolution)
		{
			m_iCurrentResolution = iNewResolution;
			m_pTexture->setTextureSize(m_iCurrentResolution, m_iCurrentResolution);
			m_pTexture->dirtyTextureObject();
			// Probably should centre this on the original viewport
			m_pCameraNode->setViewport(0, 0, m_iCurrentResolution, m_iCurrentResolution);
		}
	}

	// Recompute the shadow texture
#if 0
	osg::BoundingSphere ShadowerBounds = m_pCameraNode->getChild(0)->getBound();
#else
	// Use the shadowsphere that the caller wants, it will be where they
	//  want the shadows to be, generally around the camera.
	osg::BoundingSphere ShadowerBounds;
	v2s(m_ShadowSphere, ShadowerBounds);
#endif
	osg::BoundingSphere ShadowedBounds;
	if (m_bDepthShadow)
		ShadowedBounds = ShadowerBounds;
	else
		ShadowedBounds = m_pShadowed->getBound();
#if VTDEBUG
	VTLOG("CStructureShadowsOSG::ComputeShadows - Shadower(centre %f %f %f radius %f) Shadowed(centre %f %f %f radius %f)\n",
						ShadowerBounds.center().x(), ShadowerBounds.center().y(), ShadowerBounds.center().z(),
						ShadowerBounds.radius(),
						ShadowedBounds.center().x(), ShadowedBounds.center().x(), ShadowedBounds.center().x(),
						ShadowedBounds.radius());
#endif
	// Set camera position
	osg::Vec3 SunPosition = m_SunDirection;
	if (m_bDepthShadow)
	{
		osg::BoundingSphere Bounds = ShadowerBounds;
		Bounds.expandBy(ShadowedBounds);
		SunPosition = SunPosition * (Bounds.radius() + 1);
		SunPosition = SunPosition + Bounds.center();
	}
	else
	{
		SunPosition = SunPosition * (ShadowerBounds.radius() + 1);
		SunPosition = SunPosition + ShadowerBounds.center();
	}
	float CentreDistanceShadower = (SunPosition - ShadowerBounds.center()).length();
	float CentreDistanceShadowed = (SunPosition - ShadowedBounds.center()).length();
#if VTDEBUG
	VTLOG("CStructureShadowsOSG::ComputeShadows - CentreDistanceShadower %f CentreDistanceShadowed %f\n",
						CentreDistanceShadower, CentreDistanceShadowed);
#endif
	// Set up near and far planes of projection
	float CameraNearPlane;
	float CameraFarPlane;

	if (m_bDepthShadow)
	{
		// Set ortho near to front of shadower
		CameraNearPlane = CentreDistanceShadower - ShadowerBounds.radius();
		// Set ortho far to rear of shadower
		CameraFarPlane  = CentreDistanceShadower + ShadowerBounds.radius();
	}
	else
	{
		// Set ortho near to front of shadower
		CameraNearPlane = CentreDistanceShadower - ShadowerBounds.radius();
		// Set ortho far to rear of shadowed
		CameraFarPlane  = CentreDistanceShadowed + ShadowedBounds.radius();
	}
#if VTDEBUG
	VTLOG("CStructureShadowsOSG::ComputeShadows - CameraNearPlane %f CameraFarPlane %f\n",
						CameraNearPlane, CameraFarPlane);
#endif
	// Assert a minimum near far ratio
	float MinNearFarRatio = 0.001f;
	if (CameraNearPlane  < CameraFarPlane * MinNearFarRatio)
		CameraNearPlane = CameraFarPlane * MinNearFarRatio;
#if VTDEBUG
	VTLOG("CStructureShadowsOSG::ComputeShadows - CameraNearPlane %f CameraFarPlane %f\n",
						CameraNearPlane, CameraFarPlane);
#endif
	// Set up sides of projection
	float OrthoTop = ShadowerBounds.radius();
	float OrthoRight = OrthoTop; // Square projection canvas

	m_pCameraNode->setReferenceFrame(osg::CameraNode::ABSOLUTE_RF);
	m_pCameraNode->setProjectionMatrixAsOrtho(-OrthoRight, OrthoRight, -OrthoTop, OrthoTop, CameraNearPlane, CameraFarPlane);
	m_pCameraNode->setViewMatrixAsLookAt(SunPosition, ShadowerBounds.center(), osg::Vec3(0.0f, 0.1f, 0.0f));

	//Set up texgen - I hope that the camera View matrix does not contain any modelling transforms
	osg::Matrix ViewProjectionTranslate = m_pCameraNode->getViewMatrix() *
											m_pCameraNode->getProjectionMatrix() *
											osg::Matrix::translate(1.0f,1.0f,1.0f) *
											osg::Matrix::scale(0.5f,0.5f,0.5f);
	m_pTerrainTexGenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
	m_pTerrainTexGenNode->getTexGen()->setPlanesFromMatrix(ViewProjectionTranslate);

	vtLodGrid *pGrid = NULL;
	if (m_bDepthShadow)
	{
		osg::Group *pGroup = dynamic_cast<osg::Group*>(m_pCameraNode->getChild(0));
		if (NULL != pGroup)
			pGrid = dynamic_cast<vtLodGrid*>(pGroup->getChild(0)->getUserData());
	}
	else
		pGrid = dynamic_cast<vtLodGrid*>(m_pCameraNode->getChild(0)->getUserData());
	vtPagedStructureLodGrid *pPagedGrid = dynamic_cast<vtPagedStructureLodGrid*>(pGrid);
	float fOldDistance;
	if (NULL != pGrid)
	{
		fOldDistance = pGrid->GetDistance();
		pGrid->SetDistance(1E9);

		if (pPagedGrid)
			pPagedGrid->EnableLoading(false);
	}

	// No shadows after sunset
	osg::Node::NodeMask NodeMask;
	osg::Node *pNode;
	if (m_SunDirection.y() < 0.0f)
	{
		pNode = m_pCameraNode->getChild(0);
		NodeMask = pNode->getNodeMask();
		pNode->setNodeMask(0);
	}
	std::vector<osg::Node::NodeMask> Nodemasks;
	std::list<osg::Node*>::iterator iTr1;
	std::vector<osg::Node::NodeMask>::iterator iTr2;
	for (iTr1 = m_ExcludeFromShadower.begin(); iTr1 != m_ExcludeFromShadower.end(); iTr1++)
	{
		Nodemasks.push_back((*iTr1)->getNodeMask());
		(*iTr1)->setNodeMask(0);
	}
	m_pSceneView->getCamera()->addChild(m_pCameraNode.get());

	// Kill the traversal of the main scene
	osg::Node::NodeMask MainSceneNodemask = m_pSceneView->getSceneData()->getNodeMask();
	m_pSceneView->getSceneData()->setNodeMask(0);

	m_pSceneView->cull();
	m_pSceneView->draw();

	// Now restore everything
	m_pSceneView->getSceneData()->setNodeMask(MainSceneNodemask);

	m_pSceneView->getCamera()->removeChild(m_pCameraNode.get());
	for (iTr1 = m_ExcludeFromShadower.begin(), iTr2 = Nodemasks.begin(); iTr1 != m_ExcludeFromShadower.end(); iTr1++, iTr2++)
		(*iTr1)->setNodeMask(*iTr2);
	if (pPagedGrid)
		pPagedGrid->EnableLoading(true);
	if (NULL != pGrid)
		pGrid->SetDistance(fOldDistance);
	if (m_SunDirection.y() < 0.0f)
		pNode->setNodeMask(NodeMask);
}

void CStructureShadowsOSG::SetShadowDarkness(float fDarkness)
{
	m_fShadowDarkness = fDarkness;
	if (m_pMaterial.valid())
		m_pMaterial->setEmission(FAB, osg::Vec4(1.0f - m_fShadowDarkness, 1.0f - m_fShadowDarkness, 1.0f - m_fShadowDarkness, 1.0f));
	if (m_pAmbientBias.valid())
		m_pAmbientBias->set(osg::Vec2(0.0f, m_fShadowDarkness));
	if (m_bDepthShadow)
		m_pTexture->setShadowAmbient(1.0f - m_fShadowDarkness);

	// Force re-render of the shadows for the new shadow darkness
	ComputeShadows();
}

void CStructureShadowsOSG::SetShadowSphere(const FSphere &ShadowSphere, bool bForceRedraw)
{
	// If changed
	if (ShadowSphere != m_ShadowSphere || bForceRedraw)
	{
		m_ShadowSphere = ShadowSphere;
		// Force re-render of the shadows for the new shadow darkness
		ComputeShadows();
	}
}

void CStructureShadowsOSG::SetPolygonOffset(float fFactor, float fUnits)
{
	m_fPolygonOffsetFactor = 1.5f;
	m_fPolygonOffsetUnits = 1.0f;
	m_pPolygonOffset->setFactor(m_fPolygonOffsetFactor);
	m_pPolygonOffset->setUnits(m_fPolygonOffsetUnits);
}


void CStructureShadowsOSG::ExcludeFromShadower(osg::Node *pNode, bool bExclude)
{
	std::list<osg::Node*>::iterator _Last = m_ExcludeFromShadower.end(),
		_First = m_ExcludeFromShadower.begin();

	for (; _First != _Last; ++_First)
		if (*_First == pNode)
		{
			if (!bExclude)
				m_ExcludeFromShadower.erase(_First);
			// else nothing to do, already in list
			return;
		}
	// If it got here, it wasn't in the list
	if (bExclude)
		m_ExcludeFromShadower.push_back(pNode);
}

bool CStructureShadowsOSG::IsExcludedFromShadower(osg::Node *pNode)
{
	std::list<osg::Node*>::iterator _Last = m_ExcludeFromShadower.end(),
		_First = m_ExcludeFromShadower.begin();

	for (; _First != _Last; ++_First)
		if (*_First == pNode)
			return true;
	return false;
}

#endif // OLD_OSG_SHADOWS
