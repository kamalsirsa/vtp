//
// class Enviro: Main functionality of the Enviro application
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/Fence3d.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/Route.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/DynTerrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/Globe.h"

#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"

#include "Enviro.h"
#include "Options.h"
#include "Hawaii.h"
#include "Nevada.h"
#include "SpecificTerrain.h"

#define ORTHO_HEIGHT		40000	// 40 km in the air
#define INITIAL_SPACE_DIST	3.1f
#define SPACE_DARKNESS		0.0f
#define UNFOLD_SPEED		0.01f

int pwdemo = 0;
vtGeom *tg = NULL;


///////////////////////////////////////////////////////////

Enviro *Enviro::s_pApp = NULL;

Enviro::Enviro()
{
	s_pApp = this;

	m_mode = MM_NONE;
	m_state = AS_Initializing;
	m_iInitStep = 0;
	m_bDoPlants = true;

	m_bActiveFence = false;
	m_pCurFence = NULL;
	m_CurFenceType = FT_WIRE;
	m_fFenceHeight = 1.5f;
	m_fFenceSpacing = 2.5f;

	m_bOnTerrain = false;
	m_bEarthShade = false;

	m_pGlobeContainer = NULL;
	m_bGlobeFlat = false;
	m_fFlattening = 1.0f;
	m_fFlattenDir = 0.0f;
	m_bGlobeUnfolded = false;
	m_fFolding = 0.0f;
	m_fFoldDir = 0.0f;
	m_pIcoGlobe = NULL;
	m_pSpaceAxes = NULL;

	m_bTopDown = false;
	m_pTopDownCamera = NULL;
	m_pTerrainPicker = NULL;
	m_pGlobePicker = NULL;
	m_pCursorMGeom = NULL;

	m_pArc = NULL;
	m_pArcMesh = NULL;
	m_pArcMats = NULL;
	m_fArcLength = 0.0;

	m_fMessageTime = 0.0f;

	// plants
	m_pPlantList = NULL;
	m_bPlantsLoaded = false;
	m_PlantOpt.m_iMode = 0;
	m_PlantOpt.m_iSpecies = 0;
	m_PlantOpt.m_fHeight = 100.0f;
	m_PlantOpt.m_iVariance = 20;
	m_PlantOpt.m_fSpacing = 2.0f;

	m_bDragging = false;
	m_bSelectedStruct = false;
	m_bSelectedPlant = false;
}

Enviro::~Enviro()
{
}

void Enviro::Startup()
{
	m_pTerrainScene = new vtTerrainScene();

	g_Log._StartLog("debug.txt");
	VTLOG("\nEnviro\nBuild:");
#if _DEBUG || DEBUG
	VTLOG(" Debug");
#else
	VTLOG(" Release");
#endif
#if UNICODE
	VTLOG(" Unicode");
#endif
	VTLOG("\n\n");

	// set up the datum list we will use
	SetupEPSGDatums();
}

void Enviro::Shutdown()
{
	VTLOG("Shutdown.\n");
	delete m_pPlantList;
	if (m_pArcMats)
		m_pArcMats->Release();
	if (m_pNormalCamera)
		m_pNormalCamera->Release();
	if (m_pTopDownCamera)
		m_pTopDownCamera->Release();
	if (m_pCursorMGeom)
		m_pCursorMGeom->Release();
	delete m_pTerrainScene;
	delete m_pIcoGlobe;
}

void Enviro::StartupArgument(int i, const char *str)
{
	VTLOG("Command line %d: %s\n", i, str);

	if (!strcmp(str, "-p"))
		pwdemo = 1;

	else if (!strcmp(str, "-no_plants"))
		m_bDoPlants = false;

	else if (!strcmp(str, "-fullscreen"))
		g_Options.m_bFullscreen = true;

	else if(!strncmp(str, "-terrain=", 9))
		g_Options.m_strInitTerrain = str+9;
}

void Enviro::LoadTerrainDescriptions()
{
	VTLOG("LoadTerrainDescriptions...\n");

	for (unsigned int i = 0; i < g_Options.m_DataPaths.size(); i++)
	{
		int count = 0;
		VTLOG("  On path '%s':", (const char *) g_Options.m_DataPaths[i]);

		vtString directory = g_Options.m_DataPaths[i] + "Terrains";
		for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
		{
			if (it.is_hidden() || it.is_directory())
				continue;

			std::string name1 = it.filename();
			vtString name = name1.c_str();

			// only look for ".ini" files
			if (name.GetLength() < 5 || name.Right(4).CompareNoCase(".ini"))
				continue;

			// Some terrain .ini files want to use a different Terrain class
			vtTerrain *pTerr;
			if (name == "Hawai`i.ini" || name == "Honoka`a.ini" || name == "Kealakekua.ini" )
				pTerr = new IslandTerrain();
			else if (name == "Nevada.ini")
				pTerr = new NevadaTerrain();
			else if (name == "TransitTerrain.ini")
				pTerr = new TransitTerrain();
			else if (name == "Romania.ini")
				pTerr = new Romania();
			else
				pTerr = new vtTerrain();

			if (pTerr->SetParamFile(directory + "/" + name))
				m_pTerrainScene->AppendTerrain(pTerr);
			count++;
		}
		VTLOG(" %d terrains.\n", count);
	}
	VTLOG(" Done.\n");
}

void Enviro::StartControlEngine()
{
	VTLOG("StartControlEngine\n");

	m_pControlEng = new ControlEngine();
	m_pControlEng->SetName2("Control Engine");
	vtGetScene()->AddEngine(m_pControlEng);
}

void Enviro::DoControl()
{
	if (m_fMessageTime != 0.0f)
	{
		if ((vtGetTime() - m_fMessageStart) > m_fMessageTime)
		{
			SetMessage("");
			m_fMessageTime = 0.0f;
		}
	}
	if (m_state == AS_Initializing)
	{
		m_iInitStep++;

		VTLOG("AS_Initializing initstep=%d\n", m_iInitStep);

		if (m_iInitStep == 1)
		{
			SetupScene1();
			return;
		}
		if (m_iInitStep == 2)
		{
			SetupScene2();
			return;
		}
		if (g_Options.m_bEarthView)
		{
			FlyToSpace();
			return;
		}
		else
		{
			if (!SwitchToTerrain(g_Options.m_strInitTerrain))
			{
				SetMessage("Terrain not found");
				m_state = AS_Error;
			}
			return;
		}
	}
	if (m_state == AS_MovingIn)
	{
		m_iInitStep++;
		SetupTerrain(m_pTargetTerrain);
	}
	if (m_state == AS_MovingOut)
	{
		m_iInitStep++;
		SetupGlobe();
	}
	if (m_state == AS_Orbit && m_fFlattenDir != 0.0f)
	{
		m_fFlattening += m_fFlattenDir;
		if (m_fFlattenDir > 0.0f && m_fFlattening > 1.0f)
		{
			m_fFlattening = 1.0f;
			m_fFlattenDir = 0.0f;
		}
		if (m_fFlattenDir < 0.0f && m_fFlattening < 0.0f)
		{
			m_fFlattening = 0.0f;
			m_fFlattenDir = 0.0f;
		}
		m_pIcoGlobe->SetInflation(m_fFlattening);
	}
	if (m_state == AS_Orbit && m_fFoldDir != 0.0f)
	{
		m_fFolding += m_fFoldDir;
		if (m_fFoldDir > 0.0f && m_fFolding > 1.0f)
		{
			// all the way flattened
			m_pFlatFlyer->SetEnabled(true);

			m_fFolding = 1.0f;
			m_fFoldDir = 0.0f;
		}
		if (m_fFoldDir < 0.0f && m_fFolding < 0.0f)
		{
			m_fFolding = 0.0f;
			m_fFoldDir = 0.0f;

			// Leave Flat View
			m_pTrackball->SetEnabled(true);

			// Turn globe culling back on
			m_pIcoGlobe->SetCulling(true);
		}
		m_pIcoGlobe->SetUnfolding(m_fFolding);

		FPQ pq;
		pq.Interpolate(m_SpaceLoc, m_FlatLoc, m_fFolding);
		FMatrix4 m4;
		pq.ToMatrix(m4);
		m_pNormalCamera->SetTransform1(m4);
	}
}

void Enviro::FlyToSpace()
{
	VTLOG("FlyToSpace\n");
	if (m_state == AS_Terrain)
	{
		// remember camera position
		vtTerrain *pT = GetCurrentTerrain();
		vtCamera *pCam = vtGetScene()->GetCamera();
		FMatrix4 mat;
		pCam->GetTransform1(mat);
		pT->SetCamLocation(mat);
	}

	// turn off terrain, if any
	m_pTerrainScene->SetTerrain(NULL);
	EnableFlyerEngine(false);

	m_state = AS_MovingOut;
	m_iInitStep = 0;
	FreeArc();

	// Layer view needs to stop showing terrain layers
	RefreshLayerView();

	// Inform the GUI that there is no terrain
	SetTerrainToGUI(NULL);
}

