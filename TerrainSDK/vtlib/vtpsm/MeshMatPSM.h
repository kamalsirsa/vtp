//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPSM_MESHMATH
#define VTPSM_MESHMATH

// Vertex component flags
#undef VT_Normals
#undef VT_Colors
#undef VT_TexCoords
#define VT_Normals		VTX_Normals
#define VT_Colors		VTX_Colors
#define VT_TexCoords	VTX_TexCoords

//
// This subclass exists in order to access the protected method LoadDev.
//
class MyPSAppearance : public PSAppearance
{
public:
	void MyApply();
};

/**
 * A material is a description of how a surface should be rendered.  For a
 * good description of how Materials work, see the opengl.org website or
 * the OpenGL Red Book.
 *
 * Much of the functionality of vtMaterial is inherited from its base class,
 * vtMaterialBase.
 */
class vtMaterial : public vtMaterialBase
{
public:
	vtMaterial();
	~vtMaterial();

	void SetDiffuse(float r, float g, float b, float a = 1.0f);
	void SetDiffuse1(const RGBAf& c)	{ SetDiffuse(c.r, c.g, c.b); }
	RGBAf GetDiffuse();

	void SetSpecular(float r, float g, float b);
	void SetSpecular1(const RGBf& c);
	void SetSpecular2(float shine);
	RGBf GetSpecular();

	void SetAmbient(float r, float g, float b);
	void SetAmbient1(const RGBf& c)		{ SetAmbient(c.r, c.g, c.b); }
	RGBf GetAmbient();

	void SetEmission(float r, float g, float b);
	void SetEmission1(const RGBf& c);
	RGBf GetEmission();

	void SetCulling(bool bCulling);
	bool GetCulling();

	void SetLighting(bool bLighting);
	bool GetLighting();

	void SetTransparent(bool bOn, bool bAdd = false);
	bool GetTransparent();

	void SetWireframe(bool bOn);
	bool GetWireframe();

	void SetClamp(bool bClamp);
	bool GetClamp();

	void SetMipMap(bool bMipMap);
	bool GetMipMap();

	vtImage* GetTexture();
	void SetTexture(class vtImage *pImage);
	void SetTexture2(const char* s);

	void Apply();
	void CopyFrom(vtMaterial *pFrom)		{ m_pApp->Copy(pFrom->m_pApp); }

	MyPSAppearance *m_pApp;
};

/**
 * Contains an array of materials.  Provides useful methods for creating
 * material easily.
 */
class vtMaterialArray : public vtMaterialArrayBase
{
public:
	vtMaterialArray();
	~vtMaterialArray();

	void CopyFrom(vtMaterialArray *pFromMats);
	int AppendMaterial(vtMaterial *pMat);

	PSAppearances	*m_pApps;
};



/**
 * A Mesh is a set of graphical primitives (such as lines, triangles, or fans).
 * \par
 * The vtMesh class allows you to define and access a Mesh, including many
 * functions useful for creating and dynamically changing Meshes.
 * To add the vtMesh to the visible scene graph, add it to a vtGeom node.
 * \par
 * Most of the useful methods of this class are defined on its parent class, vtMeshBase.
 */
class vtMesh : public PSTriMesh, public vtMeshBase
{
public:
	vtMesh(GLenum PrimType, int VertType, int NumVertices);

	// Accessors
	virtual int GetNumVertices()	{ return GetNumVtx(); }
	int GetVtxType()				{ return GetStyle(); }
	void GetBoundBox(FBox3 &box)	{ PSBox3 box1; GetBound(&box1);
		box.min.x = box1.min.x;
		box.min.y = box1.min.y;
		box.min.z = box1.min.z;
		box.max.x = box1.max.x;
		box.max.y = box1.max.y;
		box.max.z = box1.max.z;
	}

	void SetMatIndex(int i)			{ SetAppIndex(i); }
	int GetMatIndex()				{ return GetAppIndex(); }

	// Adding vertices
	int AddVertex(float x, float y, float z);
	int AddVertexN(float x, float y, float z, float nx, float ny, float nz);
	int AddVertexUV(float x, float y, float z, float u, float v);

	int AddVertex(const FPoint3 &p);
	int AddVertexN(const FPoint3 &p, const FPoint3 &n);
	int AddVertexUV(const FPoint3 &p, float u, float v);
	int AddVertexUV(const FPoint3 &p, const FPoint2 &uv);
	int AddVertexNUV(const FPoint3 &p, const FPoint3 &n, const FPoint2 &uv);
	/** Add a triangle.  p0, p1, p2 are the indices of the vertices of the triangle. */
	void AddTri(int p0, int p1, int p2);

	/** Add a triangle fan with up to 6 points (center + 5 points).  The first 3
	 arguments are required, the rest are optional.  A fan will be created with as
	 many point indices as you pass. */
	void AddFan(int p0, int p1, int p2 = -1, int p3 = -1, int p4 = -1, int p5 = -1);

	/** Add a triangle fan with any number of points.
	 \param idx An array of vertex indices for the fan.
	 \param iNVerts the number of vertices in the fan.
	*/
	void AddFan(int *idx, int iNVerts);

	void AddStrip(int iNVerts, unsigned short *pIndices);
	void AddStrip2(int iNVerts, int startindex);
	void AddQuadStrip(int iNVerts, int iStartIndex);
	void AddLine(int p0, int p1);

	void SetVtxPos(int, const FPoint3&);
	FPoint3 GetVtxPos(int i) const;

	void SetVtxNormal(int, const FPoint3&);
	FPoint3 GetVtxNormal(int i) const;

	void SetVtxColor(int, const RGBf&);
	RGBf GetVtxColor(int i) const;

	void SetVtxTexCoord(int, const FPoint2&);
	FPoint2 GetVtxTexCoord(int i);

	void SetVtxPUV(int i, const FPoint3 &pos, float u, float v)
	{
		SetVtxPos(i, pos);
		SetVtxTexCoord(i, FPoint2(u, v));
	}
	void SetVtxPN(int i, const FPoint3 &pos, const FPoint3 &norm)
	{
		SetVtxPos(i, pos);
		SetVtxNormal(i, norm);
	}

	int GetNumPrims();
	void ReOptimize()	{ }
	void AllowOptimize(bool bAllow)	{ }

	int GetNumIndices() { return GetNumIdx(); }
	short GetIndex(int i) { return GetIndex(i); }
	int GetPrimLen(int i) { PSTriPrim* prim = GetPrim(i); if (prim) return prim->Size; else return 0; }
};

class vtFont : public PSImage
{
public:
	vtFont() : PSImage() { };
	bool LoadFont(const char *filename)	{ return Load(filename); }
};

class vtTextMesh : public PSTextGeometry
{
public:
	vtTextMesh(vtFont *font, bool bCenter = false);
};

#endif	// VTPSM_MESHMATH

