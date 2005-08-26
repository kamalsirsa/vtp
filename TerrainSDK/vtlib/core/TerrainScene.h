//
// TerrainScene.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINSCENEH
#define TERRAINSCENEH

#include "vtdata/FilePath.h"
#include "TimeEngines.h"
#include "Content3d.h"

// Forward references
class vtSkyDome;
class vtTerrain;
class TimeEngine;
class vtSkyTrackEngine;

/**
 * This class represents a single type of utility structure, such as a
 * telephone pole, power or transmission tower.
 */
class vtUtilStruct
{
public:
	vtUtilStruct()
	{
		m_pTower = NULL;
		m_iNumWires = 0;
	}

	vtNode	*m_pTower;	// The station may have a tower placed on it
	vtString m_sStructName;

	// The points at which the wires attach
	FLine3 m_fpWireAtt1;
	FLine3 m_fpWireAtt2;

	int m_iNumWires;
};

/** \addtogroup terrain */
/*@{*/

/**
 * vtTerrainScene provides a container for all of your vtTerrain objects.
 *
 * It keeps track of which terrain is active, and allows you to switch
 * between terrains if you have more than one.  It also creates some of the
 * surrounding environment (sunlight, skydome) which is common to all
 * of the terrains.
 */
class vtTerrainScene : public TimeTarget
{
public:
	vtTerrainScene();
	virtual ~vtTerrainScene();

	// scene setup
	vtGroup *BeginTerrainScene();
	void AppendTerrain(vtTerrain *pTerrain);
	void Finish(const vtStringArray &datapath) {}	// dummy; obsolete method
	void SetCurrentTerrain(vtTerrain *pTerrain);
	vtGroup *BuildTerrain(vtTerrain *pTerrain);
	void RemoveTerrain(vtTerrain *pTerrain);
	void CleanupScene();

	vtGroup *GetTop() { return m_pTop; }
	vtSkyDome *GetSkyDome() { return m_pSkyDome; }

	/// Access the terrains in the list.
	unsigned int NumTerrains() { return m_Terrains.size(); }
	vtTerrain *GetTerrain(unsigned int i) { return m_Terrains[i]; }

	/// Get the terrain currently being shown.
	vtTerrain *GetCurrentTerrain() { return m_pCurrentTerrain; }

	/// Find a terrain by its name.
	vtTerrain *FindTerrainByName(const char *name);
	vtTransform *GetSunLight() { return m_pSunLight; }
	TimeEngine *GetTimeEngine() { return m_pTimeEngine; }

	void SetTime(const vtTime &time);

	// skydome
	RGBf		horizon_color, azimuth_color;

	// utility structures, shared between all terrains
	vtUtilStruct *LoadUtilStructure(const vtString &name);
	float		m_fCatenaryFactor;

	// Statics
	// handle to the singleton
	static vtTerrainScene *s_pTerrainScene;

	// during creation, all data will be looked for on the global data path
	void SetDataPath(const vtStringArray &paths) { m_DataPaths = paths; }
	vtStringArray m_DataPaths;

	// Global content manager for content shared between all terrains
	vtContentManager3d m_Content;

protected:
	void _UpdateSkydomeForTerrain(vtTerrain *pTerrain);

	// main scene graph outline
	vtGroup		*m_pTop;
	vtSkyDome	*m_pSkyDome;

	void _CreateSky();
	void _CreateEngines();

	vtGroup		*m_pAtmosphereGroup;

	// array of terrain objects
	std::vector<vtTerrain*> m_Terrains;

	// the terrain that is currently active
	vtTerrain	*m_pCurrentTerrain;

	// engines
	TimeEngine		 *m_pTimeEngine;
	vtSkyTrackEngine *m_pSkyTrack;

	// Utility structures, created on demand from global content file
	Array<vtUtilStruct*>	m_StructObjs;

	vtTransform	*m_pSunLight;
};

// global helper function
vtTerrainScene *vtGetTS();
const vtStringArray &vtGetDataPath();
vtContentManager3d &vtGetContent();

/*@}*/	// Group terrain

#endif	// TERRAINSCENEH

