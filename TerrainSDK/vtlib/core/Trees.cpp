//
// Trees.cpp
//
// Implementation for the following classes:
//
// vtPlantAppearance3d
// vtPlantSpecies3d
// vtPlantList3d
// vtPlantInstance3d
// vtPlantInstanceArray3d
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Trees.h"
#include "vtlib/core/Light.h"
#include "vtdata/vtLog.h"

#define SHADOW_HEIGHT		0.1f	// distance above groundpoint in meters


/////////////////////////////////////////////////////////////////////////////
// vtPlantAppearance3d
//

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
		VTLOG("\tLoading plant texture '%s' ", (const char *) name);
		vtString fname = FindFileOnPaths(paths, name);

		m_pMats = new vtMaterialArray();

		// create textured appearance
		m_iMatIdx = m_pMats->AddTextureMaterial2(fname,
			false, true, true, false,	// cull, lighting, transp, additive
			TREE_AMBIENT, TREE_DIFFUSE,
			1.0f,		// alpha (material is opaque, alpha is in the texture)
			TREE_EMISSIVE);

		if (m_iMatIdx == -1)
			VTLOG(" Failed.\n");
		else
			VTLOG(" OK.\n");

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

/**
 * Create an object to represent a textured plant billboard.
 * Makes two intersecting polygons (4 triangles).
 */
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
	float w2 = (m_width * fTreeScale) / 2.0f;
	float h = m_height * fTreeScale;

	// keep a count of how many vertices we've added
	int vcount = 0, vstart;

	// do shadow first, so it will be drawn first
	if (bShadows && m_shadow_radius > 0.0f)
	{
		vstart = vcount;

		// shadow, on the ground
		float h1 = w2 * m_shadow_radius * 2.0f;
		float h2 = w2 * m_shadow_radius;
		float gr = SHADOW_HEIGHT;
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

		pTreeMesh->SetVtxPUV(vcount++, FPoint3(-w2, 0.0f, 0), 0.0f, 0.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3( w2, 0.0f, 0), 1.0f, 0.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(-w2, h, 0), 0.0f, 1.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3( w2, h, 0), 1.0f, 1.0f);
		//
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, 0.0f, -w2), 0.0f, 0.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, 0.0f,  w2), 1.0f, 0.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, h, -w2), 0.0f, 1.0f);
		pTreeMesh->SetVtxPUV(vcount++, FPoint3(0, h,  w2), 1.0f, 1.0f);

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
		float factor = s_fTreeScale;
		pGeom->Scale(factor, factor, factor);
#endif
	}
	return pTrans;
}


/////////////////////////////////////////////////////////////////////////////
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

/**
 * Return the species' appearance which is closest to a given height
 */
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

/**
 * Picks a random height, and return the species' appearance
 *  which is closest to that height.
 */
vtPlantAppearance3d *vtPlantSpecies3d::GetRandomAppearance()
{
	// pick a random height
	float height = random(m_fMaxHeight);

	return GetAppearanceByHeight(height);
}


void vtPlantSpecies3d::AddAppearance(AppearType type, const char *filename,
	float width, float height, float shadow_radius, float shadow_darkness)
{
	vtPlantAppearance3d *pApp = new vtPlantAppearance3d(type, filename,
		width, height, shadow_radius, shadow_darkness);
	m_Apps.Append(pApp);
}


/////////////////////////////////////////////////////////////////////////////
// vtPlantList3d
//

vtPlantList3d::vtPlantList3d()
{
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


/////////////////////////////////////////////////////////////////////////////
// vtPlantInstance3d
//

vtPlantInstance3d::vtPlantInstance3d()
{
	m_pTransform = NULL;
	m_pGeom = NULL;
	m_pHighlight = NULL;
}

void vtPlantInstance3d::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			// get bounding sphere
			FSphere sphere;
			m_pGeom->GetBoundSphere(sphere);

			m_pHighlight = CreateBoundSphereGeom(sphere);
			m_pTransform->AddChild(m_pHighlight);
		}
		m_pHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}


