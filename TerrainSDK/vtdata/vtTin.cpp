//
// vtTin.cpp
//
// Class which represents a Triangulated Irregular Network.
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtTin.h"
#include "vtLog.h"
#include "DxfParser.h"


void vtTin::AddVert(const DPoint2 &p, float z)
{
	m_vert.Append(p);
	m_z.Append(z);
}

void vtTin::AddTri(int i1, int i2, int i3)
{
	m_tri.Append(i1);
	m_tri.Append(i2);
	m_tri.Append(i3);
}

bool vtTin::_ReadTinOld(FILE *fp)
{
	int i, num;
	FPoint3 f;
	DPoint2 p;

	fread(&num, 1, sizeof(int), fp);
	m_vert.SetMaxSize(num);
	for (i = 0; i < num; i++)
	{
		fread(&f.x, 3, sizeof(float), fp);

		p.Set(f.x, f.y);
		AddVert(p, f.z);
	}
	for (i = 0; i < num/3; i++)
	{
		AddTri(i*3, i*3+1, i*3+2);
	}
	return true;
}

bool vtTin::_ReadTin(FILE *fp)
{
	int i, verts, tris, data_start, proj_len;

	char marker[5];
	fread(marker, 5, 1, fp);
	fread(&verts, 4, 1, fp);
	fread(&tris, 4, 1, fp);
	fread(&data_start, 4, 1, fp);
	fread(&proj_len, 4, 1, fp);
	if (proj_len > 2000)
		return false;

	char wkt_buf[2000], *wkt = wkt_buf;
	fread(wkt, proj_len, 1, fp);
	wkt_buf[proj_len] = 0;

	OGRErr err = m_proj.importFromWkt((char **) &wkt);
	if (err != OGRERR_NONE)
		return false;

	fseek(fp, data_start, SEEK_SET);

	// read verts
	DPoint2 p;
	float z;
	for (i = 0; i < verts; i++)
	{
		fread(&p.x, 8, 2, fp);	// 2 doubles
		fread(&z, 4, 1, fp);	// 1 float
		AddVert(p, z);
	}
	// read tris
	int tribuf[3];
	for (i = 0; i < tris; i++)
	{
		fread(tribuf, 4, 3, fp);	// 3 ints
		AddTri(tribuf[0], tribuf[1], tribuf[2]);
	}
	return true;
}

/**
 * Read the TIN from a file.  This can either be an old-style or new-style
 * .tin format (custom VTP format)
 */
bool vtTin::Read(const char *fname)
{
	// first read the point from the .tin file
	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return false;

	char buf[3];
	fread(buf, 3, 1, fp);
	fseek(fp, 0, SEEK_SET);

	bool success = _ReadTin(fp);
	if (!success)
		return false;

	fclose(fp);

	ComputeExtents();
	return true;
}

/**
 * Attempt to read TIN data from a DXF file.
 */
bool vtTin::ReadDXF(const char *fname, bool progress_callback(int))
{
	VTLOG("vtTin::ReadDXF():\n");

	std::vector<DxfEntity> entities;
	std::vector<vtString> layers;

	DxfParser parser(fname, entities, layers);
	bool bSuccess = parser.RetrieveEntities(progress_callback);
	if (!bSuccess)
	{
		VTLOG(parser.GetLastError());
		return false;
	}

	int vtx = 0;
	int faces = 0;
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		const DxfEntity &ent = entities[i];
		if (ent.m_iType == DET_3DFace)
		{
			int NumVerts = ent.m_points.size();
			if (NumVerts == 3)
			{
				for (int j = 0; j < 3; j++)
				{
					DPoint2 p(ent.m_points[j].x, ent.m_points[j].y);
					float z = (float) ent.m_points[j].z;

					AddVert(p, z);
				}
				AddTri(vtx, vtx+1, vtx+2);
				vtx += 3;
				faces ++;
			}
		}
	}
	VTLOG("  Found %d entities of type 3DFace.\n", faces);

	// If we didn't find any surfaces, we haven't got a TIN
	if (faces == 0)
		return false;

	// Test each triangle for clockwisdom, fix if needed
	CleanupClockwisdom();

	ComputeExtents();
	return true;
}


/**
 * Write the TIN to a new-style .tin file (custom VTP format).
 */
bool vtTin::Write(const char *fname) const
{
	FILE *fp = fopen(fname, "wb");
	if (!fp)
		return false;

	char *wkt;
	OGRErr err = m_proj.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
	{
		fclose(fp);
		return false;
	}
	int proj_len = strlen(wkt);
	int data_start = 5 + 4 + 4 + 4 + + 4 + proj_len;

	int i;
	int verts = NumVerts();
	int tris = NumTris();

	fwrite("tin01", 5, 1, fp);
	fwrite(&verts, 4, 1, fp);
	fwrite(&tris, 4, 1, fp);
	fwrite(&data_start, 4, 1, fp);
	fwrite(&proj_len, 4, 1, fp);
	fwrite(wkt, proj_len, 1, fp);
	OGRFree(wkt);
	// room for future extention: you can add fields here, as long as you
	// increase the data_start offset above accordingly

	// write verts
	for (i = 0; i < verts; i++)
	{
		fwrite(&m_vert[i].x, 8, 2, fp);	// 2 doubles
		fwrite(&m_z[i], 4, 1, fp);		// 1 float
	}
	// write tris
	for (i = 0; i < tris*3; i++)
	{
		fwrite(&m_tri[i], 4, 1, fp);	// 1 int
	}

	fclose(fp);
	return true;
}

