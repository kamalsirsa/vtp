//
// vtElevationGrid.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include <fstream>
using namespace std;

#include "ElevationGrid.h"
#include "ByteOrder.h"
#include "vtDIB.h"

const float meters_per_latitude = 111300.0f;

//////////////////////////////////////////////////

/**
 * Constructor: Creates an empty grid.
 */
vtElevationGrid::vtElevationGrid()
{
	m_bFloatMode = false;
	m_pData = NULL;
	m_pFData = NULL;
	m_fVMeters = 1.0f;

	for (int i = 0; i < 4; i++)
		m_Corners[i].Set(0, 0);

	m_fMinHeight = m_fMaxHeight = INVALID_ELEVATION;
	m_fVerticalScale = 1.0f;
}

/**
 * Constructor: Creates a grid of given size.
 *
 * \param area the coordinate extents of the grid (rectangular area)
 * \param iColumns number of columns in the grid (east-west)
 * \param iRows number of rows (north-south)
 * \param bFloat data size: \c true to use floating-point, \c false for shorts.
 * \param proj the geographical projection to use.
 *
 * The grid will initially have no data in it (all values are INVALID_ELEVATION).
 */
vtElevationGrid::vtElevationGrid(const DRECT &area, int iColumns, int iRows,
	bool bFloat, const vtProjection &proj)
{
	vtHeightFieldGrid3d::Initialize(proj.GetUnits(), area, INVALID_ELEVATION,
		INVALID_ELEVATION, iColumns, iRows);

	m_bFloatMode = bFloat;
	_AllocateArray();

	ComputeCornersFromExtents();

	m_proj = proj;
	m_fVMeters = 1.0f;
	m_fVerticalScale = 1.0f;
}

/**
 * Copy constructor.
 */
vtElevationGrid::vtElevationGrid(const vtElevationGrid &rhs)
{
	*this = rhs;
}

/**
 * Assignment operator.
 * \return	*this with new values
 */
vtElevationGrid & vtElevationGrid::operator=(const vtElevationGrid &rhs)
{
	if (this == &rhs)
		return *this;

	if (m_pData)
		free(m_pData);

	if (m_pFData)
		free(m_pFData);

	m_EarthExtents = rhs.m_EarthExtents;
	m_iColumns   = rhs.m_iColumns;
	m_iRows		 = rhs.m_iRows;
	m_bFloatMode = rhs.m_bFloatMode;
	m_fVMeters   = rhs.m_fVMeters;

	for (unsigned ii = 0; ii < sizeof( m_Corners ) / sizeof( *m_Corners ); ++ii)
		m_Corners[ii] = rhs.m_Corners[ii];

	m_proj = rhs.m_proj;

	m_fMinHeight = rhs.m_fMinHeight;
	m_fMaxHeight = rhs.m_fMaxHeight;

	m_strOriginalDEMName = rhs.m_strOriginalDEMName;

	if ( m_bFloatMode )
	{
		m_pData = NULL;

		if ( rhs.m_pFData )
		{
			size_t Size = m_iColumns * m_iRows * sizeof(float);

			m_pFData = (float *)malloc( Size );

			memcpy( m_pFData, rhs.m_pFData, Size );
		}
		else
			m_pFData = NULL;
	}
	else
	{
		m_pFData = NULL;

		if ( rhs.m_pData )
		{
			size_t Size = m_iColumns * m_iRows * sizeof(short);

			m_pData = (short *)malloc(Size);

			memcpy( m_pData, rhs.m_pData, Size );
		}
		else
			m_pData = NULL;
	}
	return *this;
}


/**
 * Destructor
 */
vtElevationGrid::~vtElevationGrid()
{
	if (m_pData)
		free(m_pData);
	m_pData = NULL;
	if (m_pFData)
		free(m_pFData);
	m_pFData = NULL;
}


// helper
double MetersPerLongitude(double latitude)
{
	return meters_per_latitude * cos(latitude / 180.0 * PId);
}


/**
 * Initializes an elevation grid by converting the contents of an another
 * grid to a new projection.
 *
 * \param pOld		An existing elevation grid to convert from.
 * \param NewProj	The new projection to convert to.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 *
 * \return True if successful.
 */
