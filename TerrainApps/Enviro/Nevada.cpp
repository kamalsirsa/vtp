//
// Nevada.cpp
//
// Terrain implementation specific to Black Rock City, Nevada.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Light.h"	// for TERRAIN_AMBIENT etc.
#include "vtlib/core/SkyDome.h"
#include "vtdata/DataPath.h"

#include "Nevada.h"
#include "Engines.h"
#include "wx/EnviroGUI.h"	// for g_App

// measured with GPS in 1999:
// man location
//  latitude 40 deg 41.75686'
//  longitude -119 deg 18.00042'
#define MAN_LON -119.3f
#define MAN_LAT 40.69595f

#define MAN_LONLAT MAN_LON, MAN_LAT

#define DETAIL_TILING	1200.0f
#define DETAIL_DISTANCE 1000.0f		// 1 km

#define ENABLE_PLANTS 0

#define HORIZON_FOG		RGBi(200, 227, 255)


///////////////////////////////////////

NevadaTerrain::NevadaTerrain()
{
	m_pPast = m_pPresent = m_pFuture = NULL;
	m_pWaterShape =		m_pWaterShape2 = NULL;
	m_pDetailMat =		m_pDetailMat2 = NULL;
	m_pMats = NULL;
}


//
// Nevada culture
//
void NevadaTerrain::CreateCustomCulture()
{
	// We use a different horizon color
	vtSkyDome *sky = vtGetTS()->GetSkyDome();
	sky->SetDayColors(HORIZON_FOG, RGBi(30, 70, 255));

	m_pHeightField->ConvertEarthToSurfacePoint(DPoint2(MAN_LONLAT), man_location);

	m_fGround = 1200 * m_Params.GetValueFloat(STR_VERTICALEXAG);
	m_fHigh = m_fGround + (50);
	m_fLow = m_fGround - (50);

	m_pMats = new vtMaterialArray;

	if (m_Params.GetValueBool(STR_DETAILTEXTURE))
		CreateDetailTextures();

	CreatePast();
	CreatePresent();
	CreateFuture();
	CreateWater();

	// create Epoch engine
	EpochEngine *pEE = new EpochEngine(this, m_fLow, m_fHigh,
		m_pDetailMat2, m_pDetailMat);
	pEE->setName("Epoch Engine");
	pEE->AddTarget(vtGetScene()->GetCamera());
	AddEngine(pEE);
//	addNode(pEE->m_pSprite);

#if 0
	// Buildings
	NodePtr dome = vtLoadModel("Nevada/bluedometent.dsm");
	if (dome.valid())
	{
		m_Future.addChild(dome);
		PlantModelAtPoint(dome, DPoint2(MAN_LONLAT), true);
	}
#endif
}


////////////////////////////////////////////

