//
// Nevada.cpp
//
// Terrain implementation specific to Black Rock City, Nevada.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Light.h"
#include "vtlib/core/TerrainPatch.h"

#include "Nevada.h"
#include "Engines.h"
#include "Wings.h"
#include "Hawaii.h"
#include "SwapEngine.h"
#include "TerrainSceneWP.h"

vtMaterialArray *Butterfly::m_pApps;

// measured with GPS in 1999:
// man location
//  latitude 40 deg 41.75686'
//  longitude -119 deg 18.00042'
#define MAN_LON -119.3f
#define MAN_LAT 40.69595f

#define MAN_LONLAT MAN_LON, MAN_LAT

#define DETAIL_TILING	1200.0f

#define ENABLE_TREES 1


///////////////////////////////////////

NevadaTerrain::NevadaTerrain() : PTerrain()
{
}

NevadaTerrain::~NevadaTerrain()
{
}

//
// Nevada culture
//
void NevadaTerrain::CreateCustomCulture(bool bDoSound)
{
	m_pHeightField->ConvertEarthToSurfacePoint(MAN_LONLAT, man_location);

	m_fGround = 1200 * m_Params.m_fVerticalExag * WORLD_SCALE;
	m_fHigh = m_fGround + (50 * WORLD_SCALE);
	m_fLow = m_fGround - (50 * WORLD_SCALE);

	if (m_Params.m_bDetailTexture)
		CreateDetailTextures();

	CreatePast();
	CreatePresent();
	CreateFuture();
	CreateWater();

#if 0
	vtTransform *gateway1 = LoadModel("Culture/portal1.dsm");
	vtTransform *copy = (vtTransform *) gateway1->CreateClone();

	vtTerrain *pIsland = GetTerrainScene().FindTerrainByName("Big Island");

	if (pIsland && gateway1)
	{
		MakePortalLL(pIsland, gateway1, "Gateway to Hawaii",
			MAN_LONLAT + 0.002f, 1);
	}
#endif

	// create Epoch engine
	EpochEngine *pEE = new EpochEngine(this, m_fLow, m_fHigh,
		m_pDetailMat2, m_pDetailMat);
	pEE->SetName2("Epoch Engine");
	pEE->SetTarget(vtGetScene()->GetCamera());
	AddEngine(pEE);
//	AddNode(pEE->m_pSprite);

#if 0
	// Buildings
	vtTransform *dome = LoadModel("Nevada/bluedometent.dsm");
	if (dome)
	{
		dome->Scale2(WORLD_SCALE, WORLD_SCALE, WORLD_SCALE);
		m_Future.AddChild(dome);
		PlantModelAtPoint(dome, DPoint2(MAN_LONLAT), true);
	}
#endif
}


////////////////////////////////////////////

