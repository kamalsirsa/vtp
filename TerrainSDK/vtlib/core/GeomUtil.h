//
// GeomUtil.h
//
// Useful classes and functions for working with geometry and meshes.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GEOMUTILH
#define GEOMUTILH

/** \addtogroup sg */
/*@{*/

/**
 * The vtGeomFactory class makes it easy to create geometry with a lot
 * of vertices.  You simply provide vertices, and it will create as
 * many geometries as necessary to contain them all.
 *
 * \par Example:
	This example produces a line strip with 10000 vertices.  The factory
	is told to limit the number of vertices in a single primitive to 3000,
	so it will automatically produce four geometries with 3000, 3000, 3000, and
	1000 vertices each.  The geometries are automatically added to the indicated
	geode.
	\code
	{
		vtGeode *pLineGeom = new vtGeode;
		vtGeomFactory mf(pLineGeom, osg::PrimitiveSet::LINE_STRIP, 0, 3000, 1);
		mf.PrimStart();
		for (int i = 0; i < 10000; i++)
			mf.AddVertex(FPoint3(i,i,i));
		mf.PrimEnd();
	}
	\endcode
 */
class vtGeomFactory
{
public:
	vtGeomFactory(vtGeode *pGeode, vtMesh::PrimType ePrimType,
		int iVertType, int iMaxVertsPerMesh, int iMatIndex, int iExpectedVerts = -1);
	vtGeomFactory(vtMesh *pMesh);

	void PrimStart();
	void AddVertex(const FPoint3 &p);
	void PrimEnd();

	void SetLineWidth(float width);
	void SetMatIndex(int iIdx);

	std::vector<vtMesh*> m_Meshes;

protected:
	void NewMesh();

	vtGeode *m_pGeode;
	vtMesh::PrimType m_ePrimType;
	int m_iVertType;
	int m_iMaxVertsPerMesh;
	int m_iExpectedVerts;
	int m_iMatIndex;

	vtMesh *m_pMesh;
	int m_iPrimStart;
	int m_iPrimVerts;
	float m_fLineWidth;

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
		osgText::Font *font, const char *message);

	void SetText(const char *text);

	vtGeode *m_pGeode;
	vtMaterialArrayPtr m_pMats;
	vtMesh *m_pLines;
	vtTextMesh *m_pLabel, *m_pLabel2;
};


// helper functions
vtGeode *Create3DCursor(float fSize, float fSmall, float fAlpha = 0.5f);
vtGeode *CreateBoundSphereGeode(const FSphere &sphere, int res = 24);
vtMesh *CreateSphereMesh(const FSphere &sphere, int res = 24);
vtGeode *CreatePlaneGeom(const vtMaterialArray *pMats, int iMatIdx,
						int Axis1, int Axis2, int Axis3,
						const FPoint2 &min1, const FPoint2 &max1,
						float fTiling, int steps);
vtGeode *CreateBlockGeom(const vtMaterialArray *pMats, int iMatIdx,
						const FPoint3 &size);
void AddLineMesh(vtGeode *pGeode, int iMatIdx, FPoint3 &p0, FPoint3 &p1);
vtGeode *CreateSphereGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						 float fRadius, int res);
vtGeode *CreateCylinderGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						   float hHeight, float fRadius, int res,
						   bool bTop = true, bool bBottom = true,
						   bool bCentered = true, int direction = 1);
vtGeode *CreateLineGridGeom(const vtMaterialArray *pMats, int iMatIdx,
					   const FPoint3 &min1, const FPoint3 &max1, int steps);

/**
	A dynamic visual display of a wireframe bounding box, which can be very
	useful for debugging things involving extents in the 3D scene.  To use:
\code
	// Create
	vtDynBoundBox *box = new vtDynBoundBox(RGBf(0,0,1));	// blue box
	parent->addChild(box->pGeode);

	// Use it to show a node's bounding box
	FBox3 bb;
	GetNodeBoundBox(node, bb);
	box->SetBox(bb);
\endcode
 */
class vtDynBoundBox
{
public:
	vtDynBoundBox(const RGBf &color);
	void SetBox(const FBox3 &box);
	vtGeode *pGeode;
	vtMesh *pMesh;
};

// Writing Wavefront OBJ format
struct vtOBJFile
{
	FILE *fp;
	int verts_written;
};
vtOBJFile *OBJFileBegin(vtGeode *geode, const char *filename);
void OBJFileWriteGeom(vtOBJFile *file, vtGeode *geode);
bool WriteGeomToOBJ(vtGeode *geode, const char *filename);

/*@}*/	// Group sg

#endif // GEOMUTILH

