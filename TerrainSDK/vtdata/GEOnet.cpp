//
// GEOnet.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "GEOnet.h"
#include <stdio.h>


bool Country::FindPlace(const char *place_val, DPoint2 &point, bool bFullLength)
{
	int j, num_places = m_places.GetSize();

	bool success = false;
	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];
		if (bFullLength)
			success = (place->m_fullname_nd.CompareNoCase(place_val) == 0);
		else
		{
			int len = strlen(place_val);
			if (len > 2)
				success = (place->m_fullname_nd.Left(len).CompareNoCase(place_val) == 0);
		}
		if (success)
		{
			point = place->m_pos;
			return true;
		}
	}
	return false;
}

void Countries::ReadCountryList(const char *fname)
{
	FILE *fp = fopen(fname, "rb");

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
}

void Countries::ParseRawCountryFiles(const char *path_prefix)
{
	m_path = path_prefix;

	int num = m_countries.GetSize();
	for (int i = 0; i < num; i++)
	{
		ParseRawCountry(i);
	}
}

void Countries::ParseRawCountry(int which)
{
	Country *country = m_countries[which];

	printf("Parsing %s...", country->m_full);

	char fc;	// feature classification, P = populated palce
	char pc;	// Populated Place Classification
	DPoint2 point;
	char *p, *w;
	int i, j;
	char buf[4000];
	char word[100];
	Place *place;

	Array<char> importance;
	int num_important[7];
	int line_number = 0;
	int duplicates = 0;

	for (i = 0; i < 7; i++)
		num_important[i] = 0;

	FILE *fp = fopen(m_path + country->m_abb + ".txt", "rb");
	if (!fp)
	{
		printf("couldn't open.\n");
		return;
	}
	while (fgets(buf, 4000, fp) != NULL)
	{
		line_number++;
		i = 0;
		w = word;
		for (p = buf; *p != '\n'; p++)
		{
			if (*p == '\t')
			{
				*w = '\0';
				// handle word
				if (i == 3)
					point.y = atof(word); // lat
				if (i == 4)
					point.x = atof(word); // lon
				if (i == 9)
					fc = word[0];
				if (i == 11)
				{
					pc = word[0] ? word[0] - '0' : 6;	// "importance" 1-5, 1 most
				}
				if (i == 23)
				{
					if (fc == 'P')	// populated place
					{
						// count hhow many of each importance
						if (pc >= 1 && pc <= 6) num_important[pc] ++;

						bool bReplace = false;
						if (pc < 5)	// might be more important than existing
						{
							int size = country->m_places.GetSize();
							for (j = 0; j < size; j++)
							{
								place = country->m_places.GetAt(j);
								if (!place->m_fullname_nd.Compare(word))
								{
									duplicates++;
									// if name already exists, replace the existing
									// place if this one is more important
									if (pc < importance[j])
									{
										bReplace = true;
										break;
									}
								}
							}
						}
						place = new Place();
						place->m_pos = point;
						place->m_fullname_nd = word;
						if (bReplace)
						{
							country->m_places[j] = place;
							importance[j] = pc;
						}
						else
						{
							country->m_places.Append(place);
							importance.Append(pc);
						}
					}
				}
				//printf("i=%d word=%s\n", i, word);
				i++;
				w = word;
			}
			else
			{
				*w = *p;
				w++;
			}
		}
	}
	fclose(fp);
	int size = country->m_places.GetSize();
	printf("%d places. Imps: %d %d %d %d %d %d. Dupes: %d\n",
		size, num_important[1], num_important[2], num_important[3],
		num_important[4], num_important[5], num_important[6], duplicates);
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
		printf("Reading %s...\n", country->m_full);

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

