//
// vtElevationGrid.cpp
//
// Copyright (c) 2001 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include <fstream>
using namespace std;

#include "ElevationGrid.h"
#include "ByteOrder.h"
#include "vtDIB.h"

#define meters_per_latitude	111300.0f

#ifndef max
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

//////////////////////////////////////////////////

/**
 * Constructor: Creates an empty grid.
 */
vtElevationGrid::vtElevationGrid()
{
	m_area.SetRect(0, 0, 0, 0);
	m_iColumns = 0;
	m_iRows = 0;

	m_bFloatMode = false;
	m_pData = NULL;
	m_pFData = NULL;
	m_fVMeters = 1.0f;

	for (int i = 0; i < 4; i++)
		m_Corners[i].Set(0, 0);

	m_fMinHeight = m_fMaxHeight = INVALID_ELEVATION;
	m_szOriginalDEMName[0] = 0;
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
								 bool bFloat, vtProjection &proj)
{
	m_area = area;			// raw extents
	m_iColumns = iColumns;
	m_iRows = iRows;

	m_bFloatMode = bFloat;
	_AllocateArray();
	m_fVMeters = 1.0f;

	ComputeCornersFromExtents();

	m_fMinHeight = m_fMaxHeight = INVALID_ELEVATION;
	m_proj = proj;
	m_szOriginalDEMName[0] = 0;
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
 * \param pOld An existing elevation grid to convert from.
 * \param NewProj The new projection to convert to.
 * \return True if successful.
 */
bool vtElevationGrid::ConvertProjection(vtElevationGrid *pOld,
										vtProjection &NewProj,
										void progress_callback(int))
{
	int i, j;

	// some fields are simple to set
	m_proj = NewProj;
	m_bFloatMode = pOld->m_bFloatMode;
	strcpy(m_szOriginalDEMName, pOld->GetDEMName());

	// Create conversion object
	OGRSpatialReference *pSource, *pDest;
	pSource = &pOld->GetProjection();
	pDest = &NewProj;

	OCT *trans = OGRCreateCoordinateTransformation(pSource, pDest);
	if (!trans)
	{
		// inconvertible projections
		return false;
	}

	// find where the extent corners are going to be in the new terrain
	for (i = 0; i < 4; i++)
	{
		DPoint2 point = pOld->m_Corners[i];
		trans->Transform(1, &point.x, &point.y);
		m_Corners[i] = point;
	}
	ComputeExtentsFromCorners();
	delete trans;

	// now, how large an array will we need for the new terrain?
	// try to preserve the sampling rate approximately
	//
	double meters_per_longitude;

	bool bOldGeo = (pSource->IsGeographic() != 0);
	bool bNewGeo = (pDest->IsGeographic() != 0);

	DPoint2 old_step = pOld->GetSpacing();
	DPoint2 new_step;

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
	double fColumns = m_area.Width() / new_step.x;
	double fRows = m_area.Height() / new_step.y;

	// round up to the nearest integer
	m_iColumns = (int)(fColumns + 0.999);
	m_iRows = (int)(fRows + 0.999);

	// do safety check
	if (m_iColumns > 40000 || m_iRows > 40000)
		return false;

	_AllocateArray();

	// convert each bit of data from the old array to the new
	DPoint2 p, lat, step = GetSpacing();
	float value;

	// projects points backwards, from the target to the source
	trans = OGRCreateCoordinateTransformation(pDest, pSource);
	if (!trans)
	{
		// inconvertible projections
		return false;
	}

	for (i = 0; i < m_iColumns; i++)
	{
		if (progress_callback != NULL) progress_callback(i*100/m_iColumns);

		for (j = 0; j < m_iRows; j++)
		{
			p.x = m_area.left + i * step.x;
			p.y = m_area.bottom + j * step.y;

			trans->Transform(1, &p.x, &p.y);

			value = pOld->GetFilteredValue(p.x, p.y);
			SetFValue(i, j, value);
		}
	}
	delete trans;
	return true;
}


/**
 * Scale all the valid elevation values in the grid by a given factor.
 *
 * \param bDirect If true, scale the stored height values directly.  Otherwise,
 * only the height scale (vertical meters per unit) is scaled.
 */
void vtElevationGrid::Scale(float fScale, bool bDirect)
{
	if (!bDirect)
	{
		m_fVMeters *= fScale;
		return;
	}
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


/** Scans the grid to compute the minimum and maximum height values.
 * \sa GetHeightExtents
 */
void vtElevationGrid::ComputeHeightExtents()
{
	int i, j;

	m_fMinHeight = 100000.0f;
	m_fMaxHeight = -100000.0f;

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

/** Gets the minimum and maximum height values.  The values are placed in the
 * arguments by reference.  You must have first called ComputeHeightExtents.
 */
void vtElevationGrid::GetHeightExtents(float &fMinHeight, float &fMaxHeight)
{
	fMinHeight = m_fMinHeight;
	fMaxHeight = m_fMaxHeight;
}


/** Get the grid size of the grid.
 * The values are placed into the arguments by reference.
 * \param nColumns The number of columns (east-west)
 * \param nRows THe number of rows (north-south)
 */
void vtElevationGrid::GetDimensions(int &nColumns, int &nRows)
{
	nColumns = m_iColumns;
	nRows = m_iRows;
}

/** Get the grid spacing, the width of each column and row.
 */
DPoint2 vtElevationGrid::GetSpacing()
{
	return DPoint2(m_area.Width() / (m_iColumns - 1),
				   m_area.Height() / (m_iRows - 1));
}

/** Set an elevation value to the grid.
 * \param i, j Location in the grid.
 * \param value The value in (integer) meters.
 */
void vtElevationGrid::SetValue(int i, int j, short value)
{
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
 * \param i, j Location in the grid.
 * \param value The value in (float) meters.
 */
void vtElevationGrid::SetFValue(int i, int j, float value)
{
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
 * \param i, j Location in the grid.
 * \return The value in (integer) meters.
 */
int vtElevationGrid::GetValue(int i, int j)
{
	if (m_bFloatMode)
	{
		float value = m_pFData[i*m_iRows+j];
		if (m_fVMeters == 1.0f || value == INVALID_ELEVATION)
			return (int) value;
		else
			return (int) (value * m_fVMeters);
	}
	short svalue = m_pData[i*m_iRows+j];
	if (m_fVMeters == 1.0f || svalue == INVALID_ELEVATION)
		return svalue;
	else
		return (int) ((float)svalue * m_fVMeters);
}

/** Get an elevation value from the grid.
 * \param i, j Location in the grid.
 * \return The value in (float) meters.
 */
float vtElevationGrid::GetFValue(int i, int j)
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
	m_area.left = min(m_Corners[0].x, m_Corners[1].x);
	m_area.right = max(m_Corners[2].x, m_Corners[3].x);
	m_area.bottom = min(m_Corners[0].y, m_Corners[3].y);
	m_area.top = max(m_Corners[1].y, m_Corners[2].y);
}


/** For a grid whose absolute extents are known, use
 * those extents to imply the 4 corners coordinates.
 */
void vtElevationGrid::ComputeCornersFromExtents()
{
	m_Corners[0].x = m_area.left;
	m_Corners[0].y = m_area.bottom;
	m_Corners[1].x = m_area.left;
	m_Corners[1].y = m_area.top;
	m_Corners[2].x = m_area.right;
	m_Corners[2].y = m_area.top;
	m_Corners[3].x = m_area.right;
	m_Corners[3].y = m_area.bottom;
}


//
// Allocates a data array big enough to contain the grid data.
//
void vtElevationGrid::_AllocateArray()
{
	if (m_bFloatMode) {
		m_pData = NULL;
		m_pFData = (float *)malloc(m_iColumns * m_iRows * sizeof(float));
	} else {
		m_pData = (short *)malloc(m_iColumns * m_iRows * sizeof(short));
		m_pFData = NULL;
	}

	// Initially no data
	int i, j;
	if (m_bFloatMode)
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				SetFValue(i, j, INVALID_ELEVATION);
	}
	else
	{
		for (i = 0; i < m_iColumns; i++)
			for (j = 0; j < m_iRows; j++)
				SetValue(i, j, INVALID_ELEVATION);
	}
}


void vtElevationGrid::GetEarthLocation(int i, int j, DPoint3 &loc)
{
	DPoint2 spacing = GetSpacing();
	loc.Set(m_area.left + i * spacing.x,
			m_area.bottom + j * spacing.y,
			GetFValue(i, j));
}

/** Use the height data in the grid to fill a bitmap with a shaded color image.
 * \param pDIB The bitmap to color.
 * \param color_ocean The color to use for areas at sea level.
 */
void vtElevationGrid::ColorDibFromElevation(vtDIB *pDIB, RGBi color_ocean,
											bool bZeroIsOcean)
{
	int w = pDIB->GetWidth();
	int h = pDIB->GetHeight();

	int gw, gh;
	GetDimensions(gw, gh);

	int i, j;
	int x, y;
	RGBi color;

	float fMin, fMax;
	GetHeightExtents(fMin, fMax);

	Array<RGBi> colors;
	colors.Append(RGBi(75, 155, 75));
	colors.Append(RGBi(180, 160, 120));
	colors.Append(RGBi(128, 128, 128));
	int bracket, num = colors.GetSize();
	float bracket_size = (fMax - fMin) / (num - 1);

	// iterate over the texels
	for (i = 0; i < w; i++)
	{
		x = i * gw / w;			// find corresponding location in terrain

		for (j = 0; j < h; j++)
		{
			y = j * gh / h;

			float m = GetFValue(x, y);	// local units
			float elev = m - fMin;

			color.r = color.g = color.b = 0;
			if (bZeroIsOcean && m == 0.0f)
			{
				color = color_ocean;
			}
			else if (bracket_size != 0.0f)
			{
				bracket = (int) (elev / bracket_size);
				if (bracket < 0)
					color = colors[0];
				else if (bracket < num-1)
				{
					float fraction = (elev / bracket_size) - bracket;
					RGBi diff = (colors[bracket+1] - colors[bracket]);
					color = colors[bracket] + (diff * fraction);
				}
				else
					color = colors[num-1];
			}
			else
			{
				color.Set(20, 230, 20);	// flat green
			}
			pDIB->SetPixel24(i, h-1-j, RGB(color.r, color.g, color.b));
		}
	}
}


/**
 * Get the height of the grid at a specific world coordinate.
 *
 * The value of the gridpoint closest to the specified location is returned.
 * If the location is not within the extents of the grid, INVALID_ELEVATION is returned.
 * \param x, y	The coordinate to query.
 */
float vtElevationGrid::GetClosestValue(double x, double y)
{
	int ix = (int)((x - m_area.left) / m_area.Width() * m_iColumns);
	int iy = (int)((y - m_area.bottom) / m_area.Height() * m_iRows);
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
 * \param x, y	The coordinate to query.
 */
float vtElevationGrid::GetFilteredValue(double x, double y)
{
	// what data point in t is closest to (x,y)?
	double local_x = (x - m_area.left) / (m_area.right - m_area.left);
	double local_y = (y - m_area.bottom) / (m_area.top - m_area.bottom);

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

float vtElevationGrid::GetFValueSafe(int i, int j)
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
float vtElevationGrid::GetFilteredValue2(double x, double y)
{
	float fData;

	// simple case, within the 
	if (ContainsPoint(x, y))
	{
		fData = GetFilteredValue(x, y);
		if (fData != INVALID_ELEVATION)
			return fData;
	}

	// what data point in t is closest to (x,y)?
	double local_x = (x - m_area.left) / (m_area.right - m_area.left);
	double local_y = (y - m_area.bottom) / (m_area.top - m_area.bottom);

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
		fData = (float) (fDataBL + (fDataBR-fDataBL)*diff_x +
						  (fDataTL-fDataBL)*diff_y +
						  (fDataTR-fDataTL-fDataBR+fDataBL)*diff_x*diff_y);
	}
	else if (valid == 3)
	{
		// do average; it's better than nothing
		fData = sum / valid;
	}
	else
		fData = INVALID_ELEVATION;
	
	return fData;
}

DRECT vtElevationGrid::GetAreaExtents()
{
	DPoint2 sample_size = GetSpacing();
	return DRECT(m_area.left - (sample_size.x / 2.0f),
				 m_area.top + (sample_size.y / 2.0f),
				 m_area.right + (sample_size.x / 2.0f),
				 m_area.bottom - (sample_size.y / 2.0f));
}

bool vtElevationGrid::GetCorners(DLine2 &line, bool bGeo)
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

		OCT *trans = CreateConversionIgnoringDatum(&m_proj, &Dest);
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

