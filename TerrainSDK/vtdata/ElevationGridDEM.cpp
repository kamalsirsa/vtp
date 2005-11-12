//
// vtElevationGridDEM.cpp
//
// This modules contains the implementations of the DEM file I/O methods of
// the class vtElevationGrid.
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "ElevationGrid.h"
#include "vtLog.h"

#ifdef DEBUG
#define DEBUG_DEM true
#else
#define DEBUG_DEM false
#endif

// ************** DConvert - DEM Helper function ****************

double DConvert(FILE *fp, int length, bool bDebug=false)
{
	char szCharString[64];

	fread(szCharString, length, 1, fp);
	szCharString[length] = 0;

#ifndef _MSC_VER
	// Microsoft's C standard library documentation says that the exponent in
	//  an atof() call can be {d,D,e,E}.  The ANSI C standard seems to agree.
	//  But Unix and MacOSX don't like {d,D}.  So, for them, change it.
	for (int i=0; i < length; i++)
		if (szCharString[i] == 'D' || szCharString[i] == 'd')
			szCharString[i] = 'E';
#endif

	double value = atof(szCharString);

	if (bDebug)
		VTLOG(" DConvert '%s' -> %lf\n", szCharString, value);

	return value;
}

int IConvert(FILE *fp, int length)
{
	char szCharString[64];

	fread(szCharString, length, 1, fp);
	szCharString[length] = 0;

	return atoi(szCharString);
}

