//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

void makeVec4(sgVec4 &col, float r, float g, float b)
{
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
}

void makeVec4(sgVec4 &col, float r, float g, float b, float a)
{
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0f;
}

///////////////////////////////////

vtMaterial::vtMaterial() : vtMaterialBase(), ssgSimpleState()
{
	disable(GL_COLOR_MATERIAL);
}

void vtMaterial::SetDiffuse(float r, float g, float b, float a)
{
	sgVec4 col;
	makeVec4(col, r, g, b, a);
	setMaterial(GL_DIFFUSE, col);
}

RGBAf vtMaterial::GetDiffuse()
{
	float *col = getMaterial(GL_DIFFUSE);
	return RGBAf(col[0], col[1], col[2], col[3]);
}

void vtMaterial::SetSpecular(float r, float g, float b)
{
	sgVec4 col;
	makeVec4(col, r, g, b);
	setMaterial(GL_SPECULAR, col);
}
RGBf vtMaterial::GetSpecular()
{
	float *col = getMaterial(GL_SPECULAR);
	return RGBf(col[0], col[1], col[2]);
}

void vtMaterial::SetAmbient(float r, float g, float b)
{
	sgVec4 col;
	makeVec4(col, r, g, b);
	setMaterial(GL_AMBIENT, col);
}
RGBf vtMaterial::GetAmbient()
{
	float *col = getMaterial(GL_AMBIENT);
	return RGBf(col[0], col[1], col[2]);
}

void vtMaterial::SetEmission(float r, float g, float b)
{
	sgVec4 col;
	makeVec4(col, r, g, b);
	setMaterial(GL_EMISSION, col);
}
RGBf vtMaterial::GetEmission()
{
	float *col = getMaterial(GL_EMISSION);
	return RGBf(col[0], col[1], col[2]);
}

void vtMaterial::SetCulling(bool bCulling)
{
//	Set(APP_Culling, bCulling);
//	enable();	TODO
}
bool vtMaterial::GetCulling()
{
//	return Get(APP_Culling) != 0;
	return true;
}
void vtMaterial::SetLighting(bool bLighting)
{
//	Set(APP_Lighting, bLighting);
//	enable();
}
bool vtMaterial::GetLighting()
{
//	return Get(APP_Lighting) != 0;
	return true;
}

void vtMaterial::SetTransparent(bool bOn, bool bAdd)
{
//	Set(APP_Transparency, iTransp);
//	enable();
}
bool vtMaterial::GetTransparent()
{
//	return Get(APP_Transparency);
	return true;
}

void vtMaterial::SetTexture(vtImage *pImage)
{
	setTexture(pImage->m_pTexture);
	m_pImage = pImage;
}
vtImage *vtMaterial::GetTexture()
{
	return m_pImage;
}

void vtMaterial::SetClamp(bool bClamp)
{
	// TODO
}

bool vtMaterial::GetClamp()
{
	// TODO
	return false;
}

void vtMaterial::SetMipMap(bool bMipMap)
{
	// TODO
}

bool vtMaterial::GetMipMap()
{
	// TODO
	return false;
}

void vtMaterial::Apply()
{
	apply();
}

//////////////////////////////////////////////////////////////

int vtMaterialArray::AppendMaterial(vtMaterial *pMat)
{
	// nothing special to do
	return Append(pMat);
}

//////////////////////////////////////////////////////////////

vtMesh::vtMesh(GLenum PrimType, int VertType, int VertNum) :
	 vtMeshBase(PrimType, VertType, VertNum)
{
	m_vertices = new ssgVertexArray(VertNum);
	m_vertices->setNum(VertNum);

	m_indices = new ssgIndexArray(3);

	if (VertType & VT_Normals)
	{
		m_normals = new ssgNormalArray(VertNum);
		m_normals->setNum(VertNum);
	}
	else
		m_normals = NULL;

	if (VertType & VT_Colors)
	{
		m_colours = new ssgColourArray(VertNum);
		m_colours->setNum(VertNum);
	}
	else
		m_colours = NULL;

	if (VertType & VT_TexCoords)
	{
		m_texcoords = new ssgTexCoordArray(VertNum);
		m_texcoords->setNum(VertNum);
	}
	else
		m_texcoords = NULL;

//	m_pVtx = new ssgVtxArray(PrimType, m_vertices,
//		m_normals, m_texcoords, m_colours, m_indices);

	m_pVtx = new ssgVtxTable(PrimType, m_vertices,
		m_normals, m_texcoords, m_colours);

	m_pVtx->dirtyBSphere();
	m_pVtx->setUserData(this);
}

#if 0
int vtMesh::AddVertex(float x, float y, float z)
{
	sgVec3 pos;
	v2s(FPoint3(x, y, z), pos);
	m_vertices->add(pos);

	sgVec3 norm = {1.0f, 0.0f, 0.0f};
	if (m_iVtxType & VT_Normals)
		m_normals->add(norm);

	sgVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (m_iVtxType & VT_Colors)
		m_colours->add(color);

	sgVec2 coord = {0.0f, 0.0f};
	if (m_iVtxType & VT_TexCoords)
		m_texcoords->add(coord);

	return m_pVtx->getNumVertices() - 1;
}

