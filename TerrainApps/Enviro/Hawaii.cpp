//
// Hawai`i.cpp
//
// Terrain implementation specific to the Big Island of Hawai`i.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/DynTerrain.h"
#include "Hawaii.h"
#include "Nevada.h"


////////////////////////

class TestMesh : public vtMesh
{
public:
	TestMesh(GLenum PrimType, int VertType, int NumVertices) :
	  vtMesh(PrimType, VertType, NumVertices) {};

	void CreateSpecialConicalSurface(FPoint3 &tip, double fConeRadius,
											 double theta1, double theta2,
											 double r1, double r2);
};

#define RES		40

void TestMesh::CreateSpecialConicalSurface(FPoint3 &tip, double cone_radius,
											 double theta1, double theta2,
											 double r1, double r2)
{
	int i, j, vidx;
	double tan_cr = tan(cone_radius);
	double theta, theta_step = (theta2 - theta1) / (RES - 1);
	double r, r_step = (r2 - r1) / (RES - 1);

	FPoint3 p, norm;

	r = r1;
	for (i = 0; i < RES; i++)
	{
		theta = theta1;
		for (j = 0; j < RES; j++)
		{
			p.x = tip.x + cos(theta) * r;
			p.z = tip.z - sin(theta) * r;
			p.y = tip.y - (r / tan_cr);
			vidx = AddVertex(p);

			if (GetVtxType() & VT_Normals)
			{
				// compute vertex normal for lighting
				norm.x = cos(theta) * r;
				norm.y = 0.0f;
				norm.z = sin(theta) * r;
				norm.Normalize();
				SetVtxNormal(vidx, norm);
			}

			theta += theta_step;
		}
		r += r_step;
	}
	CreateRectangularMesh(RES, RES);
}

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
		vtStructure3d *str = sa.GetStructure(i);

		success = sa.ConstructStructure(str);
		if (!success)
			continue;

		vtTransform *pTrans = str->GetTransform();
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
	TestMesh *pMesh = new TestMesh(GL_TRIANGLE_STRIP, VT_Normals, RES*RES);

	FPoint3 tip(0, 2000, 0);
	double cone_radius = PId/4;
	double theta1 = PId * 1.3;
	double theta2 = PId * 1.8;
	double r1 = 700;
	double r2 = 1500;

	pMesh->CreateSpecialConicalSurface(tip, cone_radius, theta1, theta2, r1, r2);

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
		if (PointIsInTerrain(c1)) // if area includes ben's house
		{
			// test ability to import a max model (a house)
			vtTransform *house = LoadModel("BuildingModels/house3.dsm");
			if (house)
			{
				// scale was one unit = 1 inch
				// 1 inch = 2.54 centimeters = .0254 meters
				float scale = .0254f;
				house->Scale3(scale, scale, scale);

				// plant it on the ground
				PlantModelAtPoint(house, bound[7]);

				AddNodeToLodGrid(house);
			}
		}
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
	// 36' = 10.9728 m
	// 40' = 12.1920 m
	// 48' = 14.6304 m
	// test dynamic creation of a complicated building
	if (1)
	{
		vtBuilding3d *bld = new vtBuilding3d();

		DPoint2 c2, c3, c4, c5, c6;
		c2 = c1 + DPoint2(10.9728, 0.0);
		c3 = c1 + DPoint2(14.6304, 0.0);
		c4 = c1 + DPoint2(14.6304, 12.1920);
		c5 = c1 + DPoint2(10.9728, 12.1920);
		c6 = c1 + DPoint2(0.0, 12.1920);
		DLine2 dl;
		vtLevel *pLev;
		vtWall *pWall;

		// basement/garage level
		dl.Append(c2);
		dl.Append(c3);
		dl.Append(c4);
		dl.Append(c5);
		pLev = new vtLevel();
		pLev->SetFootprint(dl);
		pLev->m_fStoryHeight = 2.4385f;
		pLev->m_iStories = 1;

		pWall = pLev->m_Wall[0];
		pWall->m_Features.Empty();
		pWall->m_Features.Append(vtWallFeature(WFC_GAP));

		pWall = pLev->m_Wall[1];
		pWall->m_Features.Empty();
		pWall->m_Features.Append(vtWallFeature(WFC_WALL, -1));
		pWall->m_Features.Append(vtWallFeature(WFC_WALL, -2, 0, 0.5));
		pWall->m_Features.Append(vtWallFeature(WFC_WALL, -4));
		pWall->m_Features.Append(vtWallFeature(WFC_WALL, -2, 0, 0.5));
		pWall->m_Features.Append(vtWallFeature(WFC_WALL, -1));

		pWall = pLev->m_Wall[2];
		pWall->m_Features.Empty();
		pWall->m_Features.Append(vtWallFeature(WFC_GAP));

		pWall = pLev->m_Wall[3];
		pWall->m_Features.Empty();
		pWall->m_Features.Append(vtWallFeature(WFC_WALL, -1, 0, 0.5));
		pWall->m_Features.Append(vtWallFeature(WFC_WALL));
		pWall->m_Features.Append(vtWallFeature(WFC_WALL, -1, 0, 0.5));

		bld->AddLevel(pLev);

		// main floor level
		dl.Empty();
		dl.Append(c1);
		dl.Append(c3);
		dl.Append(c4);
		dl.Append(c6);
		pLev = new vtLevel();
		pLev->SetFootprint(dl);
		pLev->m_fStoryHeight = 2.4385f;
		pLev->m_iStories = 1;
		// pLev->m_Wall
		bld->AddLevel(pLev);

		bld->m_RoofType = ROOF_HIP;
		bld->SetCenterFromPoly();

		vtStructure3d *str = new vtStructure3d;
		str->SetBuilding(bld);
		bool success = str->CreateNode(m_pHeightField, "roof walls detail");
		vtTransform *trans = str->GetTransform();
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

			FRECT r = m_pHeightField->m_Conversion.m_WorldExtents;
			float width_meters = r.Width();
			m_pDynGeom->SetDetailMaterial(m_pDetailMat, 0.025f * width_meters);
		}
	}
}


///////////////////////////////////////

void IslandTerrain::create_airplanes(float fSize, float fSpeed, bool bDoSound)
{
#if 0
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

