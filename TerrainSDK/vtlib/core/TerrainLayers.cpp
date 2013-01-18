//
// TerrainLayers.cpp
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/vtosg/MultiTexture.h"

#include "TParams.h"
#include "TerrainLayers.h"
#include "vtTin3d.h"


////////////////////////////////////////////////////////////////////////////
// Structures

vtStructureLayer::vtStructureLayer() : vtLayer(LT_STRUCTURE)
{
	m_Props.SetValueString("Type", TERR_LTYPE_STRUCTURE);
}


////////////////////////////////////////////////////////////////////////////
// Vegetation

vtVegLayer::vtVegLayer() : vtLayer(LT_VEG)
{
	m_Props.SetValueString("Type", TERR_LTYPE_VEGETATION);
}


////////////////////////////////////////////////////////////////////////////
// Imagery

vtImageLayer::vtImageLayer() : vtLayer(LT_IMAGE)
{
	m_Props.SetValueString("Type", TERR_LTYPE_IMAGE);

	m_pImage = new vtImageGeo;
	m_pMultiTexture = NULL;
}

vtImageLayer::~vtImageLayer()
{
	m_pMultiTexture = NULL;
}

void vtImageLayer::SetVisible(bool vis)
{
	if (m_pMultiTexture)
		m_pMultiTexture->Enable(vis);
	vtLayerBase::SetVisible(vis);
}


////////////////////////////////////////////////////////////////////////////
// Elevation Surfaces

vtElevLayer::vtElevLayer() : vtLayer(LT_ELEVATION)
{
	m_Props.SetValueString("Type", TERR_LTYPE_ELEVATION);
}

void vtElevLayer::SetLayerName(const vtString &fname)
{
	m_Props.SetValueString("Filename", fname);
}

vtString vtElevLayer::GetLayerName()
{
	return m_Props.GetValueString("Filename");
}

void vtElevLayer::SetVisible(bool vis)
{
	if (m_pTin.get() && m_pTin->GetGeometry())
	{
		m_pTin->GetGeometry()->SetEnabled(vis);
	}
	vtLayerBase::SetVisible(vis);
}

bool vtElevLayer::Load(const vtString &path, bool progress_callback(int))
{
	m_pTin = new vtTin3d;
	if (!m_pTin->Read(path))
		return false;

	vtGeode *geode = m_pTin->CreateGeometry(false);
	geode->SetCastShadow(false);
//	m_pTerrainGroup->addChild(geode);

	return true;
}


////////////////////////////////////////////////////////////////////////////
// LayerSet

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