void NevadaTerrain::CreateWater()
{
	// create water material: texture waves
	vtMaterialArray *pMats = new vtMaterialArray();
	vtString str = m_strDataPath;
	str += "GeoTypical/ocean1_256.bmp";
	pMats->AddTextureMaterial2(str,
		false, true,	// cull, light
		false, false,	// transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);
	pMats->AddTextureMaterial2(str,
		false, true,	// cull, light
		true, false,	// transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 0.6f, TERRAIN_EMISSIVE);

	FRECT world_extents;
	m_pHeightField->GetWorldExtents(world_extents);
	FPoint2 org, size;
	org.x = (float) world_extents.left;
	org.y = (float) world_extents.bottom;
	size.x = (float) world_extents.Width();
	size.y = (float) world_extents.Height();

	// create water plane
	m_pWaterShape = CreatePlaneMGeom(pMats, 0, org, size, 125.0f, 125.0f, 10);	// appidx 0
	m_pWaterShape->SetName2("WaterSurface");
	AddNode(m_pWaterShape);

	// and another plane
	m_pWaterShape2 = CreatePlaneMGeom(pMats, 1, org, size, 260.3f, 260.3f, 10);	// appidx 1
	m_pWaterShape2->SetName2("WaterSurface2");
	m_pWaterShape2->Translate1(FPoint3(0.0f, .01f, 0.0f));
	AddNode(m_pWaterShape2);
}


////////////////////////////////////////////

void NevadaTerrain::CreateDetailTextures()
{
	vtImage *pDetailTexture = new vtImage("Data/Nevada/playa3.png");
	vtImage *pDetailTexture2 = new vtImage("Data/Nevada/green3.png");

	vtMaterialArray *pDetailApps = new vtMaterialArray();
	pDetailApps->AddTextureMaterial(pDetailTexture,
						 true,	// culling
						 false,	// lighting
						 true,	// transp: blend
						 false,	// add
						 0.3f, 0.6f,	// ambient, diffuse
						 1.0f, 0.1f,	// alpha, emmisive
						 true);			// texgen
	m_pDetailMat = pDetailApps->GetAt(0);

	//initally, make partly transparent
	pDetailApps->AddTextureMaterial(pDetailTexture2,
					 true,	// culling
					 false,	// lighting
					 true,	// transp: blend
					 false, // add
					 0.3f, 0.6f,	// ambient, diffuse
					 1.0f, 0.1f,	// alpha, emmisive
					 true);			// texgen
	m_pDetailMat2 = pDetailApps->GetAt(1);
	m_pDynGeom->SetDetailMaterial(m_pDetailMat, DETAIL_TILING);
}


//////////////////////////////////////////////

void NevadaTerrain::CreatePast()
{
	AddNode(&m_Past);
	m_Past.SetName2("Past");
	m_Past.SetEnabled(false);

//  float height = (float) m_Params.m_iMinHeight + 1.0f*WORLD_SCALE;
	float height = 80.0f * WORLD_SCALE;
	FPoint3 center;
	g_Proj.convert_latlon_to_local_xz(MAN_LAT, MAN_LON, center.x, center.z);

#if 0
	//butterfly: circle radius, speed, height above ground, center, size_exag
	vtGeom *bfly = new Butterfly(this, 0.2f, 50.0f, height, center, 200.0);
	m_Past.AddChild(bfly);
	vtGeom *bfly2 = new Butterfly(this, 0.3f, 50.0f, height, center, 200.0);
	m_Past.AddChild(bfly2);
	vtGeom *bfly3 = new Butterfly(this, 0.4f, 50.0f, height, center, 200.0);
	m_Past.AddChild(bfly3);
#endif

	if (1)
	{
		typedef vtGeom *shapeptr;
		int x, y;
		FPoint3 location;
		for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++)
		{
			int num = x*8+y;
			location.x = center.x - 8 + (x * 2.0f);
			location.z = center.z - 8 + (y * 2.0f);
			Butterfly *but = new Butterfly(this, 0.8f, 60 + random(40), height, location, 40);
			m_Past.AddChild(but);
		}
	}

#if ENABLE_TREES	// enable/disable trees
	//tree generation
#define TREE_EXAG	6.0f
	vtPlantAppearance3d *pPlantApp = new vtPlantAppearance3d(AT_BILLBOARD,
		"SEQ.png",
		6.0f, 8.5f,	// width, height
		0.0f, 0.0f);
	pPlantApp->LoadAndCreate("Data/", TREE_EXAG, false, true);	// shadows, billboards

	vtTransform *tree;
	FPoint3 p3;

	// setup Tree LOD Grid
	float fLODDistance = 14000.0f;

	FRECT world_extents;
	m_pHeightField->GetWorldExtents(world_extents);
	FPoint3 origin, size;
	origin.x = (float) world_extents.left;
	origin.y = 0.0f;
	origin.z = (float) world_extents.bottom;
	size.x = (float) world_extents.Width();
	size.y = 0.0f;
	size.z = (float) -world_extents.Height();

	// use a 12x12 grid of LOD cells
	m_pTreeGrid = new vtLodGrid(origin, size, 12, fLODDistance * WORLD_SCALE);
	m_pTreeGrid->SetName2("Tree Grid");
	m_Past.AddChild(m_pTreeGrid);

	//populate with trees.  set initial size to zero?
	int tcount = 0, ccount = 0;
	float x, z;
	for (x = world_extents.left; x < world_extents.right; x += (150.f * WORLD_SCALE))
	{
		for (z = world_extents.top; z < world_extents.bottom; z += (150.f * WORLD_SCALE))
		{
			ccount++;
			p3.x = x + random_offset(70.f * WORLD_SCALE);
			p3.z = z + random_offset(70.f * WORLD_SCALE);
			if (!m_pHeightField->FindAltitudeAtPoint(p3, p3.y))
				continue;
			if (p3.y < (m_fHigh + 75 * WORLD_SCALE))
				continue;
			FPoint3 dist = p3 - man_location;
			float len = dist.Length();
			if (len > 8000*WORLD_SCALE)
				continue;
			if (len > 5000*WORLD_SCALE)
			{
				if (ccount&1)
					continue;
			}
			if (random(1.0f) > 0.7f)	// drop some
				continue;

			tree = pPlantApp->GenerateGeom();
			float fScale = 1.0f + random_offset(0.5f);
			tree->Scale3(fScale, fScale, fScale);
			tree->SetTrans(p3);
			// add tree to scene graph
			m_pTreeGrid->AppendToGrid(tree);
			//m_Past.AddChild(tree);
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
		float sc = WORLD_SCALE * 0.05f;	// abstract units, scale to taste
		m_Past.AddChild(bigmike);
		PlantModelAtPoint(bigmike, DPoint2(MAN_LONLAT), true);
		bigmike->Translate2(FPoint3(0.0f, i * 800.0f * WORLD_SCALE, 0.0f));

		JumpingEngine *pJumper = new JumpingEngine(bigmike->GetTrans(),
			sc, m_fGround, 100.f, i * PIf * 2.0f / MIKE_COUNT);
		pJumper->SetTarget(bigmike);
		AddEngine(pJumper);
	}
#endif
}


////////////////////////////////////////////

void NevadaTerrain::CreatePresent()
{
	AddNode(&m_Present);
	m_Present.SetName2("Present");
	m_Present.SetEnabled(false);

#if 0
	float overall_scale = 1.5f;	// 3x larger than real life
	float sc = overall_scale * WORLD_SCALE;	// meters
#if 0
	vtTransform *man = LoadModel("Nevada/earlyman.dsm");
#else
	vtTransform *man = LoadModel("Nevada/man_v7_e1.dsm");
#endif
	man->Scale2(sc, sc, sc);
	m_Present.AddChild(man);
	PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);

	sc = overall_scale * 0.01f * WORLD_SCALE;		// cm
	vtTransform *lamppost = LoadModel("Nevada/lamppost_v2.dsm");
	lamppost->Scale2(sc, sc, sc);

	vtTransform *lamppost2 = LoadModel("Nevada/lamppost72lod_v2.dsm");
	lamppost2->Scale2(sc, sc, sc);

	vtLOD *pLampLod = new vtLOD();
	pLampLod->AddChild(lamppost);
	pLampLod->AddChild(lamppost2);
	float ranges[3];
	ranges[0] = 1.0f * WORLD_SCALE;
	ranges[1] = 40.0f * WORLD_SCALE;
	ranges[2] = 300.0f * WORLD_SCALE;
	pLampLod->SetRanges(ranges, 3);

	vtTransform *copy;
	int i;

	if (lamppost)
	{
		for (i = 0; i < 17; i++)
		{
			copy = (vtTransform *)pLampLod->CreateClone();
			PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);
			m_Present.AddChild(copy);

			float radius = 638;
			float deg = 30.0f - (i * 15.0f);
			float angle = deg * PIf / 180.0f;
			float x = cosf(angle) * radius * WORLD_SCALE;
			float y = sinf(angle) * radius * WORLD_SCALE;
			copy->Translate2(FPoint3(x, 0.0f, -y));
			PlantModel(copy);
		}
		for (i = 0; i < 17; i++)
		{
			copy = (vtTransform *)pLampLod->CreateClone();
			PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);
			m_Present.AddChild(copy);

			float radius = 78.0f + (i * (600.0f - 78.0f) / 17);
			float deg = -90;
			float angle = deg * PIf / 180.0f;
			float x = cosf(angle) * radius * WORLD_SCALE;
			float y = sinf(angle) * radius * WORLD_SCALE;
			copy->Translate2(FPoint3(x, 0.0f, -y));
			copy->Translate2(FPoint3(8.0f * WORLD_SCALE, 0.0f, 0.0f));
			PlantModel(copy);

			copy = (vtTransform *)pLampLod->CreateClone();
			PlantModelAtPoint(man, DPoint2(MAN_LONLAT), true);
			m_Present.AddChild(copy);
			copy->Translate2(FPoint3(x, 0.0f, -y));
			copy->Translate2(FPoint3(-8.0f * WORLD_SCALE, 0.0f, 0.0f));
			PlantModel(copy);
		}
	}