bool vtElevationGrid::ConvertProjection(vtElevationGrid *pOld,
	const vtProjection &NewProj, bool progress_callback(int))
{
	int i, j;

	// Create conversion object
	const vtProjection *pSource, *pDest;
	pSource = &pOld->GetProjection();
	pDest = &NewProj;

	OCT *trans = CreateCoordTransform(pSource, pDest);
	if (!trans)
	{
		// inconvertible projections
		return false;
	}

	// find where the extent corners are going to be in the new terrain
	int success;
	for (i = 0; i < 4; i++)
	{
		DPoint2 point = pOld->m_Corners[i];
		success = trans->Transform(1, &point.x, &point.y);
		if (success == 0)
		{
			// inconvertible projections
			delete trans;
			return false;
		}
		m_Corners[i] = point;
	}
	ComputeExtentsFromCorners();
	delete trans;

	// now, how large an array will we need for the new terrain?
	// try to preserve the sampling rate approximately
	//
	bool bOldGeo = (pSource->IsGeographic() != 0);
	bool bNewGeo = (pDest->IsGeographic() != 0);

	DPoint2 old_step = pOld->GetSpacing();
	DPoint2 new_step;
	double meters_per_longitude;

	if (bOldGeo && !bNewGeo)
	{
		// convert degrees to meters (approximately)
		meters_per_longitude = MetersPerLongitude(pOld->m_Corners[0].y);
		new_step.x = old_step.x * meters_per_longitude;
		new_step.y = old_step.y * meters_per_latitude;
	}
	else if (!bOldGeo && bNewGeo)
	{
		// convert meters to degrees (approximately)
		meters_per_longitude = MetersPerLongitude(m_Corners[0].y);
		new_step.x = old_step.x / meters_per_longitude;
		new_step.y = old_step.y / meters_per_latitude;	// convert degrees to meters (approximately)
	}
	else
	{
		// check horizontal units or old and new terrain
		double units_old = pSource->GetLinearUnits(NULL);
		double units_new = pDest->GetLinearUnits(NULL);
		new_step = old_step * (units_old / units_new);
	}
	double fColumns = m_EarthExtents.Width() / new_step.x;
	double fRows = m_EarthExtents.Height() / new_step.y;

	// round up to the nearest integer
	m_iColumns = (int)(fColumns + 0.999);
	m_iRows = (int)(fRows + 0.999);

	// do safety checks
	if (m_iColumns < 1 || m_iRows < 1)
		return false;

	if (m_iColumns > 40000 || m_iRows > 40000)
		return false;

	// Now we're ready to fill in the new elevationgrid.
	// Some fields are simple to set:
	m_proj = NewProj;
	m_bFloatMode = pOld->m_bFloatMode;
	m_strOriginalDEMName = pOld->GetDEMName();

	// Others are on the parent class:
	vtHeightFieldGrid3d::Initialize(NewProj.GetUnits(), m_EarthExtents, INVALID_ELEVATION,
		INVALID_ELEVATION, m_iColumns, m_iRows);
	_AllocateArray();

	// Convert each bit of data from the old array to the new
	// Transformation points backwards, from the target to the source
	trans = CreateCoordTransform(pDest, pSource);
	if (!trans)
	{
		// inconvertible projections
		return false;
	}
	DPoint2 p, step = GetSpacing();
	float value;
	for (i = 0; i < m_iColumns; i++)
	{
		if (progress_callback != NULL) progress_callback(i*100/m_iColumns);

		for (j = 0; j < m_iRows; j++)
		{
			p.x = m_EarthExtents.left + i * step.x;
			p.y = m_EarthExtents.bottom + j * step.y;

			// Since transforming the extents succeeded, it's safe to assume
			// that the points will also transform without errors.
			trans->Transform(1, &p.x, &p.y);

			value = pOld->GetFilteredValue(p);
			SetFValue(i, j, value);
		}
	}
	delete trans;
	ComputeHeightExtents();
	return true;
}

