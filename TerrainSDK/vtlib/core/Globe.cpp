//
// Globe.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Engine.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"

#include "vtdata/shapelib/shapefil.h"

#include "Globe.h"
#include "vtdata/vtLog.h"	// for logging debug message

vtMovGeom *CreateSimpleEarth(vtString strDataPath)
{
	// create simple texture-mapped sphere
	vtMesh *mesh = new vtMesh(GL_QUADS, VT_Normals | VT_TexCoords, 20*20*2);
	int res = 20;
	FPoint3 size(1.0f, 1.0f, 1.0f);
	mesh->CreateEllipsoid(size, res);

	// fix up the texture coordinates
	int numvtx = mesh->GetNumVertices();
	for (int i = 0; i < numvtx; i++)
	{
		FPoint2 coord;
		coord.y = 1.0f - ((float) i / (res * res));
		coord.x = 1.0f - ((float (i%res)) / res);
		mesh->SetVtxTexCoord(i, coord);
	}

	vtGeom *geom = new vtGeom();
	vtMovGeom *mgeom = new vtMovGeom(geom);
	mgeom->SetName2("GlobeGeom");

	vtMaterialArray *apps = new vtMaterialArray();
	bool bCulling = false;
	bool bLighting = false;
	bool bTransp = false;
	apps->AddTextureMaterial2(strDataPath + "WholeEarth/earth2k_free.jpg",
						 bCulling, bLighting, bTransp);
	geom->SetMaterials(apps);

	geom->AddMesh(mesh, 0);

	return mgeom;
}

/////////////////////////////////////////////////////

IcoGlobe::IcoGlobe()
{
	m_top = NULL;
	m_SurfaceGeom = NULL;
	m_mats = NULL;
}