bool vtTin::ComputeExtents()
{
	int size = NumVerts();
	if (size == 0)
		return false;

	m_EarthExtents.SetRect(1E9, -1E9, -1E9, 1E9);
	m_fMinHeight = 1E9;
	m_fMaxHeight = -1E9;

	for (int j = 0; j < size; j++)
	{
		m_EarthExtents.GrowToContainPoint(m_vert[j]);

		float z = m_z[j];
		if (z > m_fMaxHeight) m_fMaxHeight = z;
		if (z < m_fMinHeight) m_fMinHeight = z;
	}
	return true;
}

void vtTin::Offset(const DPoint2 &p)
{
	int size = m_vert.GetSize();
	for (int j = 0; j < size; j++)
	{
		m_vert[j] += p;
	}
}

bool vtTin::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue) const
{
	DPoint2 p1, p2, p3;		// 2D points
	bool good;

	int v0, v1, v2;
	int tris = NumTris();
	for (int i = 0; i < tris; i++)
	{
		v0 = m_tri[i*3];
		v1 = m_tri[i*3+1];
		v2 = m_tri[i*3+2];
		// get points
		p1 = m_vert.GetAt(v0);
		p2 = m_vert.GetAt(v1);
		p3 = m_vert.GetAt(v2);

		// First try to identify which triangle
		if (!PointInTriangle(p, p1, p2, p3))
			continue;

		// compute barycentric coordinates with respect to the triangle
		double bary[3], val;
		good = BarycentricCoords(p1, p2, p3, p, bary);
		if (!good)
			continue;

		// compute barycentric combination of function values at vertices
		val = bary[0] * m_z[v0] +
			bary[1] * m_z[v1] +
			bary[2] * m_z[v2];
		fAltitude = (float) val;
		return true;
	}
	return false;
}


bool vtTin::ConvertProjection(const vtProjection &proj_new)
{
	// Create conversion object
	OCT *trans = CreateCoordTransform(&m_proj, &proj_new);
	if (!trans)
		return false;		// inconvertible projections

	int size = NumVerts();
	for (int i = 0; i < size; i++)
	{
		DPoint2 &p = m_vert[i];
		trans->Transform(1, &p.x, &p.y);
	}
	delete trans;

	// adopt new projection
	m_proj = proj_new;

	return true;
}


/**
 * Test each triangle for clockwisdom, fix if needed.  The result should
 *  be a TIN with consistent vertex ordering, such that all face normals
 *  point up rather than down, that is, counter-clockwise.
 */
void vtTin::CleanupClockwisdom()
{
	DPoint2 p1, p2, p3;		// 2D points
	int v0, v1, v2;
	unsigned int tris = NumTris();
	for (unsigned int i = 0; i < tris; i++)
	{
		v0 = m_tri[i*3];
		v1 = m_tri[i*3+1];
		v2 = m_tri[i*3+2];
		// get 2D points
		p1 = m_vert.GetAt(v0);
		p2 = m_vert.GetAt(v1);
		p3 = m_vert.GetAt(v2);

		// The so-called 2D cross product
		double cross2d = (p2-p1).Cross(p3-p1);
		if (cross2d < 0)
		{
			// flip
			m_tri[i*3+1] = v2;
			m_tri[i*3+2] = v1;
		}
	}
}

/**
 * Return the length of the longest edge of a specific triangle.
 */
double vtTin::GetTriMaxEdgeLength(int iTri) const
{
	int tris = NumTris();
	if (iTri < 0 || iTri >= tris)
		return 0.0;

	// get points
	int v0 = m_tri[iTri*3];
	int v1 = m_tri[iTri*3+1];
	int v2 = m_tri[iTri*3+2];
	DPoint2 p1 = m_vert.GetAt(v0);
	DPoint2 p2 = m_vert.GetAt(v1);
	DPoint2 p3 = m_vert.GetAt(v2);

	// check lengths
	double len1 = (p2 - p1).Length();
	double len2 = (p3 - p2).Length();
	double len3 = (p1 - p3).Length();
	return len1 > len2 ?
		(len1 > len3 ? len1 : len3) :
	(len2 > len3 ? len2 : len3);
}

// Number of bins used by the merge algorithm.  Time is roughly proportional
// to N*N/BINS, where N is the number of vertices, so increase BINS for speed.
//
#define BINS	4000

/**
 * Combine all vertices which are at the same location.  By removing these
 * redundant vertices, the mesh will consume less space in memory and on disk.
 */
