//
// Vehicles.h
//
// Classes for describing and operating on vehicles.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VEHICLEH
#define VEHICLEH

class Vehicle : public vtTransform
{
public:
	Vehicle();

	float	m_fSize;	// this many times larger than real
	vtLOD	*m_pLOD;
};

class VehicleType
{
public:
	VehicleType(const char *szName);
	~VehicleType();

	void AddModel(const char *filename, float fScale, float fDistance);
	Vehicle *CreateVehicle(const RGBf &cColor, float fSize);
	void ReleaseModels();

	vtString m_strTypeName;

	vtStringArray m_strFilename;
	Array<float> m_fScale;
	Array<float> m_fDistance;

	VehicleType *m_pNext;

protected:
	Array<vtNodeBase*> m_pModels;

	bool m_bAttemptedLoaded;
	void AttemptModelLoad();
};

class VehicleManager
{
public:
	VehicleManager();
	~VehicleManager();

	// vehicle stuff
	void create_ground_vehicles(vtTerrain *pTerrain, float fSize, float fSpeed); // dunno what to do to speed yet

	void SetupVehicles();
	void ReleaseVehicles();
	void AddVehicleType(VehicleType *vt);
	Vehicle *CreateVehicle(const char *szType, const RGBf &cColor, float fSize);

	VehicleType *m_pFirstVehicleType;
	bool m_bAttemptedVehicleLoad;
};

#endif	// VEHICLEH

