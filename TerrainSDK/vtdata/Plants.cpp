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

#include "Plants.h"
#include "MathTypes.h"
#include "xmlhelper/easyxml.hpp"
#include "shapelib/shapefil.h"

extern int FindDBField(DBFHandle db, const char *field_name);

float vtPlantAppearance::s_fTreeScale = 1.0f;

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


void vtPlantSpecies::SetCommonName(const char *CommonName)
{
	m_szCommonName = CommonName;
}

void vtPlantSpecies::SetSciName(const char *SciName)
{
	m_szSciName = SciName;
}


/////////////////////////////////////////////////////////////////////////

vtPlantList::vtPlantList()
{
}

vtPlantList::~vtPlantList()
{
	for (unsigned int i = 0; i < m_Species.GetSize(); i++)
	{
		delete m_Species[i];
	}
}

bool vtPlantList::Read(const char *fname)
{
	char buf1[80], buf2[80], buf3[80];
	int iApps = 0, j, apptype, iNumSpecies = 0, iSpecieID = 0;
	float f1, f2, f3, f4;

	FILE *fp = fopen(fname, "r");
	if (!fp) return false;

	fread(buf1, 13, 1, fp);
	if (strncmp(buf1, "plantlist1.0\n", 12))
		return false;

	fscanf(fp, "total species: %d\n", &iNumSpecies);
	for (int i = 0; i < iNumSpecies; i++)
	{
		fscanf(fp, "species %d: %s %s %s\n", &iSpecieID, buf1, buf2, buf3);
		strcat(buf2, " ");
		strcat(buf2, buf3);
		fscanf(fp, "max height: %f\n", &f1);

		AddSpecies(iSpecieID, buf1, buf2, f1);
		vtPlantSpecies *pSpecies = GetSpecies(i);

		fscanf(fp, "appearances: %d\n", &iApps);
		for (j = 0; j < iApps; j++)
		{
			fscanf(fp, "%d %s %f %f %f %f\n", &apptype, buf1, &f1, &f2, &f3, &f4);
			pSpecies->AddAppearance((apptype == 1) ? AT_BILLBOARD : AT_XFROG,
				buf1, f1, f2, f3, f4);
		}
	}
	fclose(fp);
	return true;
}

bool vtPlantList::Write(const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	if (!fp) return false;

	fprintf(fp, "plantlist1.0\n");
	fprintf(fp, "total species: %d\n", NumSpecies());
	for (int i = 0; i < NumSpecies(); i++)
	{
		fprintf(fp, "species %d: %s %s\n",
			GetSpecies(i)->GetSpecieID(),
			GetSpecies(i)->GetCommonName(),
			GetSpecies(i)->GetSciName() );
		fprintf(fp, "max height: %f\n", GetSpecies(i)->GetMaxHeight() );
		fprintf(fp, "appearances: %d\n", GetSpecies(i)->NumAppearances() );

		for (int j = 0; j < GetSpecies(i)->NumAppearances(); j++)
		{
			vtPlantAppearance* app = GetSpecies(i)->GetAppearance(j);
			fprintf(fp, "%d %s %f %f %f %f\n",
				app->m_eType, (const char *)app->m_filename, app->m_width,
				app->m_height, app->m_shadow_radius, app->m_shadow_darkness);
		}
	}
	fclose(fp);
	return true;
}