/**
 * Reprojects an elevation grid by converting just the extents to a new
 * projection.
 *
 * This is much faster than creating a new grid and reprojecting every
 * heixel, but it only produces correct results when the difference
 * between the projections is only a horizontal shift.  For example, this
 * occurs when the only difference between the old and new projection
 * is choice of Datum.
 *
 * \param proj_new	The new projection to convert to.
 *
 * \return True if successful.
 */
bool vtElevationGrid::ReprojectExtents(const vtProjection &proj_new)
{
	// Create conversion object
	const vtProjection *pSource, *pDest;
	pSource = &m_proj;
	pDest = &proj_new;

	OCT *trans = CreateCoordTransform(pSource, pDest);
	if (!trans)
	{
		// inconvertible projections
		return false;
	}
	int i, success;
	for (i = 0; i < 4; i++)
	{
		DPoint2 point = m_Corners[i];
		success = trans->Transform(1, &point.x, &point.y);
		if (success == 0)
		{
			// inconvertible projections
			delete trans;
			return false;
		}
		m_Corners[i] = point;
	}
	ComputeExtentsFromCorners();
	delete trans;

	m_proj = proj_new;

	return true;
}

/**
 * Scale all the valid elevation values in the grid by a given factor.
 *
 * \param fScale	The desired scale, e.g. 1.0 produces no change in scale.
 * \param bDirect	If true, scale the stored height values directly.
 *		Otherwise, only the height scale (vertical meters per unit) is scaled.
 * \param bRecomputeExtents  If true, recompute the height extents.
 *		Default is true.
 */
void vtElevationGrid::Scale(float fScale, bool bDirect, bool bRecomputeExtents)
{
	if (!bDirect)
	{
		m_fVMeters *= fScale;
	}
	else
	{
		int i, j;
		float f;
		for (i = 0; i < m_iColumns; i++)
		{
			for (j = 0; j < m_iRows; j++)
			{
				f = GetFValue(i, j);
				if (f != INVALID_ELEVATION)
					SetFValue(i, j, f * fScale);
			}
		}
	}
	if (bRecomputeExtents)
		ComputeHeightExtents();
}


/**
 * Scans the grid to compute the minimum and maximum height values.
 * \sa GetHeightExtents
 */
void vtElevationGrid::ComputeHeightExtents()
{
	m_fMinHeight = 100000.0f;
	m_fMaxHeight = -100000.0f;

	int i, j;
	for (i=0; i<m_iColumns; i++)
	{
		for (j=0; j<m_iRows; j++)
		{
			float value = GetFValue(i, j);
			if (value == INVALID_ELEVATION)
				continue;
			if (value > m_fMaxHeight) m_fMaxHeight = value;
			if (value < m_fMinHeight) m_fMinHeight = value;
		}
	}
}

/**
 * Offset the entire elevation grid horizontally.
 * \param delta The X,Y amount to shift the location of the grid.
 */
void vtElevationGrid::Offset(const DPoint2 &delta)
{
	// Shifting an elevation is as easy as shifting its extents
	m_EarthExtents.left += delta.x;
	m_EarthExtents.right += delta.x;
	m_EarthExtents.top += delta.y;
	m_EarthExtents.bottom += delta.y;

	// Also the corners, which are mantained in parallel
	for (int i = 0; i < 4; i++)
		m_Corners[i] += delta;
}

void vtElevationGrid::ReplaceValue(float value1, float value2)
{
	bool bModified = false;
	int i, j;
	for (i=0; i<m_iColumns; i++)
	{
		for (j=0; j<m_iRows; j++)
		{
			if (GetFValue(i, j) == value1)
				SetFValue(i, j, value2);
			bModified = true;
		}
	}
	if (bModified)
		ComputeHeightExtents();
}

/** Set an elevation value to the grid.
 * \param i, j Column and row location in the grid.
 * \param value The value in (integer) meters.
 */
