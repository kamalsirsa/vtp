//
// vtElevationGridIO.cpp
//
// This modules contains the implementations of the file I/O methods of
// the class vtElevationGrid.
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include <fstream>
using namespace std;

#include "config_vtdata.h"
#include "vtLog.h"
#include "vtDIB.h"
#include "ElevationGrid.h"
#include "ByteOrder.h"
#include "vtString.h"
#include "FilePath.h"

#if SUPPORT_NETCDF
extern "C" {
#include "netcdf.h"
}
#endif

// GDAL
#include "gdal_priv.h"

// OGR
#include <ogrsf_frmts.h>

//	Header structure for a GTOPO30 DEM header file
typedef struct
{
	char			ByteOrder[30];		//	Byte order in which image pixel values are stored
										//		M = Motorola byte order (MSB first)
										//		I = Intel byte order (LSB first)
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


/**
 * Load from a file whose type is not known a priori.  This will end up
 * calling one of the Load* member functions.
 *
 * \return true if successful.
 */
bool vtElevationGrid::LoadFromFile(const char *szFileName,
								   void progress_callback(int))
{
	vtString FileExt = GetExtension(szFileName);

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

	if ((!FileExt.CompareNoCase(".bt")) || (!FileExt.CompareNoCase(".bt.gz")))
	{
		Success = LoadFromBT(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".dem"))
	{
		if (FirstChar == '*')
			Success = LoadFromMicroDEM(szFileName, progress_callback);
		else
			Success = LoadFromDEM(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".asc"))
	{
		// GDAL's ASC reader has trouble on some machines
//		Success = LoadWithGDAL(szFileName, progress_callback);

		Success = LoadFromASC(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".bil"))
	{
		Success = LoadWithGDAL(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".ter"))
	{
		Success = LoadFromTerragen(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".cdf"))
	{
		Success = LoadFromCDF(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".hdr"))
	{
		Success = LoadFromGTOPO30(szFileName, progress_callback);
		if (!Success)
		{
			// might be NOAA GLOBE header
			Success = LoadFromGLOBE(szFileName, progress_callback);
		}
	}
	else if (!FileExt.CompareNoCase(".dte") ||
			 !FileExt.CompareNoCase(".dt0") ||
			 !FileExt.CompareNoCase(".dt1") ||
			 !FileExt.CompareNoCase(".dt2"))
	{
		Success = LoadFromDTED(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".pgm"))
	{
		Success = LoadFromPGM(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".grd"))
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
	else if (!FileExt.CompareNoCase(".catd.ddf") ||
			 !FileExt.CompareNoCase(".tif") ||
			 !FileExt.CompareNoCase(".tiff") ||
			 !FileExt.CompareNoCase(".png") ||
			 !FileExt.CompareNoCase(".adf"))
	{
		Success = LoadWithGDAL(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".hgt"))
	{
		Success = LoadFromHGT(szFileName, progress_callback);
	}
	return Success;
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
	int id_side = 0, id_xysize = 0;
	nc_inq_dimid(id, "side", &id_side);
	status = nc_inq_dimid(id, "xysize", &id_xysize);
	if (status != NC_NOERR)
	{
		// Error messages can be turned into strings with nc_strerror
		VTLOG("Could not determine size of CDF file. Error: ");
		VTLOG(nc_strerror(status));
		nc_close(id);				// close netCDF dataset
		return false;
	}

	size_t xysize_length = 0;
	nc_inq_dimlen(id, id_xysize, &xysize_length);

	// get variable IDs
	int id_xrange = 0, id_yrange = 0, id_zrange = 0;
	int id_spacing = 0, id_dimension = 0, id_z = 0;
	nc_inq_varid(id, "x_range", &id_xrange);
	nc_inq_varid(id, "y_range", &id_yrange);
	nc_inq_varid(id, "z_range", &id_zrange);
	nc_inq_varid(id, "spacing", &id_spacing);
	nc_inq_varid(id, "dimension", &id_dimension);
	nc_inq_varid(id, "z", &id_z);

	// get values of variables
	double xrange[2], yrange[2], zrange[2], spacing[2];
	int dimension[2] = { 0, 0 };
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
	char token[40];

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
	sscanf(buf, "%s %d", token, &nodata);

	m_iColumns = ncols;
	m_iRows = nrows;

	// There is no projection info in a ASC file, but there might be
	//  an accompanying .prj file, in the old ESRI .prj format.
	if (!m_proj.ReadProjFile(szFileName))
		m_proj.Clear();

	// Some ASC files contain integer data, some contain floating point.
	// There's no way for us to know in advance, so just assume float.
	m_bFloatMode = true;

	m_EarthExtents.left = xllcorner;
	m_EarthExtents.right = xllcorner + (ncols - 1) * cellsize;
	m_EarthExtents.top = yllcorner + (nrows - 1) * cellsize;
	m_EarthExtents.bottom = yllcorner;

	ComputeCornersFromExtents();
	_AllocateArray();

	int i, j;
	float z;
	for (i = 0; i < nrows; i++)
	{
		if (progress_callback != NULL) progress_callback(i*100/nrows);
		for (j = 0; j < ncols; j++)
		{
			int num = fscanf(fp, "%f", &z);
			if (z == nodata)
				SetFValue(j, nrows-1-i, INVALID_ELEVATION);
			else
				SetFValue(j, nrows-1-i, z);
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
	char buf[8];
	fread(buf, 8, 1, fp);

	char hem = buf[7];
	buf[7] = '\0';
	float seconds = (float)atof(buf + 5);
	buf[5] = '\0';
	float minutes = (float)atof(buf + 3);
	buf[3] = '\0';
	float degrees = (float)atof(buf);

	float f = degrees + minutes/60 + seconds/3600;
	if (hem == 'W') f = -f;
	if (hem == 'S') f = -f;

	return f;
}

//
// Helper for DTED reader: read an 4-byte number in the
// return as integer
//
int get_dddd(FILE *fp)
{
	char buf[5];
	buf[4] = '\0';
	fread(buf, 4, 1, fp);
	return atoi(buf);
}

//
// Helper for DTED reader: read an 4-byte number in the
// form SSSS, decimal seconds
// return as decimal degree
//
double get_ssss(FILE *fp)
{
	double f = get_dddd(fp);
	return f/36000;
}


// Macros for DTED reader
#define SIGN_BIT 0x8000		// Position of 15th bit (high bit of 2 byte field)
#define IS_NEGATIVE(x) (x & SIGN_BIT)	// Check if sign bit is turned on
#define UNSET_SIGN_BIT(x) (x &= (~SIGN_BIT))	// Turn off sign bit

/** Loads from a DTED file.
 * \par
 * Should support DTED0, DTED1 and DTED2 files, although it has only been
 * tested on DTED0.  Projection is assumed to be geographic and elevation
 * is integer meters.
 * \returns \c true if the file was successfully opened and read.
 */
// DTED chunks/bytes:
//  UHL 80
//  DSI 648
//  ACC 2700
//  Data (variable)
//
bool vtElevationGrid::LoadFromDTED(const char *szFileName,
								 void progress_callback(int))
{
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)	// Cannot Open File
		return false;

	// all DTEDs are geographic and in integer meters
	// datum is always WGS84
	m_proj.SetProjectionSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	// check for correct format
	char buf[80];
	int header = 0;
	fread(buf, 4, 1, fp);
	buf[4] = 0;
	if (!strncmp(buf, "HDR1", 4))
		header = 1;
	if (!strncmp(buf, "UHL1", 4))
		header = 2;

	if (header == 0)
	{
		// Not a DTED file
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);
	if (header == 1)
	{
		// 4 bytes for 'HDR1'
		fread(buf, 4, 1, fp);

		// 17 bytes for 'Filename'
		fread(buf, 17, 1, fp);
		buf[17] = 0;

		// 6 bytes always 'UNIVAC'
		// 4 bytes 'Reel Sequence Number'
		// 4 bytes 'File Sequence Number'
		// 6 bytes version numbers
		// 6 bytes Creation Date
		// 6 bytes Expiration Date
		// 1 byte 'Accessibility'
		// 6 bytes 'Block Count'
		// 13 bytes 'Qualifier'
		// 7 bytes blank
		fseek(fp, 80, SEEK_SET);
		header = 2;
	}

	if (header == 2)
	{
		// 4 bytes for 'UHL1'
		fread(buf, 4, 1, fp);

		m_Corners[0].x = get_dms8(fp);
		m_Corners[0].y = get_dms8(fp);

		double xInterval = get_ssss(fp);
		double yInterval = get_ssss(fp);

		// Skip over:
		// 4 bytes Absolute Vertical Accuracy
		// 3 bytes Security Code
		// 12 bytes Unique Reference
		fseek(fp, 4+3+12, SEEK_CUR);

		// get dimensions
		m_iColumns = get_dddd(fp);
		m_iRows = get_dddd(fp);

		// imply other corners
		m_Corners[1].x = m_Corners[0].x;
		m_Corners[1].y = m_Corners[0].y + yInterval * (m_iRows - 1);
		m_Corners[2].x = m_Corners[0].x + xInterval * (m_iColumns - 1);
		m_Corners[2].y = m_Corners[0].y + yInterval * (m_iRows - 1);
		m_Corners[3].x = m_Corners[0].x + xInterval * (m_iColumns - 1);
		m_Corners[3].y = m_Corners[0].y;
		ComputeExtentsFromCorners();

		// Skip over:
		// 1 byte Multiple Accuracy
		// 24 bytes Reserved
		fseek(fp, 1+24, SEEK_CUR);
	}

	_AllocateArray();

	// REF: record header length + (2 * number of rows) + checksum length
	int line_length = 8 + (2 * m_iRows) + 4;
	unsigned char *linebuf = new unsigned char[line_length];
 
	// Skip DSI and ACC headers
	fseek(fp, 648 + 2700, SEEK_CUR);

	// each elevation, z, is stored in 2 bytes
	const int z_len = 2;
	short z = 0;
	unsigned char swap[z_len];

	int i, j, offset;
	for (i = 0; i < m_iColumns; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / m_iColumns);

		fread(linebuf, line_length, 1, fp);
		if (*linebuf != 0252)	// sentinel = 252 base 8
			break;

		offset = 8;	// record header length

		for (j = 0; j < m_iRows; j++)
		{
			// memcpy(z, linebuf + offset, z_len);
			swap[1] = *(linebuf + offset);
			swap[0] = *(linebuf + offset+1);

			// z = ntohs(z);	// swap bytes
			short z = *((short *)swap);

			// DTED values are signed magnitude, so convert to complement
			if (IS_NEGATIVE(z))
			{
				UNSET_SIGN_BIT(z);
				z *= -1;
			}

			if (-32767 == z)
				SetValue(i, j, INVALID_ELEVATION);
			else
				SetValue(i, j, z);

			offset += z_len;
		}
	}
	delete [] linebuf;
	fclose(fp);
	ComputeHeightExtents();
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
			if (gh.ByteOrder[0] == 'M')
			{
				// must swap byte order
				temp = cp[0];
				cp[0] = cp[1];
				cp[1] = temp;
			}
			SetValue(i, m_iRows-1-j, (z == gh.NoData) ? 0 : z);
		}
	}
	return true;
}


