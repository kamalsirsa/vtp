//
// TerrainScene.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINSCENEH
#define TERRAINSCENEH

#include "vtdata/FilePath.h"

// Forward references
class vtSkyDome;
class vtTerrain;
class TimeEngine;
class vtSkyTrackEngine;

/**
 * vtTerrainScene provides a container for all of your vtTerrain objects.
 *
 * It keeps track of which terrain is active, and allows you to switch
 * between terrains if you have more than one.  It also creates some of the
 * surrounding environment (sunlight, skydome) which is common to all
 * of the terrains.
 */
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
	bool		m_bFog;

	// start of a linked list
	vtTerrain	*m_pFirstTerrain;

	// the terrain that is currently active
	vtTerrain	*m_pCurrentTerrain;

	// engines
	TimeEngine		 *m_pTime;
	vtSkyTrackEngine *m_pSkyTrack;

	void create_skydome(const StringArray &datapath);
	void create_fog();
	void create_engines(bool bDoSound);

	vtTerrain *FindTerrainByName(const char *name);

	// scene setup
	vtRoot *BeginTerrainScene(bool bDoSound);
	void AppendTerrain(vtTerrain *pTerrain);
	virtual void Finish(const StringArray &datapath);
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
