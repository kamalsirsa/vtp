//
// class Enviro implementation
//
// Main functionality of the Enviro application
//
// Copyright (c) 2001 Virtual Terrain Project
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

#include "vtdata/boost/directory.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"

#include "Enviro.h"
#include "Options.h"
#include "Hawaii.h"
#include "Nevada.h"
#include "TransitTerrain.h"
#include "Globe.h"

#define ORTHO_HEIGHT		40000	// 40 km in the air
#define INITIAL_SPACE_DIST	3.1f

//
// This is a 'singleton', the only instance of the global application object
//
Enviro g_App;

extern void ShowPopupMenu(const IPoint2 &pos);

///////////////////////////////////////////////////////////

Enviro::Enviro()
{
	m_mode = MM_NONE;
	m_state = AS_Initializing;
	m_iInitStep = 0;

	m_bActiveFence = false;
	m_pCurFence = NULL;
	m_CurFenceType = FT_WIRE;
	m_fFenceHeight = 1.5f;
	m_fFenceSpacing = 2.5f;

	m_bOnTerrain = false;
	m_bShowTime = false;
	m_bGlobeFlat = false;
	m_fGlobeFac = 1.0f;
	m_fGlobeDir = 0.0f;

	m_pTerrainPicker = NULL;
	m_pGlobePicker = NULL;
	m_pCursorMGeom = NULL;

	m_pArc = NULL;
	m_pArcMesh = NULL;
	m_fArcLength = 0.0;

	m_fMessageTime = 0.0f;
	m_pPlantList = NULL;

	SetPlantOptions(0, 3.0f, 3.0f);

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
	VTLOG("\nEnviro\n\n");
}

void Enviro::Shutdown()
{
	delete m_pPlantList;
	delete m_pTerrainScene;
}

void Enviro::LoadTerrainDescriptions()
{
	VTLOG("LoadTerrainDescriptions\n");

	using namespace boost::filesystem;

	vtTerrain *pTerr;
	for (int i = 0; i < g_Options.m_DataPaths.GetSize(); i++)
	{
		vtString directory = *(g_Options.m_DataPaths[i]) + "Terrains";
		for (dir_it it((const char *)directory); it != dir_it(); ++it)
		{
			if (get<is_hidden>(it) || get<is_directory>(it))
				continue;

			std::string name1 = *it;
			vtString name = name1.c_str();

			// only look for ".ini" files
			if (name.GetLength() < 5 || name.Right(4).CompareNoCase(".ini"))
				continue;

			// Some terrain .ini files want to use a different Terrain class
			if (name == "Hawai`i.ini" || name == "Honoka`a.ini" || name == "Kealakekua.ini" )
				pTerr = new IslandTerrain();
			else if (name == "Nevada.ini")
				pTerr = new NevadaTerrain();
			else if (name == "TransitTerrain.ini")
				pTerr = new TransitTerrain();
			else
				pTerr = new vtTerrain();

			if (pTerr->SetParamFile(directory + "/" + name))
				m_pTerrainScene->AppendTerrain(pTerr);
		}
	}
}

void Enviro::StartControlEngine(const char *filename)
{
	VTLOG("StartControlEngine\n");
	m_bSuppliedFilename = (filename != NULL);

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
	if (m_state == AS_Orbit && m_fGlobeDir != 0.0f)
	{
		m_fGlobeFac += m_fGlobeDir;
		if (m_fGlobeDir > 0.0f && m_fGlobeFac > 1.0f)
		{
			m_fGlobeFac = 1.0f;
			m_fGlobeDir = 0.0f;
		}
		if (m_fGlobeDir < 0.0f && m_fGlobeFac < 0.0f)
		{
			m_fGlobeFac = 0.0f;
			m_fGlobeDir = 0.0f;
		}
		m_pIcoGlobe->SetInflation(m_fGlobeFac);
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
		if (m_pGlobeMGeom == NULL)
		{
			MakeGlobe();
			m_SpaceCamLocation.Identity();
			m_SpaceCamLocation.Translate(FPoint3(0.0f, 0.0f, INITIAL_SPACE_DIST));
		}
		SetMessage("Switching to Globe");
	}
	if (m_iInitStep == 3)
	{
		vtGetScene()->SetBgColor(RGBf(0.15f, 0.15f, 0.15f));

		m_pGlobeMGeom->SetEnabled(true);
		m_pRoot->AddChild(m_pGlobeMGeom);
		m_pCursorMGeom->Identity();
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
		time_t ltime;
		struct tm *gmt;
		time( &ltime );
		gmt = gmtime( &ltime );
		SetGlobeTime(gmt);

		SetShowTime(false);
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
	}
}

