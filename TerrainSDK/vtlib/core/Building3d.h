//
// Building3d.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BUILDING3DH
#define BUILDING3DH

#include "vtdata/Building.h"
#include "vtdata/StructArray.h"
#include "Structure3d.h"

class vtHeightField;

#define COLOR_SPREAD	216		// 216 color variations

typedef Array<FLine3 *> Footprints3d;
// Overrides DestructItems to call constructors
inline void Footprints3d::DestructItems(int first, int last)
{
	for (int i = first; i <= last; ++i)
	{
		FLine3 *pFL = GetAt(i);
		delete pFL;
	}
}

struct MatMesh
{
	int		m_iMatIdx;
	vtMesh	*m_pMesh;
	int		m_iPrimType;
};

class vtBuilding3d : public vtBuilding, public vtStructure3d
{
public:
	vtBuilding3d();
	~vtBuilding3d();

	// implement vtStructure3d methods
	virtual bool CreateNode(vtHeightField *hf, const vtTagArray &options);
	vtGeom *GetGeom();
	virtual void DeleteNode();
	// display a bounding box around to object to highlight it
	virtual void ShowBounds(bool bShow);

	// copy
	vtBuilding3d &operator=(const vtBuilding &v);

	//looks up materials for the building
	void FindMaterialIndices();

	void DestroyGeometry();
	void CreateGeometry(vtHeightField *pHeightField);
	void AdjustHeight(vtHeightField *pHeightField);

	// randomize building properties
	void Randomize(int iStories);

protected:
	int FindMatIndex(BldMaterial bldApp, RGBi inputColor=RGBi(0,0,0));
	void CreateSharedMaterials();

protected:
	// material
	static vtMaterialArray *s_Materials;
	static RGBf s_Colors[COLOR_SPREAD];

	// the geometry is composed of several meshes, one for each potential material used
	Array<MatMesh>	m_Mesh;

	vtMesh *FindMatMesh(BldMaterial bm, RGBi color, int iPrimType);

	// center of the building in world coordinates (the origin of
	// the building's local coordinate system)
	FPoint3 m_center;

	// Local-coordinate Footprints, one per level
	Footprints3d m_lfp;

	// internal methods
	void DetermineWorldFootprints(vtHeightField *pHeightField);
	float GetHeightOfStories();
	void CreateUpperPolygon(vtLevel *lev, FLine3 &poly, FLine3 &poly2);

	void CreateEdgeGeometry(vtLevel *pLev, FLine3 &poly1, FLine3 &poly2,
		int iEdge, bool bShowEdge);

	// create special, simple geometry for a level which is uniform
	void CreateUniformLevel(int iLevel, float fHeight);

	// creates a wall.  base_height is height from base of floor
	// (to make siding texture match up right.)
	void AddWallSection(vtEdge *pEdge, BldMaterial bmat,
		const FLine3 &quad, float h1, float h2, float hf1 = -1.0f);

	void AddHighlightSection(vtEdge *pEdge, const FLine3 &quad);

	//adds a wall section with a door
	void AddDoorSection(vtEdge *pWall, vtEdgeFeature *pFeat,
		const FLine3 &quad);

	//adds a wall section with a window
	void AddWindowSection(vtEdge *pWall, vtEdgeFeature *pFeat,
		const FLine3 &quad);

	void AddWallNormal(vtEdge *pWall, vtEdgeFeature *pFeat,
			const FLine3 &quad);

	void AddFlatRoof(FLine3 &pp, vtLevel *pLev);
	FPoint3	Normal(const FPoint3 &p0, const FPoint3 &p1, const FPoint3 &p2);

	vtGeom		*m_pGeom;		// The geometry node which contains the building geometry
	vtGeom		*m_pHighlight;	// The wireframe highlight
};

#endif
