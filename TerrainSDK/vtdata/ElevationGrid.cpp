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

//////////////////////////////////////////////////

/** Constructor: Creates an empty grid.
 *
 */
vtElevationGrid::vtElevationGrid()
{
	m_pData = NULL;
	m_pFData = NULL;
	m_iColumns = 0;
	m_iRows = 0;
	m_bFloatMode = false;

	m_fMinHeight = m_fMaxHeight = 0.0f;
	m_szOriginalDEMName[0] = 0;
	m_fVMeters = 1.0;
	m_fGRes = 1.0;
}

/** Constructor: Creates a grid of given size.
 * \param area the coordinate extents of the grid (rectangular area)
 * \param iColumns number of columns in the grid (east-west)
 * \param iRows number of rows (north-south)
 * \param bFloat data size: \c true to use floating-point, \c false for shorts.
 * \param proj the geographical projection to use.
 *
 * The grid will initially have no data in it (all values are INVALID_ELEVATION).
 */
vtElevationGrid::vtElevationGrid(DRECT area, int iColumns, int iRows,
								 bool bFloat, vtProjection proj)
{
	m_area = area;			// raw extents
	m_iColumns = iColumns;
	m_iRows = iRows;
	m_bFloatMode = bFloat;
	m_proj = proj;
	m_szOriginalDEMName[0] = 0;

	AllocateArray();
}


/** Initializes an elevation grid by converting the contents of an another
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
	bool bOldGeo = (pSource->IsGeographic() != 0);
	bool bNewGeo = (pDest->IsGeographic() != 0);

	DPoint2 old_step = pOld->GetSpacing();
	DPoint2 new_step;

	if (bOldGeo && !bNewGeo)
	{
		// convert degrees to meters (approximately)
		new_step.x = old_step.x * meters_per_latitude * cos(pOld->m_Corners[0].y / 180.0 * PI);
		new_step.y = old_step.y * meters_per_latitude;
	}
	else if (!bOldGeo && bNewGeo)
	{
		// convert meters to degrees (approximately)
		new_step.x = old_step.x / (meters_per_latitude * cos(m_Corners[0].y / 180.0 * PI));
		new_step.y = old_step.y / (meters_per_latitude);	// convert degrees to meters (approximately)
	}
	else
	{
		// old and new terrain are in the same horizontal units
		new_step = old_step;
	}
	double fColumns = m_area.Width() / new_step.x;
	double fRows = m_area.Height() / new_step.y;

	// round up to the nearest integer
	m_iColumns = (int)(fColumns + 0.999);
	m_iRows = (int)(fRows + 0.999);
	AllocateArray();

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


/** Destructor
 *
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
		m_pFData[i*m_iRows+j] = (float)value;
	else
		m_pData[i*m_iRows+j] = value;
}

/** Set an elevation value to the grid.
 * \param i, j Location in the grid.
 * \param value The value in (float) meters.
 */
void vtElevationGrid::SetFValue(int i, int j, float value)
{
	if (m_bFloatMode)
		m_pFData[i*m_iRows+j] = value;
	else
		m_pData[i*m_iRows+j] = (short)value;
}

/** Get an elevation value from the grid.
 * \param i, j Location in the grid.
 * \return The value in (integer) meters.
 */
int vtElevationGrid::GetValue(int i, int j)
{
	if (m_bFloatMode)
		return (int) m_pFData[i*m_iRows+j];
	else
		return m_pData[i*m_iRows+j];
}

/** Get an elevation value from the grid.
 * \param i, j Location in the grid.
 * \return The value in (float) meters.
 */
float vtElevationGrid::GetFValue(int i, int j)
{
	if (m_bFloatMode)
		return m_pFData[i*m_iRows+j];
	else
		return (float) m_pData[i*m_iRows+j];
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
void vtElevationGrid::AllocateArray()
{
	if (m_bFloatMode) {
		m_pData = NULL;
		m_pFData = (float *)calloc(m_iColumns, m_iRows*sizeof(float));
	} else {
		m_pData = (short *)calloc(m_iColumns, m_iRows*sizeof(short));
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


static RGBi color_base(255, 233, 184);
static RGBi color_hill(168, 154, 112);

/** Use the height data in the grid to fill a bitmap with a shaded color image.
 * \param pDIB The bitmap to color.
 * \param color_ocean The color to use for areas at sea level.
 */
void vtElevationGrid::ColorDibFromElevation1(vtDIB *pDIB, RGBi color_ocean)
{
	int w = pDIB->GetWidth();
	int h = pDIB->GetHeight();

	int gw, gh;
	GetDimensions(gw, gh);

	int i, j;
	int x, y;
	RGBi color;

	float fMin, fMax;
	fMin = m_fMinHeight;
	fMax = m_fMaxHeight;
	float low_elev = 100.0f;

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
			if (m == 0.0f)
			{
				color = color_ocean;
			}
			else if (elev < low_elev)
			{
				color = color_base;
			}
			else if (elev < low_elev*2)
			{
				float scale = (elev - low_elev) / low_elev;
				RGBi diff = (color_hill - color_base);
				RGBi offset = (diff * scale);
				color = color_base + offset;
			}
			else
			{
				color = color_hill;
			}
			pDIB->SetPixel24(i, h-1-j, RGB(color.b, color.g, color.r));
		}
	}
}


/** Get the height of the grid at a specific world coordinate.
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


/** Get the interpolated height of the grid at a specific world coordinate.
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
	if ((index_x == m_iColumns-1) || (index_y == m_iRows-1))
		//on the edge, do what we normally do
		fData = GetFValue(index_x, index_y);
	else //do the filtering
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
			fData = fDataBL + (fDataBR-fDataBL)*diff_x +
							  (fDataTL-fDataBL)*diff_y +
							  (fDataTR-fDataTL-fDataBR+fDataBL)*diff_x*diff_y;
		else
			fData = INVALID_ELEVATION;
	}
	return (float) fData;
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
		OGRSpatialReference *pSource, Dest;
		pSource = &m_proj;

		Dest.SetWellKnownGeogCS("WGS84");

		// We can't convert datum yet.  Force assumption that destination
		// datum is the same as the source.
		const char *datum_string = m_proj.GetAttrValue("DATUM");
		const char *ellipsoid_string = m_proj.GetAttrValue("SPHEROID");
		Dest.SetGeogCS("WGS84", datum_string, ellipsoid_string,
			6378137, 298.257223563);

		OCT *trans = OGRCreateCoordinateTransformation(pSource, &Dest);
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
	}
	return true;
}

void vtElevationGrid::SetCorners(const DLine2 &line)
{
	for (int i = 0; i < 4; i++)
		m_Corners[i] = line[i];
}

