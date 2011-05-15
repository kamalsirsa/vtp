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
	for (unsigned int i = 0; i < GetSize(); i++)
	{
		if (lay == GetAt(i))
		{
			RemoveAt(i);
			return;
		}
	}
}

vtLayer *LayerSet::FindByName(const vtString &name)
{
	for (unsigned int i = 0; i < GetSize(); i++)
	{
		if (GetAt(i)->GetLayerName() == name)
			return GetAt(i);
	}
	return NULL;
}

vtStructureLayer *LayerSet::FindStructureFromNode(vtNode* pNode, int &iOffset)
{
	iOffset = -1;
	int iNumLayers = GetSize();
	bool bFound = false;

	// We might have a low-level native scenegraph node; we want the higher-level
	vtNativeNode *native = dynamic_cast<vtNativeNode *>(pNode);
	if (native)
	{
		pNode = native->FindParentVTNode();
		if (!pNode)
			return false;
	}

	for (int i = 0; i < iNumLayers && !bFound; i++)
	{
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer *>(GetAt(i));
		if (!slay)
			continue;
		int iNumStructures = slay->GetSize();
		for (int j = 0; (j < iNumStructures) && !bFound; j++)
		{
			vtStructure3d *pStructure3d = slay->GetStructure3d(j);
			if ((pNode == pStructure3d->GetContainer()) ||
				(pNode == pStructure3d->GetContained()) ||
				(pNode->GetParent() == pStructure3d->GetContained()) ||
				(pNode == pStructure3d->GetGeom()))
			{
				iOffset = j;
				return slay;
			}
		}
	}
	return NULL;
}

vtImageLayer::vtImageLayer()
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
		m_pMultiTexture->m_pNode->EnableMultiTexture(m_pMultiTexture, vis);
}
bool vtImageLayer::GetVisible()
{
	if (m_pMultiTexture)
		return m_pMultiTexture->m_pNode->MultiTextureIsEnabled(m_pMultiTexture);
	return false;
}
