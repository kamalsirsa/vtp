//
// Structure.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Structure.h"
#include "Building.h"
#include "Fence.h"

bool vtStructure::GetExtents(DRECT &rect)
{
	if (m_type == ST_BUILDING)
		return m_pBuilding->GetExtents(rect);

	if (m_type == ST_FENCE)
		return m_pFence->GetExtents(rect);

	return false;
}
