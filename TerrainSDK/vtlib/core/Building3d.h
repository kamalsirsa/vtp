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

class vtHeightField;

enum BuildingMesh {
	BM_WALL,
	BM_TRIM,
	BM_DOOR,
	BM_WINDOW,
	BM_ROOF,
	BM_TOTAL
};

enum BuildingMaterial
{
	BAP_PLAIN, BAP_SIDING, BAP_ROOF, BAP_WINDOW, BAP_DOOR 
};

class vtBuilding3d : public vtBuilding
{
public:
	vtBuilding3d();
	~vtBuilding3d();

	// copy
	vtBuilding3d &operator=(const vtBuilding &v);

	//looks up materials for the building
	void FindMaterialIndices();

	void CreateShape(vtHeightField *pHeightField, bool roof=true,
		bool walls=true, bool details=true);

	void DestroyGeometry();
	void CreateGeometry(vtHeightField *pHeightField, bool bDoRoof,
						bool bDoWalls, bool details);
	vtTransform *GetTransform();
	void AdjustHeight(vtHeightField *pHeightField);

	// randomize building properties
	void Randomize(int iStories);

	// display a bounding box around to object to highlight it
	void ShowBounds(bool bShow);

protected:
	int FindMatIndex(BuildingMaterial bldApp, RGBi inputColor=RGBi(0,0,0));
	void CreateSharedMaterials();

protected:
	// material
	static vtMaterialArray *s_Materials;
	int m_iMatIdx[BM_TOTAL];

	// center of the building in world coordinates (the origin of
	// the building's local coordinate system)
	FPoint3 m_center;

	// internal methods
	void DetermineBaseCorners(vtHeightField *pHeightField, FLine3 &base_corner);
	float GetHeightOfStories();

	void CreateWallGeometry(Array<FPoint3> &corners, vtLevel *pLev,
		float fBase, int iWall, bool details);

	// creates a wall.  base_height is height from base of floor
	// (to make siding texture match up right.)
	void AddWallSection(BuildingMesh bm, FPoint3 &p0, FPoint3 &p1,
		float height, float base_height);
	//adds a wall section with a door
	void AddDoorSection(FPoint3 &p0, FPoint3 &p1, float height);
	//adds a wall section with a window
	void AddWindowSection(FPoint3 p0, FPoint3 p1, float height);
	void AddFlatRoof(Array<FPoint3> &pp, float height);
	void AddShedRoof(Array<FPoint3> &pp, float height);
	/*	Top view:
		Hip Roof:			Gable Roof:
		_______				_______
		|\   /|				|  |  |
		| \ / |				|  |  |
		|  |  |				|  |  |
		|  |  |				|  |  |
		| / \ |				|  |  |
		|/___\|				|__|__|
		Side view:
		  ______			___________
		 /      \           |         |
		/________\			|_________| 
		(triangular from the other side.)
	*/
	void	AddHipRoof(Array<FPoint3> &pp, float height);
	void	AddGableRoof(Array<FPoint3> &pp, float height);
	void	BuildRoofPanel(FPoint3 *v, int n, ...);
	FPoint3	Normal(FPoint3 &p0, FPoint3 &p1, FPoint3 &p2);
	//normal slightly tilted up.
	FPoint3	WallNormal(FPoint3 &p0, FPoint3 &p1, FPoint3 &p2);

	vtTransform	*m_pContainer;	// The transform which is used to position the building
	vtGeom		*m_pGeom;		// The geometry node which contains the building geometry
	vtGeom		*m_pHighlight;	// The wireframe highlight

	// the geometry is composed of several meshes, one for each potential material used
	vtMesh		*m_pMesh[BM_TOTAL];
};

#endif
