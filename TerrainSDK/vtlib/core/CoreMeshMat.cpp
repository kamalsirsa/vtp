//
// CoreMeshMat.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

static FPoint3 c[8] =
{
	FPoint3(-1.0, -1.0, 1.0),		  /* 0 */
	FPoint3(-1.0, 1.0, 1.0),		   /* 1 */
	FPoint3(1.0, 1.0, 1.0),			/* 2 */
	FPoint3(1.0, -1.0, 1.0),		   /* 3 */
	FPoint3(-1.0, -1.0, -1.0),		 /* 4 */
	FPoint3(-1.0, 1.0, -1.0),		  /* 5 */
	FPoint3(1.0, 1.0, -1.0),		   /* 6 */
	FPoint3(1.0, -1.0, -1.0),		  /* 7 */
};

static FPoint3 n[6] =
{
	FPoint3( 0.0, 0.0, 1.0 ),
	FPoint3( -1.0, 0.0, 0.0 ),
	FPoint3( 0.0, 0.0, -1.0 ),
	FPoint3( 1.0, 0.0, 0.0 ),
	FPoint3( 0.0, -1.0, 0.0 ),
	FPoint3( 0.0, 1.0, 0.0 )
};

static int v_list[6][4] = {
	7, 4, 5, 6,				 /* back */
	3, 7, 6, 2,				 /* right */
	0, 3, 2, 1,				 /* front */
	4, 0, 1, 5,				 /* left */
	6, 5, 1, 2,				 /* top */
	4, 7, 3, 0,				 /* bottom */
};

static FPoint2 t[4] =
{
	FPoint2(0.0, 1.0),
	FPoint2(1.0, 1.0),
	FPoint2(1.0, 0.0),
	FPoint2(0.0, 0.0)
};


void vtMeshBase::GetBoundBox(FBox3 &box)
{
	int num = GetNumVertices();
	if (num == 0)
		return;

	box.min.Set(1E8, 1E8, 1E8);
	box.max.Set(-1E8, -1E8, -1E8);

	for (int i = 0; i < num; i++)
	{
		FPoint3 p = GetVtxPos(i);

		if (p.x < box.min.x) box.min.x = p.x;
		if (p.y < box.min.y) box.min.y = p.y;
		if (p.z < box.min.z) box.min.z = p.z;

		if (p.x > box.max.x) box.max.x = p.x;
		if (p.y > box.max.y) box.max.y = p.y;
		if (p.z > box.max.z) box.max.z = p.z;
	}
}

int vtMeshBase::AddVertex(float x, float y, float z)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, FPoint3(x, y, z));
	m_iNumVertsUsed++;
	return i;
}

int vtMeshBase::AddVertexN(float x, float y, float z, float nx, float ny, float nz)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, FPoint3(x, y, z));
	SetVtxNormal(i, FPoint3(nx, ny, nz));
	m_iNumVertsUsed++;
	return i;
}

int vtMeshBase::AddVertexUV(float x, float y, float z, float u, float v)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, FPoint3(x, y, z));
	SetVtxTexCoord(i, FPoint2(u, v));
	m_iNumVertsUsed++;
	return i;
}

int vtMeshBase::AddVertex(const FPoint3 &p)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, p);
	m_iNumVertsUsed++;
	return i;
}

int vtMeshBase::AddVertexN(const FPoint3 &p, const FPoint3 &n)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, p);
	SetVtxNormal(i, n);
	m_iNumVertsUsed++;
	return i;
}

int vtMeshBase::AddVertexUV(const FPoint3 &p, float u, float v)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, p);
	SetVtxTexCoord(i, FPoint2(u, v));
	m_iNumVertsUsed++;
	return i;
}

int vtMeshBase::AddVertexUV(const FPoint3 &p, const FPoint2 &uv)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, p);
	SetVtxTexCoord(i, uv);
	m_iNumVertsUsed++;
	return i;
}

