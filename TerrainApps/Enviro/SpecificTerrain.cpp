//
// SpecificTerrain.cpp
//
// A place to put implementation of terrain-specific behavior.
//
// Some terrain-specific implementations have their own file (e.g.
//	Hawaii.cpp, Nevada.cpp).  You can put your own implementation here, in
//	a file by itself, or wherever you like.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/Features.h"
#include "SpecificTerrain.h"
#include "Engines.h"

///////////////////////////////
// Romania

void Romania::CreateCustomCulture(bool bDoSound)
{
	vtString fname = "PointData/";
	fname += "RO.shp";
//	fname += "RO-subset.shp";
	vtString path = FindFileOnPaths(s_DataPaths, fname);
	if (path == "")
		return;

#if 1
	vtFeatures feat;
	if (!feat.LoadFrom(path))
		return;

	int i, num = feat.GetNumEntities();
	for (i = 0; i < num; i++)
	{
		int ppc = feat.GetIntegerValue(i, 1);
		if (ppc != 1)
			feat.SetToDelete(i);
	}
	feat.ApplyDeletion();
	num = feat.GetNumEntities();

	PointStyle style;
	style.m_field_index = 2;
	style.m_label_elevation = 3000;
	style.m_label_size = 3000;

	CreateStyledFeatures(feat, "Fonts/Arial.ttf", style);
#endif
}

///////////////////////////////
// TransitTerrain
// (an unfinished project to procedurally create PRT structures)

void TransitTerrain::CreateCustomCulture(bool bDoSound)
{
	//
	// TODO
	//
	vtGeom *pblock = MakeBlockGeom(FPoint3(1.0f, 4.0f, 9.0f));
	AddNode(pblock);

#if 0
	AddNode(campus);
	campus->Scale3(scale, scale, scale);
	campus->Rotate2(FPoint3(0.0f, 1.0f, 0.0f), -PIf*63/180);

	PlantModelAtPoint(campus, DPoint2(591849, 4138117));
	campus->Translate2(FPoint3(0.0f, 2.0f, 0.0f));
#endif
}

vtGeom *TransitTerrain::MakeBlockGeom(FPoint3 size)
{
	// create a Mesh
	vtMesh *mesh = new vtMesh(GL_TRIANGLE_FAN, VT_Normals, 24);

	FPoint3 half = size / 2;	// Block() will double the size

	mesh->CreateBlock(half);
//	mesh->TranslateVertices(half);	// place minimum corner at origin

	// create a single Material
	vtMaterialArray *looks = new vtMaterialArray();
	looks->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f));

	// create a Geometry and add the mesh and material
	vtGeom *thebox = new vtGeom();
	thebox->SetMaterials(looks);
	looks->Release();
	thebox->AddMesh(mesh, 0);

	return thebox;
}


///////////////////////////////////////


