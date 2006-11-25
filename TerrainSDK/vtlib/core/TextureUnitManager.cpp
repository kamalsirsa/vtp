//
// TextureUnitManager.cpp
//
// Originally written by RJ.
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "textureunitmanager.h"

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
		delete m_pAllocationArray;
}

void vtTextureUnitManager::Initialise()
{
#if VTLIB_OSG
	osg::ref_ptr<osg::Texture::Extensions> pTextureExtensions = osg::Texture::getExtensions(0, true);
	m_iNumTextureUnits = pTextureExtensions->numTextureUnits();
#else
	// Assume four.
	m_iNumTextureUnits = 4;
#endif
	m_pAllocationArray = new bool[m_iNumTextureUnits];
	for (int i = 0; i < m_iNumTextureUnits; i++)
		m_pAllocationArray[i] = false;
	m_bInitialised = true;
}

int vtTextureUnitManager::ReserveTextureUnit()
{
	int iUnit = -1;
	if (!m_bInitialised)
		Initialise();
	for (int i = 0; i < m_iNumTextureUnits; i++)
	{
		if (m_pAllocationArray[i] == false)
		{
			m_pAllocationArray[i] = true;
			iUnit = i;
			break;
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

