//
// GeomUtil.cpp
//
// Useful classes and functions for working with geometry and meshes.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "GeomUtil.h"

/**
 * Create a "3d Cursor" geometry, which consists of 3 blocks (red, green, blue)
 * along the XYZ axes.
 *
 * \param fSize The overall width, height, and depth of the geometry
 * \param fSmall The width of the blocks (generally much smaller than fSize)
 * \param fAlpha The alpha value to use, from 0 (transparent) to 1 (opaque)
 */
vtGeom *Create3DCursor(float fSize, float fSmall, float fAlpha)
{
	int i, j;
	vtMesh *mesh[3];

	for (i = 0; i < 3; i++)
		mesh[i] = new vtMesh(vtMesh::TRIANGLE_FAN, VT_Normals, 24);

	mesh[0]->CreateBlock(FPoint3(fSize, fSmall, fSmall));
	mesh[1]->CreateBlock(FPoint3(fSmall, fSize, fSmall));
	mesh[2]->CreateBlock(FPoint3(fSmall, fSmall, fSize));

	// liven the appearance up a bit by flipping alternate normals
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 24; j+=2)
			mesh[i]->SetVtxNormal(j, -mesh[i]->GetVtxNormal(j));
	}

	// Add the geometry and materials to the shape
	vtGeom *pGeom = new vtGeom();
	vtMaterialArray *pMats = new vtMaterialArray();

	pMats->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f), true, true, false, fAlpha);
	pMats->AddRGBMaterial1(RGBf(0.0f, 1.0f, 0.0f), true, true, false, fAlpha);
	pMats->AddRGBMaterial1(RGBf(0.0f, 0.0f, 1.0f), true, true, false, fAlpha);

	pGeom->SetMaterials(pMats);
	pGeom->SetName2("3D Crosshair");

	// release material array here, so that it will be automatically deleted
	// later when it's no longer needed.
	pMats->Release();

	for (i = 0; i < 3; i++)
		pGeom->AddMesh(mesh[i], i);

	// Pass ownership of the meshes to the geometry
	for (i = 0; i < 3; i++)
		mesh[i]->Release();

	return pGeom;
}

/**
 * Create a wireframe sphere which is very useful for visually representing
 * the bounding sphere of an object in the scene.
 */
vtGeom *CreateBoundSphereGeom(const FSphere &sphere, int res)
{
	vtGeom *pGeom = new vtGeom();
	vtMaterialArray *pMats = new vtMaterialArray();
	pMats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 0.0f), false, false, true);
	pGeom->SetMaterials(pMats);
	pMats->Release();	// pass ownership to the geom

	vtMesh *pMesh = new vtMesh(vtMesh::LINE_STRIP, 0, (res+1)*3*2);

	float radius = sphere.radius * 0.9f;

	FPoint3 p;
	int i, j;
	float a;

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j <= res; j++)
		{
			a = j * PI2f / res;
			p.x = sin(a) * radius;
			p.y = cos(a) * radius;
			p.z = i ? radius * 0.01f : radius * -0.01f;
			pMesh->AddVertex(p + sphere.center);
		}
		for (j = 0; j <= res; j++)
		{
			a = j * PI2f / res;
			p.y = sin(a) * radius;
			p.z = cos(a) * radius;
			p.x = i ? radius * 0.01f : radius * -0.01f;
			pMesh->AddVertex(p + sphere.center);
		}
		for (j = 0; j <= res; j++)
		{
			a = j * PI2f / res;
			p.z = sin(a) * radius;
			p.x = cos(a) * radius;
			p.y = i ? radius * 0.01f : radius * -0.01f;
			pMesh->AddVertex(p + sphere.center);
		}
	}
	for (i = 0; i < 6; i++)
		pMesh->AddStrip2((res+1), (res+1) * i);

	pGeom->AddMesh(pMesh, 0);
	pMesh->Release();	// pass ownership to the geometry

	return pGeom;
}

vtGeom *CreatePlaneGeom(const vtMaterialArray *pMats, int iMatIdx,
						const FPoint2 &base, const FPoint2 &size,
						float fTiling, int steps)
{
	vtGeom *pGeom = new vtGeom();
	vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, VT_Normals | VT_TexCoords, steps * steps);

	mesh->CreateRectangle(steps, steps, FPoint3(base.x, 0, base.y),
		FPoint3(size.x / steps, 0, size.y / steps), fTiling);

	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(mesh, iMatIdx);

	mesh->Release();	// pass ownership
	return pGeom;
}


/**
 * Create a sphere geometry with the indicated material, radius and resolution.
 *
 * \param pMats   The array of materials to use.
 * \param iMatIdx The index of the material to use.
 * \param iVertType Flags which indicate what type of information is stored with each
 *		vertex.  This can be any combination of the following bit flags:
 *		- VT_Normals - a normal per vertex.
 *		- VT_Colors - a color per vertex.
 *		- VT_TexCoords - a texture coordinate (UV) per vertex.
 * \param fRadius The radius of the sphere.
 * \param res	  The resolution (tesselation) of the sphere.  The number of
 *		vertices in the result will be res*res*2.
 */
