//
// GEOnet.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "GEOnet.h"
#include "vtdata/shapelib/shapefil.h"
#include <stdio.h>

#if SUPPORT_WSTRING

Country::Country()
{
}

Country::~Country()
{
	// manually free memory
	int j, num_places = m_places.GetSize();
	for (j = 0; j < num_places; j++)
		delete m_places[j];
	m_places.Empty();
}

bool Country::FindPlace(const char *name_nd, DPoint2 &point, bool bFullLength)
{
	int j, num_places = m_places.GetSize();

	bool match = false;
	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];
		if (bFullLength)
			match = (place->m_fullname_nd.CompareNoCase(name_nd) == 0);
		else
		{
			int len = strlen(name_nd);
			if (len > 2)
				match = (place->m_fullname_nd.Left(len).CompareNoCase(name_nd) == 0);
		}
		if (match)
		{
			point = place->m_pos;
			return true;
		}
	}
	return false;
}

bool Country::FindPlace(const std::wstring &name, DPoint2 &point, bool bFullLength)
{
	int j, num_places = m_places.GetSize();

	bool match;
	for (j = 0; j < num_places; j++)
	{
		match = false;
		Place *place = m_places[j];

		if (bFullLength)
			match = (place->m_fullname.compare(name) == 0);
		else
		{
			int len = name.length();
			if (len > 2)
				match = (place->m_fullname.compare(0, len, name) == 0);
		}
		if (match)
		{
			point = place->m_pos;
			return true;
		}
	}
	return false;
}

bool Country::FindAllMatchingPlaces(const char *name_nd, bool bFullLength, PlaceArray &places)
{
	int j, num_places = m_places.GetSize();

	bool match = false;
	places.Empty();

	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];
		if (bFullLength)
			match = (place->m_fullname_nd.CompareNoCase(name_nd) == 0);
		else
		{
			int len = strlen(name_nd);
			if (len > 2)
				match = (place->m_fullname_nd.Left(len).CompareNoCase(name_nd) == 0);
		}
		if (match)
		{
			places.Append(place);
		}
	}
	if (places.GetSize() > 0)
		return true;
	else
		return false;
}

bool Country::WriteSHP(const char *fname)
{
	SHPHandle hSHP = SHPCreate(fname, SHPT_POINT);
	if (!hSHP)
		return false;

	SHPObject *obj;

	int j, num_places;
	int longest_place_name = 0;

	num_places = m_places.GetSize();

	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];

		obj = SHPCreateSimpleObject(SHPT_POINT, 1,
			&place->m_pos.x, &place->m_pos.y, NULL);

		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);

		const char *utf8 = place->m_fullname.to_utf8();
		int len = strlen(utf8);
		if (len > longest_place_name)
			longest_place_name = len;
	}
	SHPClose(hSHP);

	// Save DBF File also
	vtString dbfname = fname;
	dbfname = dbfname.Left(dbfname.GetLength() - 4);
	dbfname += ".dbf";
	DBFHandle db = DBFCreate(dbfname);
	if (db == NULL)
		return false;

	DBFAddField(db, "Country", FTString, 2, 0);
	DBFAddField(db, "PPC", FTInteger, 2, 0);
	DBFAddField(db, "Name", FTString, longest_place_name, 0);

	int entity = 0;

	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];

		DBFWriteStringAttribute(db, entity, 0, (const char *) m_abb);

		DBFWriteIntegerAttribute(db, entity, 1, place->m_ppc);

		const char *utf8 = place->m_fullname.to_utf8();
		DBFWriteStringAttribute(db, entity, 2, utf8);

		entity++;
	}
	DBFClose(db);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//

bool Countries::ReadCountryList(const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return false;

	int off;
	vtString str, name, abb;
	char buf[400];
	while (fgets(buf, 400, fp) != NULL)
	{
		str = buf;
		off = str.Find(':');
		Country *country = new Country;
		country->m_full = str.Left(off);
		country->m_abb = str.Mid(off+1, 2);
		m_countries.Append(country);
	}
	fclose(fp);
	return true;
}

void Countries::ParseRawCountryFiles(const char *path_prefix, bool bNativeNames)
{
	m_path = path_prefix;

	unsigned int num = m_countries.GetSize();
	for (int unsigned i = 0; i < num; i++)
		ParseRawCountry(i, bNativeNames);
}

bool Countries::WriteSHPPerCountry(const char *prefix)
{
	int unsigned num = m_countries.GetSize();
	for (int unsigned i = 0; i < num; i++)
	{
		Country *country = m_countries[i];

		if (country->m_places.GetSize() > 0)
		{
			vtString out_path = prefix;
			out_path += country->m_abb;
			out_path += ".shp";
			if (!country->WriteSHP(out_path))
				return false;
		}
	}
	return true;
}

