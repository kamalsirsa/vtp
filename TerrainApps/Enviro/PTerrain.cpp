//
// PTerrain.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "PTerrain.h"
#include "SwapEngine.h"
#include "Engines.h"
#include "TerrainSceneWP.h"

PTerrain::PTerrain() : vtTerrain()
{
	m_pFirstVehicleType = NULL;
}

void PTerrain::MakePortal(vtTerrain* pTargetTerrain, vtTransform* gateway, 
						   char* name, int destination_index)
{
	float scale = 0.01f * (m_Params.m_iMinHeight*0.33f);
	float sc = WORLD_SCALE * scale;		// larger than real life
	gateway->Scale3(sc, sc, sc);
	gateway->SetName2(name);
	m_pTerrainGroup->AddChild(gateway);

#if 0
	float ws = 500.0f * sc;		// convert to 3d system units
	TriggerEngine *pTE = new TriggerEngine(pTargetTerrain, destination_index,
		GetTerrainScene().m_pSwapEng, gateway, ws);
	pTE->SetName2("Portal Trigger");
	pTE->SetTarget(pScene->GetCamera());
	AddEngine(pTE, pScene);
#endif

	SimpleBBEngine *pSBBE = new SimpleBBEngine(gateway, vtGetScene()->GetCamera());
	pSBBE->SetName2("SimpleBB Engine");
	AddEngine(pSBBE);
}

void PTerrain::MakePortalUTM(vtTerrain* pTargetTerrain, vtTransform* gateway, 
						   char* name, float utm_x, float utm_y, int destination_index)
{
	MakePortal(pTargetTerrain, gateway, name, destination_index);
	PlantModelUTM(gateway, utm_x, utm_y);
}

void PTerrain::MakePortalLL(vtTerrain* pTargetTerrain, vtTransform* gateway, 
							 char* name, float lat, float lon, int destination_index)
{
	MakePortal(pTargetTerrain, gateway, name, destination_index);
	PlantModelLL(gateway, lat, lon);
}


void PTerrain::AddVehicleType(VehicleType *vt)
{
	vt->m_pNext = m_pFirstVehicleType;
	m_pFirstVehicleType = vt;
}


void PTerrain::SetupVehicles()
{
	VehicleType *bronco = new VehicleType("bronco");
	bronco->SetModelLod(0, m_strDataPath + "Vehicles/bronco_v2.dsm", 500);
	AddVehicleType(bronco);

	VehicleType *discovery = new VehicleType("discovery");
	discovery->SetModelLod(0, m_strDataPath + "Vehicles/discovery_LOD01.dsm", 50);
	discovery->SetModelLod(1, m_strDataPath + "Vehicles/discovery_LOD02.dsm", 100);
	discovery->SetModelLod(2, m_strDataPath + "Vehicles/discovery_LOD03.dsm", 200);
	discovery->SetModelLod(3, m_strDataPath + "Vehicles/discovery_LOD04.dsm", 500);
	AddVehicleType(discovery);

	VehicleType *hele_on = new VehicleType("bus");
	hele_on->SetModelLod(0, m_strDataPath + "Vehicles/bus991025-3.dsm", 800);
	AddVehicleType(hele_on);

	VehicleType *b747 = new VehicleType("747");
	b747->SetModelLod(0, m_strDataPath + "Vehicles/747-LOD00.dsm", 200);
	b747->SetModelLod(1, m_strDataPath + "Vehicles/747-LOD01.dsm", 1000);
	b747->SetModelLod(2, m_strDataPath + "Vehicles/747-LOD02.dsm", 2000);
	b747->SetModelLod(3, m_strDataPath + "Vehicles/747-LOD03.dsm", 5000);
	AddVehicleType(b747);
}

Vehicle *PTerrain::CreateVehicle(const char *szType, RGBf &cColor, float fSize)
{
	//if vehicles haven't been created yet...
	if (m_pFirstVehicleType == NULL) {
		SetupVehicles();
	}

	VehicleType *vt;
	for (vt = m_pFirstVehicleType; vt; vt=vt->m_pNext)
	{
		if (!strcmp(szType, vt->m_strTypeName))
			break;
	}
	if (!vt)
		return NULL;

	return vt->CreateVehicle(cColor, fSize);
}


#if 0
void PTerrain::AddCarEngine(vtMovable *pVehicleModel,
								  float fSpeed, Node *pNode)
{
	CarEngine *pE1;
	if (pNode == NULL) {
		pE1 = new CarEngine(pVehicleModel->GetTrans(),
			m_pHeightField, fSpeed, .25f);
	} else {
		pE1 = new CarEngine(pVehicleModel->GetTrans(),
			m_pHeightField, fSpeed, .25f, pNode, 1, m_Params.m_fRoadHeight);
	}
	pE1->SetName2("drive");
	pE1->SetTarget(pVehicleModel);
	if (pE1->SetTires())
		AddEngine(pE1);
}

//engine that follows a path
void PTerrain::AddCarEngine(vtMovable *pVehicleModel,
								  float fSpeed, 
								  const char *filename)
{
	CarEngine *pE1;
	pE1 = new CarEngine(pVehicleModel->GetTrans(), m_pHeightField, fSpeed, .25f);
	pE1->SetName2("drive");
	pE1->GetPath(filename, m_pRoadMap);
	pE1->SetTarget(pVehicleModel);
	if (pE1->SetTires())
		AddEngine(pE1);
}
#endif

