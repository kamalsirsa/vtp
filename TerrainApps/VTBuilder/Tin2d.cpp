//
// Tin2d.cpp
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Tin2d.h"
#include "Options.h"

#include "vtdata/ElevationGrid.h"
#include "vtdata/Features.h"
#include "vtdata/Triangulate.h"

////////////////////////////////////////////////////////////////////

vtTin2d::vtTin2d()
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;
}

vtTin2d::~vtTin2d()
{
	FreeEdgeLengths();
}

vtTin2d::vtTin2d(vtElevationGrid *grid)
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;

	int cols, rows;
	grid->GetDimensions(cols, rows);
	m_proj = grid->GetProjection();

	DPoint2 p;
	for (int x = 0; x < cols; x++)
		for (int y = 0; y < rows; y++)
		{
			grid->GetEarthPoint(x, y, p);
			AddVert(p, grid->GetFValue(x, y));
		}
	for (int x = 0; x < cols-1; x++)
		for (int y = 0; y < rows-1; y++)
		{
			int base = x * rows + y;
			AddTri(base, base + rows, base+1);
			AddTri(base+1, base + rows, base + rows+1);
		}
	ComputeExtents();
}

#define ANSI_DECLARATORS
#define REAL double
extern "C" {
#include "vtdata/triangle/triangle.h"
}

/**
 * Create a TIN from a set of 3D points, using the Triangle library to derive
 *  the set of triangles from the points.
 */
vtTin2d::vtTin2d(vtFeatureSetPoint3D *set)
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;

	struct triangulateio in, out;
	int i;
	DPoint3 p3;

	// point list
	in.numberofpoints = set->GetNumEntities();
	in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
	in.numberofpointattributes = 1;
	in.pointattributelist = (REAL *) malloc(in.numberofpoints * sizeof(REAL));

	for ( i = 0; i < in.numberofpoints; ++i )
	{
		set->GetPoint(i, p3);
		in.pointlist[2*i] = p3.x;
		in.pointlist[2*i + 1] = p3.y;

		in.pointattributelist[i] = p3.z;
	}

	in.pointmarkerlist = (int *) NULL;
	in.numberoftriangles = 0;

	// no segment list
	in.numberofsegments = 0;
	in.segmentlist = (int *) NULL;
	in.segmentmarkerlist = (int *) NULL;

	// no holes or regions
	in.numberofholes = 0;
	in.holelist = (REAL *) NULL;
	in.numberofregions = 0;
	in.regionlist = (REAL *) NULL;

	// prep the output structures
	out.pointlist = (REAL *) NULL;        // Not needed if -N switch used.
	out.pointattributelist = (REAL *) NULL;
	out.pointmarkerlist = (int *) NULL;   // Not needed if -N or -B switch used.
	out.trianglelist = (int *) NULL;      // Not needed if -E switch used.
	out.triangleattributelist = (REAL *) NULL;
	out.neighborlist = (int *) NULL;      // Needed only if -n switch used.
	out.segmentlist = (int *) NULL;
	out.segmentmarkerlist = (int *) NULL;
	out.edgelist = (int *) NULL;          // Needed only if -e switch used.
	out.edgemarkerlist = (int *) NULL;    // Needed if -e used and -B not used.

	// Triangulate the points.  Switches are chosen:
	// number everything from zero (z),
	triangulate("z", &in, &out, NULL);

	// now copy the triangle results back into vtdata structures
	for ( i = 0; i < out.numberofpoints; ++i )
	{
		float z = out.pointattributelist[i];
		AddVert(DPoint2(out.pointlist[2*i], out.pointlist[2*i + 1]), z);
	}
	for ( i = 0; i < out.numberoftriangles; ++i )
	{
		int n1 = out.trianglelist[i * 3];
		int n2 = out.trianglelist[i * 3 + 1];
		int n3 = out.trianglelist[i * 3 + 2];
		AddTri(n1, n2, n3);
	}
	// free mem allocated to the "Triangle" structures
	free(in.pointlist);
	free(in.pointattributelist);

	free(out.pointlist);
	free(out.pointattributelist);
	free(out.pointmarkerlist);
	free(out.trianglelist);
	free(out.triangleattributelist);
	free(out.neighborlist);
	free(out.segmentlist);
	free(out.segmentmarkerlist);
	free(out.edgelist);
	free(out.edgemarkerlist);

	ComputeExtents();
	// Adopt CRS from the featureset
	m_proj = set->GetAtProjection();
}

