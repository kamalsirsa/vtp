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

void vtTerrainScene::_CreateSkydome(const vtStringArray &datapath)
{
	if (m_pSkyDome != NULL)
		return;

	VTLOG(" Creating SkyDome\n");

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
	m_pSkyDome->SetDawnTimes(5, 0, 7, 0);
	m_pSkyDome->SetDuskTimes(17, 0, 19, 0);
	m_pSkyDome->SetDayColors(horizon_color, azimuth_color);
	m_pSkyDome->SetName2("The Sky");
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

	// create the sun
	VTLOG("  creating Main Light\n");
	vtLight *pLight = new vtLight();
	pLight->SetName2("Main Light");
	m_pSunLight = new vtMovLight(pLight);

	// default location: over our right shoulder, pointing downward
	m_pSunLight->Translate1(FPoint3(1000.0f, 1000.0f, 1000.0f));
	m_pSunLight->RotateLocal(FPoint3(0,1,0), PIf/4.0f);
	m_pSunLight->RotateLocal(FPoint3(1,0,0), -PIf/4.0f);
	m_pSunLight->SetName2("SunLight");

	m_pTop->AddChild(m_pSunLight);

	// create sky group - this holds all celestial objects
	VTLOG("  creating Sky\n");
	m_pAtmosphereGroup = new vtGroup();
	m_pAtmosphereGroup->SetName2("Atmosphere Group");
	m_pTop->AddChild(m_pAtmosphereGroup);

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
 * Set the current Terrain for the scene.  There can only be one terrain
 * active a at time.  If you have more than one terrain, you can use this
 * method to switch between them.
 */
void vtTerrainScene::SetTerrain(vtTerrain *pTerrain)
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
		if (m_pSkyDome != NULL)
			m_pSkyDome->SetEnabled(false);
		return;
	}

	// switch
	m_pTop->AddChild(m_pCurrentTerrain->GetTopGroup());
	m_pCurrentTerrain->Enable(true);

	TParams &param = m_pCurrentTerrain->GetParams();

	// switch to the projection of this terrain
	m_pCurrentTerrain->SetGlobalProjection();

	// if the new terrain wants a skydome, and we haven't created one yet,
	// do so
	bool bDoSky = param.GetValueBool(STR_SKY);
	if (bDoSky && !m_pSkyDome)
		_CreateSkydome(vtTerrain::s_DataPaths);

	// move the sky to fit the new current terrain
	if (m_pSkyDome)
	{
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
		m_pSkyDome->SetRadius(radius);
	}

	// Set background color to match the ocean
	vtGetScene()->SetBgColor(m_pCurrentTerrain->GetOceanColor());

	// Turn on the engines specific to the new terrain
	m_pCurrentTerrain->ActivateEngines(true);

	// Setup the time engine for the new terrain
	vtTime localtime;
	localtime.SetTimeOfDay(param.GetValueInt(STR_INITTIME), 0, 0);

//	pTerrain->TranslateToGMT(time);

	m_pTime->SetTime(localtime);
	m_pTime->SetSpeed(param.GetValueFloat(STR_TIMESPEED));
	m_pTime->SetEnabled(param.GetValueBool(STR_TIMEON));

	// set the time to the time of the new terrain
	if (m_pSkyDome)
	{
		m_pSkyDome->SetSunLight(GetSunLight());
		m_pSkyDome->SetTimeOfDay(localtime.GetSecondOfDay(), true);
	}

	// handle the atmosphere
	if (m_pSkyDome)
	{
		m_pSkyDome->SetEnabled(bDoSky);
		if (bDoSky)
		{
			vtString fname = param.GetValueString(STR_SKYTEXTURE);
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
	m_pCurrentTerrain->SetFog(param.GetValueBool(STR_FOG));
}

void vtTerrainScene::SetTime(const vtTime &time)
{
	if (m_pSkyDome)
	{
		// TODO? Convert to local time?
		m_pSkyDome->SetTimeOfDay(time.GetSecondOfDay());
//		m_pSkyDome->ApplyDayColors();
// TODO? Update the fog color to match the color of the horizon.
	}
}