void vtElevationGrid::SetValue(int i, int j, short value)
{
	assert(i >= 0 && i < m_iColumns);
	assert(j >= 0 && j < m_iRows);
	if (m_bFloatMode)
	{
		if (m_fVMeters == 1.0f || value == INVALID_ELEVATION)
			m_pFData[i*m_iRows+j] = (float)value;
		else
			m_pFData[i*m_iRows+j] = (float)value / m_fVMeters;
	}
	else
	{
		if (m_fVMeters == 1.0f || value == INVALID_ELEVATION)
			m_pData[i*m_iRows+j] = value;
		else
			m_pData[i*m_iRows+j] = (short) ((float)value / m_fVMeters);
	}
}

/** Set an elevation value to the grid.
 * \param i, j Column and row location in the grid.
 * \param value The value in (float) meters.
 */
void vtElevationGrid::SetFValue(int i, int j, float value)
{
	assert(i >= 0 && i < m_iColumns);
	assert(j >= 0 && j < m_iRows);
	if (m_bFloatMode)
	{
		if (m_fVMeters == 1.0f || value == INVALID_ELEVATION)
			m_pFData[i*m_iRows+j] = value;
		else
			m_pFData[i*m_iRows+j] = value / m_fVMeters;
	}
	else
	{
		if (m_fVMeters == 1.0f || value == INVALID_ELEVATION)
			m_pData[i*m_iRows+j] = (short) value;
		else
			m_pData[i*m_iRows+j] = (short) (value / m_fVMeters);
	}
}

/** Get an elevation value from the grid.
 * \param i, j Column and row location in the grid.
 * \return The value in (integer) meters.
 */
short vtElevationGrid::GetValue(int i, int j) const
{
	if (m_bFloatMode)
	{
		float value = m_pFData[i*m_iRows+j];
		if (m_fVMeters == 1.0f || value == INVALID_ELEVATION)
			return (short) value;
		else
			return (short) (value * m_fVMeters);
	}
	short svalue = m_pData[i*m_iRows+j];
	if (m_fVMeters == 1.0f || svalue == INVALID_ELEVATION)
		return svalue;
	else
		return (short) ((float)svalue * m_fVMeters);
}

/** Get an elevation value from the grid.
 * \param i, j Column and row location in the grid.
 * \return The value in (float) meters.
 */
float vtElevationGrid::GetFValue(int i, int j) const
{
	if (m_bFloatMode)
	{
		float value = m_pFData[i*m_iRows+j];
		if (m_fVMeters == 1.0f || value == INVALID_ELEVATION)
			return value;
		else
			return value * m_fVMeters;
	}
	short svalue = m_pData[i*m_iRows+j];
	if (m_fVMeters == 1.0f || svalue == INVALID_ELEVATION)
		return (float) svalue;
	else
		return ((float)svalue * m_fVMeters);
}


/** For a grid whose 4 corners coordinates are known, use
 * those corners to imply absolute extents.
 */
void vtElevationGrid::ComputeExtentsFromCorners()
{
	// find absolute extents of corner coordinates
	m_EarthExtents.left = min(m_Corners[0].x, m_Corners[1].x);
	m_EarthExtents.right = max(m_Corners[2].x, m_Corners[3].x);
	m_EarthExtents.bottom = min(m_Corners[0].y, m_Corners[3].y);
	m_EarthExtents.top = max(m_Corners[1].y, m_Corners[2].y);
}


/** For a grid whose absolute extents are known, use
 * those extents to imply the 4 corners coordinates.
 */
void vtElevationGrid::ComputeCornersFromExtents()
{
	m_Corners[0].x = m_EarthExtents.left;
	m_Corners[0].y = m_EarthExtents.bottom;
	m_Corners[1].x = m_EarthExtents.left;
	m_Corners[1].y = m_EarthExtents.top;
	m_Corners[2].x = m_EarthExtents.right;
	m_Corners[2].y = m_EarthExtents.top;
	m_Corners[3].x = m_EarthExtents.right;
	m_Corners[3].y = m_EarthExtents.bottom;
}


