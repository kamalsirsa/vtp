//
// GeomUtil.cpp
//
// Useful classes and functions for working with geometry and meshes.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtString.h"
#include "GeomUtil.h"


//////////////////////////////////////////////////////////////

/**
 * Constructor.
 *
 * \param pGeode	The geometry node which will receive the mesh object(s)
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
vtGeomFactory::vtGeomFactory(vtGeode *pGeode, vtMesh::PrimType ePrimType,
							 int iVertType, int iMaxVertsPerMesh, int iMatIndex,
							 int iExpectedVerts)
{
	m_pGeode = pGeode;
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
	m_fLineWidth = 1;

	m_bSimple = false;
}

/**
 * Alternate, simpler constructor.
 *
 * \param pMesh The mesh which will receive all the vertices that this factory
 *		produces.
 */
vtGeomFactory::vtGeomFactory(vtMesh *pMesh)
{
	m_pGeode = NULL;
	m_pMesh = pMesh;
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
	m_fLineWidth = 1;

	m_bSimple = true;
}

void vtGeomFactory::NewMesh()
{
	m_pMesh = new vtMesh(m_ePrimType, m_iVertType, m_iExpectedVerts);
	m_pGeode->AddMesh(m_pMesh, m_iMatIndex);

	if (m_fLineWidth != 1)
		m_pMesh->SetLineWidth(m_fLineWidth);

	// Keep a list of all the meshes made in this factory
	m_Meshes.push_back(m_pMesh);
}

/** Tell the factory to start a primitive. */
void vtGeomFactory::PrimStart()
{
	if (!m_pMesh)
		NewMesh();
	m_iPrimStart = m_pMesh->NumVertices();
	m_iPrimVerts = 0;
}

