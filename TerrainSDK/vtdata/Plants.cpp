//
// Plants.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vtLog.h"
#include "Plants.h"
#include "MathTypes.h"
#include "xmlhelper/easyxml.hpp"
#include "shapelib/shapefil.h"

extern int FindDBField(DBFHandle db, const char *field_name);

/////////////////////////

vtPlantAppearance::vtPlantAppearance(AppearType type, const char *filename,
								 float width, float height,
								 float shadow_radius, float shadow_darkness)
{
	m_eType = type;
	m_filename = filename;
	m_width = width;
	m_height = height;
	m_shadow_radius = shadow_radius;
	m_shadow_darkness = shadow_darkness;
}

vtPlantAppearance::vtPlantAppearance()
{
}

vtPlantAppearance::~vtPlantAppearance()
{
}

////////////////////////////////////////////////////////////////////////

vtPlantSpecies::vtPlantSpecies()
{
}


vtPlantSpecies::~vtPlantSpecies()
{
	for (unsigned int i = 0; i < m_Apps.GetSize(); i++)
	{
		delete m_Apps[i];
	}
}


/**
 * Set the common name for this species.  Language is options.
 * 
 \param Name The common name in UTF-8.  Example: "Colorado Blue Spruce".
 \param Lang The language, as a lower-case two-character ISO 639 standard language code.
 *	Examples: en for English, de for German, zh for Chinese.
 *  The default is English.
 */
void vtPlantSpecies::AddCommonName(const char *Name, const char *Lang)
{
	CommonName name;
	name.m_strName = Name;

	if (Lang != NULL)
		name.m_strLang = Lang;
	else
		name.m_strLang = "en";	// default language is English
	m_CommonNames.push_back(name);
}

/**
 * Set the scientific name for this species, as a plain ASCII string.
 * It should have the standard form with the genus capitolized,
 * e.g. "Cocos nucifera"
 */
void vtPlantSpecies::SetSciName(const char *SciName)
{
	m_szSciName = SciName;
}


/////////////////////////////////////////////////////////////////////////

vtSpeciesList::vtSpeciesList()
{
}

vtSpeciesList::~vtSpeciesList()
{
	for (unsigned int i = 0; i < m_Species.GetSize(); i++)
	{
		delete m_Species[i];
	}
}

bool vtSpeciesList::WriteXML(const char *fname)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	FILE *fp = fopen(fname, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(fname),
				"XML Writer");
	}

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
	fprintf(fp, "<species-file file-format-version=\"1.0\">\n");

	for (unsigned int i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies *spe = GetSpecies(i);
		fprintf(fp, "\t<species name=\"%s\" max_height=\"%.2f\">\n",
			spe->GetSciName(),
			spe->GetMaxHeight());
		for (unsigned int j = 0; j < spe->NumCommonNames(); j++)
		{
			vtPlantSpecies::CommonName cname = spe->GetCommonName(j);
			fprintf(fp, "\t\t<common name=\"%s\" lang=\"%s\" />\n",
				(const char *) cname.m_strName,
				(const char *) cname.m_strLang);
		}
		for (unsigned int j = 0; j < GetSpecies(i)->NumAppearances(); j++)
		{
			vtPlantAppearance* app = GetSpecies(i)->GetAppearance(j);
			fprintf(fp, "\t\t<appearance type=\"%d\" filename=\"%s\" "
				"width=\"%.2f\" height=\"%.2f\" shadow_radius=\"%.1f\" shadow_darkness=\"%.1f\" />\n",
				app->m_eType, (const char *)app->m_filename, app->m_width,
				app->m_height, app->m_shadow_radius, app->m_shadow_darkness);
		}
		fprintf(fp, "\t</species>\n");
	}
	fprintf(fp, "</species-file>\n");
	fclose(fp);
	return true;
}


int vtSpeciesList::FindSpeciesId(vtPlantSpecies *ps)
{
	for (unsigned int i = 0; i < m_Species.GetSize(); i++)
	{
		if (m_Species[i] == ps)
			return i;
	}
	return -1;
}

vtString RemSpaces(const vtString &str)
{
	vtString out;
	for (int i = 0; i < str.GetLength(); i++)
	{
		if (str[i] != ' ')
			out += str[i];
	}
	return out;
}

