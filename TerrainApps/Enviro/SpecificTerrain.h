//
// SpecificTerrain.h
//

#ifndef SPECIFICTERRAINH
#define SPECIFICTERRAINH

#include "vtlib/core/Terrain.h"
#include "PTerrain.h"

class Romania : public vtTerrain
{
public:
	void CreateCustomCulture(bool bDoSound);
};

class TransitTerrain : public PTerrain
{
public:
	void CreateCustomCulture(bool bDoSound);
	vtGeom *MakeBlockGeom(FPoint3 size);
};

#endif	// SPECIFICTERRAINH

