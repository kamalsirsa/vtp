//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

/////////////////////////////////////////////////////////////////////////////
// vtMaterialArray
//
vtMaterialArray::vtMaterialArray()
{
	m_pApps = new PSAppearances();
}

vtMaterialArray::~vtMaterialArray()
{
	delete m_pApps;
}

void vtMaterialArray::CopyFrom(vtMaterialArray *pFromMats)
{
	m_pApps->Merge(* (pFromMats->m_pApps));
}

int vtMaterialArray::AppendMaterial(vtMaterial *pMat)
{
	Append(pMat);
	return m_pApps->Append(pMat->m_pApp);
}


/////////////////////////////////////////////////////////////////////////////
// vtMaterial
//
vtMaterial::vtMaterial()
{
	m_pApp = new MyPSAppearance();
}

vtMaterial::~vtMaterial()
{
	delete m_pApp;
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
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl == NULL)
	{
		mtl = new PSMaterial(PSCol4(r, g, b, a));
		m_pApp->SetMaterial(mtl);
	}
	else
		mtl->SetDiffuse(PSCol4(r, g, b, a));
}

/**
 * Get the diffuse color of this material.
 */
RGBAf vtMaterial::GetDiffuse()
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl)
		return (RGBAf&) mtl->GetDiffuse();
	else
		return RGBAf(1, 1, 1, 1);
}

/**
 * Set the specular color of this material.
 */
void vtMaterial::SetSpecular(float r, float g, float b)
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl == NULL)
	{
		mtl = new PSMaterial;
		mtl->SetSpecular(PSCol4(r, g, b));
		m_pApp->SetMaterial(mtl);
	}
	else
		mtl->SetSpecular(PSCol4(r, g, b));
}

void vtMaterial::SetSpecular2(float shine)
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl == NULL)
	{
		mtl = new PSMaterial;
		mtl->SetShine(shine);
		m_pApp->SetMaterial(mtl);
	}
	else
		mtl->SetShine(shine);
}

/**
 * Get the specular color of this material.
 */
RGBf vtMaterial::GetSpecular()
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl)
	{
		PSCol4 c = mtl->GetSpecular();
		return RGBf(c.r, c.g, c.b);
	}
	else
		return RGBf(0, 0, 0);
}

/**
 * Set the ambient color of this material.
 */
void vtMaterial::SetAmbient(float r, float g, float b)
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl == NULL)
	{
		mtl = new PSMaterial;
		mtl->SetAmbient(PSCol4(r, g, b));
		m_pApp->SetMaterial(mtl);
	}
	else
		mtl->SetAmbient(PSCol4(r, g, b));
}

/**
 * Get the ambient color of this material.
 */
RGBf vtMaterial::GetAmbient()
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl)
	{
		PSCol4 c = mtl->GetAmbient();
		return RGBf(c.r, c.g, c.b);
	}
	else
		return RGBf(0, 0, 0);
}

/**
 * Set the emissive color of this material.
 */
void vtMaterial::SetEmission(float r, float g, float b)
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl == NULL)
	{
		mtl = new PSMaterial;
		mtl->SetEmission(PSCol4(r, g, b));
		m_pApp->SetMaterial(mtl);
	}
	else
		mtl->SetEmission(PSCol4(r, g, b));
}
/**
 * Get the emissive color of this material.
 */
RGBf vtMaterial::GetEmission()
{
	PSMaterial* mtl = m_pApp->GetMaterial();
	if (mtl)
	{
		PSCol4 c = mtl->GetEmission();
		return RGBf(c.r, c.g, c.b);
	}
	else
		return RGBf(0, 0, 0);
}

/**
 * Set the backface culling property of this material.
 */
void vtMaterial::SetCulling(bool bCulling)
{
	m_pApp->Set(APPEAR_Culling, bCulling);
}
/**
 * Get the backface culling property of this material.
 */
bool vtMaterial::GetCulling()
{
	if (m_pApp->Get(APPEAR_Culling) > 0)
		return true;
	return false;
}

/**
 * Set the lighting property of this material.
 */
void vtMaterial::SetLighting(bool bLighting)
{
	m_pApp->Set(APPEAR_Lighting, bLighting);
}

/**
 * Get the lighting property of this material.
 */
bool vtMaterial::GetLighting()
{
	if (m_pApp->Get(APPEAR_Lighting) > 0)
		return true;
	return false;
}

/**
 * Set the transparent property of this material.
 *
 * \param bOn True to turn on transparency (blending).
 * \param bAdd True for additive blending.
 */
void vtMaterial::SetTransparent(bool bOn, bool bAdd)
{
	m_pApp->Set(APPEAR_Transparency, APPEAR_Blend);
	if (bAdd)
	{
		m_pApp->Set(APPEAR_SrcBlend, APPEAR_WhiteColor);
		m_pApp->Set(APPEAR_DstBlend, APPEAR_SrcColor | APPEAR_Invert);
	}
	else
	{
		m_pApp->Set(APPEAR_SrcBlend, APPEAR_SrcAlpha);
		m_pApp->Set(APPEAR_DstBlend, APPEAR_SrcAlpha | APPEAR_Invert);
	}
}