/**
 * Loads elevation from a USGS DEM file.
 *
 * Some non-standard variations of the DEM format are supported.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromDEM(const char *szFileName,
								  bool progress_callback(int))
{
	if (progress_callback != NULL) progress_callback(0);

	FILE *fp = fopen(szFileName,"rb");
	if (!fp)		// Cannot Open File
		return false;

	// check for version of DEM format
	int		iRow, iColumn;
	char buffer[158];

	fseek(fp, 864, 0);
	fread(buffer, 144, 1, fp);
	bool bOldFormat = (strncmp(buffer, "     1     1", 12) == 0);
	bool bNewFormat = false;
	bool bFixedLength = true;
	int  iDataStartOffset = 1024;	// set here to avoid compiler warning
	int  i, j;

	if (bOldFormat)
		iDataStartOffset = 1024;	// 1024 is record length
	else
	{
		fseek(fp, 1024, 0); 		// Check for New Format
		iRow = IConvert(fp, 6);
		iColumn = IConvert(fp, 6);
		if (iRow==1 && iColumn==1)	// File OK?
		{
			bNewFormat = true;
			iDataStartOffset = 1024;
		}
		else
		{
			// might be the Non-fixed-length record format
			// Record B can start anywhere from 865 to 1023
			// Record B is identified by starting with the row/column
			//  of its first profile, "     1     1"
			fseek(fp, 865, 0);
			fread(buffer, 158, 1, fp);
			for (i = 0; i < 158-12; i++)
			{
				if (!strncmp(buffer+i, "     1     1", 12))
				{
					// Found it
					bFixedLength = false;
					iDataStartOffset = 865+i;
					break;
				}
			}
			if (i == 158-12)
			{
				// Not a DEM file
				fclose(fp);
				return false;
			}
		}
	}

	// Read the embedded DEM name
	char szName[41];
	fseek(fp, 0, 0);
	fgets(szName, 41, fp);
	int len = strlen(szName);	// trim trailing whitespace
	while (len > 0 && szName[len-1] == ' ')
	{
		szName[len-1] = 0;
		len--;
	}
	m_strOriginalDEMName = szName;

	fseek(fp, 156, 0);
	int iCoordSystem = IConvert(fp, 6);
	int iUTMZone = IConvert(fp, 6);

	fseek(fp, 168, 0);
	double dProjParams[15];
	for (i = 0; i < 15; i++)
		dProjParams[i] = DConvert(fp, 24, DEBUG_DEM);

	int iDatum = EPSG_DATUM_NAD27;	// default

	// OLD format header ends at byte 864 (0x360); new format has Datum
	if (bNewFormat)
	{
		// year of data compilation
		char szDateBuffer[5];
		fseek(fp, 876, 0);		// 0x36C
		fread(szDateBuffer, 4, 1, fp);
		szDateBuffer[4] = 0;

		// Horizontal datum
		// 1=North American Datum 1927 (NAD 27)
		// 2=World Geodetic System 1972 (WGS 72)
		// 3=WGS 84
		// 4=NAD 83
		// 5=Old Hawaii Datum
		// 6=Puerto Rico Datum
		fseek(fp, 890, 0);	// 0x37A
		int datum = IConvert(fp, 2);
		VTLOG("DEM Reader: Read Datum Value %d\n", datum);
		switch (datum)
		{
			case 1: iDatum = EPSG_DATUM_NAD27; break;
			case 2: iDatum = EPSG_DATUM_WGS72; break;
			case 3: iDatum = EPSG_DATUM_WGS84; break;
			case 4:	iDatum = EPSG_DATUM_NAD83; break;
			case 5: iDatum = EPSG_DATUM_OLD_HAWAIIAN; break;
			case 6: iDatum = EPSG_DATUM_PUERTO_RICO; break;
		}
	}

	// Set up the projection
	bool bGeographic = false;
	bool bSuccessfulCRS = true;
	switch (iCoordSystem)
	{
	case 0:		// geographic (lat-lon)
		bGeographic = true;
		iUTMZone = -1;
		bSuccessfulCRS = m_proj.SetProjectionSimple(false, iUTMZone, iDatum);
		break;
	case 1:		// utm
		m_proj.SetProjectionSimple(true, iUTMZone, iDatum);
		break;
	case 3:		// Albers Conical Equal Area
		{
			// The Official DEM documentation says:
			// "Note: All angles (latitudes, longitudes, or azimuth) are
			// required in degrees, minutes, and arc seconds in the packed
			// real number format +DDDOMMOSS.SSSSS."
			// However, what i've actually seen is values like:
			//  0.420000000000000D+06' -> 420000
			// for 42 degrees, which is off by a decimal point from the DEM docs.
			// So, intepret the values with factor of 10000 to convert to degrees:

			// double semi_major = dProjParams[0];		// unused
			// double eccentricity = dProjParams[1];	// unused
			double lat_1st_std_parallel = dProjParams[2] / 10000;
			double lat_2nd_std_parallel = dProjParams[3] / 10000;
			double lon_central_meridian = dProjParams[4] / 10000;
			double lat_origin = dProjParams[5] / 10000;
			double false_easting = dProjParams[6];
			double false_northing = dProjParams[7];

			m_proj.SetGeogCSFromDatum(iDatum);
			m_proj.SetACEA(lat_1st_std_parallel, lat_2nd_std_parallel, lat_origin,
				lon_central_meridian, false_easting, false_northing);
		}
		break;
	case 2:		// State Plane (!)
	case 4:		// Lambert Conformal
	case 5:		// Mercator
	case 6:		// Polar Stereographic
	case 7:		// Polyconic
	case 8:		// Equidistant Conic Type A / B
	case 9:		// Transverse Mercator
	case 10:	// Stereographic
	case 11:	// Lambert Azimuthal Equal-Area
	case 12:	// Azimuthal Equidistant
	case 13:	// Gnomonic
	case 14:	// Orthographic
	case 15:	// General Vertical Near-Side Perspective
	case 16:	// Sinusoidal (Plate Caree)
	case 17:	// Equirectangular
	case 18:	// Miller Cylindrical
	case 19:	// Van Der Grinten I
	case 20:	// Oblique Mercator
		VTLOG("Warning!  We don't yet support DEM coordinate system %d.\n", iCoordSystem);
		break;
	}

	// We must have a functional CRS, or it will sebsequently fail
	if (!bSuccessfulCRS)
		return false;

	fseek(fp, 528, 0);
	int iGUnit = IConvert(fp, 6);
	int iVUnit = IConvert(fp, 6);

	// Ground (Horizontal) Units in meters
	double	fGMeters;
	switch (iGUnit)
	{
	case 0: fGMeters = 1.0;		break;	// 0 = radians (never encountered)
	case 1: fGMeters = 0.3048;	break;	// 1 = feet
	case 2: fGMeters = 1.0;		break;	// 2 = meters
	case 3: fGMeters = 30.922;	break;	// 3 = arc-seconds
	}

	// Vertical Units in meters
	double	fVertUnits;
	switch (iVUnit)
	{
	case 1:  fVertUnits = 0.3048f; break;	// feet to meter conversion
	case 2:  fVertUnits = 1.0f;	   break;	// meters == meters
	default: fVertUnits = 1.0f;	   break;	// anything else, assume meters
	}

	fseek(fp, 816, 0);
	DConvert(fp, 12, DEBUG_DEM);	// dxdelta (unused)
	double dydelta = DConvert(fp, 12, DEBUG_DEM);
	double dzdelta = DConvert(fp, 12, DEBUG_DEM);

	m_bFloatMode = false;

	// Read the coordinates of the 4 corners
	VTLOG("DEM corners:\n");
	DPoint2	corners[4];			// SW, NW, NE, SE
	fseek(fp, 546, 0);
	for (i = 0; i < 4; i++)
	{
		corners[i].x = DConvert(fp, 24, DEBUG_DEM);
		corners[i].y = DConvert(fp, 24, DEBUG_DEM);
	}
	for (i = 0; i < 4; i++)
		VTLOG(" (%lf, %lf)", corners[i].x, corners[i].y);
	VTLOG("\n");

	if (bGeographic)
	{
		for (i = 0; i < 4; i++)
		{
			// convert arcseconds to degrees
			m_Corners[i].x = corners[i].x / 3600.0;
			m_Corners[i].y = corners[i].y / 3600.0;
		}
	}
	else
	{
		// some linear coordinate system
		for (i = 0; i < 4; i++)
			m_Corners[i] = corners[i];
	}

	double dElevMin = DConvert(fp, 24, DEBUG_DEM);
	double dElevMax = DConvert(fp, 24, DEBUG_DEM);

	fseek(fp, 852, 0);
	IConvert(fp, 6);	// This "Rows" value will always be 1
	int iProfiles = IConvert(fp, 6);
	VTLOG("DEM profiles: %d\n", iProfiles);

	m_iColumns = iProfiles;

	// values we'll need while scanning the elevation profiles
	int		iProfileRows, iProfileCols;
	int		iElev;
	double	dLocalDatumElev, dProfileMin, dProfileMax;
	int		ygap;
	double	dMinY;
	DPoint2 start;

	if (bGeographic)
	{
		// If it's in degrees, it's flush square, so we can simply
		// derive the extents (m_EarthExtents) from the quad corners (m_Corners)
		ComputeExtentsFromCorners();
		dMinY = std::min(corners[0].y, corners[3].y);
	}
	else
	{
		VTLOG("DEM scanning to compute extents\n");
		m_EarthExtents.SetRect(1E9, -1E9, -1E9, 1E9);

		if (!bFixedLength)
			fseek(fp, iDataStartOffset, 0);
		// Need to scan over all the profiles, accumulating the TRUE
		// extents of the actual data points.
		int record = 0;
		int data_len;
		for (i = 0; i < iProfiles; i++)
		{
			if (progress_callback != NULL)
				progress_callback(i*49/iProfiles);

			if (bFixedLength)
				fseek(fp, iDataStartOffset + (record * 1024), 0);

			// We cannot use IConvert here, because there *might* be a spurious LF
			// after the number - seen in some rare files.
			fscanf(fp, "%d", &iRow);
			iColumn = IConvert(fp, 6);
			// assert(iColumn == i+1);
			iProfileRows = IConvert(fp, 6);
			iProfileCols = IConvert(fp, 6);

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
					// We cannot use IConvert here, because there *might* be a spurious LF
					// after the number - seen in some rare files.
					fscanf(fp, "%d", &iElev);
				}
			}
		}
		dMinY = m_EarthExtents.bottom;
	}
	VTLOG("DEM extents LRTB: %lf, %lf, %lf, %lf\n", m_EarthExtents.left,
		m_EarthExtents.right, m_EarthExtents.top, m_EarthExtents.bottom);

	// Compute number of rows
	double	fRows;
	if (bGeographic)
	{
		// degrees
		fRows = m_EarthExtents.Height() / dydelta * 3600.0f;
		m_iRows = (int)fRows + 1;	// 1 more than quad spacing
	}
	else
	{
		// some linear coordinate system
		fRows = m_EarthExtents.Height() / dydelta;
		m_iRows = (int)(fRows + 0.5) + 1;	// round to the nearest integer
	}

	// safety check
	if (m_iRows > 10000)
		return false;

	_AllocateArray();

	// jump to start of actual data
	fseek(fp, iDataStartOffset, 0);

	for (i = 0; i < iProfiles; i++)
	{
		if (progress_callback != NULL)
			progress_callback(50+i*49/iProfiles);

		// We cannot use IConvert here, because there *might* be a spurious LF
		// after the number - seen in some rare files.
		fscanf(fp, "%d", &iRow);
		iColumn = IConvert(fp, 6);
		//assert(iColumn == i+1);

		iProfileRows = IConvert(fp, 6);
		iProfileCols = IConvert(fp, 6);

		start.x = DConvert(fp, 24);
		start.y = DConvert(fp, 24);
		dLocalDatumElev = DConvert(fp, 24);
		dProfileMin = DConvert(fp, 24);
		dProfileMax = DConvert(fp, 24);

		ygap = (int)((start.y - dMinY)/dydelta);

		for (j = ygap; j < (ygap + iProfileRows); j++)
		{
			//assert(j >=0 && j < m_iRows);	// useful safety check

			// We cannot use IConvert here, because there *might* be a spurious LF
			// after the number - seen in some rare files.
			fscanf(fp, "%d", &iElev);
			if (iElev == -32767 || iElev == -32768)
				SetValue(i, j, INVALID_ELEVATION);
			else
				SetValue(i, j, (short) iElev);
		}
	}
	fclose(fp);

	m_fVMeters = (float) (fVertUnits * dzdelta);
	ComputeHeightExtents();
	if (m_fMinHeight != dElevMin || m_fMaxHeight != dElevMax)
		VTLOG("DEM Reader: elevation extents in .dem (%.1f, %.1f) don't match data (%.1f, %.1f).\n",
		dElevMin, dElevMax, m_fMinHeight, m_fMaxHeight);

	return true;
}

