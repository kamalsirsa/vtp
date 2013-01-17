//
// MultiTexture.cpp
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "MultiTexture.h"


void vtMultiTexture::Create(osg::Node *pSurfaceNode, vtImage *pImage, const vtHeightField3d *pSurface,
		const DRECT &ImageExtent, int iTextureUnit, int iTextureMode)
{
	m_pNode = pSurfaceNode;
	m_iTextureUnit = iTextureUnit;
#if VTLISPSM
	m_iMode = iTextureMode;
#endif

	// Calculate the mapping of texture coordinates
	DRECT SurfaceEarthExtents = pSurface->GetEarthExtents();
	DPoint2 scale;
	FPoint2 offset;

	FRECT worldExtents;
	pSurface->m_Conversion.ConvertFromEarth(ImageExtent, worldExtents);

	// Map input values (0-terrain size in world coords) to 0-1
	scale.Set(1.0/worldExtents.Width(), 1.0/worldExtents.Height());

	// and offset it to place it at the right place
	offset.x = (float) ((ImageExtent.left - SurfaceEarthExtents.left) / ImageExtent.Width());
	offset.y = (float) ((ImageExtent.bottom - SurfaceEarthExtents.bottom) / ImageExtent.Height());

	// apply it to the node that is above the terrain surface

	// Get a stateset to work with
	osg::ref_ptr<osg::StateSet> pStateSet = pSurfaceNode->getOrCreateStateSet();

	// Setup and enable the texture
	m_pTexture = new osg::Texture2D(pImage);
	m_pTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
	m_pTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	m_pTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
	m_pTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
	pStateSet->setTextureAttributeAndModes(iTextureUnit, m_pTexture.get(), osg::StateAttribute::ON);

	// Set up the texgen
	osg::ref_ptr<osg::TexGen> pTexgen = new osg::TexGen;
	pTexgen->setMode(osg::TexGen::EYE_LINEAR);
	pTexgen->setPlane(osg::TexGen::S, osg::Vec4(scale.x, 0.0f, 0.0f, -offset.x));
	pTexgen->setPlane(osg::TexGen::T, osg::Vec4(0.0f, 0.0f, scale.y, -offset.y));
	pStateSet->setTextureAttributeAndModes(iTextureUnit, pTexgen.get(), osg::StateAttribute::ON);
	pStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_S,  osg::StateAttribute::ON);
	pStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_T,  osg::StateAttribute::ON);

	// Set up the texenv
	osg::TexEnv::Mode mode;
	if (iTextureMode == GL_ADD) mode = osg::TexEnv::ADD;
	if (iTextureMode == GL_BLEND) mode = osg::TexEnv::BLEND;
	if (iTextureMode == GL_REPLACE) mode = osg::TexEnv::REPLACE;
	if (iTextureMode == GL_MODULATE) mode = osg::TexEnv::MODULATE;
	if (iTextureMode == GL_DECAL) mode = osg::TexEnv::DECAL;
	osg::ref_ptr<osg::TexEnv> pTexEnv = new osg::TexEnv(mode);
	pStateSet->setTextureAttributeAndModes(iTextureUnit, pTexEnv.get(), osg::StateAttribute::ON);

	// If texture mode is DECAL and internal texture format does not have an alpha channel then
	// force the format to be converted on texture binding
	if ((GL_DECAL == iTextureMode) &&
		(pImage->getInternalTextureFormat() != GL_RGBA))
	{
		// Force the internal format to RGBA
		pImage->setInternalTextureFormat(GL_RGBA);
	}
}

void vtMultiTexture::Enable(bool bEnable)
{
	osg::ref_ptr<osg::StateSet> pStateSet = m_pNode->getOrCreateStateSet();
	if (bEnable)
		pStateSet->setTextureAttributeAndModes(m_iTextureUnit, m_pTexture.get(), osg::StateAttribute::ON);
	else
	{
		osg::StateAttribute *attr = pStateSet->getTextureAttribute(m_iTextureUnit, osg::StateAttribute::TEXTURE);
		if (attr != NULL)
			pStateSet->removeTextureAttribute(m_iTextureUnit, attr);
	}
}

bool vtMultiTexture::IsEnabled()
{
	osg::ref_ptr<osg::StateSet> pStateSet = m_pNode->getOrCreateStateSet();
	osg::StateAttribute *attr = pStateSet->getTextureAttribute(m_iTextureUnit, osg::StateAttribute::TEXTURE);
	return (attr != NULL);
}

