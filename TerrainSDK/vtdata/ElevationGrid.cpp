//
// vtElevationGrid.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>

#include "ElevationGrid.h"
#include "ByteOrder.h"
#include "vtDIB.h"
#include "vtLog.h"

//////////////////////////////////////////////////

/**
 * Constructor: Creates an empty grid.
 */
vtElevationGrid::vtElevationGrid()
{
	SetupMembers();
}

void vtElevationGrid::SetupMembers()
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
	SetupMembers();
	Create(area, iColumns, iRows, bFloat, proj);
}

/**
 * Copy constructor.
 */
vtElevationGrid::vtElevationGrid(const vtElevationGrid &rhs)
{
	SetupMembers();
	*this = rhs;
}

/**
 * Assignment operator.
 * \return	*this with new values
 */
vtElevationGrid & vtElevationGrid::operator=(const vtElevationGrid &rhs)
{
	if (this != &rhs)
		CopyFrom(rhs);
	return *this;
}

bool vtElevationGrid::CopyFrom(const vtElevationGrid &rhs)
{
	if (!CopyHeaderFrom(rhs))
		return false;
	if (!CopyDataFrom(rhs))
		return false;
	return true;
}

bool vtElevationGrid::CopyHeaderFrom(const vtElevationGrid &rhs)
{
	// Free data first before copying from other grid
	FreeData();

	// Copy each vtHeightField member
	m_fMinHeight = rhs.m_fMinHeight;
	m_fMaxHeight = rhs.m_fMaxHeight;
	m_EarthExtents = rhs.m_EarthExtents;

	// Copy each vtHeightField3d member
	m_WorldExtents	  = rhs.m_WorldExtents;
	m_Conversion	  = rhs.m_Conversion;
	m_fDiagonalLength = rhs.m_fDiagonalLength;

	// Copy each vtHeightFieldGrid3d member
	m_iColumns   = rhs.m_iColumns;
	m_iRows		 = rhs.m_iRows;
	m_fXStep	 = rhs.m_fXStep;
	m_fZStep	 = rhs.m_fZStep;
	m_dXStep	 = rhs.m_dXStep;
	m_dYStep	 = rhs.m_dYStep;

	// Copy each vtElevationGrid member
	m_bFloatMode		= rhs.m_bFloatMode;
	m_fVMeters			= rhs.m_fVMeters;
	m_fVerticalScale	= rhs.m_fVerticalScale;

	for (unsigned ii = 0; ii < sizeof( m_Corners ) / sizeof( *m_Corners ); ++ii)
		m_Corners[ii] = rhs.m_Corners[ii];

	m_proj = rhs.m_proj;
	m_strOriginalDEMName = rhs.m_strOriginalDEMName;

	return _AllocateArray();
}

bool vtElevationGrid::CopyDataFrom(const vtElevationGrid &rhs)
{
	int rx, ry;
	rhs.GetDimensions(rx, ry);
	if (m_iColumns != rx || m_iRows != ry)
		return false;

	if (m_bFloatMode && rhs.m_pFData)
	{
		size_t Size = m_iColumns * m_iRows * sizeof(float);
		memcpy(m_pFData, rhs.m_pFData, Size );
	}
	else if (!m_bFloatMode && rhs.m_pData)
	{
		size_t Size = m_iColumns * m_iRows * sizeof(short);
		memcpy(m_pData, rhs.m_pData, Size );
	}
	else return false;

	return true;
}


/**
 * Destructor
 */
vtElevationGrid::~vtElevationGrid()
{
	FreeData();
}


/**
 * Create a grid of given size.
 *
 * \param area the coordinate extents of the grid (rectangular area)
 * \param iColumns number of columns in the grid (east-west)
 * \param iRows number of rows (north-south)
 * \param bFloat data size: \c true to use floating-point, \c false for shorts.
 * \param proj the geographical projection to use.
 *
 * The grid will initially have no data in it (all values are INVALID_ELEVATION).
 */
bool vtElevationGrid::Create(const DRECT &area, int iColumns, int iRows,
	bool bFloat, const vtProjection &proj)
{
	vtHeightFieldGrid3d::Initialize(proj.GetUnits(), area, INVALID_ELEVATION,
		INVALID_ELEVATION, iColumns, iRows);

	m_bFloatMode = bFloat;

	ComputeCornersFromExtents();

	m_proj = proj;
	m_fVMeters = 1.0f;
	m_fVerticalScale = 1.0f;

	return _AllocateArray();
}