void NevadaTerrain::CreateWater()
{
	FRECT world_extents = m_pHeightField->m_WorldExtents;
	FPoint2 org, size;
	org.x = (float) world_extents.left;
	org.y = (float) world_extents.bottom;
	size.x = (float) world_extents.Width();
	size.y = (float) world_extents.Height();

	// create water material: texture waves
	vtString str = FindFileOnPaths(vtGetDataPath(), "GeoTypical/ocean1_256.jpg");

	int id;

	id = m_pMats->AddTextureMaterial(str,
		false, true,	// cull, light
		false, false,	// transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);

	vtGeode *geode;

	// create water plane
	geode = CreatePlaneGeom(m_pMats, id, 0, 2, 1, org, org+size, 125.0f, 10);
	m_pWaterShape = new vtMovGeode(geode);
	m_pWaterShape->setName("WaterSurface");
	addNode(m_pWaterShape);

	id = m_pMats->AddTextureMaterial(str,
		false, true,	// cull, light
		true, false,	// transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 0.6f, TERRAIN_EMISSIVE);

	// and another plane
	geode = CreatePlaneGeom(m_pMats, id,  0, 2, 1, org, org+size, 260.3f, 10);
	m_pWaterShape2 = new vtMovGeode(geode);
	m_pWaterShape2->setName("WaterSurface2");
	m_pWaterShape2->Translate(FPoint3(0.0f, .01f, 0.0f));
	addNode(m_pWaterShape2);
}


////////////////////////////////////////////

void NevadaTerrain::CreateDetailTextures()
{
#if 0
	vtString str;

	str = FindFileOnPaths(vtGetDataPath(), "Nevada/playa3.png");
	if (str == "")
		return;
	m_pDetailTexture = osgDB::readImageFile((const char *)str);
	str = FindFileOnPaths(vtGetDataPath(), "Nevada/green3.png");
	if (str == "")
		return;
	m_pDetailTexture2 = osgDB::readImageFile((const char *)str);

	int id;
	id = m_pMats->AddTextureMaterial(m_pDetailTexture,
						 true,	// culling
						 false,	// lighting
						 true,	// transp: blend
						 false,	// add
						 0.3f, 0.6f,	// ambient, diffuse
						 1.0f, 0.1f);
	m_pDetailMat = m_pMats->at(id);
	m_pDetailMat->SetMipMap(true);

	//initally, make partly transparent
	id = m_pMats->AddTextureMaterial(m_pDetailTexture2,
					 true,	// culling
					 false,	// lighting
					 true,	// transp: blend
					 false, // add
					 0.3f, 0.6f,	// ambient, diffuse
					 1.0f, 0.1f);	// alpha, emmisive
	m_pDetailMat2 = m_pMats->at(id);
	m_pDynGeom->SetDetailMaterial(m_pDetailMat,
		DETAIL_TILING, DETAIL_DISTANCE);
#endif
}


//////////////////////////////////////////////

void NevadaTerrain::CreatePast()
{
	m_pPast = new vtGroup;
	addNode(m_pPast);
	m_pPast->setName("Past");
	m_pPast->SetEnabled(false);

	FPoint3 center;
	GetLocalConversion().convert_earth_to_local_xz(MAN_LON, MAN_LAT, center.x, center.z);

#if ENABLE_PLANTS	// enable/disable plants
	//tree generation
#define TREE_EXAG	6.0f
	vtPlantAppearance3d *pPlantApp = new vtPlantAppearance3d(AT_BILLBOARD,
		"SEQ.png",
		6.0f, 8.5f,	// width, height
		0.0f, 0.0f);
	pPlantApp->LoadAndCreate(vtGetDataPath(), TREE_EXAG, false, true);	// shadows, billboards

	vtTransform *tree;
	FPoint3 p3;

	// setup Tree LOD Grid
	float fLODDistance = 14000.0f;

	FRECT world_extents = m_pHeightField->m_WorldExtents;
	FPoint3 origin, size;
	origin.x = (float) world_extents.left;
	origin.y = 0.0f;
	origin.z = (float) world_extents.bottom;
	size.x = (float) world_extents.Width();
	size.y = 0.0f;
	size.z = (float) -world_extents.Height();

	// use a 12x12 grid of LOD cells
	m_pTreeGrid = new vtLodGrid(origin, size, 12, fLODDistance);
	m_pTreeGrid->setName("Tree Grid");
	m_pPast->addChild(m_pTreeGrid);

	//populate with trees.  set initial size to zero?
	int tcount = 0, ccount = 0;
	float x, z;
	for (x = world_extents.left; x < world_extents.right; x += 150.f)
	{
		for (z = world_extents.top; z < world_extents.bottom; z += 150.f)
		{
			ccount++;
			p3.x = x + random_offset(70.f);
			p3.z = z + random_offset(70.f);
			if (!m_pHeightField->FindAltitudeAtPoint(p3, p3.y))
				continue;
			if (p3.y < (m_fHigh + 75))
				continue;
			FPoint3 dist = p3 - man_location;
			float len = dist.Length();
			if (len > 8000)
				continue;
			if (len > 5000)
			{
				if (ccount&1)
					continue;
			}
			if (random(1.0f) > 0.7f)	// drop some
				continue;

			tree = pPlantApp->GenerateGeom();
			float fScale = 1.0f + random_offset(0.5f);
			tree->Scale(fScale);
			tree->SetTrans(p3);
			// add tree to scene graph
			m_pTreeGrid->AppendToGrid(tree);
			//m_pPast->addChild(tree);
			tcount++;
		}
	}
#endif

#if 0
	// big mike
#define MIKE_COUNT 5
	for (int i = 0; i < MIKE_COUNT; i++)
	{
//		vtTransform *bigmike = LoadModel("Nevada/bigmikev2.dsm");
		vtTransform *bigmike = LoadModel("Nevada/parameciummike.dsm");
		float sc = 0.05f;	// abstract units, scale to taste
		m_pPast->addChild(bigmike);
		PlantModelAtPoint(bigmike, DPoint2(MAN_LONLAT), true);
		bigmike->Translate2(FPoint3(0.0f, i * 800.0f, 0.0f));

		JumpingEngine *pJumper = new JumpingEngine(bigmike->GetTrans(),
			sc, m_fGround, 100.f, i * PIf * 2.0f / MIKE_COUNT);
		pJumper->AddTarget(bigmike);
		AddEngine(pJumper);
	}
#endif
}


////////////////////////////////////////////

void NevadaTerrain::CreatePresent()
{
	m_pPresent = new vtGroup;

	addNode(m_pPresent);
	m_pPresent->setName("Present");
	m_pPresent->SetEnabled(false);

#if 0
	float overall_scale = 1.5f;	// 3x larger than real life
	float sc = overall_scale;	// meters
#if 0
	vtTransform *man = LoadModel("Nevada/earlyman.dsm");
#else
	vtTransform *man = LoadModel("Nevada/man_v7_e1.dsm");
#endif
	man->Scale2(sc, sc, sc);
	m_pPresent->addChild(man);
	PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);

	sc = overall_scale * 0.01f;		// cm
	vtTransform *lamppost = LoadModel("Nevada/lamppost_v2.dsm");
	lamppost->Scale2(sc, sc, sc);

	vtTransform *lamppost2 = LoadModel("Nevada/lamppost72lod_v2.dsm");
	lamppost2->Scale2(sc, sc, sc);

	vtLOD *pLampLod = new vtLOD;
	pLampLod->addChild(lamppost);
	pLampLod->addChild(lamppost2);
	pLampLod->setRanges(0, 0.0f, 40.0f);
	pLampLod->setRanges(1, 40.0f, 300.0f);

	vtTransform *copy;
	if (lamppost)
	{
		for (int i = 0; i < 17; i++)
		{
			copy = (vtTransform *)pLampLod->Clone();
			PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);
			m_pPresent->addChild(copy);

			float radius = 638;
			float deg = 30.0f - (i * 15.0f);
			float angle = deg * PIf / 180.0f;
			float x = cosf(angle) * radius;
			float y = sinf(angle) * radius;
			copy->Translate2(FPoint3(x, 0.0f, -y));
			PlantModel(copy);
		}
		for (int i = 0; i < 17; i++)
		{
			copy = (vtTransform *)pLampLod->Clone();
			PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);
			m_pPresent->addChild(copy);

			float radius = 78.0f + (i * (600.0f - 78.0f) / 17);
			float deg = -90;
			float angle = deg * PIf / 180.0f;
			float x = cosf(angle) * radius;
			float y = sinf(angle) * radius;
			copy->Translate2(FPoint3(x, 0.0f, -y));
			copy->Translate2(FPoint3(8.0f, 0.0f, 0.0f));
			PlantModel(copy);

			copy = (vtTransform *)pLampLod->Clone();
			PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);
			m_pPresent->AddChild(copy);
			copy->Translate2(FPoint3(x, 0.0f, -y));
			copy->Translate2(FPoint3(-8.0f, 0.0f, 0.0f));
			PlantModel(copy);
		}
	}
