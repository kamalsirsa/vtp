//
// LocalConversion.h
//
// This library has a concept of current conversion from earth to world
// coordinates which is represented by the vtLocalConversion class.
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_LOCALCONVERSIONH
#define VTLIB_LOCALCONVERSIONH

// global conversion factor
#define WORLD_SCALE				1.0f	// 1 meter = 1.0 units

#include "Projections.h"		// for LinearUnits

//
// The following class represents a mapping between real earth coordinates
// (geographic or projected, elevation in meters) and a local, meters-based
// 3D coordinate system, which uses the right-handed OpenGL axis convention
// (X right, Y up, Z backwards)
//
class vtLocalConversion
{
public:
	vtLocalConversion();

	void Setup(LinearUnits units, const DPoint2 &origin);

	void ConvertToEarth(const FPoint3 &world, DPoint3 &earth);
	void ConvertToEarth(float x, float z, DPoint2 &earth);

	void ConvertFromEarth(const DPoint2 &earth, float &x, float &z);
	void ConvertFromEarth(const DPoint3 &earth, FPoint3 &world);

	void convert_earth_to_local_xz(double ex, double ey, float &x, float &z);
	void convert_local_xz_to_earth(float x, float z, double &ex, double &ey);

	LinearUnits GetUnits() { return m_units; }

protected:
	LinearUnits m_units;
	DPoint2	m_EarthOrigin;
	DPoint2	m_scale;
};

extern vtLocalConversion g_Conv;

#endif // VTLIB_LOCALCONVERSIONH

