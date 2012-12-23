//
// Structure.h
//
// Implements the vtStructure class which represents a single built structure.
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file Structure.h */


#ifndef STRUCTUREH
#define STRUCTUREH

#include "MathTypes.h"
#include "Selectable.h"
#include "Content.h"
#include "FilePath.h"

class vtBuilding;
class vtFence;
class vtStructInstance;

// Well known material names
#define BMAT_NAME_PLAIN			"Plain"
#define BMAT_NAME_WOOD			"Wood"
#define BMAT_NAME_SIDING		"Siding"
#define BMAT_NAME_BRICK			"Brick"
#define BMAT_NAME_PAINTED_BRICK	"Painted-Brick"
#define BMAT_NAME_ROLLED_ROOFING "Rolled-Roofing"
#define BMAT_NAME_CEMENT		"Cement"
#define BMAT_NAME_CORRUGATED	"Corrugated"
#define BMAT_NAME_DOOR			"Door"
#define BMAT_NAME_WINDOW		"Window"
#define BMAT_NAME_WINDOWWALL	"WindowWall"

enum vtMaterialColorEnum
{
	VT_MATERIAL_COLOUR,			// a single color
	VT_MATERIAL_COLOURABLE,		// any color, untextured
	VT_MATERIAL_SELFCOLOURED_TEXTURE,	// a single texture
	VT_MATERIAL_COLOURABLE_TEXTURE		// any color, textured
};

/**
 * This class encapsulates the description of a shared material
 */
class vtMaterialDescriptor
{
public:
	vtMaterialDescriptor();
	vtMaterialDescriptor(const char *name,
					const vtString &SourceName,
					const vtMaterialColorEnum Colorable = VT_MATERIAL_SELFCOLOURED_TEXTURE,
					const float fUVScaleX=-1,
					const float fUVScaleY=-1,
					const bool bTwoSided = false,
					const bool bAmbient = false,
					const bool bBlended = false,
					const RGBi &Color = RGBi(0,0,0));
	~vtMaterialDescriptor();

	void SetName(const vtString& Name)
	{
		m_pName = &Name;
	}
	const vtString& GetName() const
	{
		return *m_pName;
	}
	/**
	\param type One of:
	- 0: A surface material, such as brick, siding, or stucco.
	- 1: An element of a strcture edge, such as a door or window.
	- 2: Reserved for "Window Wall", an efficiency optimization material
		which contains both a window and a wall.
	- 3: A post material, for linear structures, such as a fencepost.
	*/
	void SetMatType(int type)
	{
		m_Type = type;
	}
	int GetMatType() const
	{
		return m_Type;
	}
	// UV Scale: texture units per meter, or -1 to scale to fit
	void SetUVScale(const float fScaleX, const float fScaleY)
	{
		m_UVScale.Set(fScaleX, fScaleY);
	}
	FPoint2 GetUVScale() const
	{
		return m_UVScale;
	}
	void SetMaterialIndex(const int Index)
	{
		m_iMaterialIndex = Index;
	}
	const int GetMaterialIndex() const
	{
		return m_iMaterialIndex;
	}
	void SetColorable(const vtMaterialColorEnum Type)
	{
		m_Colorable = Type;
	}
	const vtMaterialColorEnum GetColorable() const
	{
		return m_Colorable;
	}
	void SetSourceName(const vtString &SourceName)
	{
		m_SourceName = SourceName;
	}
	const vtString& GetSourceName() const
	{
		return m_SourceName;
	}
	void SetRGB(const RGBi Color)
	{
		m_RGB = Color;
	}
	const RGBi GetRGB() const
	{
		return m_RGB;
	}
	void SetTwoSided(bool bTwoSided)
	{
		m_bTwoSided = bTwoSided;
	}
	const bool GetTwoSided()
	{
		return m_bTwoSided;
	}
	void SetAmbient(bool bAmbient)
	{
		m_bAmbient = bAmbient;
	}
	const bool GetAmbient()
	{
		return m_bAmbient;
	}
	void SetBlending(bool bBlending)
	{
		m_bBlending = bBlending;
	}
	const bool GetBlending()
	{
		return m_bBlending;
	}
	// Operator  overloads
	bool operator == (const vtMaterialDescriptor& rhs) const
	{
		return (*m_pName == *rhs.m_pName);
	}
	bool operator == (const vtMaterialDescriptor& rhs)
	{
		return (*m_pName == *rhs.m_pName);
	}
	void WriteToFile(FILE *fp);

private:
	const vtString *m_pName; // Name of material
	int m_Type;				// 0 for surface materials, >0 for classification type
	vtMaterialColorEnum m_Colorable;
	vtString m_SourceName;	// Source of material
	FPoint2 m_UVScale;		// Texel scale;
	RGBi m_RGB;				// Color for VT_MATERIAL_COLOUR
	bool m_bTwoSided;		// default false
	bool m_bAmbient;		// default false
	bool m_bBlending;		// default false