void Enviro::SetGlobeTime(struct tm *gmt)
{
	int minute = gmt->tm_min + 60 * gmt->tm_hour;
	float fraction_of_day = (float) minute / (24 * 60);
	float rotation = fraction_of_day * PI2f;

	// match with actual globe
	rotation = PI2f - rotation;
	rotation -= PID2f;

	vtMovLight *pSunLight = m_pTerrainScene->GetSunLight();
	pSunLight->Identity();
	pSunLight->SetTrans(FPoint3(5.0f, 0.0f, 0.0f));
	pSunLight->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), rotation);

	// standard bright sunlight
	pSunLight->GetLight()->SetColor2(RGBf(5.0f, 5.0f, 5.0f));
	pSunLight->GetLight()->SetAmbient2(RGBf(0.5f, 0.5f, 0.5f));
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
	if (m_state == AS_Orbit)
	{
		// hide globe
		if (m_pGlobeMGeom != NULL)
		{
			m_pGlobeMGeom->SetEnabled(false);
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
}

void Enviro::SetupTerrain(vtTerrain *pTerr)
{
	int iError;

	if (m_iInitStep == 1)
	{
		vtString str;
		str.Format("Creating Terrain '%s'", pTerr->GetName());
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
	}
	if (m_iInitStep == 8)
	{
		SetMessage("Setting hither/yon");
		vtCamera *pCam = vtGetScene()->GetCamera();
		pCam->SetHither(5.0f);
		pCam->SetYon(500000.0f);
	}
	if (m_iInitStep == 9)
	{
		SetupCameras();

		// "Finish" terrain scene
		VTLOG("Finishing Terrain Scene\n");
		m_pTerrainScene->Finish(g_Options.m_DataPaths);

		if (g_Options.m_bSpeedTest)
		{
			// Benchmark engine
			m_pBench = new BenchEngine("benchmark.loc");
			vtGetScene()->AddEngine(m_pBench);
		}
		else
		{
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
		if (m_fArcLength != 0.0)
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
// check whether there is terrain under either picker
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
		}
		else
			m_pSprite2->SetText("Not on terrain");
	}
}


//
// Create the earth globe
//
void Enviro::MakeGlobe()
{
	VTLOG("MakeGlobe\n");
#if 0
	// simple globe
	m_pGlobeMGeom = CreateSimpleEarth(g_Options.m_DataPaths);
#else
	// fancy icosahedral globe
	m_pIcoGlobe = new IcoGlobe();
	m_pIcoGlobe->Create(16, g_Options.m_DataPaths, g_Options.m_strImage);
	m_pGlobeMGeom = m_pIcoGlobe->m_mgeom;
#endif

	VTLOG("\tcreating Trackball\n");
	// use a trackball engine for navigation
	//
	m_pTrackball = new vtTrackball(INITIAL_SPACE_DIST);
	m_pTrackball->SetName2("Trackball2");
	m_pTrackball->SetTarget(vtGetScene()->GetCamera());
	vtGetScene()->AddEngine(m_pTrackball);

	// determine where the terrains are, and show them as red rectangles
	//
	LookUpTerrainLocations();
	AddTerrainRectangles();

	// create the GlobePicker engine for picking features on the earth
	//
	m_pGlobePicker = new GlobePicker();
	m_pGlobePicker->SetName2("GlobePicker");
	m_pGlobePicker->SetGlobeMGeom(m_pGlobeMGeom);
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
		pStars->Create(bsc_file, 10.0f, 5.0f);	// radius, brightness
		m_pGlobeMGeom->AddChild(pStars);
	}
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
		grid.GetProjection().GetTextDescription(msg1, msg2);
		VTLOG("\t\tprojection: type %s, value %s\n", msg1, msg2);

		DPoint2 sw, nw, ne, se;
		VTLOG("\t\tGetting terrain corners\n");
		grid.GetCorners(pTerr->m_Corners_geo, true);
		nw = pTerr->m_Corners_geo[1];
		se = pTerr->m_Corners_geo[3];
		VTLOG("\t\t(%.2lf,%.2lf) - (%.2lf,%.2lf)\n", nw.x, nw.y, se.x, se.y);
		VTLOG("\t\tGot terrain corners\n");
	}
	VTLOG("\tLookUpTerrainLocations: done\n");
}

