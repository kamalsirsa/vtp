//
// Structure3d.h
//
// Implements the vtStructure3d class which extends vtStructure with the
// ability to create 3d geometry.
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTURE3DH
#define STRUCTURE3DH

#include "vtdata/StructArray.h"

class vtBuilding3d;
class vtFence3d;
class vtNode;
class vtHeightField;


/**
 * This class contains the extra methods needed by a vtStructure to
 * support the ability to construct and manage a 3d representations.
 */
class vtStructure3d
{
public:
	vtStructure3d() { m_pContainer = NULL; }

	vtTransform *GetTransform() { return m_pContainer; }

	/// Create the node(s) and position them on the indicated heightfield
	virtual bool CreateNode(vtHeightField *hf, const char *options = "") { return false; }

	/// Access the Geometry node for this structure, if it has one
	virtual vtGeom *GetGeom() { return NULL; }
	virtual void DeleteNode() {}

	/// Pass true to turn on a wireframe hightlight geometry for this instance
	virtual void ShowBounds(bool bShow) {}

protected:
	vtTransform	*m_pContainer;	// The transform which is used to position the object
};


/**
 * This class extends vtStructInstance with the ability to construct and
 * manage 3d representations of the instance.
 */
class vtStructInstance3d : public vtStructInstance, public vtStructure3d
{
public:
	vtStructInstance3d();

	// implement vtStructure3d methods
	/// Create the node(s) and position them on the indicated heightfield
	virtual bool CreateNode(vtHeightField *hf, const char *options = "");
	virtual void ShowBounds(bool bShow);

	/// (Re-)position the instance on the indicated heightfield
	void UpdateTransform(vtHeightField *pHeightField);

protected:
	vtGeom		*m_pHighlight;	// The wireframe highlight
	vtNode		*m_pModel; // the contained model
};


/**
 * This class extends vtStructureArray with the ability to construct and
 * manage 3d representations of the structures.
 */
class vtStructureArray3d : public vtStructureArray
{
public:
	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	vtStructure3d *GetStructure3d(int i);
	vtBuilding3d *GetBuilding(int i) { return (vtBuilding3d *) GetAt(i)->GetBuilding(); }
	vtFence3d *GetFence(int i) { return (vtFence3d *) GetAt(i)->GetFence(); }
	vtStructInstance3d *GetInstance(int i) { return (vtStructInstance3d *) GetAt(i)->GetInstance(); }

	/// Indicate the heightfield which will be used for the structures in this array
	void SetHeightField(vtHeightField *hf) { m_pHeightField = hf; }

	/// Construct an individual structure, return true if successful
	bool ConstructStructure(vtStructure3d *str, const char *options = "");
	void ReConstructStructure(vtStructure3d *str, const char *options);
	void OffsetSelectedStructures(const DPoint2 &offset);

	/// Deselect all structures including turning off their visual highlights
	void VisualDeselectAll();

protected:
	vtHeightField *m_pHeightField;
};

#endif // STRUCTURE3DH
