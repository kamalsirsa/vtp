//
// vtTin3d.cpp
//
// Class which represents a Triangulated Irregular Network.
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtTin3d.h"


vtTin3d::vtTin3d()
{
	m_pGeom = NULL;
	m_pDropGeom = NULL;
}

/**
 * Read the TIN from a file.  This can either be an old-style or new-style
 * .tin format (so far, a VTP-specific format)
 */
bool vtTin3d::Read(const char *fname)
{
	if (!vtTin::Read(fname))
		return false;

	ComputeExtents();
	return true;
}

FPoint3 ComputeNormal(const FPoint3 &p1, const FPoint3 &p2, const FPoint3 &p3)
{
	FPoint3 d1 = (p2 - p1);
	FPoint3 d2 = (p3 - p1);
	FPoint3 cross = d1.Cross(d2);
	cross.Normalize();
	return cross;
}


#define MAX_CHUNK_VERTS	30000

vtGeom *vtTin3d::CreateGeometry(bool bDropShadowMesh)
{
	// set up materials
	vtMaterialArray *pMats = new vtMaterialArray();
	bool lighting = false;
	pMats->AddRGBMaterial1(RGBf(1, 1, 1), false, lighting, false);
	pMats->AddRGBMaterial1(RGBf(0.5f, 0.5f, 0.5f), false, false, false);
	pMats->AddRGBMaterial1(RGBf(0, 0, 0), false, false, false);

	m_pGeom = new vtGeom();
	m_pGeom->SetMaterials(pMats);
	pMats->Release();

	int i, j, k;
	int verts = NumVerts();

	// Break it up into a series of meshes - this is good for both
	// culling and memory management

	FPoint3 ep, wp;		// earth point, world point
	FPoint3 p[3], norm;
	FPoint3 light_dir(1, 1, 0);
	RGBf color;
	float r, g=1.0f, b=0.5f;

	// most TINs are larger in the horionztal dimension than the vertical, so
	// use horizontal extents as the basis of subdivision
	DRECT rect = m_EarthExtents;
	double sizex = rect.Width();
	double sizey = rect.Height();
	float height_range = (m_fMaxHeight - m_fMinHeight);

	// make it slightly larger avoid edge condition
	rect.left -= 0.000001;
	sizex += 0.000002;
	rect.bottom -= 0.000001;
	sizey += 0.000002;

	int divx, divy;		// number of x and y divisions
	int dsize;
	Bin *bins;
	int tris = NumTris();
	int bx, by;
	DPoint2 gp;

	int divs = 4;
	bool acceptable = false;
	while (!acceptable)
	{
		// take the smaller dimension and split in to ensure a minimum level
		// of subdivision, with the larger dimension proportional
		if (sizex < sizey)
		{
			divx = divs;
			divy = (int) (divx * sizey / sizex);
		}
		else
		{
			divy = divs;
			divx = (int) (divy * sizex / sizey);
		}

		// create a 2d array of Bins
		dsize = divx*divy;
		bins = new Bin[dsize];

		int most = -1, newsize;
		for (i = 0; i < tris; i++)
		{
			j = i * 3;
			gp = (m_vert[m_tri[j]] + m_vert[m_tri[j+1]] + m_vert[m_tri[j+2]]) / 3;
			bx = (int) (divx * (gp.x - rect.left) / sizex);
			by = (int) (divy * (gp.y - rect.bottom) / sizey);

			Bin &bref = bins[bx * divy + by];
			bref.Append(i);
			newsize = bref.GetSize();
			if (newsize > most)
				most = newsize;
		}
		if (most > 10000)
		{
			delete [] bins;
			divs = divs * 3 / 2;
		}
		else
			acceptable = true;
	}

	int in_bin, tri, vidx;

	for (i = 0; i < dsize; i++)
	{
		Bin &bref = bins[i];
		in_bin = bref.GetSize();
		if (!in_bin)
			continue;

		vtMesh *pMesh = new vtMesh(GL_TRIANGLES, VT_Normals|VT_Colors, in_bin * 3);

		for (j = 0; j < in_bin; j++)
		{
			tri = bref[j];
			int tribase = tri * 3;
			int vertbase = j * 3;

			for (k = 0; k < 3; k++)
			{
				vidx = m_tri[tribase + k];
				ep.Set(m_vert[vidx].x, m_vert[vidx].y, m_z[vidx]);
				m_Conversion.ConvertFromEarth(ep, p[k]);
			}
			norm = ComputeNormal(p[0], p[1], p[2]);

			float shade = norm.Dot(light_dir);	// shading 0 (dark) to 1 (light)

			for (k = 0; k < 3; k++)
			{
				vidx = m_tri[tribase + k];

				r = (m_z[vidx] - m_fMinHeight) / height_range;
				pMesh->AddVertex(p[k]);
				pMesh->SetVtxNormal(vertbase + k, norm);

				color.Set(r, g, b);
				color *= shade;
				pMesh->SetVtxColor(vertbase + k, color);
			}
		}
		m_pGeom->AddMesh(pMesh, 0);
		m_Meshes.Append(pMesh);
	}

	/*
	int base = 0;
	remaining = verts;
	while (remaining)
	{
		int chunk = remaining;
		if (chunk > MAX_CHUNK_VERTS)
			chunk = MAX_CHUNK_VERTS;
		int tris = chunk / 3;

		vtMesh *pMesh = new vtMesh(GL_TRIANGLES, VT_Normals|VT_Colors, chunk);

		for (i = 0; i < tris; i++)
		{
			for (j = 0; j < 3; j++)
				m_Conversion.ConvertFromEarth(m_vert[base + (i*3+j)], p[j]);
			norm = ComputeNormal(p[0], p[1], p[2]);

			float shade = norm.Dot(light_dir);	// shading 0 (dark) to 1 (light)

			for (j = 0; j < 3; j++)
			{
				r = (m_points[i*3+j].z - m_fMinHeight) / (m_fMaxHeight - m_fMinHeight);
				pMesh->AddVertex(p[j]);
				pMesh->SetVtxNormal(i*3+j, norm);

				color.Set(r, g, b);
				color *= shade;
				pMesh->SetVtxColor(i*3+j, color);
			}
		}
		m_pGeom->AddMesh(pMesh, 0);
		m_Meshes.Append(pMesh);

		if (bDropShadowMesh)
		{
			vtMesh *pShadowMesh = new vtMesh(GL_TRIANGLES, 0, chunk);
			for (i = 0; i < chunk; i++)
			{
				ep = m_points[base+i];
				ep.z = m_fMinHeight - 4.9;
				m_Conversion.ConvertFromEarth(ep, wp);
				pShadowMesh->AddVertex(wp);
			}
			m_pGeom->AddMesh(pShadowMesh, 2);
		}

		remaining -= chunk;
		base += chunk;
	}
		*/

	if (bDropShadowMesh)
	{
		vtMesh *pBaseMesh = new vtMesh(GL_TRIANGLE_FAN, 0, 4);

		ep.Set(m_EarthExtents.left - 10, m_EarthExtents.bottom - 10, m_fMinHeight - 5);
		m_Conversion.ConvertFromEarth(ep, wp);
		pBaseMesh->AddVertex(wp);

		ep.Set(m_EarthExtents.right + 10, m_EarthExtents.bottom - 10, m_fMinHeight - 5);
		m_Conversion.ConvertFromEarth(ep, wp);
		pBaseMesh->AddVertex(wp);

		ep.Set(m_EarthExtents.right + 10, m_EarthExtents.top + 10, m_fMinHeight - 5);
		m_Conversion.ConvertFromEarth(ep, wp);
		pBaseMesh->AddVertex(wp);

		ep.Set(m_EarthExtents.left - 10, m_EarthExtents.top + 10, m_fMinHeight - 5);
		m_Conversion.ConvertFromEarth(ep, wp);
		pBaseMesh->AddVertex(wp);

		pBaseMesh->AddFan(0, 1, 2, 3);
		m_pGeom->AddMesh(pBaseMesh, 1);
	}
	return m_pGeom;
}

