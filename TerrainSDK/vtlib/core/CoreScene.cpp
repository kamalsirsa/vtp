//
// CoreScene.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Engine.h"

void vtSceneBase::OnMouse(vtMouseEvent &event)
{
	// Pass event to Engines
	for (int i = 0; i < m_Engines.GetSize(); i++)
	{
		vtEngine *pEng = m_Engines.GetAt(i);
		pEng->OnMouse(event);
	}
}

void vtSceneBase::OnKey(int key, int flags)
{
	// Pass event to Engines
	for (int i = 0; i < m_Engines.GetSize(); i++)
	{
		vtEngine *pEng = m_Engines.GetAt(i);
		pEng->OnKey(key, flags);
	}
}

void vtSceneBase::DoEngines()
{
	// Evaluate Engines
	for (int i = 0; i < m_Engines.GetSize(); i++)
	{
		vtEngine *pEng = m_Engines.GetAt(i);
		if (pEng->GetEnabled())
			pEng->Eval();
	}
}

////////////////////////////////////////////////////////////////

vtNode *vtNodeBase::FindDescendantByName(const char *name)
{
	if (!strcmp(GetName2(), name))
		return (dynamic_cast<vtNode *>(this));

	vtGroupBase *pGroup = dynamic_cast<vtGroupBase *>(this);
	if (pGroup)
	{
		for (int i = 0; i < pGroup->GetNumChildren(); i++)
		{
			vtNode *pChild = pGroup->GetChild(i);
			vtNode *pResult = pChild->FindDescendantByName(name);
			if (pResult)
				return pResult;
		}
	}
	return NULL;
}

