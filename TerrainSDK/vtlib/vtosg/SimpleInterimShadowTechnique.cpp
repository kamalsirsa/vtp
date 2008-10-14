//
// SimpleInterimShadowTechnique.cpp
//
// Encapsulate behavior for OSG scene graph nodes.
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SimpleInterimShadowTechnique.h"

#include <osgShadow/ShadowTexture>
#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>
#include <sstream>

using namespace osgShadow;

CSimpleInterimShadowTechnique::CSimpleInterimShadowTechnique():
    m_ShadowTextureUnit(1),
	m_ShadowTexureResolution(1024),
	m_PolygonOffsetFactor(-1.0f),
	m_PolygonOffsetUnits(-1.0f),
	m_ShadowDarkness(1.0f)
{
	m_MainSceneTextureUnits[0] = GL_MODULATE;
}

CSimpleInterimShadowTechnique::CSimpleInterimShadowTechnique(const CSimpleInterimShadowTechnique& copy, const osg::CopyOp& copyop):
    ShadowTechnique(copy,copyop),
    m_ShadowTextureUnit(copy.m_ShadowTextureUnit)
{
}

void CSimpleInterimShadowTechnique::SetShadowTextureUnit(const unsigned int Unit)
{
    m_ShadowTextureUnit = Unit;
}

void CSimpleInterimShadowTechnique::SetShadowDarkness(const float Darkness)
{
	m_ShadowDarkness = Darkness;
	osg::Uniform *pUniform;
	if (!_dirty)
	{
		if (m_pTexture.valid() && osg::Texture::getExtensions(0, true)->isShadowAmbientSupported())
		{
			m_pTexture->setShadowAmbient(1.0 - m_ShadowDarkness);
			osg::TexEnv *pTexEnv = new osg::TexEnv;
			m_pStateset->setTextureAttributeAndModes(m_ShadowTextureUnit, pTexEnv, osg::StateAttribute::ON);
		}
		else if (NULL != (pUniform = m_pStateset->getUniform("SimpleInterimShadow_shadowDarkness")))
			pUniform->set(m_ShadowDarkness);
	}
}

void CSimpleInterimShadowTechnique::AddMainSceneTextureUnit(const unsigned int Unit, const unsigned int Mode)
{
	m_MainSceneTextureUnits[Unit] = Mode;
}

void CSimpleInterimShadowTechnique::RemoveMainSceneTextureUnit(const unsigned int Unit)
{
	m_MainSceneTextureUnits.erase(Unit);
}