/**
 * Returns true if the point was over the TIN, false otherwise.
 */
bool vtTin3d::FindAltitudeAtPoint(const FPoint3 &input, float &fAltitude,
								  FPoint3 *vNormal) const
{
	// First try to identify which triangle
	FPoint2 p(input.x, input.z);

	FPoint3 wp1, wp2, wp3;
	FPoint2 p1, p2, p3;
	bool good;

	for (unsigned int m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];
		int tris = mesh->GetNumVertices() / 3;
		for (int i = 0; i < tris; i++)
		{
			// get world points
			wp1 = mesh->GetVtxPos(i*3+0);
			wp2 = mesh->GetVtxPos(i*3+1);
			wp3 = mesh->GetVtxPos(i*3+2);
			// convert to 2d
			p1.Set(wp1.x, wp1.z);
			p2.Set(wp2.x, wp2.z);
			p3.Set(wp3.x, wp3.z);

			if (!PointInTriangle(p, p1, p2, p3))
				continue;

			// compute barycentric coordinates with respect to the triangle
			float bary[3], val;
			good = BarycentricCoords(p1, p2, p3, p, bary);
			if (!good)
				continue;

			// compute barycentric combination of function values at vertices
			val = bary[0]*wp1.y +
				  bary[1]*wp2.y +
				  bary[2]*wp3.y;
			fAltitude = val;
			return true;
		}
	}
	return false;
}