//
// argument "f" goes from 0 (icosahedron) to 1 (sphere)
//
void IcoGlobe::set_face_verts1(vtMesh *mesh, int face, float f)
{
	int i, j;

	DPoint3 v0 = m_verts[icosa_face_v[face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[face][2]];
	DPoint3 vec0 = (v1 - v0)/m_freq, vec1 = (v2 - v0)/m_freq;
	DPoint3 p0, p1, p2, norm;
	FPoint3 fp0;
	double len, mag = 1.0;

	int idx = 0;
	for (j = 0; j <= m_freq; j++)
	{
		for (i = 0; i <= (m_freq-j); i++)
		{
			p0 = v0 + (vec0 * i) + (vec1 * j);

			// do interpolation between icosa face and sphere
			len = p0.Length();
			p0 = p0 / len * (f * mag + (1 - f) * len);

			// convert doubles -> floats
			fp0 = p0;

			mesh->SetVtxPos(idx, fp0);
			// for a spheroid (f == 1), this is correct:
			mesh->SetVtxNormal(idx, fp0);

			idx += 1;
		}
	}
	if (f != 1.0f)
	{
		// recalculate vertex normals from the current vertex locations
		idx = 0;
		DPoint3 d0, d1;
		for (j = 0; j <= m_freq; j++)
		{
			for (i = 0; i <= (m_freq-j); i++)
			{
				p0 = v0 + (vec0 * i) + (vec1 * j);
				p1 = v0 + (vec0 * (i+1)) + (vec1 * j);
				p2 = v0 + (vec0 * i) + (vec1 * (j+1));

				len = p0.Length();
				p0 = p0 / len * (f * mag + (1 - f) * len);
				len = p1.Length();
				p1 = p1 / len * (f * mag + (1 - f) * len);
				len = p2.Length();
				p2 = p2 / len * (f * mag + (1 - f) * len);

				d0 = (p1 - p0);
				d1 = (p2 - p0);
				d0.Normalize();
				d1.Normalize();

				norm = d0.Cross(d1);

				fp0 = norm;	// convert doubles to floats
				mesh->SetVtxNormal(idx, fp0);

				idx += 1;
			}
		}
	}
}

//
// argument "f" goes from 0 (icosahedron) to 1 (sphere)
//
void IcoGlobe::set_face_verts2(vtMesh *mesh, int mface, float f)
{
	refresh_face_positions(m_mesh[mface], mface, f);

	if (f == 1.0f)
	{
		// sphere normals pointing straight outwards
		for (int i = 0; i < m_rtv[mface].GetSize(); i++)
			mesh->SetVtxNormal(i, m_rtv[mface][i].p);
	}
	else
		m_mesh[mface]->SetNormalsFromPrimitives();
}

void IcoGlobe::add_face1(vtMesh *mesh, int face, bool second)
{
	float f = 1.0f;
	int i, j;

	DPoint3 v0 = m_verts[icosa_face_v[face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[face][2]];
	DPoint3 vec0 = (v1 - v0)/m_freq, vec1 = (v2 - v0)/m_freq;
	DPoint3 p0;
	FPoint3 vp0;
	double len, mag = 1.0;

	// two passes
	// first pass: create the vertices
	int idx = 0;
	int vtx_base = 0;
	for (j = 0; j <= m_freq; j++)
	{
		for (i = 0; i <= (m_freq-j); i++)
		{
			p0 = v0 + (vec0 * i) + (vec1 * j);

			// do interpolation between icosa face and sphere
			len = p0.Length();
			p0 = p0 / len * (f * mag + (1 - f) * len);

			// convert doubles -> floats
			vp0 = p0;

			mesh->SetVtxPos(idx, vp0);
			// for a spheroid (f == 1), this is correct:
			mesh->SetVtxNormal(idx, vp0);

			FPoint2 coord;
			if (second)
			{
				coord.x = 1.0f - (float)i / m_freq;
				coord.y = 1.0f - (float)j / m_freq;
			}
			else
			{
				coord.x = (float)i / m_freq;
				coord.y = (float)j / m_freq;
			}
			mesh->SetVtxTexCoord(idx, coord);

			idx += 1;
		}
	}
	// Next pass: the strips
	int row_start = 0;
	unsigned short *indices = new unsigned short[m_freq * 2 + 2];

	for (j = 0; j < m_freq; j++)
	{
		int row_len = (m_freq-j) + 1;

		// Number of vertices in this strip:
		int size = row_len * 2 - 1;
		int count = 0;

		indices[count++] = vtx_base + row_start + row_len-1;
		for (i = row_len-2; i >= 0; i--)
		{
			indices[count++] = vtx_base + row_start + i + row_len;
			indices[count++] = vtx_base + row_start + i;
		}
		mesh->AddStrip(count, indices);

		row_start += row_len;
	}
	delete [] indices;
}

void IcoGlobe::add_face2(vtMesh *mesh, int face, int mface, int subfaces, bool second)
{
	int i;
	IcoVert v0, v1, v2, e0, e1, e2, center;

	v0.p = m_verts[icosa_face_v[face][0]];
	v1.p  = m_verts[icosa_face_v[face][1]];
	v2.p  = m_verts[icosa_face_v[face][2]];

	// find edges and center
	e0.p = (v0.p + v1.p) / 2;
	e1.p = (v1.p + v2.p) / 2;
	e2.p = (v2.p + v0.p) / 2;
	center.p = (v0.p + v1.p + v2.p) / 3;

	FPoint2 uv[3], edge_uv[3], center_uv;
	if (second == false)
	{
		v0.uv.Set(0, 0);
		v1.uv.Set(1, 0);
		v2.uv.Set(0, 1);
	}
	else
	{
		v0.uv.Set(1, 1);
		v1.uv.Set(0, 1);
		v2.uv.Set(1, 0);
	}
	// find edge and center uv
	e0.uv = (v0.uv + v1.uv) / 2;
	e1.uv = (v1.uv + v2.uv) / 2;
	e2.uv = (v2.uv + v0.uv) / 2;
	center.uv = (v0.uv + v1.uv + v2.uv) / 3;

	m_rtv[mface].Append(v0);
	m_rtv[mface].Append(v1);
	m_rtv[mface].Append(v2);
	m_rtv[mface].Append(e0);	// 3
	m_rtv[mface].Append(e1);	// 4
	m_rtv[mface].Append(e2);	// 5
	m_rtv[mface].Append(center); // 6
	m_vert = 7;

	for (i = 0; i < 7; i++)
		mesh->AddVertexNUV(m_rtv[mface][i].p, m_rtv[mface][i].p, m_rtv[mface][i].uv);

	// iterate the 6 subfaces
	if (subfaces & (1<<1))
		add_subface(mesh, mface, 0, 3, 6, false, m_depth);
	if (subfaces & (1<<2))
		add_subface(mesh, mface, 1, 3, 6, true, m_depth);

	if (subfaces & (1<<3))
		add_subface(mesh, mface, 1, 4, 6, false, m_depth);
	if (subfaces & (1<<4))
		add_subface(mesh, mface, 2, 4, 6, true, m_depth);

	if (subfaces & (1<<5))
		add_subface(mesh, mface, 2, 5, 6, false, m_depth);
	if (subfaces & (1<<6))
		add_subface(mesh, mface, 0, 5, 6, true, m_depth);

	// now, deal with curvature
	refresh_face_positions(mesh, mface, 1);
}

void IcoGlobe::refresh_face_positions(vtMesh *mesh, int mface, float f)
{
	int i;
	double len;
	FPoint3 fp;
	int total = m_rtv[mface].GetSize();
	for (i = 0; i < total; i++)
	{
		// do interpolation between icosa face and sphere
		len = m_rtv[mface][i].p.Length();
		fp = m_rtv[mface][i].p / len * (f + (1 - f) * len);

		if (m_style == DYMAX_UNFOLD)
		{
			fp -= m_local_origin[mface];
		}

		mesh->SetVtxPos(i, fp);
	}
}

void IcoGlobe::add_subface(vtMesh *mesh, int face, int v0, int v1, int v2,
						   bool flip, int depth)
{
	if (depth > 0)
	{
		IcoVert p3, p4;

		p3.p = m_rtv[face][v1].p - (m_rtv[face][v1].p - m_rtv[face][v0].p) / 3;
		p4.p = (m_rtv[face][v0].p + m_rtv[face][v2].p) / 2;

		p3.uv = m_rtv[face][v1].uv - (m_rtv[face][v1].uv - m_rtv[face][v0].uv) / 3;
		p4.uv = (m_rtv[face][v0].uv + m_rtv[face][v2].uv) / 2;

		int v3 = m_vert++;
		int v4 = m_vert++;

		m_rtv[face].Append(p3);
		m_rtv[face].Append(p4);

		mesh->AddVertexNUV(m_rtv[face][v3].p, m_rtv[face][v3].p, m_rtv[face][v3].uv);
		mesh->AddVertexNUV(m_rtv[face][v4].p, m_rtv[face][v4].p, m_rtv[face][v4].uv);

		add_subface(mesh, face, v0, v4, v3, !flip, depth - 1);
		add_subface(mesh, face, v2, v4, v3,  flip, depth - 1);
		add_subface(mesh, face, v2, v1, v3, !flip, depth - 1);
		return;
	}

	unsigned short Indices[3];
	if (flip)
	{
		Indices[0] = v2;
		Indices[1] = v1;
		Indices[2] = v0;
	}
	else
	{
		Indices[0] = v0;
		Indices[1] = v1;
		Indices[2] = v2;
	}
	mesh->AddStrip(3, Indices);
}


void IcoGlobe::CreateMaterials(const StringArray &paths, const vtString &strImagePrefix)
{
	m_mats = new vtMaterialArray();
	bool bCulling = true;
	bool bLighting = false;

	m_red = m_mats->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f),	// red
					 false, false, true);
	m_yellow = m_mats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 0.0f),	// yellow
					 false, false, false);
	m_white = m_mats->AddRGBMaterial1(RGBf(0.2f, 0.2f, 0.2f),
					 true, true, true, 1);
	vtMaterial *mat = m_mats->GetAt(m_white);
	mat->SetTransparent(true, true);

	vtString base;
	vtString fname;
	vtString fullpath;

	int pair, index;
	for (pair = 0; pair < 10; pair++)
	{
		if (strImagePrefix == "")
		{
			m_globe_mat[pair] = m_white;
			continue;
		}
		int f1 = icosa_face_pairs[pair][0];
		int f2 = icosa_face_pairs[pair][1];

		base = "WholeEarth/";
		base += strImagePrefix;
		base += "_";

		fname.Format("%s%02d%02d.jpg", (const char *)base, f1+1, f2+1);
		VTLOG("\t texture: %s\n", (const char *)fname);

		fullpath = FindFileOnPaths(paths, (const char *)fname);
		if (fullpath == "")
		{
			// try again with png
			fname.Format("%s%02d%02d.png", (const char *)base, f1+1, f2+1);
			VTLOG("\t texture: %s\n", (const char *)fname);
			fullpath = FindFileOnPaths(paths, (const char *)fname);
		}
		if (fullpath == "")
		{
			VTLOG("\t\tnot found on data paths.\n");
			index = -1;
		}
		index = m_mats->AddTextureMaterial2(fullpath,
					 bCulling, bLighting,
					 false, false,				// transp, additive
					 0.1f, 1.0f, 1.0f, 0.0f,	// ambient, diffuse, alpha, emmisive
					 false, true, false);		// texgen, clamp, mipmap

		if (index == -1)
		{
			VTLOG("\t\ttexture load failed, using red material.\n");
			m_globe_mat[pair] = m_red;
		}
		else
			m_globe_mat[pair] = index;
	}
}