void Enviro::AddTerrainRectangles()
{
	VTLOG("AddTerrainRectangles\n");
	m_pIcoGlobe->AddTerrainRectangles();
}

int Enviro::AddGlobePoints(const char *fname)
{
	return m_pIcoGlobe->AddGlobePoints(fname);
}


void Enviro::SetupScene1()
{
	VTLOG("SetupScene1\n");

	vtScene *pScene = vtGetScene();

	vtTerrain::SetDataPath(g_Options.m_DataPaths);

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

	if (g_Options.m_bQuakeNavigation)
		m_nav = NT_Quake;
	else if (g_Options.m_bGravity)
		m_nav = NT_Gravity;
	else
		m_nav = NT_Normal;

	// create picker object and picker engine
	float size = 1.0;
	m_pCursorMGeom = new vtMovGeom(Create3DCursor(size, size/30));
	m_pCursorMGeom->SetName2("Cursor");

	m_pTerrainScene->m_pTop->AddChild(m_pCursorMGeom);
	m_pTerrainPicker = new TerrainPicker();
	m_pTerrainPicker->SetName2("TerrainPicker");
	vtGetScene()->AddEngine(m_pTerrainPicker);

	m_pTerrainPicker->SetTarget(m_pCursorMGeom);
	m_pTerrainPicker->SetEnabled(false); // turn off at startup

	m_pSprite2 = new vtSprite();
	m_pSprite2->SetName2("Sprite2");
	m_pSprite2->SetWindowRect(0.73f, 0.90f, 1.00f, 1.00f);
	m_pSprite2->SetText("...");
	m_pRoot->AddChild(m_pSprite2);

	vtFence3d::SetScale(g_Options.m_fPlantScale);

	vtPlantList pl;
	vtString species_path = FindFileOnPaths(g_Options.m_DataPaths, "PlantData/species.xml");
	if (species_path != "" && pl.ReadXML(species_path))
	{
		m_pPlantList = new vtPlantList3d();
		*m_pPlantList = pl;
		m_pPlantList->CreatePlantSurfaces(g_Options.m_DataPaths,
			g_Options.m_fPlantScale, g_Options.m_bShadows != 0, true);
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
		if (m_nav == NT_Quake)
			SetCurrentNavigator(m_pQuakeFlyer);
		if (m_nav == NT_Gravity)
			SetCurrentNavigator(m_pVFlyer);
		if (m_nav == NT_Normal)
			SetCurrentNavigator(m_pTFlyer);
	}
	else
		SetCurrentNavigator(NULL);
}

extern void SetTerrainToGUI(vtTerrain *pTerrain);