/////////////////////////////////////////////////////////////////////////////
// vtPlantInstanceArray3d
//

vtPlantInstanceArray3d::vtPlantInstanceArray3d()
{
	m_pHeightField = NULL;
	m_pPlantList = NULL;
}

vtPlantInstance3d *vtPlantInstanceArray3d::GetInstance3d(int i)
{
	if (i < 0 || i >= m_Instances3d.GetSize())
		return NULL;
	return m_Instances3d.GetAt(i);
}

int vtPlantInstanceArray3d::CreatePlantNodes()
{
	int size = GetSize();
	int created = 0;

	for (int i = 0; i < size; i++)
	{
		if (CreatePlantNode(i))
			created++;
	}
	return created;
}

bool vtPlantInstanceArray3d::CreatePlantNode(int i)
{
	if (!m_pPlantList)
		return false;

	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (!inst3d)
	{
		inst3d = new vtPlantInstance3d();
		m_Instances3d.SetAt(i, inst3d);
	}

	vtPlantInstance &pi = GetAt(i);

	vtPlantSpecies3d *ps = m_pPlantList->GetSpecies(pi.species_id);
	if (!ps)
		return false;

	vtPlantAppearance3d *pApp = ps->GetAppearanceByHeight(pi.size);
	if (!pApp)
		return false;

	inst3d->m_pTransform = pApp->GenerateGeom();
	inst3d->m_pGeom = (vtGeom *) inst3d->m_pTransform->GetChild(0);

	UpdateTransform(i);

	// We need to scale the model to produce the desired size, not the
	//  size of the appearance but of the instance.
	float scale = pi.size / pApp->m_height;
	inst3d->m_pTransform->Scale3(scale, scale, scale);

//	float size_variability = 0.3f;
//	float random_scale = 1.0f + random_offset(size_variability);
//	inst3d->m_pTransform->Scale3(random_scale, random_scale, random_scale);

	// Since the billboard are symmetric, a small rotation helps provide
	//  a more natural look.
	float random_rotation = random(PI2f);
	inst3d->m_pTransform->RotateLocal(FPoint3(0,1,0), random_rotation);

	return true;
}

vtTransform *vtPlantInstanceArray3d::GetPlantNode(int i)
{
	if (i < 0 || i >= m_Instances3d.GetSize())
		return NULL;

	return m_Instances3d.GetAt(i)->m_pTransform;
}

void vtPlantInstanceArray3d::VisualDeselectAll()
{
	int size = GetSize();

	for (int i = 0; i < size; i++)
	{
		vtPlantInstance3d *inst3d = GetInstance3d(i);
		if (inst3d)
		{
			inst3d->Select(false);
			inst3d->ShowBounds(false);
		}
	}
}

void vtPlantInstanceArray3d::VisualSelect(int i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (inst3d)
	{
		inst3d->Select(true);
		inst3d->ShowBounds(true);
	}
}


void vtPlantInstanceArray3d::OffsetSelectedPlants(const DPoint2 &offset)
{
	int size = GetSize();
	for (int i = 0; i < GetSize(); i++)
	{
		vtPlantInstance &pi = GetAt(i);
		vtPlantInstance3d *inst3d = GetInstance3d(i);

		if (!inst3d || !inst3d->IsSelected())
			continue;

		pi.m_p += offset;
		UpdateTransform(i);
	}
}

void vtPlantInstanceArray3d::UpdateTransform(int i)
{
	vtPlantInstance &pi = GetAt(i);
	vtPlantInstance3d *inst3d = GetInstance3d(i);
	FPoint3 p3;

	m_pHeightField->ConvertEarthToSurfacePoint(pi.m_p, p3);

	// Should really move the plant to a new cell in the LOD
	// Grid, but unless it's moving really far we don't need to
	// worry about this.

	inst3d->m_pTransform->SetTrans(p3);
}