void Enviro::SetupGlobe()
{
	VTLOG("SetupGlobe step %d\n", m_iInitStep);

	if (m_iInitStep == 1)
	{
		m_pTerrainPicker->SetEnabled(false);
		SetMessage("Creating Globe");
	}
	if (m_iInitStep == 2)
	{
		if (m_pGlobeContainer == NULL)
		{
			MakeGlobe();
			m_SpaceCamLocation.Identity();
			m_SpaceCamLocation.Translate(FPoint3(0.0f, 0.0f, INITIAL_SPACE_DIST));
		}
		SetMessage("Switching to Globe");
	}
	if (m_iInitStep == 3)
	{
		// put the light where the sun should be
		vtMovLight *pSunLight = m_pTerrainScene->GetSunLight();
		pSunLight->Identity();
		pSunLight->SetTrans(FPoint3(0, 0, -5));

if (pwdemo){
		pSunLight->m_pLight->SetColor(RGBf(1, 1, 1));
		pSunLight->m_pLight->SetAmbient(RGBf(0, 0, 0));
}else{
		// standard bright sunlight
		pSunLight->m_pLight->SetColor(RGBf(3, 3, 3));
		pSunLight->m_pLight->SetAmbient(RGBf(0.5f, 0.5f, 0.5f));
}
		vtGetScene()->SetBgColor(RGBf(SPACE_DARKNESS, SPACE_DARKNESS, SPACE_DARKNESS));

		m_pGlobeContainer->SetEnabled(true);
		m_pRoot->AddChild(m_pGlobeContainer);
		m_pCursorMGeom->Identity();
		m_pCursorMGeom->Scale3(.1f, .1f, .1f);
	}
	if (m_iInitStep == 4)
	{
		vtCamera *pCam = vtGetScene()->GetCamera();
		pCam->SetHither(0.01f);
		pCam->SetYon(50.0f);
		pCam->SetFOV(60 * (PIf / 180.0f));
	}
	if (m_iInitStep == 5)
	{
if (pwdemo){
		SetEarthShading(true);
}else{
		SetEarthShading(false);
}
	}
	if (m_iInitStep == 6)
	{
		vtCamera *pCam = vtGetScene()->GetCamera();
		pCam->SetTransform1(m_SpaceCamLocation);

		m_pTrackball->SetEnabled(true);
	}
	if (m_iInitStep == 7)
	{
		m_state = AS_Orbit;
		SetMode(MM_SELECT);
		if (!strncmp((const char *) g_Options.m_strImage, "geosphere", 9))
			SetMessage("Earth image (c) The GeoSphere Project", 3);
		else
			SetMessage("Earth View", 10);
		m_pGlobePicker->SetEnabled(true);

		// Layer view needs to update
		RefreshLayerView();
	}
}

bool Enviro::SwitchToTerrain(const char *name)
{
	vtTerrain *pTerr = m_pTerrainScene->FindTerrainByName(name);
	if (pTerr)
	{
		SwitchToTerrain(pTerr);
		return true;
	}
	else
		return false;
}

void Enviro::SwitchToTerrain(vtTerrain *pTerr)
{
	// The first time we switch to a terrain, try to load the plants
	if (!m_bPlantsLoaded)
	{
		m_bPlantsLoaded = true;
		SetupCommonCulture();
	}

	if (m_state == AS_Orbit)
	{
		// hide globe
		if (m_pGlobeContainer != NULL)
		{
			m_pGlobeContainer->SetEnabled(false);
			m_pGlobePicker->SetEnabled(false);
		}

		// remember camera position
		vtCamera *pCam = vtGetScene()->GetCamera();
		pCam->GetTransform1(m_SpaceCamLocation);

		m_pTrackball->SetEnabled(false);
	}
	if (m_state == AS_Terrain)
	{
		// remember camera position
		vtTerrain *pT = GetCurrentTerrain();
		vtCamera *pCam = vtGetScene()->GetCamera();
		FMatrix4 mat;
		pCam->GetTransform1(mat);
		pT->SetCamLocation(mat);
	}
	vtTerrain *pT = GetCurrentTerrain();
	if (pT)
		pT->SaveRoute();

	m_state = AS_MovingIn;
	m_pTargetTerrain = pTerr;
	m_iInitStep = 0;
	FreeArc();

	// Layer view needs to update
	RefreshLayerView();
}

void Enviro::SetupTerrain(vtTerrain *pTerr)
{
	int iError;

	if (m_iInitStep == 1)
	{
		vtString str;
		str.Format("Creating Terrain '%s'", (const char*) pTerr->GetName());
		SetMessage(str);
	}
	if (m_iInitStep == 2)
	{
		if (pTerr->IsCreated())
			m_iInitStep = 8;	// already made, skip ahead
		else
			SetMessage("Loading Elevation");
	}
	if (m_iInitStep == 3)
	{
		pTerr->LoadParams();
		pTerr->SetPlantList(m_pPlantList);
		if (!pTerr->CreateStep1(iError))
		{
			m_state = AS_Error;
			SetMessage(pTerr->DescribeError(iError));
			return;
		}
		SetMessage("Loading/Chopping/Prelighting Textures");
	}
	if (m_iInitStep == 4)
	{
		if (!pTerr->CreateStep2(iError))
		{
			m_state = AS_Error;
			SetMessage(pTerr->DescribeError(iError));
			return;
		}
		SetMessage("Building Terrain");
	}
	if (m_iInitStep == 5)
	{
		if (!pTerr->CreateStep3(iError))
		{
			m_state = AS_Error;
			SetMessage(pTerr->DescribeError(iError));
			return;
		}
		SetMessage("Building CLOD");
	}
	if (m_iInitStep == 6)
	{
		if (!pTerr->CreateStep4(iError))
		{
			m_state = AS_Error;
			SetMessage(pTerr->DescribeError(iError));
			return;
		}
		SetMessage("Creating Culture");
	}
	if (m_iInitStep == 7)
	{
		if (!pTerr->CreateStep5(g_Options.m_bSound != 0, iError))
		{
			m_state = AS_Error;
			SetMessage(pTerr->DescribeError(iError));
			return;
		}

		// Initial default location for camera for this terrain: Try center
		//  of heightfield, just above the ground
		vtHeightField3d *pHF = pTerr->GetHeightField();
		FPoint3 middle;
		FMatrix4 mat;

		pHF->GetCenter(middle);
		pHF->FindAltitudeAtPoint(middle, middle.y);
		middle.y += pTerr->GetParams().m_iMinHeight;
		mat.Identity();
		mat.SetTrans(middle);
		pTerr->SetCamLocation(mat);
	}
	if (m_iInitStep == 8)
	{
		SetMessage("Setting hither/yon");
		vtCamera *pCam = vtGetScene()->GetCamera();
		pCam->SetHither(pTerr->GetParams().m_fHither);
		pCam->SetYon(500000.0f);
	}
	if (m_iInitStep == 9)
	{
		if (g_Options.m_bSpeedTest)
		{
			// Benchmark engine - removed, need a better one
			// m_pBench = new BenchEngine();
		}
		else
		{
			VTLOG("Setting Camera Location\n");
			m_pNormalCamera->SetTransform1(pTerr->GetCamLocation());
		}
		SetMessage("Switching to Terrain");
	}
	if (m_iInitStep == 10)
	{
		// make first terrain active
		SetTerrain(pTerr);

		m_pCurRoute=pTerr->GetLastRoute();	// Error checking needed here.

		m_pTerrainPicker->SetEnabled(true);
		SetMode(MM_NAVIGATE);
	}
	if (m_iInitStep == 11)
	{
		m_state = AS_Terrain;
		vtString str;
		str.Format("Welcome to %s", (const char *)pTerr->GetName());
		SetMessage(str, 5.0f);

		// Layer view needs to update
		RefreshLayerView();
	}
}

void Enviro::FormatCoordString(vtString &str, const DPoint3 &coord, LinearUnits units)
{
	if (units == LU_DEGREES)
	{
		int deg1 = (int) coord.x;
		int min1 = (int) ((coord.x - deg1) * 60);
		if (deg1 < 0) deg1 = -deg1;
		if (min1 < 0) min1 = -min1;
		char ew = m_EarthPos.x > 0.0f ? 'E' : 'W';

		int deg2 = (int) coord.y;
		int min2 = (int) ((coord.y - deg2) * 60);
		if (deg2 < 0) deg2 = -deg2;
		if (min2 < 0) min2 = -min2;
		char ns = m_EarthPos.y > 0.0f ? 'N' : 'S';

		str.Format("%3d:%02d %c, %3d:%02d %c", deg1, min1, ew, deg2, min2, ns);
	}
	else
	{
		str.Format("%7d, %7d", (int) coord.x, (int) coord.y);
	}
}