void IcoGlobe::SetCulling(bool bCull)
{
	int pair;
	for (pair = 0; pair < 10; pair++)
	{
		vtMaterial *mat = m_mats->GetAt(m_globe_mat[pair]);
		mat->SetCulling(bCull);
	}
}

// This array describes the configuration and topology of the subfaces in
// the flattened dymaxion map.
struct dymax_info
{
	int face;
	int subfaces;
	int parent_face;
	int parent_mface;
	int parentedge;
}
dymax_subfaces[22] =
{
	{  0, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, -1, -1, -1 }, // 0

	{  1, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  0, 0, 2 },	// 1
	{  4, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  0, 0, 0 },	// 2
	{  5, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  0, 0, 1 },	// 3

	{  7, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  1, 1, 1 },	// 4
	{  2, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  1, 1, 2 },	// 5

	{  6, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  7, 4, 2 },	// 6
	{  8,               1<<4 | 1<<3 | 1<<2 | 1<<1,  7, 4, 0 },	// 7
	{  9, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  2, 5, 1 },	// 8
	{  3, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  2, 5, 2 },	// 9

	{ 16, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  8, 7, 1 },	// 10
	{  8, 1<<6 | 1<<5,                              9, 8, 2 },	// 11
	{ 11, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  3, 9, 1 },	// 12

	{ 15,               1<<4 | 1<<3 | 1<<2,        16, 10, 2 },	// 13
	{ 10, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 11, 12, 2 },	// 14
	{ 12, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 11, 12, 0 },	// 15

	{ 17, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 10, 14, 1 },	// 16
	{ 18, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 12, 15, 1 },	// 17
	{ 13, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 12, 15, 2 },	// 18

	{ 19, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 18, 17, 0 },	// 19

	{ 15, 1<<6 | 1<<5 |                      1<<1, 19, 19, 0 },	// 20
	{ 14, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 19, 19, 1 },	// 21
};

