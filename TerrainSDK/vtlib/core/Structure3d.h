//
// Structure3d.h
//
// Implements the vtStructure3d class which extends vtStructure with the
// ability to create 3d geometry.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTURE3DH
#define STRUCTURE3DH

#include "vtdata/StructArray.h"

class vtBuilding3d;
class vtFence3d;
class vtNode;
class vtHeightField;

class vtStructInstance3d : public vtStructInstance
{
public:
	vtStructInstance3d();

	bool CreateShape(vtHeightField *pHeightField);
	vtTransform *GetTransform() { return m_pTransform; }

protected:
	vtTransform *m_pTransform;
};

class vtStructure3d : public vtStructure
{
public:
	vtStructure3d();

	bool CreateNode(vtHeightField *hf, const char *options = "");
	vtTransform *GetTransform();
	vtGeom *GetGeom();
	void DeleteNode();

protected:
	vtTransform *m_pNode;
};

class vtStructureArray3d : public vtStructureArray
{
public:
	virtual vtStructure *NewStructure() { return new vtStructure3d; }
	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	vtBuilding3d *GetBuilding(int i) { return (vtBuilding3d *) GetAt(i)->GetBuilding(); }
	vtFence3d *GetFence(int i) { return (vtFence3d *) GetAt(i)->GetFence(); }

	void SetHeightField(vtHeightField *hf) { m_pHeightField = hf; }

	bool ConstructStructure(vtStructure3d *str, const char *options = "");
	void ReConstructStructure(vtStructure3d *str, const char *options);
	void OffsetSelectedBuildings(DPoint2 &offset);

protected:
	vtHeightField *m_pHeightField;
};

#endif // STRUCTURE3DH
