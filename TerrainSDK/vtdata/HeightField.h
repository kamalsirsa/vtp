
#ifndef HEIGHTFIELDH
#define HEIGHTFIELDH

#include <limits.h>			// for SHRT_MIN
#include "LocalConversion.h"

#define INVALID_ELEVATION	SHRT_MIN

/**
 * A heightfield is any collection of surfaces such that, given a horizontal
 * X,Y position, there exists only a single elevation value.
 */
class vtHeightField
{
public:
	vtHeightField();

	// Initialize the vtHeightField
	void Initialize(const DRECT &earthcover, float fMinHeight, float fMaxHeight);

	/// Return an MD5 checksum for this heightfield
	virtual void GetChecksum(unsigned char **ppChecksum) const = 0;
	virtual bool FindAltitudeAtPoint2(const DPoint2 &p, float &fAltitude) const = 0;

	/// Test if a point is within the extents of the grid.
	bool ContainsEarthPoint(const DPoint2 &p) const
	{
		return (m_EarthExtents.left <= p.x && p.x <= m_EarthExtents.right &&
				m_EarthExtents.bottom <= p.y && p.y <= m_EarthExtents.top);
	}

	/** Return geographic extents of the grid. */
	DRECT &GetEarthExtents()			{ return m_EarthExtents; }
	const DRECT &GetEarthExtents() const { return m_EarthExtents; }

	/** Set the geographic extents of the grid. */
	void SetEarthExtents(const DRECT &ext)	{ m_EarthExtents = ext; }
	void GetHeightExtents(float &fMinHeight, float &fMaxHeight) const;

protected:
	// minimum and maximum height values for the whole heightfield
	float	m_fMinHeight, m_fMaxHeight;

	DRECT	m_EarthExtents;		// raw extents (geographic or projected)
};


class vtHeightField3d : public vtHeightField
{
public:
	void Initialize(const LinearUnits units, const DRECT &earthextents,
		float fMinHeight, float fMaxHeight);

	/// Given a point in world coordinates, determine the elevation
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		FPoint3 *vNormal = NULL) const = 0;

	/// Find the intersection point of a ray with the heightfield
	virtual bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const = 0;

	int PointIsAboveTerrain(const FPoint3 &p) const;

	bool ConvertEarthToSurfacePoint(double ex, double ey, FPoint3 &p3);
	bool ConvertEarthToSurfacePoint(const DPoint2 &epos, FPoint3 &p3)
	{
		return ConvertEarthToSurfacePoint(epos.x, epos.y, p3);
	}

	bool ContainsWorldPoint(float x, float z);
	void GetCenter(FPoint3 &center);

	FRECT	m_WorldExtents;		// cooked (OpenGL) extents (in the XZ plane)
	vtLocalConversion	m_Conversion;

protected:
	float	m_fDiagonalLength;
};


/**
 * vtHeightFieldGrid extends vtHeightField with the knowledge of operating
 * on a regular grid of elevation values.
 */
class vtHeightFieldGrid3d : public vtHeightField3d
{
public:
	vtHeightFieldGrid3d();

	void Initialize(const LinearUnits units, const DRECT &earthextents,
		float fMinHeight, float fMaxHeight, int cols, int rows);

	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;
	DPoint2 GetSpacing() const;
	FPoint2 GetWorldSpacing() const;

protected:
	int		m_iColumns, m_iRows;
	float	m_fXStep, m_fZStep;	// step size between the World grid points
};

#endif	// HEIGHTFIELDH