class TestEngine : public vtEngine
{
public:
	TestEngine() { f = 0; dir = -0.004; }
	void Eval() {
		f += dir;
		if (f < -globe->DihedralAngle())
			dir = 0.004;
		if (f > 0)
			dir = -0.004;
		globe->DoTest(f);
	}
	IcoGlobe *globe;
	float f, dir;
};

int GetMaterialForFace(int face, bool &which)
{
	for (int i = 0; i < 10; i++)
	{
		if (icosa_face_pairs[i][0] == face)
		{
			which = false;
			return i;
		}
		if (icosa_face_pairs[i][1] == face)
		{
			which = true;
			return i;
		}
	}
	return -1;
}

void IcoGlobe::FindLocalOrigin(int mface)
{
	int face = dymax_subfaces[mface].face;
	int parent_face = dymax_subfaces[mface].parent_face;
	int edge = dymax_subfaces[mface].parentedge;

	DPoint3 v0 = m_verts[icosa_face_v[parent_face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[parent_face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[parent_face][2]];

	// find the center of each edge
	DPoint3 ec[3], axis[3];
	ec[0] = (v0 + v1) / 2;
	ec[1] = (v1 + v2) / 2;
	ec[2] = (v2 + v0) / 2;
	axis[0] = v1 - v0;
	axis[1] = v2 - v1;
	axis[2] = v0 - v2;

	m_local_origin[mface] = ec[edge];
	m_axis[mface] = axis[edge];
	m_axis[mface].Normalize();
}

void IcoGlobe::SetMeshConnect(int mface)
{
	int parent_face = dymax_subfaces[mface].parent_face;
	int parent_mface = dymax_subfaces[mface].parent_mface;

	vtTransform *xform = m_xform[mface];
	vtMesh *mesh = m_mesh[mface];

	// attach heirarchy
	m_xform[parent_mface]->AddChild(xform);

	// translate vertices to set origin of this mface
	int i;
	int verts = mesh->GetNumVertices();
	FPoint3 pos;

	FPoint3 edge_center = m_local_origin[mface];

	for (i = 0; i < verts; i++)
	{
		pos = mesh->GetVtxPos(i);
		pos -= edge_center;
		mesh->SetVtxPos(i, pos);
	}
	if (parent_mface == 0)
		xform->Translate1(edge_center);
	else
		xform->Translate1(edge_center - m_local_origin[parent_mface]);
}

void IcoGlobe::DoTest(float f)
{
//	SetUnfolding(f);
//	m_xform[0]->Identity();
//	m_xform[0]->RotateLocal(m_flat_axis, m_flat_angle * f);
}

void IcoGlobe::SetUnfolding(float f)
{
	// only possible on unfoldable globes
	if (m_style != DYMAX_UNFOLD)
		return;

	float dih = (float) DihedralAngle();
	for (int i = 1; i < 22; i++)
	{
		FPoint3 pos = m_xform[i]->GetTrans();
		m_xform[i]->Identity();
		m_xform[i]->SetTrans(pos);
		m_xform[i]->RotateLocal(m_axis[i], -f * dih);
	}

	// deflate as we unfold
	SetInflation(1.0f-f);

	// interpolate from No rotation to the desired rotation
	FQuat qnull;
	qnull.Set(0,0,0,1);
	FQuat q;
	q.Slerp(qnull, m_diff, f);

	FMatrix3 m3;
	q.GetMatrix(m3);

	FMatrix4 m4 = m3;
	m_xform[0]->SetTransform1(m4);
}

void IcoGlobe::SetTime(time_t time)
{
	tm *gmt = gmtime(&time);

	float second_of_day = (gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec;
	float fraction_of_day = second_of_day / (24 * 60 * 60);
	float rotation = fraction_of_day * PI2f;

	// match with actual globe
	rotation = PI2f + rotation;
	rotation -= PID2f;

	m_top->Identity();

	// seasonal axis tilt (TODO)
//	m_top->RotateLocal(FPoint3(1,0,0), tilt);

	// rotation around axis
	m_top->RotateLocal(FPoint3(0,1,0), rotation);
}

/**
 * Create the globe's geometry and nodes.
 */
void IcoGlobe::Create(int iTriangleCount, const StringArray &paths,
					  const vtString &strImagePrefix, Style style)
{
	VTLOG("IcoGlobe::Create\n");

	m_style = style;
	InitIcosa();

	CreateMaterials(paths, strImagePrefix);

	EstimateTesselation(iTriangleCount);

	// Estimate number of meshes, and number of vertices per mesh
	int i, numvtx;
	if (m_style == GEODESIC)
	{
		numvtx = (m_freq + 1) * (m_freq + 2) / 2;
		m_mfaces = 20;
	}
	else if (style == RIGHT_TRIANGLE || style == DYMAX_UNFOLD)
	{
		numvtx = 1 + 2 * ((int) pow(3, m_depth+1));
		m_mfaces = (style == RIGHT_TRIANGLE) ? 20 : 22;
	}

	for (i = 0; i < m_mfaces; i++)
	{
		m_mesh[i] = new vtMesh(GL_TRIANGLE_STRIP, VT_Normals | VT_TexCoords, numvtx);
		if (strImagePrefix == "")
			m_mesh[i]->AllowOptimize(true);
		else
			m_mesh[i]->AllowOptimize(false);
	}

	m_top = new vtTransform;
	m_top->SetName2("GlobeXForm");

	if (style == DYMAX_UNFOLD)
		CreateUnfoldableDymax();
	else
		CreateNormalSphere();
}

void IcoGlobe::EstimateTesselation(int iTriangleCount)
{
	int per_face = iTriangleCount / 20;
	if (m_style == GEODESIC)
	{
		// Frequency for a traditional geodesic tiling gives (frequency ^ 2)
		// triangles.  Find what frequency most closely matches the desired
		// triangle count.
		double exact = sqrt(per_face);
		int iLess = floor(exact);
		int iMore = ceil(exact);
		if ((iMore*iMore - per_face) < (per_face - iLess*iLess))
			m_freq = iMore;
		else
			m_freq = iLess;
	}
	else if (m_style == RIGHT_TRIANGLE || m_style == DYMAX_UNFOLD)
	{
		// Recursive right-triangle subdivision gives (2 * 3 ^ (depth+1))
		// triangles.  Find what depth most closely matches the desired
		// triangle count.
		int a, b = 6;
		for (a = 1; a < 10; a++)
		{
			b *= 3;
			if (b > per_face) break;
		}
		if ((b - per_face) < (per_face - b/3))
			m_depth = a;
		else
			m_depth = a-1;
	}
}

void IcoGlobe::CreateUnfoldableDymax()
{
	int i;
	for (i = 0; i < 22; i++)
	{
		m_xform[i] = new vtTransform;
		vtGeom *geom = new vtGeom;
		m_xform[i]->AddChild(geom);

		vtString str;
		str.Format("IcoFace %d", i);
		m_xform[i]->SetName2(str);

		int face = dymax_subfaces[i].face;
		int subfaces = dymax_subfaces[i].subfaces;

		bool which;
		int mat = GetMaterialForFace(face, which);

		add_face2(m_mesh[i], face, i, subfaces, which);

		geom->SetMaterials(m_mats);
		geom->AddMesh(m_mesh[i], m_globe_mat[mat]);
	}
	m_top->AddChild(m_xform[0]);

	for (i = 1; i < 22; i++)
		FindLocalOrigin(i);
	for (i = 1; i < 22; i++)
		SetMeshConnect(i);

	// Determine necessary rotation to orient flat map toward viewer.
	FQuat q1, q2, q3;
	FPoint3 v0 = m_verts[icosa_face_v[0][0]];
	FPoint3 v1 = m_verts[icosa_face_v[0][1]];
	FPoint3 v2 = m_verts[icosa_face_v[0][2]];

	// First, a rotation to turn the globe so that the edge faces down Z
	FPoint3 edge = v0 - v2;
	FPoint3 fnorm = (v0 + v1 + v2).Normalize();
	q1.SetFromVectors(edge, fnorm);
	q1.Invert();

	// then a rotation to turn it toward the user
	q2.SetFromVectors(FPoint3(0,1,0), FPoint3(0,0,1));

	// then a rotation to spin it PI/2
	q3.SetFromVectors(FPoint3(0,0,-1), FPoint3(1,0,0));

	// combine them
	m_diff = q1 * q2 * q3;

	// Create a geom to contain the surface mesh features
	m_SurfaceGeom = new vtGeom();
	m_SurfaceGeom->SetName2("SurfaceGeom");
	m_SurfaceGeom->SetMaterials(m_mats);
	m_top->AddChild(m_SurfaceGeom);

//	TestEngine *test = new TestEngine();
//	test->SetName2("Globe Test Engine");
//	test->globe = this;
//	vtGetScene()->AddEngine(test);
}

void IcoGlobe::CreateNormalSphere()
{
	// Create the meshes
	int pair;
	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icosa_face_pairs[pair][0];
		int f2 = icosa_face_pairs[pair][1];

		if (m_style == GEODESIC)
		{
			add_face1(m_mesh[f1], f1, false);
			add_face1(m_mesh[f2], f2, true);
		}
		else if (m_style == RIGHT_TRIANGLE)
		{
			int all = 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1;
			add_face2(m_mesh[f1], f1, f1, all, false);
			add_face2(m_mesh[f2], f2, f2, all, true);
		}
	}

	// Create a geom to contain the meshes
	m_SurfaceGeom = new vtGeom();
	m_SurfaceGeom->SetName2("GlobeGeom");
	m_SurfaceGeom->SetMaterials(m_mats);

	m_top->AddChild(m_SurfaceGeom);

	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icosa_face_pairs[pair][0];
		int f2 = icosa_face_pairs[pair][1];
		m_SurfaceGeom->AddMesh(m_mesh[f1], m_globe_mat[pair]);
		m_SurfaceGeom->AddMesh(m_mesh[f2], m_globe_mat[pair]);
	}
}

/**
 * Set the amount of inflation of the globe.
 *
 * \param f Ranges from 0 (icosahedron) to 1 (sphere)
 */
void IcoGlobe::SetInflation(float f)
{
	for (int mface = 0; mface < m_mfaces; mface++)
	{
		if (m_style == GEODESIC)
			set_face_verts1(m_mesh[mface], mface, f);
		else if (m_style == RIGHT_TRIANGLE || m_style == DYMAX_UNFOLD)
			set_face_verts2(m_mesh[mface], mface, f);
	}
}

void IcoGlobe::SetLighting(bool bLight)
{
	for (int i = 0; i < 10; i++)
	{
		vtMaterial *pApp = m_mats->GetAt(m_globe_mat[i]);
		pApp->SetLighting(bLight);
	}
}

void IcoGlobe::AddPoints(DLine2 &points, float fSize)
{
	int i, j, size;
	Array<FSphere> spheres;

	size = points.GetSize();
	spheres.SetSize(size);

	int n = 0, s = 0;
	for (i = 0; i < size; i++)
	{
		DPoint2 p = points[i];
		if (p.x == 0.0 && p.y == 0.0)
			continue;

		FPoint3 loc;
		geo_to_xyz(1.0, points[i], loc);

		if (loc.y > 0)
			n++;
		else
			s++;

		spheres[i].center = loc;
		spheres[i].radius = fSize;
	}

	FPoint3 diff;

	// volume of sphere, 4/3 PI r^3
	// surface area of sphere, 4 PI r^2
	// area of circle of sphere as seen from distance, PI r^2
	int merges;
	do {
		merges = 0;
		// Try merging overlapping points together, so that information
		// is not lost in the overlap.
		// To consider: do we combine the blobs based on their 2d radius,
		// their 2d area, their 3d radius, or their 3d volume?  See
		// Tufte, http://www.edwardtufte.com/
		// Implemented here: preserve 2d area
		for (i = 0; i < size-1; i++)
		{
			for (j = i+1; j < size; j++)
			{
				if (spheres[i].radius == 0.0f || spheres[j].radius == 0.0f)
					continue;

				diff = spheres[i].center - spheres[j].center;

				// if one sphere contains the center of the other
				if (diff.Length() < spheres[i].radius ||
					diff.Length() < spheres[j].radius)
				{
					// combine
					float area1 = PIf * spheres[i].radius * spheres[i].radius;
					float area2 = PIf * spheres[j].radius * spheres[j].radius;
					float combined = (area1 + area2);
					float newrad = sqrtf( combined / PIf );
					// larger eats the smaller
					if (area1 > area2)
					{
						spheres[i].radius = newrad;
						spheres[j].radius = 0.0f;
					}
					else
					{
						spheres[j].radius = newrad;
						spheres[i].radius = 0.0f;
					}
					merges++;
					break;
				}
			}
		}
		int got = merges;
	}
	while (merges != 0);

	// Now create and place the little geometry objects to represent the
	// point data.

#if 0
	// create simple hemisphere mesh
	int res = 6;
	vtMesh *mesh = new vtMesh(GL_TRIANGLE_STRIP, 0, res*res*2);
	FPoint3 scale(1.0f, 1.0f, 1.0f);
	mesh->CreateEllipsoid(scale, res, true);
#else
	// create cylinder mesh instead
	int res = 14;
	int verts = res * 2;
	vtMesh *mesh = new vtMesh(GL_TRIANGLE_STRIP, 0, verts);
	mesh->CreateCylinder(1.0f, 1.0f, res, true, false, false);
#endif

	// use Area to show amount, otherwise height
	bool bArea = true;

	// create and place the geometries
	size = points.GetSize();
	for (i = 0; i < size; i++)
	{
		if (spheres[i].radius == 0.0f)
			continue;

		vtGeom *geom = new vtGeom();
		geom->SetMaterials(m_mats);
		geom->AddMesh(mesh, m_yellow);

		vtMovGeom *mgeom = new vtMovGeom(geom);
		mgeom->SetName2("GlobeShape");

		mgeom->PointTowards(spheres[i].center);
		mgeom->RotateLocal(FPoint3(1,0,0), -PID2f);
		mgeom->SetTrans(spheres[i].center);
		if (bArea)
		{
			// scale just the radius of the cylinder
			mgeom->Scale3(spheres[i].radius, 0.001f, spheres[i].radius);
		}
		else
		{
			// scale just the height of the cylinder
			double area = PIf * spheres[i].radius * spheres[i].radius;
			mgeom->Scale3(0.002f, area*1000, 0.002f);
		}
		m_top->AddChild(mgeom);
	}

}

//
// Ray-Sphere intersection
//
bool FindIntersection(const FPoint3 &origin, const FPoint3 &dir,
					  const FSphere &sphere, int &iQuantity, FPoint3 point[2])
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	FPoint3 kDiff = origin - sphere.center;
	double fA = dir.LengthSquared();
	double fB = kDiff.Dot(dir);
	double fC = kDiff.LengthSquared() -
		sphere.radius*sphere.radius;

	double afT[2];
	double fDiscr = fB*fB - fA*fC;
	if ( fDiscr < 0.0 )
	{
		iQuantity = 0;
	}
	else if ( fDiscr > 0.0 )
	{
		double fRoot = sqrt(fDiscr);
		double fInvA = 1.0/fA;
		afT[0] = (-fB - fRoot)*fInvA;
		afT[1] = (-fB + fRoot)*fInvA;

		if ( afT[0] >= 0.0 )
			iQuantity = 2;
		else if ( afT[1] >= 0.0 )
			iQuantity = 1;
		else
			iQuantity = 0;
	}
	else
	{
		afT[0] = -fB/fA;
		iQuantity = ( afT[0] >= 0.0 ? 1 : 0 );
	}

	for (int i = 0; i < iQuantity; i++)
		point[i] = origin + dir * afT[i];

	return iQuantity > 0;
}


