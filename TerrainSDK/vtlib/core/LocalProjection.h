//
// LocalProjection.h
//
// This library has a concept of current conversion from earth to world
// coordinates which is represented by the vtLocalConversion class.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_LOCALPROJECTIONH
#define VTLIB_LOCALPROJECTIONH

// global conversion factor
#define WORLD_SCALE				0.001f	// 1 meter = 0.001 units

//
// The following class represents a mapping between real earth coordinates
// (geographic or projected, elevation in meters) and the OpenGL coordinates as
// rendered (x,y,z, with xz ground plane) including vertical exaggeration.
//
class vtLocalConversion
{
public:
	vtLocalConversion();

	void Setup(bool bGeo, const DRECT &earth_extents);
	void SetDegreeOrigin(const DPoint2 &degrees);
	void SetMeterOrigin(const DPoint2 &meters);
	void SetVerticalScale(float scale);

	void ConvertToEarth(const FPoint3 &world, DPoint3 &earth);
	void ConvertToEarth(float x, float z, DPoint2 &earth);

	void ConvertFromEarth(const DPoint2 &earth, float &x, float &z);
	void ConvertFromEarth(const DPoint3 &earth, FPoint3 &world);

	void convert_geo_to_local_xz(double lon, double lat, float &x, float &z);
	void convert_projected_to_local_xz(double px, double py, float &x, float &z);
	void convert_local_xz_to_projected(float x, float z, double &px, double &py);
	void convert_local_xz_to_geo(float x, float y, double &lon, double &lat);

	bool	m_bGeographic;
	FRECT	m_WorldExtents;		// cooked (OpenGL) extents (in the XZ plane)
	float	m_fVerticalScale;

protected:
	DPoint2	m_EarthOrigin;
	float	m_fMetersPerLongitude;
};

extern vtLocalConversion g_Conv;

#endif // VTLIB_LOCALPROJECTIONH