void Countries::Free()
{
	int unsigned num = m_countries.GetSize();
	for (int unsigned i = 0; i < num; i++)
	{
		Country *country = m_countries[i];
		delete country;
	}
	m_countries.SetSize(0);
}


#include "GnsCodes.h"

void GNS_to_wstring(int region, const char *word, std::wstring &output)
{
	int len = strlen(word);
	unsigned char ch;
	unsigned short unicode;

	output.clear();

	for (int i = 0; i < len; i++)
	{
		ch = (unsigned char) word[i];
		if (ch < 128)
			unicode = ch;
		else
		{
			if (region == 1)
				unicode = region1[ch-128];
			else if (region == 2)
				unicode = region2[ch-128];
			else if (region == 3)
				unicode = region3[ch-128];
			else // TODO: other 3 regions
				unicode = region3[ch-128];
			if (unicode == 0x0000)
				unicode = '?';
		}
		output += unicode;
	}
}

/*
	Fields:
	 0 RC Region Code
		1 = Western Europe/Americas;
		2 = Eastern Europe;
		3 = Africa/Middle East;
		4 = Central Asia;
		5 = Asia/Pacific;
		6 = Vietnam
	 1 UFI Unique Feature Identifier
	 2 UNI Unique Name Identifier
	 3 DD_LAT Latitude
	 4 DD_LONG Longitude
	 5 DMS_LAT Latitude
	 6 DMS_LONG Longitude
	 7 UTM Universal Transverse Mercator
	 8 JOG Joint Operations Graphic
	 9 FC Feature Classification
		A = Administrative region;
		P = Populated place;
		V = Vegetation;
		L = Locality or area;
		U = Undersea;
		R = Streets, highways, roads, or railroad;
		T = Hypsographic;
		H = Hydrographic;
		S = Spot feature.
	10 DSG Feature Designation Code
	11 PC Populated Place Classification (1 high, 5 low)
	12 CC1 Primary Country Code
	13 ADM1 First-order administrative division
	14 ADM2 Second-order administrative division
	15 DIM Dimension
	16 CC2 Secondary Country Code
	17 NT Name Type
		C = Conventional;
		D = Not verified;
		N = Native;
		V = Variant or alternate.
	18 LC Language Code
	19 SHORT_FORM
	20 GENERIC (does not apply to populated place names)
	21 SORT_NAME Upper-case form of the full name for easy sorting of the name
	22 FULL_NAME specific name, generic name, and any articles or prepositions
	23 FULL_NAME_ND Diacritics/special characters substituted with Roman characters
	24 MODIFY_DATE
*/

void Countries::ParseRawCountry(int which, bool bNativeNames)
{
	Country *country = m_countries[which];

	printf("Parsing %s...", (const char *) country->m_full);

	int i, num_important[7];
	for (i = 0; i < 7; i++)
		num_important[i] = 0;

	FILE *fp = fopen(m_path + country->m_abb + ".txt", "rb");
	if (!fp)
	{
		printf("couldn't open.\n");
		return;
	}

	DPoint2 point;
	char fc;	// feature classification, P = populated palce
	char pc;	// Populated Place Classification
	char nt;	// Name Type
	char *p, *w;
	vtString fullname, fullname_nd;
	char buf[4000];
	char word[200];	// some place names in Russia are more than 100 chars!
	int line_number = 0;
	Place *place;
	int region;
	bool bSkip;

	while (fgets(buf, 4000, fp) != NULL)
	{
		bSkip = false;
		line_number++;
		i = 0;
		w = word;
		for (p = buf; *p != '\n'; p++)
		{
			if (*p == '\t')
			{
				*w = '\0';
				// handle word
				if (i == 0)
					region = atoi(word); // RC region code
				if (i == 3)
					point.y = atof(word); // lat
				if (i == 4)
					point.x = atof(word); // lon
				if (i == 9)
					fc = word[0];
				if (i == 11)
					pc = word[0] ? word[0] - '0' : 6;	// "importance" 1-5, 1 most
				if (i == 17)	// Name Type
					nt = word[0];
				if (i == 22)	// FULL_NAME
					fullname = word;
				if (i == 23)	// FULL_NAME_ND
					fullname_nd = word;

				i++;
				w = word;
			}
			else
			{
				*w = *p;
				w++;
			}
		}
		if (fc != 'P')	// populated place
			bSkip = true;
		if (nt != 'N' && bNativeNames == true)
			bSkip = true;

		if (bSkip)
			continue;

		// count how many of each importance
		if (pc >= 1 && pc <= 6) num_important[pc] ++;

		place = new Place();
		place->m_pos = point;
		place->m_ppc = pc;
		country->m_places.Append(place);

		GNS_to_wstring(region, fullname, place->m_fullname);
		place->m_fullname_nd = fullname_nd;
	}
	fclose(fp);
	int size = country->m_places.GetSize();
	printf("\n  %d places. Importances: %d %d %d %d %d (%d)\n",
		size, num_important[1], num_important[2], num_important[3],
		num_important[4], num_important[5], num_important[6]);
}

