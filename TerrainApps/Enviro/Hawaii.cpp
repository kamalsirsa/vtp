//
// Hawai`i.cpp
//
// Terrain implementation specific to the Big Island of Hawai`i.
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/DynTerrain.h"
#include "Hawaii.h"
#include "Nevada.h"


///////////////////////////////

IslandTerrain::IslandTerrain() : PTerrain()
{
	// Points of Interest
	// top left (x,y) then bottom right (x,y)

	AddPointOfInterest(283376, 2182614, 287025, 2181205, "Hilo Airport", "http://www.hawaii.gov/dot/hilo.htm");
	AddPointOfInterest(240459, 2202455, 244033, 2188779, "Mauna Kea", "/Html/maunakea.htm");
	AddPointOfInterest(226689, 2157368, 227719, 2153045, "Mauna Loa", "http://www.yahoo.com");
	AddPointOfInterest(188873, 2153990, 195080, 2156953, "Kealakekua bay", "/Html/kealakekua.htm");
	AddPointOfInterest(253500, 2103500, 282500, 2080000, "Lo`ihi", "/Html/loihi_seamount.htm");
}

IslandTerrain::~IslandTerrain()
{
}

void IslandTerrain::create_telescopes()
{
#if 0
	// This works, but overrides the structures indicated in TParams.
	// We can do something like this once we have support for multiple
	// structure layers.
	CreateStructuresFromXML(m_strDataPath + "BuildingData/tscope_loc.vtst");
#elif 1
	vtStructureArray3d sa;
	vtString fname = FindFileOnPaths(m_DataPaths, "BuildingData/tscope_loc.vtst");
	if (fname == "")
		return;
	bool success = sa.ReadXML(fname);
	if (!success)
		return;

	int num_structs = sa.GetSize();
	sa.SetHeightField(m_pHeightField);
	for (int i = 0; i < num_structs; i++)
	{
		vtStructure3d *str3d = sa.GetStructure3d(i);

		success = sa.ConstructStructure(str3d);
		if (!success)
			continue;

		vtTransform *pTrans = str3d->GetTransform();
		if (pTrans)
			m_pLodGrid->AppendToGrid(pTrans);
	}
#endif
}


vtGeom *make_test_cone()
{
	vtMaterialArray *looks = new vtMaterialArray();
	looks->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f), false);

	////////////
	int res = 40;
	vtMesh *pMesh = new vtMesh(GL_TRIANGLE_STRIP, VT_Normals, res*res);

	FPoint3 tip(0, 2000, 0);
	double cone_radius = PId/4;
	double theta1 = PId * 1.3;
	double theta2 = PId * 1.8;
	double r1 = 700;
	double r2 = 1500;

	pMesh->CreateConicalSurface(tip, cone_radius, theta1, theta2, r1, r2, res);

	vtGeom *pGeom = new vtGeom();
	pGeom->SetMaterials(looks);
	pGeom->AddMesh(pMesh, 0);
	/////////////

	return pGeom;
}

vtGeom *IslandTerrain::make_red_cube()
{
	vtGeom *thebox = new vtGeom();
	float ws = 100.0f;	// convert 100m to 3d system units

	//code to make it a Shape
	vtMesh *mesh = new vtMesh(GL_TRIANGLE_FAN, VT_Normals, 24);
	FPoint3 size(ws, ws, ws);
	FPoint3 half = size / 2;	// Block() will double the size
	mesh->CreateBlock(half);

	vtMaterialArray *looks = new vtMaterialArray();
	looks->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f), true);
	thebox->SetMaterials(looks);
	thebox->AddMesh(mesh, 0);

	return thebox;
}