vtTin2d::vtTin2d(vtFeatureSetPolygon *set, int iFieldNum)
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;

	int num = set->GetNumEntities();
	for (int i = 0; i < num; ++i)
	{
		DPolygon2 &dpoly = set->GetPolygon(i);

		// Get z value from field 0
		float z = set->GetFloatValue(i, iFieldNum);

		DLine2 result;
		CallTriangle(dpoly, result);
//		CallPoly2Tri(dpoly, result);	// TEST
		int res = result.GetSize();
		int base = NumVerts();
		for (int j = 0; j < res; j++)
		{
			AddVert(result[j], z);
		}
		for (int j = 0; j < res/3; j++)
		{
			AddTri(base + j*3, base + j*3+1, base + j*3+2);
		}
	}
	ComputeExtents();
	// Adopt CRS from the featureset
	m_proj = set->GetAtProjection();
}

void vtTin2d::MakeOutline()
{
	// Find all the unique edges (all internal edges appear twice)
	for (uint i = 0; i < NumTris(); i++)
	{
		int v0 = m_tri[i*3+0];
		int v1 = m_tri[i*3+1];
		int v2 = m_tri[i*3+2];
		if (v0 < v1)
			m_edges.AddUniqueEdge(IntPair(v0, v1));
		else
			m_edges.AddUniqueEdge(IntPair(v1, v0));
		if (v1 < v2)
			m_edges.AddUniqueEdge(IntPair(v1, v2));
		else
			m_edges.AddUniqueEdge(IntPair(v2, v1));
		if (v2 < v0)
			m_edges.AddUniqueEdge(IntPair(v2, v0));
		else
			m_edges.AddUniqueEdge(IntPair(v0, v2));
	}
}

int vtTin2d::GetMemoryUsed() const
{
	int bytes = 0;

	bytes += sizeof(vtTin2d);
	bytes += sizeof(DPoint2) * m_vert.GetSize();
	bytes += sizeof(int) * m_tri.GetSize();
	bytes += sizeof(float) * m_z.GetSize();

	if (m_fEdgeLen)
		bytes += sizeof(double) * NumTris();

	if (m_trianglebins)
		bytes += m_trianglebins->GetMemoryUsed();

	return bytes;
}

void vtTin2d::DrawTin(wxDC *pDC, vtScaledView *pView)
{
	// Dark purple lines
	wxPen TinPen(wxColor(128,0,128), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(TinPen);

	bool bDrawSimple = g_Options.GetValueBool(TAG_DRAW_TIN_SIMPLE);
	if (bDrawSimple)
	{
		if (!m_edges.size())
		{
			// extract an outline
			MakeOutline();
		}
		// Just draw the online
		for (Outline::iterator it = m_edges.begin(); it != m_edges.end(); it++)
		{
			pView->screen(m_vert[it->v0], g_screenbuf[0]);
			pView->screen(m_vert[it->v1], g_screenbuf[1]);
			pDC->DrawLines(2, g_screenbuf);
		}
	}
	else
	{
		// Draw every triangle
		FPoint2 p2;
		uint tris = NumTris();
		for (uint i = 0; i < tris; i++)
		{
			if (m_bConstrain)
			{
				if (m_fEdgeLen[i] > m_fMaxEdge)
					continue;
			}
			int v0 = m_tri[i*3+0];
			int v1 = m_tri[i*3+1];
			int v2 = m_tri[i*3+2];

			pView->screen(m_vert[v0], g_screenbuf[0]);
			pView->screen(m_vert[v1], g_screenbuf[1]);
			pView->screen(m_vert[v2], g_screenbuf[2]);

			g_screenbuf[3] = g_screenbuf[0];
			pDC->DrawLines(4, g_screenbuf);
		}
	}
#if 0
	// For testing purposes, draw the vertices as well
	uint points = NumVerts();
	for (i = 0; i < tris; i++)
	{
		pView->screen(m_vert[i], g_screenbuf[0]);
		pDC->DrawPoint(g_screenbuf[0]);
	}
#endif
}

void vtTin2d::SetConstraint(bool bConstrain, double fMaxEdge)
{
	m_bConstrain = bConstrain;
	m_fMaxEdge = fMaxEdge;
}

void vtTin2d::ComputeEdgeLengths()
{
	int nTris = NumTris();
	m_fEdgeLen = new double[nTris];
	for (int i = 0; i < nTris; i++)
		m_fEdgeLen[i] = GetTriMaxEdgeLength(i);
}

void vtTin2d::CullLongEdgeTris()
{
	if (!m_fEdgeLen)
		return;
	int nTris = NumTris();
	int b1, b2;
	int to = 0;
	int kept = 0;
	for (int i = 0; i < nTris; i++)
	{
		b1 = i * 3;
		if (m_fEdgeLen[i] < m_fMaxEdge)
		{
			// keep
			b2 = to * 3;
			m_tri[b2] = m_tri[b1];
			m_tri[b2+1] = m_tri[b1+1];
			m_tri[b2+2] = m_tri[b1+2];
			to++;
			kept++;
		}
	}
	m_tri.SetSize(kept*3);
}

void vtTin2d::FreeEdgeLengths()
{
	delete m_fEdgeLen;
	m_fEdgeLen = NULL;
}