// helper
void ParseExtent(DRECT &extents, const char *name, const char *value)
{
	if (!strcmp(name, "upper_map_y"))
		extents.top = atof(value);
	if (!strcmp(name, "lower_map_y"))
		extents.bottom = atof(value);
	if (!strcmp(name, "left_map_x"))
		extents.left = atof(value);
	if (!strcmp(name, "right_map_x"))
		extents.right = atof(value);
}

/** Loads from a NOAA GlOBE file.
 * \par
 * In fact, there is no "GLOBE format", GLOBE files are delivered as raw
 * data, which can be intepreted using a variety of separate header files.
 * Using the GLOBE server "Select Your Own Area" feature results in
 * 2 files, a header with a .hdr extension and data with a .bin extension.
 * This method reads those file.  Pass the filename of the .hdr file to this
 * function, and it will automatically look for a corresponding .bin file in
 * the same location.
 * \par
 * Projection is always geographic and elevation is integer meters.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromGLOBE(const char *szFileName,
									void progress_callback(int))
{
	if (progress_callback != NULL)
		progress_callback(1);

	// Open the header file
	ifstream hdrFile(szFileName);
	if (!hdrFile.is_open())
	  return false;

	// Parse the file
	char strName[30];
	char szEqual[30];
	char strValue[30];
	vtString file_title;
	vtString str;

	// Read file_title
	hdrFile >> strName >> szEqual;
	file_title = get_line_from_stream(hdrFile);	// file title

	if (strcmp(strName, "file_title"))
		return false;	// not a GLOBE header

	// skip a few
	hdrFile >> strName >> szEqual;
	str = get_line_from_stream(hdrFile);	// data_type
	hdrFile >> strName >> szEqual;
	str = get_line_from_stream(hdrFile);	// grid_cell_registration
	hdrFile >> strName >> szEqual;
	str = get_line_from_stream(hdrFile);	// map_projection

	// Read the left, right, upper, lower coordinates
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);

	// Read the number of rows
	hdrFile >> strName >> szEqual >> strValue;
	m_iRows = atol(strValue);

	// Read the number of columns
	hdrFile >> strName >> szEqual >> strValue;
	m_iColumns = atol(strValue);

	// OK to skip the rest of the file, parameters which we will assume

	// Close the header file
	hdrFile.close();

	// make the corresponding filename for the DEM
	char dem_fname[200];
	strcpy(dem_fname, szFileName);
	char *ext = strrchr(dem_fname, '.');
	if (!ext)
		return false;
	strcpy(ext, ".bin");
	FILE *fp = fopen(dem_fname, "rb");
	if (!fp)
		return false;

	if (progress_callback != NULL) progress_callback(5);

	// Projection is always geographic, integer
	m_proj.SetProjectionSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	ComputeCornersFromExtents();

	_AllocateArray();

	// read the file
	int i, j;
	short z;
	for (j = 0; j < m_iRows; j++)
	{
		if (progress_callback != NULL) progress_callback(10 + j * 90 / m_iRows);
		for (i = 0; i < m_iColumns; i++)
		{
			fread(&z, sizeof(short), 1, fp);
			if (z == -500)	// 'unknown' generally used for ocean surface
				z = 0;
			SetValue(i, m_iRows-1-j, z);
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
	// open input file
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)		// Could not open input file
		return false;

	char sbuf[512],			// buffer for file input
		 dummy[2];			// used for \n and \0
	if (fread(sbuf, sizeof(char), 2, fp) != 2)
	{
		// "Could not read file"
		return false;
	}

	bool bBinary = false;	// PGM binary format flag: assume PGM ascii
	if (strncmp(sbuf,"P5",2) == 0)
		bBinary = 1;		// PGM binary format

	if (strncmp(sbuf,"GI",2) == 0) {
		fclose(fp);
		return false;
	}
	else if ((strncmp(sbuf,"P5",2) != 0) && (strncmp(sbuf,"P2",2) != 0))
	{
		/* not PGM Ascii */
		fclose(fp);
		return false;
	}

	// read PGM ASCII or binary file
	while ((fscanf(fp, "%s", sbuf) != EOF) && sbuf[0] == '#')
	{
		// comment
		fscanf(fp,"%[^\n]", sbuf);  // read comment beyond '#'
		fscanf(fp,"%[\n]", dummy);  // read newline
	}

	int xsize = atoi(sbuf);		// store xsize of array
	fscanf(fp,"%s",sbuf);		// read ysize of array
	int ysize = atoi(sbuf);
	fscanf(fp,"%s\n",sbuf);		// read maxval of array
	char *junk;					// unconverted part of a number
	double maxval = strtod(sbuf, &junk);/* store maxval. could throw away. */

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

	unsigned char oneb;		// one byte from file
	double a;
	if (bBinary)
	{
		// read PGM binary
		for (int j = 0; j < ysize; j++)
		{
			if (progress_callback != NULL) progress_callback(j * 100 / ysize);
			for (int i = 0; i < xsize; i++)
			{
				fread(&oneb, sizeof(unsigned char), 1, fp);
				SetFValue(i, ysize-1-j, oneb);
			}
		}
	}
	else
	{
		// read PGM ASCII
		for (int j = 0; j < ysize; j++)
		{
			if (progress_callback != NULL) progress_callback(j * 100 / ysize);
			for (int i = 0; i < xsize; i++)
			{
				fscanf(fp, "%s", sbuf);
				a = strtod(sbuf, &junk);
				SetFValue(i, ysize-1-j, (float)a);
			}
		}
	}
	fclose(fp);
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
 * Write the elevation grid to a 16-bit greyscale GeoTIFF.
 */