#endif
}


////////////////////////////////////////////

void NevadaTerrain::CreateFuture()
{
	m_pFuture = new vtGroup;

	addNode(m_pFuture);
	m_pFuture->setName("Future");
	m_pFuture->SetEnabled(false);
}

////////////////////////////////////////////

void NevadaTerrain::SetWaterHeight(float fWaterHeight)
{
	m_pWaterShape->SetTrans(FPoint3(0.0f, fWaterHeight, 0.0f));
}

void NevadaTerrain::SetWaterOn(bool on)
{
	m_pWaterShape->SetEnabled(on);
}

////////////////////////////////////////////////////////

#define UPWARD_VELOCITY (6)

JumpingEngine::JumpingEngine(FPoint3 center, float fScale, float fBase,
							 float fJumpHeight, float fPhase)
{
	m_center = center;
	m_fBase = fBase;
	m_fJumpHeight = fJumpHeight;
	m_vel.y = UPWARD_VELOCITY;
	m_fPhase = fPhase;
	m_fScale = fScale;
}

bool JumpingEngine::Eval(float t)
{
	vtTransform *pTarget = (vtTransform *)GetTarget();
	if (!pTarget) return false;

	FPoint3 pos = pTarget->GetTrans();

	// bounce up and down
	if (pos.y < m_fBase)
	{
		pos.y = m_fBase;
//		if (m_vel.y < 0.0f) m_vel.y = -m_vel.y;
		if (m_vel.y < 0.0f) m_vel.y = UPWARD_VELOCITY;
	}
	else
	{
		// gravity
		m_vel.y -= (0.15f);
		pos += m_vel;
	}

	// move in a circle
	pos.x = m_center.x + sinf(t/50.0f + m_fPhase) * 300.0f;
	pos.z = m_center.z + cosf(t/50.0f + m_fPhase) * 300.0f;

	pTarget->Identity();
	pTarget->Scale(m_fScale);
	pTarget->SetTrans(pos);

	// wiggle it.. just a little bit
	float wiggle1 = 1.0f;
	if (m_vel.y > 0.0f)
		wiggle1 += sinf(t*10.0f) * 70.0f * m_vel.y;
	float wiggle2 = 1.0f;
	if (m_vel.y > 0.0f)
		wiggle2 += sinf(t*10.0f + PIf) * 50.0f * m_vel.y;
//	VTLOG("wiggle %f\n", wiggle1);
	pTarget->Scale(1.0f, wiggle1, wiggle2);

	return true;
}


