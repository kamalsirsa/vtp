//
// GeomUtil.h
//
// Useful classes and functions for working with geometry and meshes.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GEOMUTILH
#define GEOMUTILH

/** \addtogroup sg */
/*@{*/

/**
 * The vtMeshFactory class makes it easy to create meshes with a lot
 * of vertices.  You simply provide vertices, and it will create as
 * many meshes as necessary to contain them all.
 */
class vtMeshFactory
{
public:
	vtMeshFactory(vtGeom *pGeom, vtMeshBase::PrimType ePrimType,
		int iVertType, int iMaxVertsPerMesh, int iMatIndex);
	vtMeshFactory(vtMesh *pMesh);

	void PrimStart();
	void AddVertex(const FPoint3 &p);
	void PrimEnd();

protected:
	void NewMesh();

	vtGeom *m_pGeom;
	vtMesh::PrimType m_ePrimType;
	int m_iVertType;
	int m_iMaxVertsPerMesh;
	int m_iMatIndex;

	vtMesh *m_pMesh;
	int m_iPrimStart;
	int m_iPrimVerts;

	bool m_bSimple;
};


/**
 * The vtDimension class creates a 'dimension' object, which draws an arrow
 * connecting two points in space along with a text label, which is useful
 * for describing the length of the line (or any other string).
 */
class vtDimension : public vtGeom
{
public:
	vtDimension(const FPoint3 &p1, const FPoint3 &p2, float height,
		const RGBf &color, vtFont *font, const char *message);

	vtMaterialArray *m_pMats;
	vtMesh *m_pLines;
	vtTextMesh *m_pLabel;
};


// helper functions
vtGeom *Create3DCursor(float fSize, float fSmall, float fAlpha = 0.5f);
vtGeom *CreateBoundSphereGeom(const FSphere &sphere, int res = 24);
vtGeom *CreateSphereGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						 float fRadius, int res);
vtGeom *CreateCylinderGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						   float hHeight, float fRadius, int res,
						   bool bTop = true, bool bBottom = true,
						   bool bCentered = true, int direction = 1);
vtGeom *CreateLineGridGeom(const vtMaterialArray *pMats, int iMatIdx,
					   const FPoint3 &min1, const FPoint3 &max1, int steps);
vtGeom *CreatePlaneGeom(const vtMaterialArray *pMats, int iMatIdx,
						const FPoint2 &base, const FPoint2 &size,
						float fTiling, int steps);


/*@}*/	// Group sg

#endif // GEOMUTILH

