//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include <osg/PolygonMode>
using namespace osg;

///////////////////////////////////

#define FAB		Material::FRONT_AND_BACK
#define SA_ON	StateAttribute::ON
#define SA_OFF	StateAttribute::OFF

vtMaterial::vtMaterial() : vtMaterialBase()
{
	m_pStateSet = new StateSet;
	m_pMaterial = new Material;
	m_pStateSet->setAttributeAndModes(m_pMaterial.get());

	// Not sure why this is required (should be the default!)
	m_pStateSet->setMode(GL_DEPTH_TEST, SA_ON);
}

vtMaterial::~vtMaterial()
{
	// do these manually, although it's not really required
	m_pMaterial = NULL;
	m_pTexture = NULL;
	m_pStateSet = NULL;
	m_pBlendFunc = NULL;

	// more dereferencing
	m_pImage = NULL;
}

/**
 * Set the diffuse color of this material.
 *
 * \param r,g,b	The rgb value (0.0 to 1.0) of this material
 * \param a		For a material with transparency enabled, the alpha component
 * of the diffuse color determines the overall transparency of the material.
 * This value ranges from 0 (totally transparent) to 1 (totally opaque.)
 *
 */
void vtMaterial::SetDiffuse(float r, float g, float b, float a)
{
	m_pMaterial->setDiffuse(FAB, Vec4(r, g, b, a));

	if (a < 1.0f)
		m_pStateSet->setMode(GL_BLEND, SA_ON);
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
	m_pStateSet->setMode(GL_CULL_FACE, bCulling ? SA_ON : SA_OFF);
}
/**
 * Get the backface culling property of this material.
 */
bool vtMaterial::GetCulling()
{
	StateAttribute::GLModeValue m;
	m = m_pStateSet->getMode(GL_CULL_FACE);
	return (m == SA_ON);
}

/**
 * Set the lighting property of this material.
 */
void vtMaterial::SetLighting(bool bLighting)
{
	m_pStateSet->setMode(GL_LIGHTING, bLighting ? SA_ON : SA_OFF);
}
/**
 * Get the lighting property of this material.
 */
bool vtMaterial::GetLighting()
{
	StateAttribute::GLModeValue m;
	m = m_pStateSet->getMode(GL_LIGHTING);
	return (m == SA_ON);
}

/**
 * Set the transparent property of this material.
 *
 * \param bOn True to turn on transparency (blending).
 * \param bAdd True for additive blending.
 */
