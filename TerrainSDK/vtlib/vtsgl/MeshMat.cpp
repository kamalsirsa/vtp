//
//
//

#include "vtlib/vtlib.h"

///////////////////////////////////

vtMaterial::vtMaterial() : vtMaterialBase()
{
	m_pMaterial = new sglMaterial;
	m_pTexture = NULL;

	sglLighting *statelet = new sglLighting(false);  // disabled on construction
	statelet->enable(true);  // enable

	m_state.push_back(statelet);
	m_state.push_back(m_pMaterial);
}

void vtMaterial::SetDiffuse(float r, float g, float b, float a)
{
	m_pMaterial->setDiffuse(r, g, b);
}
RGBAf vtMaterial::GetDiffuse()
{
	RGBAf c;
	m_pMaterial->getFrontDiffuse(c.r, c.g, c.b);
	c.a = 1.0f;
	return c;
}

void vtMaterial::SetSpecular(float r, float g, float b)
{
	m_pMaterial->setSpecular(r, g, b);
}
RGBf vtMaterial::GetSpecular()
{
	RGBf c;
	m_pMaterial->getFrontSpecular(c.r, c.g, c.b);
	return c;
}

void vtMaterial::SetAmbient(float r, float g, float b)
{
	m_pMaterial->setAmbient(r, g, b);
}
RGBf vtMaterial::GetAmbient()
{
	RGBf c;
	m_pMaterial->getFrontAmbient(c.r, c.g, c.b);
	return c;
}

