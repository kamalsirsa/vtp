//
// Tower3d.h
//
// The vtTower3d class extends vtTower with the ability to procedurally
// create 3d geometry of the Tower.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TOWER3DH
#define TOWER3DH

#include "vtdata/Tower.h"
#include "vtdata/TowerArray.h"

class vtHeightField;


class vtTower3d: public vtTower
{
public:	

	vtTower3d();
	//~vtTower3d();
	//copy
	vtTower3d &operator=(const vtTower3d &vt);
	void ShowTwrBounds(bool show);
	// need to put the operations for 3d display here
protected:
	FPoint3 m_Center;
	//vtMesh	m_pTMesh[6];
	vtTransform	*m_pContainer;	// The transform which is used to position the building
	vtGeom		*m_pTowerGeom;		// The geometry node which contains the tower geometry
	vtGeom		*m_pTowerHighlight;	// The wireframe highlight
};

class vtTowerArray3d: public vtTowerArray
{
public:
	virtual vtTower *NewTower() { return (vtTower*) (new vtTower3d); }
	vtTower3d *GetTower(int i) { return (vtTower3d *) GetAt(i); }

//	void SetHeightField(vtHeightField *hf) { m_pHeightField = hf; }

	void OffsetSelected(DPoint2 &offset);

protected:
	vtHeightField *m_pTHeightField;
};

#endif

