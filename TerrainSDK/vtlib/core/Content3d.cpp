//
// Content3d.cpp
//
// 3D Content Management class.
//
// Copyright (c) 2003-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "Content3d.h"

vtItem3d::vtItem3d()
{
	m_pGroup = NULL;
}

vtItem3d::~vtItem3d()
{
	if (m_pGroup)
		m_pGroup->Release();
	m_pGroup = NULL;
}

/**
 * Load the model(s) associated with an item.  If there are several models,
 * generally these are different levels of detail (LOD) for the item.
 */
bool vtItem3d::LoadModels(vtStringArray *pDataPaths)
{
	if (m_pGroup)
		return true;	// already loaded

	int i, models = NumModels();

	// attempt to instantiate the item
	vtGroup *pGroup;
	vtLOD *pLod;

	if (models > 1)
	{
		pLod = new vtLOD;
		m_pGroup = pLod;
	}
	else
	{
		pGroup = new vtGroup;
		m_pGroup = pGroup;
	}
	float ranges[20];
	ranges[0] = 0.0f;

	for (i = 0; i < models; i++)
	{
		vtModel *model = GetModel(i);
		vtNode *pNode = NULL;

		// if there are some data path(s) to search, use them
		if (pDataPaths != NULL)
		{
			vtString fullpath = FindFileOnPaths(*pDataPaths, model->m_filename);
			if (fullpath != "")
				pNode = vtNode::LoadModel(fullpath);
		}
		else
			// perhaps it's directly resolvable
			pNode = vtNode::LoadModel(model->m_filename);

		if (!pNode)
		{
			VTLOG("Couldn't load model from %hs\n",
				(const char *) model->m_filename);
			return false;
		}

		// Wrap in a transform node so that we can scale/rotate the node
		vtTransform *pTrans = new vtTransform();
		pTrans->AddChild(pNode);

		vtString ext = GetExtension(model->m_filename, false);

		pTrans->Identity();
		pTrans->Scale3(model->m_scale, model->m_scale, model->m_scale);

		m_pGroup->AddChild(pTrans);

		if (models > 1)
			ranges[i+1] = model->m_distance;
	}
	if (models > 1)
		pLod->SetRanges(ranges, models+1);

	return true;
}

//
// An item can store some extents, which give a rough indication of
//  the 2D area taken up by the model, useful for drawing it in traditional
//  2D GIS environments like VTBuilder.
//
//  Whenever a model is added, or the scale factor changes, the extents
//   should be updated.
//
void vtItem3d::UpdateExtents()
{
	m_extents.Empty();

	if (m_pGroup == NULL)
		return;

	// A good way to do it would be to try to get a tight bounding box,
	//  but that's non-trivial to compute with OSG.  For now, settle for
	//  making a rectangle from the loose bounding sphere.

	// Both the 3D model and the extents are in approximate meters and
	//  centered on the item's local origin.
	FSphere sph;
	m_pGroup->GetBoundSphere(sph);
	m_extents.left = sph.center.x - sph.radius;
	m_extents.right = sph.center.x + sph.radius;

	// However, the XY extents of the extents have Y pointing up, whereas
	//  the world coords have Z pointing down.
	m_extents.top = -sph.center.z + sph.radius;
	m_extents.bottom = -sph.center.z - sph.radius;
}


///////////////////////////////////////////////////////////////////////

vtContentManager3d::vtContentManager3d()
{
	m_pDataPaths = NULL;
}

vtGroup *vtContentManager3d::CreateGroupFromItemname(const char *itemname)
{
	vtItem3d *pItem = (vtItem3d *) FindItemByName(itemname);
	if (!pItem)
		return NULL;

	if (pItem->LoadModels(m_pDataPaths))
		return pItem->m_pGroup;
	else
		return NULL;
}

