//
// Transit.h
//
// This class contains the structures which define the santa clara terrain
//

#ifndef SANTACLARAH
#define SANTACLARAH

#include "vtlib/core/Terrain.h"
#include "PTerrain.h"

class TransitTerrain : public PTerrain
{
public:
	TransitTerrain();
	~TransitTerrain();

	// methods
	void CreateCustomCulture(bool bDoSound);
	vtGeom *MakeBlockGeom(FPoint3 size);

	// data

};

#endif
