//
// vtTerrainScene - Container class for all of the terrains loaded
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "TerrainScene.h"
#include "Light.h"
#include "SkyDome.h"
#include "Terrain.h"
#include "TimeEngines.h"


///////////////////////////////////////////////////////////////////////
/**
 * A small engine that allows the SkyDome to stay around the Camera.
 */
class vtSkyTrackEngine : public vtEngine
{
public:
	vtSkyTrackEngine();
	virtual void Eval();
	vtCamera *m_pCamera;
};

vtSkyTrackEngine::vtSkyTrackEngine() : vtEngine()
{
	m_pCamera = NULL;
}

void vtSkyTrackEngine::Eval()
{
	// get the location of the camera and the target (skydome)
	vtTransform *pTarget = (vtTransform *) GetTarget();
	if (!pTarget || !m_pCamera)
		return;
	FPoint3 pos1 = m_pCamera->GetTrans();
	FPoint3 pos2 = pTarget->GetTrans();

	// move the target (skydome) to be centers in XZ on the camera
	pos2.x = pos1.x;
	pos2.z = pos1.z;
	pTarget->SetTrans(pos2);
}


///////////////////////////////////////////////////////////////////////
//
// vtTerrainScene class
//

vtTerrainScene::vtTerrainScene()
{
	horizon_color.Set(0.70f, 0.85f, 1.0f);
	azimuth_color.Set(0.12f, 0.32f, 0.70f);

	m_pTop = NULL;
	m_pSkyDome = NULL;
	m_pFirstTerrain = NULL;
	m_pCurrentTerrain = NULL;
	m_pTime = NULL;
	m_pSkyTrack = NULL;
	m_pSunLight = NULL;
	m_pAtmosphereGroup = NULL;
}

vtTerrainScene::~vtTerrainScene()
{
}

void vtTerrainScene::CleanupScene()
{
	vtGetScene()->RemoveEngine(m_pTime);
	vtGetScene()->RemoveEngine(m_pSkyTrack);

	delete m_pTime;
	delete m_pSkyTrack;

	// no need to do this explicitly, it is done by releasing the scenegraph
	// if (m_pSkyDome)
	// 	m_pSkyDome->Destroy();

	while (m_pFirstTerrain)
	{
		vtTerrain *del = m_pFirstTerrain;
		m_pFirstTerrain = m_pFirstTerrain->GetNext();
		delete del;
	}
	m_pFirstTerrain = NULL;
	m_pCurrentTerrain = NULL;

	// get anything left at the top of the scene graph
	if (m_pTop)
		m_pTop->Release();

	// free some statics
	vtStructure3d::ReleaseSharedMaterials();
	vtRoute::ReleaseMaterials();
}

void vtTerrainScene::_CreateSky(const vtStringArray &datapath)
{
	// create the sun
	VTLOG(" Creating Main Light\n");
	vtLight *pLight = new vtLight();
	pLight->SetName2("Main Light");
	m_pSunLight = new vtMovLight(pLight);
	m_pSunLight->SetName2("SunLight");
	m_pTop->AddChild(m_pSunLight);

	VTLOG(" Creating SkyDome\n");
	m_pAtmosphereGroup = new vtGroup();
	m_pAtmosphereGroup->SetName2("Atmosphere Group");
	m_pTop->AddChild(m_pAtmosphereGroup);

	// 'bsc' is the Bright Star Catalog
	vtString bsc = FindFileOnPaths(datapath, "Sky/bsc.data");
	vtString sun = FindFileOnPaths(datapath, "Sky/glow2.png");
	vtString moon = FindFileOnPaths(datapath, "Sky/moon5_256.png");

	VTLOG("  Stars: '%s'\n", (const char *) bsc);
	VTLOG("    Sun: '%s'\n", (const char *) sun);
	VTLOG("   Moon: '%s'\n", (const char *) moon);

	// create a day-night dome
	m_pSkyDome = new vtSkyDome();
	m_pSkyDome->Create(bsc, 3, 1.0f, sun, moon);	// initially unit radius
	m_pSkyDome->SetDayColors(horizon_color, azimuth_color);
	m_pSkyDome->SetName2("The Sky");
	m_pSkyDome->SetSunLight(GetSunLight());
	m_pAtmosphereGroup->AddChild(m_pSkyDome);

	m_pSkyTrack = new vtSkyTrackEngine();
	m_pSkyTrack->SetName2("Sky-Camera-Following");
	m_pSkyTrack->m_pCamera = vtGetScene()->GetCamera();
	m_pSkyTrack->SetTarget(m_pSkyDome);
	vtGetScene()->AddEngine(m_pSkyTrack);

	vtGetScene()->SetBgColor(horizon_color);
}

/**
 * Find a terrain whose name begins with a given string.
 */
vtTerrain *vtTerrainScene::FindTerrainByName(const char *name)
{
	for (vtTerrain *pTerr = m_pFirstTerrain; pTerr; pTerr=pTerr->GetNext())
	{
		if (pTerr->GetName() == name)
			return pTerr;
	}
	return NULL;
}

void vtTerrainScene::_CreateEngines()
{
	// Set Time in motion
	m_pTime = new TimeEngine();
	m_pTime->SetTarget((TimeTarget *)this);
	m_pTime->SetName2("Terrain Time");
	m_pTime->SetEnabled(false);
	vtGetScene()->AddEngine(m_pTime);
}


/**
 * Call this method once before adding any terrains, to initialize
 * the vtTerrainScene object.
 */
