//
// LocalProjection.cpp
//
// This library has a concept of current conversion from earth to world
// coordinates which is represented by the vtLocalConversion class.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vtdata/MathTypes.h"
#include "vtdata/Projections.h"
#include "LocalProjection.h"

//
// The global current projection for the current terrain is stored here.
//
vtLocalConversion g_Conv;

////////////////////////////////////////////////////////////////////////////

vtLocalConversion::vtLocalConversion()
{
	m_bGeographic = true;
	m_EarthOrigin.Set(0, 0);
	m_fVerticalScale = WORLD_SCALE;
	m_fMetersPerLongitude = (float) EstimateDegreesToMeters(0);
}

void vtLocalConversion::Setup(bool bGeo, const DRECT &earth_extents)
{
	DPoint2 origin(earth_extents.left, earth_extents.bottom);
	if (bGeo)
	{
		SetDegreeOrigin(origin);
		convert_geo_to_local_xz(earth_extents.left, earth_extents.bottom, m_WorldExtents.left, m_WorldExtents.bottom);
		convert_geo_to_local_xz(earth_extents.right, earth_extents.top, m_WorldExtents.right, m_WorldExtents.top);
	}
	else
	{
		SetMeterOrigin(origin);
		convert_projected_to_local_xz(earth_extents.left, earth_extents.bottom, m_WorldExtents.left, m_WorldExtents.bottom);
		convert_projected_to_local_xz(earth_extents.right, earth_extents.top, m_WorldExtents.right, m_WorldExtents.top);
	}
}
void vtLocalConversion::SetDegreeOrigin(const DPoint2 &degrees)
{
	m_EarthOrigin = degrees;

	// estimate meters per degree of longitude, using the terrain origin
	m_fMetersPerLongitude = (float) EstimateDegreesToMeters(degrees.y);

	m_bGeographic = true;
}

void vtLocalConversion::SetMeterOrigin(const DPoint2 &meters)
{
	m_EarthOrigin = meters;

	m_bGeographic = false;
}

void vtLocalConversion::SetVerticalScale(float scale)
{
	m_fVerticalScale = scale;
}

void vtLocalConversion::convert_geo_to_local_xz(double lon, double lat,
												float &x, float &z)
{
	if (m_bGeographic)
	{
		x = (float) ((lon - m_EarthOrigin.x) * m_fMetersPerLongitude * WORLD_SCALE);
		z = (float) -((lat - m_EarthOrigin.y) * METERS_PER_LATITUDE * WORLD_SCALE);
	}
	// TODO: what if not geographic?
}

void vtLocalConversion::convert_projected_to_local_xz(double px, double py,
													  float &x, float &z)
{
	x = (float) (px - m_EarthOrigin.x) * WORLD_SCALE;
	z = (float) -(py - m_EarthOrigin.y) * WORLD_SCALE;
}

void vtLocalConversion::convert_local_xz_to_projected(float x, float z,
													  double &px, double &py)
{
	px = (x / WORLD_SCALE + m_EarthOrigin.x);
	py = (-z / WORLD_SCALE + m_EarthOrigin.y);
}

void vtLocalConversion::convert_local_xz_to_geo(float x, float z,
												double &lon, double &lat)
{
	lon = m_EarthOrigin.x + (x / m_fMetersPerLongitude / WORLD_SCALE);
	lat = m_EarthOrigin.y + (-z / METERS_PER_LATITUDE / WORLD_SCALE);
}

//
// Convert from the coordinate system of the virtual world (x,y,z) to actual
// earth coodinates (map coordinates, altitude in meters)
//
void vtLocalConversion::ConvertToEarth(const FPoint3 &world, DPoint3 &earth)
{
	if (m_bGeographic)
		convert_local_xz_to_geo(world.x, world.z, earth.x, earth.y);
	else
		convert_local_xz_to_projected(world.x, world.z, earth.x, earth.y);
	earth.z = world.y / m_fVerticalScale;
}

void vtLocalConversion::ConvertFromEarth(const DPoint3 &earth, FPoint3 &world)
{
	if (m_bGeographic)
		convert_geo_to_local_xz(earth.x, earth.y, world.x, world.y);
	else
		convert_projected_to_local_xz(earth.x, earth.y, world.x, world.z);
	world.y = (float) (earth.z * m_fVerticalScale);
}

void vtLocalConversion::ConvertToEarth(float x, float z, DPoint2 &earth)
{
	if (m_bGeographic)
		convert_local_xz_to_geo(x, z, earth.x, earth.y);
	else
		convert_local_xz_to_projected(x, z, earth.x, earth.y);
}

void vtLocalConversion::ConvertFromEarth(const DPoint2 &earth, float &x, float &z)
{
	if (m_bGeographic)
		convert_geo_to_local_xz(earth.x, earth.y, x, z);
	else
		convert_projected_to_local_xz(earth.x, earth.y, x, z);
}