int vtMeshBase::AddVertexNUV(const FPoint3 &p, const FPoint3 &n, const FPoint2 &uv)
{
	int i = m_iNumVertsUsed;
	SetVtxPos(i, p);
	SetVtxNormal(i, n);
	SetVtxTexCoord(i, uv);
	m_iNumVertsUsed++;
	return i;
}

void vtMeshBase::AddStrip2(int iNVerts, int iStartIndex)
{
	unsigned short *idx = new unsigned short[iNVerts];

	for (int i = 0; i < iNVerts; i++)
		idx[i] = iStartIndex + i;

	AddStrip(iNVerts, idx);
	delete idx;
}

void vtMeshBase::CreateBlock(const FPoint3& size)
{
	int i, j;
	FPoint3 	vtx[8];			/* individual vertex values */

	for (i = 0; i < 8; i++)
	{
		vtx[i].x = size.x * c[i].x;
		vtx[i].y = size.y * c[i].y;
		vtx[i].z = size.z * c[i].z;
	}

	int vidx = 0;
	for (i = 0; i < 6; i++)
	{
		int start = vidx;
		for (j = 0; j < 4; j++)
		{
			AddVertex(vtx[v_list[i][j]]);
			if (GetVtxType() & VT_Normals)		/* compute normals */
				SetVtxNormal(vidx, -n[i]);
			if (GetVtxType() & VT_TexCoords)	/* compute tex coords */
				SetVtxTexCoord(vidx, t[j]);
			vidx++;
		}
		AddFan(start, start+1, start+2, start+3);
	}
}

//
// OptimizedBlock
//
//	Adds a 3D block to a vtMesh as a series of 5 triangle fan primitives.
//  The bottom face is omitted, the base is placed at y=0, and texture
//  coordinates are provided such that texture bitmaps appear right-side-up
//  on the side faces.
//
void vtMeshBase::CreateOptimizedBlock(const FPoint3& size)
{
	int i, j;
	FPoint3 	vtx[8];			/* individual vertex values */
	FPoint3		half = size / 2.0f;

	for (i = 0; i < 8; i++)
	{
		vtx[i].x = half.x * c[i].x;
		vtx[i].y = half.y * c[i].y + (half.y);
		vtx[i].z = half.z * c[i].z;
	}

	int vidx = 0;
	for (i = 0; i < 5; i++)
	{
		int start = vidx;
		for (j = 0; j < 4; j++)
		{
			AddVertex(vtx[v_list[i][j]]);
			if (GetVtxType() & VT_Normals)		/* compute normals */
				SetVtxNormal(vidx, -n[i]);
			if (GetVtxType() & VT_TexCoords)	/* compute tex coords */
				SetVtxTexCoord(vidx, t[j]);
			vidx++;
		}
		AddFan(start, start+1, start+2, start+3);
	}
}

//
// Add triangle/quad strips to a mesh suitable for a n*m grid
//
void vtMeshBase::CreateRectangularMesh(int xsize, int ysize)
{
	int i, j;

	unsigned short *strip = new unsigned short[ysize*2];
	for (i = 0; i < xsize - 1; i++)
	{
		int start = i * ysize;
		int len = 0;
		for (j = 0; j < ysize; j++)
		{
			strip[len++] = start + j;
			strip[len++] = start + j + ysize;
		}
		AddStrip(ysize*2, strip);
	}
	delete strip;
}

