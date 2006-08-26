//
// Vehicles.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Roads.h"
#include "vtlib/core/TerrainScene.h"
#include "vtdata/vtLog.h"
#include "CarEngine.h"
#include "Engines.h"
#include "Hawaii.h"


///////////////////////////////
// helpers

void ConvertPurpleToColor(vtGroupBase *pModel, RGBf replace)
{
#if 0
	RGBf color;
	int i;

	vtGeom *pShape;
	vtMaterialArray *pMats;
	vtMaterial *pMat;

	// TODO
	// walk down through a part of the scene graph, converting
	// any geometry encountered
	if (pModel->m_pModel->IsClass(vtGeom))
	{
		pShape = (vtGeom *)pModel;
		bool has_purple = false;
		pMats = pShape->GetMaterials();
		if (!pMats)
			return;
		for (i = 0; i < pMats->GetSize(); i++)
		{
			pMat = pMats->GetAt(i);

			// is purple?
			color = pMat->GetDiffuse();
			if (color.r == 1.0f && color.g == 0.0f && color.b == 1.0f)
			{
				has_purple = true;
				break;
			}
		}
		if (has_purple)
		{
			vtMaterialArray *pMats2 = new vtMaterialArray;
			pMats2->CopyFrom(pMats);
			pShape->SetMaterials(pMats2);
			pMats2->Release();
			for (i = 0; i < pMats2->GetSize(); i++)
			{
				pMat = pMats2->GetAt(i);
				if (!pMat) continue;
				color = pMat->GetDiffuse();
				if (color.r == 1.0f && color.g == 0.0f && color.b == 1.0f)
				{
					vtMaterial *pMat2 = new vtMaterial;
					pMat2->Copy(pMat);
					pMats2->SetAt(i, pMat2);
					pMat2->SetDiffuse2(replace);
					pMat2->SetAmbient2(replace*0.6f);
				}
			}
		}
	}

	for (i = 0; i < pModel->GetNumChildren(); i++)
	{
		vtTransform *pChild = (vtTransform *) pModel->GetChild(i);
		ConvertPurpleToColor(pChild, replace);
	}
#endif
}

/////////////////////////////////////////

VehicleManager::VehicleManager()
{
	m_pFirstVehicleType = NULL;
	m_bAttemptedVehicleLoad = false;
}

VehicleManager::~VehicleManager()
{
	ReleaseVehicles();
}

void VehicleManager::AddVehicleType(VehicleType *vt)
{
	vt->m_pNext = m_pFirstVehicleType;
	m_pFirstVehicleType = vt;
}


void VehicleManager::SetupVehicles()
{
	vtString fname;

	fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/bronco/bronco.osg");
	if (fname != "")
	{
		VehicleType *bronco = new VehicleType("bronco");
		// the bronco is modeled in meters (scale 1.0f)
		bronco->AddModel(fname, 1.0f, 500);
		AddVehicleType(bronco);
	}

	fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/mosp1/mosp1-v3.osg");
	if (fname != "")
	{
		VehicleType *bronco = new VehicleType("mosp1");
		// the bronco is modeled in meters (scale 1.0f)
		bronco->AddModel(fname, 1.0f, 500);
		AddVehicleType(bronco);
	}

	fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/landrover/rover-v3.osg");
	if (fname != "")
	{
		// the discovery is modeled in meters
		VehicleType *discovery = new VehicleType("discovery");
		discovery->AddModel(fname, 1.0f, 500);
		AddVehicleType(discovery);
/*
		fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/discovery/discovery_LOD01.3ds");
		discovery->AddModel(fname, 0.01f, 50);
		fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/discovery/discovery_LOD02.3ds");
		discovery->AddModel(fname, 0.01f, 100);
		fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/discovery/discovery_LOD03.3ds");
		discovery->AddModel(fname, 0.01f, 200);
		fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/discovery/discovery_LOD04.3ds");
		discovery->AddModel(fname, 0.01f, 500);
		AddVehicleType(discovery);
*/
	}

	fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/hele-on/hele-on.osg");
	if (fname != "")
	{
		// the bus is modeled in meters (1.0)
		VehicleType *hele_on = new VehicleType("bus");
		hele_on->AddModel(fname, 1.00f, 800);
		AddVehicleType(hele_on);
	}

	// the 747 is modeled in meters (1.0)
	fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/747/747-lod00.ive");
	if (fname != "")
	{
		VehicleType *b747 = new VehicleType("747");
		fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/747/747-lod00.ive");
		b747->AddModel(fname, 1.0f, 200);
		fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/747/747-lod01.ive");
		b747->AddModel(fname, 1.0f, 2000);
		fname = FindFileOnPaths(vtGetDataPath(), "Vehicles/747/747-lod02.ive");
		b747->AddModel(fname, 1.0f, 20000);
		AddVehicleType(b747);
	}
}

