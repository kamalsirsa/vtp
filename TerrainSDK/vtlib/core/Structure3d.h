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


#include <ostream>

#include "vtdata/StructArray.h"

#define COLOR_SPREAD	216		// 216 color variations

class vtBuilding3d;
class vtFence3d;
class vtNode;
class vtTerrain;
class vtTransform;

extern const vtMaterialName BMAT_NAME_HIGHLIGHT;

enum vtMaterialTypeEnum
{
	VT_MATERIAL_RGB,
	VT_MATERIAL_COLOURED,
	VT_MATERIAL_SELFCOLOURED_TEXTURE,
	VT_MATERIAL_COLOURABLE_TEXTURE,
	VT_MATERIAL_BRICK,
	VT_MATERIAL_HIGHLIGHT
};

/**
 * This class encapsulates the description of a shared material
 */
class vtMaterialDescriptor
{
public:
	vtMaterialDescriptor()
	{
		m_pName = NULL;
		m_Type = VT_MATERIAL_SELFCOLOURED_TEXTURE;
		m_fUVScale = 0.0;
		m_RGB = RGBi(0,0,0);
	}
	vtMaterialDescriptor(vtMaterialName& Name,
					const vtString& SourceName,
					const vtMaterialTypeEnum Type = VT_MATERIAL_SELFCOLOURED_TEXTURE,
					const float UVScale = 0.0,
					bool bUIVisible = true,
					RGBi Color = RGBi(0,0,0))
	{
		m_pName = &Name;
		m_SourceName = SourceName;
		m_Type = Type;
		m_fUVScale = UVScale;
		if (NULL != m_pName)
			m_pName->SetUIVisible(bUIVisible);
		m_RGB = Color;
	}
	inline void SetName(vtMaterialName& Name)
	{
		m_pName = &Name;
	}
	inline const vtMaterialName& GetName() const
	{
		return *m_pName;
	}
	inline void SetUVScale(const float fScale)
	{
		m_fUVScale = fScale;
	}
	inline const float GetUVScale() const
	{
		return m_fUVScale;
	}
	inline void SetMaterialIndex(const int Index)
	{
		m_iMaterialIndex = Index;
	}
	inline const int GetMaterialIndex() const
	{
		return m_iMaterialIndex;
	}
	inline void SetMaterialType(const vtMaterialTypeEnum Type)
	{
		m_Type = Type;
	}
	inline const vtMaterialTypeEnum GetMaterialType() const
	{
		return m_Type;
	}
	inline void SetSourceName(const vtString &SourceName)
	{
		m_SourceName = SourceName;
	}
	inline const vtString& GetSourceName() const
	{
		return m_SourceName;
	}
	inline void SetUIVisible(const bool bUIVisible)
	{
		if (NULL != m_pName)
			m_pName->SetUIVisible(bUIVisible);
	}
	inline const bool GetUIVisible() const
	{
		if (NULL != m_pName)
			return m_pName->GetUIVisible();
		else
			return false;
	}
	inline void SetRGB(const RGBi Color)
	{
		m_RGB = Color;
	}
	inline const RGBi GetRGB() const
	{
		return m_RGB;
	}
	// Operator  overloads
	inline bool operator == (const vtMaterialDescriptor& rhs) const
	{
		return(*this->m_pName == *rhs.m_pName);
	}
	inline bool operator == (const vtMaterialDescriptor& rhs)
	{
		return(*this->m_pName == *rhs.m_pName);
	}
	friend inline std::ostream &operator << (std::ostream & Output, const vtMaterialDescriptor &Input)
	{
		const RGBi &rgb = Input.m_RGB;
		Output << "\t<MaterialDescriptor Name=\""<< (pcchar)*Input.m_pName << "\""
				<< " Type=\"" << Input.m_Type << "\""
				<< " Source=\"" << (pcchar)Input.m_SourceName << "\""
				<< " Scale=\"" << Input.m_fUVScale << "\""
				<< " UIVisible=\"" << Input.m_pName->GetUIVisible() << "\""
				<< " RGB=\"" << rgb.r << " " << rgb.g << " " << rgb.b << "\""
				<< "/>" << std::endl;
		return Output;
	}
private:
	vtMaterialName *m_pName; // Name of material
	vtMaterialTypeEnum m_Type;
	vtString m_SourceName; // Source of material
	float m_fUVScale; // Texel scale;
	RGBi m_RGB; // Color for VT_MATERIAL_RGB

