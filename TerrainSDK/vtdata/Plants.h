//
// Plants.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_PLANTSH
#define VTDATA_PLANTSH

#include "Projections.h"
#include "vtString.h"
#include "Array.h"
#include "MathTypes.h"

class vtPlantDensity
{
public:
	void ResetAmounts() { m_amount = 0.0f; m_iNumPlanted = 0; }

	vtString	m_common_name;
	float		m_plant_per_m2;

	int			m_list_index;		// for faster lookup

	float		m_amount;			// these two fields are using during the
	int			m_iNumPlanted;		// plant distribution process
};

class vtBioType
{
public:
	vtBioType();
	~vtBioType();

	void AddPlant(int i, const char *common_name, float plant_per_m2);
	void ResetAmounts();
	int GetWeightedRandomPlant();

	Array<vtPlantDensity *> m_Densities;

	vtString	m_name;
};

class vtBioRegion
{
public:
	vtBioRegion();
	~vtBioRegion();

	bool Read(const char *fname);
	bool Write(const char *fname);
	void AddType(vtBioType *bt) { m_Types.Append(bt); }
	vtBioType *GetBioType(int i) { return m_Types[i]; }
	int FindBiotypeIdByName(const char *name);
	void ResetAmounts();

	Array<vtBioType *> m_Types;
};

enum AppearType {
	AT_UNKNOWN,
	AT_BILLBOARD,
	AT_MODEL,
	AT_XFROG
};

class vtPlantAppearance
{
public:
	vtPlantAppearance();
	vtPlantAppearance(AppearType type, const char *filename, float width,
		float height, float shadow_radius, float shadow_darkness);
	virtual ~vtPlantAppearance();

	AppearType	m_eType;
	vtString	m_filename;
	float		m_width;
	float		m_height;
	float		m_shadow_radius;
	float		m_shadow_darkness;
	static float s_fTreeScale;
};

class vtPlantSpecies {
public:
	vtPlantSpecies();
	virtual ~vtPlantSpecies();

	// copy
	vtPlantSpecies &operator=(const vtPlantSpecies &v);

	void SetSpecieID(short SpecieID) { m_iSpecieID = SpecieID; }
	short GetSpecieID() const { return m_iSpecieID; }

	void SetCommonName(const char *CommonName);
	const char *GetCommonName() const { return m_szCommonName; }

	void SetSciName(const char *SciName);
	const char *GetSciName() const { return m_szSciName; }

	void SetMaxHeight(float f) { m_fMaxHeight = f; }
	float GetMaxHeight() const { return m_fMaxHeight; }

	virtual void AddAppearance(AppearType type, const char *filename,
		float width, float height, float shadow_radius, float shadow_darkness)
	{
		vtPlantAppearance *pApp = new vtPlantAppearance(type, filename,
			width, height, shadow_radius, shadow_darkness);
		m_Apps.Append(pApp);
	}

	int NumAppearances() const { return m_Apps.GetSize(); }
	vtPlantAppearance *GetAppearance(int i) const { return m_Apps[i]; }

protected:
	short		m_iSpecieID;
	vtString	m_szCommonName;
	vtString	m_szSciName;
	float		m_fMaxHeight;
	Array<vtPlantAppearance*> m_Apps;
};


class vtPlantList
{
public:
	vtPlantList();
	virtual ~vtPlantList();

	bool Read(const char *fname);
	bool Write(const char *fname);

	bool ReadXML(const char *fname);
	bool WriteXML(const char *fname);

	void LookupPlantIndices(vtBioType *pvtBioType);
	int NumSpecies() const { return m_Species.GetSize();  }
	vtPlantSpecies *GetSpecies(unsigned int i) const
	{
		if (i < m_Species.GetSize())
			return m_Species[i];
		else
			return NULL;
	}
	int GetSpeciesIdByName(const char *name);
	int GetSpeciesIdByCommonName(const char *name);
	virtual void AddSpecies(int SpecieID, const char *common_name,
		const char *SciName, float max_height);
	void Append(vtPlantSpecies *pSpecies)
	{
		m_Species.Append(pSpecies);
	}

protected:
	Array<vtPlantSpecies*> m_Species;
};


struct vtPlantInstance {
	DPoint2 m_p;
	float size;
	short species_id;
};

class vtPlantInstanceArray : public Array<vtPlantInstance>
{
public:
	vtPlantInstanceArray();

	void SetPlantList(vtPlantList *list) { m_pPlantList = list; }
	void AddInstance(DPoint2 &pos, float size, short species_id);
	void AppendFrom(const vtPlantInstanceArray &from);

	bool ReadVF_version11(const char *fname);
	bool ReadVF(const char *fname);
	bool ReadSHP(const char *fname);
	bool WriteVF(const char *fname);
	bool WriteSHP(const char *fname);

	bool FindClosestPlant(const DPoint2 &pos, double error_meters,
		int &plant, double &distance);
	bool GetExtent(DRECT &rect);

	void GetProjection(vtProjection &proj) const { proj = m_proj; }
	void SetProjection(const vtProjection &proj) { m_proj = proj; }

protected:
	vtPlantList *m_pPlantList;
	vtProjection m_proj;
};

#endif	// VTDATA_PLANTSH