/** Tell the factory to add a vertex to the current primitive. */
void vtGeomFactory::AddVertex(const FPoint3 &p)
{
	if (!m_bSimple)
	{
		int count = m_pMesh->NumVertices();
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
void vtGeomFactory::PrimEnd()
{
	if (m_iPrimVerts > 0)
		m_pMesh->AddStrip2(m_iPrimVerts, m_iPrimStart);
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
}

void vtGeomFactory::SetLineWidth(float width)
{
	m_fLineWidth = width;
}

void vtGeomFactory::SetMatIndex(int iIdx)
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

/**
 * Create geometry for a 2D line by draping the point on a heightfield.
 *
 * \param pHF	The heightfield to drape on.
 * \param line	The 2D line to drape, in Earth coordinates.
 * \param fSpacing	The approximate spacing of the surface tessellation, used to
 *		decide how finely to tessellate the line.
 * \param fOffset	An offset to elevate each point in the resulting geometry,
 *		useful for keeping it visibly above the ground.
 * \param bInterp	True to interpolate between the vertices of the input
 *		line. This is generally desirable when the ground is much more finely
 *		spaced than the input line.
 * \param bCurve	True to interpret the vertices of the input line as
 *		control points of a curve.  The created geometry will consist of
 *		a draped line which passes through the control points.
 * \param bTrue		True to use the true elevation of the terrain, ignoring
 *		whatever scale factor is being used to exaggerate elevation for
 *		display.
 * \return The approximate length of the resulting 3D line mesh.
 *
 * \par Example:
	\code
	DLine2 line = ...;
	vtTerrain *pTerr = ...;
	vtGeode *pLineGeom = new vtGeode;
	pTerr->AddNode(pLineGeom);
	vtGeomFactory mf(pLineGeom, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 1);
	float length = mf.AddSurfaceLineToMesh(pTerr->GetHeightfield(), dline, 1, 10, true);
	\endcode
 */
float vtGeomFactory::AddSurfaceLineToMesh(vtHeightField3d *pHF,
	const DLine2 &line, float fSpacing, float fOffset, bool bInterp, bool bCurve, bool bTrue)
{
	FLine3 tessellated;
	float fTotalLength = pHF->LineOnSurface(line, fSpacing, fOffset, bInterp, bCurve, bTrue, tessellated);

	PrimStart();
	for (uint i = 0; i < tessellated.GetSize(); i++)
		AddVertex(tessellated[i]);
	PrimEnd();

	return fTotalLength;
}


///////////////////////////////////////////////////////////////////////
// vtDimension

vtDimension::vtDimension(const FPoint3 &p1, const FPoint3 &p2, float height,
						 const RGBf &line_color, const RGBf &text_color,
						 osgText::Font *font, const char *message)
{
	// We can't orient the text message in space without a transform, so that's
	//  why we're subclassed from vtTransform.
	m_pGeode = new vtGeode;
	addChild(m_pGeode);

	// create materials and meshes
	m_pMats = new vtMaterialArray;
	m_pMats->AddRGBMaterial(line_color, false, false);	// plain, no culling
	m_pGeode->SetMaterials(m_pMats);

	m_pLines = new vtMesh(osg::PrimitiveSet::LINES, 0, 12);
	m_pGeode->AddMesh(m_pLines, 0);

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
	m_pGeode->AddTextMesh(m_pLabel, 0);

	m_pLabel->SetColor(text_color);
	m_pLabel->SetAlignment(2);	// YZ plane
	m_pLabel->SetPosition(FPoint3(0, height*0.05f, -diff.Length()/2));
	FQuat rot(FPoint3(0,1,0), PID2f);
	m_pLabel->SetRotation(rot);

	// and a second text object, facing the other way
	m_pLabel2 = new vtTextMesh(font, height, true);
	m_pGeode->AddTextMesh(m_pLabel2, 0);

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


///////////////////////////////////////////////////////////////////////////////
// Helper functions

/**
 * Create a "3d Cursor" geometry, which consists of 3 blocks (red, green, blue)
 * along the XYZ axes.
 *
 * \param fSize The overall width, height, and depth of the geometry
 * \param fSmall The width of the blocks (generally much smaller than fSize)
 * \param fAlpha The alpha value to use, from 0 (transparent) to 1 (opaque)
 */
vtGeode *Create3DCursor(float fSize, float fSmall, float fAlpha)
{
	int i, j;
	vtMesh *mesh[3];

	for (i = 0; i < 3; i++)
		mesh[i] = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_Normals, 24);

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
	vtGeode *pGeode = new vtGeode;
	vtMaterialArrayPtr pMats = new vtMaterialArray;

	pMats->AddRGBMaterial(RGBf(1.0f, 0.0f, 0.0f), true, false, false, fAlpha);
	pMats->AddRGBMaterial(RGBf(0.0f, 1.0f, 0.0f), true, false, false, fAlpha);
	pMats->AddRGBMaterial(RGBf(0.0f, 0.0f, 1.0f), true, false, false, fAlpha);

	pGeode->SetMaterials(pMats);
	pGeode->setName("3D Crosshair");

	for (i = 0; i < 3; i++)
		pGeode->AddMesh(mesh[i], i);

	return pGeode;
}

/**
 * Create a wireframe sphere which is very useful for visually representing
 * the bounding sphere of an object in the scene.
 */
vtGeode *CreateBoundSphereGeode(const FSphere &sphere, int res)
{
	vtGeode *pGeode = new vtGeode;
	vtMaterialArrayPtr pMats = new vtMaterialArray;
	pMats->AddRGBMaterial(RGBf(1.0f, 1.0f, 0.0f), false, false, true);
	pGeode->SetMaterials(pMats);

	vtMesh *pMesh = CreateSphereMesh(sphere, res);
	pGeode->AddMesh(pMesh, 0);

	return pGeode;
}

/**
 * Create a wireframe sphere which is very useful for visually representing
 * the bounding sphere of an object in the scene.
 */
vtMesh *CreateSphereMesh(const FSphere &sphere, int res)
{
	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, (res+1)*3*2);

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

vtGeode *CreatePlaneGeom(const vtMaterialArray *pMats, int iMatIdx,
						int Axis1, int Axis2, int Axis3,
						const FPoint2 &min1, const FPoint2 &max1,
						float fTiling, int steps)
{
	vtGeode *pGeode = new vtGeode;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Normals | VT_TexCoords, steps * steps);

	mesh->CreateRectangle(steps, steps, Axis1, Axis2, Axis3, min1, max1, 0.0f, fTiling);

	pGeode->SetMaterials(pMats);
	pGeode->AddMesh(mesh, iMatIdx);
	return pGeode;
}


/**
 * Create a block geometry with the indicated material and size.
 * See vtMesh::CreateBlock for how the block is constructed.
 *
 * \param pMats   The array of materials to use.
 * \param iMatIdx The index of the material to use.
 * \param size	  The dimensions of the block in x, y, z.
 */
vtGeode *CreateBlockGeom(const vtMaterialArray *pMats, int iMatIdx, const FPoint3 &size)
{
	vtGeode *pGeode = new vtGeode;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_Normals | VT_TexCoords, 24);
	mesh->CreateBlock(size);
	pGeode->SetMaterials(pMats);
	pGeode->AddMesh(mesh, iMatIdx);
	return pGeode;
}

