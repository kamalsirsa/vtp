//
// Tower3d.cpp
//
// The vtTower3d class extends vtTower with the ability to procedurally
// create 3d geometry of the buildings.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/HeightField.h"
#include "vtdata/Triangulate.h"
#include "Tower3d.h"

vtTower3d::vtTower3d()
{
	m_pContainer = NULL;
	m_pTowerGeom = NULL;
	m_pTowerHighlight = NULL;
}


vtTower3d &vtTower3d::operator =(const vtTower3d &v)
{
	// just call the copy method of the parent class
	*((vtTower*)this) = v;
	return *this;
}

#define SPHERE_STEPS 24

vtGeom *CreateTowerBoundSphereGeom(vtGeom *pOfGeom)
{
	vtGeom *pGeom = new vtGeom();
	vtMaterialArray *pMats = new vtMaterialArray();
	pMats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 0.0f), false, false, true);
	pGeom->SetMaterials(pMats);

	vtMesh *pMesh = new vtMesh(GL_LINE_STRIP, 0, (SPHERE_STEPS+1)*3*2);

	// get bounding sphere
	FSphere sphere;
	pOfGeom->GetBoundSphere(sphere);
	float radius = sphere.radius * 0.9f;

	FPoint3 p;
	int i, j;
	float a;

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j <= SPHERE_STEPS; j++)
		{
			a = j * PI2f / SPHERE_STEPS;
			p.x = sin(a) * radius;
			p.y = cos(a) * radius;
			p.z = i ? radius * 0.05f : radius * -0.05f;
			pMesh->AddVertex(p + sphere.center);
		}
		for (j = 0; j <= SPHERE_STEPS; j++)
		{
			a = j * PI2f / SPHERE_STEPS;
			p.y = sin(a) * radius;
			p.z = cos(a) * radius;
			p.x = i ? radius * 0.05f : radius * -0.05f;
			pMesh->AddVertex(p + sphere.center);
		}
		for (j = 0; j <= SPHERE_STEPS; j++)
		{
			a = j * PI2f / SPHERE_STEPS;
			p.z = sin(a) * radius;
			p.x = cos(a) * radius;
			p.y = i ? radius * 0.08f : radius * -0.08f;
			pMesh->AddVertex(p + sphere.center);
		}
	}
	for (i = 0; i < 6; i++)
		pMesh->AddStrip2((SPHERE_STEPS+1), (SPHERE_STEPS+1) * i);

	pGeom->AddMesh(pMesh, 0);
	return pGeom;
}


// display some bounding wires around the object to highlight it
void vtTower3d::ShowTwrBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pTowerHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			m_pTowerHighlight = CreateTowerBoundSphereGeom(m_pTowerGeom);
			m_pContainer->AddChild(m_pTowerHighlight);
		}
		m_pTowerHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pTowerHighlight)
			m_pTowerHighlight->SetEnabled(false);
	}
}
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////


void vtTowerArray3d::OffsetSelected(DPoint2 &offset)
{
	vtTower3d *bld;
	for (int i = 0; i < GetSize(); i++)
	{
		bld = GetTower(i);
		if (!bld->IsSelected())
			continue;
		bld->Offset(offset);

		

		// Should really move the building to a new cell in the LOD Grid,
		// but unless it's moving really far we don't need to worry about this.
	}
}
