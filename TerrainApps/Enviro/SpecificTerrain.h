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
	void CreateCustomCulture();
};

class TransitTerrain : public PTerrain
{
public:
	void CreateCustomCulture();
	vtGeom *MakeBlockGeom(FPoint3 size);
};

#endif	// SPECIFICTERRAINH

