//
// MeshMat.h - Meshes and Materials for vtlib-OpenSG
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MESHMATH
	#define VTOSG_MESHMATH

	#include <OpenSG/OSGGeometry.h>
	#include <OpenSG/OSGSimpleTexturedMaterial.h>
	#include <OpenSG/OSGScreenAlignedText.h>
	#include <OpenSG/OSGTXFFont.h>
	#include <OpenSG/OSGText.h>
	#include <OpenSG/OSGSharedFontStyleWrapper.h>
	#include <OpenSG/OSGTransform.h>

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
	bool SetTexture2(const char *szFilename);
	vtImage *GetTexture() const;
	void ModifiedTexture();

	void SetClamp(bool bClamp);
	bool GetClamp() const;

	void SetMipMap(bool bMipMap);
	bool GetMipMap() const;

	osg::StatePtr GetState() const;

	// global option
	static bool s_bTextureCompression;

	// remember this for convenience
	vtImage *m_pImage;

	// the VT material object includes texture
	osg::RefPtr<OSG::SimpleTexturedMaterialPtr> m_pMaterial;
	//osg::RefPtr<OSG::SimpleMaterialPtr> m_pMaterial;
};

/**
 * Contains an array of materials.  Provides useful methods for creating material easily.
 */
class vtMaterialArray : public vtMaterialArrayBase// EXCEPT, public osg::Referenced
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
class vtMesh : public vtMeshBase// EXCEPT, TODO public osg::Referenced
{
	friend class vtGeom;

public:
	vtMesh(enum PrimType ePrimType, int VertType, int NumVertices);

	/// Use this method instead of delete, when you are done with this object.
	void Release();

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
	int GetNumIndices() const { return m_Index->size();};
	short GetIndex(int i) const { return m_Index->getValue(i);};
	int GetPrimLen(int i) const { return m_pPrimSet->getValue(i);};
	void SetNormalsFromPrimitives();

protected:
	// Implementation
	void _AddStripNormals();

	//{ as always there is a node && a node core.
	osg::RefPtr<osg::NodePtr> m_pGeometryNode;
	//osg::NodePtr m_pGeometryNode;
	osg::GeometryPtr m_pGeometryCore;
	//} m_pGeometry

	//the vertices of the mesh
	osg::GeoPositions3fPtr m_Vert;
	//the types of the primitives
	osg::GeoPTypesUI8Ptr m_pPrimSet;
	//the lengths of primitives
	osg::GeoPLengthsUI32Ptr m_Length;
	//the indices
	osg::GeoIndicesUI32Ptr m_Index;
	//texture coodinates
	osg::GeoTexCoords2fPtr m_Tex;
	//the normals
	osg::GeoNormals3fPtr m_Norm;
	//TODO should be geocolors4fptr
	osg::GeoColors3fPtr m_Color;


	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	virtual ~vtMesh() {}
};

/**
 * A Font for use with vtTextMesh.  vtFont can be constructed from any
 * TrueType font on your computer.
 */
class vtFont {
public:
	vtFont();
	~vtFont();
	bool LoadFont(const char *filename);
	osg::FontStyle *GetFontStyle() const { return m_pFontStyle;};

private:
	osg::FontStyle *m_pFontStyle;
};

/**
 * A Text Mesh is a special kind of Mesh which contains text instead of
 * general geometry primitives.  vtTextMesh is used similarly with vtMesh:
 * you create them and add them to vtGeom objects to add them to the scene.
 */
class vtTextMesh //EXCEPT: public osg::Referenced
{
public:
	vtTextMesh(vtFont *font, float fSize = 1, bool bCenter = false);

	/// Use this method instead of delete, when you are done with this object.
	void Release();

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

	// Implementation
	void SetMatIndex(int i) { m_iMatIdx = i;};
	int GetMatIndex() const { return m_iMatIdx;};

	osg::RefPtr<osg::NodePtr> m_pOsgTextNode;
	osg::ScreenAlignedTextPtr m_pOsgText;

protected:
	osg::SharedFontStylePtr m_pSharedFontStyle;
	osg::SharedFontStyleWrapperPtr m_pSharedFontStyleWrapper;
	osg::TransformPtr m_Transform;

	int m_iMatIdx;
	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	~vtTextMesh();
};

/*@}*/	// Group sg


#endif	// VTOSG_MESHMATH

