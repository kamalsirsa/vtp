//
// vtElevationGridIO.cpp
//
// This modules contains the implementations of the file I/O methods of
// the class vtElevationGrid.
//
// Copyright (c) 2001 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include <fstream>
using namespace std;

#include "config_vtdata.h"
#include "ElevationGrid.h"
#include "ByteOrder.h"
#include "vtString.h"

#if SUPPORT_NETCDF
extern "C" {
#include "netcdf.h"
}
#endif

// GDAL
#include "gdal_priv.h"

//	Header structure for a GTOPO30 DEM header file
typedef struct
{
	char			ByteOrder[30];		//	Byte order in which image pixel values are stored
										//		M = Motorola byte order (MSB first)
	char			Layout[30];			//	Organization of the bands in the file
										//		BIL = Band interleaved by line (DEM = single band image)
	unsigned long	NumRows;			//	Number of rows in the image
	unsigned long	NumCols;			//	Number of columns in the image
	char			Bands[30];			//	Number of spectral bands in the image (1 for DEM)
	char			Bits[30];			//	Number of bits per pixel (16 for DEM)
	char			BandRowBytes[30];	//	Number of bytes per band per row
										//		(twice the number of columns for 16 bit DEM)
	char			TotalRowBytes[30];	//	Total number of bytes of data per row
										//		(twice the number of columns for 16 bit DEM)
	char			BandGapBytes[30];	//	Number of bytes between bands in a BSQ format
										//		(0 for DEM)
	short			NoData;				//	Value used for no data or masking purposes
	double			ULXMap;				//	Longitude of the center of the upper left pixel in decimal degrees.
	double			ULYMap;				//	Latitude of the center of the upper left pixel in decimal degrees.
	double			XDim;				//	x dimension of a pixel in geographic units (decimal degrees).
	double			YDim;				//	y dimension of a pixel in geographic units (decimal degrees).
} GTOPOHeader;


// ************** DConvert - DEM Helper function ****************

double DConvert(FILE *fp, int length)
{
	char szCharString[64];

	fread(szCharString, length, 1, fp);
	szCharString[length] = 0;

	return atof(szCharString);
}

#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif


/**
 * Load from a file whose type is not known a priori.  This will end up
 * calling one of the Load* member functions.
 *
 * \return true if successful.
 */
