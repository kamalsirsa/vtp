#ifndef TERRAINSCENEWPH
#define TERRAINSCENEWPH

#include "vtlib/core/TerrainScene.h"

class SwapEngine;

class TerrainSceneWP : public vtTerrainScene
{
public:
	TerrainSceneWP();
	~TerrainSceneWP();

	void SetupEngines();

	SwapEngine *m_pSwapEng;
};

//
// helper functions
//
TerrainSceneWP &GetTerrainScene();
vtTerrain *GetCurrentTerrain();

#endif