void IslandTerrain::create_airports()
{
#if 0
	vtTransform *ITO = LoadModel("Culture/ITO.dsm");
	vtTransform *KOA = LoadModel("Culture/KOA.dsm");
	vtTransform *MUE = LoadModel("Culture/MUE.dsm");

	if (ITO)
	{
		PlantModelAtPoint(ITO, DPoint2(283575, 2181163();
 		AddNode(ITO);
	}
	if (KOA)
	{
		PlantModelAtPoint(KOA, DPoint2(180290, 2184990));
 		AddNode(KOA);
	}
	if (MUE)
	{
		PlantModelAtPoint(MUE, DPoint2(219990, 2213505));
 		AddNode(MUE);
	}

/*	float fSizeExag = m_Params.m_fVehicleSize, fSpeedExag = m_Params.m_fVehicleSpeed;

	for (int i = 0; i < 3; i++)
	{
		vtTransform *copy = CreateVehicle("747", RGBf(1.0f, 1.0f, 0.0f));
		AddNode(copy);

		PlaneEngine *pEng = new PlaneEngine(fSizeExag, fSpeedExag, ITO);
		pEng->SetTarget(copy);
		pEng->SetHoop(i*2);
		AddEngine(pEng, pScene);
	}
*/
#endif
}


void IslandTerrain::CreateCustomCulture(bool bDoSound)
{
	DPoint2 bound[7], c1;
	bound[0].Set(237257, 2219644);
	bound[1] = bound[0] + DPoint2(0.0, -96.64);
	bound[2] = bound[1] + DPoint2(82.5, 0.0);
	bound[3] = bound[1] + DPoint2(178.2, 0.0);
	bound[4] = bound[3] + DPoint2(0.0, 30.48);
	bound[5] = bound[4] + DPoint2(-178.2 + 37.44f, 0.0);
	bound[6] = bound[5] + DPoint2(0.0, 96.64f - 30.48);
	c1 = bound[2] + DPoint2(0.0, 12.2);

#if 0
	if (m_Params.m_bBuildings)
	{
		//import the lighthouses
		vtTransform *lighthouse1 = LoadModel("BuildingModels/mahukonalthse.dsm");
		if (lighthouse1)
		{
			// scale was one unit = 1 m
			// plant it on the ground
			PlantModelAtPoint(lighthouse1, DPoint2(197389, 2230283));

			AddNodeToLodGrid(lighthouse1);
		}
	}
#endif

	//  8' =  2.4385 m
	// 12' =  3.6576 m
	// 28' =  8.5344 m
	// 36' = 10.9728 m
	// 40' = 12.1920 m
	// 48' = 14.6304 m
	// test dynamic creation of a complicated building
	if (1)
	{
		vtStructureArray3d *pSA = new vtStructureArray3d();
		pSA->m_proj = m_proj;

		vtBuilding3d *bld = (vtBuilding3d *) pSA->NewBuilding();

		DPoint2 c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12;
		c2 = c1 + DPoint2(10.9728, 0.0);
		c3 = c1 + DPoint2(14.6304, 0.0);
		c4 = c1 + DPoint2(14.6304, 12.1920);
		c5 = c1 + DPoint2(10.9728, 12.1920);
		c6 = c1 + DPoint2(0.0, 12.1920);
		c7 = c1 + DPoint2(3.6576, 0.0);
		c8 = c1 + DPoint2(3.6576, 3.6576);
		c9 = c1 + DPoint2(0.0, 3.6576);
		c10 = c1 + DPoint2(10.9728, 3.6576);
		c11 = c1 + DPoint2(10.9728, 8.534);
		c12 = c1 + DPoint2(3.6576, 8.534);
		DLine2 dl;
		vtLevel *pLev;
		vtEdge *pEdge;

		// basement/garage level (0)
		dl.Append(c2);
		dl.Append(c3);
		dl.Append(c4);
		dl.Append(c5);
		pLev = bld->CreateLevel(dl);
		pLev->m_fStoryHeight = 2.4385f;
		pLev->m_iStories = 1;

		pEdge = pLev->m_Edges[0];
		pLev->SetEdgeColor(RGBi(128, 128, 128));
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_GAP);

		pEdge = pLev->m_Edges[1];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_WALL, -1);
		pEdge->AddFeature(WFC_WALL, -2, 0, 0.5);
		pEdge->AddFeature(WFC_WALL, -4);
		pEdge->AddFeature(WFC_WALL, -2, 0, 0.5);
		pEdge->AddFeature(WFC_WALL, -1);
		pEdge->m_Material = BMAT_WOOD;

		pEdge = pLev->m_Edges[2];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_GAP);

		pEdge = pLev->m_Edges[3];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_WALL, -1, 0, 0.5);
		pEdge->AddFeature(WFC_WALL);
		pEdge->AddFeature(WFC_WALL, -1, 0, 0.5);
		pEdge->m_Material = BMAT_CEMENT;

		// main floor level (1)
		dl.Empty();
		dl.Append(c7);
		dl.Append(c3);
		dl.Append(c4);
		dl.Append(c6);
		dl.Append(c9);
		dl.Append(c8);
		pLev = bld->CreateLevel(dl);
		pLev->m_fStoryHeight = 2.4385f;
		pLev->m_iStories = 1;
		pLev->SetEdgeMaterial(BMAT_WOOD);

		pEdge = pLev->m_Edges[0];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_WINDOW, -8, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -8);
		pEdge->AddFeature(WFC_WINDOW, -8, 0.5f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -1);
		pEdge->AddFeature(WFC_DOOR, -3);
		pEdge->AddFeature(WFC_WALL, -8);

		pEdge = pLev->m_Edges[1];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_WINDOW, -8, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -4);
		pEdge->AddFeature(WFC_WINDOW, -8, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -6);
		pEdge->AddFeature(WFC_WINDOW, -2, 0.4f, 0.8f);
		pEdge->AddFeature(WFC_WALL, -4);
		pEdge->AddFeature(WFC_WINDOW, -4, 0.7f, 1.0f);

		pEdge = pLev->m_Edges[2];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_WALL, -6);
		pEdge->AddFeature(WFC_WINDOW, -4, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -2);
		pEdge->AddFeature(WFC_WINDOW, -8, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -8);
		pEdge->AddFeature(WFC_WINDOW, -4, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -4);
		pEdge->AddFeature(WFC_WINDOW, -4, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -8);

		pEdge = pLev->m_Edges[3];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_WALL, -8);
		pEdge->AddFeature(WFC_WINDOW, -4, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -4);
		pEdge->AddFeature(WFC_WINDOW, -8, 0.3f, 1.0f);
		pEdge->AddFeature(WFC_WALL, -4);

		pEdge = pLev->m_Edges[4];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_WALL, -4);
		pEdge->AddFeature(WFC_WINDOW, -8, 0.3f, 1.0f);

		pEdge = pLev->m_Edges[5];
		pEdge->m_Features.Empty();
		pEdge->AddFeature(WFC_DOOR, -4);
		pEdge->AddFeature(WFC_WALL, -8);

		//////////////////////////////
		// first roof level (2)
		dl.Empty();
		dl.Append(c1);
		dl.Append(c3);
		dl.Append(c4);
		dl.Append(c6);
		pLev = bld->CreateLevel(dl);
		pLev->m_iStories = 1;
		pLev->SetEdgeMaterial(BMAT_PLAIN);
		pLev->SetEdgeColor(RGBi(90, 75, 75));
		bld->SetRoofType(ROOF_HIP, 14, 2);
		pLev->m_fStoryHeight = 0.9144f;	// 3 ft
		pLev->SetEaveLength(1.0f);

		//////////////////////////////
		// second roof level (3)
		dl.Empty();
		dl.Append(c8);
		dl.Append(c10);
		dl.Append(c11);
		dl.Append(c12);
		pLev = bld->CreateLevel(dl);
		pLev->m_iStories = 1;
		pLev->SetEdgeMaterial(BMAT_PLAIN);
		pLev->SetEdgeColor(RGBi(220, 220, 220));
		bld->SetRoofType(ROOF_GABLE, 33, 3);
		pLev->m_fStoryHeight = 1.6256f;	// 5 1/3 ft

		/////////////////////////////////////////
		bld->SetCenterFromPoly();

		vtTagArray options;
		bool success = bld->CreateNode(m_pHeightField, options);
		vtTransform *trans = bld->GetTransform();
		trans->SetName2("Test House");
		AddNode(trans);
		PlantModel(trans);
	}

	if (m_Params.m_bAirports)
	{
		create_airports();
		// more max-modeled buildings
		if (PointIsInTerrain(DPoint2(240749, 2194370))) // if area includes top of Mauna Kea
			create_telescopes();
	}

