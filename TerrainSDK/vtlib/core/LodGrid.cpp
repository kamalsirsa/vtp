//
// LodGrid.cpp
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/LocalConversion.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtString.h"

#include "LodGrid.h"

#define CellIndex(a,b) ((a*m_dim)+b)

/////////////////////////////////////////////////////////////////////////////
// Base LOD Grid

vtLodGrid::vtLodGrid()
{
	m_dim = 100;
	m_fLODDistance = 0.0f;
	m_pHeightField = NULL;
}

void vtLodGrid::DetermineCell(const FPoint3 &pos, int &a, int &b)
{
	a = (int) ((pos.x - m_origin.x) / m_step.x);
	b = (int) ((pos.z - m_origin.z) / m_step.z);
}

bool vtLodGrid::AppendToGrid(vtTransform *pTNode)
{
	vtGroup *pGroup = FindCellParent(pTNode->GetTrans());
	if (pGroup)
	{
		pGroup->AddChild(pTNode);
		return true;
	}
	else
		return false;
}

bool vtLodGrid::AppendToGrid(vtGeode *pGNode)
{
	osg::BoundingSphere sph = pGNode->getBound();

	vtGroup *pGroup = FindCellParent(s2v(sph.center()));
	if (pGroup)
	{
		pGroup->addChild(pGNode);
		return true;
	}
	else
		return false;
}

void vtLodGrid::RemoveFromGrid(vtTransform *pTNode)
{
	vtGroup *pGroup = FindCellParent(pTNode->GetTrans());
	if (pGroup)
		pGroup->RemoveChild(pTNode);
}

void vtLodGrid::RemoveFromGrid(vtGeode *pGNode)
{
	osg::BoundingSphere sph = pGNode->getBound();

	vtGroup *pGroup = FindCellParent(s2v(sph.center()));
	if (pGroup)
		pGroup->removeChild(pGNode);
}

/**
 * This version is slower than calling RemoveFromGrid, but it covers more
 * situations.  It searches through all of the LOD grid's cells looking
 * for the node, so it will work even in cases where the object may have
 * moved out of its original cell.
 */
void vtLodGrid::RemoveNodeFromGrid(vtNode *pNode)
{
	int a, b;
	for (a = 0; a < m_dim; a++)
	{
		for (b = 0; b < m_dim; b++)
		{
			vtGroup *group = GetCell(a, b);
			if (group && group->ContainsChild(pNode))
			{
				group->RemoveChild(pNode);
				return;
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Simple LOD Grid

vtSimpleLodGrid::vtSimpleLodGrid()
{
	m_pCells = NULL;
}

void vtSimpleLodGrid::Setup(const FPoint3 &origin, const FPoint3 &size,
				 int iDimension, float fLODDistance, vtHeightField3d *pHF)
{
	m_origin = origin;
	m_size = size;
	m_dim = iDimension;
	m_fLODDistance = fLODDistance;
	m_step = m_size / (float)m_dim;

	// wrap with an array of simple LOD nodes
	m_pCells = (vtLOD **)malloc(m_dim * m_dim * sizeof(vtLOD *));

	int a, b;
	for (a = 0; a < m_dim; a++)
	{
		for (b = 0; b < m_dim; b++)
		{
			m_pCells[CellIndex(a,b)] = NULL;
		}
	}
	m_pHeightField = pHF;
}

void vtSimpleLodGrid::Release()
{
	// get rid of children first
	vtLOD *lod;
	int a, b;
	for (a = 0; a < m_dim; a++)
	{
		for (b = 0; b < m_dim; b++)
		{
			lod = m_pCells[CellIndex(a,b)];
			if (lod != NULL)
			{
				RemoveChild(lod);
				lod->Release();
			}
		}
	}
	free(m_pCells);
	m_pCells = NULL;

	// now self-destruct
	vtGroup::Release();
}


void vtSimpleLodGrid::AllocateCell(int a, int b)
{
	int i = CellIndex(a,b);

	m_pCells[i] = new vtLOD;
	vtString name;
	name.Format("LOD cell %d %d", a, b);
	m_pCells[i]->setName(name);

	float ranges[2];
	ranges[0] = 0.0f;
	ranges[1] = m_fLODDistance;
	m_pCells[i]->SetRanges(ranges, 2);

	// determine LOD center
	FPoint3 lod_center;
	lod_center.x = m_origin.x + ((m_size.x / m_dim) * (a + 0.5f));
	lod_center.y = m_origin.y + (m_size.y / 2.0f);
	lod_center.z = m_origin.z + ((m_size.z / m_dim) * (b + 0.5f));
	if (m_pHeightField)
		m_pHeightField->FindAltitudeAtPoint(lod_center, lod_center.y);

	vtGroup *group = new vtGroup;
	group->setName("LOD group");
	m_pCells[i]->SetCenter(lod_center);
	m_pCells[i]->AddChild(group);

	AddChild(m_pCells[i]);
}


vtGroup *vtSimpleLodGrid::GetCell(int a, int b)
{
	int i = CellIndex(a, b);
	vtLOD *pCell = m_pCells[i];
	if (!pCell)
		return NULL;
	return (vtGroup *) pCell->GetChild(0);
}

vtGroup *vtSimpleLodGrid::FindCellParent(const FPoint3 &point)
{
	int a, b;

	DetermineCell(point, a, b);
	if (a < 0 || a >= m_dim || b < 0 || b >= m_dim)
		return NULL;

	int i = CellIndex(a, b);
	if (!m_pCells[i])
		AllocateCell(a, b);

	return (vtGroup *)m_pCells[i]->GetChild(0);
}

void vtSimpleLodGrid::SetDistance(float fLODDistance)
{
	m_fLODDistance = fLODDistance;

	float ranges[2];
	ranges[0] = 0.0f;
	ranges[1] = m_fLODDistance;

	vtLOD *lod;
	int a, b;
	for (a = 0; a < m_dim; a++)
	{
		for (b = 0; b < m_dim; b++)
		{
			lod = m_pCells[CellIndex(a,b)];
			if (lod)
				lod->SetRanges(ranges, 2);
		}
	}
}


