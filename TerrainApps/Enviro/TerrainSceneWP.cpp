//
// This subclass of TerrainScene is capable of handling transitions
// between terrain.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TerrainSceneWP.h"
#include "SwapEngine.h"

TerrainSceneWP	g_TerrainScene;

TerrainSceneWP::TerrainSceneWP() : vtTerrainScene()
{
}

TerrainSceneWP::~TerrainSceneWP()
{
//	if (m_pSwapEng != NULL)
//		m_pSwapEng->Delete();
}

void TerrainSceneWP::SetupEngines()
{
	if (m_pSwapEng == NULL)
	{
		m_pSwapEng = new SwapEngine();
		m_pSwapEng->SetName2("World-Swapping Engine");
		vtGetScene()->AddEngine(m_pSwapEng);
	}
}

//
// helper functions
//

TerrainSceneWP &GetTerrainScene()
{
	return g_TerrainScene;
}

vtTerrain *GetCurrentTerrain()
{
	return g_TerrainScene.m_pCurrentTerrain;
}

