//
// Globe.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"

#include "vtdata/shapelib/shapefil.h"

#include "Globe.h"
#include "vtdata/vtLog.h"	// for logging debug message
#include "Enviro.h"			// for GetTerrainScene

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

extern int icosa_face_v[21][3];

int icos_face_pairs[10][2] =
{
	{ 1, 6 },
	{ 2, 8 },
	{ 3, 10 },
	{ 4, 12 },
	{ 5, 14 },
	{ 7, 16 },
	{ 9, 17 },
	{ 11, 18 },
	{ 13, 19 },
	{ 15, 20 }
};

IcoGlobe::IcoGlobe()
{
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
void IcoGlobe::set_face_verts2(vtMesh *mesh, int face, float f)
{
	int pair;
	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icos_face_pairs[pair][0];
		int f2 = icos_face_pairs[pair][1];

		refresh_face_positions(m_mesh[f1], f1, f);
		refresh_face_positions(m_mesh[f2], f2, f);
	}
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

#if 0
	// naive way - no strips, no vertex sharing, independent triangles
	int size = 3;
	DPoint3 p0, p1, p2;
	FPoint3 vp0, vp1, vp2;
	for (j = 0; j < freq; j++)
	{
		for (i = 0; i < (freq-j); i++)
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

			// convert doubles -> floats
			vp0 = p0;
			vp1 = p1;
			vp2 = p2;

			m_mesh->SetVtxPos(m_vidx, vp0);
			m_mesh->SetVtxPos(m_vidx+1, vp1);
			m_mesh->SetVtxPos(m_vidx+2, vp2);

			FPoint2 coord;
			coord.u = (float)i / freq;
			coord.v = (float)j / freq;
			m_mesh->SetVtxTexCoord(m_vidx, coord);
			coord.u = (float)(i+1) / freq;
			coord.v = (float)j / freq;
			m_mesh->SetVtxTexCoord(m_vidx+1, coord);
			coord.u = (float)i / freq;
			coord.v = (float)(j+1) / freq;
			m_mesh->SetVtxTexCoord(m_vidx+2, coord);

			m_vidx += 3;
		}
	}
#else
	// better way, two passes
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
#endif
}

void IcoGlobe::add_face2(vtMesh *mesh, int face, bool second, float f)
{
	int i;
	int depth = 3;
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

	m_rtv[face].Append(v0);
	m_rtv[face].Append(v1);
	m_rtv[face].Append(v2);
	m_rtv[face].Append(e0);	// 3
	m_rtv[face].Append(e1);	// 4
	m_rtv[face].Append(e2);	// 5
	m_rtv[face].Append(center); // 6
	m_vert = 7;

	for (i = 0; i < 7; i++)
		mesh->AddVertexNUV(m_rtv[face][i].p, m_rtv[face][i].p, m_rtv[face][i].uv);

	// iterate the 6 subfaces
	add_subface(mesh, face, 0, 3, 6, false, depth, f);
	add_subface(mesh, face, 1, 3, 6, true, depth, f);

	add_subface(mesh, face, 1, 4, 6, false, depth, f);
	add_subface(mesh, face, 2, 4, 6, true, depth, f);

	add_subface(mesh, face, 2, 5, 6, false, depth, f);
	add_subface(mesh, face, 0, 5, 6, true, depth, f);

	// now, deal with curvature
	refresh_face_positions(mesh, face, f);
}

void IcoGlobe::refresh_face_positions(vtMesh *mesh, int face, float f)
{
	int i;
	double len;
	FPoint3 fp;
	int total = m_rtv[face].GetSize();
	for (i = 0; i < total; i++)
	{
		// do interpolation between icosa face and sphere
		len = m_rtv[face][i].p.Length();
		fp = m_rtv[face][i].p / len * (f + (1 - f) * len);
		mesh->SetVtxPos(i, fp);
	}
}

