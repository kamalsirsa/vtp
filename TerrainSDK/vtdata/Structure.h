//
// Structure.h
//
// Implements the vtStructure class which represents a single built structure.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTUREH
#define STRUCTUREH

#include "vtdata/MathTypes.h"
#include "Selectable.h"

class vtBuilding;
class vtFence;

enum vtStructureType
{
	ST_BUILDING,
	ST_FENCE,
	ST_WALL,
	ST_INSTANCE
};

class vtStructure : public Selectable
{
public:
	vtStructure() {}

	void SetBuilding(vtBuilding *bld) { m_pBuilding = bld; m_type = ST_BUILDING; }
	void SetFence(vtFence *fen) { m_pFence = fen; m_type = ST_FENCE; }

	vtStructureType GetType() { return m_type; }
	vtBuilding *GetBuilding() { if (m_type == ST_BUILDING) return m_pBuilding; else return NULL; }
	vtFence *GetFence() { if (m_type == ST_FENCE) return m_pFence; else return NULL; }

	virtual bool GetExtents(DRECT &rect);

protected:
	union {
		vtBuilding *m_pBuilding;
		vtFence *m_pFence;
	};
	vtStructureType m_type;
};

#endif // STRUCTUREH