	int m_iMaterialIndex; // Starting or only index of this material in the shared materials array
};

class vtMaterialDescriptorArray : public Array<vtMaterialDescriptor*>
{
public:
	virtual ~vtMaterialDescriptorArray() { Empty(); free(m_Data); m_Data = NULL; m_MaxSize = 0; }
	inline void DestructItems(int first, int last)
	{
		for (int i = first; i <= last; i++)
			delete GetAt(i);
	}

	friend inline std::ostream &operator << (std::ostream & Output, vtMaterialDescriptorArray &Input)
	{
		int iSize = Input.GetSize();
		Output << "<?xml version=\"1.0\"?>" << std::endl;
		Output << "<MaterialDescriptorArray>" << std::endl;
		for (int i = 0; i < iSize; i++)
			Output << *Input.GetAt(i);
		Output << "</MaterialDescriptorArray>" << std::endl;
		return Output;
	}
	bool Load(const char *FileName);
};

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
	virtual bool CreateNode(vtTerrain *pTerr) { return false; }

	/// Access the Geometry node for this structure, if it has one
	virtual vtGeom *GetGeom() { return NULL; }
	virtual void DeleteNode() {}

	/// Pass true to turn on a wireframe hightlight geometry for this instance
	virtual void ShowBounds(bool bShow) {}

	// Get the material descriptors
	const vtMaterialDescriptorArray& GetMaterialDescriptors()
	{
		if (s_pMaterials == NULL)
			InitializeMaterialArrays();
		return s_MaterialDescriptors;
	}

	static void ReleaseSharedMaterials();

protected:
	// material
	void InitializeMaterialArrays();
	int FindMatIndex(const vtMaterialName & Material, RGBi inputColor=RGBi(0,0,0));
	vtMaterialDescriptor * FindMaterialDescriptor(const vtMaterialName& MaterialName);
	vtMaterialArray *GetSharedMaterialArray() const
	{
		return s_pMaterials;
	}
	vtTransform	*m_pContainer;	// The transform which is used to position the object

private:
	vtMaterial *MakeMaterial(RGBf &color, bool culling);
	float ColorDiff(const RGBi &c1, const RGBi &c2);
	static RGBf s_Colors[COLOR_SPREAD];
	static vtMaterialArray *s_pMaterials;
	static vtMaterialDescriptorArray s_MaterialDescriptors;
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
	virtual bool CreateNode(vtTerrain *pTerr);
	virtual void ShowBounds(bool bShow);

	/// (Re-)position the instance on the indicated heightfield
	void UpdateTransform(vtHeightField3d *pHeightField);

protected:
	vtGeom		*m_pHighlight;	// The wireframe highlight
	vtNodeBase	*m_pModel; // the contained model
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
	void SetTerrain(vtTerrain *pTerr) { m_pTerrain = pTerr; }

	/// Construct an individual structure, return true if successful
	bool ConstructStructure(vtStructure3d *str);
	bool ConstructStructure(int index);
	void OffsetSelectedStructures(const DPoint2 &offset);

	/// Deselect all structures including turning off their visual highlights
	void VisualDeselectAll();

	// Be informed of edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

	// Be informed when a structure is deleted
	virtual void DestroyStructure(int i);

protected:
	vtTerrain *m_pTerrain;
};

#endif // STRUCTURE3DH
