//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include <osg/BlendFunc>
#include <osg/PolygonMode>

using namespace osg;

//
// Useful helper functions.
//
void makeVec4(Vec4 &col, float r, float g, float b)
{
	col.set(r, g, b, 1.0f);
}

void makeVec4(Vec4 &col, float r, float g, float b, float a)
{
	col.set(r, g, b, a);
}

///////////////////////////////////

#define FAB		Material::FRONT_AND_BACK
#define GEO_ON	StateAttribute::ON
#define GEO_OFF	StateAttribute::OFF

vtMaterial::vtMaterial() : vtMaterialBase()
{
	m_pStateSet = new StateSet;
	m_pMaterial = new Material;
	m_pStateSet->setAttributeAndModes(m_pMaterial.get());

	// Not sure why this is required (should be the default!)
	m_pStateSet->setMode(GL_DEPTH_TEST, StateAttribute::ON);
}

/**
 * Set the diffuse color of this material.
 * \param a		For a material with transparency enabled, the alpha component
 * of the diffuse color determines the overall transparency of the material.
 * This value ranges from 0 (totally transparent) to 1 (totally opaque.)
 *
 */
void vtMaterial::SetDiffuse(float r, float g, float b, float a)
{
	m_pMaterial->setDiffuse(FAB, Vec4(r, g, b, a));

	if (a < 1.0f)
		m_pStateSet->setMode(GL_BLEND, GEO_ON);
}
/**
 * Get the diffuse color of this material.
 */
RGBAf vtMaterial::GetDiffuse()
{
	Vec4 col = m_pMaterial->getDiffuse(FAB);
	return RGBAf(col[0], col[1], col[2], col[3]);
}

/**
 * Set the specular color of this material.
 */
void vtMaterial::SetSpecular(float r, float g, float b)
{
	m_pMaterial->setSpecular(FAB, Vec4(r, g, b, 1.0f));
}
/**
 * Get the specular color of this material.
 */
RGBf vtMaterial::GetSpecular()
{
	Vec4 col = m_pMaterial->getSpecular(FAB);
	return RGBf(col[0], col[1], col[2]);
}

/**
 * Set the ambient color of this material.
 */
void vtMaterial::SetAmbient(float r, float g, float b)
{
	m_pMaterial->setAmbient(FAB, Vec4(r, g, b, 1.0f));
}
/**
 * Get the ambient color of this material.
 */
RGBf vtMaterial::GetAmbient()
{
	Vec4 col = m_pMaterial->getAmbient(FAB);
	return RGBf(col[0], col[1], col[2]);
}

/**
 * Set the emissive color of this material.
 */
void vtMaterial::SetEmission(float r, float g, float b)
{
	m_pMaterial->setEmission(FAB, Vec4(r, g, b, 1.0f));
}
/**
 * Get the emissive color of this material.
 */
RGBf vtMaterial::GetEmission()
{
	Vec4 col = m_pMaterial->getEmission(FAB);
	return RGBf(col[0], col[1], col[2]);
}

/**
 * Set the backface culling property of this material.
 */
void vtMaterial::SetCulling(bool bCulling)
{
	m_pStateSet->setMode(GL_CULL_FACE, bCulling ? GEO_ON : GEO_OFF);
}
/**
 * Get the backface culling property of this material.
 */
bool vtMaterial::GetCulling()
{
	StateAttribute::GLModeValue m = m_pStateSet->getMode(StateAttribute::CULLFACE);
	return (m == GEO_ON);
}

/**
 * Set the lighting property of this material.
 */
void vtMaterial::SetLighting(bool bLighting)
{
	m_pStateSet->setMode(GL_LIGHTING, bLighting ? GEO_ON : GEO_OFF);
}
/**
 * Get the lighting property of this material.
 */
bool vtMaterial::GetLighting()
{
	StateAttribute::GLModeValue m = m_pStateSet->getMode(StateAttribute::LIGHT);
	return (m == GEO_ON);
}

/**
 * Set the transparent property of this material.
 *
 * \param bOn True to turn on transparency (blending).
 * \param bAdd True for additive blending.
 */
