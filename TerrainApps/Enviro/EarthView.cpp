//
// Earth View functionality of class Enviro
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/Globe.h"
#include "vtlib/core/SkyDome.h"
#include "vtdata/vtLog.h"

#include "Enviro.h"
#include "Engines.h"
#include "Options.h"

#define SPACE_DARKNESS		0.0f
#define UNFOLD_SPEED		0.01f

// these define the vertex count and height offset of the lon-lat lines
#define LL_COUNT	640
#define LL_RADIUS	1.002

extern int pwdemo;
vtGeom *tg = NULL;


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
	SetCurrentTerrain(NULL);
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
		vtMovLight *pSunLight = GetSunLight();
		pSunLight->Identity();
		pSunLight->SetTrans(FPoint3(0, 0, -5));

if (pwdemo){
		pSunLight->m_pLight->SetDiffuse(RGBf(1, 1, 1));
		pSunLight->m_pLight->SetAmbient(RGBf(0, 0, 0));
}else{
		// standard bright sunlight
		pSunLight->m_pLight->SetDiffuse(RGBf(3, 3, 3));
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
		if (!strncmp((const char *) g_Options.m_strEarthImage, "geosphere", 9))
			SetMessage("Earth image (c) The GeoSphere Project", 3);
		else
			SetMessage("Earth View", 10);
		m_pGlobePicker->SetEnabled(true);

		// Layer view needs to update
		RefreshLayerView();

		// Let the GUI know
		SetTimeEngineToGUI(m_pGlobeTime);
	}
}

// Helper
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
	m_pIcoGlobe->Create(5000, g_Options.m_DataPaths, g_Options.m_strEarthImage,
//		IcoGlobe::GEODESIC);
//		IcoGlobe::RIGHT_TRIANGLE);
		IcoGlobe::DYMAX_UNFOLD);
	m_pGlobeContainer->AddChild(m_pIcoGlobe->GetTop());
	m_pGlobeTime->AddTarget((TimeTarget *)m_pIcoGlobe);

if (pwdemo){
/*	logo = new vtGroup;
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
	vtTime time;
	time.SetDate(2000, 6, 20);
	time.SetTimeOfDay(15,0,0);
	m_pGlobeTime->SetTime(time);

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

	vtFeatureLoader loader;
	vtFeatureSet *feat1 = loader.LoadFromSHP("../Data/PointData/vtp-users-020519.shp");
	vtFeatureSetPoint2D *ft = (vtFeatureSetPoint2D *) feat1;

	int half = ft->GetNumEntities() / 2;
	int foo = 0;
	for (i = 0; i < half; i++)
	{
		DPoint2 p1 = ft->GetPoint(i);
		DPoint2 p2 = ft->GetPoint(i+half);
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
*/
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
	m_pIcoGlobe->AddTerrainRectangles(vtGetTS());

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
		pStars->Create(bsc_file, 5.0f);	// brightness
		vtTransform *pScale = new vtTransform();
		pScale->SetName2("Star Scaling Transform");
		pScale->Scale3(20, 20, 20);
		m_pGlobeContainer->AddChild(pScale);
		pScale->AddChild(pStars);
	}

	// create some geometry showing various astronomical axes
	vtMaterialArray *pMats = new vtMaterialArray();
	int yellow = pMats->AddRGBMaterial1(RGBf(1,1,0), false, false);
	int red = pMats->AddRGBMaterial1(RGBf(1,0,0), false, false);
	int green = pMats->AddRGBMaterial1(RGBf(0,1,0), false, false);

	m_pSpaceAxes = new vtGeom();
	m_pSpaceAxes->SetName2("Earth Axes");
	m_pSpaceAxes->SetMaterials(pMats);
	pMats->Release();	// pass ownership

	vtMesh *mesh = new vtMesh(GL_LINES, 0, 6);
	mesh->AddLine(FPoint3(0,0,200), FPoint3(0,0,0));
	mesh->AddLine(FPoint3(0,0,1),   FPoint3(-.07f,0,1.1f));
	mesh->AddLine(FPoint3(0,0,1),   FPoint3( .07f,0,1.1f));
	m_pSpaceAxes->AddMesh(mesh, yellow);
	mesh->Release();	// pass ownership

	mesh = new vtMesh(GL_LINES, 0, 6);
	mesh->AddLine(FPoint3(1.5f,0,0), FPoint3(-1.5f,0,0));
	mesh->AddLine(FPoint3(-1.5f,0,0), FPoint3(-1.4f, 0.07f,0));
	mesh->AddLine(FPoint3(-1.5f,0,0), FPoint3(-1.4f,-0.07f,0));
	m_pSpaceAxes->AddMesh(mesh, green);
	mesh->Release();	// pass ownership

	mesh = new vtMesh(GL_LINES, 0, 6);
	mesh->AddLine(FPoint3(0,2,0), FPoint3(0,-2,0));
	m_pSpaceAxes->AddMesh(mesh, red);
	mesh->Release();	// pass ownership

	m_pGlobeContainer->AddChild(m_pSpaceAxes);
	m_pSpaceAxes->SetEnabled(false);

	// Lon-lat cursor lines
	m_pEarthLines = new vtGeom();
	m_pEarthLines->SetName2("Earth Lines");
	int orange = pMats->AddRGBMaterial1(RGBf(1,.7,1), false, false, true, 0.6);
	m_pEarthLines->SetMaterials(pMats);

	m_pLineMesh = new vtMesh(GL_LINE_STRIP, 0, 6);
	for (int i = 0; i < LL_COUNT*3; i++)
		m_pLineMesh->AddVertex(FPoint3(0,0,0));
	m_pLineMesh->AddStrip2(LL_COUNT*2, 0);
	m_pLineMesh->AddStrip2(LL_COUNT, LL_COUNT*2);
	m_pLineMesh->AllowOptimize(false);

	m_pEarthLines->AddMesh(m_pLineMesh, orange);
	m_pLineMesh->Release();	// pass ownership
	m_pIcoGlobe->GetSurface()->AddChild(m_pEarthLines);
	m_pEarthLines->SetEnabled(false);

	double lon = 14.1;
	double lat = 37.5;
	SetEarthLines(lon, lat);
}

