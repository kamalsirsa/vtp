
#ifndef VEHICLEH
#define VEHICLEH

#define MAX_VEHICLE_LODS	5

class Vehicle : public vtMovLOD
{
public:
	Vehicle(vtLOD *pContained = NULL) : vtMovLOD(pContained) {}

	float	m_fSize;	// this many times larger than real
};

class VehicleType
{
public:
	VehicleType(const char *szName);

	void SetModelLod(int lod, const char *filename, float fDistance);
	Vehicle *CreateVehicle(RGBf &cColor, float fSize);

	//
	int m_iLods;
	vtString m_strTypeName;

	vtString m_strFilename[MAX_VEHICLE_LODS];
	Array<float> m_fDistance;

	VehicleType *m_pNext;

protected:
	Array<vtTransform*> m_pModels;

	bool m_bAttemptedLoaded;
	void AttemptModelLoad();
};

#endif
