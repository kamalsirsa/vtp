//
// Vehicles.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TerrainSceneWP.h"
#include "Engines.h"
#include "Hawaii.h"


///////////////////////////////
// helpers

void ConvertPurpleToColor(vtGroup *pModel, RGBf replace)
{
#if 0
	RGBf color;
	int i;

	vtGeom *pShape;
	vtMaterialArray *pApps;
	vtMaterial *pApp;

	// TODO
	// walk down through a part of the scene graph, converting
	// any geometry encountered
	if (pModel->m_pModel->IsClass(vtGeom))
	{
		pShape = (vtGeom *)pModel;
		bool has_purple = false;
		pApps = pShape->GetMaterials();
		if (!pApps)
			return;
		for (i = 0; i < pApps->GetSize(); i++)
		{
			pApp = pApps->GetAt(i);

			// is purple?
			color = pApp->GetDiffuse();
			if (color.r == 1.0f && color.g == 0.0f && color.b == 1.0f)
			{
				has_purple = true;
				break;
			}
		}
		if (has_purple)
		{
			vtMaterialArray *pApps2 = new vtMaterialArray();
			pApps2->CopyFrom(pApps);
			pShape->SetMaterials(pApps2);
			for (i = 0; i < pApps2->GetSize(); i++)
			{
				pApp = pApps2->GetAt(i);
				if (!pApp) continue;
				color = pApp->GetDiffuse();
				if (color.r == 1.0f && color.g == 0.0f && color.b == 1.0f)
				{
					vtMaterial *pApp2 = new vtMaterial();
					pApp2->Copy(pApp);
					pApps2->SetAt(i, pApp2);
					pApp2->SetDiffuse2(replace);
					pApp2->SetAmbient2(replace*0.6f);
				}
			}
		}
	}

	for (i = 0; i < pModel->GetNumChildren(); i++)
	{
		vtMovable *pChild = (vtMovable *) pModel->GetChild(i);
		ConvertPurpleToColor(pChild, replace);
	}
#endif
}

/////////////////////////////////////////

void IslandTerrain::create_road_vehicles()
{
	// must have roads for them to drive on
	if (!m_pRoadMap)
		return;
}


