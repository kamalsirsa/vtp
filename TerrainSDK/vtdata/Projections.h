//
// Projections.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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
 * <a href="http://gdal.velocet.ca/projects/opengis/ogrhtml/class_ogrspatialreference.html">OGRSpatialReference</a>
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

	OGRErr	SetGeogCSFromDatum(int iDatum);

	void	SetProjectionSimple(bool bUTM, int iUTMZone, int iDatum);
	void	SetSpatialReference(OGRSpatialReference *pRef);

	const char *GetProjectionName() const;
	const char *GetProjectionNameShort() const;

	bool GetTextDescription(char *type, char *value);
	bool SetTextDescription(const char *type, const char *value);

	bool ReadProjFile(const char *filename);
	bool WriteProjFile(const char *filename) const;

	double GeodesicDistance(const DPoint2 &in, DPoint2 &out, bool bQuick = false);

	void SetDymaxion(bool bTrue) { m_bDymaxion = bTrue; }
	bool IsDymaxion() const { return m_bDymaxion; }

protected:
	bool	m_bDymaxion;

	// Useful for debugging
	void LogDescription() const;
};

struct StatePlaneInfo
{
	char *name;
	int number;
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
extern Array<EPSGDatum> g_EPSGDatums;
void SetupEPSGDatums();

StatePlaneInfo *GetStatePlaneTable();
int GetNumStatePlanes();
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

///////////////////////////

// Stick this here for now, although it really belongs in its own module
class GDALWrapper
{
public:
	GDALWrapper();
	~GDALWrapper();

	void RequestGDALFormats();
	void RequestOGRFormats();

protected:
	bool m_bGDALFormatsRegistered;
	bool m_bOGRFormatsRegistered;
};
extern GDALWrapper g_GDALWrapper;

#endif	// PROJECTIONSH
