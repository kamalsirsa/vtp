//
// Globe.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GLOBEH
#define GLOBEH

#include "vtdata/Icosa.h"
#include "vtdata/FilePath.h"

class IcoGlobe : public DymaxIcosa
{
public:
	void Create(int freq, const StringArray &paths, vtString strImagePrefix);
	void SetInflation(float f);
	void SetLighting(bool bLight);
	void AddPoints(DLine2 &points, float fSize);

	void AddTerrainRectangles();
	int AddGlobePoints(const char *fname);
	double AddSurfaceLineToMesh(vtMesh *mesh, const DPoint2 &g1, const DPoint2 &g2);

	vtGeom		*m_geom;
	vtMovGeom	*m_mgeom;
	int		m_red;
	int		m_yellow;

protected:
	void set_face_verts(vtMesh *geom, int face, float f);
	void add_face(vtMesh *geom, int face, int appidx, bool second);

	vtMaterialArray	*m_mats;
	int		m_globe_mat[10];
	vtMesh	*m_mesh[21];

	int		m_freq;	// tesselation frequency
};

vtMovGeom *CreateSimpleEarth(vtString strDataPath);

bool FindIntersection(const FPoint3 &rkOrigin, const FPoint3 &rkDirection,
					  const FSphere& rkSphere,
					  int& riQuantity, FPoint3 akPoint[2]);

void geo_to_xyz(double radius, const DPoint2 &geo, FPoint3 &p);
void geo_to_xyz(double radius, const DPoint2 &geo, DPoint3 &p);
void xyz_to_geo(double radius, const FPoint3 &p, DPoint3 &geo);

#endif
