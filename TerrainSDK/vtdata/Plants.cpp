//
// Plants.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Plants.h"
#include "MathTypes.h"

float vtPlantAppearance::s_fTreeScale = 1.0f;

/////////////////////////

vtPlantAppearance::vtPlantAppearance(bool billboard, const char *filename,
								 float width, float height,
								 float shadow_radius, float shadow_darkness)
{
	m_bBillboard = billboard;
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
	for (int i = 0; i < m_Apps.GetSize(); i++)
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
	for (int i = 0; i < m_Species.GetSize(); i++)
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
			pSpecies->AddAppearance((apptype == 1), buf1, f1, f2, f3, f4);
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
				app->m_bBillboard, (const char *)app->m_filename, app->m_width, app->
				m_height,
				app->m_shadow_radius, app->m_shadow_darkness);
		}
	}
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

void vtPlantList::LookupPlantIndices(vtBioType *bt)
{
	for (int i = 0; i < bt->m_Densities.GetSize(); i++)
	{
		const char *common_name = bt->m_Densities[i]->m_common_name;

		bt->m_Densities[i]->m_list_index = -1;
		for (int j = 0; j < NumSpecies(); j++)
		{
			vtPlantSpecies *ps = GetSpecies(j);
			if (!strcmp(common_name, ps->GetCommonName()))
			{
				bt->m_Densities[i]->m_list_index = j;
				break;
			}
		}
	}
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

//////////////////////////////////////////////////////////////////////////

vtBioRegion::vtBioRegion()
{
}

vtBioRegion::~vtBioRegion()
{
	for (int i = 0; i < m_Types.GetSize(); i++)
	{
		delete m_Types[i];
	}
}

bool vtBioRegion::Read(const char *fname)
{
	FILE *fp = fopen(fname, "rb");
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
			fprintf(fp, "\t%s %f\n", bt->m_Densities[j]->m_common_name,
				bt->m_Densities[j]->m_plant_per_m2);
		}
	}
	fclose(fp);
	return true;
}

///////////////////////////////////////////////////////////////////////

vtBioType::vtBioType()
{
}

vtBioType::~vtBioType()
{
	for (int i = 0; i < m_Densities.GetSize(); i++)
	{
		delete m_Densities[i];
	}
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

///////////////////////////////////////////////////////////////////////

void vtPlantInstanceArray::AddInstance(DPoint2 &pos, float size, short species_id)
{
	vtPlantInstance pi;
	pi.m_p = pos;
	pi.size = size;
	pi.species_id = species_id;
	Append(pi);
}

struct OldPlantInstance {
	float x, y;
	float size;
	short species_id;
};

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
	bool utm;
	int zone, datum;
	fread(&utm, 1, 1, fp);
	/*  FIXME:  Ahoy, there be byte order issues here.  See below in this routine.  */
	fread(&zone, 4, 1, fp);
	fread(&datum, 4, 1, fp);
	m_proj.SetUTM(utm, zone);
	m_proj.SetDatum((DATUM) datum);

	int size;
	fread(&size, 4, 1, fp);
	SetSize(size);

	if (version == 1.0f)
	{
		OldPlantInstance *pOld = new OldPlantInstance[size];
		fread(pOld, sizeof(OldPlantInstance), size, fp);
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
		fread(m_Data, sizeof(vtPlantInstance), size, fp);
	}
	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::WriteVF(const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	if (!fp)
		return false;

	fwrite("vf1.1", 6, 1, fp);
	bool utm = m_proj.IsUTM();
	int zone = m_proj.GetUTMZone(), datum = m_proj.GetDatum();
	fwrite(&utm, 1, 1, fp);
	/*  FIXME:  Ahoy, there be byte order issues here.  See below in this routine.  */
	fwrite(&zone, 4, 1, fp);
	fwrite(&datum, 4, 1, fp);

	int size = GetSize();
	fwrite(&size, 4, 1, fp);

	fwrite(m_Data, sizeof(vtPlantInstance), size, fp);
	fclose(fp);
	return true;
}

