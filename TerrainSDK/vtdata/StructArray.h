//
// StructArray.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTARRAYH
#define STRUCTARRAYH

#include "MathTypes.h"
#include "Projections.h"
#include "Structure.h"
#include <stdio.h>

#define BCFVERSION_CURRENT		1.2f
#define BCFVERSION_SUPPORTED	1.1f

/**  The vtStructureArray class contains a list of Built Structures
 * (vtStructure objects).  It can be loaded and saved to VTST files
 * with the ReadXML and WriteXML methods.
 *
 *****/
class vtStructureArray : public Array<vtStructure*>
{
public:
	~vtStructureArray() { Empty(); }
	virtual void DestructItems(int first, int last);	// override

	void AddBuilding(vtBuilding *bld);
	void DeleteSelected();

	bool ReadSHP(const char* pathname);
	bool ReadBCF(const char* pathname);		// read a .bcf file
	bool ReadBCF_Old(FILE *fp);				// support obsolete format
	bool ReadXML(const char* pathname);

	bool WriteSHP(const char* pathname);
	bool WriteXML(const char* pathname);

	/** Find the building corner closest to the given point, if it is within
	 * 'error' distance.  The building index, corner index, and distance from
	 * the given point are all returned by reference.
	 */
	bool FindClosestBuildingCorner(const DPoint2 &point, double error,
						   int &building, int &corner, double &distance);
	/** Find the building center closest to the given point, if it is within
	 * 'error' distance.  The building index, and distance from the given
	 * point are returned by reference.
	 */
	bool FindClosestBuildingCenter(const DPoint2 &point, double error,
						   int &building, double &distance);

	bool IsEmpty() { return (GetSize() == 0); }
	void GetExtents(DRECT &ext);

	// selection
	int NumSelected();
	void DeselectAll();

	// override these 'Factory' methods so that the vtStructureArray base
	// methods can be capable of handling subclasses of vtBuilding
	virtual vtStructure *NewStructure() { return new vtStructure; }
	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();

	vtProjection m_proj;
protected:
};

#endif
