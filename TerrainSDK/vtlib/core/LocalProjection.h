//
// LocalProjection.h
//
// This library has a concept "current projection" which is represented
// by the vtLocalProjection class.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_LOCALPROJECTIONH
#define VTLIB_LOCALPROJECTIONH

#include "vtdata/Projections.h"

// global projection factors
#define WORLD_SCALE				0.001f	// 1 meter = 0.001 units

//
// The following class represents a mapping between real earth coordinates
// (geographic or UTM, meter elevation) and the OpenGL coordinates as
// rendered (x,y,z, with xz ground plane) including vertical exaggeration.
//
class vtLocalProjection : public vtProjection
{
public:
	vtLocalProjection();

	void SetDegreeOrigin(const DPoint2 &degrees);
	void SetMeterOrigin(const DPoint2 &meters);
	void SetVerticalScale(float scale);

	void ConvertToEarth(const FPoint3 &world, DPoint3 &earth);
	void ConvertFromEarth(const DPoint3 &earth, FPoint3 &world);

	void ConvertToEarth(float x, float z, DPoint2 &earth);
	void ConvertFromEarth(const DPoint2 &earth, float &x, float &z);

	void convert_latlon_to_local_xz(float lat, float lon, float &x, float &z);
	void convert_latlon_to_local_xz(double lat, double lon, float &x, float &z);

	void convert_utm_to_xy(float utm_x, float utm_y, float &x, float &y);
	void convert_utm_to_xy(double utm_x, double utm_y, float &x, float &y);
	void convert_utm_to_local_xz(float utm_x, float utm_y, float &x, float &z);
	void convert_utm_to_local_xz(double utm_x, double utm_y, float &x, float &z);
	void convert_utm_vector_to_local_xz(float utm_x, float utm_y, float &x, float &z);
	void convert_local_xz_to_utm(float x, float z, float &utm_x, float &utm_y);
	void convert_local_xz_to_utm(float x, float z, double &utm_x, double &utm_y);
	void convert_local_xz_to_latlon(float x, float y, double &lat, double &lon);

protected:
	DPoint2	m_EarthOrigin;
	float	m_fVerticalScale;
	float	m_fMetersPerLongitude;
};

extern vtLocalProjection g_Proj;

extern void SetLocalProjection(vtProjection &proj, DPoint2 lower_left);

#endif // VTLIB_LOCALPROJECTIONH
