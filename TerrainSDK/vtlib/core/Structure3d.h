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

/**
 * This class extends vtStructInstance with the ability to construct and
 * manage 3d representations of the instance.
 */
class vtStructInstance3d : public vtStructInstance
{
public:
	vtStructInstance3d();

	/// Create the node(s) and position them on the indicated heightfield
	bool CreateShape(vtHeightField *pHeightField);

	/// Access the Transform node for this structure
	vtTransform *GetTransform() { return m_pContainer; }

	/// (Re-)position the instance on the indicated heightfield
	void UpdateTransform(vtHeightField *pHeightField);

	/// Pass true to turn on a wireframe hightlight geometry for this instance
	void ShowBounds(bool bShow);

protected:
	vtTransform *m_pContainer;
	vtGeom		*m_pHighlight;	// The wireframe highlight
	vtNode		*m_pModel; // the contained model
};

/**
 * This class extends vtStructure with the ability to construct and
 * manage 3d representations of the structure.
 */
class vtStructure3d : public vtStructure
{
public:
	vtStructure3d();

	/// Create the node(s) and position them on the indicated heightfield
	bool CreateNode(vtHeightField *hf, const char *options = "");

	/// Access the Transform node for this structure, if it has one
	vtTransform *GetTransform();

	/// Access the Geometry node for this structure, if it has one
	vtGeom *GetGeom();
	void DeleteNode();

	vtBuilding3d *GetBuilding();
	vtFence3d *GetFence();
	vtStructInstance3d *GetInstance();

protected:
	vtTransform *m_pNode;
};

/**
 * This class extends vtStructureArray with the ability to construct and
 * manage 3d representations of the structures.
 */
class vtStructureArray3d : public vtStructureArray
{
public:
	virtual vtStructure *NewStructure() { return new vtStructure3d; }
	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	vtStructure3d *GetStructure(int i) { return (vtStructure3d *) GetAt(i); }
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

	/// Select a single structure, and visually highlight it
	void VisualSelect(vtStructure3d *str);

protected:
	vtHeightField *m_pHeightField;
};

#endif // STRUCTURE3DH
