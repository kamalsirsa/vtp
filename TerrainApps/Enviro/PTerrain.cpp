//
// PTerrain.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "PTerrain.h"
#include "Engines.h"
#include "CarEngine.h"

PTerrain::PTerrain() : vtTerrain()
{
	m_pFirstVehicleType = NULL;
}

PTerrain::~PTerrain()
{
	ReleaseVehicles();
}

void PTerrain::MakePortal(vtTerrain* pTargetTerrain, vtTransform* gateway,
						  char* name, int destination_index)
{
	// larger than real life
	float scale = 0.01f * (m_Params.GetValueInt(STR_MINHEIGHT) * 0.33f);
	gateway->Scale3(scale, scale, scale);
	gateway->SetName2(name);
	AddNode(gateway);

	SimpleBillboardEngine *pSBBE = new SimpleBillboardEngine;
	pSBBE->SetTarget(gateway);
	pSBBE->SetName2("SimpleBB Engine");
	AddEngine(pSBBE);
}

void PTerrain::MakePortalUTM(vtTerrain* pTargetTerrain, vtTransform* gateway,
							 char* name, float utm_x, float utm_y, int destination_index)
{
	MakePortal(pTargetTerrain, gateway, name, destination_index);
	PlantModelAtPoint(gateway, DPoint2(utm_x, utm_y));
}

void PTerrain::AddVehicleType(VehicleType *vt)
{
	vt->m_pNext = m_pFirstVehicleType;
	m_pFirstVehicleType = vt;
}


void PTerrain::SetupVehicles()
{
	// the bronco is modeled in centimeters (0.01)
	VehicleType *bronco = new VehicleType("bronco");
	vtString fname;
	fname = FindFileOnPaths(s_DataPaths, "Vehicles/bronco/bronco_v2.3ds");
	bronco->AddModel(fname, 0.01f, 500);
	AddVehicleType(bronco);

	// the discovery is modeled in centimeters (0.01)
	VehicleType *discovery = new VehicleType("discovery");
	fname = FindFileOnPaths(s_DataPaths, "Vehicles/discovery/discovery_LOD01.3ds");
	discovery->AddModel(fname, 0.01f, 50);
	fname = FindFileOnPaths(s_DataPaths, "Vehicles/discovery/discovery_LOD02.3ds");
	discovery->AddModel(fname, 0.01f, 100);
	fname = FindFileOnPaths(s_DataPaths, "Vehicles/discovery/discovery_LOD03.3ds");
	discovery->AddModel(fname, 0.01f, 200);
	fname = FindFileOnPaths(s_DataPaths, "Vehicles/discovery/discovery_LOD04.3ds");
	discovery->AddModel(fname, 0.01f, 500);
	AddVehicleType(discovery);

	// the bus is modeled in centimeters (0.01)
	VehicleType *hele_on = new VehicleType("bus");
	fname = FindFileOnPaths(s_DataPaths, "Vehicles/hele-on/bus020101.3ds");
	hele_on->AddModel(fname, 0.01f, 800);
	AddVehicleType(hele_on);

	// the 747 is modeled in meters (1.0)
	VehicleType *b747 = new VehicleType("747");
	fname = FindFileOnPaths(s_DataPaths, "Vehicles/747/747-lod00.3ds");
	b747->AddModel(fname, 1.0f, 200);
//	fname = FindFileOnPaths(s_DataPaths, "Vehicles/747-LOD01.3ds");
//	b747->AddModelLod(1, fname, 1000);
	AddVehicleType(b747);
}

void PTerrain::ReleaseVehicles()
{
	VehicleType *vt, *next;
	for (vt = m_pFirstVehicleType; vt; vt=next)
	{
		next = vt->m_pNext;
		delete vt;
	}
}

Vehicle *PTerrain::CreateVehicle(const char *szType, const RGBf &cColor, float fSize)
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


#if 1
void PTerrain::AddCarEngine(vtTransform *pVehicleModel,
							float fSpeed, Node *pNode)
{
	CarEngine *pE1;
	if (pNode == NULL)
	{
		pE1 = new CarEngine(pVehicleModel->GetTrans(),
			m_pHeightField, fSpeed, .25f);
	}
	else
	{
		float height = m_Params.GetValueFloat(STR_ROADHEIGHT);
		pE1 = new CarEngine(pVehicleModel->GetTrans(),
			m_pHeightField, fSpeed, .25f, pNode, 1, height);
	}
	pE1->SetName2("drive");
	pE1->SetTarget(pVehicleModel);
	if (pE1->SetTires())
		AddEngine(pE1);
}

//engine that follows a path
void PTerrain::AddCarEngine(vtTransform *pVehicleModel,
							float fSpeed, const char *filename)
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