void PTerrain::create_ground_vehicles(float fSize, float fSpeed)
{
	FPoint3 vNormal, center;
	FPoint3 start_point;

	//add some cars!
	if (m_pRoadMap)
	{
		NodeGeom *n = (NodeGeom*) m_pRoadMap->GetFirstNode();
		vtTransform *car;
		int num, col;
		RGBf color;
		for (int i = 0; i < m_Params.m_iNumCars; i++)
		{
			if (n == NULL) {
				n = (NodeGeom*) m_pRoadMap->GetFirstNode();
			}
			num = i % 3;
			col = i % 5;

			switch (col) {
			case 0:
				color.Set(1.0f, 1.0f, 1.0f);
				break;
			case 1:
				color.Set(1.0f, 1.0f, 0.0f);
				break;
			case 2:
				color.Set(0.0f, 0.0f, .5f);
				break;
			case 3:
				color.Set(1.0f, 0.0f, 0.0f);
				break;
			case 4:
				color.Set(0.0f, .5f, 0.0f);
				break;
			}

			switch (num) {
			case 0:
				car = CreateVehicle("discovery", color, fSize);
				break;
			case 1:
				car = CreateVehicle("bronco", color, fSize);
				break;
			case 2:
				car = CreateVehicle("bus", color, fSize);
				break;
			}
			m_pTerrainGroup->AddChild(car);
			PlantModelUTM(car, n->m_p);
//			AddCarEngine(car, 60.0f, n);
			n = (NodeGeom*) n->m_pNext;
		}
	}

/*	// try loading a car
	CarEngine *cEng;
	vtMovable *car;

	car = LoadModel("Vehicles/bronco_v2.DSM");
	float fAltitude;
	convert_utm_to_local_xz(192200, 2157700, start_point.x, start_point.z);
	m_pLocalGrid->FindAltitudeAtPoint(start_point, fAltitude, vNormal);

	if (car != NULL)
	{
		car->Size(.01f*sc,.01f*sc,.01f*sc);
		car->RotateLocal(YAXIS, PID2f); //adjust for MAX model
#if CAR_VIEW
		FollowerEngine* follower = new FollowerEngine(car, pScene->GetCamera());
		follower->SetName2("Follower Engine");
		AddEngine(follower, pScene);
#endif
		if (m_Params.m_bDoSound)
		{
			// sound
			vtSound3D* carnoise = new vtSound3D("Data/Vehicles/loud01.wav");
			carnoise->Initialize();
			carnoise->SetName2("Corvette Sound");
			carnoise->SetModel(1,1,30,30);	//set limit of how far away sound can be heard
			carnoise->SetTarget(car);		//set target
			carnoise->SetMute(true);		//mute the sound until later
			carnoise->Play(0, 0.0f);		//play the sound (muted)
			AddEngine(carnoise, pScene);
		}

		m_pTerrainGroup->AddChild(car);

		if (m_pRoadMap != NULL) {
			//follow a road
			cEng = new CarEngine(start_point, m_pLocalGrid, 60.0f*fSize, .25f*fSize, m_pRoadMap->GetFirstNode(), 1, m_Params.m_fRoadHeight);
			cEng->SetName2("Car Engine");
//			cEng->GetPath("Data/test2.pf", m_pRoadMap);
		} else {
			//simple test: drive in a circle
			center = start_point;
			//for moving in a circle
			center.z -=4;
			cEng = new CarEngine(start_point, m_pLocalGrid, 60.0f*fSize, .25f*fSize, center);
			cEng->SetName2("Car Engine");
			//or drive in a straight line
			//cEng = new CarEngine(start_point, pLocalGrid, 60*fSize, .25*fSize);
		}

		cEng->SetTarget(car);
		if (cEng->SetTires()) {
			AddEngine(cEng, pScene);
		} else {
			//uh oh!
			delete cEng;
		}
	}
*/
}

VehicleType::VehicleType(const char *szName)
{
	m_strTypeName = szName;
	m_pNext = NULL;
	m_bAttemptedLoaded = false;
	m_iLods = 0;
}

//
// set filename and swap-out distance (in meters) for each LOD
//
void VehicleType::SetModelLod(int lod, const char *filename, float fDistance)
{
	if (lod > MAX_VEHICLE_LODS)
		return;
	m_strFilename[lod] = filename;
	if (m_iLods <= lod)
		m_iLods = lod+1;
	m_fDistance.SetAt(lod, fDistance * WORLD_SCALE);
}

void VehicleType::AttemptModelLoad()
{
	m_bAttemptedLoaded = true;
	if (m_iLods < 1)
		return;
#if 0
	// TODO
	for (int i = 0; i < m_iLods; i++)
	{
		if (vtModel *pMod = vtLoadModel(m_strFilename[i]))			// can read file?
			m_pModels.SetAt(i, pMod);
	}
#endif
}

Vehicle *VehicleType::CreateVehicle(RGBf &cColor, float fSize)
{
	// check if it's loaded yet
	if (!m_bAttemptedLoaded)
		AttemptModelLoad();
	if (m_pModels.GetSize() == 0)
		return NULL;

	Vehicle *pNewVehicle = new Vehicle();
	pNewVehicle->SetName2("VehicleLOD-" + m_strTypeName);

	float distances[10];
	// there is no distance at which the object should vanish for being too close
	distances[0] = 0.0f;

#if 0
	for (int i = 0; i < m_iLods; i++)
	{
		vtMovable *pNewModel = (Vehicle *)m_pModels.GetAt(i)->CreateClone();
		pNewVehicle->AddChild(pNewModel);
		distances[i+1] = m_fDistance.GetAt(i) * fSize;
	}
	pNewVehicle->SetRanges(distances, m_iLods+1);

	float scale = 0.01f * WORLD_SCALE;	// models are in cm
	pNewVehicle->m_fSize = fSize * scale;
	ConvertPurpleToColor(pNewVehicle, cColor);

	return pNewVehicle;
#else
	return NULL;
#endif
}

