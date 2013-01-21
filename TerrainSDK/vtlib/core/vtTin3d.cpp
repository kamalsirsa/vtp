//
// vtTin3d.cpp
//
// Class which represents a Triangulated Irregular Network.
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"	// for FindFileOnPaths
#include "vtdata/DataPath.h"
#include "vtTin3d.h"

// We will split the TIN into chunks of geometry, each with no more than this many vertices.
const int kMaxChunkVertices = 10000;
const int kColorMapTableSize = 8192;

vtTin3d::vtTin3d()
{
	m_pMats = NULL;
	m_pGeode = NULL;
	m_pDropGeode = NULL;
	m_pColorMap = NULL;
}

/**
 * Read the TIN from a file.  This can either be an old-style or new-style
 * .tin format (so far, a VTP-specific format)
 */
bool vtTin3d::Read(const char *fname)
{
	if (!vtTin::Read(fname))
		return false;

	Initialize(m_proj.GetUnits(), m_EarthExtents, m_fMinHeight, m_fMaxHeight);
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

void vtTin3d::SetMaterial(vtMaterialArray *pMats, int mat_idx)
{
	m_pMats = pMats;
	m_MatIndex = mat_idx;
}

void vtTin3d::MakeSurfaceMaterials()
{
	uint iSurfTypes = m_surftypes.size();
	bool bExplicitNormals = HasVertexNormals();

	for (uint i = 0; i < iSurfTypes; i++)
	{
		bool bLighting = bExplicitNormals;
		float fAmbient = 0.3f;

		// Might be absolute path
		vtString path = FindFileOnPaths(vtGetDataPath(), m_surftypes[i]);

		// Might be relative path
		if (path == "")
		{
			vtString relpath = "GeoTypical/";
			relpath += m_surftypes[i];
			path = FindFileOnPaths(vtGetDataPath(), relpath);
		}

		m_pMats->AddTextureMaterial(path, false, bLighting, false, false,
			fAmbient, 1.0f, 1.0f, 0.0f, false, true);	// No clamp, yes mipmap.
	}
}

vtGeode *vtTin3d::CreateGeometry(bool bDropShadowMesh, int m_matidx)
{
	bool bGeoSpecific = (m_pMats != NULL);

	uint iSurfTypes = m_surftypes.size();
	bool bUseSurfaceTypes = (m_surfidx.size() > 0 && iSurfTypes > 0);
	bool bTextured = bGeoSpecific || bUseSurfaceTypes;
	bool bExplicitNormals = HasVertexNormals();

	// The first 3 materials are hard-coded, the rest are per surface type
	int texture_base = 3;

	if (!bGeoSpecific)
	{
		// set up geotypical materials
		m_pMats = new vtMaterialArray;
		bool lighting = false;

		// 0: White: used for vertex-colored terrain surface
		m_pMats->AddRGBMaterial(RGBf(1, 1, 1), false, lighting, false);

		// 1: Grey: used for drop shadow plane
		m_pMats->AddRGBMaterial(RGBf(0.4f, 0.4f, 0.4f), false, false, false);

		// 2: Black
		m_pMats->AddRGBMaterial(RGBf(0, 0, 0), false, false, false);

		if (bUseSurfaceTypes)
			MakeSurfaceMaterials();

		// Rather than look through the color map for each pixel, pre-build
		//  a color lookup table once - should be faster in nearly all cases.
		float fMin, fMax;
		GetHeightExtents(fMin, fMax);
		m_pColorMap->GenerateColorTable(kColorMapTableSize, fMin, fMax);
	}

	m_pGeode = new vtGeode;
	m_pGeode->SetMaterials(m_pMats);

	// Break it up into a series of meshes - this is good for both
	// culling and memory management

	DPoint3 ep;		// earth point
	FPoint3 wp;		// world point
	FPoint3 p[3], norm;

	// most TINs are larger in the horizontal dimension than the vertical, so
	// use horizontal extents as the basis of subdivision
	DRECT rect = m_EarthExtents;
	double sizex = rect.Width();
	double sizey = rect.Height();

	// make it slightly larger avoid edge condition
	rect.left -= 0.000001;
	sizex += 0.000002;
	rect.bottom -= 0.000001;
	sizey += 0.000002;

	int divx, divy;		// number of x and y divisions
	uint dsize=0;
	Bin *bins = NULL;
	uint tris = NumTris();
	int bx, by;
	DPoint2 gp;
	vtArray<vtMesh *> pTypeMeshes;

	uint i, j, k;
	int divs = 4;
	bool acceptable = false;
	while (!acceptable)
	{
		// take the smaller dimension and split it to ensure a minimum level
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
			bref.push_back(i);
			newsize = bref.size();
			if (newsize > most)
				most = newsize;
		}
		if (most > kMaxChunkVertices)
		{
			delete [] bins;
			divs = divs * 3 / 2;
		}
		else
			acceptable = true;
	}

	FPoint3 light_dir(0.5, 1, 0);
	light_dir.Normalize();

	uint in_bin;
	int tri, vidx;

	for (i = 0; i < dsize; i++)
	{
		Bin &bref = bins[i];
		in_bin = bref.size();
		if (!in_bin)
			continue;

		int vert_type;
		if (bTextured)
		{
			if (bExplicitNormals)
				vert_type = VT_Normals|VT_TexCoords;
			else
				vert_type = VT_TexCoords;
		}
		else
			vert_type = VT_Normals|VT_Colors;

		vtMesh *pMesh = NULL;
		if (bUseSurfaceTypes)
		{
			pTypeMeshes.SetSize(iSurfTypes);
			for (j = 0; j < iSurfTypes; j++)
				pTypeMeshes[j] = NULL;
		}
		else
		{
			// simple case: this whole bin goes into one mesh
			pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLES, vert_type, in_bin * 3);
		}

		for (j = 0; j < in_bin; j++)
		{
			tri = bref[j];
			int tribase = tri * 3;

			for (k = 0; k < 3; k++)
			{
				vidx = m_tri[tribase + k];
				ep.Set(m_vert[vidx].x, m_vert[vidx].y, m_z[vidx]);
				m_Conversion.ConvertFromEarth(ep, p[k]);
			}
			norm = ComputeNormal(p[0], p[1], p[2]);

			float shade = norm.Dot(light_dir);	// shading 0 (dark) to 1 (light)
			if (shade < 0)
				shade = -shade;

			bool bTiled = true;
			if (bUseSurfaceTypes)
			{
				// We mush pick a mesh based on surface type
				int surftype = m_surfidx[tri];
				if (pTypeMeshes[surftype] == NULL)
					pTypeMeshes[surftype] = new vtMesh(osg::PrimitiveSet::TRIANGLES,
						vert_type, in_bin * 3);
				pMesh = pTypeMeshes[surftype];
				bTiled = m_surftype_tiled[surftype];
			}

			int vert_base = pMesh->NumVertices();
			for (k = 0; k < 3; k++)
			{
				vidx = m_tri[tribase + k];

				// This is where we actually add the vertex
				int vert_index = pMesh->AddVertex(p[k]);
				if (bTextured)
				{
					FPoint2 uv;
					if (bGeoSpecific || !bTiled)
						uv.Set((m_vert[vidx].x - m_EarthExtents.left) / sizex,
							   (m_vert[vidx].y - m_EarthExtents.bottom) / sizey);
					else
						uv.Set((m_vert[vidx].x - m_EarthExtents.left) / 6,
							   (m_vert[vidx].y - m_EarthExtents.bottom) / 6);
					pMesh->SetVtxTexCoord(vert_index, uv);

					if (bExplicitNormals)
						pMesh->SetVtxNormal(vert_index, m_vert_normal[vidx]);
				}
				else
				{
					pMesh->SetVtxNormal(vert_index, norm);

					// Color by elevation.
					const RGBi &rgb = m_pColorMap->ColorFromTable(m_z[vidx]);
					RGBf color(rgb);
					color *= shade;
					pMesh->SetVtxColor(vert_index, color);
				}
			}
			pMesh->AddTri(vert_base, vert_base+1, vert_base+2);
		}
		if (bUseSurfaceTypes)
		{
			for (j = 0; j < iSurfTypes; j++)
			{
				if (pTypeMeshes[j] != NULL)
				{
					m_pGeode->AddMesh(pTypeMeshes[j], texture_base + j);
					m_Meshes.Append(pTypeMeshes[j]);
				}
			}
		}
		else
		{
			// Simple case
			m_pGeode->AddMesh(pMesh, m_matidx);
			m_Meshes.Append(pMesh);
		}
	}

	// Free up temp arrays
	delete [] bins;

	if (bDropShadowMesh)
	{
		vtMesh *pBaseMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, 0, 4);

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
		m_pGeode->AddMesh(pBaseMesh, 1);
	}

	// The TIN is a large geometry which should not attempt to cast a shadow,
	//  because shadow algos tend to support only small regions of casters.
	m_pGeode->SetCastShadow(false);

	return m_pGeode;
}

