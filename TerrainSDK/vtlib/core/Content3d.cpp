//
// Content3d.cpp
//
// 3D Content Management class.
//
// Copyright (c) 2003 Virtual Terrain Project.
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
		pGroup = new vtLOD;
		m_pGroup = pGroup;
	}
	float ranges[20];
	ranges[0] = 0.0f;

	for (i = 0; i < models; i++)
	{
		vtModel *model = GetModel(i);
		vtNodeBase *pNode;

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
		if (ext.CompareNoCase(".3ds") == 0 ||
//			ext.CompareNoCase(".lwo") == 0 ||
			ext.CompareNoCase(".flt") == 0)
		{
			// Must rotate by 90 degrees for 3DS -> OpenGL (or Lightwave LWO)
			pTrans->Rotate2(FPoint3(1.0f, 0.0f, 0.0f), -PID2f);
		}
		pTrans->Scale3(model->m_scale, model->m_scale, model->m_scale);

		m_pGroup->AddChild(pTrans);

		if (models > 1)
			ranges[i+1] = model->m_distance;
	}
	if (models > 1)
		pLod->SetRanges(ranges, models+1);

	return true;
}

///////////////////////////////////////////////////////////////////////

vtContentManager3d::vtContentManager3d()
{
	m_pDataPaths = NULL;
}

vtGroup *vtContentManager3d::CreateInstanceOfItem(vtItem *item)
{
	vtItem3d *pItem = (vtItem3d *) item;

	if (pItem->LoadModels(m_pDataPaths))
		return pItem->m_pGroup;
	else
		return NULL;
}

