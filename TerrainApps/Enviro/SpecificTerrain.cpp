//
// SpecificTerrain.cpp
//
// A place to put implementation of terrain-specific behavior.
//
// Some terrain-specific implementations have their own file (e.g.
//	Hawaii.cpp, Nevada.cpp).  You can put your own implementation here, in
//	a file by itself, or wherever you like.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/Features.h"
#include "SpecificTerrain.h"
#include "Engines.h"


///////////////////////////////
// TransitTerrain
// (an unfinished project to procedurally create PRT structures)

void TransitTerrain::CreateCustomCulture()
{
	//
	// TODO
	//
	vtGeode *pblock = MakeBlockGeom(FPoint3(1.0f, 4.0f, 9.0f));
	addNode(pblock);

#if 0
	AddNode(campus);
	campus->Scale3(scale, scale, scale);
	campus->Rotate2(FPoint3(0.0f, 1.0f, 0.0f), -PIf*63/180);

	PlantModelAtPoint(campus, DPoint2(591849, 4138117));
	campus->Translate2(FPoint3(0.0f, 2.0f, 0.0f));
#endif
}

vtGeode *TransitTerrain::MakeBlockGeom(FPoint3 size)
{
	// create a Mesh
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_Normals, 24);

	FPoint3 half = size / 2;	// Block() will double the size

	mesh->CreateBlock(half);
//	mesh->TranslateVertices(half);	// place minimum corner at origin

	// create a single Material
	vtMaterialArray *looks = new vtMaterialArray;
	looks->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f));

	// create a Geometry and add the mesh and material
	vtGeode *thebox = new vtGeode;
	thebox->SetMaterials(looks);
	thebox->AddMesh(mesh, 0);

	return thebox;
}


///////////////////////////////////////


