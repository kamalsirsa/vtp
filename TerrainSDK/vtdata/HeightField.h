
#ifndef HEIGHTFIELDH
#define HEIGHTFIELDH

#include "LocalConversion.h"

class vtElevationGrid;
class vtTin;

/**
 * A heightfield is any collection of surfaces such that, given a horizontal
 * X,Y position, there exists only a single elevation value.
 */
class vtHeightField
{
public:
	vtHeightField();

	/// Initialize this object from an Elevation Grid
	void Initialize(vtElevationGrid *pGrid);

	/// Initialize this object from a Tin
	void Initialize(vtTin *pTin);

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
	bool ContainsWorldPoint(float x, float z);
	void GetCenter(FPoint3 &center);

	/// Test if a point is within the extents of the grid.
	bool ContainsEarthPoint(double x, double y)
	{
		return (m_EarthExtents.left <= x && x <= m_EarthExtents.right &&
				m_EarthExtents.bottom <= y && y <= m_EarthExtents.top);
	}

	/** Return geographic extents of the grid. */
	DRECT &GetEarthExtents()			{ return m_EarthExtents; }
	const DRECT &GetEarthExtents() const { return m_EarthExtents; }

	/** Set the geographic extents of the grid. */
	void SetEarthExtents(DRECT &ext)	{ m_EarthExtents = ext; }

	// minimum and maximum height values for the whole grid
	float	m_fMinHeight, m_fMaxHeight;

	vtLocalConversion	m_Conversion;
	DRECT	m_EarthExtents;		// raw extents (geographic or projected)
	FRECT	m_WorldExtents;		// cooked (OpenGL) extents (in the XZ plane)

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
	vtHeightFieldGrid();

	void Initialize(vtElevationGrid *pGrid);
	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;
	DPoint2 GetWorldSpacing();

protected:
	int		m_iColumns, m_iRows;
	float	m_fXStep, m_fZStep;	// step size between the World grid points
};

#endif	// HEIGHTFIELDH

