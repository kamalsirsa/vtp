//
// GeomUtil.cpp
//
// Useful classes and functions for working with geometry and meshes.
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtString.h"
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
	vtGeom *pGeom = new vtGeom;
	vtMaterialArray *pMats = new vtMaterialArray;

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
	vtGeom *pGeom = new vtGeom;
	vtMaterialArray *pMats = new vtMaterialArray;
	pMats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 0.0f), false, false, true);
	pGeom->SetMaterials(pMats);
	pMats->Release();	// pass ownership to the geom

	vtMesh *pMesh = CreateSphereMesh(sphere, res);
	pGeom->AddMesh(pMesh, 0);
	pMesh->Release();	// pass ownership to the geometry

	return pGeom;
}

/**
 * Create a wireframe sphere which is very useful for visually representing
 * the bounding sphere of an object in the scene.
 */
vtMesh *CreateSphereMesh(const FSphere &sphere, int res)
{
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

	return pMesh;
}

vtGeom *CreatePlaneGeom(const vtMaterialArray *pMats, int iMatIdx,
						int Axis1, int Axis2, int Axis3,
						const FPoint2 &min1, const FPoint2 &max1,
						float fTiling, int steps)
{
	vtGeom *pGeom = new vtGeom;
	vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, VT_Normals | VT_TexCoords, steps * steps);

	mesh->CreateRectangle(steps, steps, Axis1, Axis2, Axis3, min1, max1, 0.0f, fTiling);

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
	vtGeom *pGeom = new vtGeom;
	vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, iVertType, res*res*2);
	mesh->CreateEllipsoid(FPoint3(0,0,0), FPoint3(fRadius, fRadius, fRadius), res);
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

	vtGeom *pGeom = new vtGeom;
	vtMesh *mesh = new vtMesh(vtMesh::TRIANGLE_STRIP, iVertType, res*2);
	mesh->CreateCylinder(fHeight, fRadius, res, bTop, bBottom, bCentered);
	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(mesh, iMatIdx);
	mesh->Release();	// pass ownership to the Geometry
	return pGeom;
}

/**
 * Create a grid of lines in the XZ plane.  This can be useful as a reference
 * object, like a sheet of graph paper.
 */
vtGeom *CreateLineGridGeom(const vtMaterialArray *pMats, int iMatIdx,
						   const FPoint3 &min1, const FPoint3 &max1, int steps)
{
	vtGeom *pGeom = new vtGeom;
	vtMesh *mesh = new vtMesh(vtMesh::LINES, 0, (steps+1)*4);

	FPoint3 p, diff = max1 - min1, step = diff / (float)steps;
	p.y = min1.y;
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

/**
 * Constructor.
 *
 * \param pGeom		The geometry node which will receive the mesh object(s)
 *		that this factory will produce.
 * \param ePrimType	The type of mesh to produce.
 * \param iVertType	The vertex attributes for the meshes to produce.
 * \param iMaxVertsPerMesh	The largest number of vertices to allow in a single
 *		mesh.  When this number is exceeded, the current mesh will be finished
 *		and another mesh begun.
 * \param iMatIndex	The material index of the mesh when it is added to the
 *		geometry node.
 * \param iExpectedVerts If you know how many vertices will be mesh ahead of
 *		time, you can save a little time and memory by passing the number.
 */
vtMeshFactory::vtMeshFactory(vtGeom *pGeom, vtMeshBase::PrimType ePrimType,
							 int iVertType, int iMaxVertsPerMesh, int iMatIndex,
							 int iExpectedVerts)
{
	m_pGeom = pGeom;
	m_ePrimType = ePrimType;
	m_iVertType = iVertType;
	m_iMaxVertsPerMesh = iMaxVertsPerMesh;
	m_iMatIndex = iMatIndex;

	if (iExpectedVerts == -1)
		m_iExpectedVerts = m_iMaxVertsPerMesh;
	else
		m_iExpectedVerts = iExpectedVerts;

	m_pMesh = NULL;
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
	m_iLineWidth = 1;

	m_bSimple = false;
}

/**
 * Alternate, simpler constructor.
 *
 * \param pMesh The mesh which will receive all the vertices that this factory
 *		produces.
 */
vtMeshFactory::vtMeshFactory(vtMesh *pMesh)
{
	m_pGeom = NULL;
	m_pMesh = pMesh;
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
	m_iLineWidth = 1;

	m_bSimple = true;
}

void vtMeshFactory::NewMesh()
{
	m_pMesh = new vtMesh(m_ePrimType, m_iVertType, m_iExpectedVerts);
	m_pGeom->AddMesh(m_pMesh, m_iMatIndex);
	m_pMesh->Release();	// pass ownership to geometry

	if (m_iLineWidth != 1)
		m_pMesh->SetLineWidth(m_iLineWidth);

	// Keep a list of all the meshes made in this factory
	m_Meshes.push_back(m_pMesh);
}

/** Tell the factory to start a primitive. */
void vtMeshFactory::PrimStart()
{
	if (!m_pMesh)
		NewMesh();
	m_iPrimStart = m_pMesh->GetNumVertices();
	m_iPrimVerts = 0;
}

/** Tell the factory to add a vertex to the current primitive. */
void vtMeshFactory::AddVertex(const FPoint3 &p)
{
	if (!m_bSimple)
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
	}
	m_pMesh->AddVertex(p);
	m_iPrimVerts++;
}

