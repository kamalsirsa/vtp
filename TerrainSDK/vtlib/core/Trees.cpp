//
// Trees.cpp
//
// Implementation for the following classes:
//
// vtPlantAppearance3d
// vtPlantSpecies3d
// vtPlantList3d
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Trees.h"
#include "vtlib/core/Light.h"

#define SHADOW_HEIGHT		0.1f	// distance above groundpoint in meters


vtPlantAppearance3d::vtPlantAppearance3d(AppearType type, const char *filename,
	 float width, float height, float shadow_radius, float shadow_darkness)
	: vtPlantAppearance(type, filename, width, height, shadow_radius, shadow_darkness)
{
	m_pMats = NULL;
	m_pMesh = NULL;
#if SUPPORT_XFROG
	m_pFrogModel = NULL;
#endif
}

vtPlantAppearance3d::~vtPlantAppearance3d()
{
	if (m_pMats) delete m_pMats;
	if (m_pMesh) delete m_pMesh;
#if SUPPORT_XFROG
	if (m_pFrogModel) delete m_pFrogModel;
#endif
}

vtPlantAppearance3d::vtPlantAppearance3d(const vtPlantAppearance &v)
{
	m_pMats = NULL;
	m_pMesh = NULL;
#if SUPPORT_XFROG
	m_pFrogModel = NULL;
#endif

	m_eType = v.m_eType;
	m_filename = v.m_filename;
	m_width	= v.m_width;
	m_height = v.m_height;
	m_shadow_radius	= v.m_shadow_radius;
	m_shadow_darkness = v.m_shadow_darkness;
}

void vtPlantAppearance3d::LoadAndCreate(const StringArray &paths,
		float fTreeScale, bool bShadows, bool bBillboards)
{
	s_fTreeScale = fTreeScale;

	if (m_eType == AT_BILLBOARD)
	{
		vtString name = "PlantModels/";
		name += m_filename;
		vtString fname = FindFileOnPaths(paths, name);

		m_pMats = new vtMaterialArray();

		// create textured appearance
		m_iMatIdx = m_pMats->AddTextureMaterial2(fname,
			false, true, true, false,	// cull, lighting, transp, additive
			TREE_AMBIENT, TREE_DIFFUSE,
			1.0f,		// alpha
			TREE_EMISSIVE);

		if (bShadows)
		{
			// create shadow material (1)
			m_pMats->AddShadowMaterial(m_shadow_darkness);
		}

		// create a surface object to represent the tree
		m_pMesh = CreateTreeMesh(fTreeScale, bShadows, bBillboards);
	}
	else
	{
#if SUPPORT_XFROG
		char pname[160];
		strcpy(pname, datapath);
		strcat(pname, "PlantModels/");

		// xfrog plant
		m_pFrogModel = new CFrogModel(pname, m_filename);
#endif
	}
}

//
// create an object to represent a textured tree
//
// makes two intersecting polygons (4 triangles)
//
vtMesh *vtPlantAppearance3d::CreateTreeMesh(float fTreeScale, bool bShadows,
											bool bBillboards)
{
	// first determine how many vertices we'll need for this mesh
	int vtx_count = 0;
	if (bShadows && m_shadow_radius > 0.0f) vtx_count += 6;
	vtx_count += 8;

	// make a mesh
	vtMesh *pTreeMesh = new vtMesh(GL_TRIANGLE_FAN, VT_TexCoords, vtx_count);

	// size of textured, upright portion
	float w2 = (m_width * WORLD_SCALE * fTreeScale) / 2.0f;
	float h = m_height * WORLD_SCALE * fTreeScale;

	// keep a count of how many vertices we've added
	int vcount = 0, vstart;

	// do shadow first, so it will be drawn first
	if (bShadows && m_shadow_radius > 0.0f)
	{
		vstart = vcount;

		// shadow, on the ground
		float h1 = w2 * m_shadow_radius * 2.0f;
		float h2 = w2 * m_shadow_radius;
		float gr = SHADOW_HEIGHT * WORLD_SCALE;
		pTreeMesh->SetVtxPos(vcount++, FPoint3(-h1, gr,  h2));
		pTreeMesh->SetVtxPos(vcount++, FPoint3(0.0f, gr,  h1));
		pTreeMesh->SetVtxPos(vcount++, FPoint3(h1, gr,  h2));
		pTreeMesh->SetVtxPos(vcount++, FPoint3(h1, gr, -h2));
		pTreeMesh->SetVtxPos(vcount++, FPoint3(0.0f, gr, -h1));
		pTreeMesh->SetVtxPos(vcount++, FPoint3(-h1, gr, -h2));

		pTreeMesh->AddFan(vcount, vcount+1, vcount+2, vcount+3, vcount+4, vcount+5);
	}

	// the do the rest of the billboard geometry: two squares
	if (bBillboards)
	{
		vstart = vcount;

		pTreeMesh->SetVtxPUV(vcount++, FPoint3(-w2, 0.0f, 0), 0.0f, 1.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3( w2, 0.0f, 0), 1.0f, 1.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(-w2, h, 0), 0.0f, 0.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3( w2, h, 0), 1.0f, 0.0f);
		//
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, 0.0f, -w2), 0.0f, 1.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, 0.0f,  w2), 1.0f, 1.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, h, -w2), 0.0f, 0.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, h,  w2), 1.0f, 0.0f);