void AddLineMesh(vtGeode *pGeode, int iMatIdx, const FPoint3 &p0, const FPoint3 &p1)
{
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 2);
	mesh->AddLine(p0, p1);
	pGeode->AddMesh(mesh, iMatIdx);
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
vtGeode *CreateSphereGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						 float fRadius, int res)
{
	vtGeode *pGeode = new vtGeode;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, iVertType, res*res*2);
	mesh->CreateEllipsoid(FPoint3(0,0,0), FPoint3(fRadius, fRadius, fRadius), res);
	pGeode->SetMaterials(pMats);
	pGeode->AddMesh(mesh, iMatIdx);
	return pGeode;
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
vtGeode *CreateCylinderGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
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

	vtGeode *pGeode = new vtGeode;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, iVertType, res*2);
	mesh->CreateCylinder(fHeight, fRadius, res, bTop, bBottom, bCentered);
	pGeode->SetMaterials(pMats);
	pGeode->AddMesh(mesh, iMatIdx);
	return pGeode;
}

/**
 * Create a grid of lines in the XZ plane.  This can be useful as a reference
 * object, like a sheet of graph paper.
 */
vtGeode *CreateLineGridGeom(const vtMaterialArray *pMats, int iMatIdx,
						   const FPoint3 &min1, const FPoint3 &max1, int steps)
{
	vtGeode *pGeode = new vtGeode;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, (steps+1)*4);

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
	pGeode->SetMaterials(pMats);
	pGeode->AddMesh(mesh, iMatIdx);
	return pGeode;
}


vtDynBoundBox::vtDynBoundBox(const RGBf &color)
{
	pGeode = new vtGeode;

	vtMaterialArrayPtr mats = new vtMaterialArray;
	mats->AddRGBMaterial(color, false, false, true);	// wire material
	pGeode->SetMaterials(mats);

	pMesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 8);
	for (int i = 0; i < 8; i++)
		pMesh->AddVertex(0,0,0);
	pMesh->AddLine(0, 1);
	pMesh->AddLine(1, 3);
	pMesh->AddLine(3, 2);
	pMesh->AddLine(2, 0);

	pMesh->AddLine(0, 4);
	pMesh->AddLine(1, 5);
	pMesh->AddLine(3, 7);
	pMesh->AddLine(2, 6);

	pMesh->AddLine(4, 5);
	pMesh->AddLine(5, 7);
	pMesh->AddLine(7, 6);
	pMesh->AddLine(6, 4);
	pGeode->AddMesh(pMesh, 0);
	pGeode->SetCastShadow(false);
}

void vtDynBoundBox::SetBox(const FBox3 &box)
{
	pMesh->SetVtxPos(0, FPoint3(box.min.x, box.min.y, box.min.z));
	pMesh->SetVtxPos(1, FPoint3(box.max.x, box.min.y, box.min.z));
	pMesh->SetVtxPos(2, FPoint3(box.min.x, box.min.y, box.max.z));
	pMesh->SetVtxPos(3, FPoint3(box.max.x, box.min.y, box.max.z));

	pMesh->SetVtxPos(4, FPoint3(box.min.x, box.max.y, box.min.z));
	pMesh->SetVtxPos(5, FPoint3(box.max.x, box.max.y, box.min.z));
	pMesh->SetVtxPos(6, FPoint3(box.min.x, box.max.y, box.max.z));
	pMesh->SetVtxPos(7, FPoint3(box.max.x, box.max.y, box.max.z));

	pMesh->ReOptimize();
}


///////////////////////////////////////////////////////////////////////

