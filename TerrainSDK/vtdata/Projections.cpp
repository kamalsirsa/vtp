//
// Projections.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Parts of the code are derived from public-domain USGS software.
//

#include "Projections.h"
#include "StatePlane.h"
#include "MathTypes.h"

/////////////////////////////////////////////////////////////////////////////
// Implementation of class vtProjection
//

vtProjection::vtProjection()
{
	m_Datum = NO_DATUM;
}

/**
 * Assignment operator.
 */
vtProjection &vtProjection::operator=(vtProjection &ref)
{
	// copy new projection
	if (ref.GetRoot() != NULL)
	{
		OGRSpatialReference *ref_copy = ref.Clone();
		(*(OGRSpatialReference *)this) = *ref_copy;
	}

	// copy old fields
	m_Datum = ref.m_Datum;
	return *this;
}

bool vtProjection::operator==(vtProjection &ref)
{
	// Work around problem in IsSame, by detecting this type of difference
	if( IsProjected() != ref.IsProjected() )
		return false;
	return (IsSame( (OGRSpatialReference *) &ref ) != 0);
}

/**
 * Get the UTM zone of the projection.
 *
 * \param iZone
 * Should be one of the following values:
	- 1 through 60 for the northern hemisphere
	- -1 through -60 for the southern hemisphere
 */
void vtProjection::SetUTMZone(int iZone)
{
	OGRErr err = OGRSpatialReference::SetUTM( iZone );
}

/**
 * Get the UTM zone of the projection.
 *
 * \return
 *	- 1 through 60 in the northern hemisphere
 *  - -1 through -60 for the southern hemisphere
 *  - 0 if the projection is not UTM
 */
int	vtProjection::GetUTMZone()
{
	int north;
	int zone = OGRSpatialReference::GetUTMZone(&north);
	if (north)
		return zone;
	else
		return -zone;
}

/**
 * Set the datum as an enumeration (see DATUM)
 */
void vtProjection::SetDatum(DATUM datum)
{
	// Convert the DATUM enumeration to a Datum string
	OGR_SRSNode *dnode = GetAttrNode("DATUM");
	if (!dnode)
		return;
	switch (datum)
	{
	case NAD27:		dnode->GetChild(0)->SetValue("North_American_Datum_1927"); break;
	case NAD83:		dnode->GetChild(0)->SetValue("North_American_Datum_1983"); break;
	case WGS_72:	dnode->GetChild(0)->SetValue("WGS_1972"); break;
	case OLD_HAWAIIAN_MEAN:	dnode->GetChild(0)->SetValue("Old_Hawaiian"); break;
	default:
	case WGS_84:	dnode->GetChild(0)->SetValue("WGS_1984"); break;
	}
}

/**
 * Return the datum as an enumeration (see DATUM)
 */
DATUM vtProjection::GetDatum()
{
	// Convert new DATUM string to old Datum enum
	const char *datum_string = GetAttrValue("DATUM");
	if (datum_string)
	{
		if (!strcmp(datum_string, "North_American_Datum_1927"))
			return NAD27;
		if (!strcmp(datum_string, "North_American_Datum_1983"))
			return NAD83;
		if (!strcmp(datum_string, "WGS_1984"))
			return WGS_84;
		if (!strcmp(datum_string, "WGS_1972"))
			return WGS_72;
		if (!strcmp(datum_string, "Old_Hawaiian"))
			return OLD_HAWAIIAN_MEAN;
	}
	return WGS_84;	// default
}

/**
 * Return the kind of horizontal units used by the projection.  This is
 * also called "linear units."
 *
 * \return
	- 0 - Arc Degrees
	- 1 - Meters
	- 2 - Feet (International Foot)
	- 3 - Feet (U.S. Survey Foot)
 */