//
// Allocates a data array big enough to contain the grid data.
//
void vtElevationGrid::_AllocateArray()
{
	if (m_bFloatMode)
	{
		m_pData = NULL;
		m_pFData = (float *)malloc(m_iColumns * m_iRows * sizeof(float));
	}
	else
	{
		m_pData = (short *)malloc(m_iColumns * m_iRows * sizeof(short));
		m_pFData = NULL;
	}
	// Initially no data
	FillWithSingleValue(INVALID_ELEVATION);
}

void vtElevationGrid::FillWithSingleValue(float fValue)
{
	int i, j;
	if (m_bFloatMode)
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				SetFValue(i, j, fValue);
	}
	else
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				SetValue(i, j, (short) fValue);
	}
	m_fMinHeight = fValue;
	m_fMaxHeight = fValue;
}

void vtElevationGrid::GetEarthLocation(int i, int j, DPoint3 &loc) const
{
	DPoint2 spacing = GetSpacing();
	loc.Set(m_EarthExtents.left + i * spacing.x,
		m_EarthExtents.bottom + j * spacing.y,
		GetFValue(i, j));
}

/**
 * Get the height of the grid at a specific world coordinate.
 *
 * The value of the gridpoint closest to the specified location is returned.
 * If the location is not within the extents of the grid, INVALID_ELEVATION is returned.
 * \param p	The point to query.
 */
float vtElevationGrid::GetClosestValue(const DPoint2 &p) const
{
	int ix = (int)((p.x - m_EarthExtents.left) / m_EarthExtents.Width() * m_iColumns);
	int iy = (int)((p.y - m_EarthExtents.bottom) / m_EarthExtents.Height() * m_iRows);
	if (ix >= 0 && ix < m_iColumns && iy >= 0 && iy < m_iRows)
		return GetFValue(ix, iy);
	else
		return INVALID_ELEVATION;
}

/**
 * Get the interpolated height of the grid at a specific world coordinate.
 *
 * The value is linearly interpolated between the surrounding gridpoints.
 * If the location is not within the extents of the grid, INVALID_ELEVATION is returned.
 * \param p	The point to query.
 */
float vtElevationGrid::GetFilteredValue(const DPoint2 &p) const
{
	// what data point in t is closest to (x,y)?
	double local_x = (p.x - m_EarthExtents.left) / (m_EarthExtents.Width());
	double local_y = (p.y - m_EarthExtents.bottom) / (m_EarthExtents.Height());

	int index_x = (int) (local_x * (m_iColumns-1) + 0.0000000001);
	if (index_x < 0 || index_x >= m_iColumns)
		return INVALID_ELEVATION;

	int index_y = (int) (local_y * (m_iRows-1) + 0.0000000001);
	if (index_y < 0 || index_y >= m_iRows)
		return INVALID_ELEVATION;

	double findex_x = local_x * (m_iColumns-1);
	double findex_y = local_y * (m_iRows-1);

	double fData;
	float fDataBL, fDataTL, fDataTR, fDataBR;
	if (index_x == m_iColumns-1)
	{
		if (index_y == m_iRows-1)
		{
			// far corner, no interpolation
			fData = GetFValue(index_x, index_y);
		}
		// right edge - interpolate north-south
		fDataBL = GetFValue(index_x, index_y);
		fDataTL = GetFValue(index_x, index_y+1);
		if (fDataBL == INVALID_ELEVATION || fDataTL == INVALID_ELEVATION)
			return INVALID_ELEVATION;
		double diff_y = findex_y - index_y;
		fData = fDataBL + (fDataTL - fDataBL) * diff_y;
	}
	else if (index_y == m_iRows-1)
	{
		// top edge - interpolate east-west
		fDataBL = GetFValue(index_x, index_y);
		fDataBR = GetFValue(index_x+1, index_y);
		if (fDataBL == INVALID_ELEVATION || fDataBR == INVALID_ELEVATION)
			return INVALID_ELEVATION;
		double diff_x = findex_x - index_x;
		fData = fDataBL + (fDataBR - fDataBL) * diff_x;
	}
	else // do bilinear filtering
	{
		double diff_x = findex_x - index_x;
		double diff_y = findex_y - index_y;
		// catch numerical roundoff, diff must be [0..1]
		if (diff_x < 0)
			diff_x = 0;
		if (diff_y < 0)
			diff_y = 0;
		fDataBL = GetFValue(index_x, index_y);
		fDataBR = GetFValue(index_x+1, index_y);
		fDataTL = GetFValue(index_x, index_y+1);
		fDataTR = GetFValue(index_x+1, index_y+1);
		if ((fDataBL != INVALID_ELEVATION) &&
				(fDataBR != INVALID_ELEVATION) &&
				(fDataTL != INVALID_ELEVATION) &&
				(fDataTR != INVALID_ELEVATION))
		{
			fData = fDataBL + (fDataBR-fDataBL)*diff_x +
				(fDataTL-fDataBL)*diff_y +
				(fDataTR-fDataTL-fDataBR+fDataBL)*diff_x*diff_y;
		}
		else
			fData = INVALID_ELEVATION;
	}
	return (float) fData;
}