void CSimpleInterimShadowTechnique::init()
{
    if (!_shadowedScene) return;

    m_pTexture = new osg::Texture2D;
    m_pTexture->setTextureSize(m_ShadowTexureResolution, m_ShadowTexureResolution);
    m_pTexture->setInternalFormat(GL_DEPTH_COMPONENT);
    m_pTexture->setShadowComparison(true);
    m_pTexture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
	if ((m_ShadowDarkness != 1.0f) && osg::Texture::getExtensions(0, true)->isShadowAmbientSupported())
		m_pTexture->setShadowAmbient(1.0 - m_ShadowDarkness);
    m_pTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    m_pTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    m_pTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    m_pTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    m_pTexture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

    // set up the render to texture camera.
    {
        // create the camera
		m_pCamera = new osg::Camera;
		m_pCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);
		m_pCamera->setCullCallback(new CameraCullCallback(this));
		m_pCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
		m_pCamera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		m_pCamera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
		m_pCamera->setViewport(0, 0, m_ShadowTexureResolution, m_ShadowTexureResolution);
        m_pCamera->setRenderOrder(osg::Camera::PRE_RENDER);
        m_pCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        m_pCamera->attach(osg::Camera::DEPTH_BUFFER, m_pTexture.get());

		// Create the camera's stateset
		osg::StateSet* stateset = m_pCamera->getOrCreateStateSet();
		osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;
		cull_face->setMode(osg::CullFace::FRONT);
		stateset->setAttribute(cull_face.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
		polygon_offset->setFactor(m_PolygonOffsetFactor);
		polygon_offset->setUnits(m_PolygonOffsetUnits);
		stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        
    }
 
	// Set up the shadowed scene state
    {
        m_pStateset = new osg::StateSet;        
        m_pStateset->setTextureAttributeAndModes(m_ShadowTextureUnit, m_pTexture.get(),osg::StateAttribute::ON);
        m_pStateset->setTextureMode(m_ShadowTextureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        m_pStateset->setTextureMode(m_ShadowTextureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        m_pStateset->setTextureMode(m_ShadowTextureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        m_pStateset->setTextureMode(m_ShadowTextureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);
		if (m_ShadowDarkness != 1.0f)
		{
			osg::TexEnv *pTexEnv = new osg::TexEnv;
			m_pStateset->setTextureAttributeAndModes(m_ShadowTextureUnit, pTexEnv, osg::StateAttribute::ON);
		}

 
		if ((m_ShadowDarkness != 1.0f) && !osg::Texture::getExtensions(0, true)->isShadowAmbientSupported())
		{
			// Set up dummy texture
			osg::Image* pImage = new osg::Image;
			int noPixels = 1;
			pImage->allocateImage(noPixels, 1, 1, GL_RGBA,GL_FLOAT);
			pImage->setInternalTextureFormat(GL_RGBA);
			osg::Vec4* dataPtr = (osg::Vec4*)pImage->data();
			osg::Vec4 color(1, 1, 1, 1);
			*dataPtr = color;
			osg::Texture2D* pFakeTex = new osg::Texture2D;
			pFakeTex->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_EDGE);
			pFakeTex->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_EDGE);
			pFakeTex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
			pFakeTex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
			pFakeTex->setImage(pImage);

			osg::ref_ptr<osg::IntArray> UnitArray = new osg::IntArray;
			for (std::map<unsigned int, unsigned int>::iterator iTr = m_MainSceneTextureUnits.begin(); iTr != m_MainSceneTextureUnits.end(); iTr++)
			{
				unsigned int Unit = (*iTr).first;
				UnitArray->push_back(Unit);
				m_pStateset->setTextureAttribute(Unit, pFakeTex, osg::StateAttribute::ON);
				m_pStateset->setTextureMode(Unit,GL_TEXTURE_1D, osg::StateAttribute::OFF);
				m_pStateset->setTextureMode(Unit,GL_TEXTURE_2D, osg::StateAttribute::ON);
				m_pStateset->setTextureMode(Unit,GL_TEXTURE_3D, osg::StateAttribute::OFF);
			}
			osg::Program *pProgram = new osg::Program;
			osg::Shader *pFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, GenerateFragmentShaderSource());
			pProgram->addShader(pFragmentShader);
			m_pStateset->setAttribute(pProgram);
			osg::Uniform *pUniform = new osg::Uniform(osg::Uniform::INT, "SimpleInterimShadow_baseTextures", m_MainSceneTextureUnits.size());
			pUniform->setArray(UnitArray.get());
			m_pStateset->addUniform(pUniform);
			m_pStateset->addUniform(new osg::Uniform("SimpleInterimShadow_shadowTexture", (int)m_ShadowTextureUnit));
			m_pStateset->addUniform(new osg::Uniform("SimpleInterimShadow_shadowDarkness", m_ShadowDarkness));
		}

        m_pTexgen = new osg::TexGen;
    }
    
    _dirty = false;
}


void CSimpleInterimShadowTechnique::update(osg::NodeVisitor& nv)
{
    _shadowedScene->osg::Group::traverse(nv);
}