void Enviro::DescribeCoordinates(vtString &str)
{
	DPoint3 epos;
	vtString str1;

	str = "";

	if (m_state == AS_Orbit)
	{
		// give location of cursor
		str = "Cursor: ";
		m_pGlobePicker->GetCurrentEarthPos(epos);
		FormatCoordString(str1, epos, LU_DEGREES);
		str += str1;
		if (m_mode == MM_MEASURE && (m_fArcLength != 0.0 || m_bDragging))
		{
			str1.Format(", arc = %.0lf meters", m_fArcLength);
			str += str1;
		}
		vtTerrain *pTerr = FindTerrainOnEarth(DPoint2(epos.x, epos.y));
		if (pTerr)
		{
			str1.Format(", Terrain: %s", (const char *) pTerr->GetName());
			str += str1;
		}
	}
	if (m_state == AS_Terrain)
	{
		vtTerrain *pTerr = GetCurrentTerrain();

		// give location of camera and cursor
		str = "Camera: ";
		// get camera pos
		vtScene *scene = vtGetScene();
		vtCamera *camera = scene->GetCamera();
		FPoint3 campos = camera->GetTrans();

		// Find corresponding earth coordinates
		g_Conv.ConvertToEarth(campos, epos);

		FormatCoordString(str1, epos, g_Conv.GetUnits());
		str += str1;
		str1.Format(" elev %.1f", epos.z);
		str += str1;

		// ground cursor
		str += ", Cursor: ";
		bool bOn = m_pTerrainPicker->GetCurrentEarthPos(epos);
		if (bOn)
		{
			FormatCoordString(str1, epos, g_Conv.GetUnits());
			str += str1;
			str1.Format(" elev %.1f", epos.z);
			str += str1;
		}
		else
			str += " Not on ground";
	}
	str += " ";
}

void Enviro::DescribeCLOD(vtString &str)
{
	str = "";

	if (m_state != AS_Terrain) return;
	vtTerrain *t = GetCurrentTerrain();
	if (!t) return;
	vtDynTerrainGeom *dtg = t->GetDynTerrain();
	if (!dtg) return;

	//
	// McNally CLOD algo uses a triangle count target, all other current
	// implementations use a floating point factor relating to error/detail
	//
	if (t->GetParams().m_eLodMethod == LM_MCNALLY ||
		t->GetParams().m_eLodMethod == LM_ROETTGER)
	{
		str.Format("CLOD: target %d, drawn %d ", dtg->GetPolygonCount(),
			dtg->GetNumDrawnTriangles());
	}
	else
	{
		str.Format("CLOD detail: %.1f, drawn %d", dtg->GetPixelError(),
			dtg->GetNumDrawnTriangles());
	}
}

//
// Check the terrain under the picker (for both Terrain and Earth views).
//
void Enviro::DoPickers()
{
	m_bOnTerrain = false;
	DPoint3 earthpos;
	vtString str;

	if (m_state == AS_Orbit)
	{
		if (m_pGlobePicker != NULL)
			m_bOnTerrain = m_pGlobePicker->GetCurrentEarthPos(earthpos);
		if (m_bOnTerrain)
			m_EarthPos = earthpos;

		vtString str1, str2;
		FormatCoordString(str1, m_EarthPos, LU_DEGREES);
		str2 = "Cursor ";
		str2 += str1;
		m_pSprite2->SetText(str2);
	}
	if (m_state == AS_Terrain && m_pCursorMGeom->GetEnabled())
	{
		if (m_pTerrainPicker != NULL)
			m_bOnTerrain = m_pTerrainPicker->GetCurrentEarthPos(earthpos);
		if (m_bOnTerrain)
		{
			m_EarthPos = earthpos;

			// Attempt to scale the 3d cursor, for ease of use.
			// Rather than keeping it the same size in world space (it would
			// be too small in the distance) or the same size in screen space
			// (would look confusing without the spatial distance cue) we
			// compromise and scale it based on the square root of distance.
			FPoint3 gpos;
			if (m_pTerrainPicker->GetCurrentPoint(gpos))
			{
				FPoint3 campos = vtGetScene()->GetCamera()->GetTrans();
				float distance = (gpos - campos).Length();
				float sc = (float) sqrt(distance) / 1.0f;
				FPoint3 pos = m_pCursorMGeom->GetTrans();
				m_pCursorMGeom->Identity();
				m_pCursorMGeom->Scale3(sc, sc, sc);
				m_pCursorMGeom->SetTrans(pos);
			}
			str.Format("Cursor %7d, %7d", (int) m_EarthPos.x, (int) m_EarthPos.y);
			m_pSprite2->SetText(str);

			// Inform GUI, in case it cares.
			EarthPosUpdated();
		}
		else
			m_pSprite2->SetText("Not on terrain");
	}
}

void GeomAddRectMesh(vtGeom *pGeom, const FRECT &rect, float z, int matidx)
{
	vtMesh *mesh = new vtMesh(GL_TRIANGLE_FAN, VT_TexCoords, 4);
	mesh->AddVertexUV(FPoint3(rect.left, rect.bottom, z), 0, 0);
	mesh->AddVertexUV(FPoint3(rect.right, rect.bottom, z), 1, 0);
	mesh->AddVertexUV(FPoint3(rect.right, rect.top, z), 1, 1);
	mesh->AddVertexUV(FPoint3(rect.left, rect.top, z), 0, 1);
	mesh->AddFan(0, 1, 2, 3);
	pGeom->AddMesh(mesh, matidx);
}