int vtProjection::GetUnits()
{
	if( IsGeographic() )
		return 0;  // degrees

	// Get horizontal units ("linear units")
	char *pszLinearUnits;
	double dfLinearConv = GetLinearUnits(&pszLinearUnits);
	double diff;

	diff = dfLinearConv - 0.3048;
	if( EQUAL(pszLinearUnits,SRS_UL_FOOT) || fabs(diff) < 0.000000001)
		return 2;  // international feet

	diff = dfLinearConv - (1200.0/3937.0);
	if( EQUAL(pszLinearUnits,SRS_UL_US_FOOT) || fabs(diff) < 0.000000001)
		return 3;  // u.s. survey feet

	if( dfLinearConv == 1.0 )
		return 1;  // meters

	return 1;	// can't guess; assume meters
}


/**
 * Set the projection by copying from a OGRSpatialReference.
 */
void vtProjection::SetSpatialReference(OGRSpatialReference *pRef)
{
	*((OGRSpatialReference *)this) = *(pRef->Clone());
}

/**
 * Return a string describing the type of projection.
 *
 * \par Example:
 *	"Geographic", "Transverse_Mercator", "Albers_Conic_Equal_Area"
 */
const char *vtProjection::GetProjectionName()
{
	const char *proj_string = GetAttrValue("PROJECTION");
	if (!proj_string)
		return "Geographic";
	else
		return proj_string;
}

/**
 * Return a very short string describing the type of projection.
 * \par
 * Possible values are "Geo", "UTM", "TM", "Albers", "LCC", "Other", or "Unknown"
 */
const char *vtProjection::GetProjectionNameShort()
{
	if (IsGeographic())
		return "Geo";
	const char *proj_string = GetAttrValue("PROJECTION");
	if (!proj_string)
		return "Unknown";
	if (!strcmp(proj_string, SRS_PT_TRANSVERSE_MERCATOR))
	{
		if (GetUTMZone() != 0)
			return "UTM";
		else
			return "TM";
	}
	if (!strcmp(proj_string, SRS_PT_ALBERS_CONIC_EQUAL_AREA))
		return "Albers";
	if (!strncmp(proj_string, "Lambert_Conformal_Conic", 23))
		return "LCC";
	return "Other";
}

/**
 * Convenient way to set a simple projection.
 *
 * \param bUTM true for UTM, false for Geographic.
 * \param iUTMZone If UTM, this is the zone: 1 through 60 in the northern
 *		hemisphere, -1 through -60 for the southern hemisphere.
 * \param eDatum The Datum as an enumeration (see DATUM)
 */
void vtProjection::SetProjectionSimple(bool bUTM, int iUTMZone, DATUM eDatum)
{
	switch (eDatum)
	{
	case NAD27:	 SetWellKnownGeogCS( "NAD27" ); break;
	case NAD83:	 SetWellKnownGeogCS( "NAD83" ); break;
	case WGS_72: SetWellKnownGeogCS( "WGS72" ); break;
	case WGS_84: SetWellKnownGeogCS( "WGS84" ); break;
	// We may need to support more datums here!
	default:	 SetWellKnownGeogCS( "WGS84" ); break;
	}
	if (bUTM)
	{
		// Must we assume Northern Hemisphere?  Revisit if needed.
		SetUTM( iUTMZone, TRUE );
	}
	switch (eDatum)
	{
	case OLD_HAWAIIAN_MEAN:
		SetDatum(eDatum);
	}
}

/**
 * Get the projection as a text description.  If the projection is Geographic
 * or UTM, then a "simple" type string will be returned.  For all other
 * projection types, 
 *
 * \param type A string buffer to contain the type of description.
 * This buffer should be at least 7 characters long to contain either the
 * word "simple" or "wkt".
 *
 * \param value A string buffer to contain the full description.
 * This buffer should be at least 2048 characters long to contain either
 * a simple or WKT description.
 */
