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

#include <algorithm>	// for sort

vtPagedStructureLOD::vtPagedStructureLOD() : vtGroup(true)
{
	m_iNumConstructed = 0;
	m_bAddedToQueue = false;

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

/**
 * \param fDistance The distance in meters to check against.
 * \param bLoad If true, and this cell is within the distance, and it isn't
 *		loaded, then load it.
 */
bool vtPagedStructureLOD::TestVisible(float fDistance, bool bLoad)
{
	if (fDistance < m_fRange)
	{
		// Check if this group has any unbuilt structures
		if (bLoad && !m_bAddedToQueue &&
			m_iNumConstructed != m_StructureIndices.GetSize())
		{
			AppendToQueue();
			m_bAddedToQueue = true;
		}
		return true;
	}
	// It is not sufficient to do the too-far test here, because this is
	//  only called for nodes within the view frustum.
	// if (distance > m_fRange * 2 && m_bConstructed) Deconstruct();
	return false;
}

void vtPagedStructureLOD::AppendToQueue()
{
	int count = 0;
	for (unsigned int i = 0; i < m_StructureIndices.GetSize(); i++)
	{
		if (m_pGrid->AddToQueue(this, m_StructureIndices[i]))
			count++;
	}
	VTLOG("Added %d buildings to queue.\n", count);

	// We have just added a lump of structures, sort them by distance
	m_pGrid->SortQueue();
}


///////////////////////////////////////////////////////////////////////
// vtPagedStructureLodGrid

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
	m_pCells[i]->SetGrid(this);

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

bool vtPagedStructureLodGrid::AppendToGrid(int iIndex)
{
	// Get 2D extents from the unbuild structure
	vtStructure *str = m_pStructureArray->GetAt(iIndex);
	DRECT rect;
	if (str->GetExtents(rect))
	{
		float xmin, xmax, zmin, zmax;
		g_Conv.convert_earth_to_local_xz(rect.left, rect.bottom, xmin, zmin);
		g_Conv.convert_earth_to_local_xz(rect.right, rect.top, xmax, zmax);

		FPoint3 mid((xmin+xmax) / 2, 0.0f, (zmin+zmax)/2);

		vtPagedStructureLOD *pGroup = FindPagedCellParent(mid);
		if (pGroup)
			pGroup->Add(iIndex);

		return true;
	}
	return false;
}

vtPagedStructureLOD *vtPagedStructureLodGrid::GetPagedCell(int a, int b)
{
	return m_pCells[CellIndex(a,b)];
}

void vtPagedStructureLodGrid::DeconstructCell(vtPagedStructureLOD *pLOD)
{
	int count = 0;
	vtArray<int> &indices = pLOD->m_StructureIndices;
	VTLOG("Deconstruction check on %d structures: ", indices.GetSize());
	for (unsigned int i = 0; i < indices.GetSize(); i++)
	{
		vtStructure3d *str3d = m_pStructureArray->GetStructure3d(indices[i]);
		vtNode *node = str3d->GetContainer();
		if (!node)
			node = str3d->GetGeom();
		if (!node)
			continue;
		pLOD->RemoveChild(node);
		str3d->DeleteNode();
		count++;
	}
	VTLOG("%d decon.\n", count);
	pLOD->m_iNumConstructed = 0;
	pLOD->m_bAddedToQueue = false;
}

void vtPagedStructureLodGrid::RemoveCellFromQueue(vtPagedStructureLOD *pLOD)
{
	if (!pLOD->m_bAddedToQueue)
		return;
	if (pLOD->m_iNumConstructed == pLOD->m_StructureIndices.GetSize())
		return;

	vtArray<int> &indices = pLOD->m_StructureIndices;
	VTLOG("Dequeueing check on %d structures: ", indices.GetSize());
	int count = 0;
	for (unsigned int i = 0; i < indices.GetSize(); i++)
	{
		if (RemoveFromQueue(indices[i]))
			count++;
	}
	VTLOG("%d dequeued.\n", count);
	pLOD->m_bAddedToQueue = false;
}

void vtPagedStructureLodGrid::CullFarawayStructures(const FPoint3 &CamPos,
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
	// If we have too many or have items in the queue
	if (total_constructed > iMaxStructures || m_Queue.size() > 0)
	{
		// Delete/dequeue the ones that are very far
		FPoint3 center;
		for (int a = 0; a < m_dim; a++)
		{
			for (int b = 0; b < m_dim; b++)
			{
				vtPagedStructureLOD *lod = m_pCells[CellIndex(a,b)];
				if (!lod || lod->m_iNumConstructed == 0)
					continue;
				lod->GetCenter(center);
				float dist = (center - CamPos).Length();

				// If very far, delete the structures entirely
				if (total_constructed > iMaxStructures && dist > fDistance)
					DeconstructCell(lod);

				// If it has fallen out of the frustum, remove them
				//  from the queue
				if (dist > m_fLODDistance)
					RemoveCellFromQueue(lod);
			}
		}
	}
}

bool operator<(const QueueEntry& a, const QueueEntry& b)
{
	// Reverse-sort, to put smallest values (closest points) at the end
	//  of the list so they can be efficiently removed
    return a.fDistance > b.fDistance;
}

void vtPagedStructureLodGrid::SortQueue()
{
	vtCamera *cam = vtGetScene()->GetCamera();
	FPoint3 CamPos = cam->GetTrans();
	FPoint3 CamDir = cam->GetDirection();

	// Prioritization is by distance.
	// We can measure horizontal distance, which is faster.
	DPoint3 cam_epos, cam_epos2;
	g_Conv.ConvertToEarth(CamPos, cam_epos);
	g_Conv.ConvertToEarth(CamPos+CamDir, cam_epos2);
	DPoint2 cam_pos(cam_epos.x, cam_epos.y);
	DPoint2 cam_dir(cam_epos2.x - cam_epos.x, cam_epos2.y - cam_epos.y);
	cam_dir.Normalize();

	DPoint2 p;
	for (unsigned int i = 0; i < m_Queue.size(); i++)
	{
		QueueEntry &e = m_Queue[i];
		vtStructure *st = m_pStructureArray->GetAt(e.iStructIndex);
		vtBuilding *bld = st->GetBuilding();
		vtStructInstance *inst = st->GetInstance();
		if (bld)
			p = bld->GetAtFootprint(0).Centroid();
		else if (inst)
			p = inst->GetPoint();
		else
			continue;

		// Calculate distance
		DPoint2 diff = p-cam_pos;
		e.fDistance = (float) diff.Length();

		// Is point behind the camera?  If so, give it lowest priority
		if (diff.Dot(cam_dir) < 0)
			e.fDistance += 1E5;
	}
	std::sort(m_Queue.begin(), m_Queue.end());
}


void vtPagedStructureLodGrid::DoPaging(const FPoint3 &CamPos,
									   int iMaxStructures, float fDeleteDistance)
{
	static float last_cull = 0.0f, last_load = 0.0f, last_prioritize = 0.0f;
	float current = vtGetTime();

	if (current - last_prioritize > 1.0f)
	{
		// Do a re-priortization every 1 second.
		SortQueue();
		last_prioritize = current;
	}
	else if (current - last_cull > 0.25f)
	{
		// Do a paging cleanup pass every 1/4 of a second
		// Unload/unqueue anything excessive
		CullFarawayStructures(CamPos, iMaxStructures, fDeleteDistance);
		last_cull = current;
	}
	else if (current - last_load > 0.01f && !m_Queue.empty())
	{
		// Do loading every other available frame
		last_load = current;

		// Check if the camera is not moving; if so, construct more.
		int construct;
		static FPoint3 last_campos;
		if (CamPos == last_campos)
			construct = 5;
		else
			construct = 1;

		for (int i = 0; i < construct && m_Queue.size() > 0; i++)
		{
			// Gradually load anything that needs loading
			const QueueEntry &e = m_Queue.back();
			bool bSuccess = m_pStructureArray->ConstructStructure(e.iStructIndex);
			if (bSuccess)
			{
				vtStructure3d *str3d = m_pStructureArray->GetStructure3d(e.iStructIndex);
				vtTransform *pTrans = str3d->GetContainer();
				if (pTrans)
					e.pLOD->AddChild(pTrans);
				e.pLOD->m_iNumConstructed ++;
			}
			else
			{
				VTLOG("Error: couldn't construct index %d\n", e.iStructIndex);
				vtStructInstance *si = m_pStructureArray->GetInstance(e.iStructIndex);
				if (si)
				{
					const char *fname = si->GetValueString("filename", true);
					VTLOG("\tinstance fname: '%s'\n", fname ? fname : "null");
				}
			}
			m_Queue.pop_back();
		}
		last_campos = CamPos;
	}
}

bool vtPagedStructureLodGrid::AddToQueue(vtPagedStructureLOD *pLOD, int iIndex)
{
	// Check if it's already built
	vtStructure3d *str3d = m_pStructureArray->GetStructure3d(iIndex);
	if (str3d && str3d->IsCreated())
		return false;

	// If not, add it
	QueueEntry e;
	e.pLOD = pLOD;
	e.iStructIndex = iIndex;
	e.fDistance = 1E9;
	m_Queue.push_back(e);
	return true;
}

bool vtPagedStructureLodGrid::RemoveFromQueue(int iIndex)
{
	// Check if it's in the queue
	for (unsigned int i = 0; i < m_Queue.size(); i++)
	{
		QueueEntry &e = m_Queue[i];
		if (e.iStructIndex == iIndex)
		{
			m_Queue.erase(m_Queue.begin()+i);
			return true;
		}
	}
	return false;
}