//
// Create the earth globe
//
void Enviro::MakeGlobe()
{
	VTLOG("MakeGlobe\n");

	m_pGlobeTime = new TimeEngine;
	m_pGlobeTime->SetName2("GlobeTime");
	vtGetScene()->AddEngine(m_pGlobeTime);

	m_pGlobeContainer = new vtGroup;
	m_pGlobeContainer->SetName2("Globe Container");

	// simple globe
//	m_pGlobeXForm = CreateSimpleEarth(g_Options.m_DataPaths);

	// fancy icosahedral globe
	m_pIcoGlobe = new IcoGlobe();
	m_pIcoGlobe->Create(5000, g_Options.m_DataPaths, g_Options.m_strImage,
//		IcoGlobe::GEODESIC);
//		IcoGlobe::RIGHT_TRIANGLE);
		IcoGlobe::DYMAX_UNFOLD);
	m_pGlobeContainer->AddChild(m_pIcoGlobe->GetTop());
	m_pGlobeTime->AddTarget((TimeTarget *)m_pIcoGlobe);

if (pwdemo){
	logo = new vtGroup;
	IcoGlobe *Globe2 = new IcoGlobe();
	Globe2->Create(1000, g_Options.m_DataPaths, vtString(""),
		IcoGlobe::GEODESIC);
	vtTransform *trans = new vtTransform();
	trans->SetName2("2nd Globe Scaler");
	m_pGlobeContainer->AddChild(logo);
	logo->AddChild(trans);
	trans->AddChild(Globe2->GetTop());
	trans->Scale3(1.006f, 1.006f, 1.006f);
	m_pGlobeTime->AddTarget((TimeTarget *)Globe2);

	// Planetwork globe is around 3 PM GMT, summer over the north atlantic
	m_pGlobeTime->SetDate(2000, 6, 20);
	m_pGlobeTime->SetGMT(15,0,0);

	vtGeom *geom = new vtGeom();
	vtMaterialArray *mats = new vtMaterialArray();
	mats->AddTextureMaterial2("Planetwork/logo3.png", false, false, true);
	mats->AddTextureMaterial2("Planetwork/logo2.png", false, false, true);
	geom->SetMaterials(mats);
	mats->Release();
	float width = 1.9, height = .22;
	FRECT rect(-width/2, height/2, width/2, -height/2);
	GeomAddRectMesh(geom, rect, 1.15, 0);
	rect += FPoint2(0.01, -0.01);
	GeomAddRectMesh(geom, rect, 1.14, 1);
	logo->AddChild(geom);
	Globe2->SetTime(m_pGlobeTime->GetTime());

	int i;
	vtMaterialArray *rainbow = new vtMaterialArray;
	rainbow->AddRGBMaterial1(RGBf(0.5,0,0), false, true, false, 0.5f);
	rainbow->AddRGBMaterial1(RGBf(0.5,0.5,0), false, true, false, 0.5f);
	rainbow->AddRGBMaterial1(RGBf(0,0.5,0), false, true, false, 0.5f);
	rainbow->AddRGBMaterial1(RGBf(0,0.5,0.5), false, true, false, 0.5f);
	rainbow->AddRGBMaterial1(RGBf(0,0,0.5), false, true, false, 0.5f);
	rainbow->AddRGBMaterial1(RGBf(0.5,0,0.5), false, true, false, 0.5f);
	for (i = 0; i < 6; i++)
	{
		vtMaterial *mat = rainbow->GetAt(i);
		mat->SetTransparent(true, true);
	}
	tg = new vtGeom();
	tg->SetMaterials(rainbow);
	rainbow->Release();
	Globe2->GetTop()->AddChild(tg);

	vtFeatures ft;
	ft.LoadFromSHP("../Data/PointData/vtp-users-020519.shp");
	int half = ft.GetNumEntities() / 2;
	int foo = 0;
	for (i = 0; i < half; i++)
	{
		DPoint2 p1, p2;
		ft.GetPoint(i, p1);
		ft.GetPoint(i+half, p2);
		if (p1 == DPoint2(0,0) || p2 == DPoint2(0,0))
			continue;
		if (p1.y > 0 && p2.y > 0)
		{
			foo++;
			if ((foo%20)<19)
				continue;
		}
		if (p1.y < 0 && p2.y < 0)
			continue;

		vtMesh *mesh = new vtMesh(GL_LINE_STRIP, 0, 20);
		Globe2->AddSurfaceLineToMesh(mesh, p1, p2);
		tg->AddMesh(mesh, i%6);
	}
	tg->SetEnabled(false);

}else{
	logo = NULL;
}

	// pass the time along once to orient the earth
	m_pIcoGlobe->SetTime(m_pGlobeTime->GetTime());

	// use a trackball engine for navigation
	//
	VTLOG("\tcreating Trackball\n");
	m_pTrackball = new vtTrackball(INITIAL_SPACE_DIST);
	m_pTrackball->SetName2("Trackball2");
	m_pTrackball->SetTarget(vtGetScene()->GetCamera());
	m_pTrackball->SetRotateButton(VT_RIGHT, 0, false);
	m_pTrackball->SetZoomButton(VT_RIGHT, VT_SHIFT);
	vtGetScene()->AddEngine(m_pTrackball);

	// determine where the terrains are, and show them as red rectangles
	//
	LookUpTerrainLocations();
	VTLOG("AddTerrainRectangles\n");
	m_pIcoGlobe->AddTerrainRectangles(GetTerrainScene());

	// create the GlobePicker engine for picking features on the earth
	//
	m_pGlobePicker = new GlobePicker();
	m_pGlobePicker->SetName2("GlobePicker");
	m_pGlobePicker->SetGlobe(m_pIcoGlobe);
	vtGetScene()->AddEngine(m_pGlobePicker);
	m_pGlobePicker->SetTarget(m_pCursorMGeom);
	m_pGlobePicker->SetRadius(1.0);
	m_pGlobePicker->SetEnabled(false);

	// create some stars around the earth
	//
	vtStarDome *pStars = new vtStarDome();
	vtString bsc_file = FindFileOnPaths(g_Options.m_DataPaths, "Sky/bsc.data");
	if (bsc_file != "")
	{
		pStars->Create(bsc_file, 20.0f, 5.0f);	// radius, brightness
		m_pGlobeContainer->AddChild(pStars);
	}

	// create some geometry showing various astronomical axes
	vtMaterialArray *pMats = new vtMaterialArray();
	int yellow = pMats->AddRGBMaterial1(RGBf(1,1,0), false, false);
	int red = pMats->AddRGBMaterial1(RGBf(1,0,0), false, false);
	int green = pMats->AddRGBMaterial1(RGBf(0,1,0), false, false);

	m_pSpaceAxes = new vtGeom();
	m_pSpaceAxes->SetName2("Earth Axes");
	m_pSpaceAxes->SetMaterials(pMats);
	pMats->Release();	// pass ownership to Geometry

	vtMesh *mesh = new vtMesh(GL_LINES, 0, 6);
	mesh->AddVertex(FPoint3(0,0,200));
	mesh->AddVertex(FPoint3(0,0,0));
	mesh->AddLine(0,1);
	mesh->AddVertex(FPoint3(0,0,1));
	mesh->AddVertex(FPoint3(-.07f,0,1.1f));
	mesh->AddVertex(FPoint3( .07f,0,1.1f));
	mesh->AddLine(2,3);
	mesh->AddLine(2,4);
	m_pSpaceAxes->AddMesh(mesh, yellow);
	mesh->Release();	// pass ownership to Geometry

	mesh = new vtMesh(GL_LINES, 0, 6);
	mesh->AddVertex(FPoint3(1.5f,0,0));
	mesh->AddVertex(FPoint3(-1.5f,0,0));
	mesh->AddLine(0,1);
	mesh->AddVertex(FPoint3(-1.4f,0.07f,0));
	mesh->AddVertex(FPoint3(-1.4f,-0.07f,0));
	mesh->AddLine(1,2);
	mesh->AddLine(1,3);
	m_pSpaceAxes->AddMesh(mesh, green);
	mesh->Release();	// pass ownership to Geometry

	mesh = new vtMesh(GL_LINES, 0, 6);
	mesh->AddVertex(FPoint3(0,2,0));
	mesh->AddVertex(FPoint3(0,-2,0));
	mesh->AddLine(0,1);
	m_pSpaceAxes->AddMesh(mesh, red);
	mesh->Release();	// pass ownership to Geometry

	m_pGlobeContainer->AddChild(m_pSpaceAxes);
	m_pSpaceAxes->SetEnabled(false);
}

void Enviro::SetSpaceAxes(bool bShow)
{
	if (m_pSpaceAxes)
		m_pSpaceAxes->SetEnabled(bShow);
	if (m_pIcoGlobe)
		m_pIcoGlobe->ShowAxis(bShow);
}

bool Enviro::GetSpaceAxes()
{
	if (m_pSpaceAxes)
		return m_pSpaceAxes->GetEnabled();
	return false;
}

void Enviro::LookUpTerrainLocations()
{
	VTLOG("LookUpTerrainLocations\n");

	// look up the earth location of each known terrain
	vtTerrain *pTerr;
	for (pTerr = m_pTerrainScene->GetFirstTerrain(); pTerr; pTerr=pTerr->GetNext())
	{
		VTLOG("\tlooking up: %s\n", (const char *) pTerr->GetName());

		vtElevationGrid grid;
		bool success = pTerr->LoadHeaderIntoGrid(grid);

		if (!success)
		{
			VTLOG("\t\tFailed to load header info.\n");
			continue;
		}
		char msg1[2000], msg2[2000];
		success = grid.GetProjection().GetTextDescription(msg1, msg2);
		if (!success)
		{
			VTLOG("\t\tCouldn't get projection description.\n");
			continue;
		}
		VTLOG("\t\tprojection: type %s, value %s\n", msg1, msg2);

		DPoint2 sw, nw, ne, se;
		VTLOG("\t\tGetting terrain corners\n");
		success = grid.GetCorners(pTerr->m_Corners_geo, true);
		if (!success)
		{
			VTLOG("\t\tCouldn't get terrain corners.\n");
			continue;
		}
		nw = pTerr->m_Corners_geo[1];
		se = pTerr->m_Corners_geo[3];
		VTLOG("\t\t(%.2lf,%.2lf) - (%.2lf,%.2lf)\n", nw.x, nw.y, se.x, se.y);
		VTLOG("\t\tGot terrain corners\n");
	}
	VTLOG("\tLookUpTerrainLocations: done\n");
}

int Enviro::AddGlobePoints(const char *fname)
{
//	int num_added = m_pIcoGlobe->AddGlobePoints(fname, 0.0015f);	// this size works OK for the VTP recipients
	int num_added = m_pIcoGlobe->AddGlobePoints(fname, 0.003f);
//	int num_added = m_pIcoGlobe->AddGlobePoints(fname, 0.0005f);	// better for GeoURL
	if (num_added != -1)
		RefreshLayerView();
	return num_added;
}


void Enviro::SetupScene1()
{
	VTLOG("SetupScene1\n");

	vtScene *pScene = vtGetScene();

	vtTerrain::SetDataPath(g_Options.m_DataPaths);
	vtTerrain::s_Content.SetDataPaths(&g_Options.m_DataPaths);

	vtCamera *pCamera = pScene->GetCamera();
	if (pCamera) pCamera->SetName2("Standard Camera");

	m_pRoot = m_pTerrainScene->BeginTerrainScene(g_Options.m_bSound != 0);
	pScene->SetRoot(m_pRoot);

//	m_pTerrainScene->SetupEngines();
}