//////////////////////////////////////////////////////////////////

#define PAST		-14000
#define PRESENT		1999
#define FUTURE		2150
#define PAST_PRESENT_SWITCH -7000

EpochEngine::EpochEngine(NevadaTerrain *pNevada, float fLow, float fHigh,
		vtMaterial* pastApp, vtMaterial* presentApp)
{
	m_iShownYear = m_iYear = m_iTargetYear = PRESENT;
	m_iSpeed = 0;

	m_pNevada = pNevada;
	m_fLow = fLow;
	m_fHigh = fHigh;
	m_fWaterHeight = fLow;

	// Make a sprite to show the text
	m_pSpriteText = new vtTextMesh(g_App.GetArial(), 32);
	m_pSpriteText->SetText("1999 AD");
	m_pSpriteText->SetPosition(FPoint3(280,3,0));
	m_pSpriteText->SetColor(RGBAf(0,0.2f,0));

	m_pSprite = new vtGeode;
	m_pSprite->setName("Year Sprite");
	m_pSprite->AddTextMesh(m_pSpriteText, 0);

	vtGetScene()->GetHUD()->GetContainer()->addChild(m_pSprite);
	pNevada->addNode(m_pSprite);

	m_pPastMat = pastApp;
	m_pPresentMat = presentApp;
}

