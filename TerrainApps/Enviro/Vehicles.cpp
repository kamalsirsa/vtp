//
// Vehicles.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Roads.h"
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
			pApps2->Release();
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
		vtTransform *pChild = (vtTransform *) pModel->GetChild(i);
		ConvertPurpleToColor(pChild, replace);
	}
#endif
}

/////////////////////////////////////////


void PTerrain::create_ground_vehicles(float fSize, float fSpeed)
{
	FPoint3 vNormal, center;
	FPoint3 start_point;

	// add some cars!
	if (m_pRoadMap)
	{
		NodeGeom *n = (NodeGeom*) m_pRoadMap->GetFirstNode();
		vtTransform *car;
		int num, col;
		RGBf color;
		for (int i = 0; i < m_Params.GetValueInt(STR_NUMCARS); i++)
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
			AddNode(car);
			PlantModelAtPoint(car, n->m_p);
			AddCarEngine(car, 60.0f, n);
			n = (NodeGeom*) n->m_pNext;
		}
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
	m_pLOD = new vtLOD();
	AddChild(m_pLOD);
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
		if (vtNodeBase *pMod = vtLoadModel(fname))
		{
			vtTransform *trans = new vtTransform();
			trans->AddChild(pMod);
			if (fname.Right(3).CompareNoCase("3ds") == 0)
			{
				// Must rotate by 90 degrees for 3DS -> OpenGL (or Lightwave LWO)
				trans->Rotate2(FPoint3(1.0f, 0.0f, 0.0f), -PID2f);
			}
			float scale = m_fScale[i];
			trans->Scale3(scale, scale, scale);
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

	Vehicle *pNewVehicle = new Vehicle();
	pNewVehicle->SetName2("VehicleLOD-" + m_strTypeName);

	float distances[10];
	// there is no distance at which the object should vanish for being too close
	distances[0] = 0.0f;

	unsigned int iModels = m_pModels.GetSize();
	for (unsigned int i = 0; i < iModels; i++)
	{
		vtTransform *pNewModel = (Vehicle *)m_pModels.GetAt(i)->Clone();
		pNewVehicle->AddChild(pNewModel);
		distances[i+1] = m_fDistance.GetAt(i) * fScale;
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
	}
	m_pModels.Empty();
}