void vtMaterial::SetEmission(float r, float g, float b)
{
	m_pMaterial->setEmission(r, g, b);
}
RGBf vtMaterial::GetEmission()
{
	RGBf c;
	m_pMaterial->getFrontEmission(c.r, c.g, c.b);
	return c;
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
	m_pTexture = pImage->m_pTexture;
	m_pImage = pImage;
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

extern sglCurrState *hack_global_state;

void vtMaterial::Apply()
{
	m_pMaterial->apply(hack_global_state);
	if (m_pTexture)
		m_pTexture->apply(hack_global_state);
}

//////////////////////////////////////////////////////////////

int vtMaterialArray::AppendMaterial(vtMaterial *pMat)
{
	// nothing special to do?
	return Append(pMat);
}

//////////////////////////////////////////////////////////////

vtMesh::vtMesh(GLenum PrimType, int VertType, int NumVertices) :
	 vtMeshBase(PrimType, VertType, NumVertices)
{
	m_Vert.SetMaxSize(NumVertices);

	m_pGeoStripSet = NULL;
	switch (PrimType)
	{
	case GL_POINTS:
		m_pGeoSet = new sglMonoIndexedPointSet;
		break;
	case GL_LINES:
		m_pGeoSet = new sglMonoIndexedLineSet;
		break;
	case GL_TRIANGLES:
		m_pGeoSet = new sglMonoIndexedTriangleSet;
		break;
	case GL_QUADS:
		m_pGeoSet = new sglMonoIndexedQuadSet;
		break;
	case GL_LINE_STRIP:
		m_pGeoSet = m_pGeoStripSet = new sglMonoIndexedLineStripSet;
		break;
	case GL_TRIANGLE_STRIP:
		m_pGeoSet = m_pGeoStripSet = new sglMonoIndexedTriangleStripSet;
		break;
	case GL_TRIANGLE_FAN:
		m_pGeoSet = m_pGeoStripSet = new sglMonoIndexedTriangleFanSet;
		break;
	}
	m_pGeoSet->setIndexList(m_Index.GetData());
	m_pGeoSet->setCoordList(m_Vert.GetData());

	if (m_pGeoStripSet)
		m_pGeoStripSet->setPrimLengths(m_PrimLen.GetData());

	// set backpointer so we can find ourselves later
	m_pGeoSet->setUserData((sglUserData *)this);

	if (VertType & VT_Normals)
	{
		m_Norm.SetMaxSize(NumVertices);
		m_pGeoSet->setNormalList(sglGeoSet::ePER_VERTEX, m_Norm.GetData());
//		m_pGeoSet->setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
	}
	if (VertType & VT_Colors)
	{
		m_Color.SetMaxSize(NumVertices);
		m_pGeoSet->setColorList(sglGeoSet::ePER_VERTEX, m_Color.GetData());
//		m_pGeoSet->setColorBinding(osg::GeoSet::BIND_PERVERTEX);
	}
	if (VertType & VT_TexCoords)
	{
		m_Tex.SetMaxSize(NumVertices);
//		m_pGeoSet->setTextureCoords(m_Tex.GetData());
//		m_pGeoSet->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
	}
}

/*int vtMesh::GetNumVertices()
{
	return m_pVtx->getNumVertices();
}*/

void vtMesh::SendPointersToSGL()
{
	// in case the arrays got reallocated, tell SGL again
	m_pGeoSet->setCoordList(m_Vert.GetData());
	m_pGeoSet->setIndexList(m_Index.GetData());
	if (m_pGeoStripSet)
		m_pGeoStripSet->setPrimLengths(m_PrimLen.GetData());

	if (m_iVtxType & VT_Normals)
		m_pGeoSet->setNormalList(sglGeoSet::ePER_VERTEX, m_Norm.GetData());
	if (m_iVtxType & VT_Colors)
		m_pGeoSet->setColorList(sglGeoSet::ePER_VERTEX, m_Color.GetData());
//	if (m_iVtxType & VT_TexCoords)
//		m_pGeoSet->setTextureCoords(m_Tex.GetData(), m_Index.GetData());
}

void vtMesh::AddTri(int p0, int p1, int p2)
{
	m_Index.Append(p0);
	m_Index.Append(p1);
	m_Index.Append(p2);
	m_PrimLen.Append(3);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToSGL();
}

void vtMesh::AddFan(int p0, int p1, int p2, int p3, int p4, int p5)
{
	int len = 3;

	m_Index.Append(p0);
	m_Index.Append(p1);
	m_Index.Append(p2);

	if (p3 != -1) { m_Index.Append(p3); len = 4; }
	if (p4 != -1) { m_Index.Append(p4); len = 5; }
	if (p5 != -1) { m_Index.Append(p5); len = 6; }

	m_PrimLen.Append(len);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToSGL();
}

void vtMesh::AddFan(int *idx, int iNVerts)
{
	for (int i = 0; i < iNVerts; i++)
		m_Index.Append(idx[i]);

	m_PrimLen.Append(iNVerts);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToSGL();
}

void vtMesh::AddStrip(int iNVerts, unsigned short *pIndices)
{
	for (int i = 0; i < iNVerts; i++)
		m_Index.Append(pIndices[i]);

	m_PrimLen.Append(iNVerts);
	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToSGL();
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
//	m_pGeoSet->setNumPrims(m_pGeoSet->getNumPrims() + 1);

	SendPointersToSGL();
}

void vtMesh::SetVtxPos(int i, const FPoint3&p)
{
	sglVec3f s;
	v2s(p, s);
	m_Vert.SetAt(i, s);
}

FPoint3 vtMesh::GetVtxPos(int i) const
{
	FPoint3 p;
	s2v( m_Vert.GetAt(i), p);
	return p;
}

void vtMesh::SetVtxNormal(int i, const FPoint3&p)
{
	sglVec3f s;
	v2s(p, s);
	m_Norm.SetAt(i, s);
}

FPoint3 vtMesh::GetVtxNormal(int i) const
{
	FPoint3 p;
	s2v( m_Norm.GetAt(i), p);
	return p;
}

void vtMesh::SetVtxColor(int i, const RGBf&p)
{
	sglVec4f s;
	v2s(p, s);
	m_Color.SetAt(i, s);
}

RGBf vtMesh::GetVtxColor(int i) const
{
	RGBf p;
	s2v( m_Color.GetAt(i), p);
	return p;
}

void vtMesh::SetVtxTexCoord(int i, const FPoint2&p)
{
	sglVec2f s;
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
	// TODO?
}

void vtMesh::AllowOptimize(bool bAllow)
{
	// TODO?
}


