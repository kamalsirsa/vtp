//
// GEOnet.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GEONET_H
#define GEONET_H

#include "config_vtdata.h"

#if SUPPORT_WSTRING

#include "MathTypes.h"
#include "vtString.h"

class Place
{
public:
	DPoint2 m_pos;
	wstring2 m_fullname;
	vtString m_fullname_nd;
	short m_ppc; // Populated Place Classification (1 high to 5 low, 6=unknown)
};

class PlaceArray : public Array<Place *>
{
public:
	// this class is used for reference purposes only, it does not own the
	// objects it contains so it does not and should not delete them
};

class Country
{
public:
	Country();
	~Country();

	bool FindPlace(const char *name_nd, DPoint2 &point, bool bFullLength);
	bool FindPlace(const std::wstring &name, DPoint2 &point, bool bFullLength);
	bool FindAllMatchingPlaces(const char *name_nd, bool bFullLength, PlaceArray &places);

	bool WriteSHP(const char *fname);

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
	bool ReadCountryList(const char *fname);
	void ParseRawCountryFiles(const char *path_prefix, bool bNativeNames);
	void ParseRawCountry(int i, bool bNativeNames);
	void WriteGCF(const char *fname);
	bool WriteSingleSHP(const char *fname);
	bool WriteSHPPerCountry(const char *prefix);
	void Free();

	// load and use GCF
	void ReadGCF(const char *fname, void progress_callback(int) = NULL);
	bool FindPlace(const char *country, const char *place, DPoint2 &point);
	bool FindPlaceWithGuess(const char *country, const char *place, DPoint2 &point);

protected:
	Array<Country*> m_countries;
	vtString m_path;
};

#endif // SUPPORT_WSTRING

#endif // GEONET_H