bool vtElevationGrid::SaveToGeoTIFF(const char *szFileName)
{
	g_GDALWrapper.RequestGDALFormats();

	// Save with GDAL to GeoTIFF
	GDALDriverManager *pManager = GetGDALDriverManager();
	if (!pManager)
		return false;

	GDALDriver *pDriver = pManager->GetDriverByName("GTiff");
	if (!pDriver)
		return false;

	char **papszParmList = NULL;

	GDALDataset *pDataset = pDriver->Create(szFileName, m_iColumns, m_iRows,
		1, GDT_Int16, papszParmList );
	if (!pDataset)
		return false;

	DPoint2 spacing = GetSpacing();
	double adfGeoTransform[6] = { m_EarthExtents.left, spacing.x, 0,
								  m_EarthExtents.top, 0, -spacing.y };
	pDataset->SetGeoTransform(adfGeoTransform);

	GInt16 *raster = new GInt16[m_iColumns*m_iRows];

	char *pszSRS_WKT = NULL;
	m_proj.exportToWkt( &pszSRS_WKT );
	pDataset->SetProjection(pszSRS_WKT);
	CPLFree( pszSRS_WKT );

	GDALRasterBand *pBand = pDataset->GetRasterBand(1);

	int x, y;
	float value;
	for (x = 0; x < m_iColumns; x++)
	{
		for (y = 0; y < m_iRows; y++)
		{
			// flip as we copy
			value = GetFValue(x, m_iRows-1-y);
			raster[y*m_iColumns + x] = (short) value;
		}
	}
	pBand->RasterIO( GF_Write, 0, 0, m_iColumns, m_iRows,
		raster, m_iColumns, m_iRows, GDT_Int16, 0, 0 );

	delete raster;
	GDALClose(pDataset);

	return true;
}


