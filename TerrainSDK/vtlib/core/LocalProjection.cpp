//
// LocalProjection.cpp
//
// This library has a concept "current projection" which is represented
// by the vtLocalProjection class.
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
// The global current projection is stored here.
//
vtLocalProjection g_Proj;

//
// Earth's diameter: 12756 km
// approximate circumference: 40074 km
// each degree of latitude: 111.3 km
//
#define earth_radius	6378000.0f		// in meters
#define meters_per_latitude	111300.0f

vtLocalProjection::vtLocalProjection() : vtProjection()
{
	m_fVerticalScale = WORLD_SCALE;
}

void vtLocalProjection::SetDegreeOrigin(const DPoint2 &degrees)
{
	m_EarthOrigin = degrees;

	// estimate meters per degree of longitude, using the terrain origin
	double r0, circ;

	r0 = earth_radius * cos(degrees.y / 180.0 * PI);
	circ = 2.0 * r0 * PI;
	m_fMetersPerLongitude = (float) (circ / 360.0);
}

void vtLocalProjection::SetMeterOrigin(const DPoint2 &meters)
{
	m_EarthOrigin = meters;
}

void vtLocalProjection::SetVerticalScale(float scale)
{
	m_fVerticalScale = scale;
}

void vtLocalProjection::convert_latlon_to_local_xz(float lat, float lon,
												   float &x, float &z)
{
	if (IsGeographic())
	{
		x = (float) ((lon - m_EarthOrigin.x) * m_fMetersPerLongitude * WORLD_SCALE);
		z = (float) -((lat - m_EarthOrigin.y) * meters_per_latitude * WORLD_SCALE);
	}
	// TODO: what if not geographic?
}

void vtLocalProjection::convert_latlon_to_local_xz(double lat, double lon,
												   float &x, float &z)
{
	convert_latlon_to_local_xz((float) lat, (float) lon, x, z);
}

void vtLocalProjection::convert_utm_to_xy(float utm_x, float utm_y,
										  float &x, float &y)
{
	x = (float) (utm_x - m_EarthOrigin.x);
	y = (float) (utm_y - m_EarthOrigin.y);
}

void vtLocalProjection::convert_utm_to_xy(double utm_x, double utm_y,
										  float &x, float &y)
{
	x = (float) (utm_x - m_EarthOrigin.x);
	y = (float) (utm_y - m_EarthOrigin.y);
}

void vtLocalProjection::convert_utm_to_local_xz(float utm_x, float utm_y,
												float &x, float &z)
{
	x = (float) (utm_x - m_EarthOrigin.x) * WORLD_SCALE;
	z = (float) -(utm_y - m_EarthOrigin.y) * WORLD_SCALE;
}

void vtLocalProjection::convert_utm_to_local_xz(double utm_x, double utm_y,
												float &x, float &z)
{
	x = (float) (utm_x - m_EarthOrigin.x) * WORLD_SCALE;
	z = (float) -(utm_y - m_EarthOrigin.y) * WORLD_SCALE;
}

void vtLocalProjection::convert_utm_vector_to_local_xz(float utm_x,
													   float utm_y, float &x, float &z)
{
	x = (float) (utm_x * WORLD_SCALE);
	z = (float) -(utm_y * WORLD_SCALE);
}

void vtLocalProjection::convert_local_xz_to_utm(float x, float z,
												float &utm_x, float &utm_y)
{
	utm_x = (float) (x / WORLD_SCALE + m_EarthOrigin.x);
	utm_y = (float) -(z / WORLD_SCALE + m_EarthOrigin.y);
}

void vtLocalProjection::convert_local_xz_to_utm(float x, float z,
												double &utm_x, double &utm_y)
{
	utm_x = (x / WORLD_SCALE + m_EarthOrigin.x);
	utm_y = (-z / WORLD_SCALE + m_EarthOrigin.y);
}

void vtLocalProjection::convert_local_xz_to_latlon(float x, float z,
												   double &lat, double &lon)
{
	lon = m_EarthOrigin.x + (x / m_fMetersPerLongitude / WORLD_SCALE);
	lat = m_EarthOrigin.y + (-z / meters_per_latitude / WORLD_SCALE);
}

//
// Convert from the coordinate system of the virtual world (x,y,z) to actual
// earth coodinates (map coordinates, altitude in meters)
//
void vtLocalProjection::ConvertToEarth(const FPoint3 &world, DPoint3 &earth)
{
	if (IsGeographic())
		convert_local_xz_to_latlon(world.x, world.z, earth.y, earth.x);
	else
		convert_local_xz_to_utm(world.x, world.z, earth.x, earth.y);
	earth.z = world.y / m_fVerticalScale;
}

void vtLocalProjection::ConvertFromEarth(const DPoint3 &earth, FPoint3 &world)
{
	if (IsGeographic())
		convert_latlon_to_local_xz(earth.y, earth.x, world.x, world.y);
	else
		convert_utm_to_local_xz(earth.x, earth.y, world.x, world.z);
	world.y = (float) (earth.z * m_fVerticalScale);
}

void vtLocalProjection::ConvertToEarth(float x, float z, DPoint2 &earth)
{
	if (IsGeographic())
		convert_local_xz_to_latlon(x, z, earth.y, earth.x);
	else
		convert_local_xz_to_utm(x, z, earth.x, earth.y);
}

void vtLocalProjection::ConvertFromEarth(const DPoint2 &earth, float &x, float &z)
{
	if (IsGeographic())
		convert_latlon_to_local_xz(earth.y, earth.x, x, z);
	else
		convert_utm_to_local_xz(earth.x, earth.y, x, z);
}