void vtMaterial::SetTransparent(bool bOn, bool bAdd)
{
	m_pStateSet->setMode(GL_BLEND, bOn ? GEO_ON : GEO_OFF);
	if (bAdd)
	{
		BlendFunc *trans = new BlendFunc();
//		trans->setFunction(GL_ONE, GL_ONE);
//		trans->setFunction(GL_SRC_COLOR, GL_DST_COLOR);
		trans->setFunction(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		m_pStateSet->setAttribute(trans);
	}
}
/**
 * Get the transparent property of this material.
 */
bool vtMaterial::GetTransparent()
{
	// OSG 0.8.45 and before
//	StateAttribute::GLModeValue m = m_pStateSet->getMode(StateAttribute::TRANSPARENCY);
	// OSG 0.9.0
	StateAttribute::GLModeValue m;
	m = m_pStateSet->getMode(StateAttribute::BLENDFUNC);
	return (m == GEO_ON);
}


/**
 * Set the wireframe property of this material.
 *
 * \param bOn True to turn on wireframe.
 */
void vtMaterial::SetWireframe(bool bOn)
{
	if (bOn)
	{
		PolygonMode *pm = new PolygonMode();
		pm->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
		m_pStateSet->setAttributeAndModes(pm, StateAttribute::OVERRIDE | StateAttribute::ON);
	}
	else
	{
		// turn wireframe off
		m_pStateSet->setModeToInherit(GL_POLYGON_MODE);
	}
}
/**
 * Get the wireframe property of this material.
 */
bool vtMaterial::GetWireframe()
{
	// OSG 0.9.0
	StateAttribute::GLModeValue m;
	m = m_pStateSet->getMode(StateAttribute::POLYGONMODE);
	return (m == GEO_ON);
}

/**
 * Set the texture for this material.
 */
void vtMaterial::SetTexture(vtImage *pImage)
{
	if (!m_pTexture)
		m_pTexture = new Texture2D();

	m_pTexture->setImage(pImage->m_pOsgImage);

	m_pStateSet->setTextureAttributeAndModes(0, m_pTexture.get(),
		StateAttribute::ON);
}

/**
 * Set the texture clamping property for this material.
 */
void vtMaterial::SetClamp(bool bClamp)
{
	if (!m_pTexture)
		return;
	if (bClamp)
	{
		m_pTexture->setWrap(Texture::WRAP_S, Texture::CLAMP);
		m_pTexture->setWrap(Texture::WRAP_T, Texture::CLAMP);
	}
	else
	{
		m_pTexture->setWrap(Texture::WRAP_S, Texture::REPEAT);
		m_pTexture->setWrap(Texture::WRAP_T, Texture::REPEAT);
	}
}

/**
 * Get the texture clamping property of this material.
 */
bool vtMaterial::GetClamp()
{
	if (!m_pTexture)
		return false;
	Texture::WrapMode w = m_pTexture->getWrap(Texture::WRAP_S);
	return (w == Texture::CLAMP);
}

/**
 * Set the texture mipmapping property for this material.
 */
void vtMaterial::SetMipMap(bool bMipMap)
{
	if (!m_pTexture)
		return;
	if (bMipMap)
		m_pTexture->setFilter(Texture::MIN_FILTER, Texture::LINEAR_MIPMAP_LINEAR);
	else
		m_pTexture->setFilter(Texture::MIN_FILTER, Texture::LINEAR);
}

/**
 * Get the texture mipmapping property of this material.
 */
bool vtMaterial::GetMipMap()
{
	if (!m_pTexture)
		return false;
	Texture::FilterMode m = m_pTexture->getFilter(Texture::MIN_FILTER);
	return (m == Texture::LINEAR_MIPMAP_LINEAR);
}

extern State *hack_global_state;

void vtMaterial::Apply()
{
	if (hack_global_state)
		hack_global_state->apply(m_pStateSet.get());
}


//////////////////////////////////////////////////////////////

/**
 * Adds a material to this material array.
 *
 * \return The index of the material that was added.
 */
int vtMaterialArray::AppendMaterial(vtMaterial *pMat)
{
	// nothing special to do
	return Append(pMat);
}


//////////////////////////////////////////////////////////////

#ifndef DOXYGEN_SHOULD_SKIP_THIS

GeoSet2::GeoSet2()
{
	m_pMesh = NULL;
}

GeoSet2::~GeoSet2()
{
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

//////////////////////////////////////////////////////////////

/**
 * Construct a Mesh.  A Mesh is a container for a set of vertices and primitives.
 *
 * \param PrimType The type of primitive this mesh will contain.  Allowed
 *		values are:
 *		- GL_POINTS
 *		- GL_LINES
 *		- GL_LINE_STRIP
 *		- GL_TRIANGLES
 *		- GL_TRIANGLE_STRIP
 *		- GL_TRIANGLE_FAN
 *		- GL_QUADS
 *		- GL_POLYGON
 *
 * \param VertType Flags which indicate what type of information is stored
 *		with each vertex.  This can be any combination of the following bit
 *		flags:
 *		- VT_Normals - a normal per vertex.
 *		- VT_Colors - a color per vertex.
 *		- VT_TexCoords - a texture coordinate (UV) per vertex.
 *
 * \param NumVertices The expected number of vertices that the mesh will
 *		contain.  If more than this number of vertices are added, the mesh
 *		will automatically grow to contain them.  However it is more
 *		efficient if you know the number at creation time and pass it in
 *		this parameter.
 */
vtMesh::vtMesh(GLenum PrimType, int VertType, int NumVertices) :
	vtMeshBase(PrimType, VertType, NumVertices)
{
	m_pGeoSet = new GeoSet2();

	// We own the array allocation, so tell OSG not to try to free it
	m_pGeoSet->setAttributeDeleteFunctor(NULL);

	m_Vert.SetMaxSize(NumVertices);
	// all the primitives we do are indexed, so it's fair to assume that
	// there will probably be at least as many indices as vertices
	m_Index.SetMaxSize(NumVertices);

	m_pGeoSet->setCoords(m_Vert.GetData(), m_Index.GetData());
	m_pGeoSet->setPrimLengths(m_PrimLen.GetData());

	// set backpointer so we can find ourselves later
	m_pGeoSet->m_pMesh = this;

	if (VertType & VT_Normals)
	{
		m_Norm.SetMaxSize(NumVertices);
		m_pGeoSet->setNormals(m_Norm.GetData(), m_Index.GetData());
		m_pGeoSet->setNormalBinding(GeoSet::BIND_PERVERTEX);
	}
	if (VertType & VT_Colors)
	{
		m_Color.SetMaxSize(NumVertices);
		m_pGeoSet->setColors(m_Color.GetData(), m_Index.GetData());
		m_pGeoSet->setColorBinding(GeoSet::BIND_PERVERTEX);
	}
	if (VertType & VT_TexCoords)
	{
		m_Tex.SetMaxSize(NumVertices);
		m_pGeoSet->setTextureCoords(m_Tex.GetData(), m_Index.GetData());
		m_pGeoSet->setTextureBinding(GeoSet::BIND_PERVERTEX);
	}

	switch (PrimType)
	{
	case GL_POINTS:
		m_pGeoSet->setPrimType(GeoSet::POINTS);
		m_pGeoSet->setNumPrims(NumVertices);
		m_bIndexedPrims = false;
		break;
	case GL_LINES:
		m_pGeoSet->setPrimType(GeoSet::LINES);
		m_pGeoSet->setNumPrims(NumVertices/2);
		m_bIndexedPrims = true;
		break;
	case GL_LINE_STRIP:
		m_pGeoSet->setPrimType(GeoSet::LINE_STRIP);
		m_bIndexedPrims = true;
		break;
	case GL_TRIANGLES:
		m_PrimLen.SetMaxSize(NumVertices/3);
		m_pGeoSet->setPrimType(GeoSet::TRIANGLES);
		m_pGeoSet->setNumPrims(NumVertices/3);
		m_bIndexedPrims = true;
		break;
	case GL_TRIANGLE_STRIP:
		m_pGeoSet->setPrimType(GeoSet::TRIANGLE_STRIP);
		m_bIndexedPrims = true;
		break;
	case GL_TRIANGLE_FAN:
		m_pGeoSet->setPrimType(GeoSet::TRIANGLE_FAN);
		m_bIndexedPrims = true;
		break;
	case GL_QUADS:
		m_pGeoSet->setPrimType(GeoSet::QUADS);
		m_bIndexedPrims = true;
		break;
	case GL_POLYGON:
		m_pGeoSet->setPrimType(GeoSet::POLYGON);
		m_bIndexedPrims = true;
		break;
	}
	SendPointersToOSG();
}

void vtMesh::SendPointersToOSG()
{
	// in case they got reallocated, tell OSG again
	if (m_bIndexedPrims)
	{
		m_pGeoSet->setCoords(m_Vert.GetData(), m_Index.GetData());
		m_pGeoSet->setPrimLengths(m_PrimLen.GetData());
	}
	else
		m_pGeoSet->setCoords(m_Vert.GetData());

	if (m_iVtxType & VT_Normals)
		m_pGeoSet->setNormals(m_Norm.GetData(), m_Index.GetData());
	if (m_iVtxType & VT_Colors)
		m_pGeoSet->setColors(m_Color.GetData(), m_Index.GetData());
	if (m_iVtxType & VT_TexCoords)
		m_pGeoSet->setTextureCoords(m_Tex.GetData(), m_Index.GetData());
}

void vtMesh::AddTri(int p0, int p1, int p2)
{
	m_Index.Append(p0);
	m_Index.Append(p1);
	m_Index.Append(p2);
	m_PrimLen.Append(3);
	m_pGeoSet->setNumPrims(m_Index.GetSize() / 3);

	SendPointersToOSG();
}

void vtMesh::AddFan(int p0, int p1, int p2, int p3, int p4, int p5)
{
	int len = 2;

	m_Index.Append(p0);
	m_Index.Append(p1);

	if (p2 != -1) { m_Index.Append(p2); len = 3; }
	if (p3 != -1) { m_Index.Append(p3); len = 4; }
	if (p4 != -1) { m_Index.Append(p4); len = 5; }
	if (p5 != -1) { m_Index.Append(p5); len = 6; }

	m_PrimLen.Append(len);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToOSG();
}

void vtMesh::AddFan(int *idx, int iNVerts)
{
	for (int i = 0; i < iNVerts; i++)
		m_Index.Append(idx[i]);

	m_PrimLen.Append(iNVerts);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToOSG();
}

void vtMesh::AddStrip(int iNVerts, unsigned short *pIndices)
{
	for (int i = 0; i < iNVerts; i++)
		m_Index.Append(pIndices[i]);

	m_PrimLen.Append(iNVerts);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToOSG();
}

void vtMesh::AddQuadStrip(int iNVerts, int iStartIndex)
{
	// for OSG, stored the same
	AddStrip2(iNVerts, iStartIndex);
}

void vtMesh::AddLine(int p0, int p1)
{
	m_Index.Append(p0);
	m_Index.Append(p1);

	SendPointersToOSG();
}

void vtMesh::SetVtxPos(int i, const FPoint3 &p)
{
	Vec3 s;
	v2s(p, s);
	m_Vert.SetAt(i, s);
}

FPoint3 vtMesh::GetVtxPos(int i) const
{
	FPoint3 p;
	s2v( m_Vert.GetAt(i), p);
	return p;
}

void vtMesh::SetVtxNormal(int i, const FPoint3 &p)
{
	Vec3 s;
	v2s(p, s);
	m_Norm.SetAt(i, s);
}

FPoint3 vtMesh::GetVtxNormal(int i) const
{
	FPoint3 p;
	s2v( m_Norm.GetAt(i), p);
	return p;
}

void vtMesh::SetVtxColor(int i, const RGBf &p)
{
	Vec4 s;
	v2s(p, s);
	m_Color.SetAt(i, s);
}

RGBf vtMesh::GetVtxColor(int i) const
{
	RGBf p;
	s2v( m_Color.GetAt(i), p);
	return p;
}

void vtMesh::SetVtxTexCoord(int i, const FPoint2 &p)
{
	Vec2 s;
	v2s(p, s);
	m_Tex.SetAt(i, s);
}

FPoint2 vtMesh::GetVtxTexCoord(int i)
{
	FPoint2 p;
	s2v( m_Tex.GetAt(i), p);
	return p;
}

int vtMesh::GetNumPrims()
{
	return m_pGeoSet->getNumPrims();
}

void vtMesh::ReOptimize()
{
	m_pGeoSet->dirtyDisplayList();
}

void vtMesh::AllowOptimize(bool bAllow)
{
	m_pGeoSet->setUseDisplayList(bAllow);
}


/////////////////////////////////////////////////////////////////////////////
// Text

vtFont::vtFont()
{
}

bool vtFont::LoadFont(const char *filename)
{
	m_pOsgFont = new osgText::PolygonFont(filename, 24, 3);

	// TODO: is this the correct way to check for success?
//	if (m_pOsgFont->getHeight() == -1)
//		return false;

	return true;
}

////

vtTextMesh::vtTextMesh(vtFont *font, bool bCenter)
{
	m_pOsgText = new osgText::Text(font->m_pOsgFont.get());
	if (bCenter)
		m_pOsgText->setAlignment(osgText::Text::CENTER_BOTTOM);
}

void vtTextMesh::SetText(const char *text)
{
	m_pOsgText->setText(text);
}
