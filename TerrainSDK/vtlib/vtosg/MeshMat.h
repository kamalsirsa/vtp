//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MESHMATH
#define VTOSG_MESHMATH

class vtMaterial : public vtMaterialBase
{
public:
	vtMaterial();

	void SetDiffuse(float r, float g, float b, float a = 1.0f);
	RGBAf GetDiffuse();

	void SetSpecular(float r, float g, float b);
	RGBf GetSpecular();

	void SetAmbient(float r, float g, float b);
	RGBf GetAmbient();

	void SetEmission(float r, float g, float b);
	RGBf GetEmission();

	void SetCulling(bool bCulling);
	bool GetCulling();

	void SetLighting(bool bLighting);
	bool GetLighting();

	void SetTexture(class vtImage *pImage);

	void SetTransparent(bool bOn, bool bAdd = false);
	bool GetTransparent();

	void SetClamp(bool bClamp);
	bool GetClamp();

	void SetMipMap(bool bMipMap);
	bool GetMipMap();

	void Apply();

	// the VT material object includes texture
	osg::ref_ptr<osg::Material>	m_pMaterial;
	osg::ref_ptr<osg::Texture>	m_pTexture;
	osg::ref_ptr<osg::StateSet>	m_pStateSet;
};

class vtMaterialArray : public vtMaterialArrayBase
{
public:
	int AppendMaterial(vtMaterial *pMat);
};


/////////////////////////////////////////////

#ifndef DOXYGEN_SHOULD_SKIP_THIS

class GeoSet2 : public osg::GeoSet
{
public:
	GeoSet2();
	~GeoSet2();
	class vtMesh *m_pMesh;
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * A Mesh is a set of graphical primitives (such as lines, triangles, or fans).
 * \par
 * The vtMesh class allows you to define and access a Mesh, including many
 * functions useful for creating and dynamically changing Meshes.
 * \par
 * To add the vtMesh to the visible scene graph, add it to a vtGeom node.
 */
class vtMesh : public vtMeshBase
{
public:
	/** Create a vtMesh.
	 \param PrimType The OpenGL primitive type, e.g. GL_LINES, GL_TRIANGLE_STRIP
	 \param VertType The types of data that are stored for each vertex.  This can
	  be a combination of any of the following values:
	   - VT_Normals if you need vertex normals for smooth shading.
	   - VT_Colors if you need vertex colors.
	   - VT_TexCoords if you need texture coordinates for each vertex.
	 \param NumVertices The number of vertices you expect the mesh to contain.
	  This number does not need to be exact, since the vertex buffer will be
	  expanded as necessary, but it improves efficiency if you pass the correct
	  number at construction time.
	 */
	vtMesh(GLenum PrimType, int VertType, int NumVertices);

	/** Add a triangle.  p0, p1, p2 are the indices of the vertices of the triangle. */
	void AddTri(int p0, int p1, int p2);

	/** Add a triangle fan with up to 6 points (center + 5 points).  The first 3
	 arguments are required, the rest are optional.  A fan will be created with as
	 many point indices as you pass. */
	void AddFan(int p0, int p1, int p2, int p3 = -1, int p4 = -1, int p5 = -1);

	/** Add a triangle fan with any number of points.
	 \param idx An array of vertex indices for the fan.
	 \param iNVerts the number of vertices in the fan.
	*/
	void AddFan(int *idx, int iNVerts);

	void AddStrip(int iNVerts, unsigned short *pIndices);
	void AddQuadStrip(int iNVerts, int iStartIndex);
	void AddLine(int p0, int p1);

	void SetVtxPos(int, const FPoint3&);
	FPoint3 GetVtxPos(int i);

	void SetVtxNormal(int, const FPoint3&);
	FPoint3 GetVtxNormal(int i);

	void SetVtxColor(int, const RGBf&);
	RGBf GetVtxColor(int i);

	void SetVtxTexCoord(int, const FPoint2&);
	FPoint2 GetVtxTexCoord(int i);

	int GetNumPrims();
	void ReOptimize();
	void AllowOptimize(bool bAllow);

	int GetNumIndices() { return m_Index.GetSize(); }
	short GetIndex(int i) { return m_Index.GetAt(i); }
	int GetPrimLen(int i) { return m_PrimLen.GetAt(i); }

	// Implementation
	osg::ref_ptr<GeoSet2> m_pGeoSet;

	// GeoSet doesn't actually know or care about how many vertices
	// it contains - it just needs a pointer to the beginning of
	// the vertex arrays:
	Array<osg::Vec3>	m_Vert;
	Array<unsigned short>	m_Index;
	Array<int>			m_PrimLen;
	Array<osg::Vec3>	m_Norm;
	Array<osg::Vec4>	m_Color;
	Array<osg::Vec2>	m_Tex;

protected:
	// keep track of the number of vertices ourselves
	void	SendPointersToOSG();
};

#endif

