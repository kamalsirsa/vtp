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
#define WORLD_SCALE				1.0f	// 1 meter = 1.0 units

//
// The following class represents a mapping between real earth coordinates
// (geographic or projected, elevation in meters) and the OpenGL coordinates as
// rendered (x,y,z, with xz ground plane) including vertical exaggeration.
//
class vtLocalConversion
{
public:
	vtLocalConversion();

	void Setup(enum LinearUnits units, const DRECT &earth_extents);
	void SetVerticalScale(float scale);

	void ConvertToEarth(const FPoint3 &world, DPoint3 &earth);
	void ConvertToEarth(float x, float z, DPoint2 &earth);

	void ConvertFromEarth(const DPoint2 &earth, float &x, float &z);
	void ConvertFromEarth(const DPoint3 &earth, FPoint3 &world);

	void convert_earth_to_local_xz(double ex, double ey, float &x, float &z);
	void convert_local_xz_to_earth(float x, float z, double &ex, double &ey);

	FRECT	m_WorldExtents;		// cooked (OpenGL) extents (in the XZ plane)
	float	m_fVerticalScale;
	enum LinearUnits m_units;

protected:
	DPoint2	m_EarthOrigin;
	DPoint2	m_scale;
};

extern vtLocalConversion g_Conv;

#endif // VTLIB_LOCALPROJECTIONH
