
#ifndef HEIGHTFIELDH
#define HEIGHTFIELDH

#include <limits.h>			// for SHRT_MIN
#include "LocalConversion.h"

class vtBitmapBase;
#define INVALID_ELEVATION	SHRT_MIN

/**
 * This small class describes how to map elevation (as from a heightfield)
 * onto a set of colors.
 */
class ColorMap
{
public:
	ColorMap();
	bool Save(const char *fname);
	bool Load(const char *fname);
	void Add(float elev, const RGBi &color);
	void RemoveAt(int num);
	int Num() const;
	void GenerateColors(std::vector<RGBi> &table, int iTableSize, float fMin, float fMax) const;

	bool m_bBlend;
	bool m_bRelative;
	std::vector<float> m_elev;
	std::vector<RGBi> m_color;
};


/**
 * A heightfield is any collection of surfaces such that, given a horizontal
 * X,Y position, there exists only a single elevation value.
 */
class vtHeightField
{
public:
	vtHeightField();

	// Initialize the vtHeightField
	void Initialize(const DRECT &extents, float fMinHeight, float fMaxHeight);

	// Return an MD5 checksum for this heightfield
	virtual void GetChecksum(unsigned char **ppChecksum) const = 0;
	virtual bool FindAltitudeAtPoint2(const DPoint2 &p, float &fAltitude, bool bTrue = false) const = 0;

	/** Test if a point is within the extents of the grid. */
	bool ContainsEarthPoint(const DPoint2 &p) const
	{
		return (m_EarthExtents.left <= p.x && p.x <= m_EarthExtents.right &&
				m_EarthExtents.bottom <= p.y && p.y <= m_EarthExtents.top);
	}

	/** Returns the geographic extents of the grid. */
	DRECT &GetEarthExtents()			{ return m_EarthExtents; }
	const DRECT &GetEarthExtents() const { return m_EarthExtents; }

	/** Set the geographic extents of the grid. */
	virtual void SetEarthExtents(const DRECT &ext);
	void GetHeightExtents(float &fMinHeight, float &fMaxHeight) const;

protected:
	// minimum and maximum height values for the whole heightfield
	float	m_fMinHeight, m_fMaxHeight;

	DRECT	m_EarthExtents;		// raw extents (geographic or projected)
};


/**
 * This class extents vtHeightField with the abilty to operate in 'world'
 *  coordinates, that is, an artificial meters-based 3D coordinate system
 *  which is much better suited for many tasks than trying to operate
 *  directly on the raw 2D 'earth' coordinates.
 */
class vtHeightField3d : public vtHeightField
{
public:
	void Initialize(const LinearUnits units, const DRECT &earthextents,
		float fMinHeight, float fMaxHeight);

	// override heightfield method
	virtual void SetEarthExtents(const DRECT &ext);

	/// Given a point in world coordinates, determine the elevation
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, FPoint3 *vNormal = NULL) const = 0;

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
	void UpdateWorldExtents();

	float	m_fDiagonalLength;
};


/**
 * This class extends vtHeightField3d with the knowledge of operating
 * on a regular grid of elevation values.
 */
class vtHeightFieldGrid3d : public vtHeightField3d
{
public:
	vtHeightFieldGrid3d();

	void Initialize(const LinearUnits units, const DRECT &earthextents,
		float fMinHeight, float fMaxHeight, int cols, int rows);

	// override heightfield method
	virtual void SetEarthExtents(const DRECT &ext);

	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;
	bool LineOfSight(const FPoint3 &point1, const FPoint3 &point2) const;
	DPoint2 GetSpacing() const;
	FPoint2 GetWorldSpacing() const;
	void GetDimensions(int &nColumns, int &nRows) const;

	// all grids must be able to return the elevation at a grid point
	virtual float GetElevation(int iX, int iZ, bool bTrue = false) const = 0;
	virtual void GetWorldLocation(int i, int j, FPoint3 &loc, bool bTrue = false) const = 0;

	bool ColorDibFromElevation(vtBitmapBase *pBM, const ColorMap *cmap,
		void progress_callback(int) = NULL);
	void ShadeDibFromElevation(vtBitmapBase *pBM, const FPoint3 &light_dir,
							   float light_factor, void progress_callback(int) = NULL);
	void ShadeQuick(vtBitmapBase *pBM, float light_factor, void progress_callback(int) = NULL);
	void ShadowCastDib(vtBitmapBase *pBM, const FPoint3 &ight_dir,
		float light_factor, void progress_callback(int) = NULL);

protected:
	int		m_iColumns, m_iRows;
	float	m_fXStep, m_fZStep;	// step size between the World grid points
	double	m_dXStep, m_dYStep;	// step size between the Earth grid points
};

#endif	// HEIGHTFIELDH