/**
 * Returns true if the point was over the TIN, false otherwise.
 */
bool vtTin3d::FindAltitudeAtPoint(const FPoint3 &input, float &fAltitude,
								  bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
{
	// Look on culture first
	if (iCultureFlags != 0 && m_pCulture != NULL)
	{
		if (m_pCulture->FindAltitudeOnCulture(input, fAltitude, bTrue, iCultureFlags))
			return true;
	}
	return vtTin::FindAltitudeAtPoint(input, fAltitude, bTrue, iCultureFlags, vNormal);

#if 0
	// We could test against the 3D triangles with the code below, but in
	//  practice, i found it is actually faster to just use the parent class
	//  (vtTin) to test against the 2D triangles.

	FPoint2 p(input.x, input.z);

	FPoint3 wp1, wp2, wp3;
	FPoint2 p1, p2, p3;
	bool good;

	for (uint m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];
		int tris = mesh->NumVertices() / 3;
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
	fAltitude = 0;
	return false;
#endif
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
	inv_det = 1.0f / det;
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

	for (uint m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];

		// Try to quickly avoid some meshes by testing against their bounding
		// spheres.
		FBox3 box;
		mesh->GetBoundBox(box);
		FSphere sph(box);

		int iQuantity;
		FPoint3 interpoints[2];
		if (!RaySphereIntersection(point, dir, sph, iQuantity, interpoints))
		{
			continue;
		}

		int tris = mesh->NumVertices() / 3;
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

	for (uint m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];
		int points = mesh->NumVertices();
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