float vtElevationGrid::GetFValueSafe(int i, int j) const
{
	if (i < 0 || i > m_iColumns-1 || j < 0 || j > m_iRows-1)
		return INVALID_ELEVATION;
	float fData = GetFValue(i, j);
	return fData;
}

/**
 * Get the interpolated height of the grid at a specific world coordinate.
 * This method is more liberal in regards to finding a valid data point
 * among undefined data than GetFilteredValue()
 */
float vtElevationGrid::GetFilteredValue2(const DPoint2 &p) const
{
	float fData;

	// Quickly reject points well outside the greatest possible extents of
	//  this grid, for speed.
	if (GetAreaExtents().ContainsPoint(p) == false)
		return INVALID_ELEVATION;

	// simple case, within the precise extents
	if (m_EarthExtents.ContainsPoint(p, true))
	{
		fData = GetFilteredValue(p);
		if (fData != INVALID_ELEVATION)
			return fData;
	}

	// what data point in t is closest to (x,y)?
	double local_x = (p.x - m_EarthExtents.left) / (m_EarthExtents.Width());
	double local_y = (p.y - m_EarthExtents.bottom) / (m_EarthExtents.Height());

	int index_x = (int) (local_x * (m_iColumns-1) + 0.0000000001);
	int index_x2 = (int) (local_x * (m_iColumns-1) + 0.5);
	if (index_x2 < 0 || index_x2 > m_iColumns)
		return INVALID_ELEVATION;

	int index_y = (int) (local_y * (m_iRows-1) + 0.0000000001);
	int index_y2 = (int) (local_y * (m_iRows-1) + 0.5);
	if (index_y2 < 0 || index_y2 > m_iRows)
		return INVALID_ELEVATION;

	double findex_x = local_x * (m_iColumns-1);
	double findex_y = local_y * (m_iRows-1);

	float fDataBL, fDataTL, fDataTR, fDataBR;

	int valid = 0;
	float sum = 0.0f;
	fDataBL = GetFValueSafe(index_x, index_y);
	fDataBR = GetFValueSafe(index_x+1, index_y);
	fDataTL = GetFValueSafe(index_x, index_y+1);
	fDataTR = GetFValueSafe(index_x+1, index_y+1);

	if (fDataBL != INVALID_ELEVATION)
	{
		sum += fDataBL;
		valid++;
	}
	if (fDataBR != INVALID_ELEVATION)
	{
		sum += fDataBR;
		valid++;
	}
	if (fDataTL != INVALID_ELEVATION)
	{
		sum += fDataTL;
		valid++;
	}
	if (fDataTR != INVALID_ELEVATION)
	{
		sum += fDataTR;
		valid++;
	}
	if (valid == 4)	// all valid
	{
		// do bilinear filtering
		double diff_x = findex_x - index_x;
		double diff_y = findex_y - index_y;
		// catch numerical roundoff, diff must be [0..1]
		if (diff_x < 0)
			diff_x = 0;
		if (diff_y < 0)
			diff_y = 0;
		fData = (float) (fDataBL + (fDataBR-fDataBL)*diff_x +
				(fDataTL-fDataBL)*diff_y +
				(fDataTR-fDataTL-fDataBR+fDataBL)*diff_x*diff_y);
	}
	else if (valid == 3 || valid == 2)
	{
		// do average; it's better than nothing
		fData = sum / valid;
	}
	else
		fData = INVALID_ELEVATION;

	return fData;
}