void VehicleManager::ReleaseVehicles()
{
	VehicleType *vt, *next;
	for (vt = m_pFirstVehicleType; vt; vt=next)
	{
		next = vt->m_pNext;
		delete vt;
	}
}

Vehicle *VehicleManager::CreateVehicle(const char *szType, const RGBf &cColor, float fSize)
{
	//if vehicles haven't been created yet...
	if (!m_bAttemptedVehicleLoad)
	{
		SetupVehicles();
		m_bAttemptedVehicleLoad = true;
	}

	for (VehicleType *vt = m_pFirstVehicleType; vt; vt=vt->m_pNext)
	{
		if (!strcmp(szType, vt->m_strTypeName))
			return vt->CreateVehicle(cColor, fSize);
	}
	return NULL;
}


void VehicleManager::CreateSomeTestVehicles(vtTerrain *pTerrain, float fSize, float fSpeed)
{
	vtRoadMap3d *pRoadMap = pTerrain->GetRoadMap();

	// add some test vehicles
	NodeGeom *n = NULL;
	FPoint3 vNormal, center;
	FPoint3 start_point;
	int num, col;
	RGBf color;
	for (int i = 0; i < 4; i++)
	{
		if (n == NULL) {
			n = pRoadMap->GetFirstNode();
		}
		num = i % 4;
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

		vtTransform *car=NULL;
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
		case 3:
			car = CreateVehicle("mosp1", color, fSize);
			break;
		}
		if (car)
		{
			pTerrain->AddNode(car);
			pTerrain->PlantModelAtPoint(car, n->m_p);

			float fSpeed = 88.0f;
			TNode *pNode = n;

			CarEngine *pE1;
			if (pNode == NULL)
			{
				pE1 = new CarEngine(pTerrain->GetHeightField(), fSpeed, .25f,
					car->GetTrans());
			}
			else
			{
				pE1 = new CarEngine(pTerrain->GetHeightField(), fSpeed, .25f,
					pNode, 1);
			}
			pE1->SetName2("drive");
			pE1->SetTarget(car);
			if (pE1->SetTires())
				pTerrain->AddEngine(pE1);
			else
				delete pE1;
		}
		n = (NodeGeom*) n->m_pNext;
	}
}

VehicleType::VehicleType(const char *szName)
{
	m_strTypeName = szName;
	m_pNext = NULL;
	m_bAttemptedLoaded = false;
}

VehicleType::~VehicleType()
{
	ReleaseModels();
}

Vehicle::Vehicle()
{
	m_pLOD = new vtLOD;
	AddChild(m_pLOD);
	m_pHighlight = NULL;
}

void Vehicle::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			// get bounding sphere
			vtNode *contents = GetChild(0);
			if (contents)
			{
				FSphere sphere;
				GetBoundSphere(sphere);
				sphere.center.Set(0,0,0);

				m_pHighlight = CreateBoundSphereGeom(sphere);
				AddChild(m_pHighlight);
			}
		}
		m_pHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}


//
// set filename and swap-out distance (in meters) for each LOD
//
void VehicleType::AddModel(const char *filename, float fScale, float fDistance)
{
	m_strFilename.push_back(vtString(filename));
	m_fScale.Append(fScale);
	m_fDistance.Append(fDistance);
}

void VehicleType::AttemptModelLoad()
{
	m_bAttemptedLoaded = true;

	for (unsigned int i = 0; i < m_strFilename.size(); i++)
	{
		// can we read the file?
		vtString fname = m_strFilename[i];
		if (vtNode *pMod = vtNode::LoadModel(fname))
		{
			float scale = m_fScale[i];

			vtTransform *trans = new vtTransform;
			trans->AddChild(pMod);
			trans->Scale3(scale, scale, scale);

			vtString str;
			str.Format("scaling transform (%f)", scale);
			trans->SetName2(str);

			m_pModels.SetAt(i, trans);
		}
	}
}

