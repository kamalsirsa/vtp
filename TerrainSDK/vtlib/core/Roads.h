//
// Roads.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINROADSH
#define TERRAINROADSH

#include "vtdata/FilePath.h"
#include "vtdata/RoadMap.h"
#include "vtdata/HeightField.h"
#include "vtlib/core/LodGrid.h"

#define ROAD_CLUSTER	16

//
// A place where 2 or more roads meet
//
class NodeGeom : public Node
{
public:
	NodeGeom();
	~NodeGeom();

	class RoadGeom *GetRoad(int n) { return (class RoadGeom *)m_r[n]; }
	void BuildIntersection();
	void FindVerticesForRoad(Road *pR, FPoint3 &p0, FPoint3 &p1);
	vtMesh *GenerateGeometry();
	FPoint3 GetRoadVector(int i);
	FPoint3 GetUnitRoadVector(int i);

	int m_iVerts;
	FPoint3 *m_v;

	FPoint3 m_p3;
};


class Lane
{
public:
	FPoint3 *m_p3;
};

class RoadBuildInfo {
public:
	RoadBuildInfo(int iCoords);
	~RoadBuildInfo();
	FPoint3 *left;
	FPoint3 *right;
	FPoint3 *center;
	FPoint3 *crossvector;
	float *fvLength;

	int verts;
	int vert_index;
};

enum normal_direction {
	ND_UP,
	ND_LEFT,
	ND_RIGHT
};

class VirtualTexture
{
public:
	int		m_idx;	// material index
	FRECT	m_rect;	// region of the base texture to use

	void Adapt(const FPoint2 &in, FPoint2 &out)
	{
		out.x = m_rect.left + in.x * (m_rect.right - m_rect.left);
		out.y = m_rect.bottom + in.y * (m_rect.top - m_rect.bottom);
	}
};

//
// virtual texture indices
//
enum RoadVTIndices
{
	VTI_MARGIN,
	VTI_SIDEWALK,
	VTI_1LANE,
	VTI_2LANE1WAY,
	VTI_2LANE2WAY,
	VTI_3LANE1WAY,
	VTI_3LANE2WAY,
	VTI_4LANE1WAY,
	VTI_4LANE2WAY,
	VTI_TOTAL
};

//
// a series of points, connecting one node to another
//
class RoadGeom : public Road
{
public:
	RoadGeom();
	~RoadGeom();

	FPoint3 FindPointAlongRoad(float fDistance);
	float	Length();	// find 2D length in world units

	// road-construction methods
	void SetupBuildInfo(RoadBuildInfo &bi);
	void AddRoadStrip(vtMesh *pGeom, RoadBuildInfo &bi,
					float offset_left, float offset_right,
					float height_left, float height_right,
					VirtualTexture &vt,
					float u1, float u2, float uv_scale,
					normal_direction nd);
	void GenerateGeometry(class vtRoadMap3d *rmgeom);

	NodeGeom *GetNode(int n) { return (NodeGeom *)m_pNode[n]; }
	RoadGeom *GetNext() { return (RoadGeom *)m_pNext; }

	int m_vti;
	FPoint3 *m_p3;
	Lane *m_pLanes;
};


class vtRoadMap3d : public vtRoadMap
{
public:
	vtRoadMap3d();
	~vtRoadMap3d();

	// overrides for virtual methods
	RoadGeom	*GetFirstRoad() { return (RoadGeom *) m_pFirstRoad; }
	NodeGeom	*GetFirstNode() { return (NodeGeom *) m_pFirstNode; }
	Node		*NewNode() { return new NodeGeom; }
	Road		*NewRoad() { return new RoadGeom; }

	void DrapeOnTerrain(vtHeightField *pHeightField);
	void BuildIntersections();
	void AddMesh(vtMesh *pMesh, int iMatIdx);
	void GatherExtents(FPoint3 &cluster_min, FPoint3 &cluster_max);
	vtGroup *GenerateGeometry(bool do_texture, const StringArray &paths);
	void GenerateSigns(vtLodGrid *pLodGrid);
	vtGroup *GetGroup() { return m_pGroup; }
	void SetLodDistance(float fDistance) { m_fLodDistance = fDistance; }
	void SetHeightOffGround(float fHeight) { s_fHeight = fHeight; }
	void DetermineSurfaceAppearance();

public:
	static float s_fHeight;

	// virtual textures
	VirtualTexture	m_vt[VTI_TOTAL];

	// material indices
	int		m_mi_roadside;
	int		m_mi_roads;
	int		m_mi_4wd;
	int		m_mi_trail;
	int		m_mi_red;

protected:
	vtGroup	*m_pGroup;
	vtMaterialArray *m_pMats;

	vtLOD		*m_pRoads[ROAD_CLUSTER][ROAD_CLUSTER];
	FPoint3		m_cluster_min;
	FPoint3		m_cluster_max;
	FPoint3		m_cluster_range;
	float		m_fLodDistance;		// in meters
};

// Useful typedefs
typedef NodeGeom *NodeGeomPtr;
typedef RoadGeom *RoadGeomPtr;

#endif

