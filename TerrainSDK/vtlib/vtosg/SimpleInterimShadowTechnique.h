//
// SimpleInterimShadowTechnique.h
//
// It's simple because it is.
// It's interim because it should be replaced with something better.
// It's a shadow technique.
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_SIMPLEINTERIMSHADOWTECHNIQUEH
#define VTOSG_SIMPLEINTERIMSHADOWTECHNIQUEH

#include <osg/Camera>
#include <osg/Material>
#include <osgShadow/ShadowTechnique>

/** CSimpleInterimShadowTechnique provides an implementation of a depth map shadow tuned to the needs of VTP.*/
class CSimpleInterimShadowTechnique : public osgShadow::ShadowTechnique
{
public:
	CSimpleInterimShadowTechnique();
	CSimpleInterimShadowTechnique(const CSimpleInterimShadowTechnique& es, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
	META_Object(osgShadow, CSimpleInterimShadowTechnique);
	void SetShadowTextureUnit(const unsigned int Unit);
	void SetShadowDarkness(const float Darkness);
	float GetShadowDarkness() { return m_ShadowDarkness; }
	void AddMainSceneTextureUnit(const unsigned int Unit);
	void RemoveMainSceneTextureUnit(const unsigned int Unit);

	virtual void init();
	virtual void update(osg::NodeVisitor& nv);
	virtual void cull(osgUtil::CullVisitor& cv);
	virtual void cleanSceneGraph();

	osg::ref_ptr<osg::Camera> makeDebugHUD();

protected :

	virtual ~CSimpleInterimShadowTechnique() {}

	std::string GenerateFragmentShaderSource();

	osg::ref_ptr<osg::Camera>		m_pCamera;
	osg::ref_ptr<osg::TexGen>		m_pTexgen;
	osg::ref_ptr<osg::Texture2D>	m_pTexture;
	osg::ref_ptr<osg::StateSet>		m_pStateset;
	osg::ref_ptr<osg::Material>		m_pMaterial;
	unsigned int					m_ShadowTextureUnit;
	unsigned int					m_ShadowTexureResolution;
	float							m_PolygonOffsetFactor;
	float							m_PolygonOffsetUnits;
	float							m_ShadowDarkness;
	std::set<const unsigned int>	m_MainSceneTextureUnits;
};
#endif