void IcoGlobe::add_subface(vtMesh *mesh, int face, int v0, int v1, int v2,
										 bool flip, int depth, float f)
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

		add_subface(mesh, face, v0, v4, v3, !flip, depth - 1, f);
		add_subface(mesh, face, v2, v4, v3,  flip, depth - 1, f);
		add_subface(mesh, face, v2, v1, v3, !flip, depth - 1, f);
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


void IcoGlobe::Create(int freq, const StringArray &paths, vtString strImagePrefix)
{
	VTLOG("IcoGlobe::Create\n");
	int i;
	m_freq = freq;

	InitIcosa();

	int numvtx = (freq + 1) * (freq + 2) / 2;
	for (i = 1; i <= 20; i++)
	{
		m_mesh[i] = new vtMesh(GL_TRIANGLE_STRIP, VT_Normals | VT_TexCoords, numvtx);
		m_mesh[i]->AllowOptimize(false);
	}

	int pair;
	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icos_face_pairs[pair][0];
		int f2 = icos_face_pairs[pair][1];

//		add_face1(m_mesh[f1], f1, false);
//		add_face1(m_mesh[f2], f2, true);
		add_face2(m_mesh[f1], f1, false, 1);
		add_face2(m_mesh[f2], f2, true, 1);
	}

	m_geom = new vtGeom();
	m_geom->SetName2("GlobeGeom");
	m_mgeom = new vtMovGeom(m_geom);
	m_mgeom->SetName2("GlobeShape");

	m_mats = new vtMaterialArray();
	bool bCulling = true;
	bool bLighting = false;

	m_red = m_mats->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f),	// red
					 false, false, true);
	m_yellow = m_mats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 0.0f),	// yellow
					 true, false, false);

	vtString base;
	vtString fname;
	vtString fullpath;

	int index;
	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icos_face_pairs[pair][0];
		int f2 = icos_face_pairs[pair][1];

		base = "WholeEarth/";
		base += strImagePrefix;
		base += "_";

		fname.Format("%s%02d%02d.jpg", (const char *)base, f1, f2);
		VTLOG("\t texture: %s\n", (const char *)fname);

		fullpath = FindFileOnPaths(paths, (const char *)fname);
		if (fullpath == "")
		{
			// try again with png
			fname.Format("%s%02d%02d.png", (const char *)base, f1, f2);
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
//		g_Log.Printf("\t\tindex: %d\n", index);

		if (index == -1)
		{
			VTLOG("\t\ttexture load failed, using red material.\n");
			m_globe_mat[pair] = m_red;
		}
		else
			m_globe_mat[pair] = index;
	}

	m_geom->SetMaterials(m_mats);

	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icos_face_pairs[pair][0];
		int f2 = icos_face_pairs[pair][1];
		m_geom->AddMesh(m_mesh[f1], m_globe_mat[pair]);
		m_geom->AddMesh(m_mesh[f2], m_globe_mat[pair]);
	}
}

//
// takes an argument from 0 (icosahedron) to 1 (sphere)
//
void IcoGlobe::SetInflation(float f)
{
	for (int pair = 0; pair < 10; pair++)
	{
		int f1 = icos_face_pairs[pair][0];
		int f2 = icos_face_pairs[pair][1];

		set_face_verts2(m_mesh[f1], f1, f);
		set_face_verts2(m_mesh[f2], f2, f);
	}
}

void IcoGlobe::SetLighting(bool bLight)
{
	for (int i = 0; i < 10; i++)
	{
		vtMaterial *pApp = m_mats->GetAt(m_globe_mat[i]);
		if (bLight)
		{
			pApp->SetLighting(true);
//			pApp->Set(APP_TextureOp, APP_Modulate);
		}
		else
		{
			pApp->SetLighting(false);
//			pApp->Set(APP_TextureOp, APP_Replace);
		}
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
		m_mgeom->AddChild(mgeom);
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

void IcoGlobe::AddTerrainRectangles()
{
	FPoint3 p;

	for (vtTerrain *pTerr = GetTerrainScene()->GetFirstTerrain(); pTerr;
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
		m_geom->AddMesh(mesh, m_red);
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


