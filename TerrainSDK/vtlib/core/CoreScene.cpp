//
// CoreScene.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

/**
\mainpage vtlib library documentation

\section overview Overview
The <b>vtlib</b> library is built on top of the <b>vtdata</b> library which
handles many kinds of geospatial data.  It extends <b>vtdata</b> with the
ability to create 3d geometry of the data for interactive visualization.

It is part of the <a href="http://vterrain.org/">Virtual Terrain Project</a>
and distributed under a completely free <a href="../../license.txt">open
source license</a>.

<b>vtlib</b> contains an abstraction of a Scene Graph API, built on top of
OpenGL and other lower-level APIs such as OSG and SGL.
Because it is higher-level, <b>vtlib</b> lets you construct 3d geometry
and simulations much faster and easier than using the lower-level libraries
directly.
*/

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