void Enviro::SetupScene2()
{
	VTLOG("SetupScene2\n");

	// Make navigation engines
	m_pOrthoFlyer = new vtOrthoFlyer(1.0f);
	m_pOrthoFlyer->SetName2("Orthographic View Flyer");
	m_pOrthoFlyer->SetEnabled(false);
	vtGetScene()->AddEngine(m_pOrthoFlyer);

	m_pQuakeFlyer = new QuakeFlyer(1.0f, 1.0f, true);
	m_pQuakeFlyer->SetName2("Quake-Style Flyer");
	m_pQuakeFlyer->SetEnabled(false);
	vtGetScene()->AddEngine(m_pQuakeFlyer);

	m_pVFlyer = new VFlyer(1.0f, 1.0f, true);
	m_pVFlyer->SetName2("Velocity-Gravity Flyer");
	m_pVFlyer->SetEnabled(false);
	vtGetScene()->AddEngine(m_pVFlyer);

	m_pTFlyer = new vtTerrainFlyer(1.0f, 1.0f, true);
	m_pTFlyer->SetName2("Terrain-following Flyer");
	m_pTFlyer->SetEnabled(false);
	vtGetScene()->AddEngine(m_pTFlyer);

	m_pGFlyer = new GrabFlyer(1.0f, 1.0f, true);
	m_pGFlyer->SetName2("Grab-Pivot Flyer");
	m_pGFlyer->SetEnabled(false);
	vtGetScene()->AddEngine(m_pGFlyer);

	m_pFlatFlyer = new FlatFlyer();
	m_pFlatFlyer->SetName2("Flat Flyer");
	m_pFlatFlyer->SetEnabled(false);
	vtGetScene()->AddEngine(m_pFlatFlyer);
	m_pFlatFlyer->SetEnabled(false);

	m_pPanoFlyer = new vtPanoFlyer(1.0f, 1.0f, true);
	m_pPanoFlyer->SetName2("Panoramic Flyer");
	m_pPanoFlyer->SetEnabled(false);
	vtGetScene()->AddEngine(m_pPanoFlyer);

	m_nav = NT_Normal;

	// create picker object and picker engine
	float size = 1.0;
	m_pCursorMGeom = new vtMovGeom(Create3DCursor(size, size/35));
	m_pCursorMGeom->SetName2("Cursor");

	m_pTerrainScene->GetTop()->AddChild(m_pCursorMGeom);
	m_pTerrainPicker = new TerrainPicker();
	m_pTerrainPicker->SetName2("TerrainPicker");
	vtGetScene()->AddEngine(m_pTerrainPicker);

	m_pTerrainPicker->SetTarget(m_pCursorMGeom);
	m_pTerrainPicker->SetEnabled(false); // turn off at startup

	// Connect to the GrabFlyer
	m_pGFlyer->SetTerrainPicker(m_pTerrainPicker);

#if 0
	m_pSprite2 = new vtSprite();
	m_pSprite2->SetName2("Sprite2");
	m_pSprite2->SetWindowRect(0.73f, 0.90f, 1.00f, 1.00f);
	m_pSprite2->SetText("...");
	m_pRoot->AddChild(m_pSprite2);
#endif

	VTLOG("Setting up Cameras\n");
	m_pNormalCamera = vtGetScene()->GetCamera();

	// Create second camera (for Top-Down view)
	if (m_pTopDownCamera == NULL)
	{
		VTLOG("Creating Top-Down Camera\n");
		m_pTopDownCamera = new vtCamera();
		m_pTopDownCamera->SetOrtho(true);
		m_pTopDownCamera->SetWidth(10000.0f);
		m_pTopDownCamera->SetName2("Top-Down Camera");
		m_pOrthoFlyer->SetTarget(m_pTopDownCamera);
	}

	m_pQuakeFlyer->SetTarget(m_pNormalCamera);
	m_pVFlyer->SetTarget(m_pNormalCamera);
	m_pTFlyer->SetTarget(m_pNormalCamera);
	m_pGFlyer->SetTarget(m_pNormalCamera);
	m_pFlatFlyer->SetTarget(m_pNormalCamera);
	m_pPanoFlyer->SetTarget(m_pNormalCamera);
}

void Enviro::SetupCommonCulture()
{
	VTLOG("SetupCommonCulture\n");

	vtFence3d::SetScale(g_Options.m_fPlantScale);

	if (m_bDoPlants)
	{
		// First look for species.xml with terrain name prepended, otherwise fall
		//  back on just "species.xml"
		vtString species_fname = "PlantData/" + g_Options.m_strInitTerrain + "-species.xml";
		vtString species_path = FindFileOnPaths(g_Options.m_DataPaths, species_fname);
		if (species_path == "")
			species_path = FindFileOnPaths(g_Options.m_DataPaths, "PlantData/species.xml");

		vtPlantList pl;
		if (species_path != "" && pl.ReadXML(species_path))
		{
			m_pPlantList = new vtPlantList3d();
			*m_pPlantList = pl;
			m_pPlantList->CreatePlantSurfaces(g_Options.m_DataPaths,
				g_Options.m_fPlantScale, g_Options.m_bShadows != 0, true);
		}
	}
}

void Enviro::SetCurrentNavigator(vtTerrainFlyer *pE)
{
	if (m_pCurrentFlyer != NULL)
	{
		const char *name = m_pCurrentFlyer->GetName2();
		m_pCurrentFlyer->SetEnabled(false);
	}
	m_pCurrentFlyer = pE;
	if (m_pCurrentFlyer != NULL)
	{
		m_pCurrentFlyer->SetEnabled(true);
	}
}

void Enviro::EnableFlyerEngine(bool bEnable)
{
	if (bEnable)
	{
		if (m_nav == NT_Normal)
			SetCurrentNavigator(m_pTFlyer);
		if (m_nav == NT_Velo)
			SetCurrentNavigator(m_pVFlyer);
		if (m_nav == NT_Grab)
			SetCurrentNavigator(m_pGFlyer);
		if (m_nav == NT_Quake)
			SetCurrentNavigator(m_pQuakeFlyer);
		if (m_nav == NT_Pano)
			SetCurrentNavigator(m_pPanoFlyer);
	}
	else
		SetCurrentNavigator(NULL);
}

void Enviro::SetNavType(NavType nav)
{
	if (m_mode == MM_NAVIGATE)
		EnableFlyerEngine(false);
	m_nav = nav;
	if (m_mode == MM_NAVIGATE)
		EnableFlyerEngine(true);
}

void Enviro::SetMaintain(bool bOn)
{
	if (m_pCurrentFlyer != NULL)
	{
		m_pCurrentFlyer->SetMaintain(bOn);
		m_pCurrentFlyer->SetMaintainHeight(0);
	}
}

bool Enviro::GetMaintain()
{
	if (m_pCurrentFlyer == NULL)
		return false;
	return m_pCurrentFlyer->GetMaintain();
}


void Enviro::SetTerrain(vtTerrain *pTerrain)
{
	VTLOG("Enviro::SetTerrain '%s'\n", (const char *) pTerrain->GetName());

	// safety check
	if (!pTerrain)
		return;
	vtHeightField3d *pHF = pTerrain->GetHeightField();
	if (!pHF)
		return;

	m_pTerrainScene->SetTerrain(pTerrain);

	TParams &param = pTerrain->GetParams();

	SetNavType((enum NavType) param.m_iNavStyle);	// TODO: TEST THIS

	EnableFlyerEngine(true);

	// inform the navigation engine of the new terrain
	m_pCurrentFlyer->SetTarget(m_pNormalCamera);
	m_pCurrentFlyer->SetHeight(param.m_iMinHeight);
	m_pTFlyer->SetSpeed(param.m_fNavSpeed);
	m_pVFlyer->SetSpeed(param.m_fNavSpeed);
	m_pPanoFlyer->SetSpeed(param.m_fNavSpeed);
	m_pOrthoFlyer->SetSpeed(param.m_fNavSpeed);
	m_pCurrentFlyer->SetEnabled(true);
	m_pCurrentFlyer->SetExag(param.m_bAccel);

	// TODO: a more elegant way of keeping all nav engines current
	m_pQuakeFlyer->SetHeightField(pHF);
	m_pVFlyer->SetHeightField(pHF);
	m_pTFlyer->SetHeightField(pHF);
	m_pGFlyer->SetHeightField(pHF);
	m_pPanoFlyer->SetHeightField(pHF);

	// set the top-down viewpoint to a point over
	//  the center of the new terrain
	FPoint3 middle;
	pHF->GetCenter(middle);
	middle.y = ORTHO_HEIGHT;
	m_pTopDownCamera->SetTrans(middle);
	m_pTopDownCamera->RotateLocal(TRANS_XAxis, -PID2f);
	m_pTopDownCamera->SetHither(5.0f);
	m_pTopDownCamera->SetYon(middle.y * 2.0f);

	if (m_pTerrainPicker != NULL)
		m_pTerrainPicker->SetHeightField(pHF);

	// Inform the GUI that the terrain has changed
	SetTerrainToGUI(pTerrain);
}