void Enviro::SetEarthLines(double lon, double lat)
{
	int i;
	DPoint2 p;
	FPoint3 p3;

	// lat line
	p.y = lat;
	for (i = 0; i < LL_COUNT+LL_COUNT; i++)
	{
		p.x = (double)i / (LL_COUNT+LL_COUNT-1) * 360;
		geo_to_xyz(LL_RADIUS, p, p3);
		m_pLineMesh->SetVtxPos(i, p3);
	}
	// lon line
	p.x = lon;
	for (i = 0; i < LL_COUNT; i++)
	{
		p.y = -90 + (double)i / (LL_COUNT-1) * 180;
		geo_to_xyz(LL_RADIUS, p, p3);
		m_pLineMesh->SetVtxPos(LL_COUNT+LL_COUNT+i, p3);
	}
}

void Enviro::ShowEarthLines(bool bShow)
{
	if (m_pEarthLines)
		m_pEarthLines->SetEnabled(bShow);
}

void Enviro::SetSpaceAxes(bool bShow)
{
	if (m_state == AS_Orbit)
	{
		if (m_pSpaceAxes)
			m_pSpaceAxes->SetEnabled(bShow);
		if (m_pIcoGlobe)
			m_pIcoGlobe->ShowAxis(bShow);
	}
	else if (m_state == AS_Terrain)
	{
		vtGetTS()->GetSkyDome()->ShowMarkers(bShow);
	}
}

bool Enviro::GetSpaceAxes()
{
	if (m_state == AS_Orbit)
	{
		if (m_pSpaceAxes)
			return m_pSpaceAxes->GetEnabled();
	}
	else if (m_state == AS_Terrain)
	{
		return vtGetTS()->GetSkyDome()->MarkersShown();
	}
	return false;
}

void Enviro::LookUpTerrainLocations()
{
	VTLOG("LookUpTerrainLocations\n");

	// look up the earth location of each known terrain
	for (unsigned int i = 0; i < NumTerrains(); i++)
	{
		vtTerrain *pTerr = GetTerrain(i);
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

void Enviro::DoControlOrbit()
{
	if (m_fFlattenDir != 0.0f)
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
	if (m_fFoldDir != 0.0f)
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


//
// Check the terrain under the 3D cursor (for Earth View).
//
void Enviro::DoCursorOnEarth()
{
	m_bOnTerrain = false;
	DPoint3 earthpos;

	if (m_pGlobePicker != NULL)
		m_bOnTerrain = m_pGlobePicker->GetCurrentEarthPos(earthpos);
	if (m_bOnTerrain)
	{
		m_EarthPos = earthpos;

		// Update Earth Lines
		SetEarthLines(m_EarthPos.x, m_EarthPos.y);
	}
	vtString str1, str2;
	FormatCoordString(str1, m_EarthPos, LU_DEGREES);
	str2 = "Cursor ";
	str2 += str1;
	m_pSprite2->SetText(str2);
}


vtTerrain *Enviro::FindTerrainOnEarth(const DPoint2 &p)
{
	vtTerrain *t, *smallest = NULL;
	float diag, smallest_diag = 1E7;

	for (unsigned int i = 0; i < NumTerrains(); i++)
	{
		t = GetTerrain(i);
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

bool Enviro::GetEarthShading()
{
	return m_bEarthShade;
}

void Enviro::SetEarthShading(bool bShade)
{
	m_bEarthShade = bShade;

	vtMovLight *pMovLight = GetSunLight();

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

void Enviro::SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2)
{
	SetupArcMesh();

	double angle = m_pIcoGlobe->AddSurfaceLineToMesh(m_pArcMesh, g1, g2);

	// estimate horizontal distance (angle * radius)
	m_fArcLength = angle * EARTH_RADIUS;

	m_pArc->AddMesh(m_pArcMesh, 0);
	m_pArcMesh->Release();		// Pass ownership
}

void Enviro::DescribeCoordinatesEarth(vtString &str)
{
	DPoint3 epos;
	vtString str1;

	// give location of 3D cursor
	str = "Cursor: ";
	m_pGlobePicker->GetCurrentEarthPos(epos);
	FormatCoordString(str1, epos, LU_DEGREES);
	str += str1;
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