vtGeom *CreateSphereGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						 float fRadius, int res)
{
	vtGeom *pGeom = new vtGeom();
	vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, iVertType, res*res*2);
	mesh->CreateEllipsoid(FPoint3(fRadius, fRadius, fRadius), res);
	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(mesh, iMatIdx);
	mesh->Release();	// pass ownership to the Geometry
	return pGeom;
}

/**
 * Create a cylinder geometry with the indicated attributes.
 *
 * \param pMats   The array of materials to use.
 * \param iMatIdx The index of the material to use.
 * \param iVertType Flags which indicate what type of information is stored
 *		with each vertex.  This can be any combination of the following bit
 *		flags:
 *		- VT_Normals - a normal per vertex.
 *		- VT_Colors - a color per vertex.
 *		- VT_TexCoords - a texture coordinate (UV) per vertex.
 * \param fHeight The height of the cylinder.
 * \param fRadius The radius of the cylinder.
 * \param res	  The resolution (number of sides) of the cylinder.
 * \param bTop	  True to create the top of the cylinder.
 * \param bBottom True to create the bottom of the cylinder.  You could set
 *		this to false, for example, if the cylinder is going to sit on a
 *		flat surface where you will never see its bottom.
 * \param bCentered True to create a cylinder centered around its origin,
 *		false for a cylinder with its base at the origin that extends outward.
 * \param direction An orientation, 0-2 corresponds to X, Y, Z.  Default is 1 (Y).
 */
vtGeom *CreateCylinderGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						   float fHeight, float fRadius, int res, bool bTop,
						   bool bBottom, bool bCentered, int direction)
{
	// Vertex shading of both the sides and top/bottom requires twice as
	// many vertices.
	int verts;
	if ((bTop || bBottom) && (iVertType & VT_Normals))
		verts = res * 2 * 2;
	else
		verts = res * 2;

	vtGeom *pGeom = new vtGeom();
	vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, iVertType, res*2);
	mesh->CreateCylinder(fHeight, fRadius, res, bTop, bBottom, bCentered);
	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(mesh, iMatIdx);
	mesh->Release();	// pass ownership to the Geometry
	return pGeom;
}

vtGeom *CreateLineGridGeom(const vtMaterialArray *pMats, int iMatIdx,
						   const FPoint3 &min1, const FPoint3 &max1, int steps)
{
	vtGeom *pGeom = new vtGeom();
	vtMesh *mesh = new vtMesh(vtMesh::LINES, 0, (steps+1)*4);

	FPoint3 p, diff = max1 - min1, step = diff / (float)steps;
	p.y = diff.y;
	int i, idx = 0;
	for (i = 0; i < steps+1; i++)
	{
		p.x = min1.x + step.x * i;
		p.z = min1.z;
		mesh->AddVertex(p);
		p.z = max1.z;
		mesh->AddVertex(p);
		mesh->AddLine(idx, idx+1);
		idx += 2;
	}
	for (i = 0; i < steps+1; i++)
	{
		p.z = min1.z + step.z * i;
		p.x = min1.x;
		mesh->AddVertex(p);
		p.x = max1.x;
		mesh->AddVertex(p);
		mesh->AddLine(idx, idx+1);
		idx += 2;
	}
	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(mesh, iMatIdx);
	mesh->Release();	// pass ownership to the Geometry
	return pGeom;
}


//////////////////////////////////////////////////////////////

vtMeshFactory::vtMeshFactory(vtGeom *pGeom, vtMeshBase::PrimType ePrimType,
							 int iVertType, int iMaxVertsPerMesh, int iMatIndex)
{
	m_pGeom = pGeom;
	m_ePrimType = ePrimType;
	m_iVertType = iVertType;
	m_iMaxVertsPerMesh = iMaxVertsPerMesh;
	m_iMatIndex = iMatIndex;

	m_pMesh = NULL;
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
}

void vtMeshFactory::NewMesh()
{
	m_pMesh = new vtMesh(m_ePrimType, m_iVertType, m_iMaxVertsPerMesh);
	m_pGeom->AddMesh(m_pMesh, m_iMatIndex);
}

void vtMeshFactory::PrimStart()
{
	if (!m_pMesh)
		NewMesh();
	m_iPrimStart = m_pMesh->GetNumVertices();
	m_iPrimVerts = 0;
}

void vtMeshFactory::AddVertex(const FPoint3 &p)
{
	int count = m_pMesh->GetNumVertices();
	if (count == m_iMaxVertsPerMesh)
	{
		// repeat vertex; it needs to appear in both meshes
		m_pMesh->AddVertex(p);
		m_iPrimVerts++;

		// close that primitive and start another on a new mesh
		PrimEnd();
		NewMesh();
		PrimStart();
	}
	m_pMesh->AddVertex(p);
	m_iPrimVerts++;
}

void vtMeshFactory::PrimEnd()
{
	m_pMesh->AddStrip2(m_iPrimVerts, m_iPrimStart);
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
}

