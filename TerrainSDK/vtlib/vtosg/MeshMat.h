//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MESHMATH
#define VTOSG_MESHMATH

#include <osgText/Font>
#include <osgText/Text>
#include <osg/BlendFunc>

class vtImage;

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

	void SetTransparent(bool bOn, bool bAdd = false);
	bool GetTransparent();

	void SetWireframe(bool bOn);
	bool GetWireframe();

	void SetTexture(vtImage *pImage);
	void SetTexture2(const char *szFilename);
	vtImage	*GetTexture();

	void SetClamp(bool bClamp);
	bool GetClamp();

	void SetMipMap(bool bMipMap);
	bool GetMipMap();

	void Apply();

	// remember this for convenience
	osg::ref_ptr<vtImage>	m_pImage;

	// the VT material object includes texture
	osg::ref_ptr<osg::Material>		m_pMaterial;
	osg::ref_ptr<osg::Texture2D>	m_pTexture;
	osg::ref_ptr<osg::StateSet>		m_pStateSet;
	osg::ref_ptr<osg::BlendFunc>	m_pBlendFunc;
};

/**
 * Contains an array of materials.  Provides useful methods for creating material easily.
 */
class vtMaterialArray : public vtMaterialArrayBase, public osg::Referenced
{
public:
	vtMaterialArray();
	void Release();

	int AppendMaterial(vtMaterial *pMat);

protected:
	virtual ~vtMaterialArray();
};


/////////////////////////////////////////////

/**
 * A Mesh is a set of graphical primitives (such as lines, triangles,
 *	or fans).
 * \par
 * The vtMesh class allows you to define and access a Mesh, including many
 * functions useful for creating and dynamically changing Meshes.
 * To add the vtMesh to the visible scene graph, add it to a vtGeom node.
 * \par
 * Most of the useful methods of this class are defined on its parent
 *	class, vtMeshBase.
 */
class vtMesh : public vtMeshBase, public osg::Referenced
{
	friend class vtGeom;

public:
	vtMesh(GLenum PrimType, int VertType, int NumVertices);
	void Release();

	// Adding primitives
	void AddTri(int p0, int p1, int p2);
	void AddFan(int p0, int p1, int p2 = -1, int p3 = -1, int p4 = -1, int p5 = -1);
	void AddFan(int *idx, int iNVerts);
	void AddStrip(int iNVerts, unsigned short *pIndices);
	void AddLine(int p0, int p1);

	// Access vertex properties
	void SetVtxPos(int, const FPoint3&);
	FPoint3 GetVtxPos(int i) const;

	void SetVtxNormal(int, const FPoint3&);
	FPoint3 GetVtxNormal(int i) const;

	void SetVtxColor(int, const RGBf&);
	RGBf GetVtxColor(int i) const;

	void SetVtxTexCoord(int, const FPoint2&);
	FPoint2 GetVtxTexCoord(int i);

	// Control rendering optimization ("display lists")
	void ReOptimize();
	void AllowOptimize(bool bAllow);

	// Access values
	int GetNumPrims();
	int GetNumIndices() { return m_Index.GetSize(); }
	short GetIndex(int i) { return m_Index.GetAt(i); }
	int GetPrimLen(int i) { return m_PrimLen.GetAt(i); }

	void SetNormalsFromPrimitives();

protected:
	// Implementation
	void _AddStripNormals();
	osg::ref_ptr<osg::GeoSet> m_pGeoSet;

	// GeoSet doesn't actually know or care about how many vertices
	// it contains - it just needs a pointer to the beginning of
	// the vertex arrays:
	Array<osg::Vec3>	m_Vert;
	Array<unsigned short>	m_Index;
	Array<int>			m_PrimLen;
	Array<osg::Vec3>	m_Norm;
	Array<osg::Vec4>	m_Color;
	Array<osg::Vec2>	m_Tex;

	// Point OSG to the vertex and primitive data that we maintain
	void	SendPointersToOSG();

	virtual ~vtMesh();
};

class vtFont
{
public:
	vtFont();
	bool LoadFont(const char *filename);

	// Implementation
	osg::ref_ptr<osgText::Font> m_pOsgFont;
};

class vtTextMesh : public osg::Referenced
{
public:
	vtTextMesh(vtFont *font, float fSize = 1, bool bCenter = false);

	void Release();

	void SetText(const char *text);
	void SetText(const wchar_t *text);
	void SetText(const std::wstring &text);
	void SetPosition(const FPoint3 &pos);
	void SetAlignment(int align);

	// Implementation
	osg::ref_ptr<osgText::Text> m_pOsgText;

protected:
	~vtTextMesh();
};

#endif	// VTOSG_MESHMATH