/**
 * Free any memory being used by this class for elevation data.
 */
void vtElevationGrid::FreeData()
{
	if (m_pData)
		free(m_pData);
	m_pData = NULL;
	if (m_pFData)
		free(m_pFData);
	m_pFData = NULL;
}

/**
 Set all the values in the grid to zero.
 */
void vtElevationGrid::Clear()
{
	int i, j;
	if (m_bFloatMode)
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				m_pFData[i*m_iRows+j] = 0.0f;
	}
	else
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				m_pData[i*m_iRows+j] = 0;
	}
}

/**
 Set all the values in the grid to INVALID_ELEVATION.
 */
void vtElevationGrid::Invalidate()
{
	int i, j;
	if (m_bFloatMode)
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				m_pFData[i*m_iRows+j] = INVALID_ELEVATION;
	}
	else
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				m_pData[i*m_iRows+j] = INVALID_ELEVATION;
	}
}

/**
 * Initializes an elevation grid by converting the contents of an another
 * grid to a new projection.
 *
 * \param pOld		An existing elevation grid to convert from.
 * \param NewProj	The new projection to convert to.
 * \param bUpgradeToFloat	If true, the resulting grid will always use
 *		floating-point values.  Otherwise, it matches the input grid.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 *
 * \return True if successful.
 */
