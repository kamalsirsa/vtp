//
// HeightField.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "HeightField.h"
#include "LocalGrid.h"
#include "vtTin3d.h"

void vtHeightField::Initialize(vtLocalGrid *pLocalGrid)
{
	m_EarthExtents = pLocalGrid->GetGridExtents();

	m_Conversion = pLocalGrid->m_Conversion;

	// minimum and maximum height values for the whole grid
	pLocalGrid->GetHeightExtents(m_fMinHeight, m_fMaxHeight);

	if (m_fMinHeight == INVALID_ELEVATION ||
		m_fMinHeight == INVALID_ELEVATION)
	{
		// we need height extents, so force them to be computed
		pLocalGrid->ComputeHeightExtents();
		pLocalGrid->GetHeightExtents(m_fMinHeight, m_fMaxHeight);
	}
	FPoint2 hypo(m_Conversion.m_WorldExtents.Width(),
				 m_Conversion.m_WorldExtents.Height());
	m_fDiagonalLength = hypo.Length();
}

void vtHeightField::Initialize(vtTin3d *pTin)
{
	m_Conversion.Setup(pTin->m_proj.GetUnits(), m_EarthExtents);
	m_Conversion.m_fVerticalScale;

	FPoint2 hypo(m_Conversion.m_WorldExtents.Width(),
				 m_Conversion.m_WorldExtents.Height());
	m_fDiagonalLength = hypo.Length();
}

/**
 * \return 0 if below terrain, 1 if above terrain, -1 if off terrain.
 */
int vtHeightField::PointIsAboveTerrain(const FPoint3 &p1)
{
	float alt;
	if (!FindAltitudeAtPoint(p1, alt))
		return -1;
	if (alt < p1.y)
		return 1;
	else
		return 0;
}

#define PICK_ACCURACY	1000

/**
 * Tests a ray against a heightfield grid.
 *
 * Note: this algorithm is NOT guaranteed to give correct results,
 * nor is it fast or efficient.  It checks a series of PICK_ACCURACY
 * line segments against the terrain.  When one is found to straddle
 * the terrain, it refines the segment in a binary fashion.
 *
 * Need a better algorithm!  There are probably many ray-heightfield
 * approaches in the raytracing literature.
 *
 * \return true if hit terrain.
 */
bool vtHeightFieldGrid::CastRayToSurface(const FPoint3 &point,
										 const FPoint3 &dir, FPoint3 &result)
{
	// cast a series of line segment along the ray
	int i, above;
	FPoint3 p0 = point, p1, p2 = point;
	FPoint3 delta = dir * (m_fDiagonalLength / (m_iXPoints * 1.41f));
	bool found_above = false;
	for (i = 0; i < PICK_ACCURACY; i++)
	{
		above = PointIsAboveTerrain(p2);
		if (above == 0)	// below
		{
			p1 = p2;
			break;
		}
		if (above == 1)	// above
		{
			found_above = true;
			p0 = p2;
		}
		p2 += delta;
	}
	if (i == PICK_ACCURACY || !found_above)
		return false;
	// now, do a binary search to refine the result
	for (i = 0; i < 10; i++)
	{
		p2 = (p0 + p1) / 2.0f;
		above = PointIsAboveTerrain(p2);
		if (above == 1)	// above
			p0 = p2;
		else if (above == 0)	// below
			p1 = p2;
	}
	p2 = (p0 + p1) / 2.0f;
	// make sure it's precisely on the ground
	FindAltitudeAtPoint(p2, p2.y);
	result = p2;
	return true;
}

/**
 * Converts a earth coordinate (project or geographic) to a world coordinate
 * on the surface of the heightfield.
 */
void vtHeightField::ConvertEarthToSurfacePoint(double x, double y, FPoint3 &p3)
{
	// convert earth -> XZ
	m_Conversion.ConvertFromEarth(DPoint2(x, y), p3.x, p3.z);

	// look up altitude
	FindAltitudeAtPoint(p3, p3.y);
}

/**
 * Tests whether a given point is within the current terrain
 */
bool vtHeightField::PointIsInTerrain(float x, float z)
{
	return m_Conversion.m_WorldExtents.ContainsPoint(x, z);
}


void vtHeightField::GetCenter(FPoint3 &center)
{
	FPoint2 c;
	m_Conversion.m_WorldExtents.Center(c);
	center.x = c.x;
	center.z = c.y;
	center.y = 0.0f;
}


/////////////////////////////////////////////////////////////////////////////

void vtHeightFieldGrid::Initialize(vtLocalGrid *pLocalGrid)
{
	// first initialize parent
	vtHeightField::Initialize(pLocalGrid);

	pLocalGrid->GetDimensions(m_iXPoints, m_iYPoints);
	m_fXStep = m_Conversion.m_WorldExtents.Width() / (m_iXPoints-1);
	m_fZStep = -m_Conversion.m_WorldExtents.Height() / (m_iYPoints-1);
}

