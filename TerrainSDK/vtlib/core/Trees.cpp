//
// Trees.cpp
//
// Implementation for the following classes:
//
// vtPlantAppearance3d
// vtPlantSpecies3d
// vtSpeciesList3d
// vtPlantInstance3d
// vtPlantInstanceArray3d
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"
#include "vtdata/FilePath.h"
#include "vtdata/HeightField.h"
#include "Trees.h"
#include "Light.h"
#include "GeomUtil.h"	// for CreateBoundSphereGeom

#define SHADOW_HEIGHT		0.1f	// distance above groundpoint in meters

float vtPlantAppearance3d::s_fPlantScale = 1.0f;
bool vtPlantAppearance3d::s_bPlantShadows = false;


/////////////////////////////////////////////////////////////////////////////
// vtPlantAppearance3d
//

vtPlantAppearance3d::vtPlantAppearance3d(AppearType type, const char *filename,
	 float width, float height, float shadow_radius, float shadow_darkness)
	: vtPlantAppearance(type, filename, width, height, shadow_radius, shadow_darkness)
{
	_Defaults();
}

vtPlantAppearance3d::~vtPlantAppearance3d()
{
#if SUPPORT_XFROG
	if (m_pFrogModel) delete m_pFrogModel;
#endif
}

vtPlantAppearance3d::vtPlantAppearance3d(const vtPlantAppearance &v)
{
	_Defaults();

	m_eType = v.m_eType;
	m_filename = v.m_filename;
	m_width	= v.m_width;
	m_height = v.m_height;
	m_shadow_radius	= v.m_shadow_radius;
	m_shadow_darkness = v.m_shadow_darkness;
}

void vtPlantAppearance3d::_Defaults()
{
	m_pGeode = NULL;
	m_pMats = NULL;
	m_pMesh = NULL;
#if SUPPORT_XFROG
	m_pFrogModel = NULL;
#endif
	m_pExternal = NULL;
	m_bAvailable = false;
	m_bCreated = false;
}

// Helper
vtString FindPlantModel(const vtString &filename)
{
	vtString name = "PlantModels/";
	name += filename;
	return FindFileOnPaths(vtGetDataPath(), name);
}

void vtPlantAppearance3d::CheckAvailability()
{
	if (m_eType == AT_BILLBOARD || m_eType == AT_MODEL)
	{
		// check if file exists and is readable
		vtString fname = FindPlantModel(m_filename);
		if (fname == "")
			return;

		FILE *fp = vtFileOpen(fname, "rb");
		if (fp != NULL)
		{
			m_bAvailable = true;
			fclose(fp);
		}
#if 1
		else
			VTLOG(" C0uldn't find appearance: %s\n", (const char *)fname);
#endif
	}
	else if (m_eType == AT_XFROG)
	{
	}
}

void vtPlantAppearance3d::LoadAndCreate()
{
	// only need to create once
	if (m_bCreated)
		return;

	if (m_eType == AT_BILLBOARD)
	{
		VTLOG("\tLoading plant texture '%s' ", (const char *) m_filename);
		vtString fname = FindPlantModel(m_filename);

		m_pMats = new vtMaterialArray;

		// create textured appearance
		m_iMatIdx = m_pMats->AddTextureMaterial2(fname,
			false, true, true, false,	// cull, lighting, transp, additive
			TREE_AMBIENT, TREE_DIFFUSE,
			1.0f,		// alpha (material is opaque, alpha is in the texture)
			TREE_EMISSIVE);

		if (m_iMatIdx == -1)
			VTLOG(" Failed.\n");
		else
		{
			VTLOG(" OK.\n");
			m_bCreated = true;
		}

		if (s_bPlantShadows)
		{
			// create shadow material (1)
			m_pMats->AddShadowMaterial(m_shadow_darkness);
		}

		// create a surface object to represent the tree
		m_pMesh = CreateTreeMesh(s_fPlantScale, s_bPlantShadows);
		m_pGeode = new vtGeode;
		m_pGeode->SetMaterials(m_pMats);
		m_pGeode->AddMesh(m_pMesh, m_iMatIdx);
	}
	else if (m_eType == AT_XFROG)
	{
#if SUPPORT_XFROG
		// xfrog plant
		vtString fname = FindPlantModel(vtGetDataPath(), m_filename);
		m_pFrogModel = new CFrogModel(fname, m_filename);
#endif
	}
	else if (m_eType == AT_MODEL)
	{
		m_pExternal = vtLoadModel(m_filename);
		if (!m_pExternal)
		{
			vtString fname = FindPlantModel(m_filename);
			if (fname != "")
				m_pExternal = vtLoadModel(fname);
		}
		if (m_pExternal != NULL)
			m_bCreated = true;
	}
}