void Enviro::SetTerrain(vtTerrain *pTerrain)
{
	VTLOG("Enviro::SetTerrain '%s'\n", (const char *) pTerrain->GetName());

	// safety check
	if (!pTerrain)
		return;
	vtHeightField *pHF = pTerrain->GetHeightField();
	if (!pHF)
		return;

	m_pTerrainScene->SetTerrain(pTerrain);

	TParams &param = pTerrain->GetParams();

	EnableFlyerEngine(true);

	// inform the navigation engine of the new terrain
	m_pCurrentFlyer->SetTarget(m_pNormalCamera);
	m_pCurrentFlyer->SetHeightField(pHF);
	m_pCurrentFlyer->SetHeight(param.m_iMinHeight);
	m_pCurrentFlyer->SetSpeed(param.m_fNavSpeed);
	m_pCurrentFlyer->SetEnabled(true);

#if 1
	// set the top-down viewpoint to a point over
	//  the center of the new terrain
	FPoint3 middle;
	pHF->GetCenter(middle);
	middle.y = ORTHO_HEIGHT;
	m_pTopDownCamera->SetTrans(middle);
	m_pTopDownCamera->RotateLocal(TRANS_XAxis, -PID2f);
	m_pTopDownCamera->SetHither(0.1f);
	m_pTopDownCamera->SetYon(1000.0f);
//	m_pTopDownCamera->SetHeight(ORTHO_HEIGHT);
#endif

	if (m_pTerrainPicker != NULL)
		m_pTerrainPicker->SetHeightField(pHF);

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
	VTLOG("  SetMessage: %s\n", msg);

	vtString str = msg;

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
		if (fTime != 0.0f)
		{
			m_fMessageStart = vtGetTime();
			m_fMessageTime = fTime;
		}
	}
	m_strMessage = str;
}

void Enviro::SetFlightSpeed(float speed)
{
	if (m_pCurrentFlyer != NULL)
		m_pCurrentFlyer->SetSpeed(speed);
}

float Enviro::GetFlightSpeed()
{
	if (m_pCurrentFlyer != NULL)
		return m_pCurrentFlyer->GetSpeed();
	else
		return 0.0f;
}

void Enviro::SetMode(MouseMode mode)
{
	VTLOG("SetMode %d\n", mode);

	if (m_pCursorMGeom)
	{
		switch (mode)
		{
		case MM_SELECT:
		case MM_FENCES:
		case MM_ROUTES:
		case MM_PLANTS:
		case MM_MOVE:
		case MM_LINEAR:
			m_pCursorMGeom->SetEnabled(true);
			break;
		default:
			m_pCursorMGeom->SetEnabled(false);
			break;
		}
	}

	m_bActiveFence = false;
	m_mode = mode;
}

void Enviro::SetupCameras()
{
	VTLOG("SetupCameras: Setting up normal camera\n");
	m_pNormalCamera = vtGetScene()->GetCamera();

#if 1
	// Create second camera (for Top-Down view)
	if (m_pTopDownCamera == NULL)
	{
		VTLOG("Creating Top-Down Camera\n");
		m_pTopDownCamera = new vtCamera();
		m_pTopDownCamera->SetOrtho(10000.0f);
		m_pTopDownCamera->SetName2("Top-Down Camera");
	}
#endif

	// Set up a camera for the route (if any)
	if (m_pRouteFollowerCamera == NULL)
	{
		m_pRouteFollowerCamera = vtGetScene()->GetCamera();
	}

	if (m_pTFlyer != NULL)
	{
		m_pTFlyer->SetTarget(m_pNormalCamera);
		m_pTFlyer->SetName2("Terrain Flyer");
	}
}


void Enviro::SetTopDown(bool bTopDown)
{
	if (bTopDown)
	{
		vtGetScene()->SetCamera(m_pTopDownCamera);
		m_pCurrentFlyer->SetTarget(m_pTopDownCamera);
		m_pCurrentFlyer->FollowTerrain(false);
	}
	else
	{
		vtGetScene()->SetCamera(m_pNormalCamera);
		m_pCurrentFlyer->SetTarget(m_pNormalCamera);
		m_pCurrentFlyer->FollowTerrain(true);
	}
}

void Enviro::OnMouse(vtMouseEvent &event)
{
	// check for what is under the pickers
	DoPickers();

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
		OnMouseMove(event);
	if (event.type == VT_UP)
	{
		if (event.button == VT_LEFT)
			m_bDragging = false;
		if (event.button == VT_RIGHT)
			OnMouseRightUp(event);
	}
}

