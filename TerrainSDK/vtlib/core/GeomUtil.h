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
 *
 * \par Example:
	This example produces a line strip with 10000 vertices.  The factory
	is told to limit the number of vertices in a single primitive to 3000,
	so it will automatically produce four meshes with 3000, 3000, 3000, and
	1000 vertices each.  The meshes are automatically added to the indicated
	geometry node.
	\code
	{
		vtGeom *pLineGeom = new vtGeom;
		vtMeshFactory mf(pLineGeom, vtMesh::LINE_STRIP, 0, 3000, 1);
		mf.PrimStart();
		for (int i = 0; i < 10000; i++)
			mf.AddVertex(FPoint3(i,i,i));
		mf.PrimEnd();
	}
	\endcode
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
class vtDimension : public vtTransform
{
public:
	vtDimension(const FPoint3 &p1, const FPoint3 &p2, float height,
		const RGBf &line_color, const RGBf &text_color,
		vtFont *font, const char *message);

	void SetText(const char *text);

	vtGeom *m_pGeom;
	vtMaterialArray *m_pMats;
	vtMesh *m_pLines;
	vtTextMesh *m_pLabel, *m_pLabel2;
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
						int Axis1, int Axis2, int Axis3,
						const FPoint2 &min1, const FPoint2 &max1,
						float fTiling, int steps);

/*@}*/	// Group sg

#endif // GEOMUTILH

