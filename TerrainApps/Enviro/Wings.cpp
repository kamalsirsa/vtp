//
// Wings.cpp
//
// All types of flying critters
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Light.h"
#include "vtlib/core/TerrainPatch.h"
#include "Wings.h"
#include "Nevada.h"

Butterfly::Butterfly(vtTerrain *terrain, float radius, float speed, 
					 float height, FPoint3 center, float size_exag)
{
	static int num = 0;

	char *fname;
	int number = (num++)%3;
	if (number == 0)		fname = "Nevada/butterfly1_v3.png";
	else if (number == 1)	fname = "Nevada/butterfly2_v1.png";
	else if (number == 2)	fname = "Nevada/butterfly3_v2.png";

	vtString path = FindFileOnPaths(vtTerrain::m_DataPaths, fname);
	m_pApps = new vtMaterialArray();
	m_pApps->AddTextureMaterial2(path,
		false, true,	// cull, light
		true, false,	// transp: blend, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE,
		1.0f, TERRAIN_EMISSIVE);
	m_pApps->AddRGBMaterial1(RGBf(0.0f, 0.0f, 1.0f), false, false);	// blue
	m_pApps->AddRGBMaterial1(RGBf(0.0f, 0.7f, 0.0f), false, false);	// dk. green
	m_pApps->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f), false, false);  // red

	FPoint2 org, siz;
	org.x = 0;
	org.y = 0;
	siz.x = 0.001f * size_exag;	//"real" butterfly size * size_exaggeration
	siz.y = 0.0005f * size_exag;

	m_pWing1 = CreatePlaneMGeom(m_pApps, 0, org, siz, 1.0f, 1.0f, 1);
	m_pWing1->SetName2("wing1");
	SetName2("ButterflyWing1");
	m_pWing1->Rotate2(FPoint3(0.0f, 0.0f, 1.0f), PIf); 
	AddChild(m_pWing1);

	m_pWing2 = CreatePlaneMGeom(m_pApps, 0, org, siz, 1.0f, 1.0f, 1);
	m_pWing2->SetName2("wing2");
	SetName2("ButterflyWing2");
	AddChild(m_pWing2);

	//make the FlyingCritter engine
	FlyingCritterEngine *pFCE;
	pFCE = new FlyingCritterEngine(this, terrain, center, speed, height,
		terrain->GetHeightField(), radius);
	pFCE->SetTarget(this);
	terrain->AddEngine(pFCE);
}

FlyingCritterEngine::FlyingCritterEngine(Butterfly* critter, vtTerrain* terrain, 
										 FPoint3 center, float speed, float height,
										 vtHeightField3d* grid, float radius)
{
	m_critter = critter;
	m_terrain = terrain;
	m_num = 0;

	m_fSpeed = speed * 1000/60/60; 
	m_vCenterPos = center;
//	m_fCircleRadius = sqrt(pow(m_vCurPos.x - m_vCenterPos.x,2) + pow(m_vCurPos.z - m_vCenterPos.z,2));
	m_fCircleRadius = radius;
	m_fLastTime = vtGetTime() * m_fSpeed;
	m_fHeightAboveGround = height;

	m_pHeightField = grid;
	m_fCurRotation = 0;
}

void FlyingCritterEngine::Eval()
{
	vtTransform* target = (vtTransform*) GetTarget();
	if (!target)
		return;

	// next code implements the wing flapping
	if (m_num >= 8)
	{
	}
	else if ((m_num < 2) || (m_num >= 6)) 
	{
		m_critter->m_pWing1->Rotate2(FPoint3(0.0f, 0.0f, 1.0f), -PIf/8);
		m_critter->m_pWing2->Rotate2(FPoint3(0.0f, 0.0f, 1.0f), PIf/8);
	}
	else if ((m_num >= 2) || (m_num < 6)) 
	{
		m_critter->m_pWing1->Rotate2(FPoint3(0.0f, 0.0f, 1.0f), PIf/8);
		m_critter->m_pWing2->Rotate2(FPoint3(0.0f, 0.0f, 1.0f), -PIf/8);
	}
	m_num++;
	if (m_num == 12) m_num = 0;	//reset after a while

	// next code implements the circling

	// calculate position
	float time = vtGetTime() * m_fSpeed;
	FPoint3 vNext;
	vNext.x = m_vCenterPos.x + m_fCircleRadius * cosf(time);
	vNext.z = m_vCenterPos.z + m_fCircleRadius * sinf(time);

	// calculate x, z, and angle change since last update
	float delta_x, delta_z;
	delta_x = vNext.x - m_vCurPos.x;
	delta_z = vNext.z - m_vCurPos.z;
	float newangle = atan2f(-delta_z, delta_x) - PIf/2.0f;

	// calculate new y position
	float fAltitude;
	m_pHeightField->FindAltitudeAtPoint(vNext, fAltitude);
	vNext.y = fAltitude + m_fHeightAboveGround;

	// perform the appropriate updates to the critter
	target->Identity();
	target->Rotate2(FPoint3(0.0f, 1.0f, 0.0f), newangle);
	target->Translate1(vNext);

	// save position for the next update cycle
	m_vCurPos = vNext;
}