void WriteString(FILE *fp, const vtString &str)
{
	short len = str.GetLength();
	fwrite(&len, 2, 1, fp);
	const char *buf = (const char *)str;
	fwrite(buf, len, 1, fp);
}

void ReadString(FILE *fp, vtString &str)
{
	char buf[100];
	short len;
	fread(&len, 2, 1, fp);
	fread(buf, len, 1, fp);
	buf[len] = '\0';
	str = buf;
}

bool Countries::WriteSingleSHP(const char *fname)
{
	SHPHandle hSHP = SHPCreate(fname, SHPT_POINT);
	if (!hSHP)
		return false;

	SHPObject *obj;

	int i, j, num_places;
	int longest_place_name = 0;
	int num_countries = m_countries.GetSize();

	for (i = 0; i < num_countries; i++)
	{
		Country *country = m_countries[i];
		num_places = country->m_places.GetSize();

		for (j = 0; j < num_places; j++)
		{
			Place *place = country->m_places[j];

			obj = SHPCreateSimpleObject(SHPT_POINT, 1,
				&place->m_pos.x, &place->m_pos.y, NULL);

			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);

			const char *utf8 = place->m_fullname.to_utf8();
			int len = strlen(utf8);
			if (len > longest_place_name)
				longest_place_name = len;
		}
	}
	SHPClose(hSHP);

	// Save DBF File also
	vtString dbfname = fname;
	dbfname = dbfname.Left(dbfname.GetLength() - 4);
	dbfname += ".dbf";
	DBFHandle db = DBFCreate(dbfname);
	if (db == NULL)
		return false;

	DBFAddField(db, "Country", FTString, 2, 0);
	DBFAddField(db, "PPC", FTInteger, 2, 0);
	DBFAddField(db, "Name", FTString, longest_place_name, 0);

	int entity = 0;
	for (i = 0; i < num_countries; i++)
	{
		Country *country = m_countries[i];
		num_places = country->m_places.GetSize();
		for (j = 0; j < num_places; j++)
		{
			Place *place = country->m_places[j];

			DBFWriteStringAttribute(db, entity, 0, (const char *) country->m_abb);

			DBFWriteIntegerAttribute(db, entity, 1, place->m_ppc);

			const char *utf8 = place->m_fullname.to_utf8();
			DBFWriteStringAttribute(db, entity, 2, utf8);

			entity++;
		}
	}
	DBFClose(db);

	return true;
}

void Countries::WriteGCF(const char *fname)
{
	FILE *fp = fopen(fname, "wb");

	int num = m_countries.GetSize();
	fwrite(&num, sizeof(int), 1, fp);

	int i, j, num_places;
	for (i = 0; i < num; i++)
	{
		Country *country = m_countries[i];
		num_places = country->m_places.GetSize();

		WriteString(fp, country->m_full);
		fwrite(&num_places, sizeof(int), 1, fp);

		for (j = 0; j < num_places; j++)
		{
			Place *place = country->m_places[j];
			fwrite(&place->m_pos.x, sizeof(double), 2, fp);
			WriteString(fp, place->m_fullname_nd);
		}
	}
	fclose(fp);
}

void Countries::ReadGCF(const char *fname, void progress_callback(int))
{
	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return;

	int num;
	fread(&num, sizeof(int), 1, fp);

	m_countries.SetMaxSize(num);

	int i, j, num_places;
	for (i = 0; i < num; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / num);

		Country *country = new Country();
		m_countries.Append(country);

		ReadString(fp, country->m_full);
		printf("Reading %s...\n", (const char *) country->m_full);

		fread(&num_places, sizeof(int), 1, fp);
		country->m_places.SetMaxSize(num_places);

		for (j = 0; j < num_places; j++)
		{
			Place *place = new Place();
			fread(&place->m_pos.x, sizeof(double), 2, fp);
			ReadString(fp, place->m_fullname_nd);
			country->m_places.Append(place);
		}
	}
	fclose(fp);
}

bool Countries::FindPlace(const char *country_val, const char *place_val,
						  DPoint2 &point)
{
	Country *country;

	int num = m_countries.GetSize();

	int i;
	for (i = 0; i < num; i++)
	{
		country = m_countries[i];

		if (country->m_full.CompareNoCase(country_val) != 0)
			continue;

		bool success = country->FindPlace(place_val, point, true);

		// try again with just the length of the initial sea
		if (!success)
			success = country->FindPlace(place_val, point, false);

		if (success)
			return true;
	}
	return false;
}

#endif // SUPPORT_WSTRING

