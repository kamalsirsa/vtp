
#ifndef HEIGHTFIELDH
#define HEIGHTFIELDH

#include "vtdata/Projections.h"
#include "LocalProjection.h"
#include "LocalGrid.h"

class vtHeightField
{
public:
	void Initialize(vtLocalGrid *pLocalGrid);

	virtual bool FindAltitudeAtPoint(FPoint3 &p3, float &fAltitude, FPoint3 *vNormal = NULL) = 0;

	bool PointAboveTerrain(FPoint3 &p1);
	bool CastRayToSurface(FPoint3 &point, FPoint3 &dir, FPoint3 &result);

	void ConvertEarthToSurfacePoint(double utm_x, double utm_y, FPoint3 &p3);
	void ConvertEarthToSurfacePoint(const DPoint2 &epos, FPoint3 &p3)
	{
		ConvertEarthToSurfacePoint(epos.x, epos.y, p3);
	}
	bool PointIsInTerrain(float x, float z);
	void GetCenter(FPoint3 &center);

	// Accessors
	void GetEarthExtents(DRECT &ext) { ext = m_EarthExtents; }
	void GetWorldExtents(FRECT &ext) { ext = m_WorldExtents; }
	void GetWorldSize(FPoint2 &size) { size = m_WorldSize; }

	// minimum and maximum height values for the whole grid
	float	m_fMinHeight, m_fMaxHeight;

	vtProjection &GetProjection() { return m_proj; }

protected:
	int		m_iXPoints, m_iYPoints;	// height field grid dimensions

	DRECT	m_EarthExtents;		// raw extents in the given projection
								// (lon/lat or UTM x/y)
	FRECT	m_WorldExtents;		// cooked (OpenGL) extents (in the XZ plane)
	FPoint2	m_WorldSize;		// size of WorldExtents
	float	m_fXStep, m_fZStep;	// step size between the World grid points
	float	m_fDiagonalLength;

	vtProjection	m_proj;
};

#endif
