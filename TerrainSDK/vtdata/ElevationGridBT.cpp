//
// vtElevationGridDEM.cpp
//
// This modules contains the implementations of the BT file I/O methods of
// the class vtElevationGrid.
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "ElevationGrid.h"
#include "ByteOrder.h"
#include "vtdata/vtLog.h"


/** Loads just the header information from a BT (Binary Terrain) file.
 * \par
 * This can be useful if you want to check the information such as the
 * size of the data, without reading the entire file.
 * This method works whether it is given a normal BT file, or one which
 * has been compressed with gzip.
 * \returns \c true if the header was successfully parsed.
 */
bool vtElevationGrid::LoadBTHeader(const char *szFileName)
{
	// The gz functions (gzopen etc.) behave exactly like the stdlib
	//  functions (fopen etc.) in the case where the input file is not in
	//  gzip format, so we can simply use them without worry.
	gzFile fp = gzopen(szFileName, "rb");
	if (!fp)
		return false;		// Cannot Open File

	char buf[11];
	gzread(fp, buf, 10);
	buf[10] = '\0';

	if (strncmp(buf, "binterr", 7))
	{
		gzclose(fp);
		return false;		// Not a current BT file
	}

	// Get version
	int iMajor = (int) buf[7]-'0';
	int iMinor = (int) buf[9]-'0';

	VTLOG("BT header: version %d.%d", iMajor, iMinor);

	// NOTE:  BT format is little-endian
	GZFRead(&m_iColumns, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
	GZFRead(&m_iRows,	   DT_INT, 1, fp, BO_LITTLE_ENDIAN);

	// Default to internal projection
	short external = 0;

	short svalue, proj_type, zone, datum;
	int ivalue;
	float ftmp;
	if (iMajor == 1 && iMinor == 0)		// Version 1.0
	{
		// data size
		GZFRead(&ivalue, DT_INT, 1, fp, BO_LITTLE_ENDIAN);

		// UTM flag
		GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		proj_type = (svalue == 1);

		// UTM zone
		GZFRead(&zone, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// 1.0 didn't support Datum, so assume WGS84
		datum = EPSG_DATUM_WGS84;

		// coordinate extents left-right
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.left = ftmp;
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.right = ftmp;

		// coordinate extents bottom-top
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.bottom = ftmp;
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		m_EarthExtents.top = ftmp;

		// is the data floating point or integers?
		GZFRead(&m_bFloatMode, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
		if (m_bFloatMode != true)
			m_bFloatMode = false;
	}
	else if (iMajor == 1 && (iMinor == 1 || iMinor == 2 || iMinor == 3))
	{
		// Version 1.1, 1.2, or 1.3

		// data size
		GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// Is floating point data?
		GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		m_bFloatMode = (svalue == 1);

		// Projection (0 = geo, 1 = utm, 2 = feet, 3 = u.s. feet)
		GZFRead(&proj_type, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// UTM zone (ignore unless projection == 1)
		GZFRead(&zone, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// Datum (ignore unless projection == 0 or 1)
		GZFRead(&datum, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// coordinate extents
		GZFRead(&m_EarthExtents.left, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFRead(&m_EarthExtents.right, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFRead(&m_EarthExtents.bottom, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFRead(&m_EarthExtents.top, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);

		// External projection flag
		GZFRead(&external, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// 1.3 adds the vertical scale field
		if (iMajor == 1 && iMinor == 3)
			GZFRead(&m_fVMeters, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
	}
	else
		return false;	// unsupported version

	gzclose(fp);

	VTLOG(", float %d, CRS external %d", m_bFloatMode, external);

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
		VTLOG(" (type %d, zone %d, datum %d", proj_type, zone, datum);
		m_proj.SetProjectionSimple(proj_type == 1, zone, datum);
	}

	ComputeCornersFromExtents();
	VTLOG("\n");

	return true;
}


/** Loads from a BT (Binary Terrain) file.
 * \par
 * Both the current version (1.1) and older BT versions are supported.
 * This method works whether it is given a normal BT file, or one which
 * has been compressed with gzip.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromBT(const char *szFileName, bool progress_callback(int))
{
	// First load the header
	if (!LoadBTHeader(szFileName))
		return false;

	gzFile fp = gzopen(szFileName, "rb");
	if (!fp)
		return false;		// Cannot Open File

	// elevation data always starts at offset 256
	gzseek(fp, 256, SEEK_SET);

	_AllocateArray();

	int i;
#if 0
	// slow way
	int value;
	float fvalue;
	for (i = 0; i < m_iColumns; i++)
	{
		if (progress_callback != NULL) progress_callback(i * 100 / m_iColumns);
		for (j = 0; j < m_iRows; j++)
		{
			if (m_bFloatMode)
			{
				GZFRead(&fvalue, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
				SetFValue(i, j, fvalue);
			}
			else
			{
				GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
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
			if (progress_callback != NULL)
			{
				if (progress_callback(i * 100 / m_iColumns))
				{
					// Cancel
					gzclose(fp);
					return false;
				}
			}
			GZFRead(m_pFData + (i*m_iRows), DT_FLOAT, m_iRows, fp, BO_LITTLE_ENDIAN);
		}
	}
	else
	{
		for (i = 0; i < m_iColumns; i++)
		{
			if (progress_callback != NULL)
			{
				if (progress_callback(i * 100 / m_iColumns))
				{
					// Cancel
					gzclose(fp);
					return false;
				}
			}
			GZFRead(m_pData + (i*m_iRows), DT_SHORT, m_iRows, fp, BO_LITTLE_ENDIAN);
		}
	}
#endif

	// be sure to close the file!
	gzclose(fp);

	ComputeHeightExtents();
	return true;
}


/**
 * Writes the grid to a BT (Binary Terrain) file.
 * The current BT format version (1.3) is written.
 *
 * \param szFileName		The file name to write to.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 * \param bGZip			If true, the data will be compressed with gzip.
 *				If true, you should Use a filename ending with ".gz".
 */
bool vtElevationGrid::SaveToBT(const char *szFileName,
							   bool progress_callback(int), bool bGZip)
{
	int w = m_iColumns;
	int h = m_iRows;
	short zone = (short) m_proj.GetUTMZone();
	short datum = (short) m_proj.GetDatum();
	short isfloat = (short) IsFloatMode();
	short external = 1;		// always true: we always write an external .prj file

	LinearUnits units = m_proj.GetUnits();
	int hunits = (int) units;

	// Latest header, version 1.2
	short datasize = m_bFloatMode ? 4 : 2;

	if (bGZip == false)
	{
		// Use conventional IO
		FILE *fp = fopen(szFileName, "wb");
		if (!fp)
			return false;

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
	}
	else
	{
		// Use GZip IO
		gzFile fp = gzopen(szFileName, "wb");
		if (!fp)
			return false;

		gzwrite(fp, (void *)"binterr1.3", 10);
		gzwrite(fp, &w, 4);
		gzwrite(fp, &h, 4);
		gzwrite(fp, &datasize, 2);
		gzwrite(fp, &isfloat, 2);
		gzwrite(fp, &hunits, 2);		// Horizontal Units (0, 1, 2, 3)
		gzwrite(fp, &zone, 2);		// UTM zone
		gzwrite(fp, &datum, 2);		// Datum

		// coordinate extents
		gzwrite(fp, &m_EarthExtents.left, 8);
		gzwrite(fp, &m_EarthExtents.right, 8);
		gzwrite(fp, &m_EarthExtents.bottom, 8);
		gzwrite(fp, &m_EarthExtents.top, 8);

		gzwrite(fp, &external, 2);	// External projection specification
		gzwrite(fp, &m_fVMeters, 4);	// External projection specification

		// now write the data: always starts at offset 256
		gzseek(fp, 256, SEEK_SET);

		// fast way, with the assumption that the data is stored column-first in memory
		if (m_bFloatMode)
		{
			for (int i = 0; i < w; i++)
			{
				if (progress_callback != NULL) progress_callback(i * 100 / w);
				gzwrite(fp, m_pFData + (i * m_iRows), 4 * m_iRows);
			}
		}
		else
		{
			for (int i = 0; i < w; i++)
			{
				if (progress_callback != NULL) progress_callback(i * 100 / w);
				gzwrite(fp, m_pData + (i * m_iRows), 2 * m_iRows);
			}
		}
		gzclose(fp);
	}

	if (external)
	{
		// Write external projection file (.prj)
		char prj_name[256];
		strcpy(prj_name, szFileName);
		int len = strlen(prj_name);
		if (bGZip)
			strcpy(prj_name + len - 6, ".prj"); // overwrite the .bt.gz
		else
			strcpy(prj_name + len - 3, ".prj"); // overwrite the .bt
		m_proj.WriteProjFile(prj_name);
	}

	return true;
}

