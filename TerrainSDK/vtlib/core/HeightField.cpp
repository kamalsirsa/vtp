//
// HeightField.cpp
//
// A class to represent any regular grid of points representing a height field.
// Includes functionality to find the surface height at any given point.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "HeightField.h"
#include "LocalGrid.h"

void vtHeightField::Initialize(vtLocalGrid *pLocalGrid)
{
	// get earth (raw geographic) extents
	m_EarthExtents = pLocalGrid->GetGridExtents();

	// get grid size and world extents
	pLocalGrid->GetDimensions(m_iXPoints, m_iYPoints);
	pLocalGrid->GetWorldExtents(m_WorldExtents);

	// derive some useful values: world size and step size
	m_WorldSize.x = m_WorldExtents.right - m_WorldExtents.left;
	m_WorldSize.y = m_WorldExtents.bottom - m_WorldExtents.top;

	m_fXStep = m_WorldExtents.Width() / (m_iXPoints-1);
	m_fZStep = -m_WorldExtents.Height() / (m_iYPoints-1);

	// minimum and maximum height values for the whole grid
	pLocalGrid->GetHeightExtents(m_fMinHeight, m_fMaxHeight);

	// copy the projection information
	m_proj = pLocalGrid->GetProjection();

	m_fDiagonalLength = (float)sqrt(m_WorldSize.x * m_WorldSize.x +
		m_WorldSize.y * m_WorldSize.y);
}


bool vtHeightField::PointAboveTerrain(FPoint3 &p1)
{
	float alt;
	FindAltitudeAtPoint(p1, alt);
	if (alt < p1.y)
		return true;
	else
		return false;
}

#define PICK_ACCURACY	300

//
// Return true if hit terrain.
//
// Note: this algorithm is NOT gauranteed to give correct results,
// nor is it fast or efficient.  It checks a series of PICK_ACCURACY
// line segments against the terrain.  When one is found to straddle
// the terrain, it refines the segment in a binary fashion.
//
// Need a better algorithm!  There are probably many ray-heightfield
// approaches in the raytracing literature.
//
bool vtHeightField::CastRayToSurface(FPoint3 &point, FPoint3 &dir,
								   FPoint3 &result)
{
	// cast a series of line segment along the ray
	int i;
	FPoint3 p0 = point, p1, p2;
	for (i = 1; i < PICK_ACCURACY; i++)
	{
		p1 = point + (dir * (i * m_fDiagonalLength / PICK_ACCURACY));
		if (!PointAboveTerrain(p1))
			break;
		p0 = p1;
	}
	if (i == PICK_ACCURACY)
		return false;
	// now binary search
	for (i = 0; i < 20; i++)
	{
		p2 = (p0 + p1) / 2.0f;
		if (PointAboveTerrain(p2))
			p0 = p2;
		else
			p1 = p2;
	}
	p2 = (p0 + p1) / 2.0f;
	// make sure it's precisely on the ground
	FindAltitudeAtPoint(p2, p2.y);
	result = p2;
	return true;
}

//
// Convert
//
void vtHeightField::ConvertEarthToSurfacePoint(double x, double y, FPoint3 &p3)
{
	// convert earth -> XZ
	g_Proj.ConvertFromEarth(DPoint2(x, y), p3.x, p3.z);

	// look up altitude
	FindAltitudeAtPoint(p3, p3.y);
}

//
// Test whether a given point is within the current terrain
//
bool vtHeightField::PointIsInTerrain(float x, float z)
{
	if (x < m_WorldExtents.left || x > m_WorldExtents.right)
		return false;
	if (z > m_WorldExtents.bottom || z < m_WorldExtents.top)
		return false;
	return true;
}


void vtHeightField::GetCenter(FPoint3 &center)
{
	center.x = (float) (m_WorldExtents.left + m_WorldSize.x / 2.0f);
	center.z = (float) (m_WorldExtents.top + m_WorldSize.y / 2.0f);
	center.y = 0.0f;
}

