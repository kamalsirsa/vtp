//
// TerrainPatch.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TerrainPatch.h"

#define OSC_MAG		2.0f	// oscillation magnitude


TerrainPatch::TerrainPatch(int VtxType, int NumVertices) :
	 vtMesh(GL_TRIANGLE_STRIP, VtxType, NumVertices)
//	 vtMesh(GL_LINE_STRIP, VtxType, NumVertices)
{
	// nothing more to do here
}

//
// Constructor for TerrainPatch
//
// x and z are number of quad patches
//
bool TerrainPatch::MakeGrid(int iXQuads, int iZQuads,
							float fXSize, float fZSize,
							float fXOffset, float fZOffset,
							float xTiling, float zTiling)
{
	FPoint3	p;
	int		i, j, iNVerts;

	// safety check
	if (iXQuads < 1) iXQuads = 1;		// min x = 1, z = 1
	if (iZQuads < 1) iZQuads = 1;		// min x = 1, z = 1

	// size of vertex grid is one larger than the size of the quad grid
	m_iXVerts = iXQuads + 1;
	m_iZVerts = iZQuads + 1;

	iNVerts = m_iXVerts * m_iZVerts;

	m_fXSize = fXSize;
	m_fZSize = fZSize;

	//
	// Populate the quadmesh with locations, normals, and texture coordinates
	//
	for (i = 0; i < m_iXVerts; i++)
	{
		for (j = 0; j < m_iZVerts; j++)
		{
			p.Set(fXOffset + i*fXSize, 0.0, fZOffset + j*fZSize);
			int vidx = AddVertex(p);
			if (GetVtxType() & VT_Normals)
			{
				p.x = 0.0f;
				p.y = 1.0f;
				p.z = 0.0f;
				SetVtxNormal(vidx, p);
			}
			if (GetVtxType() & VT_TexCoords)		/* compute tex coords */
			{
				FPoint2 tc((float) i/iXQuads * xTiling,
							(float) j/iZQuads * zTiling);
				SetVtxTexCoord(vidx, tc);
			}
		}
	}
	CreateRectangularMesh(m_iXVerts, m_iZVerts);
	return true;
}


float dist_to_point(FPoint2 p1, FPoint2 p2)
{
	FPoint2 diff = p1 - p2;
	return diff.Length() / 3.0f;
}

void TerrainPatch::MoveVerticesIntoSineCurve(float fOffset)
{
	int iNum = GetNumVertices();

//	float ang;
	FPoint3 point;
	FPoint2 p1, p2(45.0f, -20.f), p3(65.0f, -55.f), base, diff;
	bool first = true;

	for (int i = 0; i < iNum; i++)
	{
		point = GetVtxPos(i);
		p1.x = point.x;
		p1.y = point.z;
		point.y = (OSC_MAG/2.0f) * sinf(dist_to_point(p1, p2)+fOffset) +
				(OSC_MAG/2.0f) * sinf(dist_to_point(p1, p3)+fOffset);
		SetVtxPos(i, point);
	}

	// must do this for correct lighting after moving the vertex positions
//	if (GetVtxType() & VT_Normals)
//		MakeNormals();

	if (GetVtxType() & VT_Colors)
		SetVertexColors();
}


void TerrainPatch::SetVertexTextureCoords()
{
	int iNum = GetNumVertices();
	FPoint3 p3;

	for (int i = 0; i < iNum; i++)
	{
		p3 = GetVtxPos(i);
		SetVtxTexCoord(i, FPoint2(p3.x / 10.0f, -p3.z / 10.0f));
	}
}


void TerrainPatch::SetVertexColors()
{
	int iNum = GetNumVertices();
	FPoint3 p3;

	for (int i = 0; i < iNum; i++)
	{
		p3 = GetVtxPos(i);
		float r, g, b;
		if (p3.y > 0.0f)
			r = g = b = p3.y / OSC_MAG;
		else
		{
			r = g = 0.0f;
			b = (-p3.y / OSC_MAG);
		}
		SetVtxColor(i, RGBf(r, g, b));
	}
}