bool vtElevationGrid::LoadFromFile(const char *szFileName,
								   void progress_callback(int))
{
	vtString FileName(szFileName);
	vtString FileExt = FileName.Right(3);

	if (FileExt == "")
		return false;

	// The first character in the file is useful for telling which format
	// the file really is.
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;
	char FirstChar = fgetc(fp);
	fclose(fp);

	bool Success = false;

	if (!FileExt.CompareNoCase(".bt"))
	{
		Success = LoadFromBT(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("dem"))
	{
		if (FirstChar == '*')
			Success = LoadFromMicroDEM(szFileName, progress_callback);
		else
			Success = LoadFromDEM(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("asc"))
	{
		Success = LoadWithGDAL(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("bil"))
	{
		Success = LoadWithGDAL(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("ter"))
	{
		Success = LoadFromTerragen(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("cdf"))
	{
		Success = LoadFromCDF(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("hdr"))
	{
		Success = LoadFromGTOPO30(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("dte") ||
			 !FileExt.CompareNoCase("dt0") ||
			 !FileExt.CompareNoCase("dt1") ||
			 !FileExt.CompareNoCase("dt2"))
	{
		Success = LoadFromDTED(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("pgm"))
	{
		Success = LoadFromPGM(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase("grd"))
	{
		// might by CDF, might be GRD
		if (FirstChar == 'D')
			Success = LoadFromGRD(szFileName, progress_callback);
		else
			Success = LoadFromCDF(szFileName, progress_callback);

		if (!Success)
		{
			// Might be 'Arc Binary Grid', try GDAL
			Success = LoadWithGDAL(szFileName, progress_callback);
		}
	}
	else if (!FileName.Right(8).CompareNoCase("catd.ddf") ||
			 !FileExt.CompareNoCase("tif") ||
			 !FileExt.CompareNoCase("png") ||
			 !FileExt.CompareNoCase("adf"))
	{	
		Success = LoadWithGDAL(szFileName, progress_callback);
	}
	return Success;
}


/**
 * Loads elevation from a USGS DEM file.
 *
 * Some non-standard variations of the DEM format are supported.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromDEM(const char *szFileName,
								  void progress_callback(int))
{
	FILE	*fp;
	int		i, j;
	int		iRow, iColumn;
	int		iElev;
	int		iVUnit, iGUnit;
	double	fRows;
	double	fVertUnits;
	double 	dxdelta, dydelta, dzdelta;
	double	dElevMax, dElevMin;
	bool	bOldFormat = false, bNewFormat = false, bFixedLength = true;
	int		iCoordSystem;
	int		iProfiles;
	char	szName[41];
	char	szDateBuffer[5];
	DPoint2	corners[4];			// SW, NW, NE, SE
	double	fGMeters;			// ground (horizontal) units
	int		iDataStartOffset;
	bool	bUTM;
	int		iUTMZone;
	int		iDatum;
	int		iProfileRows, iProfileCols;
	double	dLocalDatumElev, dProfileMin, dProfileMax;
	int		ygap;
	DPoint2 start;
	double	dMinY;

	if (progress_callback != NULL) progress_callback(0);

	if (NULL == (fp = fopen(szFileName,"rb")))
	{
		// Cannot Open File
		return false;
	}

	// check for version of DEM format
	fseek(fp, 864, 0);
	fscanf(fp, "%d", &iRow);
	fscanf(fp, "%d", &iColumn);
	bOldFormat = (iRow==1 && iColumn==1);
	if (bOldFormat)
		iDataStartOffset = 1024;	// 1024 is record length
	else
	{
		fseek(fp, 1024, 0); 		// Check for New Format
		fscanf(fp, "%d", &iRow);
		fscanf(fp, "%d", &iColumn);
		if (iRow==1 && iColumn==1)	// File OK?
		{
			bNewFormat = true;
			iDataStartOffset = 1024;
		}
		else
		{
			fseek(fp, 893, 0);	 	// Non-fixed-length record format
			fscanf(fp, "%d", &iRow);
			fscanf(fp, "%d", &iColumn);
			if (iRow==1 && iColumn==1)	// File OK?
			{
				bFixedLength = false;
				iDataStartOffset = 893;
			}
			else
			{
				// Not a DEM file
				fclose(fp);
				return false;
			}
		}
	}

	// Read the embedded DEM name
	fseek(fp, 0, 0);
	fgets(szName, 41, fp);
	int len = strlen(szName);	// trim trailing whitespace
	while (len > 0 && szName[len-1] == ' ')
	{
		szName[len-1] = 0;
		len--;
	}
	strcpy(m_szOriginalDEMName, szName);

	fseek(fp, 156, 0);
	fscanf(fp, "%d", &iCoordSystem);
	fscanf(fp, "%d", &iUTMZone);

	if (iCoordSystem == 0)	// geographic (lat-lon)
	{
		bUTM = false;
		iUTMZone = -1;
	}
	if (iCoordSystem == 1)	// utm
		bUTM = true;

	fseek(fp, 528, 0);
	fscanf(fp, "%d", &iGUnit);
	fscanf(fp, "%d", &iVUnit);

	// Ground Units in meters
	switch (iGUnit)
	{
	case 1: fGMeters = 0.3048;	break;	// 1 = feet
	case 2: fGMeters = 1.0;		break;	// 2 = meters
	case 3: fGMeters = 30.922;	break;	// 3 = arc-seconds
	}

	// Vertical Units in meters
	switch (iVUnit)
	{
	case 1:  fVertUnits = 0.3048f; break;	// feet to meter conversion
	case 2:  fVertUnits = 1.0f;	   break;	// meters == meters
	default: fVertUnits = 1.0f;	   break;	// anything else, assume meters
	}

	fseek(fp, 816, 0);
	dxdelta = DConvert(fp, 12);
	dydelta = DConvert(fp, 12);
	dzdelta = DConvert(fp, 12);

	m_bFloatMode = false;

	// Read the coordinates of the 4 corners
	fseek(fp, 546, 0);
	for (i = 0; i < 4; i++)
	{
		corners[i].x = DConvert(fp, 24);
		corners[i].y = DConvert(fp, 24);
	}

	if (bUTM)	// UTM
	{
		for (i = 0; i < 4; i++)
			m_Corners[i] = corners[i];
	}
	else
	{
		for (i = 0; i < 4; i++)
		{
			// convert arcseconds to degrees
			m_Corners[i].x = corners[i].x / 3600.0;
			m_Corners[i].y = corners[i].y / 3600.0;
		}
	}

	dElevMin = DConvert(fp, 24);
	dElevMax = DConvert(fp, 24);

	int rows;
	fseek(fp, 852, 0);
	fscanf(fp, "%d", &rows);
	fscanf(fp, "%d", &iProfiles);

	iDatum = EPSG_DATUM_NAD27;	// default

	// OLD format header ends at byte 864
	if (bNewFormat)
	{
		// year of data compilation
		fseek(fp, 876, 0);
		fread(szDateBuffer, 4, 1, fp);
		szDateBuffer[4] = 0;

		// Horizontal datum
		// 1=North American Datum 1927 (NAD 27)
		// 2=World Geodetic System 1972 (WGS 72)
		// 3=WGS 84
		// 4=NAD 83
		// 5=Old Hawaii Datum
		// 6=Puerto Rico Datum
		int datum;
		fseek(fp, 890, 0);
		fscanf(fp, "%d", &datum);
		switch (datum) {
			case 1: iDatum = EPSG_DATUM_NAD27; break;
			case 2: iDatum = EPSG_DATUM_WGS72; break;
			case 3: iDatum = EPSG_DATUM_WGS84; break;
			case 4:	iDatum = EPSG_DATUM_NAD83; break;
			case 5: iDatum = EPSG_DATUM_OLD_HAWAIIAN; break;
			case 6: iDatum = EPSG_DATUM_PUERTO_RICO; break;
		}
	}

	// Set up the projection
	m_proj.SetProjectionSimple(bUTM, iUTMZone, iDatum);
	m_iColumns = iProfiles;

	if (!bUTM)
	{
		// If it's in degrees, it's flush square, so we can simply
		// derive the extents (m_EarthExtents) from the quad corners (m_Corners)
		ComputeExtentsFromCorners();
		dMinY = min(corners[0].y, corners[3].y);
	}
	else
	{
		m_EarthExtents.SetRect(1E9, -1E9, -1E9, 1E9);

		if (!bFixedLength)
			fseek(fp, iDataStartOffset, 0);
		// Need to scan over all the profiles, accumulating the TRUE
		// extents of the actual data points.
		int record = 0;
		int data_len;
		for (i = 0; i < iProfiles; i++)
		{
			if (bFixedLength)
				fseek(fp, iDataStartOffset + (record * 1024), 0);
			fscanf(fp, "%d", &iRow);
			fscanf(fp, "%d", &iColumn);
			fscanf(fp, "%d", &iProfileRows);
			fscanf(fp, "%d", &iProfileCols);
			start.x = DConvert(fp, 24);
			start.y = DConvert(fp, 24);
			m_EarthExtents.GrowToContainPoint(start);
			start.y += (iProfileRows * dydelta);
			m_EarthExtents.GrowToContainPoint(start);

			if (bFixedLength)
			{
				record++;
				data_len = 144 + (iProfileRows * 6);
				while (data_len > 1020)	// max bytes in a record
				{
					data_len -= 1020;
					record++;
				}
			}
			else
			{
				dLocalDatumElev = DConvert(fp, 24);
				dProfileMin = DConvert(fp, 24);
				dProfileMax = DConvert(fp, 24);
				for (j = 0; j < iProfileRows; j++)
				{
					fscanf(fp, "%d", &iElev);
				}
			}
		}
		dMinY = m_EarthExtents.bottom;
	}

	// Compute number of rows
	if (bUTM)	// UTM
	{
		fRows = m_EarthExtents.Height() / dydelta;
		m_iRows = (int)(fRows + 0.5) + 1;	// round to the nearest integer
	}
	else	// degrees
	{
		fRows = m_EarthExtents.Height() * 1200.0f;
		m_iRows = (int)fRows + 1;	// 1 more than quad spacing
	}

	_AllocateArray();

	// jump to start of actual data
	fseek(fp, iDataStartOffset, 0);

	for (i = 0; i < iProfiles; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i*100/m_iColumns);

		fscanf(fp, "%d", &iRow);
		fscanf(fp, "%d", &iColumn);
		fscanf(fp, "%d", &iProfileRows);
		fscanf(fp, "%d", &iProfileCols);
		start.x = DConvert(fp, 24);
		start.y = DConvert(fp, 24);
		dLocalDatumElev = DConvert(fp, 24);
		dProfileMin = DConvert(fp, 24);
		dProfileMax = DConvert(fp, 24);

		ygap = (int)((start.y - dMinY)/dydelta);

		for (j = ygap; j < (ygap + iProfileRows); j++)
		{
//			assert(j < m_iRows);	// useful safety check
			fscanf(fp, "%d", &iElev);
			SetValue(i, j, iElev);
		}
	}
	fclose(fp);

	m_fVMeters = (float) (fVertUnits * dzdelta);
	ComputeHeightExtents();

	return true;
}


/** Loads from a netCDF file.
 * Elevation values are assumed to be integer meters.  Projection is
 * assumed to be geographic.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromCDF(const char *szFileName,
								void progress_callback(int))
{
#if SUPPORT_NETCDF
	int id;

	/* open existing netCDF dataset */
	int status = nc_open(szFileName, NC_NOWRITE, &id);
	if (status != NC_NOERR)
		return false;

	if (progress_callback != NULL) progress_callback(0);

	// get dimension IDs
	int id_side, id_xysize;
	nc_inq_dimid(id, "side", &id_side);
	nc_inq_dimid(id, "xysize", &id_xysize);
	size_t xysize_length;
	nc_inq_dimlen(id, id_xysize, &xysize_length);

	// get variable IDs
	int id_xrange, id_yrange, id_zrange;
	int id_spacing, id_dimension, id_z;
	nc_inq_varid(id, "x_range", &id_xrange);
	nc_inq_varid(id, "y_range", &id_yrange);
	nc_inq_varid(id, "z_range", &id_zrange);
	nc_inq_varid(id, "spacing", &id_spacing);
	nc_inq_varid(id, "dimension", &id_dimension);
	nc_inq_varid(id, "z", &id_z);

	// get values of variables
	double xrange[2], yrange[2], zrange[2], spacing[2];
	int dimension[2];
	nc_get_var_double(id, id_xrange, xrange);
	nc_get_var_double(id, id_yrange, yrange);
	nc_get_var_double(id, id_zrange, zrange);
	nc_get_var_double(id, id_spacing, spacing);
	nc_get_var_int(id, id_dimension, dimension);

	double *z = new double[xysize_length];
	if (progress_callback != NULL) progress_callback(20);

	nc_get_var_double(id, id_z, z);
	if (progress_callback != NULL) progress_callback(60);

	nc_close(id);				// close netCDF dataset

	// Now copy the values into the vtElevationGrid object
	m_iColumns = dimension[0];
	m_iRows = dimension[1];

	m_bFloatMode = false;
	_AllocateArray();
	if (progress_callback != NULL) progress_callback(80);

	int i, j;
	for (i = 0; i < m_iColumns; i++)
	{
		for (j = 0; j < m_iRows; j++)
		{
			SetValue(i, m_iRows-1-j, (short)z[j*m_iColumns+i]);
		}
	}
	if (progress_callback != NULL) progress_callback(90);

	m_proj.SetProjectionSimple(false, 0, EPSG_DATUM_WGS84);

	m_EarthExtents.left = xrange[0];
	m_EarthExtents.right = xrange[1];
	m_EarthExtents.top = yrange[1];
	m_EarthExtents.bottom = yrange[0];

	ComputeCornersFromExtents();

	// delete temporary storage
	delete z;

	return true;
#else
	// no support for netCDF
	return false;
#endif
}


/** Loads from a Arc/Info compatible ASCII grid file.
 * Projection is read from a corresponding .prj file.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromASC(const char *szFileName,
								void progress_callback(int))
{
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;

	if (progress_callback != NULL) progress_callback(0);
	char buf[4000];

	// get dimension IDs
	fgets(buf, 4000, fp);

	int ncols, nrows, xllcorner, yllcorner, cellsize, nodata;
	int result = sscanf(buf, "ncols %d", &ncols);
	if (result != 1)
		return false;	// not an ASC file

	fgets(buf, 4000, fp);
	sscanf(buf, "nrows %d", &nrows);

	fgets(buf, 4000, fp);
	sscanf(buf, "xllcorner %d", &xllcorner);

	fgets(buf, 4000, fp);
	sscanf(buf, "yllcorner %d", &yllcorner);

	fgets(buf, 4000, fp);
	sscanf(buf, "cellsize %d", &cellsize);

	fgets(buf, 4000, fp);
	sscanf(buf, "NODATA_value %d", &nodata);

	m_iColumns = ncols;
	m_iRows = nrows;

	m_proj.SetProjectionSimple(true, 1, EPSG_DATUM_WGS84);

	m_bFloatMode = false;

	m_EarthExtents.left = xllcorner;
	m_EarthExtents.right = xllcorner + (ncols - 1) * cellsize;
	m_EarthExtents.top = yllcorner + (nrows - 1) * cellsize;
	m_EarthExtents.bottom = yllcorner;

	ComputeCornersFromExtents();
	_AllocateArray();

	int i, j, z;
	for (i = 0; i < nrows; i++)
	{
		if (progress_callback != NULL) progress_callback(i*100/nrows);
		for (j = 0; j < ncols; j++)
		{
			fscanf(fp, "%d", &z);
			if (z != nodata)
				SetValue(j, nrows-1-i, (short)z);
		}
	}
	fclose(fp);
	return true;
}


/** Loads from a Terragen Terrain file.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromTerragen(const char *szFileName,
								void progress_callback(int))
{
	char buf[8];
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;

	// get file identifier
	fread(buf, 8, 1, fp);
	if (strncmp(buf, "TERRAGEN", 8))
		return false;
	fread(buf, 8, 1, fp);
	if (strncmp(buf, "TERRAIN ", 8))
		return false;

	if (progress_callback != NULL) progress_callback(0);

	m_proj.SetProjectionSimple(true, 1, EPSG_DATUM_WGS84);

	m_bFloatMode = false;

	short svalue, dummy;
	FPoint3 scale(30.0f, 30.0f, 30.0f);
	float fvalue;
	int i, j;

	while (fread(buf, 4, 1, fp) == 1)
	{
		if (!strncmp(buf, "SIZE", 4))
		{
			fread(&svalue, 2, 1, fp);
			fread(&dummy, 2, 1, fp);
			m_iRows = m_iColumns = svalue + 1;
		}
		else if (!strncmp(buf, "XPTS", 4))
		{
			fread(&svalue, 2, 1, fp);
			fread(&dummy, 2, 1, fp);
			m_iColumns = svalue;
		}
		else if (!strncmp(buf, "YPTS", 4))
		{
			fread(&svalue, 2, 1, fp);
			fread(&dummy, 2, 1, fp);
			m_iRows = svalue;
		}
		else if (!strncmp(buf, "SCAL", 4))
		{
			fread(&scale.x, 4, 1, fp);
			fread(&scale.y, 4, 1, fp);
			fread(&scale.z, 4, 1, fp);
		}
		else if (!strncmp(buf, "CRAD", 4))
		{
			// radius of planet
			fread(&fvalue, 4, 1, fp);
		}
		else if (!strncmp(buf, "CRVM", 4))
		{
			// "curve mode"
			fread(&svalue, 2, 1, fp);
			fread(&dummy, 2, 1, fp);
		}
		else if (!strncmp(buf, "ALTW", 4))
		{
			short HeightScale;
			short BaseHeight;
			fread(&HeightScale, 2, 1, fp);
			fread(&BaseHeight, 2, 1, fp);

			_AllocateArray();
			for (j = 0; j < m_iRows; j++)
			{
				if (progress_callback != NULL) progress_callback(j*100/m_iRows);
				for (i = 0; i < m_iColumns; i++)
				{
					fread(&svalue, 2, 1, fp);
					SetFValue(i, j, scale.z * (BaseHeight + ((float)svalue * HeightScale / 65536.0f)));
				}
			}
		}
		else if (!strncmp(buf, "EOF ", 4))
		{
			// do nothing
		}
	}
	fclose(fp);

	// make up some extents, based on the scaling
	m_EarthExtents.left = 0;
	m_EarthExtents.right = (m_iColumns - 1) * scale.x;
	m_EarthExtents.top = (m_iRows - 1) * scale.y;
	m_EarthExtents.bottom = 0;
	ComputeCornersFromExtents();

	return true;
}


//
// Helper for DTED reader: read an 8-byte number in the
// form DDDMMSSH, degrees-minute-seconds-hemisphere
//
float get_dms8(FILE *fp)
{
	float f;
	char hem;
	float degrees, minutes, seconds;

	char buf[8];
	fread(buf, 8, 1, fp);

	hem = buf[7];
	buf[7] = '\0';
	seconds = (float)atof(buf + 5);
	buf[5] = '\0';
	minutes = (float)atof(buf + 3);
	buf[3] = '\0';
	degrees = (float)atof(buf);

	f = degrees + minutes/60 + seconds/3600;
	if (hem == 'W') f = -f;

	return f;
}

//
// Helper for DTED reader: read an 4-byte number in the
// return as integer
//
int get_dddd(FILE *fp)
{
	int i;

	char buf[5];
	buf[4] = '\0';
	fread(buf, 4, 1, fp);
	i = atoi(buf);

	return i;
}

//
// Helper for DTED reader: read an 4-byte number in the
// form SSSS, decimal seconds
// return as decimal degree
//
float get_ssss(FILE *fp)
{
	float f;

	f = (float)get_dddd(fp);
	f = f/36000;

	return f;
}

/** Loads from a DTED file.
 * \par
 * Should support DTED0, DTED1 and DTED2 files, although it has only been
 * tested on DTED0.  Projection is assumed to be geographic and elevation
 * is integer meters.
 * \returns \c true if the file was successfully opened and read.
 */
//
// DTED chunks/bytes:
//  UHL 80
//  DSI 648
//  ACC 2700
//  Data (variable)
//
bool vtElevationGrid::LoadFromDTED(const char *szFileName,
								 void progress_callback(int))
{
	FILE *fp;
	char buf[80];

	if (!(fp = fopen(szFileName, "rb")))
	{
		// Cannot Open File
		return false;
	}

	// all DTEDs are geographic and in integer meters
	// datum is always WGS84
	m_proj.SetProjectionSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	// check for correct format
	fread(buf, 4, 1, fp);
	if (strncmp(buf, "UHL1", 4))
	{
		// Not a DTED file
		fclose(fp);
		return false;
	}

	float xInterval = get_ssss(fp);
	float yInterval = get_ssss(fp);

	// get dimensions
	fseek(fp, 47, 0);
	m_iColumns = get_dddd(fp);
	m_iRows = get_dddd(fp);

	m_Corners[0].x = get_dms8(fp);
	m_Corners[0].y = get_dms8(fp);

	// imply other corners
	m_Corners[1].x = m_Corners[0].x;
	m_Corners[1].y = m_Corners[0].y + yInterval * (m_iRows - 1);
	m_Corners[2].x = m_Corners[0].x + xInterval * (m_iColumns - 1);
	m_Corners[2].y = m_Corners[0].y + yInterval * (m_iRows - 1);
	m_Corners[3].x = m_Corners[0].x + xInterval * (m_iColumns - 1);
	m_Corners[3].y = m_Corners[0].y;
	ComputeExtentsFromCorners();

	_AllocateArray();

	int line_length = 12 + 2 * m_iRows;
	unsigned char *linebuf = new unsigned char[line_length];
	unsigned char swap[2];

	fseek(fp, 80 + 648 + 2700, 0);
	int i, j, offset;
	for (i = 0; i < m_iColumns; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / m_iColumns);

		/*  FIXME:  there be byte order issues here.  See below in this routine.  */
		fread(linebuf, line_length, 1, fp);
		if (*linebuf != 0xaa)
		{
			break;
		}
		offset = 8;
		for (j = 0; j < m_iRows; j++)
		{
			swap[1] = *(linebuf + offset++);
			swap[0] = *(linebuf + offset++);
			int z = *((unsigned short *)swap);
			SetValue(i, j, z);
		}
	}
	delete linebuf;
	fclose(fp);
	return true;
}


/** Loads from a GTOPO30 file.
 * \par
 * GTOPO30 files are actually composed of at least 2 files, a header with a
 * .hdr extension and data with a .dem extension.  Pass the filename of
 * the .hdr file to this function, and it will automatically look for
 * a corresponding .dem file in the same location.
 * \par
 * Projection is always geographic and elevation is integer meters.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromGTOPO30(const char *szFileName,
									void progress_callback(int))
{
	if (progress_callback != NULL) progress_callback(1);

	// Open the header file
	ifstream hdrFile(szFileName);
	if (!hdrFile.is_open())
	  return false;

	// Parse the file
	char strName[30];
	char strValue[30];

	GTOPOHeader gh;

	// Read the byte order
	hdrFile >> strName >> gh.ByteOrder;

	// Read the layout
	hdrFile >> strName >> gh.Layout;
	// Read the number of rows
	hdrFile >> strName >> strValue;
	gh.NumRows = atol(strValue);

	// Read the number of columns
	hdrFile >> strName >> strValue;
	gh.NumCols = atol(strValue);

	// Read the number of bands
	hdrFile >> strName >> gh.Bands;

	// Read the number of bits per pixel
	hdrFile >> strName >> gh.Bits;

	// Read the band row bytes
	hdrFile >> strName >> gh.BandRowBytes;

	// Read the total row bytes
	hdrFile >> strName >> gh.TotalRowBytes;

	// Read the band gap bytes
	hdrFile >> strName >> gh.BandGapBytes;

	// Read the no data value
	hdrFile >> strName >> strValue;
	gh.NoData = atoi(strValue);

	// Read the upper left x coordinate
	hdrFile >> strName >> strValue;
	gh.ULXMap = atof(strValue);

	// Read the upper left y coordinate
	hdrFile >> strName >> strValue;
	gh.ULYMap = atof(strValue);

	// Read the x pixel spacing
	hdrFile >> strName >> strValue;
	gh.XDim = atof(strValue);

	// Read the y pixel spacing
	hdrFile >> strName >> strValue;
	gh.YDim = atof(strValue);

	// Close the file
	hdrFile.close();

	// make the corresponding filename for the DEM
	char dem_fname[200];
	strcpy(dem_fname, szFileName);
	char *ext = strrchr(dem_fname, '.');
	if (!ext)
		return false;
	strcpy(ext, ".dem");
	FILE *fp = fopen(dem_fname, "rb");
	if (!fp)
		return false;

	if (progress_callback != NULL) progress_callback(5);

	// Projection is always geographic, integer
	m_proj.SetProjectionSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	m_EarthExtents.left = gh.ULXMap;
	m_EarthExtents.top = gh.ULYMap;
	m_EarthExtents.right = m_EarthExtents.left + (gh.XDim * (gh.NumCols-1));
	m_EarthExtents.bottom = m_EarthExtents.top - (gh.YDim * (gh.NumRows-1));
	ComputeCornersFromExtents();

	// set up for an array of the indicated size
	m_iColumns = gh.NumCols;
	m_iRows = gh.NumRows;

	_AllocateArray();

	// read the file
	int i, j;
	short z;
	char *cp = (char *)&z, temp;
	for (j = 0; j < m_iRows; j++)
	{
		if (progress_callback != NULL) progress_callback(10 + j * 90 / m_iRows);
		for (i = 0; i < m_iColumns; i++)
		{
			/*  FIXME:  there be byte order issues here.  See below in this routine.  */
			fread(&z, sizeof(short), 1, fp);
			// must swap byte order
			temp = cp[0];
			cp[0] = cp[1];
			cp[1] = temp;
			SetValue(i, m_iRows-1-j, (z == gh.NoData) ? 0 : z);
		}
	}
	return true;
}


/** Loads from a Surfer binary grid file (GRD)
 * \par
 * Projection is always geographic and elevation is floating-point.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromGRD(const char *szFileName,
								void progress_callback(int))
{
	if (progress_callback != NULL) progress_callback(1);

	// Open the header file
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
	  return false;

	// Parse the file
	char szHeader[5];
	fread(szHeader, 4, 1, fp);
	if (strncmp(szHeader, "DSBB", 4))
	{
		// not the right kind of file
		fclose(fp);
		return false;
	}

	// read GRD header data
	short nx, ny;
	double xlo, xhi, ylo, yhi, zlo, zhi;
	/*  FIXME:  there be byte order issues here.  See below in this routine.  */
	fread(&nx, 2, 1, fp);
	fread(&ny, 2, 1, fp);
	fread(&xlo, 8, 1, fp);
	fread(&xhi, 8, 1, fp);
	fread(&ylo, 8, 1, fp);
	fread(&yhi, 8, 1, fp);
	fread(&zlo, 8, 1, fp);
	fread(&zhi, 8, 1, fp);

	// Set the projection (actually we don't know it)
	m_proj.SetProjectionSimple(true, 1, EPSG_DATUM_WGS84);

	// set the corresponding vtElevationGrid info
	m_bFloatMode = true;
	m_EarthExtents.left = xlo;
	m_EarthExtents.top = yhi;
	m_EarthExtents.right = xhi;
	m_EarthExtents.bottom = ylo;
	ComputeCornersFromExtents();

	m_iColumns = nx;
	m_iRows = ny;

	_AllocateArray();

	int x, y;
	float z;
	for (y = 0; y < ny; y++)
	{
		if (progress_callback != NULL) progress_callback(y * 100 / ny);
		for (x = 0; x < nx; x++)
		{
			fread(&z, 4, 1, fp);
			SetFValue(x, y, z);
		}
	}
	fclose(fp);
	return true;
}


/** Loads from a PGM (Portable Gray Map) file.
 * Both PGM Binary and ASCII varieties are supported.
 * \par
 * PGM does not contain any information about geographic location, so
 * the following assumptions are made: UTM coordinates, 1-meter spacing,
 * origin at (0,0).
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromPGM(const char *szFileName, void progress_callback(int))
{
	int xsize,ysize;
	char sbuf[512],			/* buffer for file input */
	dummy[2];				/* used for \n and \0   */
	char *junk;				/* unconverted part of a number */
	double a, maxval;
	FILE *fpin;				/* input file pointer */
	bool bBinary;			/* PGM binary format flag */
	unsigned char oneb;		/* one byte from file */

	/* open input file */
	if ((fpin = fopen(szFileName, "rb")) == NULL)
	{
		// "Could not open input file"
		return false;
	}

	if (fread(sbuf, sizeof(char), 2, fpin) != 2)
	{
		// "Could not read file"
		return false;
	}

	bBinary = false;	/* assume PGM ascii */
	if (strncmp(sbuf,"P5",2) == 0)
		bBinary = 1;	/* PGM binary format */

	if (strncmp(sbuf,"GI",2) == 0) {
		fclose(fpin);
		return false;
	}
	else if ((strncmp(sbuf,"P5",2) != 0) && (strncmp(sbuf,"P2",2) != 0))
	{
		/* not PGM Ascii */
		fclose(fpin);
		return false;
	}

	/* read PGM ASCII or binary file */
	while ((fscanf(fpin, "%s", sbuf) != EOF) && sbuf[0] == '#')
	{
		// comment
		fscanf(fpin,"%[^\n]", sbuf);  /* read comment beyond '#' */
		fscanf(fpin,"%[\n]", dummy);  /* read newline */
	}

	xsize = atoi(sbuf);				/* store xsize of array */
	fscanf(fpin,"%s",sbuf);			/* read ysize of array */
	ysize = atoi(sbuf);
	fscanf(fpin,"%s\n",sbuf);		/* read maxval of array */
	maxval = strtod(sbuf, &junk);	/* store maxval. could throw away. */

	// Set the projection (actually we don't know it)
	m_proj.SetProjectionSimple(true, 1, EPSG_DATUM_WGS84);

	// set the corresponding vtElevationGrid info
	m_bFloatMode = true;
	m_EarthExtents.left = 0;
	m_EarthExtents.top = ysize;
	m_EarthExtents.right = xsize;
	m_EarthExtents.bottom = 0;
	ComputeCornersFromExtents();

	m_iColumns = xsize;
	m_iRows = ysize;

	_AllocateArray();

	if (bBinary)
	{
		/* read PGM binary */
		for (int j = 0; j < ysize; j++)
		{
			if (progress_callback != NULL) progress_callback(j * 100 / ysize);
			for (int i = 0; i < xsize; i++)
			{
				fread(&oneb, sizeof(unsigned char), 1, fpin);
				SetFValue(i, ysize-1-j, oneb);
			}
		}
	}
	else
	{
		/* read PGM ASCII */
		for (int j = 0; j < ysize; j++)
		{
			if (progress_callback != NULL) progress_callback(j * 100 / ysize);
			for (int i = 0; i < xsize; i++)
			{
				fscanf(fpin, "%s", sbuf);
				a = strtod(sbuf, &junk);
				SetFValue(i, ysize-1-j, (float)a);
			}
		}
	}
	fclose(fpin);
	return true;
}


/** Loads just the header information from a BT (Binary Terrain) file.
 * \par
 * This can be useful if you want to check the information such as the
 * size of the data, without reading the entire file.
 * \returns \c true if the header was successfully parsed.
 */
bool vtElevationGrid::LoadBTHeader(const char *szFileName)
{
	short svalue, proj_type, zone, datum, external;
	int ivalue;
	char buf[11];
	float ftmp;

	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;		// Cannot Open File

	fread(buf, 10, 1, fp);
	buf[10] = '\0';

	if (strncmp(buf, "binterr", 7))
	{
		fclose(fp);
		return false;		// Not a current BT file
	}

	float version;
	sscanf(buf+7, "%f", &version);

	/*  NOTE:  BT format is little-endian  */
	FRead(&m_iColumns, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
	FRead(&m_iRows,	   DT_INT, 1, fp, BO_LITTLE_ENDIAN);

	// Default to internal projection
	external = 0;

	if (version == 1.0f)
	{
		// data size
		FRead(&ivalue, DT_INT, 1, fp, BO_LITTLE_ENDIAN);

		// UTM flag
		FRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		proj_type = (svalue == 1);

		// UTM zone
		FRead(&zone, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// 1.0 didn't support Datum, so assume WGS84
		datum = EPSG_DATUM_WGS84;

		// coordinate extents left-right
		FRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.left = ftmp;
		FRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.right = ftmp;

		// coordinate extents bottom-top
		FRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.bottom = ftmp;
		FRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.top = ftmp;

		// is the data floating point or integers?
		FRead(&m_bFloatMode, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
		if (m_bFloatMode != true)
			m_bFloatMode = false;
	}
	else if (version == 1.1f || version == 1.2f || version == 1.3f)
	{
		// data size
		FRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// Is floating point data?
		FRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		m_bFloatMode = (svalue == 1);

		// Projection (0 = geo, 1 = utm, 2 = feet, 3 = u.s. feet)
		FRead(&proj_type, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// UTM zone (ignore unless projection == 1)
		FRead(&zone, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// Datum (ignore unless projection == 0 or 1)
		FRead(&datum, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// coordinate extents
		FRead(&m_EarthExtents.left, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		FRead(&m_EarthExtents.right, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		FRead(&m_EarthExtents.bottom, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		FRead(&m_EarthExtents.top, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);

		// External projection flag
		FRead(&external, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
	}
	if (version == 1.3f)
	{
		FRead(&m_fVMeters, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
	}

	// Set up projection
	if (external == 1)
	{
		// Read external projection (.prj) file
		if (!m_proj.ReadProjFile(szFileName))
			return false;
	}
	else
	{
		// Internal specification: proj_type 0 = Geo, 1 = UTM
		m_proj.SetProjectionSimple(proj_type == 1, zone, datum);
	}

	ComputeCornersFromExtents();

	fclose(fp);
	return true;
}


/** Loads from a BT (Binary Terrain) file.
 * \par
 * Both the current version (1.1) and older BT versions are supported.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromBT(const char *szFileName, void progress_callback(int))
{
	int i;

	// First load the header
	if (!LoadBTHeader(szFileName))
		return false;

	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;		// Cannot Open File

	// elevation data always starts at offset 256
	fseek(fp, 256, SEEK_SET);

	_AllocateArray();

#if 0
	// slow way
	int value;
	float fvalue;
	for (i = 0; i < m_iColumns; i++)
	{
		if (progress_callback != NULL) progress_callback(i * 100 / m_iColumns);
		for (j = 0; j < m_iRows; j++)
		{
			if (m_bFloatMode) {
				FRead(&fvalue, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
				SetFValue(i, j, fvalue);
			}
			else
			{
				FRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
				SetValue(i, j, svalue);
			}
		}
	}
#else
	// fast way
	if (m_bFloatMode)
	{
		for (i = 0; i < m_iColumns; i++)
		{
			if (progress_callback != NULL) progress_callback(i * 100 / m_iColumns);
			FRead(m_pFData + (i*m_iRows), DT_FLOAT, m_iRows, fp, BO_LITTLE_ENDIAN);
		}
	}
	else
	{
		for (i = 0; i < m_iColumns; i++)
		{
			if (progress_callback != NULL) progress_callback(i * 100 / m_iColumns);
			FRead(m_pData + (i*m_iRows), DT_SHORT, m_iRows, fp, BO_LITTLE_ENDIAN);
		}
	}
#endif

	// be sure to close the file!
	fclose(fp);

	ComputeHeightExtents();
	return true;
}


/**
 * Writes the grid to a TerraGen Terrain file.
 * \param szFileName The filename to write to.
 */
bool vtElevationGrid::SaveToTerragen(const char *szFileName)
{
	FILE *fp = fopen(szFileName, "wb");
	if (!fp)
		return false;

	short w = m_iColumns;
	short h = m_iRows;
	short dummy = 0;

	// write identifying header
	fwrite("TERRAGEN", 8, 1, fp);
	fwrite("TERRAIN ", 8, 1, fp);

	// write dimension
	fwrite("SIZE", 4, 1, fp);
	fwrite(&w, 2, 1, fp);
	fwrite(&dummy, 2, 1, fp);

	fwrite("XPTS", 4, 1, fp);
	fwrite(&w, 2, 1, fp);
	fwrite(&dummy, 2, 1, fp);

	fwrite("YPTS", 4, 1, fp);
	fwrite(&h, 2, 1, fp);
	fwrite(&dummy, 2, 1, fp);

	// write "scale"
	// this is actually the grid spacing (x, y) and elevation precision (z)
	fwrite("SCAL", 4, 1, fp);
	FPoint2 spacing = GetSpacing();
	float zscale = 1.0f;
	fwrite(&spacing.x, 4, 1, fp);
	fwrite(&spacing.y, 4, 1, fp);
	fwrite(&zscale, 4, 1, fp);

	// don't write "CRAD" and "CRVM"
	// they are optional and would not have meaningful values

	// write altitude (elevation data)
	fwrite("ALTW", 4, 1, fp);
	short HeightScale = 16384;		// divided by 65526, means units of 1/4 meter
	short BaseHeight = 0;
	fwrite(&HeightScale, 2, 1, fp);
	fwrite(&BaseHeight, 2, 1, fp);

	int i, j;
	float fvalue;
	short svalue;
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			fvalue = GetFValue(i, j);
			svalue = (short) (fvalue * 4.0f);
			fwrite(&svalue, 2, 1, fp);
		}
	}
	fclose(fp);
	return true;
}


/**
 * Writes the grid to a BT (Binary Terrain) file.
 * The current BT format version (1.3) is written.
 *
 * \param szFileName		The file name to write to.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 */
bool vtElevationGrid::SaveToBT(const char *szFileName, void progress_callback(int))
{
	FILE *fp = fopen(szFileName, "wb");
	if (!fp)
		return false;

	int w = m_iColumns;
	int h = m_iRows;
	short zone = m_proj.GetUTMZone();
	short datum = (short) m_proj.GetDatum();
	short isfloat = (short) IsFloatMode();
	short external = 1;		// always true: we always write an external .prj file

	LinearUnits units = m_proj.GetUnits();
	int hunits = (int) units;

	// Latest header, version 1.2
	short datasize = m_bFloatMode ? 4 : 2;

	fwrite("binterr1.3", 10, 1, fp);
	fwrite(&w, 4, 1, fp);
	fwrite(&h, 4, 1, fp);
	fwrite(&datasize, 2, 1, fp);
	fwrite(&isfloat, 2, 1, fp);
	fwrite(&hunits, 2, 1, fp);		// Horizontal Units (0, 1, 2, 3)
	fwrite(&zone, 2, 1, fp);		// UTM zone
	fwrite(&datum, 2, 1, fp);		// Datum

	// coordinate extents
	fwrite(&m_EarthExtents.left, 8, 1, fp);
	fwrite(&m_EarthExtents.right, 8, 1, fp);
	fwrite(&m_EarthExtents.bottom, 8, 1, fp);
	fwrite(&m_EarthExtents.top, 8, 1, fp);

	fwrite(&external, 2, 1, fp);	// External projection specification
	fwrite(&m_fVMeters, 4, 1, fp);	// External projection specification

	// now write the data: always starts at offset 256
	fseek(fp, 256, SEEK_SET);

#if 0
	// slow way, one heixel at a time
	for (int i = 0; i < w; i++)
	{
		if (progress_callback != NULL) progress_callback(i * 100 / w);
		for (j = 0; j < h; j++)
		{
			if (m_bFloatMode) {
				fvalue = GetFValue(i, j);
				fwrite(&fvalue, datasize, 1, fp);
			} else {
				svalue = GetValue(i, j);
				fwrite(&svalue, datasize, 1, fp);
			}
		}
	}
#else
	// fast way, with the assumption that the data is stored column-first in memory
	if (m_bFloatMode)
	{
		for (int i = 0; i < w; i++)
		{
			if (progress_callback != NULL) progress_callback(i * 100 / w);
			fwrite(m_pFData + (i * m_iRows), 4, m_iRows, fp);
		}
	}
	else
	{
		for (int i = 0; i < w; i++)
		{
			if (progress_callback != NULL) progress_callback(i * 100 / w);
			fwrite(m_pData + (i * m_iRows), 2, m_iRows, fp);
		}
	}
#endif
	fclose(fp);

	if (external)
	{
		// Write external projection file (.prj)
		char prj_name[256];
		strcpy(prj_name, szFileName);
		strcpy(prj_name + strlen(prj_name) - 3, ".prj");
		m_proj.WriteProjFile(prj_name);
	}

	return true;
}


/**
 * Loads an elevation grid using the GDAL library.  The GDAL library
 * supports a very large number of formats, including SDTS-DEM.  See
 * http://www.remotesensing.org/gdal/formats_list.html for the complete list
 * of GDAL-supported formats.
 *
 * \param szFileName The file name to read from.
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 *
 * \returns True if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadWithGDAL(const char *szFileName,
								   void progress_callback(int))
{
	GDALDataset  *poDataset;

	GDALAllRegister();

	poDataset = (GDALDataset *) GDALOpen(szFileName, GA_ReadOnly);
	if (poDataset == NULL)
	{
		// failed.
		return false;
	}
	m_iColumns = poDataset->GetRasterXSize();
	m_iRows = poDataset->GetRasterYSize();

	// Get the projection information
	const char *str1 = poDataset->GetProjectionRef();
	char *str2 = (char *) str1;
	OGRErr err = m_proj.importFromWkt(&str2);
	if (err == OGRERR_CORRUPT_DATA)
	{
		// just assume that it's geographic
		m_proj.SetWellKnownGeogCS("WGS84");
	}

	// Get spacing and extents
	double		adfGeoTransform[6];
	if (poDataset->GetGeoTransform(adfGeoTransform) != CE_None)
		return false;

	// Upper left corner is adfGeoTransform[0], adfGeoTransform[3]
	m_EarthExtents.left = adfGeoTransform[0];
	m_EarthExtents.top = adfGeoTransform[3];
	m_EarthExtents.right = m_EarthExtents.left + (adfGeoTransform[1] * m_iColumns);
	m_EarthExtents.bottom = m_EarthExtents.top + (adfGeoTransform[5] * m_iRows);
	ComputeCornersFromExtents();

	// Raster count should be 1 for elevation datasets
	int rc = poDataset->GetRasterCount();

	GDALRasterBand *poBand = poDataset->GetRasterBand(1);

	// Assume block size will be one raster line
//	int			 nBlockXSize, nBlockYSize;
//	poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);

	// Check data type - it's either integer or float
	GDALDataType rtype = poBand->GetRasterDataType();
	if (rtype == GDT_Int16 || rtype == GDT_Byte)
		m_bFloatMode = false;
	else
		m_bFloatMode = true;

	// Check vertical units - if they're feet, scale to meters
	float fScale = 1.0f;
	const char *runits = poBand->GetUnitType();
	if (runits && !strcmp(runits, "ft"))
	{
		// feet to meter conversion
		if (m_bFloatMode)
			fScale = 0.3048f;	// already floats, just multiply
		else
			SetScale(0.3048f);	// stay with shorts, use scaling
	}

	_AllocateArray();

	short *pasScanline;
	short elev;
	float *pafScanline;
	float fElev;
	int   nXSize = poBand->GetXSize();

	pasScanline = (short *) CPLMalloc(sizeof(short)*nXSize);
	pafScanline = (float *) CPLMalloc(sizeof(float)*nXSize);
	int i, j;
	for (j = 0; j < m_iRows; j++)
	{
		if (m_bFloatMode)
		{
			poBand->RasterIO(GF_Read, 0, j, nXSize, 1,
							  pafScanline, nXSize, 1, GDT_Float32,
							  0, 0);
			for (i = 0; i < nXSize; i++)
			{
				fElev = pafScanline[i];

				// check for several different commonly used values meaning
				// "no data at this location"
				if (fElev == -9999 || fElev == -32766 || fElev == 32767 || fElev < -100000)
					SetValue(i, m_iRows-1-j, INVALID_ELEVATION);
				else
					SetFValue(i, m_iRows-1-j, fElev * fScale);
			}
		}
		else
		{
			poBand->RasterIO(GF_Read, 0, j, nXSize, 1,
							  pasScanline, nXSize, 1, GDT_Int16,
							  0, 0);
			for (i = 0; i < nXSize; i++)
			{
				elev = pasScanline[i];

				// check for several different commonly used values meaning
				// "no data at this location"
				if (elev == -9999 || elev == -32766 || elev == 32767)
					SetValue(i, m_iRows-1-j, INVALID_ELEVATION);
				else
					SetFValue(i, m_iRows-1-j, elev * fScale);
			}
		}
		if (progress_callback != NULL)
			progress_callback(100*j/m_iRows);
	}

	// Clean up
	CPLFree(pasScanline);
	CPLFree(pafScanline);
	delete poDataset;

	// Return success
	return true;
}

/**
 * Loads from a RAW file (a naked array of elevation values).
 * The file will not contain any information about at all about data size,
 * data type, or projection, so this information must be passed in as
 * arguments.
 *
 * \param szFileName	The file name to read from.
 * \param width			The width of the expected array.
 * \param height		The height of the expected array.
 * \param bytes_per_element The number of bytes for each elevation value.  If
 *		this is 1 or 2, the data is assumed to be integer.  If 4, then the
 *		data is assumed to be floating-point values.
 * \param vertical_units Indicates what scale factor to apply to the
 *		elevation values to convert them to meters.  E.g. if the file is in
 *		meters, pass 1.0, if in feet, pass 0.3048.
 * \param bBigEndian	True for Big-endian byte order, false for
 *		Little-endian (Intel byte order).
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromRAW(const char *szFileName, int width, int height,
								int bytes_per_element, float vertical_units,
								bool bBigEndian, void progress_callback(int))
{
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;

	m_iColumns = width;
	m_iRows = height;

	// set extents arbitrarily for now; if the user knows them, they can set
	// them after loading
	m_EarthExtents.left = 0;
	m_EarthExtents.top = height;
	m_EarthExtents.right = width;
	m_EarthExtents.bottom = 0;
	ComputeCornersFromExtents();

	m_proj.SetProjectionSimple(true, 1, EPSG_DATUM_WGS84);
	if (bytes_per_element == 4)
		m_bFloatMode = true;
	else
		m_bFloatMode = false;

	_AllocateArray();

	ByteOrder order;
	if (bBigEndian)
		order = BO_BIG_ENDIAN;
	else
		order = BO_LITTLE_ENDIAN;

	int i, j, z;
	void *data = &z;
	for (j = 0; j < m_iRows; j++)
	{
		if (progress_callback != NULL)
			progress_callback(100*j/m_iRows);
		for (i = 0; i < m_iColumns; i++)
		{
			if (bytes_per_element == 1)
			{
				fread(data, 1, 1, fp);
				SetValue(i, m_iRows-1-j, *((unsigned char *)data));
			}
			if (bytes_per_element == 2)
			{
				FRead(data, DT_SHORT, 1, fp, order);
				SetFValue(i, m_iRows-1-j, *((short *)data) * vertical_units);
			}
			if (bytes_per_element == 4)
			{
				FRead(data, DT_FLOAT, 1, fp, order);
				SetFValue(i, m_iRows-1-j, *((float *)data) * vertical_units);
			}
		}
	}

	// Clean up
	fclose(fp);

	SetScale(vertical_units);

	// Return success
	return true;
}


/** Loads from a MicroDEM format file.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromMicroDEM(const char *szFileName, void progress_callback(int))
{
	FILE *fp;
	int offset_to_header;
	int offset_to_data;
	char buf[40];
	short xsize, ysize;
	short max_elev, min_elev;
	char xspacing, yspacing;
	int utm_x_lowerleft;
	int utm_y_lowerleft;
	char elev_unit_type;
	char utm_zone;
	char dem_type;
	char spacing_unit;
	char hemi;
	short elev;
	char digitize_datum;
	double scalefactor;
	int i, j;

	/* open input file */
	if ((fp = fopen(szFileName, "rb")) == NULL)
	{
		// "Could not open input file"
		return false;
	}

	fread(buf, 9, 1, fp);
	if (strncmp(buf, "*MICRODEM", 9))
	{
		// not MicroDEM format
		fclose(fp);
		return false;
	}

	// Find offsets to header and data
	fseek(fp, 14, SEEK_SET);
	fread(buf, 5, 1, fp);
	buf[5] = '\0';
	offset_to_header = atoi(buf);
	fseek(fp, 36, SEEK_SET);
	fread(buf, 5, 1, fp);
	buf[5] = '\0';
	offset_to_data = atoi(buf);

	// Read header
	fseek(fp, offset_to_header, SEEK_SET);
	fread(&xsize, 2, 1, fp);
	fread(&ysize, 2, 1, fp);
	fread(buf, 3, 1, fp);			// "OSquareID"
	fread(&max_elev, 2, 1, fp);
	fread(&min_elev, 2, 1, fp);
	fread(&yspacing, 1, 1, fp);		// N-S data point spacing, in sec, min, or m
	fread(&xspacing, 1, 1, fp);		// E-W data point spacing, in sec, min, or m
	fread(buf, 20, 1, fp);			// unused
	fread(&utm_x_lowerleft, 4, 1, fp);
	fread(&utm_y_lowerleft, 4, 1, fp);
	fread(&elev_unit_type, 1, 1, fp);	// (Meters, Feet, TenthMgal,
		// Milligal, TenthGamma, Decimeters, Gammas, HundredthMGal, DeciFeet,
		// Centimeters, OtherElev, HundredthMa, HundredthPercentSlope,
		// Undefined, zDegrees, UndefinedHundredth
	fread(buf, 8, 1, fp);			// unused
	fread(&utm_zone, 1, 1, fp);
	fread(&dem_type, 1, 1, fp);		// 0 = UTM DEM, 1 = ArcSecond DEM
	fread(&spacing_unit, 1, 1, fp);	// Meters,Seconds,Minutes,KM,100m,Feet
		// KFeet,Degrees,HundredthSecond,MercProj100m,PolarStereo100m,100
	fread(&digitize_datum, 1, 1, fp);	// (WGS72, WGS84, NAD27, NAD83, Spherical, Local)
	fread(buf, 12, 1, fp);			// unused
	fread(&hemi, 1, 1, fp);			// hemisphere 'N' or 'S'
	// rest of header unused

	// Read data
	fseek(fp, offset_to_data, SEEK_SET);

	// Set the projection
	switch (digitize_datum)
	{
	case 0: //WGS72
		m_proj.SetProjectionSimple(dem_type==0, utm_zone, EPSG_DATUM_WGS72);
		break;
	case 1: //WGS84
		m_proj.SetProjectionSimple(dem_type==0, utm_zone, EPSG_DATUM_WGS84);
		break;
	case 2: //NAD27
		m_proj.SetProjectionSimple(dem_type==0, utm_zone, EPSG_DATUM_NAD27);
		break;
	case 3: //NAD83
		m_proj.SetProjectionSimple(dem_type==0, utm_zone, EPSG_DATUM_NAD83);
		break;
	case 4: //Spherical
	case 5: //Local
	default:
		m_proj.SetProjectionSimple(dem_type==0, utm_zone, EPSG_DATUM_WGS84);
		break;
	}

	// convert extents to degrees or metres
	switch (spacing_unit) //Meters,Seconds,Minutes,KM,100m, Feet,KFeet, Degrees,HundredthSecond,
	  // MercProj100m,  PolarStereo100m,10m,TenthSecond
	{
	case 0: // Metres
		scalefactor = 1.0;
		break;
	case 1: // Arc Seconds
		scalefactor = 3600.0;
		break;
	case 2: // Arc Minutes
		scalefactor = 60.0;
		break;
	case 3: // Kilometres
		scalefactor = 0.001;
		break;
	case 4: // 100 Metres
		scalefactor = 0.01;
		break;
	case 5: // Feet
		scalefactor = 0.3048;
		break;
	case 6: // 1000 Feet
		scalefactor = 0.0003048;
		break;
	case 7: // Degrees
		scalefactor = 1;
		break;
	case 8: // .01 Arc Second
		scalefactor = 360000.0;
		break;
	case 9: // Mercator projection 100M
		scalefactor = 0.01;
		break;
	case 10: // PolarStereo 100M
		scalefactor = 0.01;
		break;
	case 11: // .1 Arc Second
		scalefactor = 36000.0;
		break;
	default:
		scalefactor = 1;
		break;
	}

	m_EarthExtents.left = utm_x_lowerleft / scalefactor;
	m_EarthExtents.top = (utm_y_lowerleft + (ysize - 1) * yspacing) / scalefactor;
	m_EarthExtents.right = (utm_x_lowerleft + (xsize - 1) * xspacing) / scalefactor;
	m_EarthExtents.bottom = utm_y_lowerleft / scalefactor;

	ComputeCornersFromExtents();

	// set the corresponding vtElevationGrid info
	m_bFloatMode = false;

	m_iColumns = xsize;
	m_iRows = ysize;

	_AllocateArray();

	for (i = 0; i < xsize; i++)
	{
		if (progress_callback != NULL) progress_callback(i * 100 / xsize);
		for (j = 0; j < ysize; j++)
		{
			fread(&elev, 2, 1, fp);
			if (elev == 32767)
				SetValue(i, j, INVALID_ELEVATION);
			else
				SetValue(i, j, elev);
		}
	}
	fclose(fp);

	// Apply the vertical scaling factor *after* we have copied the raw
	// elevation values, since they are not meter values.
	switch (elev_unit_type)
	{
	case 0: // Meters
		break;
	case 1:	// Feet
		SetScale(0.3084f);
		break;
	case 5:	// Decimeters
		SetScale(0.1f);
		break;
	case 8:	// DeciFeet
		SetScale(0.3084f / 10);
		break;
	case 9:	// Centimeters
		SetScale(0.01f);
		break;
	}

	return true;
}