#if 0
		// 4 triangles
		pTreeMesh->AddTri(0, vstart+0, vstart+1, vstart+2);
		pTreeMesh->AddTri(0, vstart+2, vstart+1, vstart+3);
		pTreeMesh->AddTri(0, vstart+4, vstart+5, vstart+6);
		pTreeMesh->AddTri(0, vstart+6, vstart+5, vstart+7);
#else
		// 2 fans
		pTreeMesh->AddFan(vstart+0, vstart+1, vstart+3, vstart+2);
		pTreeMesh->AddFan(vstart+4, vstart+5, vstart+7, vstart+6);
#endif
	}
	return pTreeMesh;
}


vtTransform *vtPlantAppearance3d::GenerateGeom()
{
	vtTransform *pTrans = NULL;
	if (m_eType == AT_BILLBOARD)
	{
		vtGeom *pGeom = new vtGeom();
		pGeom->SetMaterials(m_pMats);
		pGeom->AddMesh(m_pMesh, m_iMatIdx);
		pTrans = new vtMovGeom(pGeom);
	}
	else if (m_eType == AT_XFROG)
	{
#if SUPPORT_XFROG
		pGeom = m_pFrogModel->CreateShape(1.0f);
		float factor = WORLD_SCALE * s_fTreeScale;
		pGeom->Scale(factor, factor, factor);
#endif
	}
	return pTrans;
}


//
// return the species' appearance which is closest to a given height
//
vtPlantAppearance3d *vtPlantSpecies3d::GetAppearanceByHeight(float fHeight)
{
	// simple case: if only one appearance, nothing random is possible
	if (m_Apps.GetSize() == 1)
		return (vtPlantAppearance3d *) m_Apps[0];

	// find the appearance closest to that height
	float closest_diff = m_fMaxHeight;
	vtPlantAppearance *closest = NULL;

	for (int i = 0; i < m_Apps.GetSize(); i++)
	{
		vtPlantAppearance *pa = m_Apps[i];
		float diff = fabsf(pa->m_height - fHeight);
		if (diff < closest_diff)
		{
			closest_diff = diff;
			closest = pa;
		}
	}
	return (vtPlantAppearance3d *)closest;
}

//
// pick a random height, and return the species' appearance
//  which is closest to that height
//
vtPlantAppearance3d *vtPlantSpecies3d::GetRandomAppearance()
{
	// pick a random height
	float height = random(m_fMaxHeight);

	return GetAppearanceByHeight(height);
}

/////////////////////////////////////////////////////
// vtPlantSpecies3d
//
vtPlantSpecies3d::vtPlantSpecies3d() : vtPlantSpecies()
{
}

vtPlantSpecies3d &vtPlantSpecies3d::operator=(const vtPlantSpecies &v)
{
	m_iSpecieID = v.GetSpecieID();
	m_szCommonName = v.GetCommonName();
	m_szSciName = v.GetSciName();
	m_fMaxHeight = v.GetMaxHeight();

	int apps = v.NumAppearances();
	for (int i = 0; i < apps; i++)
	{
		vtPlantAppearance3d *pa3d = new vtPlantAppearance3d(*(v.GetAppearance(i)));
//		*pa3d = ;
		m_Apps.Append(pa3d);
	}
	return *this;
}

void vtPlantSpecies3d::AddAppearance(AppearType type, const char *filename,
	float width, float height, float shadow_radius, float shadow_darkness)
{
	vtPlantAppearance3d *pApp = new vtPlantAppearance3d(type, filename,
		width, height, shadow_radius, shadow_darkness);
	m_Apps.Append(pApp);
}

/////////////////////////////////////////////////////
// vtPlantList3d
//

