//
// LightSpacePerspectiveShadowTechnique.h
//
// It's simple because it is.
// It's interim because it should be replaced with something better.
// It's a shadow technique.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if OSG_VERSION_MAJOR == 2 && OSG_VERSION_MINOR >= 7

#ifndef VTOSG_LIGHTSPACEPERSPECTIVESHADOWTECHNIQUEH
#define VTOSG_LIGHTSPACEPERSPECTIVESHADOWTECHNIQUEH

#include <osgShadow/LightSpacePerspectiveShadowMap>

class CLightSpacePerspectiveShadowTechnique : public osgShadow::LightSpacePerspectiveShadowMapVB
{    
public:
	/** Convenient typedef used in definition of ViewData struct and methods */
	typedef CLightSpacePerspectiveShadowTechnique                     ThisClass;
	/** Convenient typedef used in definition of ViewData struct and methods */
	typedef osgShadow::LightSpacePerspectiveShadowMapVB BaseClass;

	CLightSpacePerspectiveShadowTechnique();

protected:
	struct ViewData: public BaseClass::ViewData
	{
		virtual void init( ThisClass * st, osgUtil::CullVisitor * cv );           
	};

	// This macro is required if you override ViewData and ViewData::init
	// It generates virtual stub function in the Base class which 
	// calls associated ViewData::init. 
	META_ViewDependentShadowTechniqueData( ThisClass, ThisClass::ViewData )

	void InitJittering(osg::StateSet *pStateSet);

	unsigned int m_JitterTextureUnit;
};


#endif	// VTOSG_LIGHTSPACEPERSPECTIVESHADOWTECHNIQUEH

#endif