void geo_to_xyz(double radius, const DPoint2 &geo, FPoint3 &p)
{
	DPoint3 dp;
	geo_to_xyz(radius, geo, dp);
	p = dp;
}

void geo_to_xyz(double radius, const DPoint2 &geo, DPoint3 &p)
{
	// Convert spherical polar coordinates to cartesian coordinates
	// The angles are given in degrees
	double theta = geo.x / 180.0 * PId;
	double phi = (geo.y / 180.0 * PId);

	phi += PID2d;
	theta = PI2d - theta;

	double x = sin(phi) * cos(theta) * radius;
	double y = sin(phi) * sin(theta) * radius;
	double z = cos(phi) * radius;

	p.x = x;
	p.y = -z;
	p.z = y;
}

double radians(double degrees)
{
	return degrees / 180.0 * PId;
}

void xyz_to_geo(double radius, const FPoint3 &p, DPoint3 &geo)
{
	double x = p.x / radius;
	double y = p.z / radius;
	double z = -p.y / radius;

	double a, lat, lng;
	lat = acos(z);
	if (x>0.0 && y>0.0) a = radians(0.0);
	if (x<0.0 && y>0.0) a = radians(180.0);
	if (x<0.0 && y<0.0) a = radians(180.0);
	if (x>0.0 && y<0.0) a = radians(360.0);
	if (x==0.0 && y>0.0) lng = radians(90.0);
	if (x==0.0 && y<0.0) lng = radians(270.0);
	if (x>0.0 && y==0.0) lng = radians(0.0);
	if (x<0.0 && y==0.0) lng = radians(180.0);
	if (x!=0.0 && y!=0.0) lng = atan(y/x) + a;

	lng = PI2d - lng;
	lat = lat - PID2d;

	// convert angles to degrees
	geo.x = lng * 180.0 / PId;
	geo.y = lat * 180.0 / PId;

	// keep in expected range
	if (geo.x > 180.0) geo.x -= 360.0;

	// we don't know elevation (yet)
	geo.z = 0.0f;

	int foo = 1;
}

