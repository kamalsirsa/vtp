//
// TextureUnitManager.cpp
//
// Originally written by RJ.
//
// Copyright (c) 2006-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TextureUnitManager.h"

//
// RJ's warning: NONE OF THIS IS THREAD SAFE!!!
//

vtTextureUnitManager::vtTextureUnitManager(void)
{
	m_bInitialised = false;
	m_pAllocationArray = NULL;
}

vtTextureUnitManager::~vtTextureUnitManager(void)
{
	if (NULL != m_pAllocationArray)
		delete[] m_pAllocationArray;
}

void vtTextureUnitManager::Initialise()
{
	// Ensure this thread has a valid graphics context
	// The following code will create
	// 1. A OSG Traits object
	// 2. A new OSG GraphicsContext object
	// 3. A new zero sized window
	// 4. A new OpenGL rendering context
	// 5. A new OSG contextID
	// The ref_ptrs will ensure all these things will be destroyed when this routine returns
	// although OSG may spit out a warning about destroying a valid window!
	osg::ref_ptr<osg::GraphicsContext::Traits> pTraits = new osg::GraphicsContext::Traits;
	osg::ref_ptr<osg::GraphicsContext> pGraphicsContext = osg::GraphicsContext::createGraphicsContext(pTraits.get());
	pGraphicsContext->realize();
	pGraphicsContext->makeCurrent();
	osg::ref_ptr<osg::Texture::Extensions> pTextureExtensions = osg::Texture::getExtensions(pGraphicsContext->getState()->getContextID(), true);
	m_iNumTextureUnits = pTextureExtensions->numTextureUnits();
	if (m_iNumTextureUnits < 0)
	{
		// got a bogus value, probably because there is no OpenGL context yet.
		m_iNumTextureUnits = 4;
	}

	m_pAllocationArray = new bool[m_iNumTextureUnits];
	for (int i = 0; i < m_iNumTextureUnits; i++)
		m_pAllocationArray[i] = false;
	m_bInitialised = true;
}

int vtTextureUnitManager::ReserveTextureUnit(bool bHighest)
{
	int iUnit = -1;
	if (!m_bInitialised)
		Initialise();
	if (bHighest)
	{
		for (int i = m_iNumTextureUnits - 1; i >= 0; i--)
		{
			if (m_pAllocationArray[i] == false)
			{
				m_pAllocationArray[i] = true;
				iUnit = i;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_iNumTextureUnits; i++)
		{
			if (m_pAllocationArray[i] == false)
			{
				m_pAllocationArray[i] = true;
				iUnit = i;
				break;
			}
		}
	}
	return iUnit;
}

void vtTextureUnitManager::FreeTextureUnit(int iUnit)
{
	if (!m_bInitialised || (iUnit >= m_iNumTextureUnits))
		return;
	m_pAllocationArray[iUnit] = false;
}

