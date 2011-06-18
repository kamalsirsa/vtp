//
// SpecificTerrain.h
//

#ifndef SPECIFICTERRAINH
#define SPECIFICTERRAINH

#include "vtlib/core/Terrain.h"

class TransitTerrain : public vtTerrain
{
public:
	void CreateCustomCulture();
	vtGeode *MakeBlockGeom(FPoint3 size);
};

#endif	// SPECIFICTERRAINH