vtOBJFile *OBJFileBegin(vtGeode *geode, const char *filename)
{
	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
		return false;

	fprintf(fp, "#  Wavefront OBJ generated by the VTP software (http://vterrain.org/)\n\n");

	uint i;
	const vtMaterialArray *mats = geode->GetMaterials();
	uint num_mats = 0;
	if (mats)
		num_mats = mats->size();
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
			const vtMaterial *mat = mats->at(i).get();
			vtString matname;
			matname.Format("mat%03d", i);
			fprintf(fp2, "\nnewmtl %s\n", (const char *) matname);

			RGBf amb = mat->GetAmbient();
			RGBf dif = mat->GetDiffuse();
			RGBf spe = mat->GetSpecular();
			fprintf(fp2, "Ka %f %f %f\n", amb.r, amb.g, amb.b);
			fprintf(fp2, "Kd %f %f %f\n", dif.r, dif.g, dif.b);
			fprintf(fp2, "Ks %f %f %f\n", spe.r, spe.g, spe.b);

			osg::Image *tex = mat->GetTexture();
			if (tex)
			{
				vtString texfname = tex->getFileName().c_str();
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

void OBJFileWriteGeom(vtOBJFile *file, vtGeode *geode)
{
	uint i, j, k;
	uint num_mesh = geode->NumMeshes();
	for (i = 0; i < num_mesh; i++)
	{
		vtMesh *mesh = geode->GetMesh(i);
		if (!mesh)
			continue;

		vtMesh::PrimType ptype = mesh->getPrimType();

		// For now, this method only does tristrips, fans, and individual triangles
		if (ptype != osg::PrimitiveSet::TRIANGLE_STRIP && ptype != osg::PrimitiveSet::TRIANGLE_FAN
			 && ptype != osg::PrimitiveSet::TRIANGLES)
			continue;

		// First write the vertices
		int base_vert = file->verts_written;

		uint num_vert = mesh->NumVertices();
		fprintf(file->fp, "# %d vertices\n", num_vert);
		for (j = 0; j < num_vert; j++)
		{
			FPoint3 pos = mesh->GetVtxPos(j);
			fprintf(file->fp, "v %f %f %f\n", pos.x, pos.y, pos.z);
		}
		if (mesh->hasVertexTexCoords())
		{
			for (j = 0; j < num_vert; j++)
			{
				FPoint2 uv = mesh->GetVtxTexCoord(j);
				fprintf(file->fp, "vt %f %f\n", uv.x, uv.y);
			}
		}
		if (mesh->hasVertexNormals())
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

		uint num_prims = mesh->NumPrims();
		int idx0, idx1, idx2;
		if (ptype == osg::PrimitiveSet::TRIANGLES)
		{
			for (k = 0; k < num_prims; k++)
			{
				idx0 = mesh->GetIndex(k*3);
				idx1 = mesh->GetIndex(k*3+1);
				idx2 = mesh->GetIndex(k*3+2);
				fprintf(file->fp, "f %d %d %d\n",
					idx0+1, idx1+1, idx2+1);	// Wavefront indices are actually 1-based!

			}
		}
		if (ptype == osg::PrimitiveSet::TRIANGLE_STRIP || ptype == osg::PrimitiveSet::TRIANGLE_FAN)
		{
			// OBJ doesn't do strips, so break them into individual triangles
			int prim_start = 0;
			for (k = 0; k < num_prims; k++)
			{
				int len = mesh->GetPrimLen(k);
				for (int t = 0; t < len-2; t++)
				{
					if (ptype == osg::PrimitiveSet::TRIANGLE_STRIP)
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
					else if (ptype == osg::PrimitiveSet::TRIANGLE_FAN)
					{
						idx0 = mesh->GetIndex(prim_start);
						idx1 = mesh->GetIndex(prim_start + t+1);
						idx2 = mesh->GetIndex(prim_start + t+2);
					}
					// Wavefront indices are actually 1-based!
					idx0 = idx0 + base_vert + 1;
					idx1 = idx1 + base_vert + 1;
					idx2 = idx2 + base_vert + 1;
					if (mesh->hasVertexNormals() && mesh->hasVertexTexCoords())
						fprintf(file->fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
						idx0, idx0, idx0, idx1, idx1, idx1, idx2, idx2, idx2);
					else if (mesh->hasVertexNormals() || mesh->hasVertexTexCoords())
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
bool WriteGeomToOBJ(vtGeode *geode, const char *filename)
{
	vtOBJFile *file = OBJFileBegin(geode, filename);
	if (!file)
		return false;

	// Now write the geometry itself
	OBJFileWriteGeom(file, geode);
	fclose(file->fp);
	delete file;
	return true;
}