/**
 * Write the elevation grid to a 8-bit BMP file.  Much information is lost,
 * including precision, sign, and geographic location.
 */
bool vtElevationGrid::SaveToBMP(const char *szFileName)
{
	// We must scale from our actual range down to 8 bits
	float fMin = m_fMinHeight;
	if (fMin < 0)
		fMin = 0;
	float fMax = m_fMaxHeight;
	float fRange = fMax - fMin;
	float fScale = (fRange == 0.0f ? 0.0f : 255.0f / fRange);

	vtDIB dib;
	if (!dib.Create(m_iColumns, m_iRows, 8, true))
		return false;

	int x, y;
	float value;
	for (x = 0; x < m_iColumns; x++)
	{
		for (y = 0; y < m_iRows; y++)
		{
			value = GetFValue(x, y);
			dib.SetPixel8(x, y, (unsigned char) ((value - fMin) * fScale));
		}
	}
	return dib.WriteBMP(szFileName);
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

	g_GDALWrapper.RequestGDALFormats();

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
	if (err != OGRERR_NONE)
	{
		// No projection info; just assume that it's geographic
		m_proj.Clear();
	}

	// Get spacing and extents
	double		adfGeoTransform[6];
	if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None)
	{
		// Upper left corner is adfGeoTransform[0], adfGeoTransform[3]
		m_EarthExtents.left = adfGeoTransform[0];
		m_EarthExtents.top = adfGeoTransform[3];
		m_EarthExtents.right = m_EarthExtents.left + (adfGeoTransform[1] * m_iColumns);
		m_EarthExtents.bottom = m_EarthExtents.top + (adfGeoTransform[5] * m_iRows);
	}
	else
	{
		// No extents
		m_EarthExtents.Empty();
	}

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
 * Loads an elevation grid from an UK
 * Ordnance Survey NTF level 5 file using the OGR library
 *
 * \param szFileName The file name to read from.
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 *
 * \returns True if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromNTF5(const char *szFileName,
								   void progress_callback(int))
{
	OGREnvelope Extent;
	bool bRet = false;
	OGRFeature *pFeature = NULL;
	OGRDataSource *pDatasource = NULL;

	// let GDAL know we're going to use its OGR format drivers
	g_GDALWrapper.RequestOGRFormats();

	try
	{
		OGRDataSource *pDatasource = OGRSFDriverRegistrar::Open(szFileName);
		if (NULL == pDatasource)
			throw "No datasource";

		if (1 != pDatasource->GetLayerCount())
			throw "Layer count isn't 1";

		OGRLayer *pLayer = pDatasource->GetLayer(0);
		if (NULL == pLayer)
			throw "Couldn't get layer";

		OGRFeatureDefn *pFeatureDefn = pLayer->GetLayerDefn();
		if (NULL == pFeatureDefn)
			throw "Couldn't get feature definition";

		if (0 != strncmp(pFeatureDefn->GetName(), "DTM_", 4))
			throw "Feature definition doesn't start with 'DTM_'";

		if (wkbPoint25D != pFeatureDefn->GetGeomType())
			throw "Feature type isn't Point25D";

		if (1 != pFeatureDefn->GetFieldCount())
			throw "Field count isn't 1";

		OGRFieldDefn *pFieldDefn = pFeatureDefn->GetFieldDefn(0);
		if (NULL == pFieldDefn)
			throw "Couldn't get field definition";

		if (0 != strcmp(pFieldDefn->GetNameRef(), "HEIGHT"))
			throw "Couldn't get HEIGHT field";

		OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
		if (NULL == pSpatialRef)
			throw "Couldn't get spatial reference";

		pLayer->GetExtent(&Extent);

		// Get number of features. In this case the total number of cells
		// in the elevation matrix
		int iTotalCells = pLayer->GetFeatureCount();

  		pLayer->ResetReading();

		// Prescan the features to calculate the x and y intervals
		// this is a horrible kludge
		int iRowCount = 0;
		OGRPoint *pPoint;
		double dX;
		while ( (pFeature = pLayer->GetNextFeature()) != NULL )
		{
			if (NULL == (pPoint = (OGRPoint*)pFeature->GetGeometryRef()))
				throw "Couldn't get point feature";
	//		if (wkbPoint25D != pPoint->getGeometryType())
			if (wkbPoint != wkbFlatten(pPoint->getGeometryType()))	// RJ fix 03.11.21
				throw "Couldn't flatten point feature";
			if (0 == iRowCount)
				dX = pPoint->getX();
			else
				if (pPoint->getX() != dX)
				{
					delete pFeature;
					pFeature = NULL;
					break;
				}
			delete pFeature;
			pFeature = NULL;
			iRowCount++;
		}

		int iColCount = iTotalCells / iRowCount;

		m_iColumns = iColCount;
		m_iRows = iRowCount;
		m_proj.SetSpatialReference(pSpatialRef);
		m_bFloatMode = true;
		m_EarthExtents.left = Extent.MinX;
		m_EarthExtents.top = Extent.MaxY;
		m_EarthExtents.right = Extent.MaxX;
		m_EarthExtents.bottom = Extent.MinY;
		ComputeCornersFromExtents();

		_AllocateArray();

  		pLayer->ResetReading();

		int i;
		for (i = 0; i < iTotalCells; i++)
		{
			if (NULL == (pFeature = pLayer->GetNextFeature()))
				throw "Couldn't get next feature";
			if (NULL == (pPoint = (OGRPoint*)pFeature->GetGeometryRef()))
				throw "Couldn't get point feature";
			if (wkbPoint != wkbFlatten(pPoint->getGeometryType()))
				throw "Couldn't flatten point feature";
			SetFValue(i / iRowCount, i % iRowCount, (float)pPoint->getZ());
			delete pFeature;
			pFeature = NULL;
			if (progress_callback != NULL)
				progress_callback(i * 100 / iTotalCells);
		}

		ComputeHeightExtents();
		bRet = true;
		throw "done";
	}
	catch (const char *msg)
	{
		VTLOG("LoadFromNTF5 result: %s.\n", msg);
	}
	delete pFeature;
	delete pDatasource;

	return bRet;
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
	/* open input file */
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)	// Could not open input file
		return false;

	char buf[40];
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
	int offset_to_header = atoi(buf);
	fseek(fp, 36, SEEK_SET);
	fread(buf, 5, 1, fp);
	buf[5] = '\0';
	int offset_to_data = atoi(buf);

	// Read header
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


