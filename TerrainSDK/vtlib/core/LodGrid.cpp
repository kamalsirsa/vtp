//
// LodGrid.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/LocalConversion.h"
#include "vtdata/HeightField.h"
#include "LodGrid.h"

#define index(a,b) ((a*m_dim)+b)

vtLodGrid::vtLodGrid(const FPoint3 &origin, const FPoint3 &size,
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
			m_pCells[index(a,b)] = NULL;
		}
	}
	m_pHeightField = pHF;
}

void vtLodGrid::Release()
{
	// get rid of children first
	vtLOD *lod;
	int a, b;
	for (a = 0; a < m_dim; a++)
	{
		for (b = 0; b < m_dim; b++)
		{
			lod = m_pCells[index(a,b)];
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


void vtLodGrid::AllocateCell(int a, int b)
{
	int i = index(a,b);

	m_pCells[i] = new vtLOD();

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

	m_pCells[i]->SetCenter(lod_center);
	m_pCells[i]->AddChild(new vtGroup());

	AddChild(m_pCells[i]);
}

void vtLodGrid::DetermineCell(const FPoint3 &pos, int &a, int &b)
{
	a = (int) ((pos.x - m_origin.x) / m_step.x);
	b = (int) ((pos.z - m_origin.z) / m_step.z);
}

vtGroup *vtLodGrid::FindCellParent(const FPoint3 &point)
{
	int a, b;

	DetermineCell(point, a, b);
	if (a < 0 || a >= m_dim || b < 0 || b >= m_dim)
		return NULL;

	int i = index(a, b);
	if (!m_pCells[i])
		AllocateCell(a, b);

	return (vtGroup *)m_pCells[i]->GetChild(0);
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

bool vtLodGrid::AppendToGrid(vtGeom *pGNode)
{
	FSphere sph;
	pGNode->GetBoundSphere(sph);

	vtGroup *pGroup = FindCellParent(sph.center);
	if (pGroup)
	{
		pGroup->AddChild(pGNode);
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

void vtLodGrid::RemoveFromGrid(vtGeom *pGNode)
{
	FSphere sph;
	pGNode->GetBoundSphere(sph);

	vtGroup *pGroup = FindCellParent(sph.center);
	if (pGroup)
		pGroup->RemoveChild(pGNode);
}

/**
 * This version is slower but safer than calling RemoveFromGrid.  Is
 * searches through all of the LOD grid's cells looking for the node,
 * so it will work even in cases where the object may have moved
 * out of its original cell.
 */
void vtLodGrid::RemoveNodeFromGrid(vtNode *pNode)
{
	vtLOD *lod;
	int a, b;
	for (a = 0; a < m_dim; a++)
	{
		for (b = 0; b < m_dim; b++)
		{
			lod = m_pCells[index(a,b)];
			if (lod == NULL)
				continue;
			vtGroup *group = (vtGroup *) lod->GetChild(0);
			if (group->ContainsChild(pNode))
			{
				group->RemoveChild(pNode);
				return;
			}
		}
	}
}

void vtLodGrid::SetDistance(float fLODDistance)
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
			lod = m_pCells[index(a,b)];
			if (lod)
				lod->SetRanges(ranges, 2);
		}
	}
}

float vtLodGrid::GetDistance()
{
	return m_fLODDistance;
}