bool vtPlantList::WriteXML(const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(fname),
				"XML Writer");
	}

	fprintf(fp, "<?xml version=\"1.0\"?>\n\n");
	fprintf(fp, "<species-file file-format-version=\"1.0\">\n");

	for (int i = 0; i < NumSpecies(); i++)
	{
		fprintf(fp, "\t<species id=\"%d\" name=\"%s\" max_height=\"%.2f\">\n",
			GetSpecies(i)->GetSpecieID(),
			GetSpecies(i)->GetSciName(),
			GetSpecies(i)->GetMaxHeight());
		fprintf(fp, "\t\t<common name=\"%s\" />\n",
			GetSpecies(i)->GetCommonName());
		for (int j = 0; j < GetSpecies(i)->NumAppearances(); j++)
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


void vtPlantList::AddSpecies(int SpecieID, const char *CommonName,
						   const char *SciName, float MaxHeight)
{
	vtPlantSpecies *pSpecie = new vtPlantSpecies();
	pSpecie->SetSpecieID(SpecieID);
	pSpecie->SetCommonName(CommonName);
	pSpecie->SetSciName(SciName);
	pSpecie->SetMaxHeight(MaxHeight);
	m_Species.Append(pSpecie);
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

void vtPlantList::LookupPlantIndices(vtBioType *bt)
{
	for (unsigned int i = 0; i < bt->m_Densities.GetSize(); i++)
	{
		vtString common_name = RemSpaces(bt->m_Densities[i]->m_common_name);

		bt->m_Densities[i]->m_list_index = -1;
		for (int j = 0; j < NumSpecies(); j++)
		{
			vtPlantSpecies *ps = GetSpecies(j);
			if (common_name == RemSpaces(ps->GetCommonName()))
			{
				bt->m_Densities[i]->m_list_index = j;
				break;
			}
		}
	}
}

int vtPlantList::GetSpeciesIdByName(const char *name)
{
	for (int j = 0; j < NumSpecies(); j++)
	{
		if (!strcmp(name, m_Species[j]->GetSciName()))
			return j;
	}
	return -1;
}

int vtPlantList::GetSpeciesIdByCommonName(const char *name)
{
	for (int j = 0; j < NumSpecies(); j++)
	{
		if (!strcmp(name, m_Species[j]->GetCommonName()))
			return j;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of PlantList files.
////////////////////////////////////////////////////////////////////////

class PlantListVisitor : public XMLVisitor
{
public:
	PlantListVisitor(vtPlantList *pl) :
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

	vtPlantList *m_pPL;
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

			// Get id, name, max height.
			attval = atts.getValue("id");
			if (attval != NULL)
				m_pSpecies->SetSpecieID(atoi(attval));
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
			// Get the common name (assumes English language)
			attval = atts.getValue("name");
			if (attval != NULL)
				m_pSpecies->SetCommonName(attval);
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

bool vtPlantList::ReadXML(const char* pathname)
{
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

bool vtBioRegion::Read(const char *fname)
{
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
			bt->AddPlant(j, common_name, plant_per_m2);
		}
		m_Types.Append(bt);
	}
	fclose(fp);
	return true;
}

bool vtBioRegion::Write(const char *fname)
{
	FILE *fp = fopen(fname, "wb");
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
	return true;
}

int vtBioRegion::FindBiotypeIdByName(const char *name)
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

void vtBioType::AddPlant(int i, const char *common_name, float plant_per_m2)
{
	vtPlantDensity *pd = new vtPlantDensity;
	pd->m_common_name = common_name;
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
	m_pPlantList = NULL;
}

void vtPlantInstanceArray::AddInstance(DPoint2 &pos, float size,
									   short species_id)
{
	vtPlantInstance pi;
	pi.m_p = pos;
	pi.size = size;
	pi.species_id = species_id;
	Append(pi);
}

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

	int size;
	fread(&size, 4, 1, fp);
	SetSize(size);

	if (version == 1.0f)
	{
		PlantInstance10 *pOld = new PlantInstance10[size];
		fread(pOld, sizeof(PlantInstance10), size, fp);
		vtPlantInstance pi;
		for (int i = 0; i < size; i++)
		{
			pi.m_p.x = pOld[i].x;
			pi.m_p.y = pOld[i].y;
			pi.size = pOld[i].size;
			pi.species_id = pOld[i].species_id;
			SetAt(i, pi);
		}
		delete pOld;
	}
	else if (version == 1.1f)
	{
		PlantInstance11 *pTemp = new PlantInstance11[size];
		fread(pTemp, sizeof(PlantInstance11), size, fp);
		vtPlantInstance pi;
		for (int i = 0; i < size; i++)
		{
			pi.m_p = pTemp[i].m_p;
			pi.size = pTemp[i].size;
			pi.species_id = pTemp[i].species_id;
			SetAt(i, pi);
		}
		delete pTemp;
	}
	else
		return false;
	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::ReadVF(const char *fname)
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
	SetSize(numinstances);

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
		GetAt(i).m_p = origin + DPoint2(local_offset);

		// height in centimeters
		fread(&height, sizeof(short), 1, fp);
		GetAt(i).size = (float) height / 100.0f;

		// species id
		fread(&local_species_id, sizeof(short), 1, fp);
		// convert from file-local id to new id
		GetAt(i).species_id = temp_ids[local_species_id];
	}

	delete temp_ids;
	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::WriteVF(const char *fname)
{
	int i, numinstances = GetSize();
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
	GetExtent(rect);
	DPoint2 origin, diff;
	rect.GetCenter(origin);
	fwrite(&origin, sizeof(double), 2, fp);

	// write instances
	FPoint2 offset;
	for (i = 0; i < numinstances; i++)
	{
		// location
		diff = GetAt(i).m_p - origin;
		offset = diff;	// acceptable to use single precision for local offset
		fwrite(&offset, sizeof(float), 2, fp);

		// height in centimeters
		short height = (short) (GetAt(i).size * 100);
		fwrite(&height, sizeof(short), 1, fp);

		// species id
		short species_id = GetAt(i).species_id;
		fwrite(&species_id, sizeof(short), 1, fp);
	}

	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::ReadSHP(const char *fname)
{
	SHPHandle hSHP = SHPOpen(fname, "rb");
	if (hSHP == NULL)
		return false;

	int i, nEntities, nShapeType;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);
	if (nShapeType != SHPT_POINT)
		return false;

	// Open DBF File & Get DBF Info:
	DBFHandle db = DBFOpen(fname, "rb");
	if (db == NULL)
		return false;

	int	field_height = FindDBField(db, "Height");
	int	field_id = FindDBField(db, "Species");
	if (field_height == -1 || field_id == -1)
		return false;

	SetSize(nEntities);
	for (i = 0; i < nEntities; i++)
	{
		SHPObject *psShape = SHPReadObject(hSHP, i);

		GetAt(i).m_p.Set(psShape->padfX[0], psShape->padfY[0]);

		// height
		double height = DBFReadDoubleAttribute(db, i, field_height);
		GetAt(i).size = (float) height;

		// species id
		GetAt(i).species_id = DBFReadIntegerAttribute(db, i, field_id);
	}
	DBFClose(db);
	SHPClose(hSHP);
	return true;
}

bool vtPlantInstanceArray::WriteSHP(const char *fname)
{
	SHPHandle hSHP = SHPCreate(fname, SHPT_POINT);
	if (!hSHP)
		return false;

	SHPObject *obj;

	int i, numinstances = GetSize();

	for (i = 0; i < numinstances; i++)
	{
		vtPlantInstance &plant = GetAt(i);

		obj = SHPCreateSimpleObject(SHPT_POINT, 1,
			&plant.m_p.x, &plant.m_p.y, NULL);

		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);
	}
	SHPClose(hSHP);

	// Save DBF File also
	vtString dbfname = fname;
	dbfname = dbfname.Left(dbfname.GetLength() - 4);
	dbfname += ".dbf";
	DBFHandle db = DBFCreate(dbfname);
	if (db == NULL)
		return false;

	DBFAddField(db, "Height", FTDouble, 4, 2);
	DBFAddField(db, "Species", FTInteger, 6, 0);

	for (i = 0; i < numinstances; i++)
	{
		vtPlantInstance &plant = GetAt(i);

		DBFWriteDoubleAttribute(db, i, 0, plant.size);
		DBFWriteIntegerAttribute(db, i, 1, plant.species_id);
	}
	DBFClose(db);

	// and projection
	char prj_name[256];
	strcpy(prj_name, fname);
	int len = strlen(prj_name);
	strcpy(prj_name + len - 4, ".prj"); // overwrite the .bt
	m_proj.WriteProjFile(prj_name);

	return true;
}

bool vtPlantInstanceArray::FindClosestPlant(const DPoint2 &point, double error_meters,
											int &plant, double &closest)
{
	plant = -1;
	closest = 1E8;

	if (IsEmpty())
		return false;

	double dist;

	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtPlantInstance &pi = GetAt(i);
		dist = (pi.m_p - point).Length();
		if (dist > error_meters)
			continue;
		if (dist < closest)
		{
			plant = i;
			closest = dist;
		}
	}
	return (plant != -1);
}

bool vtPlantInstanceArray::GetExtent(DRECT &rect)
{
	int size = GetSize();
	if (size == 0)
		return false;
	for (int i = 0; i < size; i++)
		rect.GrowToContainPoint(GetAt(i).m_p);
	return true;
}