void EpochEngine::Eval()
{
	// deal with the viewpoint interactions with the water:
	vtCamera *pCam = (vtCamera *)GetTarget();
	FPoint3 campos = pCam->GetTrans();

	static bool bAboveWater = false;
	// turn on murkiness if we're under water?
	if (campos.y < m_fWaterHeight)
	{
		if (bAboveWater)
		{
			RGBf color(70.0f/255, 70.0f/255, 145.0f/255);
			m_pNevada->SetFogDistance(500);
			m_pNevada->SetFogColor(color);
			vtGetScene()->SetBgColor(color);
		}
		bAboveWater = false;
	}
	else
	{
		if (!bAboveWater)
		{
			m_pNevada->SetFogDistance(25000);
			m_pNevada->SetFogColor(HORIZON_FOG);
			vtGetScene()->SetBgColor(HORIZON_FOG);
		}
		bAboveWater = true;
	}

#if 0
	// keep head above water
	float minimum = m_fWaterHeight + 15.0f;
	if (campos.y < minimum)
	{
		campos.y = minimum;
		pCam->SetTrans(campos);
	}
#endif

	static int pause = 0;
	if (pause)
	{
		pause--;
		return;
	}

	bool set_water = (m_iSpeed != 0);
	m_iYear += m_iSpeed;
	if (m_iSpeed > 0 && m_iYear > m_iTargetYear)	// got there?
	{
		m_iYear = m_iTargetYear;	// stop
		m_iSpeed = 0;
	}
	if (m_iSpeed < 0 && m_iYear < m_iTargetYear)	// got there?
	{
		m_iYear = m_iTargetYear;	// stop
		m_iSpeed = 0;
	}
	if (set_water)
	{
		float amount = (float)(m_iYear - PAST) / (PRESENT - PAST);
		m_fWaterHeight = m_fHigh + (m_fLow - m_fHigh) * amount;
		m_pNevada->SetWaterHeight(m_fWaterHeight);
		if (m_iYear < PRESENT)
			m_pNevada->SetWaterOn(true);
		else
			m_pNevada->SetWaterOn(false);

		if (m_pPastMat && m_pPresentMat)
		{
			// try to fade green onto land
			float alpha;
			vtMaterial *pMaterial;
			RGBAf diffuse;
			if (m_iYear < PAST_PRESENT_SWITCH)
			{
				// green
				alpha = 0.7f * (1.0f - ((float)(PAST - m_iYear))/(float)(PAST - PAST_PRESENT_SWITCH));
				pMaterial = m_pPastMat;
				diffuse = pMaterial->GetDiffuse();
				diffuse.a = alpha;
				pMaterial->SetDiffuse(diffuse);
				//m_pNevada->GetDynTerrain()->SetDetailMaterial(m_pPastMat,
				//	DETAIL_TILING, DETAIL_DISTANCE);
			}
			else
			{
				// dry
				alpha = 1 - ((float)(PRESENT - m_iYear))/(float)(PRESENT - PAST_PRESENT_SWITCH);
				pMaterial = m_pPresentMat;
				diffuse = pMaterial->GetDiffuse();
				diffuse.a = alpha;
				pMaterial->SetDiffuse(diffuse);
				//m_pNevada->GetDynTerrain()->SetDetailMaterial(m_pPresentMat,
				//	DETAIL_TILING, DETAIL_DISTANCE);
			}
		}
	}
	m_pNevada->m_pPast->SetEnabled(m_iYear == PAST);
	m_pNevada->m_pPresent->SetEnabled(m_iYear == PRESENT);
	m_pNevada->m_pFuture->SetEnabled(m_iYear == FUTURE);

	// bounce test
	if (m_iSpeed == 0)
	{
		if (m_iYear == PRESENT)
		{
			m_iTargetYear = PAST;
			m_iSpeed = -48;
			pause = 460;
		}
		if (m_iYear == PAST)
		{
			m_iTargetYear = PRESENT;
			m_iSpeed = 48;
			pause = 40;
		}
	}

	// update the displayed year
	if (m_iShownYear != m_iYear)
	{
		m_iShownYear = m_iYear;
		vtString str;
		if (m_iShownYear > 0)
			str.Format("%d AD", m_iShownYear);
		else
			str.Format("%d BC", -m_iShownYear);
		m_pSpriteText->SetText(str);
	}
}



