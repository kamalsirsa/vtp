//
// TransitTerrain.cpp
//
// Terrain implementation with the addition of simulated Transit.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TransitTerrain.h"
#include "TerrainSceneWP.h"
#include "Engines.h"

///////////////////////////////

TransitTerrain::TransitTerrain() : PTerrain()
{
}

TransitTerrain::~TransitTerrain()
{
}

void TransitTerrain::CreateCustomCulture(bool bDoSound)
{
	//
	// TODO
	//
	vtGeom *pblock = MakeBlockGeom(FPoint3(1.0f, 4.0f, 9.0f));
	m_pTerrainGroup->AddChild(pblock);

#if 0
	m_pTerrainGroup->AddChild(campus);
	campus->Scale3(scale, scale, scale);
	campus->Rotate2(FPoint3(0.0f, 1.0f, 0.0f), -PIf*63/180);

	PlantModelUTM(campus, 591849, 4138117);
	campus->Translate2(FPoint3(0.0f, 2.0f*WORLD_SCALE, 0.0f));
#endif
}

vtGeom *TransitTerrain::MakeBlockGeom(FPoint3 size)
{
	// create a Mesh
	vtMesh *mesh = new vtMesh(GL_TRIANGLE_FAN, VT_Normals, 24);

	size *= WORLD_SCALE;		// convert meters -> scene
	FPoint3 half = size / 2;	// Block() will double the size

	mesh->CreateBlock(half);
//	mesh->TranslateVertices(half);	// place minimum corner at origin

	// create a single Material
	vtMaterialArray *looks = new vtMaterialArray();
	looks->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f));

	// create a Geometry and add the mesh and material
	vtGeom *thebox = new vtGeom();
	thebox->SetMaterials(looks);
	thebox->AddMesh(mesh, 0);

	return thebox;
}


///////////////////////////////////////