void CSimpleInterimShadowTechnique::cull(osgUtil::CullVisitor& cv)
{
    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = cv.getTraversalMask();

    osgUtil::RenderStage* orig_rs = cv.getRenderStage();

    // Do traversal of shadow receiving scene.
	// Note that we do not use the ReceivesShadowMask as it could filter out
	// unshadowed nodes that should be rendered
	// We could do this in a more controlled way by not forcing the shadow state
	// at the top of the cull as we do at the moment, but by using a cull callback
	// to conditionally apply it to each node that needs it.
    {
        cv.pushStateSet(m_pStateset.get());
    
        _shadowedScene->osg::Group::traverse(cv);
        
        cv.popStateSet();
    }
    
    // need to compute view frustum for RTT camera.
    // 1) get the light position
    // 2) get the center and extents of the view frustum

    const osg::Light* selectLight = 0;
    osg::Vec4 lightpos;

    osgUtil::PositionalStateContainer::AttrMatrixList& aml = orig_rs->getPositionalStateContainer()->getAttrMatrixList();
    for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        itr != aml.end();
        ++itr)
    {
        const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light)
        {
            osg::RefMatrix* matrix = itr->second.get();
            if (matrix) lightpos = light->getPosition() * (*matrix);
            else lightpos = light->getPosition();

            selectLight = light;
			// We should only have one light (sun) in the scene
			break;
        }
    }
    
    osg::Matrix eyeToWorld;
    eyeToWorld.invert(*cv.getModelViewMatrix());
    
    lightpos = lightpos * eyeToWorld;

    if (selectLight)
    {

        // get the bounds of the model.    
        osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
        cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());
        
        _shadowedScene->osg::Group::traverse(cbbv);
        
        osg::BoundingBox bb = cbbv.getBoundingBox();

		// make an orthographic projection
		osg::Vec3 lightDir(lightpos.x(), lightpos.y(), lightpos.z());
		lightDir.normalize();

		// set the position far away along the light direction
		osg::Vec3 position = bb.center() + lightDir * bb.radius() * 2.0;

		float centerDistance = (position-bb.center()).length();

		float znear = centerDistance-bb.radius();
		float zfar  = centerDistance+bb.radius();
		float zNearRatio = 0.001f;
		if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

		float top   = bb.radius();
		float right = top;

		m_pCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
		m_pCamera->setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);
		m_pCamera->setViewMatrixAsLookAt(position, bb.center(), osg::Vec3(0.0f,1.0f,0.0f));


		// compute the matrix which takes a vertex from local coords into tex coords
		// will use this later to specify osg::TexGen..
		osg::Matrix VPT = m_pCamera->getViewMatrix() * 
						   m_pCamera->getProjectionMatrix() *
						   osg::Matrix::translate(1.0,1.0,1.0) *
						   osg::Matrix::scale(0.5f,0.5f,0.5f);
		                   
		m_pTexgen->setMode(osg::TexGen::EYE_LINEAR);
		m_pTexgen->setPlanesFromMatrix(VPT);


        cv.setTraversalMask( traversalMask & 
                             getShadowedScene()->getCastsShadowTraversalMask() );

        // do RTT camera traversal
        m_pCamera->accept(cv);

        orig_rs->getPositionalStateContainer()->addPositionedTextureAttribute(m_ShadowTextureUnit, cv.getModelViewMatrix(), m_pTexgen.get());
    }

	// reapply the original traversal mask
	cv.setTraversalMask( traversalMask );
}

void CSimpleInterimShadowTechnique::cleanSceneGraph()
{
}

std::string CSimpleInterimShadowTechnique::GenerateFragmentShaderSource()
{
	std::stringstream ShaderSource;
	ShaderSource
		<< "uniform sampler2D SimpleInterimShadow_baseTextures[" << m_MainSceneTextureUnits.size() << "];" << std::endl
		<< "uniform sampler2DShadow SimpleInterimShadow_shadowTexture;" << std::endl
		<< "uniform float SimpleInterimShadow_shadowDarkness;" << std::endl
		<< std::endl
		<< "void main(void)" << std::endl
		<< "{" << std::endl
		<< "    vec4 Colour = gl_Color;" << std::endl
		<< "    vec4 TexColour;" << std::endl;
	for (std::map<unsigned int, unsigned int>::iterator iTr = m_MainSceneTextureUnits.begin(); iTr != m_MainSceneTextureUnits.end(); iTr++)
	{
		int Unit = (*iTr).first;
		int Mode = (*iTr).second;
		switch (Mode)
		{
		case GL_ADD:
			ShaderSource
				<< "    TexColour = texture2D(SimpleInterimShadow_baseTextures["
				<< Unit << "], gl_TexCoord["
				<< Unit << "].xy);" << std::endl
				<< "    Colour.rgb = Colour.rgb + TexColour.rgb;" << std::endl
				<< "    Colour.a = Colour.a *  TexColour.a;" << std::endl;
			break;
		case GL_BLEND:
			ShaderSource
				<< "    TexColour = texture2D(SimpleInterimShadow_baseTextures["
				<< Unit << "], gl_TexCoord["
				<< Unit << "].xy);" << std::endl
				<< "    Colour.rgb = Colour.rgb * (1 - TexColour.rgb) + TexColour.rgb;" << std::endl
				<< "    Colour.a = Colour.a *  TexColour.a;" << std::endl;
			break;
		case GL_REPLACE:
			ShaderSource
				<< "    Colour = texture2D(SimpleInterimShadow_baseTextures["
				<< Unit << "], gl_TexCoord["
				<< Unit << "].xy);" << std::endl;
			break;
		case GL_MODULATE:
			ShaderSource
				<< "    Colour = Colour * texture2D(SimpleInterimShadow_baseTextures["
				<< Unit << "], gl_TexCoord["
				<< Unit << "].xy);" << std::endl;
			break;
		case GL_DECAL:
			ShaderSource
				<< "    TexColour = texture2D(SimpleInterimShadow_baseTextures["
				<< Unit << "], gl_TexCoord["
				<< Unit << "].xy);" << std::endl
				<< "    Colour.rgb = Colour.rgb * (1 - TexColour.a) + TexColour.rgb * TexColour.a;" << std::endl;
			break;
		}
	}
	ShaderSource
		<< "    gl_FragColor = Colour * (1.0 - SimpleInterimShadow_shadowDarkness + shadow2DProj(SimpleInterimShadow_shadowTexture, gl_TexCoord["
		<< m_ShadowTextureUnit << "]) * SimpleInterimShadow_shadowDarkness);" << std::endl
		<< "}" << std::endl;
	return ShaderSource.str();
}

