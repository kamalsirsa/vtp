//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MESHMATH
#define VTOSG_MESHMATH

#include <osgText/Font>
#include <osgText/Text>

#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/AlphaFunc>
#include <osg/PrimitiveSet>
#include <osg/Geometry>

class vtImage;

/** \addtogroup sg */
/*@{*/

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
	RGBAf GetDiffuse() const;

	void SetSpecular(float r, float g, float b);
	RGBf GetSpecular() const;

	void SetAmbient(float r, float g, float b);
	RGBf GetAmbient() const;

	void SetEmission(float r, float g, float b);
	RGBf GetEmission() const;

	void SetCulling(bool bCulling);
	bool GetCulling() const;

	void SetLighting(bool bLighting);
	bool GetLighting() const;

	void SetTransparent(bool bOn, bool bAdd = false);
	bool GetTransparent() const;

	void SetWireframe(bool bOn);
	bool GetWireframe() const;

	void SetTexture(vtImage *pImage);
	void SetTexture2(const char *szFilename);
	vtImage	*GetTexture() const;

	void SetClamp(bool bClamp);
	bool GetClamp() const;

	void SetMipMap(bool bMipMap);
	bool GetMipMap() const;

	void Apply();

	// global option
	static bool s_bTextureCompression;

	// remember this for convenience
	osg::ref_ptr<vtImage>	m_pImage;

	// the VT material object includes texture
	osg::ref_ptr<osg::Material>		m_pMaterial;
	osg::ref_ptr<osg::Texture2D>	m_pTexture;
	osg::ref_ptr<osg::StateSet>		m_pStateSet;
	osg::ref_ptr<osg::BlendFunc>	m_pBlendFunc;
	osg::ref_ptr<osg::AlphaFunc>	m_pAlphaFunc;
};

/**
 * Contains an array of materials.  Provides useful methods for creating material easily.
 */
class vtMaterialArray : public vtMaterialArrayBase, public osg::Referenced
{
public:
	vtMaterialArray();

	/// Use this method instead of delete, when you are done with this object.
	void Release();

	/// Adds a material to this material array.
	int AppendMaterial(vtMaterial *pMat);

protected:
	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
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

	// Override with ability to get OSG bounding box
	void GetBoundBox(FBox3 &box);

	/// Use this method instead of delete, when you are done with this object.
	void Release();

	// Adding primitives
	void AddTri(int p0, int p1, int p2);
	void AddFan(int p0, int p1, int p2 = -1, int p3 = -1, int p4 = -1, int p5 = -1);
	void AddFan(int *idx, int iNVerts);
	void AddStrip(int iNVerts, unsigned short *pIndices);
	void AddLine(int p0, int p1);
	int  AddLine(const FPoint3 &pos1, const FPoint3 &pos2);
	void AddQuad(int p0, int p1, int p2, int p3);

	// Access vertex properties
	void SetVtxPos(int, const FPoint3&);
	FPoint3 GetVtxPos(int i) const;

	void SetVtxNormal(int, const FPoint3&);
	FPoint3 GetVtxNormal(int i) const;

	void SetVtxColor(int, const RGBf&);
	RGBf GetVtxColor(int i) const;

	void SetVtxTexCoord(int, const FPoint2&);
	FPoint2 GetVtxTexCoord(int i) const;

	void SetLineWidth(float fWidth);

	// Control rendering optimization ("display lists")
	void ReOptimize();
	void AllowOptimize(bool bAllow);

	// Access values
	int GetNumPrims() const;
	int GetNumIndices() const { return m_Index->size(); }
	short GetIndex(int i) const { return m_Index->at(i); }
	int GetPrimLen(int i) const { return dynamic_cast<const osg::DrawArrayLengths*>(m_pPrimSet.get())->at(i); }

	void SetNormalsFromPrimitives();

protected:
	// Implementation
	void _AddStripNormals();

	// Holder for all osg geometry information
	osg::ref_ptr<osg::Geometry> m_pGeometry;

	// The vertex co-ordinates array
	osg::ref_ptr<osg::Vec3Array>	m_Vert;
	// The vertex normals array
	osg::ref_ptr<osg::Vec3Array>	m_Norm;
	// The vetex colors array
	osg::ref_ptr<osg::Vec4Array>	m_Color;
	//  The vertex texture co-ordinates array
	osg::ref_ptr<osg::Vec2Array>	m_Tex;

	// The vertex index array
	osg::ref_ptr<osg::UIntArray>	m_Index;

	osg::ref_ptr<osg::PrimitiveSet>	m_pPrimSet;

	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	virtual ~vtMesh() {}
};

/**
 * A Font for use with vtTextMesh.  vtFont can be constructed from any
 * TrueType font on your computer.
 */
class vtFont
{
public:
	vtFont();
	bool LoadFont(const char *filename);

	// Implementation
	osg::ref_ptr<osgText::Font> m_pOsgFont;
};

/**
 * A Text Mesh is a special kind of Mesh which contains text instead of
 * general geometry primitives.  vtTextMesh is used similarly with vtMesh:
 * you create them and add them to vtGeom objects to add them to the scene.
 */
class vtTextMesh : public osg::Referenced
{
public:
	vtTextMesh(vtFont *font, float fSize = 1, bool bCenter = false);

	/// Use this method instead of delete, when you are done with this object.
	void Release();

	/// Set the text string that this TextMesh should display
	void SetText(const char *text);

	/// Set the (wide) text string that this TextMesh should display
	void SetText(const wchar_t *text);
#if SUPPORT_WSTRING
	/// Set the (wide) text string that this TextMesh should display
	void SetText(const std::wstring &text);
#endif
	/// Set the position (XYZ translation) of the text in its own frame
	void SetPosition(const FPoint3 &pos);

	/// Set the text alignment: 0 for XY plane, 1 for XZ, 2 for YZ
	void SetAlignment(int align);

	/// Set the color
	void SetColor(const RGBAf &rgba);

	// Implementation
	osg::ref_ptr<osgText::Text> m_pOsgText;

	void SetMatIndex(int i) { m_iMatIdx = i; }
	int GetMatIndex() const { return m_iMatIdx; }

protected:
	int	m_iMatIdx;

	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	~vtTextMesh();
};

/*@}*/	// Group sg

#endif	// VTOSG_MESHMATH

