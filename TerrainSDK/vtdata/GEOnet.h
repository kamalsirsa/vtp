//
// GEOnet.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GEONET_H
#define GEONET_H

#include "MathTypes.h"
#include "vtString.h"

class Place
{
public:
	DPoint2 m_pos;
	vtString m_fullname_nd;
};

class Country
{
public:
	bool FindPlace(const char *place_val, DPoint2 &point, bool bFullLength);

	vtString m_abb;
	vtString m_full;
	Array<Place*> m_places;
};

/**
 * This class parses and utilizes a subset of the information from the
 * GEOnet Names Server.
 *
 * See http://www.nima.mil/gns/html/ for the description of the data and
 * the raw data files.
 *
 * From the 700 MB of raw source files, a compact 'GCF' file containing only
 * the geographic coordinate and a single name string are extracted for
 * each populated place.
 *
 * The GCF (75 MB) can be then loaded and used for rough geocoding of
 * international addresses
 */
class Countries
{
public:
	// create GCF from raw GEOnet Names Server (GNS) files
	void ReadCountryList(const char *fname);
	void ParseRawCountryFiles(const char *path_prefix);
	void ParseRawCountry(int i);
	void WriteGCF(const char *fname);

	// load and use GCF
	void ReadGCF(const char *fname, void progress_callback(int) = NULL);
	bool FindPlace(const char *country, const char *place, DPoint2 &point);

protected:
	Array<Country*> m_countries;
	vtString m_path;
};

#endif // GEONET_H

