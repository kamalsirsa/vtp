//
// HeightField.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "HeightField.h"

vtHeightField::vtHeightField()
{
	m_EarthExtents.SetRect(0, 0, 0, 0);
}

void vtHeightField::Initialize(const DRECT &earthcover, float fMinHeight,
							   float fMaxHeight)
{
	m_EarthExtents = earthcover;
	m_fMinHeight = fMinHeight;
	m_fMaxHeight = fMaxHeight;
}

/** Gets the minimum and maximum height values.  The values are placed in the
 * arguments by reference.  You must have first called ComputeHeightExtents.
 */
void vtHeightField::GetHeightExtents(float &fMinHeight, float &fMaxHeight) const
{
	fMinHeight = m_fMinHeight;
	fMaxHeight = m_fMaxHeight;
}


/**
 * \return 0 if below terrain, 1 if above terrain, -1 if off terrain.
 */
int vtHeightField3d::PointIsAboveTerrain(const FPoint3 &p) const
{
	float alt;
	if (!FindAltitudeAtPoint(p, alt))
		return -1;
	if (alt < p.y)
		return 1;
	else
		return 0;
}

#define PICK_DISTANCE	1000

/**
 * Tests a ray against a heightfield grid.
 *
 * Note: this algorithm is not guaranteed to give absolutely correct results,
 * but it is reasonably fast or efficient.  It checks a series of PICK_DISTANCE
 * points along the ray against the terrain.  When a pair of points (segment)
 * is found to straddle the terrain, it refines the segment in a binary fashion.
 *
 * Since the length of the test is proportional to a single grid element,
 * there is a very small chance that it will give results that are off by
 * a small distance (less than 1 grid element)
 *
 * \return true if hit terrain.
 */
bool vtHeightFieldGrid3d::CastRayToSurface(const FPoint3 &point,
										   const FPoint3 &dir, FPoint3 &result) const
{
	// cast a series of line segment along the ray
	int i, above;
	FPoint3 p0 = point, p1, p2 = point;
	FPoint3 delta = dir * (m_fDiagonalLength / (m_iColumns * 1.41f));
	bool found_above = false;
	for (i = 0; i < PICK_DISTANCE; i++)
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
	if (i == PICK_DISTANCE || !found_above)
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
bool vtHeightField3d::ConvertEarthToSurfacePoint(double x, double y, FPoint3 &p3)
{
	// convert earth -> XZ
	m_Conversion.ConvertFromEarth(DPoint2(x, y), p3.x, p3.z);

	// look up altitude
	return FindAltitudeAtPoint(p3, p3.y);
}

/**
 * Tests whether a given point is within the current terrain
 */
bool vtHeightField3d::ContainsWorldPoint(float x, float z)
{
	const FRECT &we = m_WorldExtents;
	return (x > we.left && x < we.right && z < we.bottom && z > we.top);
}


void vtHeightField3d::GetCenter(FPoint3 &center)
{
	FPoint2 c;
	m_WorldExtents.Center(c);
	center.x = c.x;
	center.z = c.y;
	center.y = 0.0f;
}


/////////////////////////////////////////////////////////////////////////////

void vtHeightField3d::Initialize(const LinearUnits units,
	const DRECT &earthextents, float fMinHeight, float fMaxHeight)
{
	vtHeightField::Initialize(earthextents, fMinHeight, fMaxHeight);

	m_Conversion.Setup(units,
		DPoint2(m_EarthExtents.left, m_EarthExtents.bottom));

	m_Conversion.convert_earth_to_local_xz(
		m_EarthExtents.left, m_EarthExtents.bottom,
		m_WorldExtents.left, m_WorldExtents.bottom);

	m_Conversion.convert_earth_to_local_xz(
		m_EarthExtents.right, m_EarthExtents.top,
		m_WorldExtents.right, m_WorldExtents.top);

	FPoint2 hypo(m_WorldExtents.Width(), m_WorldExtents.Height());
	m_fDiagonalLength = hypo.Length();
}


/////////////////////////////////////////////////////////////////////////////
// vtHeightFieldGrid3d

vtHeightFieldGrid3d::vtHeightFieldGrid3d()
{
	m_iColumns = 0;
	m_iRows = 0;
	m_fXStep = 0.0f;
	m_fZStep = 0.0f;
}

void vtHeightFieldGrid3d::Initialize(const LinearUnits units,
	const DRECT& earthextents, float fMinHeight, float fMaxHeight,
	int cols, int rows)
{
	// first initialize parent
	vtHeightField3d::Initialize(units, earthextents, fMinHeight, fMaxHeight);

	m_iColumns = cols;
	m_iRows = rows;

	m_fXStep = m_WorldExtents.Width() / (m_iColumns-1);
	m_fZStep = -m_WorldExtents.Height() / (m_iRows-1);
}


/** Get the grid spacing, the width of each column and row.
 */
DPoint2 vtHeightFieldGrid3d::GetSpacing() const
{
	return DPoint2(m_EarthExtents.Width() / (m_iColumns - 1),
		m_EarthExtents.Height() / (m_iRows - 1));
}

FPoint2 vtHeightFieldGrid3d::GetWorldSpacing() const
{
	return FPoint2(m_fXStep, m_fZStep);
}