void vtMaterial::SetTransparent(bool bOn, bool bAdd)
{
//	m_pStateSet->setMode(GL_BLEND, bOn ? SA_ON : SA_OFF);
	if (bOn)
	{
		if (!m_pBlendFunc.valid())
			m_pBlendFunc = new BlendFunc;
		m_pStateSet->setAttributeAndModes(m_pBlendFunc.get(), SA_ON);
		if (!m_pAlphaFunc.valid())
			m_pAlphaFunc = new AlphaFunc;
		m_pAlphaFunc->setFunction(AlphaFunc::GEQUAL,0.05f);
		m_pStateSet->setAttributeAndModes(m_pAlphaFunc.get(), SA_ON );
		m_pStateSet->setRenderingHint(StateSet::TRANSPARENT_BIN);
	}
	else
	{
		m_pStateSet->setMode(GL_BLEND, SA_OFF);
		m_pStateSet->setRenderingHint( StateSet::OPAQUE_BIN );
	}

	if (bAdd)
	{
		if (!m_pBlendFunc.valid())
			m_pBlendFunc = new BlendFunc;
//		m_pBlendFunc->setFunction(GL_ONE, GL_ONE);
//		m_pBlendFunc->setFunction(GL_SRC_COLOR, GL_DST_COLOR);
		m_pBlendFunc->setFunction(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		m_pStateSet->setAttribute(m_pBlendFunc.get());
	}
}
/**
 * Get the transparent property of this material.
 */
bool vtMaterial::GetTransparent()
{
	// OSG 0.8.45 and before
//	StateAttribute::GLModeValue m = m_pStateSet->getMode(StateAttribute::TRANSPARENCY);
	// OSG 0.9.0 onwards
	StateAttribute::GLModeValue m;
	m = m_pStateSet->getMode(StateAttribute::BLENDFUNC);
	return (m == SA_ON);
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
		m_pStateSet->setAttributeAndModes(pm, StateAttribute::OVERRIDE | SA_ON);
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
	return (m == SA_ON);
}

/**
 * Set the texture for this material.
 */
void vtMaterial::SetTexture(vtImage *pImage)
{
	if (!m_pTexture)
		m_pTexture = new Texture2D();

	m_pTexture->setImage(pImage->m_pOsgImage.get());

	// store a reference so that it won't get deleted without this material's permission
	m_pImage = pImage;

	// From the OSG list: "Why doesn't the OSG deallocate image buffer right
	// *after* a glTextImage2D?
	// By default the OSG doesn't do it bacause the user may actually want to
	// do things with the image after its been bound.  You can make the
	// osg::Texture classes unref their images automatically by doing:
	// texture->setUnRefImageDataAfterApply(true);

	// So i tried this, but it doesn't seem to have any affect on runtime memory
	//  footprint:
//	m_pTexture->setUnRefImageDataAfterApply(true);

	m_pStateSet->setTextureAttributeAndModes(0, m_pTexture.get(), SA_ON);
}

/**
 * Loads and sets the texture for a material.
 */
void vtMaterial::SetTexture2(const char *szFilename)
{
	vtImage *image = new vtImage(szFilename);
	if (image->LoadedOK())
	{
		SetTexture(image);
	}
	image->Release();	// don't hold on to it; it will either be owned by this material, or deleted
}


/**
 * Returns the texture (image) associated with a material.
 */
vtImage	*vtMaterial::GetTexture()
{
	return m_pImage.get();
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


/////////////////////////////////////////////////////////////////////////////
// vtMaterialArray
//

vtMaterialArray::vtMaterialArray()
{
	ref();		// artficially set refcount to 1
}

vtMaterialArray::~vtMaterialArray()
{
}

void vtMaterialArray::Release()
{
	unref();	// trigger self-deletion if no more references
}

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


/////////////////////////////////////////////////////////////////////////////
// vtMesh
//

/**
 * Construct a Mesh.
 * A Mesh is a container for a set of vertices and primitives.
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
	m_pGeoSet = new GeoSet();

	// set backpointer so we can find ourselves later
	// also increases our own refcount
	m_pGeoSet->setUserData(this);

	// We own the array allocation, so tell OSG not to try to free it
	m_pGeoSet->setAttributeDeleteFunctor(NULL);

	m_Vert.SetMaxSize(NumVertices);

	// All the primitives except GL_POINTS are indexed, so it's fair to
	//  assume that there will be at least as many indices as vertices.
	if (PrimType != GL_POINTS)
		m_Index.SetMaxSize(NumVertices);

	m_pGeoSet->setCoords(m_Vert.GetData(), m_Index.GetData());
	m_pGeoSet->setPrimLengths(m_PrimLen.GetData());

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
		break;
	case GL_LINES:
		m_pGeoSet->setPrimType(GeoSet::LINES);
		m_pGeoSet->setNumPrims(NumVertices/2);
		break;
	case GL_LINE_STRIP:
		m_pGeoSet->setPrimType(GeoSet::LINE_STRIP);
		break;
	case GL_TRIANGLES:
		m_PrimLen.SetMaxSize(NumVertices/3);
		m_pGeoSet->setPrimType(GeoSet::TRIANGLES);
		m_pGeoSet->setNumPrims(NumVertices/3);
		break;
	case GL_TRIANGLE_STRIP:
		m_pGeoSet->setPrimType(GeoSet::TRIANGLE_STRIP);
		break;
	case GL_TRIANGLE_FAN:
		m_pGeoSet->setPrimType(GeoSet::TRIANGLE_FAN);
		break;
	case GL_QUADS:
		m_pGeoSet->setPrimType(GeoSet::QUADS);
		break;
	case GL_POLYGON:
		m_pGeoSet->setPrimType(GeoSet::POLYGON);
		break;
	}
	SendPointersToOSG();
}

vtMesh::~vtMesh()
{
}

void vtMesh::Release()
{
	// explicit dereference. if Release is not called, this dereference should
	//  also occur implicitly in the destructor
	m_pGeoSet = NULL;
}

/**
 * Add a triangle.
 *  p0, p1, p2 are the indices of the vertices of the triangle.
 */
void vtMesh::AddTri(int p0, int p1, int p2)
{
	m_Index.Append(p0);
	m_Index.Append(p1);
	m_Index.Append(p2);
	m_PrimLen.Append(3);
	m_pGeoSet->setNumPrims(m_Index.GetSize() / 3);

	SendPointersToOSG();
}

/**
 * Add a triangle fan with up to 6 points (center + 5 points).  The first 3
 * arguments are required, the rest are optional.  A fan will be created
 * with as many point indices as you pass.
 */
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

/**
 * Add a triangle fan with any number of points.
 *	\param idx An array of vertex indices for the fan.
 *	\param iNVerts the number of vertices in the fan.
 */
void vtMesh::AddFan(int *idx, int iNVerts)
{
	for (int i = 0; i < iNVerts; i++)
		m_Index.Append(idx[i]);

	m_PrimLen.Append(iNVerts);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToOSG();
}

/**
 * Adds an indexed triangle strip to the mesh.
 *
 * \param iNVerts The number of vertices in the strip.
 * \param pIndices An array of the indices of the vertices in the strip.
 */
void vtMesh::AddStrip(int iNVerts, unsigned short *pIndices)
{
	for (int i = 0; i < iNVerts; i++)
		m_Index.Append(pIndices[i]);

	m_PrimLen.Append(iNVerts);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToOSG();
}

/**
 * Add a single line primitive to a mesh.
 *	\param p0, p1	The indices of the two vertices of the line.
 */
void vtMesh::AddLine(int p0, int p1)
{
	m_Index.Append(p0);
	m_Index.Append(p1);

	SendPointersToOSG();
}

/**
 * Set the position of a vertex.
 *	\param i	Index of the vertex.
 *	\param p	The position.
 */
void vtMesh::SetVtxPos(int i, const FPoint3 &p)
{
	Vec3 s;
	v2s(p, s);
	m_Vert.SetAt(i, s);

	if (m_ePrimType == GL_POINTS)
		m_pGeoSet->setNumPrims(m_Vert.GetSize());
}

/**
 * Get the position of a vertex.
 */
FPoint3 vtMesh::GetVtxPos(int i) const
{
	FPoint3 p;
	s2v( m_Vert.GetAt(i), p);
	return p;
}

/**
 * Set the normal of a vertex.  This is used for lighting, if the mesh
 *	is used with a material with lighting enabled.  Generally you will
 *	want to use a vector of unit length.
 *
 *	\param i	Index of the vertex.
 *	\param norm	The normal vector.
 */
void vtMesh::SetVtxNormal(int i, const FPoint3 &norm)
{
	Vec3 s;
	v2s(norm, s);
	m_Norm.SetAt(i, s);
}

/**
 * Get the normal of a vertex.
 */
FPoint3 vtMesh::GetVtxNormal(int i) const
{
	FPoint3 p;
	s2v( m_Norm.GetAt(i), p);
	return p;
}

/**
 * Set the color of a vertex.  This color multiplies with the color of the
 *	material used with the mesh, so if you want the vertex color to be
 *	dominant, use a white material.
 *
 *	\param i		Index of the vertex.
 *	\param color	The color.
 */
void vtMesh::SetVtxColor(int i, const RGBf &color)
{
	Vec4 s;
	v2s(color, s);
	m_Color.SetAt(i, s);
}

/**
 * Get the color of a vertex.
 */
RGBf vtMesh::GetVtxColor(int i) const
{
	RGBf p;
	s2v( m_Color.GetAt(i), p);
	return p;
}

/**
 * Set the texture coordinates of a vertex.  Generally these values are
 *	in the range of 0 to 1, although you can use higher values if you want
 *	repeating tiling.  The components of the texture coordinates are
 *  usually called "u" and "v".
 *
 *	\param i	Index of the vertex.
 *	\param uv	The texture coordinate.
 */
void vtMesh::SetVtxTexCoord(int i, const FPoint2 &uv)
{
	Vec2 s;
	v2s(uv, s);
	m_Tex.SetAt(i, s);
}

/**
 * Get the texture coordinates of a vertex.
 */
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

/**
 * Set whether to allow rendering optimization of this mesh.  With OpenGL,
 *	this optimization is called a "display list", which increases the speed
 *	of rendering by creating a special representation of the mesh the first
 *	time it is drawn.  The tradeoff is that subsequent changes to the mesh
 *	are not applied unless you call ReOptimize().
 *
 *	\param bAllow	True to allow optimization.  The default is true.
 */
void vtMesh::AllowOptimize(bool bAllow)
{
	m_pGeoSet->setUseDisplayList(bAllow);
}

/**
 * For a mesh with rendering optimization enabled, forces an update of the
 *	optimized representation.
 */
void vtMesh::ReOptimize()
{
	m_pGeoSet->dirtyDisplayList();
}

/**
 * Set the normals of the vertices by combining the normals of the
 * surrounding faces.  This requires going through all the primitives
 * to average their contribution to each vertex.
 */
void vtMesh::SetNormalsFromPrimitives()
{
	int verts = GetNumVertices();
	int i;

	for (i = 0; i < verts; i++)
		m_Norm[i].set(0,0,0);

	switch (m_ePrimType)
	{
	case GL_POINTS:
	case GL_LINES:
	case GL_LINE_STRIP:
	case GL_TRIANGLES:
		break;
	case GL_TRIANGLE_STRIP:
		_AddStripNormals();
		break;
	case GL_TRIANGLE_FAN:
	case GL_QUADS:
	case GL_POLYGON:
		break;
	}

	for (i = 0; i < verts; i++)
		m_Norm[i].normalize();
}

void vtMesh::_AddStripNormals()
{
	int prims = GetNumPrims();
	int i, j, len, idx;
	unsigned short v0 = 0, v1 = 0, v2 = 0;
	osg::Vec3 p0, p1, p2, d0, d1, norm;

	idx = 0;
	for (i = 0; i < prims; i++)
	{
		len = m_PrimLen[i];
		for (j = 0; j < len; j++)
		{
			v0 = v1; p0 = p1;
			v1 = v2; p1 = p2;
			v2 = m_Index[idx];
			p2 = m_Vert[v2];
			if (j >= 2)
			{
				d0 = (p1 - p0);
				d1 = (p2 - p0);
				d0.normalize();
				d1.normalize();

				norm = d0^d1;

				m_Norm[v0] += norm;
				m_Norm[v1] += norm;
				m_Norm[v2] += norm;
			}
			idx++;
		}
	}
}


//
// Point OSG to the vertex and primitive data that we maintain
//
void vtMesh::SendPointersToOSG()
{
	// in case they got reallocated, tell OSG again
	if (m_ePrimType == GL_POINTS)
	{
		m_pGeoSet->setCoords(m_Vert.GetData());
		if (m_iVtxType & VT_Colors)
			m_pGeoSet->setColors(m_Color.GetData());
	}
	else
	{
		m_pGeoSet->setCoords(m_Vert.GetData(), m_Index.GetData());
		if (m_iVtxType & VT_Normals)
			m_pGeoSet->setNormals(m_Norm.GetData(), m_Index.GetData());
		if (m_iVtxType & VT_Colors)
			m_pGeoSet->setColors(m_Color.GetData(), m_Index.GetData());
		if (m_iVtxType & VT_TexCoords)
			m_pGeoSet->setTextureCoords(m_Tex.GetData(), m_Index.GetData());
	}

	// three geometry types don't use 'primitive lengths'
	int NumVertices = m_Vert.GetSize();
	int NumIndices = m_Index.GetSize();
	switch (m_ePrimType)
	{
	case GL_POINTS:
		m_pGeoSet->setNumPrims(NumVertices);
		break;
	case GL_LINES:
		m_pGeoSet->setNumPrims(NumIndices/2);
		break;
	case GL_TRIANGLES:
		m_pGeoSet->setNumPrims(NumIndices/3);
		break;
	default:
		// all the other types do
		m_pGeoSet->setPrimLengths(m_PrimLen.GetData());
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Text

vtFont::vtFont()
{
}

bool vtFont::LoadFont(const char *filename)
{
	// OSG 0.9.3
//	m_pOsgFont = new osgText::Font(filename, 24, 3);

	// OSG 0.9.4
	m_pOsgFont = osgText::readFontFile(filename);

	if (m_pOsgFont == NULL)
		return false;

	return true;
}

////

vtTextMesh::vtTextMesh(vtFont *font, float fSize, bool bCenter)
{
	// OSG 0.9.3
//	m_pOsgText = new osgText::Text(font->m_pOsgFont.get());

	// OSG 0.9.4
	m_pOsgText = new osgText::Text;
	m_pOsgText->setFont(font->m_pOsgFont.get());

	// set backpointer so we can find ourselves later
	m_pOsgText->setUserData(this);

	// Set the Font reference width and height resolution in texels.
	m_pOsgText->setFontSize(32,32);

	// Set the rendered character size in object coordinates.
	m_pOsgText->setCharacterSize(fSize);

	if (bCenter)
		m_pOsgText->setAlignment(osgText::Text::CENTER_BOTTOM);
}

vtTextMesh::~vtTextMesh()
{
}

void vtTextMesh::Release()
{
	m_pOsgText = NULL;	// dereference
}

void vtTextMesh::SetText(const char *text)
{
	m_pOsgText->setText(text);
}

void vtTextMesh::SetText(const wchar_t *text)
{
	m_pOsgText->setText(text);
}

void vtTextMesh::SetText(const std::wstring &text)
{
	m_pOsgText->setText(text.c_str());
}

void vtTextMesh::SetPosition(const FPoint3 &pos)
{
	Vec3 s;
	v2s(pos, s);
	m_pOsgText->setPosition(s);
}

void vtTextMesh::SetAlignment(int align)
{
	m_pOsgText->setAlignment((osgText::Text::AlignmentType) align);
}

