//
// vtElevationGrid.h
//
// Copyright (c) 2001 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef ELEVATIONGRIDH
#define ELEVATIONGRIDH

#include <limits.h>			// for SHRT_MIN
#include "MathTypes.h"
#include "Projections.h"

class vtDIB;

#define INVALID_ELEVATION	SHRT_MIN

/**  The vtElevationGrid class represents a generic grid of elevation data.
 * It supports reading and writing the data from several file formats.
 * \par
 * Height elements ("heixels") can be either integer (2 bytes) or floating
 * point (4 bytes).  Heixel values are always in meters.  Areas of unknown
 * elevation are represented by the value INVALID_ELEVATION.
 * \par
 * To load a grid from a file, first create an empty grid, then call the
 * appropriated Load method.
 */
class vtElevationGrid
{
public:
	vtElevationGrid();
	vtElevationGrid(DRECT area, int iColumns, int iRows, bool bFloat, vtProjection proj);
	~vtElevationGrid();

	bool ConvertProjection(vtElevationGrid *pOld, vtProjection &NewProj, void progress_callback(int) = NULL);

	// Load
	bool LoadFromDEM(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromASC(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromTerragen(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromCDF(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromDTED(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromGTOPO30(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromGRD(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromPGM(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadFromRAW(const char *szFileName, int width,	int height,
		int bytes_per_element, float vertical_units);
	bool LoadFromBT(const char *szFileName, void progress_callback(int) = NULL);
	bool LoadBTHeader(const char *szFileName);

	// Use GDAL to read a file
	bool LoadWithGDAL(const char *szFileName, void progress_callback(int));

	// Save
	bool SaveToTerragen(const char *szFileName);
	bool SaveToBT(const char *szFileName, void progress_callback(int) = NULL);

	void ComputeHeightExtents();
	void GetHeightExtents(float &fMinHeight, float &fMaxHeight);
	void GetDimensions(int &nColumns, int &nRows);
	DPoint2 GetSpacing();

	/** Test if a point is within the extents of the grid.
	 */
	bool ContainsPoint(float x, float y)
	{
		return (m_area.left < x && x < m_area.right &&
				m_area.bottom < y && y < m_area.top);
	}

	// Set/Get height values
	void  SetFValue(int i, int j, float value);
	void  SetValue(int i, int j, short value);
	int   GetValue(int i, int j);	// returns height value as a integer
	float GetFValue(int i, int j);	// returns height value as a float

	float GetClosestValue(double x, double y);
	float GetFilteredValue(double x, double y);

	// Accessors
	/** Return the embedded name of the DEM is it has one */
	char *GetDEMName()	{ return m_szOriginalDEMName; }

	/** Return geographic extents of the grid. */
	DRECT &GetGridExtents()	{ return m_area; }

	/** Return geographic extents of the *area* covered by grid. */
	DRECT GetAreaExtents();

	/** Set the geographic extents of the grid. */
	void SetGridExtents(DRECT &ext)	{ m_area = ext; }

	/** Get the data size of the grid: \c true if floating point (4-byte),
	 * \c false if integer (2-byte).
	 */
	bool  IsFloatMode()	{ return m_bFloatMode; }

	void ColorDibFromElevation1(vtDIB *pDIB, RGBi color_ocean);

	vtProjection &GetProjection() { return m_proj; }
	void SetProjection(vtProjection &proj) { m_proj = proj; }

	bool GetCorners(DLine2 &line, bool bGeo);
	void SetCorners(const DLine2 &line);

protected:
	DRECT	m_area;		// bounds in the original data space
	int		m_iColumns;
	int		m_iRows;
	bool	m_bFloatMode;
	short	*m_pData;
	float	*m_pFData;

	void ComputeExtentsFromCorners();
	void ComputeCornersFromExtents();

	DPoint2		m_Corners[4];		// data corners, in the projection of this terrain

	vtProjection	m_proj;

private:
	// minimum and maximum height values for the whole grid
	float	m_fMinHeight, m_fMaxHeight;
	char 	m_szOriginalDEMName[41];

	void	AllocateArray();

	// only used by DEM reader code
	double	m_fVMeters;
	double	m_fGRes;
	int		m_iDataSize;
};

#endif	// ELEVATIONGRIDH