void Enviro::OnMouseLeftDownTerrain(vtMouseEvent &event)
{
	vtTerrain *pTerr = GetCurrentTerrain();

	// Build fences on click
	if (m_bOnTerrain && m_mode == MM_FENCES)
	{
		if (!m_bActiveFence)
		{
			start_new_fence();
			m_bActiveFence = true;
		}
		pTerr->AddFencepoint(m_pCurFence, DPoint2(m_EarthPos.x, m_EarthPos.y));
	}
	if (m_bOnTerrain && m_mode == MM_ROUTES)
	{
		if (!m_bActiveRoute)
		{
			start_new_route();
			m_bActiveRoute = true;
		}
		GetCurrentTerrain()->add_routepoint_earth(m_pCurRoute,
			DPoint2(m_EarthPos.x, m_EarthPos.y), m_sStructType);
	}
	if (m_bOnTerrain && m_mode == MM_PLANTS)
	{
		// try planting a tree there
		PlantATree(DPoint2(m_EarthPos.x, m_EarthPos.y));
	}
	if (m_bOnTerrain && m_mode == MM_SELECT)
	{
		// See if camera ray intersects a structure?  NO, it's simpler
		//  to just test whether the ground cursor is near a structure
		DPoint2 gpos(m_EarthPos.x, m_EarthPos.y);

		double dist1, dist2, dist3;
		vtStructureArray3d &structures = pTerr->GetStructures();
		structures.VisualDeselectAll();
		m_bSelectedStruct = false;

		int structure;		// index of closest structure
		bool result1 = structures.FindClosestStructure(gpos, 20.0,
			structure, dist1);

		vtPlantInstanceArray3d &plants = GetCurrentTerrain()->GetPlantInstances();
		plants.VisualDeselectAll();
		m_bSelectedPlant = false;

		int plant;		// index of closest plant
		bool result2 = plants.FindClosestPlant(gpos, 20.0, plant, dist2);

		vtRouteMap &routes = GetCurrentTerrain()->GetRouteMap();
		m_bSelectedUtil = false;
		bool result3 = routes.FindClosestUtilNode(gpos, 20.0, m_pSelRoute,
			m_pSelUtilNode, dist3);

		bool click_struct = (result1 && dist1 < dist2 && dist1 < dist3);
		bool click_plant = (result2 && dist2 < dist1 && dist2 < dist3);
		bool click_route = (result3 && dist3 < dist1 && dist3 < dist2);

		if (click_struct)
		{
			vtStructure3d *str = structures.GetStructure3d(structure);
			structures.VisualSelect(str);
			m_bDragging = true;
			m_bSelectedStruct = true;
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
	}
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
	if (m_mode == MM_LINEAR)
	{
		m_EarthPosDown = m_EarthPos;
		m_bDragging = true;
	}
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
			vtTerrain *pTerr = GetCurrentTerrain();
			vtStructureArray3d &structures = pTerr->GetStructures();

			if (structures.NumSelected() != 0)
				ShowPopupMenu(event.pos);
		}
	}
}

