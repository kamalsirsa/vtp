//
// Globe.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GLOBEH
#define GLOBEH

#include "vtdata/Icosa.h"
#include "vtdata/FilePath.h"
#include "vtlib/core/TimeEngines.h"

class vtTerrainScene;

struct IcoVert
{
	DPoint3 p;
	FPoint2 uv;
};

/**
 * IcoGlobe is an icosahedral globe.  To use it:
 *  - call Create() with the desired parameters to construct the object
 *  - call GetTop() to get the top node in the globe's scene graph
 *  - add that node to your scene graph
 *
 * The globe has unit radius.
 */
class IcoGlobe : public DymaxIcosa, public TimeTarget
{
public:
	IcoGlobe();

	enum Style
	{
		GEODESIC, RIGHT_TRIANGLE, DYMAX_UNFOLD
	};

	void Create(int iTriangleCount, const StringArray &paths,
		const vtString &strImagePrefix, Style style = GEODESIC);
	void SetInflation(float f);
	void SetLighting(bool bLight);
	void AddPoints(DLine2 &points, float fSize);

	void AddTerrainRectangles(vtTerrainScene *pTerrainScene);
	int AddGlobePoints(const char *fname);
	double AddSurfaceLineToMesh(vtMesh *mesh, const DPoint2 &g1, const DPoint2 &g2);
	vtTransform *GetTop() { return m_top; }

	void SetUnfolding(float f);
	void SetTime(time_t time);
	void SetCulling(bool bCull);
	void ShowAxis(bool bShow);
	void DoSeasonalTilt(bool bTilt) { m_bTilt = bTilt; }

protected:
	void CreateMaterials(const StringArray &paths, const vtString &strImagePrefix);
	void FindLocalOrigin(int mface);
	void SetMeshConnect(int mface);
	void EstimateTesselation(int iTriangleCount);
	void CreateUnfoldableDymax();
	void CreateNormalSphere();

	// these methods create a mesh for each face composed of strips
	void add_face1(vtMesh *mesh, int face, bool second);
	void set_face_verts1(vtMesh *geom, int face, float f);

	// these methods use a right-triangle recursion to create faces
	void add_face2(vtMesh *mesh, int face, int mface, int subfaces, bool second);
	void set_face_verts2(vtMesh *geom, int face, float f);
	void add_subface(vtMesh *mesh, int face, int v0, int v1, int v2,
								   bool flip, int depth);
	void refresh_face_positions(vtMesh *mesh, int mface, float f);

	IcoGlobe::Style m_style;

	// Common to all globe styles
	vtTransform	*m_top;
	vtGeom		*m_SurfaceGeom;
	vtGeom		*m_pAxisGeom;
	vtMaterialArray	*m_mats;
	int			m_globe_mat[10];
	int			m_red;
	int			m_yellow;
	int			m_white;
	vtMesh		*m_mesh[22];
	int			m_mfaces;	// either 20 or 22
	bool		m_bUnfolded;
	bool		m_bTilt;
	FQuat		m_Rotation;

	// for GEODESIC
	int		m_freq;		// tesselation frequency

	// for RIGHT_TRIANGLE
	int		m_vert;
	Array<IcoVert>	m_rtv[22];	// right-triangle vertices
	int		m_depth;	// tesselation depth

	// for DYMAX_UNFOLD
	vtTransform *m_xform[22];
	vtGeom *m_geom[22];
	FPoint3 m_local_origin[22];
	FPoint3 m_axis[22];

	FPoint3 m_flat_axis;
	float m_flat_angle;

	FQuat m_diff;
};

vtMovGeom *CreateSimpleEarth(vtString strDataPath);

bool FindIntersection(const FPoint3 &rkOrigin, const FPoint3 &rkDirection,
					  const FSphere& rkSphere,
					  int& riQuantity, FPoint3 akPoint[2]);

void geo_to_xyz(double radius, const DPoint2 &geo, FPoint3 &p);
void geo_to_xyz(double radius, const DPoint2 &geo, DPoint3 &p);
void xyz_to_geo(double radius, const FPoint3 &p, DPoint3 &geo);

#endif	// GLOBEH