/*void vtSpeciesList::LookupPlantIndices(vtBioType *bt)
{
	for (unsigned int i = 0; i < bt->m_Densities.GetSize(); i++)
	{
		vtString common_name = RemSpaces(bt->m_Densities[i]->m_common_name);

		bt->m_Densities[i]->m_list_index = -1;
		for (unsigned int j = 0; j < NumSpecies(); j++)
		{
			vtPlantSpecies *ps = GetSpecies(j);
			if (common_name == RemSpaces(ps->GetCommonName()))
			{
				bt->m_Densities[i]->m_list_index = j;
				break;
			}
		}
	}
}*/

int vtSpeciesList::GetSpeciesIdByName(const char *name) const
{
	for (unsigned int j = 0; j < NumSpecies(); j++)
	{
		if (!strcmp(name, m_Species[j]->GetSciName()))
			return j;
	}
	return -1;
}

int vtSpeciesList::GetSpeciesIdByCommonName(const char *name) const
{
	unsigned int i, j;
	for (i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies *spe = GetSpecies(i);
		for (j = 0; j < spe->NumCommonNames(); j++)
		{
			vtPlantSpecies::CommonName cname = spe->GetCommonName(j);
			if (!strcmp(name, cname.m_strName))
				return j;

			// also, for backward compatibility, look for a match without spaces
			vtString nospace = RemSpaces(cname.m_strName);
			if (!strcmp(name, nospace))
				return i;
		}
	}
	return i;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of PlantList files.
////////////////////////////////////////////////////////////////////////

class PlantListVisitor : public XMLVisitor
{
public:
	PlantListVisitor(vtSpeciesList *pl) :
		m_state(0), m_pPL(pl) {}

	virtual ~PlantListVisitor () {}

	void startXML() { m_state = 0; }
	void startElement(const char * name, const XMLAttributes &atts);
	void endElement(const char * name);
	void data(const char * s, int length);

private:
	void push_state()
	{
		m_state++;
		m_data = "";
	}
	void pop_state() { m_state--; }

	vtPlantSpecies *m_pSpecies;
	string m_data;
	int m_state;

	vtSpeciesList *m_pPL;
};

void PlantListVisitor::startElement(const char * name, const XMLAttributes &atts)
{
	const char *attval;

	push_state();

	if (m_state == 1)
	{
		if (string(name) != (string)"species-file")
		{
			string message = "Root element name is ";
			message += name;
			message += "; expected species-file";
			throw xh_io_exception(message, "XML Reader");
		}
	}

	if (m_state == 2)
	{
		if (string(name) == (string)"species")
		{
			m_pSpecies = new vtPlantSpecies();

			// Get name and max_height
			attval = atts.getValue("name");
			if (attval != NULL)
				m_pSpecies->SetSciName(attval);
			attval = atts.getValue("max_height");
			if (attval != NULL)
				m_pSpecies->SetMaxHeight((float)atof(attval));

		}
	}

	if (m_pSpecies && m_state == 3)
	{
		if (string(name) == (string)"common")
		{
			// Get the common name
			vtString name, lang;
			attval = atts.getValue("name");
			if (attval != NULL)
			{
				name = attval;
				attval = atts.getValue("lang");
				m_pSpecies->AddCommonName(name, attval);
			}
		}
		else if (string(name) == (string)"appearance")
		{
			AppearType type;
			vtString filename;
			float width, height, shadow_radius = 1.0f, shadow_darkness = 0.0f;
			attval = atts.getValue("type");
			if (attval != NULL)
				type = (AppearType) atoi(attval);
			attval = atts.getValue("filename");
			if (attval != NULL)
				filename = attval;
			attval = atts.getValue("width");
			if (attval != NULL)
				width = (float)atof(attval);
			attval = atts.getValue("height");
			if (attval != NULL)
				height = (float)atof(attval);
			attval = atts.getValue("shadow_radius");
			if (attval != NULL)
				shadow_radius = (float)atof(attval);
			attval = atts.getValue("shadow_darkness");
			if (attval != NULL)
				shadow_darkness = (float)atof(attval);
			m_pSpecies->AddAppearance(type, filename, width, height,
				shadow_radius, shadow_darkness);
		}
	}
}

void PlantListVisitor::endElement(const char *name)
{
	if (m_pSpecies != NULL && m_state == 2)
	{
		m_pPL->Append(m_pSpecies);
		m_pSpecies = NULL;
	}
	pop_state();
}

void PlantListVisitor::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

bool vtSpeciesList::ReadXML(const char* pathname)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	PlantListVisitor visitor(this);
	try
	{
		readXML(pathname, visitor);
	}
	catch (xh_exception &)
	{
		// TODO: would be good to pass back the error message.
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////

vtBioRegion::vtBioRegion()
{
}

vtBioRegion::~vtBioRegion()
{
	for (unsigned int i = 0; i < m_Types.GetSize(); i++)
	{
		delete m_Types[i];
	}
}

bool vtBioRegion::Read(const char *fname, const vtSpeciesList &species)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	FILE *fp = fopen(fname, "r");
	if (!fp) return false;

	int num = 0;
	char buf[80];

	fread(buf, 13, 1, fp);
	if (strncmp(buf, "bioregion1.0\n", 12))
		return false;

	fscanf(fp, "types: %d\n", &num);

	for (int i = 0; i < num; i++)
	{
		vtBioType *bt = new vtBioType;
		int num2;
		fscanf(fp, "species: %d\n", &num2);
		for (int j = 0; j < num2; j++)
		{
			char common_name[80];
			float plant_per_m2;
			fscanf(fp, "\t%s %f\n", common_name, &plant_per_m2);
			int snum = species.GetSpeciesIdByCommonName(common_name);
			if (snum != -1)
				bt->AddPlant(species.GetSpecies(snum), plant_per_m2);
		}
		m_Types.Append(bt);
	}
	fclose(fp);
	return true;
}

bool vtBioRegion::Write(const char *fname) const
{
	// TODO: replace with XML format
/*	FILE *fp = fopen(fname, "wb");
	if (!fp) return false;

	int num = m_Types.GetSize();
	fprintf(fp, "bioregion1.0\n");
	fprintf(fp, "types: %d\n", num);

	for (int i = 0; i < num; i++)
	{
		vtBioType *bt = m_Types[i];
		int num2 = bt->m_Densities.GetSize();
		fprintf(fp, "species: %d\n", num2);
		for (int j = 0; j < num2; j++)
		{
			const char *common = bt->m_Densities[j]->m_common_name;
			fprintf(fp, "\t%s %f\n", common,
				bt->m_Densities[j]->m_plant_per_m2);
		}
	}
	fclose(fp);
	*/
	return true;
}

int vtBioRegion::FindBiotypeIdByName(const char *name) const
{
	int num = m_Types.GetSize();
	for (int i = 0; i < num; i++)
	{
		vtBioType *bt = m_Types[i];
		if (bt->m_name == name)
			return i;
	}
	return -1;
}

void vtBioRegion::ResetAmounts()
{
	int num = m_Types.GetSize();
	for (int i = 0; i < num; i++)
	{
		m_Types[i]->ResetAmounts();
	}
}


///////////////////////////////////////////////////////////////////////

vtBioType::vtBioType()
{
}

vtBioType::~vtBioType()
{
	for (unsigned int i = 0; i < m_Densities.GetSize(); i++)
		delete m_Densities[i];
}

void vtBioType::AddPlant(vtPlantSpecies *pSpecies, float plant_per_m2)
{
	vtPlantDensity *pd = new vtPlantDensity;
	pd->m_pSpecies = pSpecies;
	pd->m_plant_per_m2 = plant_per_m2;
	pd->m_amount = 0.0f;
	pd->m_iNumPlanted = 0;
	m_Densities.Append(pd);
}

void vtBioType::ResetAmounts()
{
	for (unsigned int i = 0; i < m_Densities.GetSize(); i++)
		m_Densities[i]->ResetAmounts();
}

/**
 * Request a plant (species ID) from this biotype.  Returns a random plant,
 *  weighted by the relative densities of each species in this biotype.
 */
int vtBioType::GetWeightedRandomPlant()
{
	// The species are weighted by multiplying a random number against each
	//  density.  The species with the highest resulting value is chosen.
	float highest = 0;
	int picked = -1;
	float val;

	int densities = m_Densities.GetSize();
	for (int i = 0; i < densities; i++)
	{
		val = random(m_Densities[i]->m_plant_per_m2);
		if (val > highest)
		{
			highest = val;
			picked = i;
		}
	}
	return picked;
}


///////////////////////////////////////////////////////////////////////


vtPlantInstanceArray::vtPlantInstanceArray()
{
	m_SizeField = AddField("Size", FT_Float);
	m_SpeciesField = AddField("Species", FT_Short);
}

int vtPlantInstanceArray::AddPlant(const DPoint2 &pos, float size,
									   short species_id)
{
	if (size < 0.0001f || size > 100.0f)
		VTLOG(" Warning: Plant with unusual height of %f\n", size);
	int index = AddPoint(pos);
	SetValue(index, m_SizeField, size);
	SetValue(index, m_SpeciesField, species_id);
	return index;
}

int vtPlantInstanceArray::AddPlant(const DPoint2 &pos, float size,
									vtPlantSpecies *ps)
{
	int species_id = m_pPlantList->FindSpeciesId(ps);
	if (species_id == -1)
		return -1;
	return AddPlant(pos, size, species_id);
}

void vtPlantInstanceArray::SetPlant(int iNum, float size, short species_id)
{
	SetValue(iNum, m_SizeField, size);
	SetValue(iNum, m_SpeciesField, species_id);
}

void vtPlantInstanceArray::GetPlant(int iNum, float &size, short &species_id)
{
	size = GetFloatValue(iNum, m_SizeField);
	species_id = GetShortValue(iNum, m_SpeciesField);
}

/*void vtPlantInstanceArray::AppendFrom(const vtPlantInstanceArray &from)
{
	// TODO: match actual species
	for (unsigned int i = 0; i < from.GetSize(); i++)
	{
		Append(from[i]);
	}
}*/

struct PlantInstance10 {
	float x, y;
	float size;
	short species_id;
};

struct PlantInstance11 {
	DPoint2 m_p;
	float size;
	short species_id;
};

struct vtPlantInstance20 {
	DPoint2 m_p;
	float size;
	short species_id;
};

bool vtPlantInstanceArray::ReadVF_version11(const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return false;

	char buf[6];
	fread(buf, 6, 1, fp);
	if (strncmp(buf, "vf", 2))
	{
		fclose(fp);
		return false;
	}
	float version = (float) atof(buf+2);
	bool utm;
	int zone, datum;
	fread(&utm, 1, 1, fp);
	/* FIXME:  Ahoy, there be byte order issues here. See below in this routine. */
	fread(&zone, 4, 1, fp);
	fread(&datum, 4, 1, fp);
	if (utm)
		m_proj.SetUTMZone(zone);
	m_proj.SetDatum(datum);

	int i, size;
	fread(&size, 4, 1, fp);
	Reserve(size);

	if (version == 1.0f)
	{
		PlantInstance10 *pOld = new PlantInstance10[size];
		fread(pOld, sizeof(PlantInstance10), size, fp);
		vtPlantInstance20 pi;
		for (i = 0; i < size; i++)
			AddPlant(DPoint2(pOld[i].x, pOld[i].y), pOld[i].size, pOld[i].species_id);

		delete [] pOld;
	}
	else if (version == 1.1f)
	{
		PlantInstance11 *pTemp = new PlantInstance11[size];
		fread(pTemp, sizeof(PlantInstance11), size, fp);

		for (i = 0; i < size; i++)
			AddPlant(pTemp[i].m_p, pTemp[i].size, pTemp[i].species_id);

		delete [] pTemp;
	}
	else
		return false;
	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::ReadVF(const char *fname)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	FILE *fp = fopen(fname, "rb");
	if (!fp)
		return false;

	char buf[6];
	fread(buf, 6, 1, fp);
	if (strncmp(buf, "vf", 2))
	{
		fclose(fp);
		return false;
	}
	float version = (float) atof(buf+2);
	if (version < 2.0f)
	{
		fclose(fp);
		return ReadVF_version11(fname);
	}

	int i, numinstances, numspecies;

	// read WKT SRS
	short len;
	fread(&len, sizeof(short), 1, fp);

	char wkt_buf[2000], *wkt = wkt_buf;
	fread(wkt, len, 1, fp);
	OGRErr err = m_proj.importFromWkt(&wkt);
	if (err != OGRERR_NONE)
	{
		// It shouldn't be fatal to encounter a missing or unparsable projection
		// return false;
	}

	// read number of species
	fread(&numspecies, sizeof(int), 1, fp);

	// read species binomial strings, creating lookup table of new IDs
	short *temp_ids = new short[numspecies];
	char name[200];
	for (i = 0; i < numspecies; i++)
	{
		fread(&len, sizeof(short), 1, fp);
		fread(name, len, 1, fp);
		name[len] = 0;
		temp_ids[i] = m_pPlantList->GetSpeciesIdByName(name);
	}

	// read number of instances
	fread(&numinstances, sizeof(int), 1, fp);
	Reserve(numinstances);

	// read local origin (center of exents) as double-precision coordinates
	DPoint2 origin;
	fread(&origin, sizeof(double), 2, fp);

	// read instances
	short height;
	FPoint2 local_offset;
	short local_species_id;
	for (i = 0; i < numinstances; i++)
	{
		// location
		fread(&local_offset, sizeof(float), 2, fp);
		DPoint2 pos = origin + DPoint2(local_offset);

		// height in centimeters
		fread(&height, sizeof(short), 1, fp);
		float size = (float) height / 100.0f;

		// species id
		fread(&local_species_id, sizeof(short), 1, fp);

		// convert from file-local id to new id
		if (local_species_id < 0 || local_species_id > numspecies-1)
		{
			VTLOG(" Warning: species index %d out of range [0..%d]\n", local_species_id, numspecies-1);
		}
		else
		{
			short species_id = temp_ids[local_species_id];
			AddPlant(pos, size, species_id);
		}
	}

	delete [] temp_ids;
	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::WriteVF(const char *fname)
{
	int i, numinstances = GetNumEntities();
	if (numinstances == 0)
		return false;	// empty files not allowed
	if (!m_pPlantList)
		return false;
	int numspecies = m_pPlantList->NumSpecies();
	short len;	// for string lengths

	FILE *fp = fopen(fname, "wb");
	if (!fp)
		return false;

	fwrite("vf2.0", 6, 1, fp);

	// write SRS as WKT
	char *wkt;
	OGRErr err = m_proj.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
		return false;
	len = strlen(wkt);
	fwrite(&len, sizeof(short), 1, fp);
	fwrite(wkt, len, 1, fp);
	OGRFree(wkt);

	// write number of species
	fwrite(&numspecies, sizeof(int), 1, fp);

	// write species binomial strings
	for (i = 0; i < numspecies; i++)
	{
		const char *name = m_pPlantList->GetSpecies(i)->GetSciName();
		len = strlen(name);
		fwrite(&len, sizeof(short), 1, fp);
		fwrite(name, len, 1, fp);
	}

	// write number of instances
	fwrite(&numinstances, sizeof(int), 1, fp);

	// write local origin (center of exents) as double-precision coordinates
	DRECT rect;
	ComputeExtent(rect);
	DPoint2 origin, diff;
	rect.GetCenter(origin);
	fwrite(&origin, sizeof(double), 2, fp);

	// write instances
	FPoint2 offset;
	for (i = 0; i < numinstances; i++)
	{
		// location
		diff = GetPoint(i) - origin;
		offset = diff;	// acceptable to use single precision for local offset
		fwrite(&offset, sizeof(float), 2, fp);

		// height in centimeters
		short height = (short) (GetFloatValue(i, m_SizeField) * 100.0f);
		fwrite(&height, sizeof(short), 1, fp);

		// species id
		short species_id = GetShortValue(i, m_SpeciesField);
		fwrite(&species_id, sizeof(short), 1, fp);
	}

	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::ReadSHP(const char *fname)
{
	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(fname, "rb");
	if (hSHP == NULL)
		return false;

	LoadGeomFromSHP(hSHP);
	SHPClose(hSHP);

	if (!LoadDataFromDBF(fname))
		return false;

	m_SizeField = GetFieldIndex("Size");
	m_SpeciesField = GetFieldIndex("Species");

	if (m_SizeField == -1 || m_SpeciesField == -1)
		return false;

	return true;
}