bool vtProjection::GetTextDescription(char *type, char *value)
{
	DATUM datum = GetDatum();
	const char *datum_string = datumToString(datum);
	int zone = GetUTMZone();

	if (IsGeographic())
	{
		strcpy(type, "simple");
		sprintf(value, "geo, datum %s", datum_string);
	}
	else if (zone != 0)
	{
		strcpy(type, "simple");
		sprintf(value, "utm, datum %s, zone %d", datum_string, zone);
	}
	else
	{
		// Something less common, must use full WTK representation
		strcpy(type, "wkt");

		char *wkt;
		OGRErr err = exportToWkt(&wkt);
		if (err != OGRERR_NONE)
			return false;
		strcpy(value, wkt);
		OGRFree(wkt);
	}
	return true;
}

/**
 * Set the projection using a text description.
 *
 * \param type The type of description, either "simple" for short simple
 * string, or "wkt" for a full-length WKT (Well-Known Text) description.
 *
 * \param value The description itself.  A WKT description should be a
 * single string, with no extra whitespace.  A simple string can have the
 * following forms:
 *		- geo, datum <D>
 *		- utm, datum <D>, zone <Z>
 *
 * \par Example:
	\code
	proj.SetTextDescription("simple", "utm, datum WGS_84, zone 11");
	\endcode
 */
bool vtProjection::SetTextDescription(const char *type, const char *value)
{
	if (!strcmp(type, "simple"))
	{
		char datum[60];
		int iUTMZone;

		if (!strncmp(value, "geo", 3))
		{
			sscanf(value, "geo, datum %s", datum);
			SetWellKnownGeogCS(datum);
			return true;
		}
		else if (!strncmp(value, "utm", 3))
		{
			sscanf(value, "utm, datum %s zone %d", datum, &iUTMZone);
			if (datum[strlen(datum)-1] == ',')
				datum[strlen(datum)-1] = 0;
			SetWellKnownGeogCS(datum);
			SetUTM( iUTMZone, TRUE );
			return true;
		}
	}
	else if (!strcmp(type, "wkt"))
	{
		char wkt_buf[2000], *wkt = wkt_buf;
		strcpy(wkt, value);
		OGRErr err = importFromWkt((char **) &wkt);
		return (err == OGRERR_NONE);
	}
	return false;
}

/**
 * Read the projection from a .prj file.
 * 
 * If the filename does not have the file extension ".prj", this
 * method will look for a file which has the same name with a
 * ".prj" extension.
 *
 * \return true if successful.
 */