/** Tell the factory to end a primitive. */
void vtMeshFactory::PrimEnd()
{
	if (m_iPrimVerts > 0)
		m_pMesh->AddStrip2(m_iPrimVerts, m_iPrimStart);
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
}

void vtMeshFactory::SetLineWidth(int width)
{
	m_iLineWidth = width;
}

void vtMeshFactory::SetMatIndex(int iIdx)
{
	if (iIdx != m_iMatIndex)
	{
		// Material is changing, we must start a new mesh
		PrimEnd();
		m_iMatIndex = iIdx;
		NewMesh();
		PrimStart();
	}
}

///////////////////////////////////////////////////////////////////////
// vtDimension

vtDimension::vtDimension(const FPoint3 &p1, const FPoint3 &p2, float height,
						 const RGBf &line_color, const RGBf &text_color,
						 vtFont *font, const char *message)
{
	// We can't orient the text message in space without a transform, so that's
	//  why we're subclassed from vtTransform.
	m_pGeom = new vtGeom;
	AddChild(m_pGeom);

	// create materials and meshes
	m_pMats = new vtMaterialArray;
	m_pMats->AddRGBMaterial1(line_color, false, false);	// plain, no culling
	m_pGeom->SetMaterials(m_pMats);
	m_pMats->Release();

	m_pLines = new vtMesh(vtMesh::LINES, 0, 12);
	m_pGeom->AddMesh(m_pLines, 0);
	m_pLines->Release();

	// Now determine the points in space which define the geometry.
	FPoint3 diff = p2 - p1;

	FPoint3 q1(0, 0, 0), q2(0, 0, -diff.Length());
	FPoint3 perp(0, height/2, 0);
	FPoint3 along(0, 0, -height/2);

	// Put the points and primitives into the mesh
	m_pLines->AddLine(q1+perp, q1-perp);
	m_pLines->AddLine(q1, q1+perp+along);
	m_pLines->AddLine(q1, q1-perp+along);

	m_pLines->AddLine(q2+perp, q2-perp);
	m_pLines->AddLine(q2, q2+perp-along);
	m_pLines->AddLine(q2, q2-perp-along);

	m_pLines->AddLine(q1, q2);

	// add the text object.
	m_pLabel = new vtTextMesh(font, height, true);
	m_pGeom->AddTextMesh(m_pLabel, 0);
	m_pLabel->Release();

	m_pLabel->SetColor(text_color);
	m_pLabel->SetAlignment(2);	// YZ plane
	m_pLabel->SetPosition(FPoint3(0, height*0.05f, -diff.Length()/2));
	FQuat rot(FPoint3(0,1,0), PID2f);
	m_pLabel->SetRotation(rot);

	// and a second text object, facing the other way
	m_pLabel2 = new vtTextMesh(font, height, true);
	m_pGeom->AddTextMesh(m_pLabel2, 0);
	m_pLabel2->Release();

	m_pLabel2->SetColor(text_color);
	m_pLabel2->SetAlignment(2);	// YZ plane
	m_pLabel2->SetPosition(FPoint3(0, height*0.05f, -diff.Length()/2));
	FQuat rot2(FPoint3(0,1,0), -PID2f);
	m_pLabel2->SetRotation(rot2);

	SetText(message);

	// Now, orient it into the desired location
	PointTowards(diff);
	SetTrans(p1);
}

void vtDimension::SetText(const char *text)
{
	m_pLabel->SetText(text);
	m_pLabel2->SetText(text);
}


///////////////////////////////////////////////////////////////////////

vtOBJFile *OBJFileBegin(vtGeom *geom, const char *filename)
{
	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
		return false;

	fprintf(fp, "#  Wavefront OBJ generated by the VTP software (http://vterrain.org/)\n\n");

	unsigned int i;
	const vtMaterialArray *mats = geom->GetMaterials();
	unsigned int num_mats = 0;
	if (mats)
		num_mats = mats->GetSize();
	if (num_mats > 0)
	{
		// Write corresponding material file
		vtString path = filename;
		vtString fname = StartOfFilename(filename);
		RemoveFileExtensions(path);
		RemoveFileExtensions(fname);
		path += ".mtl";
		fname += ".mtl";

		fprintf(fp, "mtllib %s\n\n", (const char *) fname);

		FILE *fp2 = vtFileOpen(path, "wb");
		fprintf(fp2, "#  Wavefront MTL generated by the VTP software (http://vterrain.org/)\n");

		for (i = 0; i < num_mats; i++)
		{
			const vtMaterial *mat = mats->GetAt(i);
			vtString matname;
			matname.Format("mat%03d", i);
			fprintf(fp2, "\nnewmtl %s\n", (const char *) matname);

			RGBf amb = mat->GetAmbient();
			RGBf dif = mat->GetDiffuse();
			RGBf spe = mat->GetSpecular();
			fprintf(fp2, "Ka %f %f %f\n", amb.r, amb.g, amb.b);
			fprintf(fp2, "Kd %f %f %f\n", dif.r, dif.g, dif.b);
			fprintf(fp2, "Ks %f %f %f\n", spe.r, spe.g, spe.b);

			vtImage *tex = mat->GetTexture();
			if (tex)
			{
				vtString texfname = tex->GetFilename();
				fprintf(fp2, "map_Kd %s\n", (const char *) texfname);
			}
		}
		fclose(fp2);
	}

	vtOBJFile *file = new vtOBJFile;
	file->fp  = fp;
	file->verts_written = 0;
	return file;
}

