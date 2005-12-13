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
#include <osgUtil/RenderStage>
#ifdef ENVIRON_OCCLUDER_SAVE_SHADOW_IMAGE
#include <osgDB/WriteFile>
#endif

#if VTDEBUG

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

class MyRenderStage : public osgUtil::RenderStage
{
public:
	MyRenderStage()
	{
		m_bWriteImage = true;
		osgUtil::RenderStage::setImage(new osg::Image);
	}

	virtual void draw(osg::State& state, osgUtil::RenderLeaf*& previous)
	{
		RenderStage::draw(state, previous);
		if (m_bWriteImage)
		{
			m_bWriteImage = false;
			osgDB::writeImageFile(*osgUtil::RenderStage::getImage(), "c:\\testtexture.rgb");
			osgUtil::RenderStage::setImage(NULL);		
		}
	}
protected:
	bool m_bWriteImage;
};
#endif


class CreateProjectedShadowTextureCullCallback : public osg::NodeCallback
{
public:
	CreateProjectedShadowTextureCullCallback(osg::Node* shadower,
		int iRez = 1024, const osg::Vec3 &position = osg::Vec3(),
		const osg::Vec4 &ambientLightColor = osg::Vec4(),
		unsigned int textureUnit = 1);

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	void SetLightPosition(const osg::Vec3 &position);
	void SetInitialLightPosition(const osg::Vec3 &position) { m_position = position; m_bRecomputeShadows = true; }
	void SetAmbientLightColor(const osg::Vec4 &ambientLightColor) { m_ambientLightColor = ambientLightColor; }
	void SetShadower(osg::Node* pShadower) { m_shadower = pShadower; }
	void SetEnabled(bool enable) { m_bDrawEnabled = enable; }
	void SetShadowDarkness(float fDarkness);
	void RecomputeShadows() { m_bRecomputeShadows = true; }
	Array<osg::Node *> *shadow_ignore_nodes;

protected:
	void doPreRender(osg::Node& node, osgUtil::CullVisitor& cv);
	void DoRecomputeShadows(osg::Node& node, osgUtil::CullVisitor &cv);
	
 	bool m_bRecomputeShadows;
	bool m_bInCallback;
	bool m_bDrawEnabled;
	bool m_bDepthShadow;
	float m_fShadowDarkness;

	osg::ref_ptr<osg::Node>		m_shadower;
	osg::Vec3					m_position;
	osg::Vec4					m_ambientLightColor;
	unsigned int				m_unit;
	unsigned int				m_iRez;
	osg::ref_ptr<osg::StateSet>	m_shadowState;
	osg::ref_ptr<osg::StateSet>	m_shadowedState;
	osg::Matrix					m_VPOSTransform;
#if VTDEBUG
	osg::ref_ptr<MyTexture2D>	m_texture;
	osg::ref_ptr<MyRenderStage>	m_pRtts;
#else
	osg::ref_ptr<osgUtil::RenderStage> m_pRtts;
	osg::ref_ptr<osg::Texture2D> m_texture;
#endif
	osg::ref_ptr<osg::Material>  m_material;

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