void vtTin::MergeSharedVerts(bool progress_callback(int))
{
	unsigned int verts = NumVerts();

	unsigned int i, j;
	int bin;

	DRECT rect = m_EarthExtents;
	double width = rect.Width();

	// make it slightly larger avoid edge condition
	rect.left -= 0.000001;
	width += 0.000002;

	m_bReplace = new int[verts];
	m_vertbin = new Bin[BINS];
	m_tribin = new Bin[BINS];

	// sort the vertices into bins
	for (i = 0; i < verts; i++)
	{
		// flag all vertices initially not to remove
		m_bReplace[i] = -1;

		// find the correct bin, and add the index of this vertex to it
		bin = (int) (BINS * (m_vert[i].x - rect.left) / width);
		m_vertbin[bin].Append(i);
	}
	unsigned int trisize = m_tri.GetSize();
	for (i = 0; i < trisize; i++)
	{
		// find the correct bin, and add the index of this index to it
		bin = (int) (BINS * (m_vert[m_tri[i]].x - rect.left) / width);
		m_tribin[bin].Append(i);
	}

	// compare within each bin, and between each adjacent bin,
	// looking for matching vertices to flag for removal
	for (bin = 0; bin < BINS; bin++)
	{
		if (progress_callback != NULL)
			progress_callback(bin * 100 / BINS);

		_CompareBins(bin, bin);
		if (bin < BINS-1)
			_CompareBins(bin, bin+1);
	}
	// now update each triangle index to point to the merge result
	for (bin = 0; bin < BINS; bin++)
	{
		if (progress_callback != NULL)
			progress_callback(bin * 100 / BINS);

		_UpdateIndicesInInBin(bin);
	}

	// now compact the vertex bins into a single array

	// make a copy to copy from
	DLine2 *vertcopy = new DLine2(m_vert);
	float *zcopy = new float[m_z.GetSize()];
	for (i = 0; i < m_z.GetSize(); i++)
		zcopy[i] = m_z[i];

	int inew = 0;	// index into brand new array (actually re-using old)

	for (bin = 0; bin < BINS; bin++)
	{
		if (progress_callback != NULL)
			progress_callback(bin * 100 / BINS);

		unsigned int binverts = m_vertbin[bin].GetSize();
		for (i = 0; i < binverts; i++)
		{
			int v_old = m_vertbin[bin].GetAt(i);
			if (m_bReplace[v_old] != -1)
				continue;

			int v_new = inew;

			// copy old to new
			m_vert[v_new] = vertcopy->GetAt(v_old);
			m_z[v_new] = zcopy[v_old];

			unsigned int bintris = m_tribin[bin].GetSize();
			for (j = 0; j < bintris; j++)
			{
				int trindx = m_tribin[bin].GetAt(j);
				if (m_tri[trindx] == v_old)
					m_tri[trindx] = v_new;
			}
			inew++;
		}
	}

	// our original array containers now hold the compacted result
	int newsize = inew;
	m_vert.SetSize(newsize);
	m_z.SetSize(newsize);

	// free up all the crud we allocated along the way
	delete [] m_bReplace;
	delete [] m_vertbin;
	delete [] m_tribin;
	delete vertcopy;
	delete [] zcopy;
}

void vtTin::_UpdateIndicesInInBin(int bin)
{
	int i, j;

	int binverts = m_vertbin[bin].GetSize();
	for (i = 0; i < binverts; i++)
	{
		int v_before = m_vertbin[bin].GetAt(i);
		int v_after = m_bReplace[v_before];

		if (v_after == -1)
			continue;

		int bintris = m_tribin[bin].GetSize();
		for (j = 0; j < bintris; j++)
		{
			int trindx = m_tribin[bin].GetAt(j);
			if (m_tri[trindx] == v_before)
				m_tri[trindx] = v_after;
		}
	}
}

void vtTin::_CompareBins(int bin1, int bin2)
{
	int i, j;
	int ix1, ix2;
	int start;

	int size1 = m_vertbin[bin1].GetSize();
	int size2 = m_vertbin[bin2].GetSize();
	for (i = 0; i < size1; i++)
	{
		ix1 = m_vertbin[bin1].GetAt(i);

		// within a bin, we can do N*N/2 instead of N*N compares
		// i.e. size1*size1/2, instead of size1*size2
		if (bin1 == bin2)
			start = i+1;
		else
			start = 0;

		for (j = start; j < size2; j++)
		{
			ix2 = m_vertbin[bin2].GetAt(j);

			// don't compare against itself
			if (ix1 == ix2)
				continue;

			if (m_vert[ix1] == m_vert[ix2])
			{
				// ensure that one of them is flagged
				if (m_bReplace[ix1] == -1)
				{
					if (m_bReplace[ix2] == -1)
						m_bReplace[ix1] = ix2;
					else if (m_bReplace[ix2] != ix1)
						m_bReplace[ix1] = m_bReplace[ix2];
				}
				if (m_bReplace[ix2] == -1)
				{
					if (m_bReplace[ix1] == -1)
						m_bReplace[ix2] = ix1;
					else if (m_bReplace[ix1] != ix2)
						m_bReplace[ix2] = m_bReplace[ix1];
				}
			}
		}
	}
}

