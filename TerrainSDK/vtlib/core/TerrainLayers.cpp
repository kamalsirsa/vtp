//
// TerrainLayers.cpp
//
// Copyright (c) 2006-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TerrainLayers.h"

////////////////////////////////////////////////////////////////////////////
// Layers

void LayerSet::Remove(vtLayer *lay)
{
	for (size_t i = 0; i < size(); i++)
	{
		if (lay == at(i))
		{
			erase(begin()+i);
			return;
		}
	}
}

vtLayer *LayerSet::FindByName(const vtString &name)
{
	for (size_t i = 0; i < size(); i++)
	{
		if (at(i)->GetLayerName() == name)
			return at(i);
	}
	return NULL;
}

vtStructureLayer *LayerSet::FindStructureFromNode(osg::Node *pNode, int &iOffset)
{
	iOffset = -1;
	size_t iNumLayers = size();
	bool bFound = false;

	for (size_t i = 0; i < iNumLayers && !bFound; i++)
	{
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer *>(at(i).get());
		if (!slay)
			continue;
		int iNumStructures = slay->size();
		for (int j = 0; (j < iNumStructures) && !bFound; j++)
		{
			vtStructure3d *pStructure3d = slay->GetStructure3d(j);
			if (FindAncestor(pNode, pStructure3d->GetContainer()) ||
				FindAncestor(pNode, pStructure3d->GetContained()) ||
				FindAncestor(pNode, pStructure3d->GetGeom()))
			{
				iOffset = j;
				return slay;
			}
		}
	}
	return NULL;
}

vtImageLayer::vtImageLayer() : vtLayer(LT_IMAGE)
{
	m_pImage = new vtImageGeo;
	m_pMultiTexture = NULL;
}

vtImageLayer::~vtImageLayer()
{
	delete m_pMultiTexture;
}

void vtImageLayer::SetVisible(bool vis)
{
	if (m_pMultiTexture)
		EnableMultiTexture(m_pMultiTexture->m_pNode, m_pMultiTexture, vis);
}
bool vtImageLayer::GetVisible()
{
	if (m_pMultiTexture)
		return MultiTextureIsEnabled(m_pMultiTexture->m_pNode, m_pMultiTexture);
	return false;
}
