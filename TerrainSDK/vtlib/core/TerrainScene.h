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

	// scene setup
	vtRoot *BeginTerrainScene(bool bDoSound);
	void AppendTerrain(vtTerrain *pTerrain);
	virtual void Finish(const StringArray &datapath);
	void SetTerrain(vtTerrain *pTerrain);

	/// Get the first terrain in the list.
	vtTerrain *GetFirstTerrain() { return m_pFirstTerrain; }

	/// Get the terrain currently being shown.
	vtTerrain *GetCurrentTerrain() { return m_pCurrentTerrain; }

	/// Find a terrain by its name.
	vtTerrain *FindTerrainByName(const char *name);
	vtMovLight *GetSunLight() { return m_pSunLight; }
	TimeEngine *GetTimeEngine() { return m_pTime; }

	void ToggleFog();
	void SetFog(bool fog);
	bool GetFog() { return m_bFog; }
	void SetTimeOfDay(unsigned int time, bool bFullRefresh = false);

	// main scene graph outline
	vtRoot		*m_pTop;
	vtSkyDome	*m_pSkyDome;

protected:
	void _CreateSkydome(const StringArray &datapath);
	void _CreateEngines(bool bDoSound);

	vtGroup		*m_pAtmosphereGroup;

	// atmospherics
	RGBf		horizon_color, azimuth_color;

	// start of a linked list
	vtTerrain	*m_pFirstTerrain;

	// the terrain that is currently active
	vtTerrain	*m_pCurrentTerrain;

	// engines
	TimeEngine		 *m_pTime;
	vtSkyTrackEngine *m_pSkyTrack;

	bool		m_bFog;
	vtMovLight	*m_pSunLight;
};

#endif