//
// Display a message as a text sprite in the middle of the window.
//
// The fTime argument lets you specify how long the message should
// appear, in seconds.
//
void Enviro::SetMessage(const char *msg, float fTime)
{
	VTLOG("  SetMessage: '%s'\n", msg);

	vtString str = msg;

#if 0
	if (m_pMessageSprite == NULL)
	{
		m_pMessageSprite = new vtSprite();
		m_pMessageSprite->SetName2("MessageSprite");
		m_pRoot->AddChild(m_pMessageSprite);
	}
	if (str == "")
		m_pMessageSprite->SetEnabled(false);
	else
	{
		m_pMessageSprite->SetEnabled(true);
		m_pMessageSprite->SetText(str);
		int len = str.GetLength();
		m_pMessageSprite->SetWindowRect(0.5f - (len * 0.01f), 0.45f, 
										0.5f + (len * 0.01f), 0.55f);
	}
#endif
	if (str != "" && fTime != 0.0f)
	{
		m_fMessageStart = vtGetTime();
		m_fMessageTime = fTime;
	}
	m_strMessage = str;
}

void Enviro::SetFlightSpeed(float speed)
{
	if (m_bTopDown && m_pOrthoFlyer != NULL)
		m_pOrthoFlyer->SetSpeed(speed);
	else if (m_pCurrentFlyer != NULL)
		m_pCurrentFlyer->SetSpeed(speed);
}

float Enviro::GetFlightSpeed()
{
	if (m_bTopDown && m_pOrthoFlyer != NULL)
		return m_pOrthoFlyer->GetSpeed();
	else if (m_pCurrentFlyer != NULL)
		return m_pCurrentFlyer->GetSpeed();
	else
		return 0.0f;
}

void Enviro::SetFlightAccel(bool bAccel)
{
	m_pCurrentFlyer->SetExag(bAccel);
}

bool Enviro::GetFlightAccel()
{
	return m_pCurrentFlyer->GetExag();
}

void Enviro::SetMode(MouseMode mode)
{
	VTLOG("SetMode %d\n", mode);

	if (m_pCursorMGeom)
	{
		switch (mode)
		{
		case MM_NAVIGATE:
			m_pCursorMGeom->SetEnabled(false);
			EnableFlyerEngine(true);
			break;
		case MM_SELECT:
		case MM_FENCES:
		case MM_ROUTES:
		case MM_PLANTS:
		case MM_INSTANCES:
		case MM_MOVE:
		case MM_MEASURE:
			m_pCursorMGeom->SetEnabled(true);
			EnableFlyerEngine(false);
			break;
		case MM_FLYROUTE:
			m_pCursorMGeom->SetEnabled(false);
			EnableFlyerEngine(false);
			break;
		}
	}
	m_bActiveFence = false;
	m_mode = mode;
}

void Enviro::SetRouteFollower(bool bOn)
{
	if (!m_pRouteFollower)
	{
		m_pRouteFollower = new RouteFollowerEngine(m_pCurRoute);
		m_pRouteFollower->SetTarget(vtGetScene()->GetCamera());
		vtGetScene()->AddEngine(m_pRouteFollower);
	}
	m_pRouteFollower->SetEnabled(bOn);
}

bool Enviro::GetRouteFollower()
{
	if (m_pRouteFollower)
		return m_pRouteFollower->GetEnabled();
	else
		return false;
}


void Enviro::SetTopDown(bool bTopDown)
{
	m_bTopDown = bTopDown;

	if (bTopDown)
		vtGetScene()->SetCamera(m_pTopDownCamera);
	else
		vtGetScene()->SetCamera(m_pNormalCamera);

	m_pOrthoFlyer->SetEnabled(bTopDown);
	EnableFlyerEngine(!bTopDown);
}

