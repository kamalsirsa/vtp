//
// Name:	 ItemGroup.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "frame.h"
#include "ItemGroup.h"


ItemGroup::ItemGroup(vtItem *pItem)
{
	m_pItem = pItem;
}

void ItemGroup::CreateNodes()
{
	m_pCage = NULL;
	m_pGroup = new vtGroup();
	m_pLOD = new vtLOD();
	m_pTop = new vtGroup;
	m_pTop->SetName2("ItemGroupTop");
	m_pTop->AddChild(m_pLOD);
	m_pTop->AddChild(m_pGroup);
}

void ItemGroup::AttemptToLoadModels()
{
	int i, num_models = m_pItem->NumModels();

	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		vtTransform *trans = GetMainFrame()->m_nodemap[mod];
		if (!trans && !mod->m_attempted_load)
		{
			// haven't tried to load it yet
			GetMainFrame()->AttemptLoad(mod);
		}
	}
}

void ItemGroup::AttachModels()
{
	// Undo previous attachments
	vtNode *pNode;
	while (pNode = m_pLOD->GetChild(0))
		m_pLOD->RemoveChild(pNode);
	while (pNode = m_pGroup->GetChild(0))
		m_pGroup->RemoveChild(pNode);

	// re-attach
	int i, num_models = m_pItem->NumModels();
	FSphere sph(FPoint3(0,0,0), 1.0f);
	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		vtNode *node = GetMainFrame()->m_nodemap[mod];
		if (node)
		{
			m_pGroup->AddChild(node);
			m_pLOD->AddChild(node);
			node->GetBoundSphere(sph);
		}
	}

	// Update ruler
	if (m_pCage)
	{
		m_pTop->RemoveChild(m_pCage);
		delete m_pCage;
	}
	float size = sph.radius * 1.1f;
	m_pCage = Create3DCursor(size, size/40);
	m_pCage->SetName2("Cage");
	m_pTop->AddChild(m_pCage);
}

void ItemGroup::SetRanges()
{
	// Now set the LOD ranges for each model
	int i, num_models = m_pItem->NumModels();
	if (!num_models)
		return;

	// LOD documentation: For N children, you must have N+1 range values.
	// "Note that the last child (n) does not implicitly have a maximum
	//  distance value of infinity.  You must add a n+1'st range value to
	//  specify its maximum distance.  Otherwise, "bad things" will happen."

	m_ranges[0] = 0.0f;
	if (num_models == 1)
		m_ranges[1] = 10000000.0f;
	else
	{
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			m_ranges[i+1] = mod->m_distance;
		}
	}
	m_pLOD->SetRanges(m_ranges, num_models+1);
}

void ItemGroup::ShowLOD(bool bTrue)
{
	m_pLOD->SetEnabled(bTrue);
	m_pGroup->SetEnabled(!bTrue);
	if (bTrue)
	{
		// LOD requires all models to be enabled
		int i, num_models = m_pItem->NumModels();
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			vtTransform *trans = GetMainFrame()->m_nodemap[mod];
			if (trans)
				trans->SetEnabled(true);
		}
	}
	else
	{
		// Group requires all models to be (initially) disabled
		int i, num_models = m_pItem->NumModels();
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			vtTransform *trans = GetMainFrame()->m_nodemap[mod];
			if (trans)
				trans->SetEnabled(false);
		}
	}
}