DRECT vtElevationGrid::GetAreaExtents() const
{
	return DRECT(m_EarthExtents.left - (m_dXStep / 2.0f),
		m_EarthExtents.top + (m_dYStep / 2.0f),
		m_EarthExtents.right + (m_dXStep / 2.0f),
		m_EarthExtents.bottom - (m_dYStep / 2.0f));
}

bool vtElevationGrid::GetCorners(DLine2 &line, bool bGeo) const
{
	int i;

	if (!bGeo || m_proj.IsGeographic())
	{
		// no need to convert
		for (i = 0; i < 4; i++)
			line.SetAt(i, m_Corners[i]);
	}
	else
	{
		// must convert from whatever we are, to geo
		vtProjection Dest;
		Dest.SetWellKnownGeogCS("WGS84");

		// safe (won't fail on tricky Datum conversions)
		OCT *trans = CreateConversionIgnoringDatum(&m_proj, &Dest);

		// unsafe, but potentially more accurate
//		OCT *trans = CreateCoordTransform(&m_proj, &Dest, true);

		if (!trans)
		{
			// inconvertible projections
			return false;
		}
		for (i = 0; i < 4; i++)
		{
			DPoint2 p = m_Corners[i];
			trans->Transform(1, &p.x, &p.y);
			line.SetAt(i, p);
		}
		delete trans;
	}
	return true;
}

void vtElevationGrid::SetCorners(const DLine2 &line)
{
	for (int i = 0; i < 4; i++)
		m_Corners[i] = line[i];
}

void vtElevationGrid::SetupConversion(float fVerticalExag)
{
	if (m_fMinHeight == INVALID_ELEVATION ||
		m_fMaxHeight == INVALID_ELEVATION)
	{
		// we need height extents, so force them to be computed
		ComputeHeightExtents();
	}

	// initialize parent class
	Initialize(m_proj.GetUnits(), m_EarthExtents, m_fMinHeight, m_fMaxHeight,
		m_iColumns, m_iRows);

	m_fVerticalScale = fVerticalExag;
}

float vtElevationGrid::GetElevation(int iX, int iZ, bool bTrue) const
{
	// we ignore bTrue because this class always stores true elevation
	return GetFValue(iX, iZ);
}

void vtElevationGrid::GetWorldLocation(int i, int j, FPoint3 &loc, bool bTrue) const
{
	if (bTrue)
	{
		loc.Set(m_WorldExtents.left + i * m_fXStep,
			GetFValue(i,j),
			m_WorldExtents.bottom - j * m_fZStep);
	}
	else
	{
		loc.Set(m_WorldExtents.left + i * m_fXStep,
			GetFValue(i,j) * m_fVerticalScale,
			m_WorldExtents.bottom - j * m_fZStep);
	}
}

float vtElevationGrid::GetWorldValue(int i, int j, bool bTrue) const
{
	if (bTrue)
		return GetFValue(i, j);
	else
		return GetFValue(i, j) * m_fVerticalScale;
}


/**
 * Quick n' dirty special-case raycast for perfectly regular grid terrain
 * Find altitude (y) and surface normal, given (x,z) local coordinates
 *
 * This approach is very straightforward, so it could be significantly
 * sped up if needed.
 */