bool vtProjection::ReadProjFile(const char *filename)
{
	char prj_name[256];
	int len = strlen(filename);

	// check file extension
	if (len >= 4 && !stricmp(filename + len - 4, ".prj"))
	{
		strcpy(prj_name, filename);
	}
	else
	{
		strcpy(prj_name, filename);
		char *dot = strrchr(prj_name, '.');
		if (dot)
			strcpy(dot, ".prj");
		else
			strcat(prj_name, ".prj");
	}

	FILE *fp2 = fopen(prj_name, "rb");
	if (!fp2)
		return false;
	char wkt_buf[2000], *wkt = wkt_buf;
	fgets(wkt, 2000, fp2);
	fclose(fp2);
	OGRErr err = importFromWkt((char **) &wkt);
	if (err != OGRERR_NONE)
		return false;
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// Helper functions


/**
 * Given a non-geographic projection, produce a geographic projection which
 * has the same datum/ellipsoid values.
 */
void CreateSimilarGeographicProjection(vtProjection &source,
									   vtProjection &geo)
{
	geo.SetWellKnownGeogCS("WGS84");

	// We can't convert datum yet.  Force assumption that source datum
	// is the same as the destination.
	const char *datum_string = source.GetAttrValue("DATUM");
	const char *ellipsoid_string = source.GetAttrValue("SPHEROID");
	geo.SetGeogCS("WGS84", datum_string, ellipsoid_string,
		source.GetSemiMajor(), source.GetInvFlattening());
}


/**
 * Provides access to the State Plane Table
 */
StatePlaneInfo *GetStatePlaneTable()
{
	return g_StatePlaneInfo;
}

int GetNumStatePlanes()
{
	return sizeof(g_StatePlaneInfo) /  sizeof(StatePlaneInfo);
}

/**
 * Convert an enumerated DATUM to a string of the Datum Name.
 */
const char *datumToString(DATUM d)
{
	switch ( d )
	{
	case ADINDAN:
		return "ADINDAN";
	case ARC1950:
		return "ARC1950";
	case ARC1960:
		return "ARC1960";
	case AUSTRALIAN_GEODETIC_1966:
		return "AUSTRALIAN GEODETIC 1966";
	case AUSTRALIAN_GEODETIC_1984:
		return "AUSTRALIAN GEODETIC 1984";
	case CAMP_AREA_ASTRO:
		return "CAMP AREA ASTRO";
	case CAPE:
		return "CAPE";
	case EUROPEAN_DATUM_1950:
		return "EUROPEAN DATUM 1950";
	case EUROPEAN_DATUM_1979:
		return "EUROPEAN DATUM 1979";
	case GEODETIC_DATUM_1949:
		return "GEODETIC DATUM 1949";
	case HONG_KONG_1963:
		return "HONG KONG 1963";
	case HU_TZU_SHAN:
		return "HU TZU SHAN";
	case INDIAN:
		return "INDIAN";
	case NAD27:
		return "NAD27";
	case NAD83:
		return "NAD83";
	case OLD_HAWAIIAN_MEAN:
		return "OLD HAWAIIAN MEAN";
	case OMAN:
		return "OMAN";
	case ORDNANCE_SURVEY_1936:
		return "ORDNANCE SURVEY 1936";
	case PULKOVO_1942:
		return "PULKOVO 1942";
	case PROVISIONAL_S_AMERICAN_1956:
		return "PROVISIONAL SOUTH AMERICAN 1956";
	case TOKYO:
		return "TOKYO";
	case WGS_72:
		return "WGS72";
	case WGS_84:
		return "WGS84";
	case NO_DATUM:
		return "NO DATUM";
	default:
		return "Unknown Datum";
	}
}


/**
 * Determine an approximate conversion from degrees of longitude to meters,
 * given a latitude in degrees.
 */
double EstimateDegreesToMeters(double latitude)
{
	// estimate meters per degree of longitude, using the terrain origin
	double r0 = EARTH_RADIUS * cos(latitude / 180.0 * PId);
	double circ = 2.0 * r0 * PId;
	return circ / 360.0f;
}

/**
 * Create a conversion between projections, making the assumption that
 * the Datum of the target is the same as the Datum of the source.
 */
OCT *CreateConversionIgnoringDatum(vtProjection *pSource, vtProjection *pTarget)
{
	vtProjection DummyTarget = *pTarget;

	const char *datum_string = pSource->GetAttrValue("DATUM");

	OGR_SRSNode *dnode = DummyTarget.GetAttrNode("DATUM");
	if (dnode)
		dnode->GetChild(0)->SetValue(datum_string);

	OGR_SRSNode *enode1 = pSource->GetAttrNode("SPHEROID");
	OGR_SRSNode *enode2 = DummyTarget.GetAttrNode("SPHEROID");
	if (enode1 && enode2)
	{
		enode2->GetChild(0)->SetValue(enode1->GetChild(0)->GetValue());
		enode2->GetChild(1)->SetValue(enode1->GetChild(1)->GetValue());
		enode2->GetChild(2)->SetValue(enode1->GetChild(2)->GetValue());
	}

#if DEBUG && 1
	// Debug: Check texts in PROJ4
	char *str3, *str4;
	pSource->exportToProj4(&str3);
	DummyTarget.exportToProj4(&str4);

	char *wkt1, *wkt2;
	OGRErr err = pSource->exportToWkt(&wkt1);
	err = DummyTarget.exportToWkt(&wkt2);

	OGRFree(wkt1);
	OGRFree(wkt2);
#endif

	return OGRCreateCoordinateTransformation(pSource, &DummyTarget);
}


