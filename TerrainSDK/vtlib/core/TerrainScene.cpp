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
//
// A small engine that allows the SkyDome to stay around the Camera.
//
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
	vtTransform *pTarget = (vtTransform *) GetTarget();
	if (!pTarget || !m_pCamera)
		return;
	FPoint3 pos1 = m_pCamera->GetTrans();
	FPoint3 pos2 = pTarget->GetTrans();
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

	m_yondist = 1000.0f * WORLD_SCALE;

	m_pSkyDome = NULL;
	m_pFirstTerrain = NULL;
	m_pCurrentTerrain = NULL;
	m_pTime = NULL;
	m_pSunLight = NULL;
}

vtTerrainScene::~vtTerrainScene()
{
	// delete all the terrains that were appended
	// due to magic of reference counting, this is sufficient:
//	m_pFirstTerrain->Delete();	// TODO: proper cleanup
}

void vtTerrainScene::create_skydome(vtString datapath)
{
	if (m_pSkyDome != NULL)
		return;

	// create a day-night dome
	m_pSkyDome = new vtSkyDome();
	m_pSkyDome->SetDayColors(horizon_color, azimuth_color);
	m_pSkyDome->SetDawnTimes(5, 0, 0, 7, 0, 0);
	m_pSkyDome->SetDuskTimes(17, 0, 0, 19, 0, 0);
	m_pSkyDome->Create(datapath + "Sky/bsc.data", 3,
					   1.0f,		// initially unit radius
					   datapath + "Sky/glow2.png",
					   datapath + "Sky/moon5_256.png");
	m_pSkyDome->SetName2("The Sky");
	m_pAtmosphereGroup->AddChild(m_pSkyDome);

	vtSkyTrackEngine *pEng = new vtSkyTrackEngine();
	pEng->SetName2("Sky-Camera-Following");
	pEng->m_pCamera = vtGetScene()->GetCamera();
	pEng->SetTarget(m_pSkyDome);
	vtGetScene()->AddEngine(pEng);
}

//
// Find a terrain whose name begins with a given string
//
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

void vtTerrainScene::create_fog()
{
#if 0
	int dist = m_pFirstTerrain->m_Params.m_iFogDistance;

	m_pFog = new vtFog();
	m_pFog->SetStart(1.0f);  // only useful for linear - broken
	m_pFog->SetEnd(dist * 1000.0f * WORLD_SCALE);

	// 1 / ( dist / slope + max )
	// dist = visability distance
	// slope = how fast density varies by dist, smaller = see farther (guessing)
	// max = max density of fog = (2 - max), so 1.2 = max density of 0.8

	m_pFog->SetDensity( 1.0f / (dist/4.0f + 1.3f) ); // vis dis function of density

	m_pFog->SetColor(horizon_color);
	//m_pFog->SetKind(FOG_Exponential2 | FOG_Pixel); // from linear
	m_pFog->SetKind(FOG_Linear); // from linear
#endif
}


void vtTerrainScene::create_engines(bool bDoSound)
{
	// Set Time in motion
	m_pTime = new TimeEngine(this, 0);
	m_pTime->SetName2("Time");
	vtGetScene()->AddEngine(m_pTime);
}


vtRoot *vtTerrainScene::BeginTerrainScene(bool bDoSound)
{
	create_engines(bDoSound);

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

void vtTerrainScene::AppendTerrain(vtTerrain *pTerrain)
{
	// add to linked list
	pTerrain->SetNext(m_pFirstTerrain);
	m_pFirstTerrain = pTerrain;
}

//
// call after you have appended all terrains
//
void vtTerrainScene::Finish(const char *datapath)
{
	create_skydome(datapath);

	// create fog AFTER the shapes
//	if (m_pFirstTerrain != NULL)
//		create_fog();

	// start out with no scene active
	for (vtTerrain *t = m_pFirstTerrain; t != NULL; t=t->GetNext())
		t->ActivateEngines(false);
}


void vtTerrainScene::SetTerrain(vtTerrain *pTerrain)
{
	if (m_pCurrentTerrain != NULL)
	{
		// turn off the scene graph of the previous terrain
		m_pCurrentTerrain->m_pTerrainGroup->SetEnabled(false);

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
	m_pTop->AddChild(m_pCurrentTerrain->m_pTerrainGroup);
	m_pCurrentTerrain->m_pTerrainGroup->SetEnabled(true);

	// switch to the projection of this terrain
	m_pCurrentTerrain->SetGlobalProjection();

	// move the sky to fit the new current terrain
	m_pSkyDome->SetEnabled(true);
	float radius = pTerrain->GetRadius() * 2.9f;
	float max_radius = (450000 * WORLD_SCALE);
	if (radius > max_radius)
		radius = max_radius;
	m_pSkyDome->SetRadius(radius);
	FPoint3 center = pTerrain->GetCenter();
	center.y = 0.0f;
	m_pSkyDome->SetTrans(center);

	// Set background color to match the ocean
	vtGetScene()->SetBgColor(m_pCurrentTerrain->GetOceanColor());

	// turn on the engines specific to the new terrain
	m_pCurrentTerrain->ActivateEngines(true);

	// set the time to the time of the new terrain
	TParams &param = m_pCurrentTerrain->GetParams();
	int time = param.m_iInitTime * 3600;
	SetTimeOfDay(time, true);

	// setup time engine
	m_pTime->SetTime(time);
	m_pTime->SetRealIncrement(param.m_fTimeSpeed);
	m_pTime->SetEnabled(param.m_bTimeOn);

	// handle the atmosphere
	m_pSkyDome->SetEnabled(param.m_bSky);
	SetFog(param.m_bFog);
}

void vtTerrainScene::ToggleFog()
{
	SetFog(!m_bFog);
}

void vtTerrainScene::SetFog(bool fog)
{
#if 0
	m_bFog = fog;
	if (m_bFog)
	{
		vtGetScene()->SetFog(m_pFog);
		vtGetScene()->SetBackColor(horizon_color);
//		vtGetScene()->GetCamera()->GetYon(m_pFirstTerrain
		vtGetScene()->GetCamera()->SetYon(m_pFirstTerrain->m_Params.m_iFogDistance	* 1000.0f * WORLD_SCALE);
	}
	else
	{
		vtGetScene()->SetFog(NULL);
		vtScene::SetBgColor(m_pCurrentTerrain->GetOceanColor());
		vtGetScene()->GetCamera()->SetYon(m_yondist);
	}
#endif
}

void vtTerrainScene::SetTimeOfDay(unsigned int time, bool fFullRefresh)
{
	if (m_pSkyDome)
	{
		m_pSkyDome->SetSunLight(GetSunLight());
		m_pSkyDome->SetTimeOfDay(time, fFullRefresh);
//		m_pSkyDome->ApplyDayColors();
	}

#if SET_FOG_EACH_FRAME
	float brightness = m_pSkyDome->GetDayDome()->GetSkyBrightness();
	if (m_pFog != NULL)
	{
		m_pFog->SetColor(horizon_color * brightness);
	}
#endif
}