/**
 * Create an object to represent a textured plant billboard.
 * Makes two intersecting polygons (4 triangles).
 */
vtMesh *vtPlantAppearance3d::CreateTreeMesh(float fTreeScale, bool bShadows)
{
	// first determine how many vertices we'll need for this mesh
	int vtx_count = 0;
	if (bShadows && m_shadow_radius > 0.0f) vtx_count += 6;
	vtx_count += 8;

	// make a mesh
	vtMesh *pTreeMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_TexCoords, vtx_count);

	// size of textured, upright portion
	float w2 = (m_width * fTreeScale) / 2.0f;
	float h = m_height * fTreeScale;

	// keep a count of how many vertices we've added
	int vcount = 0, vstart;

#if 0	// This code would need to be re-written to use a separate mesh for
		// the shadow.  However, it's probably not worth it as a polygon
		// plant shadow is at best a poor visual effect.
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
#endif

	// the do the rest of the billboard geometry: two squares
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

	return pTreeMesh;
}


bool vtPlantAppearance3d::GenerateGeom(vtTransform *container)
{
	if (m_eType == AT_BILLBOARD)
	{
		container->addChild(m_pGeode);
		return true;
	}
	else if (m_eType == AT_XFROG)
	{
#if SUPPORT_XFROG
		pGeode = m_pFrogModel->CreateShape(1.0f);
		float factor = s_fPlantScale;
		pGeode->Scale(factor, factor, factor);
#endif
	}
	else if (m_eType == AT_MODEL)
	{
		if (m_pExternal)
		{
			container->addChild(m_pExternal);
			return true;
		}
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////
// vtPlantSpecies3d
//

vtPlantSpecies3d::vtPlantSpecies3d() : vtPlantSpecies()
{
}

vtPlantSpecies3d &vtPlantSpecies3d::operator=(const vtPlantSpecies &v)
{
	uint i;

	m_szSciName = v.GetSciName();
	m_fMaxHeight = v.GetMaxHeight();

	uint cnames = v.NumCommonNames();
	m_CommonNames.resize(cnames);
	for (i = 0; i < cnames; i++)
		m_CommonNames[i] = v.GetCommonName(i);

	uint apps = v.NumAppearances();
	for (i = 0; i < apps; i++)
	{
		vtPlantAppearance3d *pa3d = new vtPlantAppearance3d(*(v.GetAppearance(i)));
		m_Apps.Append(pa3d);
	}
	return *this;
}

/**
 * Return the species' appearance which is closest to a given height
 */
vtPlantAppearance3d *vtPlantSpecies3d::GetAppearanceByHeight(float fHeight)
{
	uint i, size = m_Apps.GetSize();

	// simple case: if only one appearance, nothing random is possible
	if (size == 1)
		return (vtPlantAppearance3d *) m_Apps[0];

	// find the appearance closest to that height
	float closest_diff = 1E9;
	float closest_value=0;
	for (i = 0; i < size; i++)
	{
		vtPlantAppearance *pa = m_Apps[i];
		float diff = fabsf(pa->m_height - fHeight);
		if (diff < closest_diff)
		{
			closest_diff = diff;
			closest_value = pa->m_height;
		}
	}

	// If there is more than one appearance with the same height,
	//  find them and pick one of them at random.
	std::vector<vtPlantAppearance *> close;
	for (i = 0; i < size; i++)
	{
		vtPlantAppearance *pa = m_Apps[i];
		if (pa->m_height == closest_value)
			close.push_back(pa);
	}
	if (close.size() == 1)
	{
		// simple case
		return (vtPlantAppearance3d *) close[0];
	}
	else
	{
		int which = rand() % close.size();
		return (vtPlantAppearance3d *) close[which];
	}
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

void vtPlantSpecies3d::CheckAvailability()
{
#if VTDEBUG
	VTLOG(" species '%s' checking availability of %d appearances\n",
		(const char *) m_szSciName, NumAppearances());
#endif
	for (uint i = 0; i < NumAppearances(); i++)
		GetAppearance(i)->CheckAvailability();
}

int vtPlantSpecies3d::NumAvailableInstances()
{
	int num = 0;
	for (uint i = 0; i < NumAppearances(); i++)
		if (GetAppearance(i)->IsAvailable())
			num++;
	return num;
}

void vtPlantSpecies3d::AddAppearance(AppearType type, const char *filename,
	float width, float height, float shadow_radius, float shadow_darkness)
{
	vtPlantAppearance3d *pApp = new vtPlantAppearance3d(type, filename,
		width, height, shadow_radius, shadow_darkness);
	m_Apps.Append(pApp);
}


/////////////////////////////////////////////////////////////////////////////
// vtSpeciesList3d
//

vtSpeciesList3d::vtSpeciesList3d()
{
}

// copy operator
vtSpeciesList3d &vtSpeciesList3d::operator=(const vtSpeciesList &v)
{
	int sp = v.NumSpecies();
	for (int i = 0; i < sp; i++)
	{
		vtPlantSpecies *pOld = v.GetSpecies(i);
		vtPlantSpecies3d *pNew = new vtPlantSpecies3d;
		*pNew = *pOld;
		m_Species.Append((vtPlantSpecies *) pNew);
	}
	return *this;
}


vtPlantSpecies3d *vtSpeciesList3d::GetSpecies(uint i) const
{
	if (i >= 0 && i < m_Species.GetSize())
		return (vtPlantSpecies3d *) m_Species[i];
	else
		return NULL;
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


int vtSpeciesList3d::CheckAvailability()
{
	VTLOG("vtSpeciesList3d checking availability of %d species.\n",
		NumSpecies());

	int num = 0;
	for (uint i = 0; i < NumSpecies(); i++)
	{
		GetSpecies(i)->CheckAvailability();
		num += GetSpecies(i)->NumAvailableInstances();
	}
	return num;
}

/**
 * Create all of the appearances for all the species in this species list.
 */
void vtSpeciesList3d::CreatePlantSurfaces()
{
	for (uint i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies3d *pSpecies = GetSpecies(i);
		int iApps = pSpecies->NumAppearances();
		for (int j = 0; j < iApps; j++)
		{
			vtPlantAppearance3d *pApp = pSpecies->GetAppearance(j);
			pApp->LoadAndCreate();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// vtPlantInstance3d
//

vtPlantInstance3d::vtPlantInstance3d()
{
	m_pContainer = NULL;
	m_pHighlight = NULL;
}

vtPlantInstance3d::~vtPlantInstance3d()
{
	// Don't release the instance's nodes here.  They will either be released
	//  by DeletePlant(), or automatically as the whole scene graph is
	//  destructed at the time the terrain is destructed.
}

void vtPlantInstance3d::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			// get bounding sphere
			osg::Node *contents = m_pContainer->getChild(0);
			if (contents)
			{
				FSphere sphere;
				GetBoundSphere(contents, sphere);

				m_pHighlight = CreateBoundSphereGeom(sphere);
				m_pContainer->addChild(m_pHighlight);
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

void vtPlantInstance3d::ReleaseContents()
{
	if (!m_pContainer)	// safety check
		return;

	int ch = m_pContainer->getNumChildren();
	for (int i = 0; i < ch; i++)
	{
		osg::Node *node = m_pContainer->getChild(ch-1-i);
		if (node != m_pHighlight)	// don't delete the highlight
			m_pContainer->removeChild(node);
	}
	m_pContainer->Identity();
}


/////////////////////////////////////////////////////////////////////////////
// vtPlantInstanceArray3d
//

vtPlantInstanceArray3d::vtPlantInstanceArray3d()
{
	m_pHeightField = NULL;
	m_pPlantList = NULL;
}

vtPlantInstanceArray3d::~vtPlantInstanceArray3d()
{
	int i, num = m_Instances3d.GetSize();
	for (i = 0; i < num; i++)
	{
		vtPlantInstance3d *pi = m_Instances3d[i];
		delete pi;
	}
}

vtPlantInstance3d *vtPlantInstanceArray3d::GetInstance3d(uint i) const
{
	if (i < 0 || i >= m_Instances3d.GetSize())
		return NULL;
	return m_Instances3d.GetAt(i);
}

int vtPlantInstanceArray3d::CreatePlantNodes(bool progress_dialog(int))
{
	uint i, size = GetNumEntities();
	int created = 0;
	m_iOffTerrain = 0;

	m_Instances3d.SetSize(size);
	for (i = 0; i < size; i++)
	{
		// Clear value first, in case it doesn't construct.
		m_Instances3d.SetAt(i, NULL);

		if (CreatePlantNode(i))
			created++;

		if (progress_dialog != NULL)
			progress_dialog(i * 100 / size);
	}
	return created;
}

bool vtPlantInstanceArray3d::CreatePlantNode(uint i)
{
	// If it was already constructed, destruct so we can build again
	ReleasePlantGeometry(i);

	if (!m_pPlantList)
		return false;

	DPoint2 pos = GetPoint(i);
	float size;
	short species_id;
	GetPlant(i, size, species_id);

	if (!m_pHeightField->ContainsEarthPoint(pos))
	{
		m_iOffTerrain++;
		return false;
	}

	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (!inst3d)
	{
		inst3d = new vtPlantInstance3d;
		m_Instances3d.SetAt(i, inst3d);
	}

	vtPlantSpecies3d *ps = GetPlantList()->GetSpecies(species_id);
	if (!ps)
		return false;

	vtPlantAppearance3d *pApp = ps->GetAppearanceByHeight(size);
	if (!pApp)
		return false;

	// confirm that it is loaded and ready to use
	pApp->LoadAndCreate();

	if (!inst3d->m_pContainer)
		inst3d->m_pContainer = new vtTransform;

	pApp->GenerateGeom(inst3d->m_pContainer);

	UpdateTransform(i);

	// We need to scale the model to produce the desired size, not the
	//  size of the appearance but of the instance.
	float scale = size / pApp->m_height;
	inst3d->m_pContainer->Scale(scale);

	if (pApp->m_eType == AT_BILLBOARD)
	{
		// Since the billboard are symmetric, a small rotation helps provide
		//  a more natural look.
		float random_rotation = random(PI2f);
		inst3d->m_pContainer->RotateLocal(FPoint3(0,1,0), random_rotation);
	}

	return true;
}

void vtPlantInstanceArray3d::ReleasePlantGeometry(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (inst3d)
	{
		inst3d->ReleaseContents();
	}
}

vtTransform *vtPlantInstanceArray3d::GetPlantNode(uint i) const
{
	if (i >= m_Instances3d.GetSize())
		return NULL;

	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (inst3d)
		return inst3d->m_pContainer;
	return NULL;
}

void vtPlantInstanceArray3d::VisualDeselectAll()
{
	uint size = GetNumEntities();

	for (uint i = 0; i < size; i++)
	{
		vtPlantInstance3d *inst3d = GetInstance3d(i);
		if (inst3d)
		{
			Select(i, false);
			inst3d->ShowBounds(false);
		}
	}
}

void vtPlantInstanceArray3d::VisualSelect(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (inst3d && inst3d->m_pContainer != NULL)
	{
		Select(i, true);
		inst3d->ShowBounds(true);
	}
}

void vtPlantInstanceArray3d::OffsetSelectedPlants(const DPoint2 &offset)
{
	uint size = GetNumEntities();
	for (uint i = 0; i < size; i++)
	{
		if (!IsSelected(i))
			continue;

		SetPoint(i, GetPoint(i) + offset);
		UpdateTransform(i);
	}
}

void vtPlantInstanceArray3d::UpdateTransform(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);

	FPoint3 p3;
	m_pHeightField->ConvertEarthToSurfacePoint(GetPoint(i), p3);

	// We should really move the plant to a new cell in the LOD
	// Grid, but unless it's moving really far we don't need to
	// worry about this.

	inst3d->m_pContainer->SetTrans(p3);
}


//
// Note you must remove the plant from the scene graph before deleting it!
//
void vtPlantInstanceArray3d::DeletePlant(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);

	// get rid of the instance
	SetToDelete(i);
	ApplyDeletion();

	// and its 3D component
	m_Instances3d.RemoveAt(i);
	delete inst3d;
}

bool vtPlantInstanceArray3d::FindPlantFromNode(osg::Node *pNode, int &iOffset)
{
	bool bFound = false;
	uint i, j;

	for (i = 0; (i < m_Instances3d.GetSize()) & !bFound; i++)
	{
		vtTransform *pTransform = GetPlantNode(i);
		if (!pTransform)	// safety check
			continue;
		for (j = 0; (j < pTransform->getNumChildren()) && !bFound; j++)
		{
			osg::Node *pPlantNode = pTransform->getChild(j);
			if (pPlantNode != GetInstance3d(i)->m_pHighlight)
			{
				if (pPlantNode == pNode)
				{
					bFound = true;
					iOffset = i;
				}
			}
		}
	}
	return bFound;
}

