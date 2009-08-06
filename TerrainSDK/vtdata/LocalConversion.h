//
// LocalConversion.h
//
// This library has a concept of current conversion from earth to world
// coordinates which is represented by the vtLocalConversion class.
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_LOCALCONVERSIONH
#define VTLIB_LOCALCONVERSIONH

// global conversion factor
#define WORLD_SCALE				1.0f	// 1 meter = 1.0 units

#include "Projections.h"		// for LinearUnits

/**
 * The following class represents a mapping between real earth coordinates
 * (geographic or projected, elevation in meters) and a local, meters-based
 * 3D coordinate system, which uses the right-handed OpenGL axis convention
 * (X right, Y up, Z backwards)
 */
class vtLocalConversion
{
public:
	vtLocalConversion();

	void Setup(LinearUnits units, const DRECT &earthextents);
	void SetOrigin(const DPoint2 &origin);

	void convert_earth_to_local_xz(double ex, double ey, float &x, float &z) const;
	void convert_local_xz_to_earth(float x, float z, double &ex, double &ey) const;

	void ConvertToEarth(const FPoint3 &world, DPoint3 &earth) const;
	void ConvertToEarth(float x, float z, DPoint2 &earth) const;

	void ConvertFromEarth(const DPoint2 &earth, float &x, float &z) const;
	void ConvertFromEarth(const DPoint3 &earth, FPoint3 &world) const;
	void ConvertFromEarth(const DRECT &earth, FRECT &world) const;

	void ConvertVectorToEarth(float x, float z, DPoint2 &earth) const;
	void ConvertVectorFromEarth(const DPoint2 &earth, float &x, float &z) const;

	LinearUnits GetUnits() const { return m_units; }

protected:
	LinearUnits m_units;
	DPoint2	m_EarthOrigin;
	DPoint2	m_scale;
};

extern vtLocalConversion g_Conv;

#endif // VTLIB_LOCALCONVERSIONH