void vtMeshBase::CreateEllipsoid(FPoint3 size, int res, bool hemi)
{
	int		i, j;
	int		vidx;
	FPoint3 v;
	float	theta_step, phi_step;
	int		theta_res, phi_res;
	float	phi_range;

	if (hemi)
	{
		phi_res = res/2;
		phi_range = PID2f;
	}
	else
	{
		phi_res = res;
		phi_range = PIf;
	}
	theta_res = res * 2;

	theta_step = PI2f / (theta_res - 1);
	phi_step = phi_range / (phi_res - 1);
	for (i = 0; i < theta_res; i++)
	{
		float theta = i * theta_step;
		for (j = 0; j < phi_res; j++)
		{
			float phi = j * phi_step;

			v.x = cosf(theta) * sinf(phi);
			v.z = sinf(theta) * sinf(phi);
			v.y = cosf(phi);

			FPoint3 p(size.x * v.x, size.y * v.y, size.z * v.z);
			vidx = AddVertex(p);
			if (GetVtxType() & VT_Normals)		/* compute normals */
				SetVtxNormal(vidx, v);
			if (GetVtxType() & VT_TexCoords)	/* compute tex coords */
			{
				FPoint2 t((float)i / (theta_res-1), (float)j / (phi_res-1));
				SetVtxTexCoord(vidx, t);
			}
		}
	}
	CreateRectangularMesh(theta_res, phi_res);
}

//
// Create the vertices and a fan for a single flat rectangle
//
void vtMeshBase::CreateRectangle(float xsize, float ysize)
{
	int vidx =
	AddVertexUV(-xsize/2, 0.0f, -ysize/2,	0.0f, 0.0f);
	AddVertexUV( xsize/2, 0.0f, -ysize/2,	1.0f, 0.0f);
	AddVertexUV( xsize/2, 0.0f,  ysize/2,	1.0f, 1.0f);
	AddVertexUV(-xsize/2, 0.0f,  ysize/2,	0.0f, 1.0f);
	AddFan(vidx, vidx+1, vidx+2, vidx+3);
}

void vtMeshBase::TransformVertices(FMatrix4 &mat)
{
	int i, num = GetNumVertices();
	FPoint3 p, p2;

	for (i = 0; i < num; i++)
	{
		p = GetVtxPos(i);
		mat.Transform(p, p2);
		SetVtxPos(i, p2);
	}
	if (GetVtxType() & VT_Normals)
	{
		for (i = 0; i < num; i++)
		{
			p = GetVtxNormal(i);
			mat.TransformVector(p, p2);
			SetVtxNormal(i, p2);
		}
	}
}

vtGeom *Create3DCursor(float fSize, float fSmall)
{
	int i, j;
	vtMesh *geo[3];

	for (i = 0; i < 3; i++)
		geo[i] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals, 24);

	geo[0]->CreateBlock(FPoint3(fSize, fSmall, fSmall));
	geo[1]->CreateBlock(FPoint3(fSmall, fSize, fSmall));
	geo[2]->CreateBlock(FPoint3(fSmall, fSmall, fSize));

	// liven the cursor up a bit by flipping normals
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 24; j+=2)
			geo[i]->SetVtxNormal(j, -geo[i]->GetVtxNormal(j));
	}

	// Add the geometry and materials to the shape
	vtGeom *pGeom = new vtGeom();
	vtMaterialArray *pMats = new vtMaterialArray();

	float alpha = 0.6f;
	pMats->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f), true, true, false, alpha);
	pMats->AddRGBMaterial1(RGBf(0.0f, 1.0f, 0.0f), true, true, false, alpha);
	pMats->AddRGBMaterial1(RGBf(0.0f, 0.0f, 1.0f), true, true, false, alpha);

	pGeom->SetMaterials(pMats);
	pGeom->SetName2("3D Crosshair");

	for (i = 0; i < 3; i++)
		pGeom->AddMesh(geo[i], i);

	return pGeom;
}

////////////////////////////////////////////////////////////////////

int vtMaterialArrayBase::AddTextureMaterial(vtImage *pImage,
						 bool bCulling, bool bLighting,
						 bool bTransp, bool bAdditive,
						 float fAmbient, float fDiffuse,
						 float fAlpha, float fEmissive,
						 bool bTexGen, bool bClamp,
						 bool bMipMap)
{
	vtMaterial *pMat = new vtMaterial();
	pMat->SetTexture(pImage);
	pMat->SetCulling(bCulling);
	pMat->SetLighting(bLighting);
	pMat->SetTransparent(bTransp, bAdditive);
	pMat->SetAmbient(fAmbient, fAmbient, fAmbient);
	pMat->SetDiffuse(fDiffuse, fDiffuse, fDiffuse, fAlpha);
	pMat->SetEmission(fEmissive, fEmissive, fEmissive);
	pMat->SetClamp(bClamp);
	pMat->SetMipMap(bMipMap);

	return AppendMaterial(pMat);
}

