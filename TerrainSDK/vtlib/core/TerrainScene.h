//
// TerrainScene.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINSCENEH
#define TERRAINSCENEH

// Forward references
class vtSkyDome;
class vtTerrain;
class TimeEngine;

//
// Container class for all of the terrains loaded
//
class vtTerrainScene
{
public:
	vtTerrainScene();
	~vtTerrainScene();

	// main scene graph outline
	vtRoot		*m_pTop;
	vtGroup		*m_pAtmosphereGroup;

	// atmospherics
	RGBf		horizon_color, azimuth_color;
	vtSkyDome	*m_pSkyDome;
//	Ref<ISMFog>	m_pFog;
	bool		m_bFog;

	// start of a linked list
	vtTerrain	*m_pFirstTerrain;

	// the terrain that is currently active
	vtTerrain	*m_pCurrentTerrain;

	// engines
	TimeEngine	*m_pTime;

	void create_skydome(vtString datapath);
	void create_fog();
	void create_engines(bool bDoSound);

	vtTerrain *FindTerrainByName(const char *name);

	// scene setup
	vtRoot *BeginTerrainScene(bool bDoSound);
	void AppendTerrain(vtTerrain *pTerrain);
	virtual void Finish(const char *datapath);
	void SetTerrain(vtTerrain *pTerrain);

	void ToggleFog();
	void SetFog(bool fog);
	void SetYon(float dist) { m_yondist = dist; }
	void SetTimeOfDay(unsigned int time, bool bFullRefresh = false);

	vtMovLight *GetSunLight() { return m_pSunLight; }

protected:
	vtMovLight	*m_pSunLight;
	float		m_yondist;
};

#endif
