//
// Island.h
//
// This class contains the structures which define the island
//

#ifndef ISLANDH
#define ISLANDH

#include "vtlib/core/Terrain.h"
#include "PTerrain.h"
#include "Engines.h"

// extents of the island landmass
#define WEST_EXTENT		-156.064270f
#define EAST_EXTENT		-154.8093872f 
#define SOUTH_EXTENT	 18.9136925f
#define NORTH_EXTENT	 20.2712955f

#define	TRANS_XAxis	FPoint3(1,0,0)
#define	TRANS_YAxis	FPoint3(0,1,0)
#define	TRANS_ZAxis	FPoint3(0,0,1)

class IslandTerrain : public PTerrain
{
public:
	IslandTerrain();
	~IslandTerrain();

	// methods
	void CreateCustomCulture(bool bDoSound);
	vtGeom *make_red_cube();
	void create_airports();
	void create_telescopes();
	void create_airplanes(float fSize, float fSpeed, bool bDoSound);

	vtMaterialArray *m_pDetailMats;
	vtMaterial *m_pDetailMat;
};

#endif
