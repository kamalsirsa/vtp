//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPLIB_MESHMATH
#define VTPLIB_MESHMATH

class vtMaterial : public vtMaterialBase, public ssgSimpleState
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
};

class vtMaterialArray : public vtMaterialArrayBase
{
public:
	int AppendMaterial(vtMaterial *pMat);
};


#if 0
//
// The ssg array classes don't have ability to set values (!)
// so, subclass to extend them with this ability
//

class ssgVertexArray2 : public ssgVertexArray
{
public:
	ssgVertexArray2(int init) : ssgVertexArray(init) {}

	void set ( unsigned int n, sgVec3 thing )
	{ memcpy ( & list [ size_of * n ], thing, size_of ); }
};

class ssgNormalArray2 : public ssgNormalArray
{
public:
	ssgNormalArray2(int init) : ssgNormalArray(init) {}

	void set ( unsigned int n, sgVec3 thing )
	{ memcpy ( & list [ size_of * n ], thing, size_of ); }
};

class ssgColourArray2 : public ssgColourArray
{
public:
	ssgColourArray2(int init) : ssgColourArray(init) {}

	void set ( unsigned int n, sgVec4 thing )
	{ memcpy ( & list [ size_of * n ], thing, size_of ); }
};

class ssgTexCoordArray2 : public ssgTexCoordArray
{
public:
	ssgTexCoordArray2(int init) : ssgTexCoordArray(init) {}

	void set ( unsigned int n, sgVec2 thing )
	{ memcpy ( & list [ size_of * n ], thing, size_of ); }
};
#endif

/////////////////////////////////////////////

class vtMesh : public vtMeshBase, public ssgBase
{
public:
	vtMesh(GLenum PrimType, int VertType, int NumVertices = 0);

	void AddTri(int p0, int p1, int p2);
	void AddFan(int p0, int p1, int p2, int p3 = -1, int p4 = -1, int p5 = -1);
	void AddFan(int *idx, int iNVerts);
	void AddStrip(int iNVerts, unsigned short *pIndices);
	void AddLine(int p0, int p1);

	void SetVtxPos(int, const FPoint3&);
	FPoint3 GetVtxPos(int i) const;

	void SetVtxNormal(int, const FPoint3&);
	FPoint3 GetVtxNormal(int i) const;

	void SetVtxColor(int, const RGBf&);
	RGBf GetVtxColor(int i) const;

	void SetVtxTexCoord(int, const FPoint2&);
	FPoint2 GetVtxTexCoord(int i);

	int GetNumPrims();
	void ReOptimize();

	int GetNumIndices() { return m_indices->getSizeOf(); }
	short GetIndex(int i) { return *m_indices->get(i); }
	int GetPrimLen(int i)
	{
		// TODO - where are the lengths of the primitives?
		// return m_PrimLen.get(i);
		return 0;
	}

	// Implementation
//	ssgVtxArray		  *m_pVtx;
	ssgVtxTable		  *m_pVtx;

	ssgVertexArray   *m_vertices;
	ssgNormalArray   *m_normals;
	ssgTexCoordArray *m_texcoords;
	ssgColourArray   *m_colours;
	ssgIndexArray	 *m_indices;
};

#endif

