//
// Structure.h
//
// Implements the vtStructure class which represents a single built structure.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file Structure.h */


#ifndef STRUCTUREH
#define STRUCTUREH

#include "vtdata/MathTypes.h"
#include "Selectable.h"
#include "Content.h"

class vtBuilding;
class vtFence;

/**
 * This class represents a reference to an external model, such as a unique
 * building which has been created in a 3D Modelling Tool.  It is derived from
 * vtTagArray which provides a set of arbitrary tags (name/value pairs).
 * At least one of the following two tags should be present:
 * - filename, which contains a resolvable path to an external 3d model file.
 *	 An example is filename="MyModels/GasStation.3ds"
 * - itemname, which contains the name of a content item which will be resolved
 *	 by a list maintained by a vtContentManager.  An example is
 *	 itemname="Gemini Observatory"
 */
class vtStructInstance : public vtTagArray
{
public:
	vtStructInstance();

	void WriteXML(FILE *fp, bool bDegrees);
	bool GetExtents(DRECT &rect);
	void Offset(const DPoint2 &delta);

	DPoint2	m_p;			// earth position
	float	m_fRotation;	// in radians
	float	m_fScale;		// meters per unit
};

/**
 * Structure type.
 */
enum vtStructureType
{
	ST_BUILDING,	/**< A Building (vtBuilding) */
	ST_FENCE,		/**< A Fence (vtFence) */
	ST_INSTANCE,	/**< A Structure Instance (vtStructInstance) */
	ST_NONE
};

/**
 * The vtStructure class represents any "built structure".  These are
 * generally immobile, artificial entities of human-scale and larger, such
 * as buildings and fences.
 * \par
 * Structures are implemented as 3 types:
 *  - Buildings (vtBuilding)
 *  - Fences and walls (vtFence)
 *  - Instances (vtStructInstance)
 * \par
 * For enclosed and linear structures which can be well-described
 * parametrically, vtBuilding and vtFence provide efficient data representation.
 * For other structures which are not easily reduced to parameters, the Instance
 * type allows you to reference any external model, such as a unique building
 * which has been created in a 3D Modelling Tool.
 */
class vtStructure : public Selectable
{
public:
	vtStructure();
	~vtStructure();

	void SetBuilding(vtBuilding *bld) { m_pBuilding = bld; m_type = ST_BUILDING; }
	void SetFence(vtFence *fen) { m_pFence = fen; m_type = ST_FENCE; }
	void SetInstance(vtStructInstance *inst) { m_pInstance = inst; m_type = ST_INSTANCE; }

	vtStructureType GetType() { return m_type; }
	vtBuilding *GetBuilding() { if (m_type == ST_BUILDING) return m_pBuilding; else return NULL; }
	vtFence *GetFence() { if (m_type == ST_FENCE) return m_pFence; else return NULL; }
	vtStructInstance *GetInstance() { if (m_type == ST_INSTANCE) return m_pInstance; else return NULL; }

	virtual bool GetExtents(DRECT &rect);
	bool IsContainedBy(const DRECT &rect);

protected:
	union {
		vtBuilding *m_pBuilding;
		vtFence *m_pFence;
		vtStructInstance *m_pInstance;
	};
	vtStructureType m_type;
};

#endif // STRUCTUREH