Vehicle *VehicleType::CreateVehicle(const RGBf &cColor, float fScale)
{
	// check if it's loaded yet
	if (!m_bAttemptedLoaded)
		AttemptModelLoad();
	if (m_pModels.GetSize() == 0)
		return NULL;

	Vehicle *pNewVehicle = new Vehicle;
	pNewVehicle->SetName2("VehicleLOD-" + m_strTypeName);

	float distances[10];
	// there is no distance at which the object should vanish for being too close
	distances[0] = 0.0f;

	unsigned int iModels = m_pModels.GetSize();
	for (unsigned int i = 0; i < iModels; i++)
	{
		vtNode *node = (vtNode *) m_pModels.GetAt(i);

		//VTLOG1("-----------------\n");
		//vtLogGraph(node);
		//VTLOG1("-----------------\n");
		//vtLogNativeGraph(node->GetOsgNode());

		vtTransform *pNewModel = (vtTransform *)node->Clone(true);	// Deep copy

		//VTLOG1("-----------------\n");
		//vtLogNativeGraph(pNewModel->GetOsgNode());

		pNewVehicle->AddChild(pNewModel);
		distances[i+1] = m_fDistance.GetAt(i) * fScale;

		vtNode *pFrontLeft = pNewModel->FindNativeNode("front_left");
		vtNode *pFrontRight = pNewModel->FindNativeNode("front_right");
		vtNode *pRearLeft = pNewModel->FindNativeNode("rear_left");
		vtNode *pRearRight = pNewModel->FindNativeNode("rear_right");

		if (!pFrontLeft || !pFrontRight || !pRearLeft || !pRearRight)
		{
			// Didn't find them.
			continue;
		}
		if (dynamic_cast<vtTransform*>(pFrontLeft))
		{
			// They are already transforms, no need to insert any
			pFrontLeft->SetName2("front_left_xform");
			pFrontRight->SetName2("front_right_xform");
			pRearLeft->SetName2("rear_left_xform");
			pRearRight->SetName2("rear_right_xform");
		}
		else
		{
			// Stick transform above them (this seems like a bad way to go)
			vtTransform *pTransform;

			pTransform = new vtTransform;
			pTransform->SetName2("front_left_xform");
			pTransform->AddChild(pFrontLeft);
			pFrontLeft->GetParent()->RemoveChild(pFrontLeft);
			pNewModel->AddChild(pTransform);

			pTransform = new vtTransform;
			pTransform->SetName2("front_right_xform");
			pTransform->AddChild(pFrontRight);
			pFrontRight->GetParent()->RemoveChild(pFrontRight);
			pNewModel->AddChild(pTransform);

			pTransform = new vtTransform;
			pTransform->SetName2("rear_left_xform");
			pTransform->AddChild(pRearLeft);
			pRearLeft->GetParent()->RemoveChild(pRearLeft);
			pNewModel->AddChild(pTransform);

			pTransform = new vtTransform;
			pTransform->SetName2("rear_right_xform");
			pTransform->AddChild(pRearRight);
			pRearRight->GetParent()->RemoveChild(pRearRight);
			pNewModel->AddChild(pTransform);

			//VTLOG1("-----------------\n");
			//vtLogNativeGraph(pNewModel->GetOsgNode());
		}
	}
	pNewVehicle->m_pLOD->SetRanges(distances, iModels+1);

	pNewVehicle->m_fSize = fScale;
	ConvertPurpleToColor(pNewVehicle, cColor);

	return pNewVehicle;
}

void VehicleType::ReleaseModels()
{
	for (unsigned int i = 0; i < m_pModels.GetSize(); i++)
	{
		m_pModels[i]->Release();

		//vtNode *pTyres = m_pTyres[i];
		//if(NULL != pTyres)
		//	pTyres->Release();
	}
	m_pModels.Empty();
	m_pTyres.Empty();
}


///////////////////////////////////////////////////////////////////////

VehicleSet::VehicleSet()
{
	m_iSelected = -1;
}

void VehicleSet::AddEngine(CarEngine *e)
{
	m_Engines.push_back(e);
}

int VehicleSet::FindClosestVehicle(const FPoint3 &point, float &closest)
{
	closest = 1E9;
	int vehicle = -1;
	for (unsigned int i = 0; i < m_Engines.size(); i++)
	{
		float dist = (point - m_Engines[i]->GetCurPos()).Length();
		if (dist < closest)
		{
			closest = dist;
			vehicle = i;
		}
	}
	return vehicle;
}

void VehicleSet::VisualSelect(int vehicle)
{
	Vehicle *car = dynamic_cast<Vehicle*> (m_Engines[vehicle]->GetTarget());
	if (!car)
		return;
	car->ShowBounds(true);
	m_iSelected = vehicle;
}

void VehicleSet::VisualDeselectAll()
{
	unsigned int size = m_Engines.size();
	for (unsigned int i = 0; i < size; i++)
	{
		Vehicle *car = dynamic_cast<Vehicle*> (m_Engines[i]->GetTarget());
		if (car)
			car->ShowBounds(false);
	}
	m_iSelected = -1;
}

void VehicleSet::SetVehicleSpeed(int vehicle, float fMetersPerSec)
{
	CarEngine *eng = m_Engines[vehicle];
	eng->SetTargetSpeed(fMetersPerSec);
}