vtPlantList3d::vtPlantList3d()
{
#if 0
	char buf[80];
	int iApps, j, apptype, iNumSpecies;
	float f1, f2, f3, f4;

	FILE *fp = fopen(filename, "r");
	if (!fp) return;

	fscanf(fp, "total species: %d\n", &iNumSpecies);
	for (int i = 0; i < iNumSpecies; i++)
	{
		fscanf(fp, "species: %s\n", buf);
		fscanf(fp, "max height: %f\n", &f1);

		AddSpecies(buf, f1);
		vtPlantSpecies *pSpecies = GetSpecies(i);

		fscanf(fp, "appearances: %d\n", &iApps);
		for (j = 0; j < iApps; j++)
		{
			fscanf(fp, "%d %s %f %f %f %f\n", &apptype, buf, &f1, &f2, &f3, &f4);
			pSpecies->AddAppearance(apptype == 1, buf, f1, f2, f3, f4);
		}
	}
	fclose(fp);
#endif
}

// copy
vtPlantList3d &vtPlantList3d::operator=(const vtPlantList &v)
{
	int sp = v.NumSpecies();
	for (int i = 0; i < sp; i++)
	{
		vtPlantSpecies *pOld = v.GetSpecies(i);
		vtPlantSpecies3d *pNew = new vtPlantSpecies3d();
		*pNew = *pOld;
		m_Species.Append((vtPlantSpecies *) pNew);
	}
	return *this;
}


vtPlantSpecies3d *vtPlantList3d::GetSpecies(int i) const
{
	if (i >= 0 && i < m_Species.GetSize())
		return (vtPlantSpecies3d *) m_Species[i];
	else
		return NULL;
}

void vtPlantList3d::AddSpecies(const char *common_name, float max_height)
{
	vtPlantSpecies3d *pSpe = new vtPlantSpecies3d();
	pSpe->SetCommonName(common_name);
	pSpe->SetMaxHeight(max_height);
	m_Species.Append(pSpe);
}

/*vtGeom *vtPlantList3d::plant_nursery(vtHeightField *pHeightField, float lat, float lon)
{
	int i;
	float latitude, longitude;

	// create nursery containing one of each plant
	SimpleLOD *pNursery = new SimpleLOD();
	pNursery->SetName("Tree Nursery");
	pNursery->SetRange(200.0f);
	for (i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies3d *pSpecies = GetSpecies(i);
		int iApps = pSpecies->NumAppearances();
		for (int j = 0; j < iApps; j++)
		{
			vtPlantAppearance3d *pApp = pSpecies->GetAppearance(j);

			longitude = lon + (0.0006f * i);
			latitude = lat + (0.0002f * i);

			plant_tree_latlon(pHeightField, longitude, latitude,
						pNursery, pApp->m_pSurf, pApp->m_pMats,
						0.0f,	// spacing variability
						0.0f,	// size variability
						pSpecies->GetCommonName());
		}
	}
	return pNursery;
}*/

////////////

#if 0
// forest_derived_from_image

	for (i = 0; i < pDIB->m_iWidth; i += step)
	{
		a = (i * FOREST_CLUSTER / pDIB->m_iWidth);
		if (a == FOREST_CLUSTER)
			break;
		for (j = 0; j < pDIB->m_iHeight; j += step)
		{
			b = (j * FOREST_CLUSTER / pDIB->m_iHeight);
			pixel = pDIB->GetPixel(i, pDIB->m_iHeight - 1 - j);

			// yes, it's BACKWARDS.  Not sure why, but it is BGR, blue green red
			blue = GetRValue(pixel);
			green = GetGValue(pixel);
			red = GetBValue(pixel);

			if (red <= 86 &&
				green >= 76 && green <= 178 &&
				blue <= 62)
				probability = 0.8f;		// probably forest
			else
			if (red >= 87 && red <= 140 &&
				green >= 76 && green <= 220 &&
				blue <= 86)
				probability = 0.1f;		// might have scattered trees (eg. pasture)
			else
				probability = 0.0f;		// no trees

			if (random(1.0f) < probability)
			{
				tree_count++;
			}
		}
	}
#endif


void vtPlantList3d::CreatePlantSurfaces(const StringArray &paths,
		float fTreeScale, bool bShadows, bool bBillboards)
{
	for (int i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies3d *pSpecies = GetSpecies(i);
		int iApps = pSpecies->NumAppearances();
		for (int j = 0; j < iApps; j++)
		{
			vtPlantAppearance3d *pApp = pSpecies->GetAppearance(j);
			pApp->LoadAndCreate(paths, fTreeScale, bShadows, bBillboards);
		}
	}
}


//
// Look up an appropriate plant appearance, given a common name and a requested height
//
vtPlantAppearance3d *vtPlantList3d::GetAppearanceByName(const char *szName, float fHeight)
{
	for (int i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies3d *ps = GetSpecies(i);
		if (!strcmp(szName, ps->GetCommonName()))
		{
			// found it
			return ps->GetAppearanceByHeight(fHeight);
		}
	}
	return NULL;
}
