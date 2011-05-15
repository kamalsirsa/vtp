//
// MeshMat.h for OSG
//
// Copyright (c) 2001-2009 Virtual Terrain Project
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

// Shorthand
#define FAB		osg::Material::FRONT_AND_BACK

class vtImage;

/** \addtogroup sg */
/*@{*/

/**
 * A material is a description of how a surface should be rendered.  For a
 * good description of how Materials work, see the opengl.org website or
 * the OpenGL Red Book.
 */
class vtMaterial : public osg::StateSet
{
public:
	vtMaterial();
	~vtMaterial();

	void CopyFrom(vtMaterial *pFrom);

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
	bool SetTexture2(const char *szFilename);
	vtImage	*GetTexture() const;
	void ModifiedTexture();

	void SetClamp(bool bClamp);
	bool GetClamp() const;

	void SetMipMap(bool bMipMap);
	bool GetMipMap() const;

	void SetDiffuse1(const RGBAf &c) { SetDiffuse(c.r, c.g, c.b, c.a); }
	void SetDiffuse2(float f) { SetDiffuse(f, f, f); }

	void SetSpecular1(const RGBf &c) { SetSpecular(c.r, c.g, c.b); }
	void SetSpecular2(float f) { SetSpecular(f, f, f); }

	void SetAmbient1(const RGBf &c) { SetAmbient(c.r, c.g, c.b); }
	void SetAmbient2(float f) { SetAmbient(f, f, f); }

	void SetEmission1(const RGBf &c) { SetEmission(c.r, c.g, c.b); }
	void SetEmission2(float f) { SetEmission(f, f, f); }

	void SetName(const vtString &name) { setName(name); }
	const vtString GetName() const { return getName().c_str(); }

	// global option
	static bool s_bTextureCompression;

	// remember this for convenience and referencing
	osg::ref_ptr<vtImage> m_Image;

	// the VT material object includes texture
	osg::ref_ptr<osg::Material>		m_pMaterial;
	osg::ref_ptr<osg::Texture2D>	m_pTexture;
	osg::ref_ptr<osg::BlendFunc>	m_pBlendFunc;
	osg::ref_ptr<osg::AlphaFunc>	m_pAlphaFunc;
};
typedef osg::ref_ptr<vtMaterial> vtMaterialPtr;

/**
 * Contains an array of materials.  Provides useful methods for creating material easily.
 */
class vtMaterialArray : public std::vector<vtMaterialPtr>, public osg::Referenced
{
public:
	vtMaterialArray();
	~vtMaterialArray();

	int Find(vtMaterial *mat);
	int AddTextureMaterial(class vtImage *pImage,
						 bool bCulling, bool bLighting,
						 bool bTransp = false, bool bAdditive = false,
						 float fAmbient = 0.0f, float fDiffuse = 1.0f,
						 float fAlpha = 1.0f, float fEmissive = 0.0f,
						 bool bTexGen = false, bool bClamp = false,
						 bool bMipMap = false);
	int AddTextureMaterial2(const char *fname,
						 bool bCulling, bool bLighting,
						 bool bTransp = false, bool bAdditive = false,
						 float fAmbient = 0.0f, float fDiffuse = 1.0f,
						 float fAlpha = 1.0f, float fEmissive = 0.0f,
						 bool bTexGen = false, bool bClamp = false,
						 bool bMipMap = false);
	int AddRGBMaterial(const RGBf &diffuse, const RGBf &ambient,
					 bool bCulling = true, bool bLighting= true, bool bWireframe = false,
					 float fAlpha = 1.0f, float fEmissive = 0.0f);
	int AddRGBMaterial1(const RGBf &diffuse,
					 bool bCulling = true, bool bLighting= true, bool bWireframe = false,
					 float fAlpha = 1.0f, float fEmissive = 0.0f);
	void AddShadowMaterial(float fOpacity);
	int FindByDiffuse(const RGBAf &rgba) const;
	int FindByImage(const vtImage *image) const;

	void CopyFrom(vtMaterialArray *pFromMats);

	void RemoveMaterial(vtMaterial *pMat);

	/// Adds a material to this material array.
	int AppendMaterial(vtMaterial *pMat);
};
typedef osg::ref_ptr<vtMaterialArray> vtMaterialArrayPtr;

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
class vtMesh : public vtMeshBase, public osg::Geometry
{
	friend class vtGeom;

public:
	vtMesh(enum PrimType ePrimType, int VertType, int NumVertices);

	// Override with ability to get OSG bounding box
	void GetBoundBox(FBox3 &box) const;

	// Adding primitives
	void AddTri(int p0, int p1, int p2);
	void AddFan(int p0, int p1, int p2 = -1, int p3 = -1, int p4 = -1, int p5 = -1);
	void AddFan(int *idx, int iNVerts);
	void AddStrip(int iNVerts, unsigned short *pIndices);
	void AddLine(int p0, int p1);
	int  AddLine(const FPoint3 &pos1, const FPoint3 &pos2);
	void AddQuad(int p0, int p1, int p2, int p3);

	// Access vertex properties
	unsigned int GetNumVertices() const;

	void SetVtxPos(unsigned int, const FPoint3&);
	FPoint3 GetVtxPos(unsigned int i) const;

	void SetVtxNormal(unsigned int, const FPoint3&);
	FPoint3 GetVtxNormal(unsigned int i) const;

	void SetVtxColor(unsigned int, const RGBAf&);
	RGBAf GetVtxColor(unsigned int i) const;

	void SetVtxTexCoord(unsigned int, const FPoint2&);
	FPoint2 GetVtxTexCoord(unsigned int i) const;

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
	void _AddPolyNormals();
	void _AddTriangleNormals();
	void _AddQuadNormals();

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
};

/** A Font for use with vtTextMesh. */
typedef osg::ref_ptr<osgText::Font> vtFontPtr;

/**
 * A Text Mesh is a special kind of Mesh which contains text instead of
 * general geometry primitives.  vtTextMesh is used similarly with vtMesh:
 * you create them and add them to vtGeom objects to add them to the scene.
 */
class vtTextMesh : public osgText::Text
{
public:
	vtTextMesh(osgText::Font *font, float fSize = 1, bool bCenter = false);

	// Override with ability to get OSG bounding box
	void GetBoundBox(FBox3 &box) const;

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

	/// Set the rotation (Quaternion) of the text in its own frame
	void SetRotation(const FQuat &rot);

	/// Set the text alignment: 0 for XY plane, 1 for XZ, 2 for YZ
	void SetAlignment(int align);

	/// Set the color
	void SetColor(const RGBAf &rgba);

	void SetMatIndex(int i) { m_iMatIdx = i; }
	int GetMatIndex() const { return m_iMatIdx; }

protected:
	int	m_iMatIdx;
};

/*@}*/	// Group sg

#endif	// VTOSG_MESHMATH