void Enviro::OnMouseMove(vtMouseEvent &event)
{
	if (m_state == AS_Terrain && m_mode == MM_SELECT && m_bDragging)
	{
		DPoint3 delta = m_EarthPos - m_EarthPosLast;
		DPoint2 ground_delta(delta.x, delta.y);

		vtTerrain *pTerr = GetCurrentTerrain();
		if (m_bSelectedStruct)
		{
			vtStructureArray3d &structures = pTerr->GetStructures();
			structures.OffsetSelectedStructures(ground_delta);
		}
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
	if (m_state == AS_Orbit && m_mode == MM_LINEAR && m_bDragging)
	{
		DPoint2 epos1(m_EarthPosDown.x, m_EarthPosDown.y);
		DPoint2 epos2(m_EarthPos.x, m_EarthPos.y);
		SetDisplayedArc(epos1, epos2);
	}
}

bool Enviro::GetShowTime()
{
	return m_bShowTime;
}

void Enviro::SetShowTime(bool bShow)
{
	m_bShowTime = bShow;

	vtMovLight *pMovLight = m_pTerrainScene->GetSunLight();

	pMovLight->SetEnabled(bShow);
	m_pIcoGlobe->SetLighting(bShow);
}

void Enviro::SetEarthShape(bool bFlat)
{
	m_bGlobeFlat = bFlat;
	if (m_bGlobeFlat)
		m_fGlobeDir = -0.03f;
	else
		m_fGlobeDir = 0.03f;
}

void Enviro::SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2)
{
	// first determine how many points we should use for a smooth arc
	DPoint3 p1, p2;
	geo_to_xyz(1.0, g1, p1);
	geo_to_xyz(1.0, g2, p2);
	double angle = acos(p1.Dot(p2));
	int points = (int) (angle * 3000);
	if (points < 3)
		points = 3;

	// calculate the axis of rotation
	DPoint3 cross = p1.Cross(p2);
	cross.Normalize();
	double angle_spacing = angle / (points-1);
	DMatrix4 rot4;
	rot4.AxisAngle(cross, angle_spacing);
	DMatrix3 rot3;
	rot3.SetByMatrix4(rot4);

	// estimate horizontal distance (angle * radius)
	m_fArcLength = angle * EARTH_RADIUS;

	// create geometry container
	if (!m_pArc)
	{
		m_pArc = new vtGeom();
		m_pGlobeMGeom->AddChild(m_pArc);
		vtMaterialArray *pMats = new vtMaterialArray();
		int yellow = pMats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 0.0f),	// yellow
						 false, false, false);
		m_pArc->SetMaterials(pMats);
	}
	// re-create mesh if not the first time
	if (m_pArcMesh)
	{
		m_pArc->RemoveMesh(m_pArcMesh);
		delete m_pArcMesh;
	}
	// set the points of the arc
	m_pArcMesh = new vtMesh(GL_LINE_STRIP, 0, points);
	for (int i = 0; i < points; i++)
	{
		FPoint3 fp = p1 * 1.0002;
		m_pArcMesh->AddVertex(fp);
		rot3.Transform(p1, p2);
		p1 = p2;
	}
	m_pArcMesh->AddStrip2(points, 0);
	m_pArc->AddMesh(m_pArcMesh, 0);
}

////////////////////////////////////////////////////////////////
// Fences

void Enviro::start_new_fence()
{
	vtFence3d *fence = new vtFence3d(m_CurFenceType, m_fFenceHeight, m_fFenceSpacing);
	GetCurrentTerrain()->AddFence(fence);
	m_pCurFence = fence;
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

//
// Plant a tree at the given (utm) location
//
void Enviro::PlantATree(const DPoint2 &epos)
{
	if (!m_pPlantList)
		return;

	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	// check distance from other plants
	vtPlantInstanceArray &pia = pTerr->GetPlantInstances();
	int size = pia.GetSize();
	double len, closest = 1E8;
	DPoint2 diff;

	for (int i = 0; i < size; i++)
	{
		diff = epos - pia.GetAt(i).m_p;
		len = diff.Length();

		if (len < closest) closest = len;
	}
	if (closest > m_fPlantSpacing)
		pTerr->AddPlant(epos, m_iSpecies, m_fPlantSize);
}

void Enviro::SetPlantOptions(int iSpecies, float fSize, float fSpacing)
{
	m_iSpecies = iSpecies;
	m_fPlantSize = fSize;
	m_fPlantSpacing = fSpacing;
}

////////////////////////////////////////////////////////////////////////

void ControlEngine::Eval()
{
	g_App.DoControl();
}

////////////////////////////////////////////////////////////////////////

vtTerrain *GetCurrentTerrain()
{
	return g_App.m_pTerrainScene->GetCurrentTerrain();
}

vtTerrainScene *GetTerrainScene()
{
	return g_App.m_pTerrainScene;
}


