//
// StructArray.h
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTARRAYH
#define STRUCTARRAYH

#include "MathTypes.h"
#include "Projections.h"
#include "Structure.h"
#include <stdio.h>


/**  The vtStructureArray class contains a list of Built Structures
 * (vtStructure objects).  It can be loaded and saved to VTST files
 * with the ReadXML and WriteXML methods.
 *
 */
class vtStructureArray : public Array<vtStructure*>
{
public:
	vtStructureArray() { m_pEditBuilding = NULL; }
	~vtStructureArray() { Empty(); }
	virtual void DestructItems(int first, int last);	// override

	void DeleteSelected();
	virtual void DestroyStructure(int i) {}

	bool ReadSHP(const char* pathname, vtStructureType type,
		const DRECT &rect, bool bFlip, void progress_callback(int) = NULL);
	bool ReadBCF(const char* pathname);		// read a .bcf file
	bool ReadBCF_Old(FILE *fp);				// support obsolete format
	bool ReadXML(const char* pathname);

	bool WriteSHP(const char* pathname);
	bool WriteXML(const char* pathname);
	bool WriteXML_Old(const char* pathname);

	bool FindClosestBuildingCorner(const DPoint2 &point, double error,
						   int &building, int &corner, double &distance);
	bool FindClosestBuildingCenter(const DPoint2 &point, double error,
						   int &building, double &distance);

	/** Find the structure which is closest to the given point, if it is within
	 * 'error' distance.  The structure index and distance are returned by
	 * reference.
	 */
	bool FindClosestStructure(const DPoint2 &point, double error,
						   int &building, double &distance);

	bool IsEmpty() { return (GetSize() == 0); }
	void GetExtents(DRECT &ext);

	// selection
	int NumSelected();
	void DeselectAll();

	// override these 'Factory' methods so that the vtStructureArray base
	// methods can be capable of handling subclasses of vtBuilding
	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	// override to catch edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

	vtProjection m_proj;

protected:
	// used to indicate which edge should be hightlighted during editing
	vtBuilding *m_pEditBuilding;
	int m_iEditLevel;
	int m_iEditEdge;
};

// Helper
int GetSHPType(const char *filename);

#endif
