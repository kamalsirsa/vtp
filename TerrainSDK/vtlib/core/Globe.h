//
// Globe.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GLOBEH
#define GLOBEH

#include "vtdata/Icosa.h"
#include "vtdata/Features.h"
#include "vtdata/FilePath.h"
#include "TimeEngines.h"
#include "GeomUtil.h"

class vtTerrainScene;

typedef Array<vtFeatureSet*> vtFeaturesSet;

/** \addtogroup terrain */
/*@{*/

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
	enum Style
	{
		GEODESIC, INDEPENDENT_GEODESIC, RIGHT_TRIANGLE, DYMAX_UNFOLD
	};

	// construction
	IcoGlobe();
	virtual ~IcoGlobe();

	void Create(int iTriangleCount, const vtStringArray &paths,
		const vtString &strImagePrefix, Style style = GEODESIC);
	vtTransform *GetTop() { return m_top; }
	vtGroup *GetSurface() { return m_SurfaceGroup; }

	// control globe behavior
	void SetInflation(float f);
	void SetUnfolding(float f);
	void SetCulling(bool bCull);
	void SetLighting(bool bLight);
	void SetTime(const vtTime &time);
	void ShowAxis(bool bShow);
	void SetSeasonalTilt(bool bTilt) { m_bTilt = bTilt; }
	bool GetSeasonalTilt() { return m_bTilt; }

	// surface features
	vtFeaturesSet &GetFeaturesSet() { return m_features; }
	int AddGlobePoints(const char *fname, float fSize);
	void AddTerrainRectangles(vtTerrainScene *pTerrainScene);
	double AddSurfaceLineToMesh(vtMeshFactory *pMF, const DPoint2 &g1, const DPoint2 &g2);
	double AddSurfaceLineToMesh(vtMeshFactory *pMF, const DLine2 *line);

protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	struct IcoVert
	{
		DPoint3 p;
		FPoint2 uv;
	};

	// Each movable face, composed of 1 to 6 subfaces of an original
	//  icosahedron face.
	struct MFace
	{
		vtTransform *xform;
		vtGeom *geom;
		vtGroup *surfgroup;
		FPoint3 local_origin;
		FPoint3 axis;		// axis of rotation ("hinge") for each face
	};
#endif // DOXYGEN_SHOULD_SKIP_THIS

	int GetMFace(int face, int subface);
	void BuildSphericalFeatures(vtFeatureSet *feat, float fSize);
	void BuildSphericalPoints(vtFeatureSet *feat, float fSize);
	void BuildSphericalLines(vtFeatureSet *feat, float fSize);
	void BuildSphericalPolygons(vtFeatureSet *feat, float fSize);
	void BuildFlatFeatures(vtFeatureSet *feat, float fSize);
	void BuildFlatPoint(vtFeatureSet *feat, int i, float fSize);

	void CreateMaterials(const vtStringArray &paths, const vtString &strImagePrefix);

	void FindLocalOrigin(int mface);
	void SetMeshConnect(int mface);

	void EstimateTesselation(int iTriangleCount);
	void CreateUnfoldableDymax();
	void CreateNormalSphere();
	void CreateIndependentGeodesicSphere();

	// these methods create a mesh for each face composed of strips
	void add_face1(vtMesh *mesh, int face, bool second);
	void set_face_verts1(vtMesh *geom, int face, float f);

	// these methods use a right-triangle recursion to create faces
	void add_face2(vtMesh *mesh, int face, int mface, int subfaces, bool second);
	void set_face_verts2(vtMesh *geom, int face, float f);
	void add_subface(vtMesh *mesh, int face, int v0, int v1, int v2,
								   bool flip, int depth);
	void refresh_face_positions(vtMesh *mesh, int mface, float f);

	// these methods create several meshes per face
	void create_independent_face(int face, bool second);
	void add_face_independent_meshes(int pair, int face, bool second);

	IcoGlobe::Style m_style;

	// Common to all globe styles
	vtTransform	*m_top;
	vtGroup		*m_SurfaceGroup;
	vtGeom		*m_pAxisGeom;
	vtMaterialArray	*m_mats;
	std::vector<int> m_globe_mat;
	int			m_red;
	int			m_yellow;
	int			m_white;
	std::vector<vtMesh*> m_mesh;
	int			m_meshes;	// 20 or 22 for normal or unfoldable globe
	bool		m_bUnfolded;
	bool		m_bTilt;
	FQuat		m_Rotation;

	vtMesh		*m_cylinder;

	// TEMP- replace with feature layer soon
	vtGeom *m_pRectangles;

	// for GEODESIC
	vtGeom	*m_GlobeGeom;
	int		m_freq;		// tesselation frequency
	int		m_subfreq;	// tesselation subfrequency

	// for RIGHT_TRIANGLE
	int		m_vert;
	Array<IcoVert>	m_rtv[22];	// right-triangle vertices
	int		m_depth;	// tesselation depth

	// for DYMAX_UNFOLD
	MFace	m_mface[22];
	FQuat	m_diff;

	// Features (point, line, polygon..) draped on the globe
	vtFeaturesSet	m_features;
};

vtMovGeom *CreateSimpleEarth(const vtString &strDataPath);

void geo_to_xyz(double radius, const DPoint2 &geo, FPoint3 &p);
void geo_to_xyz(double radius, const DPoint2 &geo, DPoint3 &p);
void xyz_to_geo(double radius, const FPoint3 &p, DPoint3 &geo);

/*@}*/	// Group terrain

#endif	// GLOBEH