/**
 * Get the transparent property of this material.
 */
bool vtMaterial::GetTransparent()
{
	if (m_pApp->Get(APPEAR_Transparency) > 0)
		return true;
	return false;
}


/**
 * Set the wireframe property of this material.
 *
 * \param bOn True to turn on wireframe.
 */
void vtMaterial::SetWireframe(bool bOn)
{
	m_pApp->Set(APPEAR_Shading, APPEAR_Wire);
}

/**
 * Get the wireframe property of this material.
 */
bool vtMaterial::GetWireframe()
{
	if (m_pApp->Get(APPEAR_Shading) == APPEAR_Wire)
		return true;
	return false;
}

/**
 * Set the texture clamping property for this material.
 */
void vtMaterial::SetClamp(bool bClamp)
{
//	if (!m_pTexture)
//		return;
//	if (bClamp)
//	{
//		m_pTexture->setWrap(Texture::WRAP_S, Texture::CLAMP);
//		m_pTexture->setWrap(Texture::WRAP_T, Texture::CLAMP);
//	}
//	else
//	{
//		m_pTexture->setWrap(Texture::WRAP_S, Texture::REPEAT);
//		m_pTexture->setWrap(Texture::WRAP_T, Texture::REPEAT);
//	}
}

/**
 * Get the texture clamping property of this material.
 */
bool vtMaterial::GetClamp()
{
//	if (!m_pTexture)
		return false;
//	Texture::WrapMode w = m_pTexture->getWrap(Texture::WRAP_S);
//	return (w == Texture::CLAMP);
}

/**
 * Set the texture mipmapping property for this material.
 */
void vtMaterial::SetMipMap(bool bMipMap)
{
	if (bMipMap)
		m_pApp->Set(APPEAR_MinFilter, APPEAR_Linear | APPEAR_MipMap);
	else
		m_pApp->Set(APPEAR_MinFilter, APPEAR_Linear);
}

/**
 * Get the texture mipmapping property of this material.
 */
bool vtMaterial::GetMipMap()
{
	if (m_pApp->Get(APPEAR_MinFilter) & APPEAR_MipMap)
		return true;
	return false;
}

vtImage* vtMaterial::GetTexture()
{
	return (vtImage*) m_pApp->GetImage();
}

void vtMaterial::SetTexture(vtImage *pImage)
{
	m_pApp->SetImage(pImage);
	m_pApp->Set(APPEAR_Texturing, pImage != NULL);
}

void vtMaterial::SetTexture2(const char* s)
{
	vtImage *i = new vtImage(s);
	m_pApp->SetImage(i);
	m_pApp->Set(APPEAR_Texturing, true);
}

void vtMaterial::Apply()
{
	m_pApp->MyApply();
}

void MyPSAppearance::MyApply()
{
	PSAppearance::LoadDev(PSGetScene());
}

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
	PSTriMesh(VertType, NumVertices), vtMeshBase(PrimType, VertType, NumVertices)
{
	PSVtxArray* verts = GetVertices();
	if (verts == NULL)
		SetVertices(new PSVtxArray(VertType, 4));
	m_ePrimType = PrimType;
}


void vtMesh::AddTri(int p0, int p1, int p2)
{
	PSTriPrim prim(GEO_Triangle, 3);
	prim.VtxIndex = GetNumIdx();
	AddIndex(p0);
	AddIndex(p1);
	AddIndex(p2);
	AddPrim(prim, NULL);
}

/**
 * Adds a vertex to the mesh.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertex(float x, float y, float z)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, PSVec3(x, y, z));
	}
	return i;
}

/**
 * Adds a vertex to the mesh, with a vertex normal.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexN(float x, float y, float z, float nx, float ny, float nz)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, PSVec3(x, y, z));
		verts->SetNormal(i, PSVec3(nx, ny, nz));
	}
	return i;
}

/**
 * Adds a vertex to the mesh, with UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexUV(float x, float y, float z, float u, float v)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, PSVec3(x, y, z));
		verts->SetTexCoord(i, PSTexCoord(u, v));
	}
	return i;
}

/**
 * Adds a vertex to the mesh.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertex(const FPoint3 &p)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, v2p(p));
	}
	return i;
}

/**
 * Adds a vertex to the mesh, with a vertex normal.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexN(const FPoint3 &p, const FPoint3 &n)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, v2p(p));
		verts->SetNormal(i, v2p(n));
	}
	return i;
}

/**
 * Adds a vertex to the mesh, with UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexUV(const FPoint3 &p, float u, float v)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, v2p(p));
		verts->SetTexCoord(i, PSTexCoord(u, v));
	}
	return i;
}

/**
 * Adds a vertex to the mesh, with UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexUV(const FPoint3 &p, const FPoint2 &uv)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, v2p(p));
		verts->SetTexCoord(i, PSTexCoord(uv.x, uv.y));
	}
	return i;
}

/**
 * Adds a vertex to the mesh, with a vertex normal and UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexNUV(const FPoint3 &p, const FPoint3 &n, const FPoint2 &uv)
{
	PSVtxArray* verts = GetVertices();
	int i = 0;
	if (verts)
	{
		i = verts->GetNumVtx();
		verts->SetLoc(i, v2p(p));
		verts->SetNormal(i, v2p(n));
		verts->SetTexCoord(i, PSTexCoord(uv.x, uv.y));
	}
	return i;
}

/**
 * Adds an indexed strip to the mesh, with the assumption that the indices
 * are in linear order.
 *
 * \param iNVerts The number of vertices in the strip.
 * \param iStartIndex The index that starts the linear sequence.
 *
 * \return The index of the vertex that was added.
 */