#if 1
	if (PointIsInTerrain(DPoint2(234900, 2185840))) // if area includes saddle
	{
		vtTransform *bench = LoadModel("Culture/parkbench.3ds");
		if (bench)
		{
			float scale = 3.0f * 0.01f;
			bench->Scale3(scale, scale*1.1f, scale);
			PlantModelAtPoint(bench, DPoint2(234900, 2185840));
			m_pLodGrid->AppendToGrid(bench);
		}
	}
#endif

	DPoint2 mauna_loa(227611, 2155222);
	if (PointIsInTerrain(mauna_loa)) // if area includes top of Mauna Loa
	{
		vtGeom *thebox = make_red_cube();
		vtGeom *thecone = make_test_cone();
		vtTransform *container = new vtTransform();
		container->AddChild(thebox);
		container->AddChild(thecone);
		AddNode(container);
		PlantModelAtPoint(container, mauna_loa);
	}

	if (m_Params.m_bVehicles)
	{
		SetupVehicles();
		create_airplanes(m_Params.m_fVehicleSize, m_Params.m_fVehicleSpeed, bDoSound);
		create_ground_vehicles(m_Params.m_fVehicleSize, m_Params.m_fVehicleSpeed);
	}

	if (m_Params.m_bDetailTexture)
	{
		m_pDetailMats = new vtMaterialArray();
		vtString path = FindFileOnPaths(m_DataPaths, "GeoTypical/grass_repeat2_512.bmp");
		vtImage *pDetailTexture = new vtImage(path);
		if (pDetailTexture->LoadedOK())
		{
			int index = m_pDetailMats->AddTextureMaterial(pDetailTexture,
							 true,	// culling
							 false,	// lighting
							 true,	// transp: blend
							 false,	// additive
							 0.0f, 1.0f,	// ambient, diffuse
							 0.5f, 0.0f,	// alpha, emmisive
							 true, false,	// texgen, clamp
							 true);			// mipmap
			m_pDetailMat = m_pDetailMats->GetAt(index);

			FRECT r = m_pHeightField->m_WorldExtents;
			float width_meters = r.Width();
			m_pDynGeom->SetDetailMaterial(m_pDetailMat, 0.025f * width_meters);
		}
	}
}


///////////////////////////////////////

void IslandTerrain::create_airplanes(float fSize, float fSpeed, bool bDoSound)
{
#if 1
	// make some planes
	for (int i = 0; i < 6; i++)
	{
		vtTransform *copy = CreateVehicle("747", RGBf(1.0f, 1.0f, 0.0f), fSize);
		AddNode(copy);

		// make it bigger and faster than real life
		float fSpeedExag = fSpeed;

		// attach engine
		AirportCodes code;
		code = KOA;

		PlaneEngine *pEng = new PlaneEngine(fSpeedExag, code);
		pEng->SetTarget(copy);
		pEng->SetHoop(i);
		AddEngine(pEng);

#if 0
		if (bDoSound)
		{
			//sound stuff
			vtSound3D* plane = new vtSound3D(m_strDataPath + "Vehicles/Airport-trim1.wav");
			plane->Initialize();
			plane->SetName2("Plane Sound");
			plane->SetModel(1,1,200,200);	//set limit of how far away sound can be heard
			plane->SetTarget(copy);			//set target
			plane->SetMute(true);			//mute the sound until later
			plane->Play(0, 0.0f);			//play the sound (muted)
			AddEngine(plane, pScene);
		}
#endif
	}
#endif
}