bool vtElevationGrid::FindAltitudeAtPoint(const FPoint3 &p, float &fAltitude,
	bool bTrue, FPoint3 *vNormal) const
{
	int iX = (int)((p.x - m_WorldExtents.left) / m_fXStep);
	int iZ = (int)((p.z - m_WorldExtents.bottom) / -m_fZStep);

	// safety check
	if (iX < 0 || iX >= m_iColumns-1 || iZ < 0 || iZ >= m_iRows-1)
	{
		if (p.x == m_WorldExtents.right || p.z == m_WorldExtents.top)
		{
			// right on the edge: allow this point, but don't interpolate
			fAltitude = GetFValue(iX, iZ);
			if (!bTrue)
				fAltitude *= m_fVerticalScale;
			if (vNormal != NULL)
				vNormal->Set(0,1,0);
			return true;
		}
		else
		{
			fAltitude = 0.0f;
			if (vNormal) vNormal->Set(0.0f, 1.0f, 0.0f);
			return false;
		}
	}

	if (vNormal != NULL)
	{
		FPoint3 p0, p1, p2, p3;
		GetWorldLocation(iX, iZ, p0, bTrue);
		GetWorldLocation(iX+1, iZ, p1, bTrue);
		GetWorldLocation(iX+1, iZ+1, p2, bTrue);
		GetWorldLocation(iX, iZ+1, p3, bTrue);

		// find fractional amount (0..1 across quad)
		float fX = (p.x - p0.x) / m_fXStep;
		float fZ = (p.z - p0.z) / -m_fZStep;

		// which of the two triangles in the quad is it?
		if (fX + fZ < 1)
		{
			fAltitude = p0.y + fX * (p1.y - p0.y) + fZ * (p3.y - p0.y);

			// find normal also
			FPoint3 edge0 = p1 - p0;
			FPoint3 edge1 = p3 - p0;
			*vNormal = edge0.Cross(edge1);
			vNormal->Normalize();
		}
		else
		{
			fAltitude = p2.y + (1.0f-fX) * (p3.y - p2.y) + (1.0f-fZ) * (p1.y - p2.y);

			// find normal also
			FPoint3 edge0 = p3 - p2;
			FPoint3 edge1 = p1 - p2;
			*vNormal = edge0.Cross(edge1);
			vNormal->Normalize();
		}
	}
	else
	{
		// It's faster to simpler to operate only the elevations, if we don't
		//  need to compute a normal vector.
		float alt0 = GetFValue(iX, iZ);
		float alt1 = GetFValue(iX+1, iZ);
		float alt2 = GetFValue(iX+1, iZ+1);
		float alt3 = GetFValue(iX, iZ+1);

		// find fractional amount (0..1 across quad)
		float fX = (p.x - (m_WorldExtents.left + iX * m_fXStep)) / m_fXStep;
		float fY = (p.z - (m_WorldExtents.bottom - iZ * m_fZStep)) / -m_fZStep;

		// which of the two triangles in the quad is it?
		if (fX + fY < 1)
			fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt3 - alt0));
		else
			fAltitude = (float) (alt2 + (1.0f-fX) * (alt3 - alt2) + (1.0f-fY) * (alt1 - alt2));

		if (!bTrue)
			fAltitude *= m_fVerticalScale;
	}
	return true;
}

void vtElevationGrid::GetChecksum(unsigned char **ppChecksum) const
{
	// TODO
}

bool vtElevationGrid::FindAltitudeAtPoint2(const DPoint2 &p, float &fAltitude,
										   bool bTrue) const
{
	// we ignore bTrue because this class always stores true elevation
	DPoint2 spacing = GetSpacing();
	int iX = (int)((p.x - m_EarthExtents.left) / spacing.x);
	int iY = (int)((p.y - m_EarthExtents.bottom) / spacing.y);

	// safety check
	if (iX < 0 || iX >= m_iColumns-1 || iY < 0 || iY >= m_iRows-1)
	{
		fAltitude = 0.0f;
		return false;
	}

	float alt0 = GetFValue(iX, iY);
	float alt1 = GetFValue(iX+1, iY);
	float alt2 = GetFValue(iX+1, iY+1);
	float alt3 = GetFValue(iX, iY+1);

	// find fractional amount (0..1 across quad)
	double fX = (p.x - (m_EarthExtents.left + iX * spacing.x)) / spacing.x;
	double fY = (p.y - (m_EarthExtents.bottom + iY * spacing.y)) / spacing.y;

	// which of the two triangles in the quad is it?
	if (fX + fY < 1)
		fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt3 - alt0));
	else
		fAltitude = (float) (alt2 + (1.0-fX) * (alt3 - alt2) + (1.0-fY) * (alt1 - alt2));

	return true;
}