void Enviro::DumpCameraInfo()
{
	vtCamera *cam = m_pNormalCamera;
	FPoint3 pos = cam->GetTrans();
	FPoint3 dir;
	cam->GetDirection(dir);
	VTLOG("Camera: pos %f %f %f, dir %f %f %f\n",
		pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
}

void Enviro::SetSpeed(float x)
{
	if (!m_pGlobeTime)
		return;
	m_pGlobeTime->SetSpeed(x);
}

float Enviro::GetSpeed()
{
	if (!m_pGlobeTime)
		return 0.0f;
	return m_pGlobeTime->GetSpeed();
}

void Enviro::OnMouse(vtMouseEvent &event)
{
	// check for what is under the pickers
	DoPickers();

	// give the child classes first chance to take this event
	bool bCancel = OnMouseEvent(event);
	if (bCancel)
		return;

	if (event.type == VT_DOWN)
	{
		if (event.button == VT_LEFT)
		{
			if (m_state == AS_Terrain)
				OnMouseLeftDownTerrain(event);
			else if (m_state == AS_Orbit)
				OnMouseLeftDownOrbit(event);
		}
		else if (event.button == VT_RIGHT)
			OnMouseRightDown(event);
	}
	if (event.type == VT_MOVE)
	{
		OnMouseMove(event);
	}
	if (event.type == VT_UP)
	{
		if (event.button == VT_LEFT)
			OnMouseLeftUp(event);
		if (event.button == VT_RIGHT)
			OnMouseRightUp(event);
	}
}

void Enviro::OnMouseLeftDownTerrain(vtMouseEvent &event)
{
	if (!m_bOnTerrain)
		return;

	vtTerrain *pTerr = GetCurrentTerrain();

	// Build fences on click
	if (m_mode == MM_FENCES)
	{
		if (!m_bActiveFence)
		{
			start_new_fence();
			m_bActiveFence = true;
		}
		pTerr->AddFencepoint(m_pCurFence, DPoint2(m_EarthPos.x, m_EarthPos.y));
	}
	if (m_mode == MM_ROUTES)
	{
		if (!m_bActiveRoute)
		{
			start_new_route();
			m_bActiveRoute = true;
		}
		pTerr->add_routepoint_earth(m_pCurRoute,
			DPoint2(m_EarthPos.x, m_EarthPos.y), m_sStructType);
	}
	if (m_mode == MM_PLANTS)
	{
		// try planting a tree there
		VTLOG("Create a plant at %.2lf,%.2lf:", m_EarthPos.x, m_EarthPos.y);
		bool success = PlantATree(DPoint2(m_EarthPos.x, m_EarthPos.y));
		VTLOG(" %s.\n", success ? "yes" : "no");
	}
	if (m_mode == MM_INSTANCES)
		PlantInstance();

	if (m_mode == MM_SELECT)
		OnMouseLeftDownTerrainSelect(event);

	if (m_mode == MM_MEASURE)
	{
		m_EarthPosDown = m_EarthPos;
		m_bDragging = true;
		DPoint2 g1(m_EarthPosDown.x, m_EarthPosDown.y);
		SetTerrainMeasure(g1, g1);
		ShowDistance(g1, g1, 0, 0);
	}
}

void Enviro::OnMouseLeftDownTerrainSelect(vtMouseEvent &event)
{
	vtTerrain *pTerr = GetCurrentTerrain();

	// See if camera ray intersects a structure?  NO, it's simpler and
	//  easier for the user to just test whether the ground cursor is
	//  near a structure's origin.
	DPoint2 gpos(m_EarthPos.x, m_EarthPos.y);

	double dist1, dist2, dist3;
	vtStructureArray3d *structures = pTerr->GetStructures();
	structures->VisualDeselectAll();
	m_bSelectedStruct = false;

	int structure;		// index of closest structure
	bool result1 = pTerr->FindClosestStructure(gpos,
		g_Options.m_fSelectionCutoff, structure, dist1);
	vtStructureArray3d *structures_picked = pTerr->GetStructures();

	vtPlantInstanceArray3d &plants = pTerr->GetPlantInstances();
	plants.VisualDeselectAll();
	m_bSelectedPlant = false;

	int plant;		// index of closest plant
	bool result2 = plants.FindClosestPlant(gpos,
		g_Options.m_fSelectionCutoff, plant, dist2);

	vtRouteMap &routes = pTerr->GetRouteMap();
	m_bSelectedUtil = false;
	bool result3 = routes.FindClosestUtilNode(gpos,
		g_Options.m_fSelectionCutoff, m_pSelRoute, m_pSelUtilNode, dist3);

	bool click_struct = (result1 && dist1 < dist2 && dist1 < dist3);
	bool click_plant = (result2 && dist2 < dist1 && dist2 < dist3);
	bool click_route = (result3 && dist3 < dist1 && dist3 < dist2);

	if (click_struct)
	{
		vtStructure *str = structures_picked->GetAt(structure);
		vtStructure3d *str3d = structures_picked->GetStructure3d(structure);
		str->Select(true);
		str3d->ShowBounds(true);
		vtStructInstance *inst = str->GetInstance();
		if (inst != NULL && (event.flags & VT_SHIFT) != 0)
		{
			m_StartRotation = inst->m_fRotation;
			m_bRotating = true;
		}
		else
			m_bDragging = true;
		m_bSelectedStruct = true;

		if (structures_picked != structures)
		{
			// active structure set (layer) has changed due to picking
			ShowLayerView();
			RefreshLayerView();
		}
	}
	if (click_plant)
	{
		plants.VisualSelect(plant);
		m_bDragging = true;
		m_bSelectedPlant = true;
	}
	if (click_route)
	{
		m_bDragging = true;
		m_bSelectedUtil = true;
	}
	m_EarthPosDown = m_EarthPosLast = m_EarthPos;
	m_MouseDown = event.pos;
}

vtTerrain *Enviro::FindTerrainOnEarth(const DPoint2 &p)
{
	vtTerrain *t, *smallest = NULL;
	float diag, smallest_diag = 1E7;
	for (t = m_pTerrainScene->GetFirstTerrain(); t; t=t->GetNext())
	{
		if (t->m_Corners_geo.ContainsPoint(p))
		{
			// normally, doing comparison on latlon coordinates wouldn't be
			// meaningful, but in this case we know that the two areas compared
			// are overlapping and therefore numerically similar
			diag = (t->m_Corners_geo[2] - t->m_Corners_geo[0]).Length();
			if (diag < smallest_diag)
			{
				smallest_diag = diag;
				smallest = t;
			}
		}
	}
	return smallest;
}

void Enviro::OnMouseLeftDownOrbit(vtMouseEvent &event)
{
	// from orbit, check if we've clicked on a terrain
	if (!m_bOnTerrain)
		return;
	if (m_mode == MM_SELECT)
	{
		vtTerrain *pTerr = FindTerrainOnEarth(DPoint2(m_EarthPos.x, m_EarthPos.y));
		if (pTerr)
			SwitchToTerrain(pTerr);
	}
	if (m_mode == MM_MEASURE)
	{
		m_EarthPosDown = m_EarthPos;
		m_bDragging = true;
		UpdateEarthArc();
	}
}

void Enviro::OnMouseLeftUp(vtMouseEvent &event)
{
	m_bDragging = m_bRotating = false;

	if (m_state == AS_Orbit && m_mode == MM_MEASURE && m_bDragging)
		UpdateEarthArc();
}

void Enviro::OnMouseRightDown(vtMouseEvent &event)
{
}

void Enviro::OnMouseRightUp(vtMouseEvent &event) 
{
	if (m_state == AS_Terrain)
	{
		// close off the fence if we have one
		if (m_mode == MM_FENCES)
			close_fence();
		if (m_mode == MM_ROUTES)
			close_route();
		if (m_mode == MM_SELECT)
		{
			vtTerrain *t = GetCurrentTerrain();
			vtStructureArray3d *sa = t->GetStructures();
			vtPlantInstanceArray3d &plants = t->GetPlantInstances();

			if (sa->NumSelected() != 0 || plants.NumSelected() != 0)
				ShowPopupMenu(event.pos);
		}
	}
}

void Enviro::OnMouseMove(vtMouseEvent &event)
{
	if (m_state == AS_Terrain)
		OnMouseMoveTerrain(event);

	if (m_state == AS_Orbit && m_mode == MM_MEASURE && m_bDragging)
		UpdateEarthArc();
}

void Enviro::OnMouseMoveTerrain(vtMouseEvent &event)
{
	if (m_mode == MM_SELECT && (m_bDragging || m_bRotating))
	{
		DPoint3 delta = m_EarthPos - m_EarthPosLast;
		DPoint2 ground_delta(delta.x, delta.y);

		vtTerrain *pTerr = GetCurrentTerrain();
		if (m_bSelectedStruct)
		{
			vtStructureArray3d *structures = pTerr->GetStructures();
			if (m_bDragging)
				structures->OffsetSelectedStructures(ground_delta);
			else if (m_bRotating)
			{
				int sel = structures->GetFirstSelected();
				vtStructInstance *inst = structures->GetAt(sel)->GetInstance();
				vtStructInstance3d *str3d = structures->GetInstance(sel);

				inst->m_fRotation = m_StartRotation + (event.pos.x - m_MouseDown.x) / 100.0f;
				str3d->UpdateTransform(pTerr->GetHeightField());
			}
		}
		if (m_bDragging)
		{
			if (m_bSelectedPlant)
			{
				vtPlantInstanceArray3d &plants = pTerr->GetPlantInstances();
				plants.OffsetSelectedPlants(ground_delta);
			}
			if (m_bSelectedUtil)
			{
				vtRouteMap &routemap = pTerr->GetRouteMap();
				m_pSelUtilNode->Offset(ground_delta);
				m_pSelRoute->Dirty();
				routemap.BuildGeometry(pTerr->GetHeightField());
			}
		}

		m_EarthPosLast = m_EarthPos;
	}
	if (m_mode == MM_SELECT && m_pTerrainPicker != NULL)
	{
		vtTerrain *ter = GetCurrentTerrain();
		if (ter && ter->GetShowPOI())
		{
			ter->HideAllPOI();
			DPoint2 epos(m_EarthPos.x, m_EarthPos.y);
			vtPointOfInterest *poi = ter->FindPointOfInterest(epos);
			if (poi)
				ter->ShowPOI(poi, true);
		}
	}
	if (m_mode == MM_MEASURE && m_bDragging && m_bOnTerrain)
	{
//		VTLOG("MouseMove, MEASURE & Drag & OnTerrain: %.1lf, %.1lf\n", m_EarthPos.x, m_EarthPos.y);
		DPoint2 g1(m_EarthPosDown.x, m_EarthPosDown.y);
		DPoint2 g2(m_EarthPos.x, m_EarthPos.y);
		SetTerrainMeasure(g1, g2);
		ShowDistance(g1, g2, m_fArcLength, m_EarthPos.z - m_EarthPosDown.z);
	}
}

bool Enviro::GetEarthShading()
{
	return m_bEarthShade;
}

void Enviro::SetEarthShading(bool bShade)
{
	m_bEarthShade = bShade;

	vtMovLight *pMovLight = m_pTerrainScene->GetSunLight();

	pMovLight->SetEnabled(bShade);
	m_pIcoGlobe->SetLighting(bShade);
}

void Enviro::SetEarthShape(bool bFlat)
{
	m_bGlobeFlat = bFlat;
	if (m_bGlobeFlat)
		m_fFlattenDir = -0.03f;
	else
		m_fFlattenDir = 0.03f;
}

void Enviro::SetEarthTilt(bool bTilt)
{
	if (!m_pIcoGlobe)
		return;
	m_pIcoGlobe->SetSeasonalTilt(bTilt);

	// remind the earth of the time/date to refresh orientation
	m_pIcoGlobe->SetTime(m_pGlobeTime->GetTime());
}

bool Enviro::GetEarthTilt()
{
	if (!m_pIcoGlobe)
		return false;
	return m_pIcoGlobe->GetSeasonalTilt();
}

void Enviro::SetEarthUnfold(bool bUnfold)
{
	m_bGlobeUnfolded = bUnfold;
	if (m_bGlobeUnfolded)
	{
		// Enter Flat View
		FMatrix4 m4;
		m_pNormalCamera->GetTransform1(m4);
		m_SpaceLoc.FromMatrix(m4);

		m_FlatLoc.p.Set(0.85f,-0.75f,5.6);
		m_FlatLoc.q.Init();

//		m_pNormalCamera->SetTrans(m_FlatPos);
		m_pNormalCamera->PointTowards(FPoint3(0.85f,-0.75f,0));
		m_pTrackball->SetEnabled(false);

		// turn off axes
		SetSpaceAxes(false);

		// turn off globe shading and culling
		m_pIcoGlobe->SetCulling(false);
		SetEarthShading(false);

		m_fFoldDir = UNFOLD_SPEED;
	}
	else
	{
		if (m_fFolding == 1.0f)
			m_FlatLoc.p = m_pNormalCamera->GetTrans();
		m_fFoldDir = -UNFOLD_SPEED;

		m_pFlatFlyer->SetEnabled(false);
	}
}

void Enviro::UpdateEarthArc()
{
//	VTLOG("UpdateEarthArc %.1lf %.1lf,  %.1lf %.1lf\n", m_EarthPosDown.x, m_EarthPosDown.y, m_EarthPos.x, m_EarthPos.y);
	DPoint2 epos1(m_EarthPosDown.x, m_EarthPosDown.y);
	DPoint2 epos2(m_EarthPos.x, m_EarthPos.y);
	SetDisplayedArc(epos1, epos2);
	ShowDistance(epos1, epos2, FLT_MIN, FLT_MIN);
}

void Enviro::SetupArcMesh()
{
	if (!m_pArcMats)
	{
		m_pArcMats = new vtMaterialArray();
		m_pArcMats->AddRGBMaterial1(RGBf(1, 1, 0), false, false); // yellow
		m_pArcMats->AddRGBMaterial1(RGBf(1, 0, 0), false, false); // red
	}
	// create geometry container, if needed
	if (!m_pArc)
	{
		m_pArc = new vtGeom();
		if (m_state == AS_Orbit)
			m_pIcoGlobe->GetTop()->AddChild(m_pArc);
		else if (m_state == AS_Terrain)
			GetCurrentTerrain()->GetTopGroup()->AddChild(m_pArc);
		m_pArc->SetMaterials(m_pArcMats);
	}

	// re-create mesh if not the first time
	FreeArcMesh();
	// set the points of the arc
	m_pArcMesh = new vtMesh(GL_LINE_STRIP, 0, 20);
}

void Enviro::FreeArc()
{
	FreeArcMesh();
	if (m_pArc)
	{
		m_pArc->Release();
		m_pArc = NULL;
	}
}

void Enviro::FreeArcMesh()
{
	if (m_pArc && m_pArcMesh)
	{
		m_pArc->RemoveMesh(m_pArcMesh);
		m_pArcMesh->Release();
		m_pArcMesh = NULL;
	}
}

void Enviro::SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2)
{
	SetupArcMesh();

	double angle = m_pIcoGlobe->AddSurfaceLineToMesh(m_pArcMesh, g1, g2);

	// estimate horizontal distance (angle * radius)
	m_fArcLength = angle * EARTH_RADIUS;

	m_pArc->AddMesh(m_pArcMesh, 0);
	m_pArcMesh->Release();
}