/*
 * Algorithm from 'Fast, Minimum Storage Ray-Triangle Intersection',
 * Thomas Möller and Ben Trumbore, 1997
 * Adapted to use C++ and the vtdata math classes.
 */
#define EPSILON 0.000001
bool intersect_triangle(const FPoint3 &orig, const FPoint3 &dir,
			const FPoint3 &vert0, const FPoint3 &vert1, const FPoint3 &vert2,
			float &t, float &u, float &v)
{
	FPoint3 edge1, edge2, tvec, pvec, qvec;
	float det,inv_det;

	/* find vectors for two edges sharing vert0 */
	edge1 = vert1 - vert0;
	edge2 = vert2 - vert0;

	/* begin calculating determinant - also used to calculate U parameter */
	pvec = dir.Cross(edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1.Dot(pvec);

	if (det < EPSILON)
		return false;

	/* calculate distance from vert0 to ray origin */
	tvec = orig - vert0;

	/* calculate U parameter and test bounds */
	u = tvec.Dot(pvec);
	if (u < 0.0 || u > det)
		return false;

	/* prepare to test V parameter */
	qvec = tvec.Cross(edge1);

	 /* calculate V parameter and test bounds */
	v = dir.Dot(qvec);
	if (v < 0.0 || u + v > det)
		return false;

	/* calculate t, scale parameters, ray intersects triangle */
	t = edge2.Dot(qvec);
	inv_det = 1.0 / det;
	t *= inv_det;
	u *= inv_det;
	v *= inv_det;

	return true;
}

bool vtTin3d::CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
							   FPoint3 &result) const
{
	FPoint3 wp1, wp2, wp3;
	float t, u, v, closest = 1E9;
	int i;

	for (unsigned int m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];
		int tris = mesh->GetNumVertices() / 3;
		for (i = 0; i < tris; i++)
		{
			// get world points
			wp1 = mesh->GetVtxPos(i*3+0);
			wp2 = mesh->GetVtxPos(i*3+1);
			wp3 = mesh->GetVtxPos(i*3+2);
			if (intersect_triangle(point, dir, wp1, wp2, wp3, t, u, v))
			{
				if (t < closest)
					closest = t;
			}
		}
	}
	if (closest == 1E9)
		return false;

	result = point + (dir * closest);
	return true;
}

FPoint3 vtTin3d::FindVectorToClosestVertex(const FPoint3 &pos)
{
	FPoint3 vert, diff, closest_diff;
	float len, minlen = 1E9;

	for (unsigned int m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];
		int points = mesh->GetNumVertices();
		for (int i = 0; i < points; i++)
		{
			vert = mesh->GetVtxPos(i);
			diff = vert - pos;
			len = diff.Length();
			if (len < minlen)
			{
				minlen = len;
				closest_diff = diff;
			}
		}
	}
	return closest_diff;
}

