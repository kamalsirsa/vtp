//
// Projections.h
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Derived from public-domain USGS software.
//
/** \file Projections.h */

#ifndef PROJECTIONSH
#define PROJECTIONSH

//
// Earth's diameter: 12756 km
// approximate circumference: 40074 km
// each degree of latitude: 111.3 km
//
#define EARTH_RADIUS		6378000.0f	// in meters
#define METERS_PER_LATITUDE	111317.1f

// Some class names are just too long!
#define OCT OGRCoordinateTransformation

// Do not change the order of this enumeration
enum LinearUnits
{
	LU_DEGREES,
	LU_METERS,
	LU_FEET_INT,	// International Foot
	LU_FEET_US,		// U.S. Survey Foot
	LU_UNITEDGE,	// Unit Edges: Dymaxion Projection
	LU_UNKNOWN
};

// Define a few common datums for convenience
#define EPSG_DATUM_OLD_HAWAIIAN	6135
#define EPSG_DATUM_PUERTO_RICO	6139
#define EPSG_DATUM_NAD27		6267
#define EPSG_DATUM_NAD83		6269
#define EPSG_DATUM_WGS72		6322
#define EPSG_DATUM_WGS84		6326

#include "ogr_spatialref.h"
#include "MathTypes.h"

///////////////////////////

/**  The vtProjection class represents an earth coordinate reference system
 * (CRS), which is generally a projected coordinate system (PCS).  It is
 * based on the class
 * <a href="http://www.gdal.org/ogr/classOGRSpatialReference.html">OGRSpatialReference</a>
 * which represents a full OpenGIS Spatial Reference System.  The vtProjection
 * class extends OGRSpatialReference with several useful methods.
 */
class vtProjection : public OGRSpatialReference
{
public:
	vtProjection();
	~vtProjection();

	// Assignment
	vtProjection &operator=(const vtProjection &ref);

	// Equality
	bool operator==(const vtProjection &ref) const;
	bool operator!=(const vtProjection &ref) const;

	void	SetUTMZone(int iZone);
	int		GetUTMZone() const;
	OGRErr	SetDatum(int iDatum);
	int		GetDatum() const;
	LinearUnits	GetUnits() const;
	int		GuessEPSGCode() const;

	OGRErr	SetGeogCSFromDatum(int iDatum);

	bool	SetProjectionSimple(bool bUTM, int iUTMZone, int iDatum);
	void	SetSpatialReference(OGRSpatialReference *pRef);

	const char *GetProjectionName() const;
	const char *GetProjectionNameShort() const;

	bool GetTextDescription(char *type, char *value) const;
	bool SetTextDescription(const char *type, const char *value);

	bool ReadProjFile(const char *filename);
	bool WriteProjFile(const char *filename) const;

	static double GeodesicDistance(const DPoint2 &in, const DPoint2 &out, bool bQuick = false);

	void SetDymaxion(bool bTrue) { m_bDymaxion = bTrue; }
	bool IsDymaxion() const { return m_bDymaxion; }

protected:
	bool	m_bDymaxion;

	// Useful for debugging
	void LogDescription() const;
};

struct StatePlaneInfo
{
	const char *name;
	bool bNAD27;
	bool bNAD83;
	int usgs_code;
};

class Geodesic
{
public:
	void CalculateInverse();

	double	a;
	double	lam1, phi1;
	double	lam2, phi2;
	double	S;
	double	onef, f, f2, f4, f64;
};


//////////////////////////////
// Helper functions

const char *DatumToString(int d);
const char *DatumToStringShort(int d);
struct EPSGDatum
{
	bool bCommon;
	int iCode;
	const char *szName;
	const char *szShortName;
};
extern std::vector<EPSGDatum> g_EPSGDatums;
void SetupEPSGDatums();

StatePlaneInfo *GetStatePlaneTable();
int NumStatePlanes();
void CreateSimilarGeographicProjection(const vtProjection &source, vtProjection &geo);
OCT *CreateConversionIgnoringDatum(const vtProjection *pSource, vtProjection *pTarget);
OCT *CreateCoordTransform(const vtProjection *pSource,
						  const vtProjection *pTarget, bool bLog = false);

/**
 * Determine an approximate conversion from degrees of longitude to meters,
 * given a latitude in degrees.
 */
double EstimateDegreesToMeters(double latitude);

/**
 * Return the number of meters for a given type of linear units
 */
double GetMetersPerUnit(LinearUnits lu);

/**
 * Return a string describing the units.
 */
const char *GetLinearUnitName(LinearUnits lu);

/**
 * Return number of meters per degree of longitude, at a given latitude.
 */
double MetersPerLongitude(double latitude);

/**
 * Read the contents of a world file.  You can pass any filename, and it will
 * look for the corresponding world file.
 */
bool ReadAssociatedWorldFile(const char *filename_base, double params[6]);

///////////////////////////

// Stick this here for now, although it really belongs in its own module

class GDALInitResult
{
public:
	GDALInitResult() { hasGDAL_DATA = false; hasPROJ_LIB = false; hasPROJSO = false; }

	bool hasGDAL_DATA;
	bool hasPROJ_LIB;
	bool hasPROJSO;

	bool Success() { return hasGDAL_DATA && hasPROJ_LIB && hasPROJSO; }
};

class GDALWrapper
{
public:
	GDALWrapper();
	~GDALWrapper();

	void RequestGDALFormats();
	void RequestOGRFormats();
	bool Init();
	GDALInitResult* GetInitResult() { return &m_initResult; }
	bool TestPROJ4();

protected:
	bool FindGDALData();
	bool FindPROJ4Data();
	bool FindPROJ4SO();

	bool m_bGDALFormatsRegistered;
	bool m_bOGRFormatsRegistered;
	GDALInitResult m_initResult;
};

extern GDALWrapper g_GDALWrapper;

#ifdef WIN32
	#define DEFAULT_LOCATION_GDAL_DATA "../../GDAL-data/"
	#define DEFAULT_LOCATION_PROJ_LIB "../../PROJ4-data/"
#elif __APPLE__
	#define DEFAULT_LOCATION_GDAL_DATA "Shared/share/gdal/"
	#define DEFAULT_LOCATION_PROJ_LIB "Shared/share/proj/"
	#define DEFAULT_LOCATION_PROJSO "Shared/lib/"
#else // other unixes
	#define DEFAULT_LOCATION_GDAL_DATA "/usr/local/share/gdal/"
	#define DEFAULT_LOCATION_PROJ_LIB "/usr/local/share/proj/"
#  if _LP64
#       define DEFAULT_LOCATION_PROJSO "/usr/local/lib64/"
#  else
#       define DEFAULT_LOCATION_PROJSO "/usr/local/lib/"
#  endif
#endif

#endif	// PROJECTIONSH