	// The following field is only used in 3d construction, but it's not
	//  enough distinction to warrant creating a subclass to contain it.
	int m_iMaterialIndex; // Starting or only index of this material in the shared materials array
};

/**
 * Contains a set of material descriptors.
 */
class vtMaterialDescriptorArray : public vtArray<vtMaterialDescriptor*>
{
public:
	virtual ~vtMaterialDescriptorArray() { Clear(); free(m_Data); m_Data = NULL; m_MaxSize = 0; }
	void DestructItems(uint first, uint last)
	{
		for (uint i = first; i <= last; i++)
			delete GetAt(i);
	}
	bool LoadExternalMaterials();
	const vtString *FindName(const char *matname) const;
	void CreatePlain();

	bool Load(const char *szFileName);
	bool Save(const char *szFileName);
};

/**
 * Structure type.
 */
enum vtStructureType
{
	ST_BUILDING,	/**< A Building (vtBuilding) */
	ST_LINEAR,		/**< A Linear (vtFence) */
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
 * parametrically, vtBuilding and vtFence provide efficient data
 * representation.  For other structures which are not easily reduced to
 * parameters, the Instance type allows you to reference any external model,
 * such as a unique building which has been created in a 3D Modelling Tool.
 */
class vtStructure : public Selectable, public vtTagArray
{
public:
	vtStructure();
	virtual ~vtStructure();

	// use an explicit method to avoid assignment operator
	void CopyFrom(const vtStructure &v);

	void SetType(vtStructureType t) { m_type = t; }
	vtStructureType GetType() const { return m_type; }

	void SetElevationOffset(float fOffset) { m_fElevationOffset = fOffset; }
	float GetElevationOffset() const { return m_fElevationOffset; }
	void SetAbsolute(bool b) { m_bAbsolute = b; }
	bool GetAbsolute() const { return m_bAbsolute; }

	vtBuilding *GetBuilding() { if (m_type == ST_BUILDING) return (vtBuilding *)this; else return NULL; }
	vtFence *GetFence() { if (m_type == ST_LINEAR) return (vtFence *)this; else return NULL; }
	vtStructInstance *GetInstance() { if (m_type == ST_INSTANCE) return (vtStructInstance *)this; else return NULL; }

	virtual bool GetExtents(DRECT &rect) const = 0;
	virtual bool IsContainedBy(const DRECT &rect) const = 0;
	virtual void WriteXML(GZOutput &out, bool bDegrees) const = 0;

	void WriteTags(GZOutput &out) const;

#ifdef VIAVTDATA
	bool m_bIsVIAContributor;
	bool m_bIsVIATarget;
#endif

protected:
	vtStructureType m_type;

	// Offset that the structure should be moved up or down relative to its
	// default position on the ground
	// for buildings this is (lowest corner of its base footprint)
	// for linear features this is the lowest point of the feature.
	// for instances this is the datum point
	float m_fElevationOffset;

	// If true, elevation offset is relative to sealevel, not to the
	//  heightfield surface.
	bool m_bAbsolute;

private:
	// Don't let unsuspecting users stumble into assuming that object
	// copy semantics will work.  Declare them private and never
	// define them:
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
 *	 itemname="stopsign"
 */
class vtStructInstance : public vtStructure
{
public:
	vtStructInstance();

	// copy operator
	vtStructInstance &operator=(const vtStructInstance &v);

	void WriteXML(GZOutput &out, bool bDegrees) const;
	void Offset(const DPoint2 &delta) { m_p += delta; }

	bool GetExtents(DRECT &rect) const;
	bool IsContainedBy(const DRECT &rect) const;

	virtual double DistanceToPoint(const DPoint2 &p, float fMaxRadius) const;

	void SetPoint(const DPoint2 &p) { m_p = p; }
	DPoint2 GetPoint() const { return m_p; }
	void SetRotation(float f) { m_fRotation = f; }
	float GetRotation() const { return m_fRotation; }
	void SetScale(float f) { m_fScale = f; }
	float GetScale() const { return m_fScale; }
	void SetItem(vtItem *pItem) { m_pItem = pItem; }
	vtItem *GetItem() { return m_pItem; }

protected:
	DPoint2	m_p;			// earth position
	float	m_fRotation;	// in radians
	float	m_fScale;		// meters per unit

	vtItem *m_pItem;	// If this is an instance of a content item
};

bool LoadGlobalMaterials();
void SetGlobalMaterials(vtMaterialDescriptorArray *mats);
vtMaterialDescriptorArray *GetGlobalMaterials();
void FreeGlobalMaterials();

#endif // STRUCTUREH