class DrawableDrawWithDepthShadowComparisonOffCallback: public osg::Drawable::DrawCallback
{
public:
// 
	DrawableDrawWithDepthShadowComparisonOffCallback(osg::Texture2D * texture, unsigned stage = 0) 
														: _texture(texture), _stage(stage)
{
}

virtual void drawImplementation(osg::RenderInfo & ri,const osg::Drawable* drawable ) const
{
	if(_texture.valid())
	{
		// make sure proper texture is currently applied
		ri.getState()->applyTextureAttribute( _stage, _texture.get() );

		// Turn off depth comparison mode
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
		}

		drawable->drawImplementation(ri);

		if( _texture.valid() )
		{
			// Turn it back on
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, 
			GL_COMPARE_R_TO_TEXTURE_ARB );
		}
	}

	unsigned _stage;
	osg::ref_ptr< osg::Texture2D > _texture;
};

////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Camera> CSimpleInterimShadowTechnique::makeDebugHUD()
{
	std::stringstream ShaderSource;
	ShaderSource
		<< "uniform sampler2D osgShadow_shadowTexture;" << std::endl
		<< std::endl
		<< "void main(void)" << std::endl
		<< "{" << std::endl
		<< "   vec4 texResult = texture2D(osgShadow_shadowTexture, gl_TexCoord[" << m_ShadowTextureUnit << "].st );" << std::endl
		<< "   float value = texResult.r;" << std::endl
		<< "   gl_FragColor = vec4( value, value, value, 0.8 );" << std::endl
		<< "}" << std::endl;

	// Make sure we attach initialized texture to HUD
	if(!m_pTexture.valid())
		init();

	osg::ref_ptr<osg::Camera> pCamera = new osg::Camera;

	osg::Vec2 size(1280, 1024);
	// set the projection matrix
	pCamera->setProjectionMatrix(osg::Matrix::ortho2D(0, size.x(), 0, size.y()));

	// set the view matrix    
	pCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	pCamera->setViewMatrix(osg::Matrix::identity());

	// only clear the depth buffer
	pCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
	pCamera->setClearColor(osg::Vec4(0.2f, 0.3f, 0.5f, 0.2f));

	// draw subgraph after main camera view.
	pCamera->setRenderOrder(osg::Camera::POST_RENDER);

	// we don't want the camera to grab event focus from the viewers main camera(s).
	pCamera->setAllowEventFocus(false);

	osg::Geode* pGeode = new osg::Geode;

	osg::Vec3 position(10.0f,size.y()-100.0f,0.0f);
	osg::Vec3 delta(0.0f,-120.0f,0.0f); 
	float length = 300.0f;

	osg::Vec3 widthVec(length, 0.0f, 0.0f);
	osg::Vec3 depthVec(0.0f, length, 0.0f);
	osg::Vec3 centerBase(10.0f + length/2, size.y() - length/2, 0.0f);
	centerBase += delta;

	osg::Geometry *pGeometry = osg::createTexturedQuadGeometry(centerBase - widthVec * 0.5f - depthVec * 0.5f, widthVec, depthVec );

	pGeode->addDrawable(pGeometry);

	pGeometry->setDrawCallback(new DrawableDrawWithDepthShadowComparisonOffCallback(m_pTexture.get()));

	osg::StateSet* pStateset = pGeode->getOrCreateStateSet();

	pStateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	pStateset->setMode(GL_BLEND,osg::StateAttribute::ON);
	pStateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	//shader for correct display
	osg::ref_ptr<osg::Program> pProgram = new osg::Program;
	pStateset->setAttribute(pProgram.get());
	osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, ShaderSource.str());
	pProgram->addShader(fragment_shader);
	pCamera->addChild(pGeode);

	return pCamera;
}