#endif
}


////////////////////////////////////////////

void NevadaTerrain::CreateFuture()
{
	AddNode(&m_Future);
	m_Future.SetName2("Future");
	m_Future.SetEnabled(false);
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

#define UPWARD_VELOCITY (6 * WORLD_SCALE)

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
		m_vel.y -= (0.15f * WORLD_SCALE);
		pos += m_vel;
	}

	// move in a circle
	pos.x = m_center.x + sinf(t/50.0f + m_fPhase) * 300.0f * WORLD_SCALE;
	pos.z = m_center.z + cosf(t/50.0f + m_fPhase) * 300.0f * WORLD_SCALE;

	pTarget->Identity();
	pTarget->Scale3(m_fScale, m_fScale, m_fScale);
	pTarget->SetTrans(pos);

	// wiggle it.. just a little bit
	float wiggle1 = 1.0f;
	if (m_vel.y > 0.0f)
		wiggle1 += sinf(t*10.0f) * 70.0f * m_vel.y;
	float wiggle2 = 1.0f;
	if (m_vel.y > 0.0f)
		wiggle2 += sinf(t*10.0f + PIf) * 50.0f * m_vel.y;
//	TRACE("wiggle %f\n", wiggle1);
	pTarget->Scale3(1.0f, wiggle1, wiggle2);

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