int vtMesh::AddVertexN(float x, float y, float z, float nx, float ny, float nz)
{
	sgVec3 pos;
	v2s(FPoint3(x, y, z), pos);
	m_vertices->add(pos);

	sgVec3 norm;
	v2s(FPoint3(nx, ny, nz), norm);
	if (m_iVtxType & VT_Normals)
		m_normals->add(norm);

	sgVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (m_iVtxType & VT_Colors)
		m_colours->add(color);

	sgVec2 coord = {0.0f, 0.0f};
	if (m_iVtxType & VT_TexCoords)
		m_texcoords->add(coord);

	return m_pVtx->getNumVertices() - 1;
}

int vtMesh::AddVertexUV(float x, float y, float z, float u, float v)
{
	sgVec3 pos;
	v2s(FPoint3(x, y, z), pos);
	m_vertices->add(pos);

	sgVec3 norm = {1.0f, 0.0f, 0.0f};
	if (m_iVtxType & VT_Normals)
		m_normals->add(norm);

	sgVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (m_iVtxType & VT_Colors)
		m_colours->add(color);

	sgVec2 coord;
	sgSetVec2(coord, u, v);
	if (m_iVtxType & VT_TexCoords)
		m_texcoords->add(coord);

	return m_pVtx->getNumVertices() - 1;
}

//
// More compact forms
//
int vtMesh::AddVertex(FPoint3 &p)
{
	return AddVertex(p.x, p.y, p.z);
}
int vtMesh::AddVertexN(FPoint3 &p, FPoint3 &n)
{
	return AddVertexN(p.x, p.y, p.z, n.x, n.y, n.z);
}
int vtMesh::AddVertexUV(FPoint3 &p, float u, float v)
{
	return AddVertexUV(p.x, p.y, p.z, u, v);
}
#endif

void vtMesh::AddTri(int p0, int p1, int p2)
{
	m_indices->add(p0);
	m_indices->add(p1);
	m_indices->add(p2);

//	AddPrim(&tri, NULL, true);
}

void vtMesh::AddFan(int p0, int p1, int p2, int p3, int p4, int p5)
{
	m_indices->add(p0);
	m_indices->add(p1);
	m_indices->add(p2);

	if (p3 != -1) m_indices->add(p3);
	if (p4 != -1) m_indices->add(p4);
	if (p5 != -1) m_indices->add(p5);

//	AddPrim(&fan, NULL, true);
}

void vtMesh::AddFan(int *idx, int iNVerts)
{
	for (int i = 0; i < iNVerts; i++)
		m_indices->add(idx[i]);

//	AddPrim(&fan, NULL, true);
}

void vtMesh::AddStrip(int iNVerts, unsigned short *pIndices)
{
#if 0
	// no indexing - the start index IS the index into the vertex array
	GeoPrim		strip(GEO_Strip, iNVerts, appidx);
	strip.VtxIndex = iStartIndex;

	AddPrim(&strip, NULL, true);
#endif
}

void vtMesh::AddQuadStrip(int iNVerts, int iStartIndex)
{
#if 0
	// no indexing - the start index IS the index into the vertex array
	GeoPrim		strip(GEO_Strip, iNVerts, appidx);
	strip.VtxIndex = iStartIndex;

	AddPrim(&strip, NULL, true);
#endif
}

void vtMesh::AddLine(int iNVerts, int iStartIndex)
{
#if 0
	// no indexing - the start index IS the index into the vertex array
	GeoPrim		strip(GEO_Strip, iNVerts, appidx);
	strip.VtxIndex = iStartIndex;

	AddPrim(&strip, NULL, true);
#endif
}

void vtMesh::SetVtxPos(int i, const FPoint3&p)
{
	sgVec3 s;
	v2s(p, s);
	m_vertices->set(s, i);
}

FPoint3 vtMesh::GetVtxPos(int i)
{
	FPoint3 p;
	s2v( m_vertices->get(i), p);
	return p;
}

void vtMesh::SetVtxNormal(int i, const FPoint3&p)
{
	sgVec3 s;
	v2s(p, s);
	m_normals->set(s, i);
}

FPoint3 vtMesh::GetVtxNormal(int i)
{
	FPoint3 p;
	s2v( m_normals->get(i), p);
	return p;
}

void vtMesh::SetVtxColor(int i, const RGBf&p)
{
	sgVec4 s;
	v2s(p, s);
	m_colours->set(s, i);
}

RGBf vtMesh::GetVtxColor(int i)
{
	RGBf p;
	s2v( m_colours->get(i), p);
	return p;
}

void vtMesh::SetVtxTexCoord(int i, const FPoint2&p)
{
	sgVec2 s;
	v2s(p, s);
	m_texcoords->set(s, i);
}

FPoint2 vtMesh::GetVtxTexCoord(int i)
{
	FPoint2 p;
	s2v( m_texcoords->get(i), p);
	return p;
}

int vtMesh::GetNumPrims()
{
	// TODO
//	return m_pGeoSet->getNumPrims();
	return 0;
}

void vtMesh::ReOptimize()
{
	// TODO?
//	m_pGeoSet->dirtyDisplayList();
}