void Enviro::SetTerrainMeasure(const DPoint2 &g1, const DPoint2 &g2)
{
	// place the arc for the distance measuring tool on the terrain
	SetupArcMesh();

	DLine2 dline;
	dline.Append(g1);
	dline.Append(g2);

	vtTerrain *pTerr = GetCurrentTerrain();
	m_fArcLength = pTerr->AddSurfaceLineToMesh(m_pArcMesh, dline, 10.0f);

	m_pArc->AddMesh(m_pArcMesh, 1);
	m_pArcMesh->Release();
}


////////////////////////////////////////////////////////////////
// Fences

void Enviro::start_new_fence()
{
	vtFence3d *fence = new vtFence3d(m_CurFenceType, m_fFenceHeight, m_fFenceSpacing);
	GetCurrentTerrain()->AddFence(fence);
	m_pCurFence = fence;

	// update count shown in layer view
	RefreshLayerView();
}

void Enviro::finish_fence()
{
	m_bActiveFence = false;
}

void Enviro::close_fence()
{
	if (m_bActiveFence && m_pCurFence)
	{
		DLine2 &pts = m_pCurFence->GetFencePoints();
		if (pts.GetSize() > 2)
		{
			DPoint2 FirstFencePoint = pts.GetAt(0);
			m_pCurFence->AddPoint(FirstFencePoint);
			GetCurrentTerrain()->RedrawFence(m_pCurFence);
		}
	}
	m_bActiveFence = false;
}

void Enviro::SetFenceOptions(FenceType type, float fHeight, float fSpacing)
{
	m_CurFenceType = type;
	m_fFenceHeight = fHeight;
	m_fFenceSpacing = fSpacing;
	finish_fence();
}


////////////////////////////////////////////////////////////////
// Route

void Enviro::start_new_route()
{
	vtRoute *route = new vtRoute(GetCurrentTerrain());
	GetCurrentTerrain()->AddRoute(route);
	m_pCurRoute = route;
}

void Enviro::finish_route()
{
	m_bActiveRoute = false;
}

void Enviro::close_route()
{
	if (m_bActiveRoute && m_pCurRoute)
	{
		GetCurrentTerrain()->SaveRoute();
	}
	m_bActiveRoute = false;
}

void Enviro::SetRouteOptions(const vtString &sStructType)
{
	m_sStructType = sStructType;
}


////////////////////////////////////////////////
// Plants

void Enviro::SetPlantOptions(PlantingOptions &opt)
{
	m_PlantOpt = opt;
	if (m_mode == MM_SELECT)
	{
		vtPlantInstanceArray3d &pia = GetCurrentTerrain()->GetPlantInstances();
		for (unsigned int i = 0; i < pia.GetSize(); i++)
		{
			vtPlantInstance3d *inst3d = pia.GetInstance3d(i);
			if (inst3d->IsSelected())
			{
				vtPlantInstance &inst = pia.GetAt(i);
				inst.species_id = opt.m_iSpecies;
				inst.size = opt.m_fHeight;
				pia.CreatePlantNode(i);
			}
		}
	}
}

/**
 * Plant a tree at the given location (in earth coordinates)
 */
bool Enviro::PlantATree(const DPoint2 &epos)
{
	if (!m_pPlantList)
		return false;

	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return false;

	// check distance from other plants
	vtPlantInstanceArray &pia = pTerr->GetPlantInstances();
	int size = pia.GetSize();
	double len, closest = 1E8;
	DPoint2 diff;

	bool bPlant = true;
	if (m_PlantOpt.m_fSpacing > 0.0f)
	{
		for (int i = 0; i < size; i++)
		{
			diff = epos - pia.GetAt(i).m_p;
			len = diff.Length();

			if (len < closest) closest = len;
		}
		if (closest < m_PlantOpt.m_fSpacing)
			bPlant = false;
		VTLOG(" closest plant %.2fm,%s planting..", closest, bPlant ? "" : " not");
	}
	if (!bPlant)
		return false;

	float height = m_PlantOpt.m_fHeight;
	float variance = m_PlantOpt.m_iVariance / 100.0f;
	height *= (1.0 + random(variance*2) - variance);
	if (!pTerr->AddPlant(epos, m_PlantOpt.m_iSpecies, height))
		return false;

	return true;
}


//// Instances

void Enviro::PlantInstance()
{
	vtString path = GetPathFromGUI();
	if (path == "")
		return;

	// create a new Instance object
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *structs = pTerr->GetStructures();
	vtStructInstance3d *inst = (vtStructInstance3d *) structs->NewInstance();
	inst->SetValue("filename", path);
	inst->m_p.Set(m_EarthPos.x, m_EarthPos.y);

	int index = structs->Append(inst);
	bool success = pTerr->CreateStructure(structs, index);
	if (success)
	{
		RefreshLayerView();
	}
	else
	{
		// creation failed
		inst->Select(true);
		structs->DeleteSelected();
		return;
	}
}

// this was a quick hack for the PW conference.  if we ever need a real
// 'logo' functionality, it should be re-done.
void Enviro::ToggleLogo()
{
	if (!logo) return;
	static int st = 1;

	st++;
	if (st == 3)
		st = 0;
	if (st == 0)
	{
		logo->SetEnabled(false);
		tg->SetEnabled(false);
	}
	if (st == 1)
	{
		logo->SetEnabled(true);
		tg->SetEnabled(false);
	}
	if (st == 2)
	{
		logo->SetEnabled(true);
		tg->SetEnabled(true);
	}
}

void Enviro::GetStatusText(vtString &str)
{
	vtScene *scene = vtGetScene();
	if (!scene) return;

	vtString str2;

	// get framerate
	float fps = scene->GetFrameRate();

	// only show 3 significant digits
	if (fps < 10)
		str.Format("fps %1.2f, ", fps);
	else if (fps < 80)
		str.Format("fps %2.1f, ", fps);
	else
		str.Format("fps %3.0f, ", fps);

	// get time of day
	TimeEngine *te = GetTerrainScene()->GetTimeEngine();
	if (te && te->GetEnabled())
	{
		int hr, min, sec;
		te->GetTime(hr, min, sec);

		str2.Format("time %02d:%02d:%02d, ", hr, min, sec);
		str += str2;
	}

	vtString vs;
	DescribeCoordinates(vs);
	str += vs;

	// get CLOD triangle counts, if appropriate
	DescribeCLOD(vs);
	str += vs;

	str += m_strMessage;
}

////////////////////////////////////////////////////////////////////////

void ControlEngine::Eval()
{
	Enviro::s_pApp->DoControl();
}

////////////////////////////////////////////////////////////////////////

vtTerrain *GetCurrentTerrain()
{
	return Enviro::s_pApp->m_pTerrainScene->GetCurrentTerrain();
}

vtTerrainScene *GetTerrainScene()
{
	return Enviro::s_pApp->m_pTerrainScene;
}