#if 0
	// Make a sprite to show the text
	m_pSprite = new vtSprite;
	m_pSprite->SetName2("Year Sprite");

	m_pSprite->SetWindowRect(0.4f, 0.95f, 0.6f, 1.05f);
	m_pSprite->SetText("1999 AD");
	m_pSprite->SetTextFont("Data/Fonts/default.txf");
#endif

#if 0
	// create overwater haze (fog)
	m_pFog1 = new vtFog();
	m_pFog1->SetStart(1.0f * WORLD_SCALE);	// 1 m
	m_pFog1->SetEnd(5000.0f * WORLD_SCALE);	// 5 km
//	m_pFog1->SetColor(RGBf(255.0f/255, 240.0f/255, 225.0f/255));	// tan
	m_pFog1->SetColor(RGBf(176.0f/255, 215.0f/255, 255.0f/255));	// blue horizon color
	m_pFog1->SetKind(FOG_Linear);

	// create underwater murk (fog)
	m_pFog2 = new vtFog();
	m_pFog2->SetStart(1.0f * WORLD_SCALE);	// 1 m
	m_pFog2->SetEnd(50.0f * WORLD_SCALE);	// 50 m
	m_pFog2->SetColor(RGBf(70.0f/255, 70.0f/255, 145.0f/255));
	m_pFog2->SetKind(FOG_Linear);
#endif
	m_pPastApp = pastApp;
	m_pPresentApp = presentApp;
}

void EpochEngine::Eval()
{
	// deal with the viewpoint interactions with the water:
	vtCamera *pCam = (vtCamera *)GetTarget();
	FPoint3 campos = pCam->GetTrans();

#if 0
	// turn on murkiness if we're under water?
	if (campos.y < m_fWaterHeight)
		vtGetScene()->SetFog(m_pFog2);
	else
		vtGetScene()->SetFog(m_pFog1);
	vtGetScene()->GetFog()->SetChanged(true);
#else
	// keep head above water
	float minimum = m_fWaterHeight + (15.0f * WORLD_SCALE);
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
	int lastYear = m_iYear;
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

		// try to fade green onto land
		float alpha;
		vtMaterial *pMaterial;
		RGBAf diffuse;
		if (m_iYear < PAST_PRESENT_SWITCH)
		{
			// green
			alpha = 0.7f * (1.0f - ((float)(PAST - m_iYear))/(float)(PAST - PAST_PRESENT_SWITCH));
			pMaterial = m_pPastApp;
			diffuse = pMaterial->GetDiffuse();
			diffuse.a = alpha;
			pMaterial->SetDiffuse1(diffuse);
			m_pNevada->m_pDynGeom->SetDetailMaterial(m_pPastApp, DETAIL_TILING);
		}
		else
		{
			// dry
			alpha = 1 - ((float)(PRESENT - m_iYear))/(float)(PRESENT - PAST_PRESENT_SWITCH);
			pMaterial = m_pPresentApp;
			diffuse = pMaterial->GetDiffuse();
			diffuse.a = alpha;
			pMaterial->SetDiffuse1(diffuse);
			m_pNevada->m_pDynGeom->SetDetailMaterial(m_pPresentApp, DETAIL_TILING);
		}
	}
	m_pNevada->m_Past.SetEnabled(m_iYear == PAST);
	m_pNevada->m_Present.SetEnabled(m_iYear == PRESENT);
	m_pNevada->m_Future.SetEnabled(m_iYear == FUTURE);

	// bounce test
	if (m_iSpeed == 0)
	{
		if (m_iYear == PRESENT)
		{
			m_iTargetYear = PAST;
			m_iSpeed = -178;
			pause = 460;
		}
		if (m_iYear == PAST)
		{
			m_iTargetYear = PRESENT;
			m_iSpeed = 178;
			pause = 460;
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
		m_pSprite->SetText(str);
	}
}