/**
 * Loads from an "XYZ file", which is a simple text file containing
 * a set of grid points in the form X,Y,Z - where Z is elevation.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromXYZ(const char *szFileName, void progress_callback(int))
{
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;

	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	char buf[80];
	double x, y, z;
	m_EarthExtents.SetRect(1E9, -1E9, -1E9, 1E9);
	int iNum = 0;

	// Make two passes over the file; the first time, we collect extents
	while (fgets(buf, 80, fp) != NULL)
	{
		if (sscanf(buf, "%lf,%lf,%lf", &x, &y, &z) != 3)
			return false;
		m_EarthExtents.GrowToContainPoint(DPoint2(x, y));
		iNum++;
	}
	ComputeCornersFromExtents();

	// Go back and look at the first two points
	fseek(fp, 0, SEEK_SET);
	DPoint2 testp[2];
	int i;
	for (i = 0; fgets(buf, 80, fp) != NULL && i < 2; i++)
	{
		sscanf(buf, "%lf,%lf,%lf", &x, &y, &z);
		testp[i].Set(x, y);
	}

	// Compare them and decide whether we are row or column order
	DPoint2 diff = testp[1] - testp[0];
	if (diff.x == 0)
	{
		// column-first ordering
		m_iRows = (int) (m_EarthExtents.Height() / fabs(diff.y)) + 1;
		m_iColumns = iNum / m_iRows;
	}
	else if (diff.y == 0)
	{
		// row-first ordering
		m_iColumns = (int) (m_EarthExtents.Width() / fabs(diff.x)) + 1;
		m_iRows = iNum / m_iColumns;
	}
	else
		return false;

	// Go back and read all the points
	DPoint2 base(m_EarthExtents.left, m_EarthExtents.bottom);
	DPoint2 spacing = GetSpacing();
	_AllocateArray();

	fseek(fp, 0, SEEK_SET);
	DPoint2 p;
	int xpos, ypos;
	while (fgets(buf, 80, fp) != NULL)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / iNum);

		sscanf(buf, "%lf,%lf,%lf", &x, &y, &z);
		p.Set(x, y);
		xpos = (int) ((x - base.x) / spacing.x);
		ypos = (int) ((y - base.y) / spacing.y);
		SetFValue(xpos, ypos, (float) z);
	}
	fclose(fp);
	return true;
}


/**
 * Loads from an "HGT" file, which is the format used by the USGS SRTM
 * FTP site for their 1-degree blocks of SRTM data.
 * It is simply a raw block of signed 2-byte data, in WGS84 geographic coords.
 *
 * The file will either be:
 *	 2,884,802 bytes (for 3 arcsec, 1201*1201) or
 *	25,934,402 bytes (for 1 arcsec, 3601*3601)
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromHGT(const char *szFileName, void progress_callback(int))
{
	FILE *fp = fopen(szFileName, "rb");
	if (!fp)
		return false;

	// extract extents from the filename
	const char *fname = StartOfFilename(szFileName);
	if (!fname)
		return false;

	char ns, ew;
	int lat, lon;
	sscanf(fname, "%c%02d%c%03d", &ns, &lat, &ew, &lon);
	if (ns == 'S')
		lat = -lat;
	else if (ns != 'N')
		return false;
	if (ew == 'W')
		lon = -lon;
	else if (ew != 'E')
		return false;

	fseek(fp, 0, SEEK_END);
	bool b1arcsec = (ftell(fp) > 3000000);
	fseek(fp, 0, SEEK_SET);

	m_EarthExtents.SetRect(lon, lat+1, lon+1, lat);
	ComputeCornersFromExtents();

	// Projection is always geographic, integer
	m_proj.SetProjectionSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	if (b1arcsec)
		m_iColumns = m_iRows = 3601;
	else
		m_iColumns = m_iRows = 1201;
	_AllocateArray();

#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )

	short *buf = new short[m_iColumns];
	short value;
	int i, j;
	for (j = 0; j < m_iRows; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / m_iRows);

		fread(buf, 2, m_iColumns, fp);
		for (i = 0; i < m_iColumns; i++)
		{
			value = SWAP_2(buf[i]);
			SetValue(i, m_iRows-1-j, value);
		}
	}
	delete [] buf;
	fclose(fp);
	return true;
}


////////////////////////////////////////////////////
// Defines for STM writer

#define MAGIC_VALUE 0x04030201
typedef union {
    unsigned int val;
    unsigned char bytes[4];
} bitTest;
#define SWAP(a,b)  { a^=b; b^=a; a^=b; }
#define BYTE    unsigned char
#define BSWAP_W(w) SWAP( (((BYTE *)&w)[0]), (((BYTE *)&w)[1]) )

/**
 * Write elevation to the STM (Simple Terrain Model) format created by Michael
 * Garland for his 'Scape' Terrain Simplification software.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::SaveToSTM(const char *szFileName, void progress_callback(int))
{
	FILE *outf = fopen(szFileName, "wb");
	if (!outf)
		return false;

	short *data = new short[m_iColumns];

	bitTest test;
	test.val = MAGIC_VALUE;
	fprintf(outf, "STM %d %d %c%c%c%c", m_iColumns, m_iRows,
		test.bytes[0], test.bytes[1], test.bytes[2], test.bytes[3]);
	fputc(0x0A, outf);

	for (int j = 0; j < m_iRows; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / m_iRows);
		for (int i = 0; i < m_iRows; i++)
		{
			/* This byte swap is only necessary for PC and Alpha machines */
			short val = GetValue(i, j);
			BSWAP_W(val);
			data[i] = val;
			data[i] += 32767;
		}
		fwrite(data, sizeof(short), m_iColumns, outf);
	}
	delete [] data;
	fclose(outf);
	return true;
}

