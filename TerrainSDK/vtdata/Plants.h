//
// Plants.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_PLANTSH
#define VTDATA_PLANTSH

#include "Array.h"
#include "Features.h"

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
	const char *GetCommonName() const { return m_strCommonName; }

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
	vtString	m_strCommonName;		// Stored in UTF-8
	vtString	m_szSciName;
	float		m_fMaxHeight;
	Array<vtPlantAppearance*> m_Apps;
};


class vtPlantDensity
{
public:
	void ResetAmounts() { m_amount = 0.0f; m_iNumPlanted = 0; }

	vtPlantSpecies	*m_pSpecies;
	float		m_plant_per_m2;

	float		m_amount;			// these two fields are using during the
	int			m_iNumPlanted;		// plant distribution process
};

class vtBioType
{
public:
	vtBioType();
	~vtBioType();

	void AddPlant(vtPlantSpecies *pSpecies, float plant_per_m2);
	void ResetAmounts();
	int GetWeightedRandomPlant();

	Array<vtPlantDensity *> m_Densities;

	vtString	m_name;
};

class vtSpeciesList
{
public:
	vtSpeciesList();
	virtual ~vtSpeciesList();

	bool Read(const char *fname);
	bool Write(const char *fname);

	bool ReadXML(const char *fname);
	bool WriteXML(const char *fname);

	unsigned int NumSpecies() const { return m_Species.GetSize();  }
	vtPlantSpecies *GetSpecies(unsigned int i) const
	{
		if (i < m_Species.GetSize())
			return m_Species[i];
		else
			return NULL;
	}
	int GetSpeciesIdByName(const char *name) const;
	int GetSpeciesIdByCommonName(const char *name) const;
	virtual void AddSpecies(int SpecieID, const char *common_name,
		const char *SciName, float max_height);
	void Append(vtPlantSpecies *pSpecies)
	{
		m_Species.Append(pSpecies);
	}
	int FindSpeciesId(vtPlantSpecies *ps);
	void Clear() { m_Species.Empty(); }

protected:
	Array<vtPlantSpecies*> m_Species;
};

class vtBioRegion
{
public:
	vtBioRegion();
	~vtBioRegion();

	bool Read(const char *fname, const vtSpeciesList &species);
	bool Write(const char *fname) const;
	int AddType(vtBioType *bt) { return m_Types.Append(bt); }
	int NumTypes() const { return m_Types.GetSize(); }
	vtBioType *GetBioType(int i) const { return m_Types[i]; }
	int FindBiotypeIdByName(const char *name) const;
	void ResetAmounts();
	void Clear() { m_Types.Empty(); }

	Array<vtBioType *> m_Types;
};

class vtPlantInstanceArray : public vtFeatureSetPoint2D
{
public:
	vtPlantInstanceArray();

	void SetPlantList(vtSpeciesList *list) { m_pPlantList = list; }
	int AddPlant(const DPoint2 &pos, float size, short species_id);
	int AddPlant(const DPoint2 &pos, float size, vtPlantSpecies *ps);
	void SetPlant(int iNum, float size, short species_id);
	void GetPlant(int iNum, float &size, short &species_id);

	bool ReadVF_version11(const char *fname);
	bool ReadVF(const char *fname);
	bool ReadSHP(const char *fname);
	bool WriteVF(const char *fname);

protected:
	vtSpeciesList *m_pPlantList;

	int m_SizeField;
	int m_SpeciesField;
};

#endif	// VTDATA_PLANTSH