void OBJFileWriteGeom(vtOBJFile *file, vtGeom *geom)
{
	unsigned int i, j, k;
	unsigned int num_mesh = geom->GetNumMeshes();
	for (i = 0; i < num_mesh; i++)
	{
		vtMesh *mesh = geom->GetMesh(i);
		if (!mesh)
			continue;

		vtMesh::PrimType ptype = mesh->GetPrimType();

		// For now, this method only does tristrips and fans
		if (ptype != vtMesh::TRIANGLE_STRIP && ptype != vtMesh::TRIANGLE_FAN)
			continue;

		// First write the vertices
		int base_vert = file->verts_written;
		int vtype = mesh->GetVtxType();
		unsigned int num_vert = mesh->GetNumVertices();
		fprintf(file->fp, "# %d vertices\n", num_vert);
		for (j = 0; j < num_vert; j++)
		{
			FPoint3 pos = mesh->GetVtxPos(j);
			fprintf(file->fp, "v %f %f %f\n", pos.x, pos.y, pos.z);
		}
		if (vtype & VT_TexCoords)
		{
			for (j = 0; j < num_vert; j++)
			{
				FPoint2 uv = mesh->GetVtxTexCoord(j);
				fprintf(file->fp, "vt %f %f\n", uv.x, uv.y);
			}
		}
		if (vtype & VT_Normals)
		{
			for (j = 0; j < num_vert; j++)
			{
				FPoint3 norm = mesh->GetVtxNormal(j);
				fprintf(file->fp, "vn %f %f %f\n", norm.x, norm.y, norm.z);
			}
		}

		int matidx = mesh->GetMatIndex();
		vtString matname;
		matname.Format("mat%03d", matidx);
		fprintf(file->fp, "usemtl %s\n", (const char *) matname);

		if (ptype == vtMesh::TRIANGLE_STRIP || ptype == vtMesh::TRIANGLE_FAN)
		{
			// OBJ doesn't do strips, so break them into individual triangles
			unsigned int num_prims = mesh->GetNumPrims();
			int prim_start = 0;
			for (k = 0; k < num_prims; k++)
			{
				int len = mesh->GetPrimLen(k);
				for (int t = 0; t < len-2; t++)
				{
					int idx0, idx1, idx2;
					if (ptype == vtMesh::TRIANGLE_STRIP)
					{
						idx0 = mesh->GetIndex(prim_start + t);
						// unwind strip: even and odd faces
						if (t & 1)
						{
							idx1 = mesh->GetIndex(prim_start + t+1);
							idx2 = mesh->GetIndex(prim_start + t+2);
						}
						else
						{
							idx2 = mesh->GetIndex(prim_start + t+1);
							idx1 = mesh->GetIndex(prim_start + t+2);
						}
					}
					else if (ptype == vtMesh::TRIANGLE_FAN)
					{
						idx0 = mesh->GetIndex(prim_start);
						idx1 = mesh->GetIndex(prim_start + t+1);
						idx2 = mesh->GetIndex(prim_start + t+2);
					}
					// Wavefront indices are actually 1-based!
					idx0 = idx0 + base_vert + 1;
					idx1 = idx1 + base_vert + 1;
					idx2 = idx2 + base_vert + 1;
					if ((vtype & VT_Normals) && (vtype & VT_TexCoords))
						fprintf(file->fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
						idx0, idx0, idx0, idx1, idx1, idx1, idx2, idx2, idx2);
					else if ((vtype & VT_Normals) || (vtype & VT_TexCoords))
						fprintf(file->fp, "f %d/%d %d/%d %d/%d\n",
						idx0, idx0, idx1, idx1, idx2, idx2);
					else
						fprintf(file->fp, "f %d %d %d\n",
						idx0, idx1, idx2);
				}
				prim_start += len;
			}
		}
		file->verts_written += num_vert;
	}
}

/**
 * Write a geometry node to a old-fashioned Wavefront OBJ file.
 */
bool WriteGeomToOBJ(vtGeom *geom, const char *filename)
{
	vtOBJFile *file = OBJFileBegin(geom, filename);
	if (!file)
		return false;

	// Now write the geometry itself
	OBJFileWriteGeom(file, geom);
	fclose(file->fp);
	delete file;
	return true;
}