//
// Quick n' dirty elevation test for perfectly regular grid terrain
// Find altitude (y) and surface normal, given (x,z) local coordinates
//
// This approach is really straightforward, so it could be majorly sped up if needed
//
bool TerrainPatch::FindAltitudeAtPoint(const FPoint3 &point, float &fAltitude,
									   FPoint3 *vNormal) const
{
	if (m_bIsFlat)
	{
		fAltitude = 0.0f;
		if (vNormal) vNormal->Set(0.0f, 1.0f, 0.0f);
		return true;
	}

	FPoint3 porigin = GetVtxPos(0);

	int iX = (int)((point.x - porigin.x) / m_fXSize);
	int iZ = -(int)((point.z - porigin.z) / m_fZSize);

	// safety check
	if (iX < 0 || iX >= m_iXVerts-1 || iZ < 0 || iZ >= m_iZVerts-1)
	{
		fAltitude = 0.0f;
		if (vNormal) vNormal->Set(0.0f, 1.0f, 0.0f);
		return false;
	}

	int iIndex0 = iX * m_iZVerts + iZ;
	int iIndex1 = iIndex0 + m_iZVerts;
	int iIndex2 = iIndex1 + 1;
	int iIndex3 = iIndex0 + 1;

	FPoint3 p0 = GetVtxPos(iIndex0);
	FPoint3 p1 = GetVtxPos(iIndex1);
	FPoint3 p2 = GetVtxPos(iIndex2);
	FPoint3 p3 = GetVtxPos(iIndex3);

	// find fractional amount (0..1 across quad)
	float fX = (point.x - p0.x) / m_fXSize;
	float fZ = -(point.z - p0.z) / m_fZSize;

	// which of the two triangles in the quad is it?
	if (fX + fZ < 1)
	{
		fAltitude = p0.y + fX * (p1.y - p0.y) + fZ * (p3.y - p0.y);

		if (vNormal)
		{
			// find normal also
			FPoint3 edge0 = p1 - p0;
			FPoint3 edge1 = p3 - p0;
			*vNormal = edge0.Cross(edge1);
			vNormal->Normalize();
		}
	}
	else
	{
		fAltitude = p2.y + (1.0f-fX) * (p3.y - p2.y) + (1.0f-fZ) * (p1.y - p2.y);

		if (vNormal)
		{
			// find normal also
			FPoint3 edge0 = p3 - p2;
			FPoint3 edge1 = p1 - p2;
			*vNormal = edge0.Cross(edge1);
			vNormal->Normalize();
		}
	}
	return true;
}

/////////////////////
// helpers

vtMovGeom *CreatePlaneMGeom(vtMaterialArray *pMats, int iMatIdx,
							FPoint2 org, FPoint2 size,
							float xTiling, float zTiling, int steps)
{
	vtGeom *pGeom = new vtGeom();
	TerrainPatch *geo = new TerrainPatch(VT_Normals | VT_TexCoords, steps * steps);
	geo->MakeGrid(steps, steps, size.x/steps, size.y/steps,
		org.x, org.y,
		xTiling, zTiling);		// tiling
	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(geo, iMatIdx);
	return new vtMovGeom(pGeom);
}

vtGeom *CreatePlaneGeom(int iMatIdx, FPoint2 org, FPoint2 size,
						   float xTiling, float zTiling, int steps)
{
	vtGeom *pGeom = new vtGeom();
	TerrainPatch *geo = new TerrainPatch(VT_Normals | VT_TexCoords,
		(steps+1)*(steps+1));
	geo->MakeGrid(steps, steps,
		size.x/steps, size.y/steps,
		org.x, org.y,
		xTiling, zTiling);		// tiling
	pGeom->AddMesh(geo, iMatIdx);
	return pGeom;
}

