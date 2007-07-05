//
// PagedLodGrid.cpp
//
// Copyright (c) 2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Structure3d.h"

#include "vtdata/LocalConversion.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtLog.h"

#include "PagedLodGrid.h"


vtPagedStructureLOD::vtPagedStructureLOD() : vtGroup(true)
{
	m_bConstructed = false;
	m_pNativeLOD = new PagedNativeLOD;
	m_pNativeLOD->SetCenter(FPoint3(0, 0, 0));
#if VTLIB_OSG
	SetOsgGroup(m_pNativeLOD);
#endif
}

void vtPagedStructureLOD::Release()
{
#if VTLIB_OSG
	// Check if this node is no longer referenced.
	if (m_pNode->referenceCount() == 1)
		m_pNativeLOD = NULL;
#endif
	vtGroup::Release();
}

void vtPagedStructureLOD::SetCenter(const FPoint3 &center)
{
	m_pNativeLOD->SetCenter(center);
}

void vtPagedStructureLOD::GetCenter(FPoint3 &center)
{
	m_pNativeLOD->GetCenter(center);
}

bool vtPagedStructureLOD::TestVisible(float distance)
{
	if (distance < m_fRange)
	{
		if (!m_bConstructed)
			Construct();
		return true;
	}
	// It is not sufficient to do the test here, because this is only called
	//  for nodes within the view frustum.
	// if (distance > m_fRange * 2 && m_bConstructed) Deconstruct();
	return false;
}

void vtPagedStructureLOD::Construct()
{
	VTLOG("Constructing %d buildings\n", m_Structures.GetSize());
	for (unsigned int i = 0; i < m_Structures.GetSize(); i++)
	{
		vtStructure3d *s3d = m_Structures[i];
		bool bSuccess = m_pStructureArray->ConstructStructure(s3d);
		if (bSuccess)
		{
			vtTransform *pTrans = s3d->GetContainer();
			if (pTrans)
				AddChild(pTrans);
		}
	}
	m_bConstructed = true;
}

void vtPagedStructureLOD::Deconstruct()
{
	int count = 0;
	VTLOG("Deconstruction check on %d structures: ", m_Structures.GetSize());
	for (unsigned int i = 0; i < m_Structures.GetSize(); i++)
	{
		vtStructure3d *str3d = m_Structures[i];
		vtNode *node = str3d->GetContainer();
		if (!node)
			node = str3d->GetGeom();
		if (!node)
			continue;
		RemoveChild(node);
		str3d->DeleteNode();
		count++;
	}
	VTLOG("%d decon.\n", count);
	m_bConstructed = false;
}

#define CellIndex(a,b) ((a*m_dim)+b)

vtPagedStructureLodGrid::vtPagedStructureLodGrid()
{
	m_pStructureArray = NULL;
	m_pCells = NULL;
}

void vtPagedStructureLodGrid::Setup(const FPoint3 &origin, const FPoint3 &size,
				 int iDimension, float fLODDistance, vtHeightField3d *pHF)
{
	m_origin = origin;
	m_size = size;
	m_dim = iDimension;
	m_fLODDistance = fLODDistance;
	m_step = m_size / (float)m_dim;

	// wrap with an array of simple LOD nodes
	m_pCells = (vtPagedStructureLOD **)malloc(m_dim * m_dim * sizeof(vtPagedStructureLOD *));

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

void vtPagedStructureLodGrid::Release()
{
	// get rid of children first
	vtPagedStructureLOD *lod;
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


void vtPagedStructureLodGrid::AllocateCell(int a, int b)
{
	int i = CellIndex(a,b);

	m_pCells[i] = new vtPagedStructureLOD;
	vtString name;
	name.Format("LOD cell %d %d", a, b);
	m_pCells[i]->SetName2(name);

	m_pCells[i]->SetRange(m_fLODDistance);

	// determine LOD center
	FPoint3 lod_center;
	lod_center.x = m_origin.x + ((m_size.x / m_dim) * (a + 0.5f));
	lod_center.y = m_origin.y + (m_size.y / 2.0f);
	lod_center.z = m_origin.z + ((m_size.z / m_dim) * (b + 0.5f));
	if (m_pHeightField)
		m_pHeightField->FindAltitudeAtPoint(lod_center, lod_center.y);
	m_pCells[i]->SetCenter(lod_center);
	m_pCells[i]->SetArray(m_pStructureArray);

	AddChild(m_pCells[i]);
}

vtGroup *vtPagedStructureLodGrid::GetCell(int a, int b)
{
	int i = CellIndex(a, b);
	return m_pCells[i];
}

vtPagedStructureLOD *vtPagedStructureLodGrid::FindPagedCellParent(const FPoint3 &point)
{
	int a, b;

	DetermineCell(point, a, b);
	if (a < 0 || a >= m_dim || b < 0 || b >= m_dim)
		return NULL;

	int i = CellIndex(a, b);
	if (!m_pCells[i])
		AllocateCell(a, b);

	return m_pCells[i];
}

vtGroup *vtPagedStructureLodGrid::FindCellParent(const FPoint3 &point)
{
	return FindPagedCellParent(point);
}

void vtPagedStructureLodGrid::SetDistance(float fLODDistance)
{
	m_fLODDistance = fLODDistance;

	for (int a = 0; a < m_dim; a++)
	{
		for (int b = 0; b < m_dim; b++)
		{
			vtPagedStructureLOD *lod = m_pCells[CellIndex(a,b)];
			if (lod)
				lod->SetRange(m_fLODDistance);
		}
	}
}

bool vtPagedStructureLodGrid::AppendToGrid(vtStructure *str, vtStructure3d *str3d)
{
	DRECT rect;
	if (str->GetExtents(rect))
	{
		float xmin, xmax, zmin, zmax;
		g_Conv.convert_earth_to_local_xz(rect.left, rect.bottom, xmin, zmin);
		g_Conv.convert_earth_to_local_xz(rect.right, rect.top, xmax, zmax);

		FPoint3 mid((xmin+xmax) / 2, 0.0f, (zmin+zmax)/2);

		vtPagedStructureLOD *pGroup = FindPagedCellParent(mid);
		if (pGroup)
			pGroup->Add(str3d);

		return true;
	}
	return false;
}

void vtPagedStructureLodGrid::DeleteFarawayStructures(const FPoint3 &CamPos,
													  int iMaxStructures,
													  float fDistance)
{
	int total_constructed = 0;
	for (int a = 0; a < m_dim; a++)
	{
		for (int b = 0; b < m_dim; b++)
		{
			vtPagedStructureLOD *lod = m_pCells[CellIndex(a,b)];
			if (lod) total_constructed += lod->GetNumChildren();
		}
	}
	if (total_constructed < iMaxStructures)
		return;

	FPoint3 center;
	for (int a = 0; a < m_dim; a++)
	{
		for (int b = 0; b < m_dim; b++)
		{
			vtPagedStructureLOD *lod = m_pCells[CellIndex(a,b)];
			if (!lod || !lod->IsConstructed())
				continue;
			lod->GetCenter(center);
			float dist = (center - CamPos).Length();
			if (dist > fDistance)
				lod->Deconstruct();
		}
	}
}
