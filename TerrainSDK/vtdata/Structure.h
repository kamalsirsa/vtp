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

#include "MathTypes.h"
#include "Selectable.h"
#include "Content.h"

class vtBuilding;
class vtFence;
class vtStructInstance;

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
class vtStructure : public Selectable, public vtTagArray
{
public:
	vtStructure();
	virtual ~vtStructure();

	void SetType(vtStructureType t) { m_type = t; }
	vtStructureType GetType() { return m_type; }

	vtBuilding *GetBuilding() { if (m_type == ST_BUILDING) return (vtBuilding *)this; else return NULL; }
	vtFence *GetFence() { if (m_type == ST_FENCE) return (vtFence *)this; else return NULL; }
	vtStructInstance *GetInstance() { if (m_type == ST_INSTANCE) return (vtStructInstance *)this; else return NULL; }

	virtual bool GetExtents(DRECT &rect) const = 0;
	virtual bool IsContainedBy(const DRECT &rect) const = 0;
	virtual void WriteXML(FILE *fp, bool bDegrees) = 0;
	virtual void WriteXML_Old(FILE *fp, bool bDegrees) = 0;

	void WriteTags(FILE *fp);

protected:
	vtStructureType m_type;

private:
	// Don't let unsuspecting users stumble into assuming that object
	// copy semantics will work.  Declare them private and never
	// define them,

	vtStructure( const vtStructure & );
	vtStructure &operator=( const vtStructure & );
};

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
class vtStructInstance : public vtStructure
{
public:
	vtStructInstance();

	void WriteXML(FILE *fp, bool bDegrees);
	void WriteXML_Old(FILE *fp, bool bDegrees);
	void Offset(const DPoint2 &delta);

	bool GetExtents(DRECT &rect) const;
	bool IsContainedBy(const DRECT &rect) const;

	DPoint2	m_p;			// earth position
	float	m_fRotation;	// in radians
	float	m_fScale;		// meters per unit
};

#endif // STRUCTUREH