bool vtElevationGrid::ConvertProjection(vtElevationGrid *pOld,
	const vtProjection &NewProj, float bUpgradeToFloat, bool progress_callback(int))
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
		m_strError = "Couldn't convert between coordinate systems.";
		return false;
	}

	// find where the extent corners are going to be in the new terrain
	int success;
	m_EarthExtents.SetRect(1E9, -1E9, -1E9, 1E9);
	for (i = 0; i < 4; i++)
	{
		DPoint2 point = pOld->m_Corners[i];
		success = trans->Transform(1, &point.x, &point.y);
		if (success == 0)
		{
			// inconvertible projections
			delete trans;
			m_strError = "Couldn't convert between coordinate systems.";
			return false;
		}
		m_Corners[i] = point;

		// Convert a number of segments along each edge, to be certain the
		//  new extents cover the entire area.
		DPoint2 p1 = pOld->m_Corners[i];
		DPoint2 p2 = pOld->m_Corners[(i+1)%4], diff = p2 - p1;
		for (j = 0; j < 50; j++)
		{
			DPoint2 p = p1 + (diff / 50 * j);
			trans->Transform(1, &p.x, &p.y);
			m_EarthExtents.GrowToContainPoint(p);
		}
	}
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
		new_step.y = old_step.y * METERS_PER_LATITUDE;
	}
	else if (!bOldGeo && bNewGeo)
	{
		// convert meters to degrees (approximately)
		meters_per_longitude = MetersPerLongitude(m_Corners[0].y);
		new_step.x = old_step.x / meters_per_longitude;
		new_step.y = old_step.y / METERS_PER_LATITUDE;	// convert degrees to meters (approximately)
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
	if (m_iColumns < 1 || m_iRows < 1 || m_iColumns > 40000 || m_iRows > 40000)
	{
		m_strError = "Grid is too small or too large.";
		return false;
	}

	// Now we're ready to fill in the new elevationgrid.
	// Some fields are simple to set:
	m_proj = NewProj;
	m_bFloatMode = pOld->m_bFloatMode;
	m_strOriginalDEMName = pOld->GetDEMName();

	// Allow user to upgrade to a float grid if they want
	if (bUpgradeToFloat)
		m_bFloatMode = true;

	// Others are on the parent class:
	vtHeightFieldGrid3d::Initialize(NewProj.GetUnits(), m_EarthExtents, INVALID_ELEVATION,
		INVALID_ELEVATION, m_iColumns, m_iRows);
	if (!_AllocateArray())
		return false;

	// Convert each bit of data from the old array to the new
	// Transformation points backwards, from the target to the source
	trans = CreateCoordTransform(pDest, pSource);
	if (!trans)
	{
		// inconvertible projections
		m_strError = "Couldn't convert between coordinate systems.";
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
 * Vertically offset the valid elevation values in the grid by a given amount.
 *
 * \param fAmount	The amount to be added to every elevation value.
 */
void vtElevationGrid::VertOffset(float fAmount)
{
	float f;
	for (int i = 0; i < m_iColumns; i++)
	{
		for (int j = 0; j < m_iRows; j++)
		{
			f = GetFValue(i, j);
			if (f != INVALID_ELEVATION)
				SetFValue(i, j, f + fAmount);
		}
	}
}

/**
 * Scans the grid to compute the minimum and maximum height values.
 * \sa GetHeightExtents
 */
void vtElevationGrid::ComputeHeightExtents()
{
	m_fMinHeight = 100000.0f;
	m_fMaxHeight = -100000.0f;

	if (!HasData())
		return;

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

/**
 * Replace one value with another.  For example, replace all heixels of value
 * INVALID_ELEVATION with 0.0.
 *
 * \param value1 The heixel value to replace.
 * \param value2 The value to replace it with.
 * \return The number of heixels that were modified.
 */
int vtElevationGrid::ReplaceValue(float value1, float value2)
{
	int replaced = 0;
	int i, j;
	for (i=0; i<m_iColumns; i++)
	{
		for (j=0; j<m_iRows; j++)
		{
			if (GetFValue(i, j) == value1)
				SetFValue(i, j, value2);
			replaced++;
		}
	}
	if (replaced > 0)
		ComputeHeightExtents();
	return replaced;
}

/**
 * Fill the gaps (heixels of value INVALID_ELVATION) in this grid, by
 * interpolating from the valid values.
 *
 * This method uses a simple, unoptimized algorithm to move across the grid,
 * one column at a time, averaging the surrounding pixels to fill each gap.
 *
 * \param area Optionally, restrict the operation to a given area.
 * \param progress_callback Provide if you want a callback on progress.
 * \return true if successful, false if cancelled.
 */
bool vtElevationGrid::FillGaps(DRECT *area, bool progress_callback(int))
{
	int i, j, ix, jx, surrounding;
	int gaps = 1;
	float value, value2, sum;

	int xmin = 0, xmax = m_iColumns, ymin = 0, ymax = m_iRows;
	if (area)
	{
		// Restrict the operation to a given area.
		DPoint2 spacing = GetSpacing();
		xmin = (int) ((area->left - m_EarthExtents.left)/spacing.x);
		if (xmin < 0) xmin = 0;
		if (xmin > m_iColumns) return true;

		ymin = (int) ((area->bottom - m_EarthExtents.bottom)/spacing.y);
		if (ymin < 0) ymin = 0;
		if (ymin > m_iRows) return true;

		xmax = (int) ((area->right - m_EarthExtents.left)/spacing.x);
		if (xmax < 0) return true;
		if (xmax > m_iColumns) xmax = m_iColumns;

		ymax = (int) ((area->top - m_EarthExtents.bottom)/spacing.y);
		if (ymax < 0) return true;
		if (ymax > m_iRows) ymax = m_iRows;
	}

	std::vector<float> patch_column(m_iRows);
	std::vector<float> patch_row(m_iColumns);

	// For speed, remember which lines already have no gaps, so we don't have
	// to visit them again.
	std::vector<bool> line_gap_columns(m_iColumns, true);
	std::vector<bool> line_gap_rows(m_iRows, true);

	int iPass = 0;
	int iTotalGaps;

	while (gaps > 0)
	{
		gaps = 0;

		// iterate through the heixels of the new elevation grid
		int start, step;
		if (iPass & 1) { start = xmin; step = 1; }
				  else { start = xmax-1; step = -1; }

		for (i = start; i >= xmin && i < xmax; i += step)
		{
			if (!line_gap_columns[i])
				continue;

			line_gap_columns[i] = false;

			bool patches = false;
			for (j = ymin; j < ymax; j++)
				patch_column[j] = INVALID_ELEVATION;

			for (j = ymin; j < ymax; j++)
			{
				value = GetFValue(i, j);
				if (value != INVALID_ELEVATION)
					continue;

				// else gap
				gaps++;
				line_gap_columns[i] = true;

				// look at surrounding pixels
				sum = 0;
				surrounding = 0;
				for (ix = -1; ix <= 1; ix++)
				{
					for (jx = -1; jx <= 1; jx++)
					{
						value2 = GetFValueSafe(i+ix, j+jx);
						if (value2 != INVALID_ELEVATION)
						{
							sum += value2;
							surrounding++;
						}
					}
				}
				if (surrounding > 1)
				{
					patch_column[j] = sum / surrounding;
					patches = true;
				}
			}
			if (patches)
			{
				for (j = ymin; j < ymax; j++)
				{
					if (patch_column[j] != INVALID_ELEVATION)
						SetFValue(i, j, patch_column[j]);
				}
			}
		}

		// Now the other way
		if (iPass & 1) { start = ymin; step = 1; }
				  else { start = ymax-1; step = -1; }

		for (j = start; j >= ymin && j < ymax; j += step)
		{
			if (!line_gap_rows[j])
				continue;

			line_gap_rows[j] = false;

			bool patches = false;
			for (i = xmin; i < xmax; i++)
				patch_row[i] = INVALID_ELEVATION;

			for (i = xmin; i < xmax; i++)
			{
				value = GetFValue(i, j);
				if (value != INVALID_ELEVATION)
					continue;

				// else gap
				gaps++;
				line_gap_rows[j] = true;

				// look at surrounding pixels
				sum = 0;
				surrounding = 0;
				for (ix = -1; ix <= 1; ix++)
				{
					for (jx = -1; jx <= 1; jx++)
					{
						value2 = GetFValueSafe(i+ix, j+jx);
						if (value2 != INVALID_ELEVATION)
						{
							sum += value2;
							surrounding++;
						}
					}
				}
				if (surrounding > 1)
				{
					patch_row[i] = sum / surrounding;
					patches = true;
				}
			}
			if (patches)
			{
				for (i = xmin; i < xmax; i++)
				{
					if (patch_row[i] != INVALID_ELEVATION)
						SetFValue(i, j, patch_row[i]);
				}
			}
		}

		if (iPass == 0)
		{
			iTotalGaps = gaps;
			if (progress_callback != NULL)
				progress_callback(0);
		}
		else
		{
			if (progress_callback != NULL)
			{
				if (progress_callback((iTotalGaps-gaps)*99/iTotalGaps))
					return false;
			}
		}
		iPass++;
	}

	// recompute what has likely changed
	ComputeHeightExtents();
	return true;
}


/**
 * Fill the gaps (heixels of value INVALID_ELVATION) in this grid, by
 * interpolating from the valid values.
 *
 * This method attempts to be a little better than FillGaps(), by keeping an
 * entire second grid for the interpolated results on each pass, to avoid
 * some cases of the results getting "smeared" left to right.  However, this
 * makes it much slower on most data.
 *
 * \param area Optionally, restrict the operation to a given area.
 * \param progress_callback Provide if you want a callback on progress.
 * \return true if successful, false if cancelled.
 */
bool vtElevationGrid::FillGapsSmooth(DRECT *area, bool progress_callback(int))
{
	int i, j, ix, jx;
	int gaps = 1;
	float value, value2, sum, surrounding;

	int xmin = 0, xmax = m_iColumns, ymin = 0, ymax = m_iRows;
	if (area)
	{
		// Restrict the operation to a given area.
		DPoint2 spacing = GetSpacing();
		xmin = (int) ((area->left - m_EarthExtents.left)/spacing.x);
		if (xmin < 0) xmin = 0;
		if (xmin > m_iColumns) return true;

		ymin = (int) ((area->bottom - m_EarthExtents.bottom)/spacing.y);
		if (ymin < 0) ymin = 0;
		if (ymin > m_iRows) return true;

		xmax = (int) ((area->right - m_EarthExtents.left)/spacing.x);
		if (xmax < 0) return true;
		if (xmax > m_iColumns) xmax = m_iColumns;

		ymax = (int) ((area->top - m_EarthExtents.bottom)/spacing.y);
		if (ymax < 0) return true;
		if (ymax > m_iRows) ymax = m_iRows;
	}

	vtElevationGrid delta(GetAreaExtents(), m_iColumns, m_iRows, true, GetProjection());

	// For speed, remember which lines already have no gaps, so we don't have
	// to visit them again.
	std::vector<bool> line_gap, has_delta;
	line_gap.resize(m_iColumns);
	has_delta.resize(m_iColumns);
	for (i = 0; i < m_iColumns; i++)
	{
		line_gap[i] = true;
		has_delta[i] = false;
	}

	bool bFirstPass = true;
	int iTotalGaps;

	while (gaps > 0)
	{
		gaps = 0;
		int num_filled = 0;

		// iterate through the heixels of the elevation grid
		for (i = xmin; i < xmax; i++)
		{
			// Don't visit lines without a gap
			if (!line_gap[i])
				continue;

			line_gap[i] = false;	// by default

			for (j = ymin; j < ymax; j++)
			{
				value = GetFValue(i, j);
				if (value != INVALID_ELEVATION)
					continue;

				// else gap
				gaps++;
				line_gap[i] = true;

				// look at surrounding pixels
				sum = 0;
				surrounding = 0;
				for (ix = -2; ix <= 2; ix++)
				{
					for (jx = -2; jx <= 2; jx++)
					{
						value2 = GetFValueSafe(i+ix, j+jx);
						if (value2 != INVALID_ELEVATION)
						{
							sum += value2;
							surrounding++;
						}
					}
				}
				if (surrounding > 4)
				{
					delta.SetFValue(i, j, sum / surrounding);
					has_delta[i] = true;
					num_filled++;
				}
				else
					delta.SetFValue(i, j, INVALID_ELEVATION);
			}
		}
		for (i = xmin; i < xmax; i++)
		{
			if (has_delta[i])
			{
				for (j = ymin; j < ymax; j++)
				{
					if (GetFValue(i, j) == INVALID_ELEVATION)
						SetFValue(i, j, delta.GetFValue(i, j));
				}
				has_delta[i] = false;
			}
		}
		if (bFirstPass)
		{
			iTotalGaps = gaps;
			if (progress_callback != NULL)
				progress_callback(0);
			bFirstPass = false;
		}
		else
		{
			if (progress_callback != NULL)
			{
				if (progress_callback((iTotalGaps-gaps)*99/iTotalGaps))
					return false;
			}
		}
		// If we reach a point where no gaps are filled on a pass, then exit so
		//  we are not stuck forever
		if (num_filled == 0)
			return true;
	}

	// recompute what has likely changed
	ComputeHeightExtents();
	return true;
}

/**
 * Fill-in algorithm.
 * Replaces no-data values by repeated region growing.
 * Smoothly extrapolates the filled-in value via partial derivatives.
 * Restricts the fill-in operation to concavities with a diameter of less than radius^2+1 pixels.
 *
 * \return The number of no-data heixels that were filled.
 *		Returns -1 on error, for example if there was not enought memory to
 *		create the temporary buffers.
 */
int vtElevationGrid::FillGapsByRegionGrowing(int radius_start, int radius_stop, bool progress_callback(int))
{
	uint count = 0;

	for (int r=radius_start; r<=radius_stop; r++)
	{
		 int result = FillGapsByRegionGrowing(r, progress_callback);

		 VTLOG("FillGapsByRegionGrowing(%d): %d filled\n", r, result);

		 if (result == -1)
			 return -1;
		 count += result;
	}
	return count;
}

/**
 * Fill-in algorithm.
 * Replaces no-data values by repeated region growing.
 * Smoothly extrapolates the filled-in value via partial derivatives.
 * Restricts the fill-in operation to concavities with a diameter of less than radius^2+1 pixels.
 *
 * Adapted subset from original code by: Stefan Roettger.
 * 
 * \return The number of no-data heixels that were filled.
 *		Returns -1 on error, for example if there was not enought memory to
 *		create the temporary buffers.
 */
int vtElevationGrid::FillGapsByRegionGrowing(int radius, bool progress_callback(int))
{
	uint count = 0;

	int i,j;
	int m,n;

	vtElevationGrid buf;
	vtElevationGrid cnt;
	vtElevationGrid tmp;

	int size;
	int sizex,sizey;

	float v1,v2;
	float dx,dy;
	int dxnum,dynum;

	// don't do anything unless there are gaps to fill
	int unknown = FindNumUnknown();
	if (unknown==0)
		return 0;

	// copy working buffer
	if (!buf.CopyFrom(*this))
		return -1;

	// allocate counting buffer
	if (!cnt.Create(m_EarthExtents, m_iColumns, m_iRows, false, m_proj))
		return -1;
	if (!tmp.Create(m_EarthExtents, m_iColumns, m_iRows, false, m_proj))
		return -1;

	// calculate foot print size
	size=2*radius+1;
	if (size<3) size=3;

	bool done = false;
	while (!done)
	{
		done = true;

		// calculate foot print size in x/y/z-direction
		if (m_iColumns<2)
		{
			sizex=1;
			sizey=1;
		}
		else if (m_iRows<2)
		{
			sizex=size;
			sizey=1;
		}
		else
		{
			sizex=size;
			sizey=size;
		}

		// calculate growing threshold
		int thres=(sizex*sizey+1)/2;

		// clear counting buffer
		cnt.FillWithSingleValue(0);

		// search for no-data values
		for (i=0; i<(int)m_iColumns; i++)
			for (j=0; j<(int)m_iRows; j++)
				if (GetFValue(i,j)!=INVALID_ELEVATION)
					cnt.SetValue(i, j, 1);

		// accumulate no-data values in x-direction
		if (m_iColumns>1)
		{
			for (j=0; j<(int)m_iRows; j++)
			{
				int cells=0;
				for (i=-sizex/2; i<(int)m_iColumns; i++)
				{
					if (i-sizex/2-1 >= 0)
						cells -= cnt.GetShortValue(i-sizex/2-1, j);
					if (i+sizex/2 < m_iColumns)
						cells += cnt.GetShortValue(i+sizex/2, j);
					if (i>=0)
						tmp.SetValue(i, j, cells);
				}
			}
		}

		// copy counting buffer back
		cnt.CopyDataFrom(tmp);

		// accumulate no-data values in y-direction
		if (m_iRows>1)
			for (i=0; i<(int)m_iColumns; i++)
			{
				int cells=0;
				for (j=-sizey/2; j<(int)m_iRows; j++)
				{
					if (j-sizey/2-1 >= 0)
						cells -= cnt.GetShortValue(i, j-sizey/2-1);
					if (j+sizey/2 < m_iRows)
						cells += cnt.GetShortValue(i, j+sizey/2);
					if (j>=0)
						tmp.SetValue(i,j,cells);
				}
			}

		// copy counting buffer back
		cnt.CopyDataFrom(tmp);

		// search for no-data values
		for (i = 0; i < m_iColumns; i++)
		{
			for (j = 0; j < m_iRows; j++)
			{
				if (GetFValue(i,j) != INVALID_ELEVATION)
					continue;

				// check number of foot print cells against growing threshold
				if (cnt.GetShortValue(i,j) < thres)
					continue;

				dx=dy=0.0f;
				dxnum=dynum=0;

				// average partial derivatives
				for (m=-sizex/2; m<=sizex/2; m++)
				{
					for (n=-sizey/2; n<=sizey/2; n++)
					{
						if (i+m>=0 && i+m < m_iColumns &&
							j+n>=0 && j+n < m_iRows)
						{
							v1=GetFValue(i+m,j+n);

							if (v1==INVALID_ELEVATION)
								continue;

							if (i+m-1>=0 && m>-sizex/2)
							{
								v2=GetFValue(i+m-1,j+n);
								if (v2!=INVALID_ELEVATION)
								{
									dx+=v1-v2;
									dxnum++;
								}
							}
							if (j+n-1>=0 && n>-sizey/2)
							{
								v2=GetFValue(i+m,j+n-1);
								if (v2!=INVALID_ELEVATION)
								{
									dy+=v1-v2;
									dynum++;
								}
							}
						}
					}
				}

				if (dxnum>0) dx/=dxnum;
				if (dynum>0) dy/=dynum;

				float val=0.0f;
				float sum=0.0f;

				// extrapolate partial derivatives
				for (m=-sizex/2; m<=sizex/2; m++)
				{
					for (n=-sizey/2; n<=sizey/2; n++)
					{
						if (i+m>=0 && i+m < m_iColumns && 
							j+n>=0 && j+n < m_iRows)
						{
							v1=GetFValue(i+m,j+n);

							if (v1!=INVALID_ELEVATION)
							{
								v2=v1-m*dx-n*dy;
								float weight = (float) (m*m+n*n);

								if (weight>0.0f)
								{
									val+=v2/weight;
									sum+=1.0f/weight;
								}
							}
						}
					}
				}

				// fill-in extrapolated value
				if (sum>0.0f)
				{
					val/=sum;
					if (val==INVALID_ELEVATION) val+=1.0f;

					buf.SetFValue(i,j,val);
					count++;

					done=false;
				}
			}
		}

		// copy working buffer back
		CopyDataFrom(buf);

		int remaining = FindNumUnknown();
		if (progress_callback != NULL)
		{
			if (progress_callback((unknown-remaining) * 99 / unknown))
			{
				// cancelled by user, but we've already modified the buffer,
				//  so report how many we've already done.  This is more of a
				//  'stop work' than a 'cancel'.
				return count;
			}
		}
	}

	// free working buffer
	buf.FreeData();

	// free counting buffer
	cnt.FreeData();
	tmp.FreeData();

	return(count);
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

/** Get a value direct from the grid, in the special case
 * where the grid is composed of short integers with no scaling.
 * \param i, j Column and row location in the grid.
 * \return The value of the grid.
 */
short vtElevationGrid::GetShortValue(int i, int j) const
{
	return m_pData[i*m_iRows+j];
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
	m_EarthExtents.left = std::min(m_Corners[0].x, m_Corners[1].x);
	m_EarthExtents.right = std::max(m_Corners[2].x, m_Corners[3].x);
	m_EarthExtents.bottom = std::min(m_Corners[0].y, m_Corners[3].y);
	m_EarthExtents.top = std::max(m_Corners[1].y, m_Corners[2].y);
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
bool vtElevationGrid::_AllocateArray()
{
	if (m_bFloatMode)
	{
		m_pData = NULL;
		m_pFData = (float *)malloc(m_iColumns * m_iRows * sizeof(float));
		if (!m_pFData)
		{
			m_strError.Format("Could not allocate a floating-point elevation grid of size %d x %d (%d MB)\n",
				m_iColumns, m_iRows, (m_iColumns * m_iRows * 4) / (1024 * 1024));
			VTLOG(m_strError);
			return false;
		}
	}
	else
	{
		m_pData = (short *)malloc(m_iColumns * m_iRows * sizeof(short));
		m_pFData = NULL;
		if (!m_pData)
		{
			m_strError.Format("Could not allocate a short-integer elevation grid of size %d x %d (%d MB)\n",
				m_iColumns, m_iRows, (m_iColumns * m_iRows * 2) / (1024 * 1024));
			VTLOG(m_strError);
			return false;
		}
	}
	// Initially no data
	FillWithSingleValue(INVALID_ELEVATION);
	return true;
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

void vtElevationGrid::GetEarthPoint(int i, int j, DPoint2 &p) const
{
	p.Set(m_EarthExtents.left + i * m_dXStep,
		m_EarthExtents.bottom + j * m_dYStep);
}

void vtElevationGrid::GetEarthLocation(int i, int j, DPoint3 &loc) const
{
	loc.Set(m_EarthExtents.left + i * m_dXStep,
		m_EarthExtents.bottom + j * m_dYStep,
		GetFValue(i, j));
}

/**
 * Get the height of the grid at a specific world coordinate (nearest neighbor)
 *
 * The value of the gridpoint closest to the specified location is returned.
 * If the location is not within the extents of the grid, INVALID_ELEVATION is returned.
 * \param p	The point to query.
 */
float vtElevationGrid::GetClosestValue(const DPoint2 &p) const
{
	int ix = (int)((p.x - m_EarthExtents.left) / m_EarthExtents.Width() * (m_iColumns-1) + 0.5);
	int iy = (int)((p.y - m_EarthExtents.bottom) / m_EarthExtents.Height() * (m_iRows-1) + 0.5);
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
 * The height field has a 0.5 pixel safety boundary to catch all samples on the edges.
 * \param p	The point to query.
 */
float vtElevationGrid::GetFilteredValue(const DPoint2 &p) const
{
	double local_x = (p.x - m_EarthExtents.left) / m_EarthExtents.Width();
	double local_y = (p.y - m_EarthExtents.bottom) / m_EarthExtents.Height();

	double findex_x = local_x * (m_iColumns-1);
	double findex_y = local_y * (m_iRows-1);

	if (findex_x < -0.5 || findex_x > m_iColumns - 0.5 ||
		findex_y < -0.5 || findex_y > m_iRows -0.5)
		return INVALID_ELEVATION;

	// clamp the near edges
	if (findex_x < 0.0) findex_x = 0.0;
	if (findex_x > m_iColumns-1) findex_x = m_iColumns-1;

	// clamp the far edges
	if (findex_y < 0.0) findex_y = 0.0;
	if (findex_y > m_iRows-1) findex_y = m_iRows-1;

	return GetInterpolatedElevation(findex_x, findex_y);
}

/**
 * The standard extents of an elevation grid are the min and max of its data
 * points.  However, because each point in the grid is a spot elevation that
 * implies the elevation of the ground around itself, the area over which the
 * elevation could be understood to describe is actually half a heixel larger
 * in each direction.  This method returns that larger area.
 */
DRECT vtElevationGrid::GetAreaExtents() const
{
	return DRECT(m_EarthExtents.left - (m_dXStep / 2.0f),
		m_EarthExtents.top + (m_dYStep / 2.0f),
		m_EarthExtents.right + (m_dXStep / 2.0f),
		m_EarthExtents.bottom - (m_dYStep / 2.0f));
}

float vtElevationGrid::GetFValueSafe(int i, int j) const
{
	if (i < 0 || i > m_iColumns-1 || j < 0 || j > m_iRows-1)
		return INVALID_ELEVATION;
	return GetFValue(i, j);
}

void vtElevationGrid::SetProjection(const vtProjection &proj)
{
	LinearUnits newunits = proj.GetUnits();
	if (newunits != m_proj.GetUnits())
	{
		// change of units requires change in local coordinate system
		// units change; all else remains same
		vtHeightFieldGrid3d::Initialize(newunits, m_EarthExtents,
			m_fMinHeight, m_fMaxHeight, m_iColumns, m_iRows);
	}
	m_proj = proj;
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

/**
 * Set up the terrain's coordinate systems, including the conversion from Earth
 * coordinates to the World coordinates of the terrain itself.  You should call
 * this method after loading or creating a vtElevationGrid, before using methods
 * on it such as FindAltitudeOnEarth or GetSpacing.
 *
 * For explanation of vtlib/vtdata world coordinates, please see
 * http://vterrain.org/Doc/coords.html
 *
 * \param fVerticalExag The vertical exaggeration of the terrain, which is how
 *	many times higher it should appear than it really is.  Pass 1.0 for no
 *	exaggeration.
 */
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
	return GetFValueSafe(iX, iZ);
}

void vtElevationGrid::GetWorldLocation(int i, int j, FPoint3 &loc, bool bTrue) const
{
	if (bTrue)
	{
		loc.Set(m_WorldExtents.left + i * m_fXStep,
			GetFValueSafe(i,j),
			m_WorldExtents.bottom - j * m_fZStep);
	}
	else
	{
		float value = GetFValueSafe(i,j);
		if (value != INVALID_ELEVATION) value *= m_fVerticalScale;
		loc.Set(m_WorldExtents.left + i * m_fXStep,
			value,
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
 * Simple elevation test for perfectly regular grid terrain.
 * Find altitude (y) and (optionally) surface normal, given (x,z) world coordinates.
 *
 * This approach is very straightforward, so it could be significantly
 * sped up if needed.
 *
 * \param p A 3D point in world coordinates.  Only the X and Z values are used.
 * \param fAltitude If the test succeeds, this contains the result by reference.
 * \param bTrue Pass true to the use the true elevation, false to consider the
 *		vertical exaggeration in effect.
 * \param iCultureFlags Pass 0 to test only the heightfield itself,
 *		non-zero to test any culture objects which may be sitting on
 *		the heightfield.  Values include:
 *		- CE_STRUCTURES	Test structures on the ground.
 *		- CE_ROADS Test roads on the ground.
 *		- CE_ALL Test everything on the ground.
 * \param vNormal If you pass a pointer to a vector, it will be filled in with
 *		the upwards-pointing surface normal at the given point.
 *
 * \return true if the point was inside the elevation grid, false if outside.
 */
bool vtElevationGrid::FindAltitudeAtPoint(const FPoint3 &p, float &fAltitude,
	bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
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

/**
 * Return the elevation value at a given point in earth coordinates.
 *
 * \param p A 2D point in earth coordinates.
 * \param fAltitude If the test succeeds, this contains the result by reference.
 * \param bTrue Pass true to the use the true elevation, false to consider the
 *		vertical exaggeration in effect.
 *
 * \return true if the point was inside the elevation grid, false if outside.
 */
bool vtElevationGrid::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude,
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
