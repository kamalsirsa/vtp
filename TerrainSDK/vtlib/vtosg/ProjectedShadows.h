//
// ProjectedShadows.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CREATEPROJECTEDSHADOWEDSCENE_H
#define CREATEPROJECTEDSHADOWEDSCENE_H

#define ENVIRON_OCCLUDER_SAVE_SHADOW_IMAGE

#include <osg/Node>
#include <osg/Vec3>
#include <osg/TexGen>
#include <osgUtil/RenderToTextureStage>
#ifdef ENVIRON_OCCLUDER_SAVE_SHADOW_IMAGE
#include <osgDB/WriteFile>
#endif

#ifdef _DEBUG

class MyTexture2D : public osg::Texture2D
{
public:
	virtual void apply(osg::State& state) const
	{
		osg::Texture2D::apply(state);
	}
};

struct MyDrawCallback : public osg::Drawable::DrawCallback
{
	virtual void drawImplementation(osg::State& state, const osg::Drawable* drawable) const
	{
		drawable->drawImplementation(state);
	}
};

class MyRenderStage : public osgUtil::RenderToTextureStage
{
public:
	MyRenderStage()
	{
		m_bWriteImage = true;
		osgUtil::RenderToTextureStage::setImage(new osg::Image);
	}

	virtual void draw(osg::State& state, osgUtil::RenderLeaf*& previous)
	{
		RenderToTextureStage::draw(state, previous);
		if (m_bWriteImage)
		{
			m_bWriteImage = false;
			osgDB::writeImageFile(*osgUtil::RenderToTextureStage::getImage(), "c:\\testtexture.rgb");
			osgUtil::RenderToTextureStage::setImage(NULL);		
		}
	}
protected:
	bool m_bWriteImage;
};
#endif


class CreateProjectedShadowTextureCullCallback : public osg::NodeCallback
{
public:
	CreateProjectedShadowTextureCullCallback(osg::Node* shadower, int iRez = 1024, const osg::Vec3& position = osg::Vec3(), const osg::Vec4& ambientLightColor = osg::Vec4(), unsigned int textureUnit = 1):
		m_shadower(shadower),
		m_position(position),
		m_ambientLightColor(ambientLightColor),
		m_unit(textureUnit),
		m_shadowState(new osg::StateSet),
		m_shadowedState(new osg::StateSet)
	{
#ifdef _DEBUG
		m_texture = new MyTexture2D;
#else
		m_texture = new osg::Texture2D;
#endif
		m_texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		m_texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
		m_texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
		m_texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
		m_texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		m_bRecomputeShadows = false;
		m_iRez = iRez;
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	void SetLightPosition(const osg::Vec3& position);
	void SetInitialLightPosition(const osg::Vec3& position) { m_position = position; m_bRecomputeShadows = true; }
	void SetAmbientLightColor(const osg::Vec4& ambientLightColor) { m_ambientLightColor = ambientLightColor; }
	void SetShadower(osg::Node* pShadower) { m_shadower = pShadower; }
	
protected:
	void doPreRender(osg::Node& node, osgUtil::CullVisitor& cv);
	
 	bool m_bRecomputeShadows;
	bool m_bInCallback;
	osg::ref_ptr<osg::Node>      m_shadower;
	osg::Vec3                    m_position;
	osg::Vec4                    m_ambientLightColor;
	unsigned int                 m_unit;
	unsigned int				 m_iRez;
	osg::ref_ptr<osg::StateSet>  m_shadowState;
	osg::ref_ptr<osg::StateSet>  m_shadowedState;
	osg::Matrix m_VPOSTransform;
#ifdef _DEBUG
	osg::ref_ptr<MyTexture2D> m_texture;
	osg::ref_ptr<MyRenderStage> m_pRtts;
#else
	osg::ref_ptr<osgUtil::RenderToTextureStage> m_pRtts;
	osg::ref_ptr<osg::Texture2D> m_texture;
#endif

	// we need this to get round the order dependance
	// of eye linear tex gen...    
	class MyTexGen : public osg::TexGen
	{
	public:
		void setMatrix(const osg::Matrix& matrix)
		{
			m_matrix = matrix;
		}

		virtual void apply(osg::State& state) const
		{
			glPushMatrix();
			glLoadMatrix(m_matrix.ptr());
			TexGen::apply(state);
			glPopMatrix();
		}
		osg::Matrix m_matrix;
	};
};

#endif	// CREATEPROJECTEDSHADOWEDSCENE_H

