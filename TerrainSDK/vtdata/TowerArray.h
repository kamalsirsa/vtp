//
// TowerArray.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TOWERARRAY_H
#define TOWERARRAY_H

#include "MathTypes.h"
#include "Projections.h"
#include "Tower.h"
#include <stdio.h>

#define UTLVERSION_CURRENT		1.1f
#define UTLVERSION_SUPPORTED	1.1f

/*****
 *
 * Name: vtTowerArray
 * Parent: Array<vtTowerPtr>
 *
 * Description:
 *	Array of Towers
 *
 *****/
class vtTowerArray : public Array<vtTowerPtr>
{
public:
	~vtTowerArray() { Empty(); }
	virtual void DestructItems(int first, int last);	// override

	void AddTower(vtTowerPtr bld) { Append(bld); }
	void Remove(int index, DPoint2 &thepoint)	// remove a specific coordinate
	{
		// grab the point out of the array
		thepoint = GetAt(index)->GetLocation();
		RemoveAt(index, 1);
	}
	void DeleteSelected();

	bool ReadSHP(const char* pathname);			// all of this will need to pull from the object model
	bool ReadUTL(const char* pathname);			// read a .utl file
	bool WriteSHP(const char* pathname);
	bool WriteUTL(const char* pathname);		// write to a .utl file

	int IsTowerThere(const DPoint2 &point, double error);//checks to see if a coordinate (within
													//the given spatial error) is in the array
	bool FindClosestCorner(const DPoint2 &point, double error,
						   int &building, int &corner, double &distance);
	bool FindClosestCenter(const DPoint2 &point, double error,
						   int &building, double &distance);

	bool IsEmpty() { return (GetSize() == 0); }
	void GetExtents(DRECT &ext);

	// selection
	int NumSelected();
	void DeselectAll();

	// override this so that the TowerArray base methods can be capable
	// of handling subclasses of vtTower
	virtual vtTower *NewTower() { return new vtTower; }

	vtProjection m_proj;
};

#endif
