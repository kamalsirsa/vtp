//
// Structure3d.h
//
// Implements the vtStructure3d class which extends vtStructure with the
// ability to create 3D geometry.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTURE3DH
#define STRUCTURE3DH

/** \addtogroup struct */
/*@{*/

#include "vtdata/StructArray.h"

#define COLOR_SPREAD	216		// 216 color variations

class vtBuilding3d;
class vtFence3d;
class vtTerrain;
class vtTransform;

extern const vtString BMAT_NAME_HIGHLIGHT;

/**
 * This class extents vtMaterialDescriptorArray with the ability to construct
 * real vtlib Materials.  You won't need to use this class directly, unless
 * you wish to use the shared pool of materials used by the vtBuilding3d
 * class.
 */
class vtMaterialDescriptorArray3d : public vtMaterialDescriptorArray
{
public:
	vtMaterialDescriptorArray3d();

	vtMaterial *MakeMaterial(vtMaterialDescriptor *desc, const RGBf &color);
	int FindMatIndex(const vtString & Material, const RGBf &inputColor = RGBf(),
		int iType = -1);
	vtMaterialDescriptor *FindMaterialDescriptor(const vtString& MaterialName,
		const RGBf &color = RGBf(), int iType = -1) const;
	void InitializeMaterials();
	void CreateMaterials();
	vtMaterialArray *GetMatArray() const { return m_pMaterials; };

protected:
	// There is a single array of materials, shared by all buildings.
	// This is done to save memory.  For a list of 16000+ buildings, this can
	//  save about 200MB of RAM.
	vtMaterialArrayPtr m_pMaterials;

	bool m_bMaterialsCreated;

	void CreateSelfColoredMaterial(vtMaterialDescriptor *descriptor);
	void CreateColorableTextureMaterial(vtMaterialDescriptor *descriptor);

	RGBf m_Colors[COLOR_SPREAD];

	// indices of internal materials
	int m_hightlight1, m_hightlight2, m_hightlight3, m_wire;
};


/**
 * This class contains the extra methods needed by a vtStructure to
 * support the ability to construct and manage a 3D representation.
 */
class vtStructure3d
{
public:
	vtStructure3d() { m_pContainer = NULL; m_bIsVIAContributor = false; m_bIsVIATarget = false; }

	vtTransform *GetContainer() { return m_pContainer; }
	virtual osg::Node *GetContained() = 0;

	/// Create the node(s) and position them on the indicated heightfield
	virtual bool CreateNode(vtTerrain *pTerr) = 0;
	virtual bool IsCreated() = 0;

	/// Access the Geometry node for this structure, if it has one
	virtual vtGeode *GetGeom() { return NULL; }
	virtual void DeleteNode() = 0;

	/// Pass true to turn on a wireframe hightlight geometry for this instance
	virtual void ShowBounds(bool bShow) {}

	void SetCastShadow(bool b);
	bool GetCastShadow();

	// Get the material descriptors
	static vtMaterialDescriptorArray3d& GetMaterialDescriptors()
	{
		return s_MaterialDescriptors;
	}

	static void InitializeMaterialArrays();

	// all fences share the same set of materials
	static void CreateSharedMaterials();

	// Visual Impact
	const bool GetVIAContributor() const { return m_bIsVIAContributor; }
	const bool GetVIATarget() const { return m_bIsVIATarget; }
	void SetVIAContributor(const bool bVIAContributor) { m_bIsVIAContributor = bVIAContributor; }
	void SetVIATarget(const bool bVIATarget) { m_bIsVIATarget = bVIATarget; }

protected:
	// material
	int FindMatIndex(const vtString &Material, const RGBf &inputColor = RGBf(), int iType = -1)
	{
		return s_MaterialDescriptors.FindMatIndex(Material, inputColor, iType);
	}
	vtMaterialArray *GetSharedMaterialArray() const
	{
		return s_MaterialDescriptors.GetMatArray();
	}
	vtTransformPtr m_pContainer;	// The transform which is used to position the object

protected:
	float ColorDiff(const RGBi &c1, const RGBi &c2);

	static vtMaterialDescriptorArray3d s_MaterialDescriptors;
	static bool s_bMaterialsLoaded;

	// Visual Impact
    bool m_bIsVIAContributor;
	bool m_bIsVIATarget;

};


/**
 * This class extends vtStructInstance with the ability to construct and
 * manage 3D representations of the instance.
 */
class vtStructInstance3d : public vtStructInstance, public vtStructure3d
{
public:
	vtStructInstance3d();

	// implement vtStructInstance methods
	virtual double DistanceToPoint(const DPoint2 &p, float fMaxRadius) const;

	// implement vtStructure3d methods
	/// Create the node(s) and position them on the indicated heightfield
	virtual bool CreateNode(vtTerrain *pTerr);
	virtual bool IsCreated();
	virtual void DeleteNode();
	virtual void ShowBounds(bool bShow);
	virtual osg::Node *GetContained() { return m_pModel.get(); }

	/// (Re-)position the instance on the indicated heightfield
	void UpdateTransform(vtHeightField3d *pHeightField);

	/// Attempt to reload from disk
	void Reload();

protected:
	vtGeode		*m_pHighlight;	// The wireframe highlight
	osg::ref_ptr<osg::Node> m_pModel; // the contained model
};


/**
 * This class extends vtStructureArray with the ability to construct and
 * manage 3D representations of the structures.
 */
class vtStructureArray3d : public vtStructureArray, public vtEnabledBase
{
public:
	vtStructureArray3d();

	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	vtStructure3d *GetStructure3d(int i);
	vtBuilding3d *GetBuilding(int i) { return (vtBuilding3d *) GetAt(i)->GetBuilding(); }
	vtFence3d *GetFence(int i) { return (vtFence3d *) GetAt(i)->GetFence(); }
	vtStructInstance3d *GetInstance(int i) { return (vtStructInstance3d *) GetAt(i)->GetInstance(); }

	/// Indicate the heightfield which will be used for the structures in this array
	void SetTerrain(vtTerrain *pTerr) { m_pTerrain = pTerr; }

	/// Construct an individual structure, return true if successful
	bool ConstructStructure(vtStructure3d *str);
	bool ConstructStructure(int index);
	void OffsetSelectedStructures(const DPoint2 &offset);
	void OffsetSelectedStructuresVertical(float offset);

	/// Deselect all structures including turning off their visual highlights
	void VisualDeselectAll();

	/// Enable (set visibility of) the structures
	void SetEnabled(bool bTrue);

	/// Set shadows on/off for all structures
	void SetShadows(bool bTrue);

	// Be informed of edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

	// Be informed when a structure is deleted
	virtual void DestroyStructure(int i);

protected:
	vtTerrain *m_pTerrain;
};

/*@}*/	// Group struct

#endif // STRUCTURE3DH

