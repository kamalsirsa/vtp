
#ifndef HEIGHTFIELDH
#define HEIGHTFIELDH

#include "LocalProjection.h"

class vtLocalGrid;
class vtTin3d;

/**
 * A heightfield is any collection of surfaces such that, given a horizontal
 * X,Y position, there exists only a single elevation value.
 */
class vtHeightField
{
public:
	/// Initialize this object from a LocalGrid
	void Initialize(vtLocalGrid *pLocalGrid);

	/// Initialize this object from a Tin
	void Initialize(vtTin3d *pTin);

	/// Given a point in world coordinates, determine the elevation
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		FPoint3 *vNormal = NULL) const = 0;

	/// Find the intersection point of a ray with the heightfield
	virtual bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const = 0;

	int PointIsAboveTerrain(const FPoint3 &p1) const;

	void ConvertEarthToSurfacePoint(double ex, double ey, FPoint3 &p3);
	void ConvertEarthToSurfacePoint(const DPoint2 &epos, FPoint3 &p3)
	{
		ConvertEarthToSurfacePoint(epos.x, epos.y, p3);
	}
	bool PointIsInTerrain(float x, float z);
	void GetCenter(FPoint3 &center);

	// minimum and maximum height values for the whole grid
	float	m_fMinHeight, m_fMaxHeight;

	vtLocalConversion	m_Conversion;
	DRECT	m_EarthExtents;		// raw extents (geographic or projected)

protected:
	float	m_fDiagonalLength;
};


/**
 * vtHeightFieldGrid extends vtHeightField with the knowledge of operating
 * on a regular grid of elevation values.
 */
class vtHeightFieldGrid : public vtHeightField
{
public:
	void Initialize(vtLocalGrid *pLocalGrid);
	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;

protected:
	int		m_iXPoints, m_iYPoints;	// height field grid dimensions
	float	m_fXStep, m_fZStep;	// step size between the World grid points
};

#endif	// HEIGHTFIELDH

