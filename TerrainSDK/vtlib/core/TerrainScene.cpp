//
// vtTerrainScene - Container class for all of the terrains loaded
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
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

	m_pSkyDome = NULL;
	m_pFirstTerrain = NULL;
	m_pCurrentTerrain = NULL;
	m_pTime = NULL;
	m_pSkyTrack = NULL;
	m_pSunLight = NULL;
}

vtTerrainScene::~vtTerrainScene()
{
	vtGetScene()->RemoveEngine(m_pTime);
	vtGetScene()->RemoveEngine(m_pSkyTrack);

	delete m_pTime;
	delete m_pSkyTrack;

	while (m_pFirstTerrain)
	{
		vtTerrain *del = m_pFirstTerrain;
		m_pFirstTerrain = m_pFirstTerrain->GetNext();
		delete del;
	}
	m_pFirstTerrain = NULL;
	m_pCurrentTerrain = NULL;
}

void vtTerrainScene::_CreateSkydome(const StringArray &datapath)
{
	if (m_pSkyDome != NULL)
		return;

	vtString bsc = FindFileOnPaths(datapath, "Sky/bsc.data");
	vtString sun = FindFileOnPaths(datapath, "Sky/glow2.png");
	vtString moon = FindFileOnPaths(datapath, "Sky/moon5_256.png");

	// create a day-night dome
	m_pSkyDome = new vtSkyDome();
	m_pSkyDome->SetDayColors(horizon_color, azimuth_color);
	m_pSkyDome->SetDawnTimes(5, 0, 7, 0);
	m_pSkyDome->SetDuskTimes(17, 0, 19, 0);

	m_pSkyDome->Create(bsc, 3, 1.0f,	// initially unit radius
					   sun, moon);
	m_pSkyDome->SetName2("The Sky");
	m_pAtmosphereGroup->AddChild(m_pSkyDome);

	m_pSkyTrack = new vtSkyTrackEngine();
	m_pSkyTrack->SetName2("Sky-Camera-Following");
	m_pSkyTrack->m_pCamera = vtGetScene()->GetCamera();
	m_pSkyTrack->SetTarget(m_pSkyDome);
	vtGetScene()->AddEngine(m_pSkyTrack);
}

/**
 * Find a terrain whose name begins with a given string.
 */
vtTerrain *vtTerrainScene::FindTerrainByName(const char *name)
{
	int len = strlen(name);
	if (!len)
		return NULL;

	for (vtTerrain *pTerr = m_pFirstTerrain; pTerr; pTerr=pTerr->GetNext())
	{
		if (!strncmp(name, pTerr->GetName(), len))
			return pTerr;
	}
	return NULL;
}

void vtTerrainScene::_CreateEngines(bool bDoSound)
{
	// Set Time in motion
	m_pTime = new TimeEngine(this, 0);
	m_pTime->SetName2("Time");
	m_pTime->SetEnabled(false);
	vtGetScene()->AddEngine(m_pTime);
}


/**
 * Call this method once before adding any terrains, to initialize
 * the vtTerrainScene object.
 */
vtRoot *vtTerrainScene::BeginTerrainScene(bool bDoSound)
{
	_CreateEngines(bDoSound);

	m_pTop = new vtRoot();
	m_pTop->SetName2("All Terrain");

	// create the sun
	vtLight *pLight = new vtLight();
	m_pSunLight = new vtMovLight(pLight);

	// default location: over our right shoulder, pointing downward
	m_pSunLight->Translate1(FPoint3(1000.0f, 1000.0f, 1000.0f));
	m_pSunLight->RotateLocal(FPoint3(0,1,0), PIf/4.0f);
	m_pSunLight->RotateLocal(FPoint3(1,0,0), -PIf/4.0f);
	m_pSunLight->SetName2("SunLight");

	m_pTop->AddChild(m_pSunLight);

	// create sky group - this holds all celestial objects
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
 * Call once after you have appended all of your terrains.
 */
void vtTerrainScene::Finish(const StringArray &datapath)
{
	_CreateSkydome(datapath);

	// start out with no scene active
	for (vtTerrain *t = m_pFirstTerrain; t != NULL; t=t->GetNext())
		t->ActivateEngines(false);
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

	// move the sky to fit the new current terrain
	m_pSkyDome->SetEnabled(true);
	FRECT world_ext = pTerrain->GetHeightField()->m_Conversion.m_WorldExtents;
	float radius = world_ext.Width() * 5.0;
	float max_radius = 450000;
	if (radius > max_radius)
		radius = max_radius;
	m_pSkyDome->SetRadius(radius);

	// Set background color to match the ocean
	vtGetScene()->SetBgColor(m_pCurrentTerrain->GetOceanColor());

	// turn on the engines specific to the new terrain
	m_pCurrentTerrain->ActivateEngines(true);

	// set the time to the time of the new terrain
	int time = param.m_iInitTime * 3600;
	SetTimeOfDay(time, true);

	// setup time engine
	m_pTime->SetTime(time);
	m_pTime->SetRealIncrement(param.m_fTimeSpeed);
	m_pTime->SetEnabled(param.m_bTimeOn);

	// handle the atmosphere
	m_pSkyDome->SetEnabled(param.m_bSky);
	if (param.m_strSkyTexture != "")
	{
		vtString filename = "Sky/";
		filename += param.m_strSkyTexture;
		vtString skytex = FindFileOnPaths(vtTerrain::m_DataPaths, filename);
		if (skytex != "")
			m_pSkyDome->SetTexture(skytex);
	}
	SetFog(param.m_bFog);
}

void vtTerrainScene::ToggleFog()
{
	SetFog(!m_bFog);
}

void vtTerrainScene::SetFog(bool fog)
{
	m_bFog = fog;
	if (m_bFog)
	{
		TParams &param = m_pCurrentTerrain->GetParams();
		m_pCurrentTerrain->GetTopGroup()->SetFog(true, 0, param.m_iFogDistance * 1000);
		vtGetScene()->SetBgColor(horizon_color);
	}
	else
	{
		m_pCurrentTerrain->GetTopGroup()->SetFog(false);
		vtGetScene()->SetBgColor(m_pCurrentTerrain->GetOceanColor());
	}
}

void vtTerrainScene::SetTimeOfDay(unsigned int time, bool fFullRefresh)
{
	if (m_pSkyDome)
	{
		m_pSkyDome->SetSunLight(GetSunLight());
		m_pSkyDome->SetTimeOfDay(time, fFullRefresh);
//		m_pSkyDome->ApplyDayColors();
	}
	// TODO? Update the fog color to match the color of the horizon.
}