int vtMaterialArrayBase::AddTextureMaterial2(const char *fname,
						 bool bCulling, bool bLighting,
						 bool bTransp, bool bAdditive,
						 float fAmbient, float fDiffuse,
						 float fAlpha, float fEmissive,
						 bool bTexGen, bool bClamp,
						 bool bMipMap)
{
	vtImage *pImage = new vtImage(fname);
	if (!pImage->LoadedOK())
		return -1;

	return AddTextureMaterial(pImage, bCulling, bLighting,
		bTransp, bAdditive, fAmbient, fDiffuse, fAlpha, fEmissive, bTexGen, bClamp);
}

int vtMaterialArrayBase::AddRGBMaterial(const RGBf &diffuse, const RGBf &ambient,
					 bool bCulling, bool bLighting, bool bWireframe,
					 float fAlpha, float fEmissive)
{
	vtMaterial *pMat = new vtMaterial();
	pMat->SetCulling(bCulling);
	pMat->SetLighting(bLighting);
	pMat->SetDiffuse(diffuse.r, diffuse.g, diffuse.b, fAlpha);
	pMat->SetSpecular(0.0f, 0.0f, 0.0f);
	pMat->SetAmbient(ambient.r, ambient.g, ambient.b);
	pMat->SetEmission(fEmissive, fEmissive, fEmissive);
	return AppendMaterial(pMat);
}

int vtMaterialArrayBase::AddRGBMaterial1(const RGBf &diffuse,
				 bool bCulling, bool bLighting, bool bWireframe,
				 float fAlpha, float fEmissive)
{
	return AddRGBMaterial(diffuse, diffuse/4, bCulling, bLighting, bWireframe,
		fAlpha, fEmissive);
}

void vtMaterialArrayBase::AddShadowMaterial(float fOpacity)
{
	vtMaterial *pMat = new vtMaterial();
	pMat->SetCulling(true);
	pMat->SetLighting(false);
	pMat->SetTransparent(true);
	pMat->SetDiffuse(0.0f, 0.0f, 0.0f, fOpacity);
	AppendMaterial(pMat);
}

void vtMaterialArrayBase::CopyFrom(vtMaterialArrayBase *pFrom)
{
	int num = pFrom->GetSize();
	for (int i = 0; i < num; i++)
	{
		vtMaterial *pMat1 = pFrom->GetAt(i);
		vtMaterial *pMat2 = new vtMaterial();
		pMat2->CopyFrom(pMat1);
		AppendMaterial(pMat2);
	}
}

///////////////////////////////////////////////////////////

vtMaterialBase::vtMaterialBase()
{
	m_pImage = NULL;
}

void vtMaterialBase::SetTexture2(const char *szFilename)
{
	SetTexture(new vtImage(szFilename));
}

void vtMaterialBase::CopyFrom(vtMaterial *pFrom)
{
	SetDiffuse1(pFrom->GetDiffuse());
	SetSpecular1(pFrom->GetSpecular());
	SetAmbient1(pFrom->GetAmbient());
	SetEmission1(pFrom->GetEmission());

	SetCulling(pFrom->GetCulling());
	SetLighting(pFrom->GetLighting());

	SetTexture(pFrom->GetTexture());
	SetTransparent(pFrom->GetTransparent());
}

/////////////////////////////////////////////

void vtGeomBase::SetMaterials(class vtMaterialArray *mats)
{
	m_pMaterialArray = mats;
}

vtMaterial *vtGeomBase::GetMaterial(int idx)
{
	if (m_pMaterialArray && idx < m_pMaterialArray->GetSize())
		return m_pMaterialArray->GetAt(idx);
	else
		return NULL;
}

