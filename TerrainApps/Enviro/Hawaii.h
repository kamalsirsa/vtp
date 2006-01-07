//
// Hawaii.h
//
// The IslandTerrain class contains Hawaii-specific functionality and test code.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.

#ifndef ISLANDH
#define ISLANDH

#include "vtlib/core/Terrain.h"
#include "Engines.h"
#include "Vehicles.h"

// extents of the island landmass
#define WEST_EXTENT		-156.064270f
#define EAST_EXTENT		-154.8093872f
#define SOUTH_EXTENT	 18.9136925f
#define NORTH_EXTENT	 20.2712955f

#define	TRANS_XAxis	FPoint3(1,0,0)
#define	TRANS_YAxis	FPoint3(0,1,0)
#define	TRANS_ZAxis	FPoint3(0,0,1)

class IslandTerrain : public vtTerrain
{
public:
	IslandTerrain();
	~IslandTerrain();

	// methods
	void CreateCustomCulture();

	// override for texture tests
	void PaintDib(bool progress_callback(int) = NULL);

	vtGeom *make_test_cone();
	vtGeom *make_red_cube();
	void create_state_park();
	void create_airports();
	void create_telescopes();
	void create_building_manually();
	void do_test_code();
	void create_airplanes(float fSize, float fSpeed);
	void create_airplane(int i, float fSize, float fSpeed);

	vtStructureArray3d *m_pSA;
	vtStructureArray3d *m_pTelescopes;

	VehicleManager m_Vehicles;
};

#endif // ISLANDH