double IcoGlobe::AddSurfaceLineToMesh(vtMesh *mesh, const DPoint2 &g1, const DPoint2 &g2)
{
	// first determine how many points we should use for a smooth arc
	DPoint3 p1, p2;
	geo_to_xyz(1.0, g1, p1);
	geo_to_xyz(1.0, g2, p2);
	double angle = acos(p1.Dot(p2));
	int points = (int) (angle * 3000);
	if (points < 3)
		points = 3;

	// calculate the axis of rotation
	DPoint3 cross = p1.Cross(p2);
	cross.Normalize();
	double angle_spacing = angle / (points-1);
	DMatrix4 rot4;
	rot4.AxisAngle(cross, angle_spacing);
	DMatrix3 rot3;
	rot3.SetByMatrix4(rot4);

	// curved arc on great-circle path
	int start = mesh->GetNumVertices();
	for (int i = 0; i < points; i++)
	{
		FPoint3 fp = p1 * 1.0002;
		mesh->AddVertex(fp);
		rot3.Transform(p1, p2);
		p1 = p2;
	}
	mesh->AddStrip2(points, start);
	return angle;
}

void IcoGlobe::AddTerrainRectangles(vtTerrainScene *pTerrainScene)
{
	FPoint3 p;

	for (vtTerrain *pTerr = pTerrainScene->GetFirstTerrain(); pTerr;
			pTerr=pTerr->GetNext())
	{
		// skip if undefined
		if (pTerr->m_Corners_geo.GetSize() == 0)
			continue;

		int numvtx = 4;
		vtMesh *mesh = new vtMesh(GL_LINE_STRIP, 0, numvtx);

		int i, j;
		DPoint2 p1, p2;
		for (i = 0; i < 4; i++)
		{
			j = (i+1) % 4;
			p1 = pTerr->m_Corners_geo[i];
			p2 = pTerr->m_Corners_geo[j];
			AddSurfaceLineToMesh(mesh, p1, p2);
		}
		m_SurfaceGeom->AddMesh(mesh, m_red);
	}
}


int IcoGlobe::AddGlobePoints(const char *fname)
{
	SHPHandle hSHP = SHPOpen(fname, "rb");
	if (hSHP == NULL)
	{
		return -1;
	}

	int		nEntities, nShapeType;
	double 	adfMinBound[4], adfMaxBound[4];
	DPoint2 point;
	DLine2	points;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);
	if (nShapeType != SHPT_POINT)
	{
		SHPClose(hSHP);
		return -2;
	}

	for (int i = 0; i < nEntities; i++)
	{
		SHPObject *psShape = SHPReadObject(hSHP, i);
		point.x = psShape->padfX[0];
		point.y = psShape->padfY[0];
		if (point.x != 0.0 || point.y != 0.0)
			points.Append(point);
		SHPDestroyObject(psShape);
	}
//	AddPoints(points, 0.0015f);	// this size works OK for the VTP recipients
	AddPoints(points, 0.0005f);
	SHPClose(hSHP);
	return nEntities;
}