void vtMesh::AddStrip2(int iNVerts, int iStartIndex)
{
	PSTriPrim prim(GEO_Strip, iNVerts);
	prim.VtxIndex = GetNumIdx();		/* where the indices for this primitive start */
	AddPrim(prim, GEO_MakeIndexData, iStartIndex);
}


void vtMesh::AddFan(int p0, int p1, int p2, int p3, int p4, int p5)
{
	PSTriPrim prim(GEO_Fan, 2);
	prim.VtxIndex = GetNumIdx();
	AddIndex(p0);
	AddIndex(p1);
	if (p2 != -1) { AddIndex(p2); prim.Size = 3; }
	if (p3 != -1) { AddIndex(p3); prim.Size = 4; }
	if (p4 != -1) { AddIndex(p4); prim.Size = 5; }
	if (p5 != -1) { AddIndex(p5); prim.Size = 6; }
	AddPrim(prim, NULL);
}

void vtMesh::AddFan(int *idx, int iNVerts)
{
	PSTriPrim prim(GEO_Fan, iNVerts);
	prim.VtxIndex = GetNumIdx();		/* where the indices for this primitive start */
	for (int i = 0; i < iNVerts; i++)
		AddIndex(idx[i]);
	AddPrim(prim, NULL);
}

void vtMesh::AddStrip(int iNVerts, unsigned short *pIndices)
{
	PSTriPrim prim(GEO_Strip, iNVerts);
	prim.VtxIndex = GetNumIdx();		/* where the indices for this primitive start */
	for (int i = 0; i < iNVerts; i++)
		AddIndex(pIndices[i]);
	AddPrim(prim, NULL);
}

void vtMesh::AddLine(int p0, int p1)
{
	AddIndex(p0);
	AddIndex(p1);
}

void vtMesh::SetVtxPos(int i, const FPoint3 &p)
{
	PSVtxArray* verts = GetVertices();
	if (verts)
		verts->SetLoc(i, v2p(p));
}

FPoint3 vtMesh::GetVtxPos(int i) const
{
	const PSVtxArray* verts = GetVertices();
	if (verts)
		return p2v(verts->GetLoc(i));
	else
		return FPoint3(0, 0, 0);
}

void vtMesh::SetVtxNormal(int i, const FPoint3 &p)
{
	PSVtxArray* verts = GetVertices();
	if (verts)
		verts->SetNormal(i, v2p(p));
}

FPoint3 vtMesh::GetVtxNormal(int i) const
{
	const PSVtxArray* verts = GetVertices();
	if (verts)
		return p2v(verts->GetNormal(i));
	else
		return FPoint3(0, 0, 0);
}

void vtMesh::SetVtxColor(int i, const RGBf &p)
{
	PSVtxArray* verts = GetVertices();
	if (verts)
		verts->SetColor(i, v2p(p));
}

RGBf vtMesh::GetVtxColor(int i) const
{
	const PSVtxArray* verts = GetVertices();
	if (verts)
	{
		return p2v(verts->GetColor(i));
	}
	else
		return RGBf(0, 0, 0);
}

void vtMesh::SetVtxTexCoord(int i, const FPoint2 &p)
{
	PSVtxArray* verts = GetVertices();
	if (verts)
		verts->SetTexCoord(i, PSTexCoord(p.x, p.y));
}

FPoint2 vtMesh::GetVtxTexCoord(int i)
{
	FPoint2 p(0, 0);
	PSVtxArray* verts = GetVertices();
	if (verts)
	{
		PSTexCoord tc = verts->GetTexCoord(i);
		p.x = tc.u;
		p.y = tc.v;
	}
	return p;
}

int vtMesh::GetNumPrims()
{
	return PSTriMesh::GetNumPrim();
}

vtTextMesh::vtTextMesh(vtFont *font, bool bCenter) : PSTextGeometry()
{
	if (font)
		SetFontName(font->GetFileName());
}

