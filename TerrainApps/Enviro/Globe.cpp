//
// Globe.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"

#include "vtdata/shapelib/shapefil.h"

#include "Globe.h"
#include "TerrainSceneWP.h"
#include "Enviro.h"		// for logging debug message

vtMovGeom *CreateSimpleEarth(vtString strDataPath)
{
	// create simple texture-mapped sphere
	vtMesh *mesh = new vtMesh(GL_QUADS, VT_Normals | VT_TexCoords, 20*20*2);
	int appidx = 0;
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
	apps->AddTextureMaterial2(strDataPath + "WholeEarth/earth2k_free.bmp",
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

//
// argument "f" goes from 0 (icosahedron) to 1 (sphere)
//
void IcoGlobe::set_face_verts(vtMesh *mesh, int face, float f)
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

			fp0 = p0;	// convert doubles -> floats

			mesh->SetVtxPos(idx, fp0);
			mesh->SetVtxNormal(idx, fp0);	// for f == 1.0

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

void IcoGlobe::add_face(vtMesh *mesh, int face, int appidx, bool second)
{
	float f = 1.0f;
	int i, j;

	DPoint3 v0 = m_verts[icosa_face_v[face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[face][2]];
	DPoint3 vec0 = (v1 - v0)/m_freq, vec1 = (v2 - v0)/m_freq;
	double len;

#if 0
	// old way - no strips, no vertex sharing, independent triangles
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
	// new way, two passes
	// first pass: create the vertices
	int idx = 0;
	int vtx_base = 0;
	DPoint3 p0;
	FPoint3 vp0;
	double mag = 1.0;
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
			// for a spheroid, this is correct:
			mesh->SetVtxNormal(idx, vp0);

			FPoint2 coord;
			if (second)
			{
				coord.x = 1.0f - (float)i / m_freq;
				coord.y = (float)j / m_freq;
			}
			else
			{
				coord.x = (float)i / m_freq;
				coord.y = 1.0f - (float)j / m_freq;
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
	delete indices;
#endif
}

void IcoGlobe::Create(int freq, vtString strDataPath, vtString strImagePrefix)
{
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

		add_face(m_mesh[f1], f1, pair, false);
		add_face(m_mesh[f2], f2, pair, true);
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
					 false, false, false);

	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icos_face_pairs[pair][0];
		int f2 = icos_face_pairs[pair][1];

		vtString base = strDataPath;
		base += "WholeEarth/";
		base += strImagePrefix;
		base += "_";

		vtString fname;
		fname.Format("%s%02d%02d.png", (const char *)base, f1, f2);

		vtString msg;
		msg.Format("\t texture: %s\n", fname);
		g_App._Log(msg);

		int index = m_mats->AddTextureMaterial2(fname,
						 bCulling, bLighting,
						 false, false,				// transp, additive
						 0.1f, 1.0f, 1.0f, 0.0f,	// ambient, diffuse, alpha, emmisive
						 false, true, false);		// texgen, clamp, mipmap
		msg.Format("\t\tindex: %d\n", index);
		g_App._Log(msg);

		if (index == -1)
			m_globe_mat[pair] = m_red;
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

		set_face_verts(m_mesh[f1], f1, f);
		set_face_verts(m_mesh[f2], f2, f);
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
	// create simple texture-mapped sphere
	int res = 5;
	vtMesh *mesh = new vtMesh(GL_TRIANGLE_STRIP, 0, res*res*2);
	FPoint3 size(fSize, fSize, fSize);
	mesh->CreateEllipsoid(size, res);

	for (int i = 0; i < points.GetSize(); i++)
	{
		DPoint2 p = points[i];
		if (p.x == 0.0 && p.y == 0.0)
			continue;

		vtGeom *geom = new vtGeom();
		geom->SetMaterials(m_mats);
		geom->AddMesh(mesh, m_yellow);

		vtMovGeom *mgeom = new vtMovGeom(geom);
		mgeom->SetName2("GlobeShape");

		FPoint3 loc;
		geo_to_xyz(1.0, points[i], loc);
		mgeom->SetTrans(loc);
		m_mgeom->AddChild(mgeom);
	}
}

//
// Ray-Sphere intersection
//
bool FindIntersection(const FPoint3 &rkOrigin, const FPoint3 &rkDirection,
					  const FSphere& rkSphere,
					  int& riQuantity, FPoint3 akPoint[2])
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	FPoint3 kDiff = rkOrigin - rkSphere.center;
	double fA = rkDirection.LengthSquared();
	double fB = kDiff.Dot(rkDirection);
	double fC = kDiff.LengthSquared() -
		rkSphere.radius*rkSphere.radius;

	double afT[2];
	double fDiscr = fB*fB - fA*fC;
	if ( fDiscr < 0.0 )
	{
		riQuantity = 0;
	}
	else if ( fDiscr > 0.0 )
	{
		double fRoot = sqrt(fDiscr);
		double fInvA = 1.0/fA;
		afT[0] = (-fB - fRoot)*fInvA;
		afT[1] = (-fB + fRoot)*fInvA;

		if ( afT[0] >= 0.0 )
			riQuantity = 2;
		else if ( afT[1] >= 0.0 )
			riQuantity = 1;
		else
			riQuantity = 0;
	}
	else
	{
		afT[0] = -fB/fA;
		riQuantity = ( afT[0] >= 0.0 ? 1 : 0 );
	}

	for (int i = 0; i < riQuantity; i++)
		akPoint[i] = rkOrigin + rkDirection * afT[i];

	return riQuantity > 0;
}


void geo_to_xyz(double radius, const DPoint2 &geo, FPoint3 &p)
{
	// Convert spherical polar coordinates to cartesian coordinates
	// The angles are given in degrees
	double theta = geo.x / 180.0 * PI;
	double phi = (geo.y / 180.0 * PI);

	phi += PID2;
	theta = PI2 - theta;

	double x = sin(phi) * cos(theta) * radius;
	double y = sin(phi) * sin(theta) * radius;
	double z = cos(phi) * radius;

	p.x = (float) x;
	p.y = (float) -z;
	p.z = (float) y;
}

double radians(double degrees)
{
    return degrees / 180.0 * PI;
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

	lng = PI2 - lng;
	lat = lat - PID2;

	// convert angles to degrees
	geo.x = lng * 180.0 / PI;
	geo.y = lat * 180.0 / PI;

	// keep in expected range
	if (geo.x > 180.0) geo.x -= 360.0;

	// we don't know elevation (yet)
	geo.z = 0.0f;

	int foo = 1;
}

void IcoGlobe::AddTerrainRectangles()
{
	FPoint3 p;

	for (vtTerrain *pTerr = GetTerrainScene().m_pFirstTerrain; pTerr;
			pTerr=pTerr->GetNext())
	{
		// skip if undefined
		if (pTerr->m_Corners_geo.GetSize() == 0)
			continue;

		int numvtx = 4;
		vtMesh *mesh = new vtMesh(GL_LINE_STRIP, 0, numvtx);

		for (int i = 0; i < 5; i++)
		{
			int j = i % 4;
			geo_to_xyz(1.001, pTerr->m_Corners_geo[j], p);
			mesh->AddVertex(p);
		}

		mesh->AddQuadStrip(5, 0);
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
		points.SetAt(i, point);
		SHPDestroyObject(psShape);
	}
	AddPoints(points, 0.003f);
	SHPClose(hSHP);
	return nEntities;
}


