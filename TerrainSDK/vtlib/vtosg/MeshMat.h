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
	vtImage *GetTexture();

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

class GeoSet2 : public osg::GeoSet
{
public:
	class vtMesh *m_pMesh;
};

class vtMesh : public vtMeshBase
{
public:
	vtMesh(GLenum PrimType, int VertType, int NumVertices);

	void AddTri(int p0, int p1, int p2);
	void AddFan(int p0, int p1, int p2, int p3 = -1, int p4 = -1, int p5 = -1);
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