vtGroup *vtTerrainScene::BeginTerrainScene()
{
	VTLOG("BeginTerrainScene:\n");
	_CreateEngines();

	m_pTop = new vtGroup();
	m_pTop->SetName2("All Terrain");

	// create sky group - this holds all celestial objects
	_CreateSky(vtTerrain::s_DataPaths);

	return m_pTop;
}

/**
 * Adds a terrain to the scene.
 */
void vtTerrainScene::AppendTerrain(vtTerrain *pTerrain)
{
	// add to linked list
	pTerrain->SetNext(m_pFirstTerrain);
	m_pFirstTerrain = pTerrain;
}


/**
 * BuildTerrain constructs all geometry, textures and objects for a given terrain.
 *
 * \param pTerrain	The terrain to build.
 * \returns			A vtGroup which is the top of the terrain's scene graph.
 */
vtGroup *vtTerrainScene::BuildTerrain(vtTerrain *pTerrain)
{
	if (!pTerrain->CreateStep1())
		return NULL;

	// Set time to that of the new terrain
	m_pSkyDome->SetTime(pTerrain->GetInitialTime());

	// Tell the skydome where on the planet we are
	DPoint2 geo = pTerrain->GetCenterGeoLocation();
	m_pSkyDome->SetGeoLocation(geo);

	if (!pTerrain->CreateStep2(GetSunLight()))
		return NULL;

	if (!pTerrain->CreateStep3())
		return NULL;

	if (!pTerrain->CreateStep4())
		return NULL;

	if (!pTerrain->CreateStep5())
		return NULL;

	return pTerrain->GetTopGroup();
}


/**
 * Set the current Terrain for the scene.  There can only be one terrain
 * active a at time.  If you have more than one terrain, you can use this
 * method to switch between them.
 */
void vtTerrainScene::SetCurrentTerrain(vtTerrain *pTerrain)
{
	if (m_pCurrentTerrain != NULL)
	{
		// turn off the scene graph of the previous terrain
		m_pCurrentTerrain->Enable(false);

		// turn off the engines specific to the previous terrain
		m_pCurrentTerrain->ActivateEngines(false);
	}
	m_pCurrentTerrain = pTerrain;

	// if setting to no terrain nothing more to do
	if (!pTerrain)
	{
		m_pSkyDome->SetEnabled(false);
		m_pTime->SetEnabled(false);
		return;
	}

	// switch
	m_pTop->AddChild(m_pCurrentTerrain->GetTopGroup());
	m_pCurrentTerrain->Enable(true);

	TParams &param = m_pCurrentTerrain->GetParams();

	// switch to the projection of this terrain
	m_pCurrentTerrain->SetGlobalProjection();

	// Set background color to match the ocean
	vtGetScene()->SetBgColor(m_pCurrentTerrain->GetOceanColor());

	// Turn on the engines specific to the new terrain
	m_pCurrentTerrain->ActivateEngines(true);

	// Setup the time engine for the new terrain
	vtTime localtime = pTerrain->GetInitialTime();

	// handle the atmosphere
	_UpdateSkydomeForTerrain(pTerrain);

	m_pCurrentTerrain->SetFog(param.GetValueBool(STR_FOG));

	// Update the time engine, which also calls us back to update the skydome
	m_pTime->SetTime(localtime);
	if (param.GetValueBool(STR_TIMEON))
		m_pTime->SetSpeed(param.GetValueFloat(STR_TIMESPEED));
	else
		m_pTime->SetSpeed(0.0f);
	m_pTime->SetEnabled(true);
}

void vtTerrainScene::_UpdateSkydomeForTerrain(vtTerrain *pTerrain)
{
	TParams &param = m_pCurrentTerrain->GetParams();

	// move the sky to fit the new current terrain
	// use 5x larger than terrain's maximum dimension
	vtHeightField3d *hf = pTerrain->GetHeightField();
	FRECT world_ext = hf->m_WorldExtents;
	float radius;
	float width = world_ext.Width();
	float depth = world_ext.Height();
	float minheight, maxheight;
	hf->GetHeightExtents(minheight, maxheight);

	radius = width;
	if (radius < depth)
		radius = depth;
	if (radius < maxheight)
		radius = maxheight;

	radius *= 5;
	float max_radius = 450000;
	if (radius > max_radius)
		radius = max_radius;
	m_pSkyDome->Identity();
	m_pSkyDome->Scale3(radius, radius, radius);

	// Tell the skydome where on the planet we are
	DPoint2 geo = pTerrain->GetCenterGeoLocation();
	m_pSkyDome->SetGeoLocation(geo);

	// Does this terrain want to show the skydome?
	bool bDoSky = param.GetValueBool(STR_SKY);

	m_pSkyDome->SetEnabled(bDoSky);
	if (bDoSky)
	{
		vtString fname = param.GetValueString(STR_SKYTEXTURE, true);
		if (fname != "")
		{
			vtString filename = "Sky/";
			filename += fname;
			vtString skytex = FindFileOnPaths(vtTerrain::s_DataPaths, filename);
			if (skytex != "")
				m_pSkyDome->SetTexture(skytex);
		}
	}
}

void vtTerrainScene::SetTime(const vtTime &time)
{
	if (m_pSkyDome)
	{
		// TODO? Convert to local time?
		m_pSkyDome->SetTime(time);
//		m_pSkyDome->ApplyDayColors();
// TODO? Update the fog color to match the color of the horizon.
	}
}

