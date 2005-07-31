//
// Structure3d.cpp
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include <fstream>
#include "vtdata/vtLog.h"

#include "Structure3d.h"
#include "Building3d.h"
#include "Fence3d.h"
#include "Terrain.h"
#include "TerrainScene.h"

const vtString BMAT_NAME_HIGHLIGHT = "Highlight";

// Static members
vtMaterialDescriptorArray3d vtStructure3d::s_MaterialDescriptors;


// Helper: Linear distance in RGB space
float ColorDiff(const RGBi &c1, const RGBi &c2)
{
	FPoint3 diff;
	diff.x = (float) (c1.r - c2.r);
	diff.y = (float) (c1.g - c2.g);
	diff.z = (float) (c1.b - c2.b);
	return diff.Length();
}

///////////////////////////////////////////////////////////////////////

vtStructInstance3d::vtStructInstance3d() : vtStructInstance()
{
	m_pContainer = NULL;
	m_pHighlight = NULL;
	m_pModel = NULL;
}

void vtStructInstance3d::UpdateTransform(vtHeightField3d *pHeightField)
{
	m_pContainer->Identity();

	if (m_fScale != 1.0f)
		m_pContainer->Scale3(m_fScale, m_fScale, m_fScale);

	if (m_fRotation != 0.0f)
		m_pContainer->Rotate2(FPoint3(0,1,0), m_fRotation);

	FPoint3 point;
	pHeightField->ConvertEarthToSurfacePoint(m_p, point);
	if (m_bAbsolute)
		// Absolute means elevation is relative to sealevel
		point.y = m_fElevationOffset;
	else
		// Elevation Offset is relative to the terrain surface
		point.y += m_fElevationOffset;

	m_pContainer->SetTrans(point);
}

void vtStructInstance3d::Reload()
{
	// Is this method necessary?  Or can the object simply be reconstructed
	//  to produce the same effect?
}

/**
 * Display some bounding wires around the object to highlight it.
 */
void vtStructInstance3d::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			// get bounding sphere
			FSphere sphere;
			m_pModel->GetBoundSphere(sphere);

			m_pHighlight = CreateBoundSphereGeom(sphere);
			m_pContainer->AddChild(m_pHighlight);
		}
		m_pHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}

// implement vtStructure3d methods
bool vtStructInstance3d::CreateNode(vtTerrain *pTerr)
{
	// if previously created, destroy to re-create
	bool bRecreating = false;
	if (m_pModel)
	{
		bRecreating = true;
		m_pContainer->RemoveChild(m_pModel);
		m_pModel->Release();
		m_pModel = NULL;
	}

	const char *filename = GetValueString("filename", true, true);
	if (filename)
	{
		// relative path: look on the standards data paths
		vtString fullpath = FindFileOnPaths(vtGetDataPath(), filename);
		if (fullpath == "")
		{
			// also look in BuildingModels on data path
			vtString prefix = "BuildingModels/";
			fullpath = FindFileOnPaths(vtGetDataPath(), prefix+filename);
		}
		if (fullpath == "")
		{
			// still can't find it - give up.
			VTLOG("Couldn't find file '%s'\n", filename);
			return false;
		}
		// If they are re-creating this object, it's probably because
		//  the object has changed on disk, so force a reload
		bool bForce = bRecreating;

		m_pModel = vtNode::LoadModel(fullpath, !bForce);
		if (m_pModel)
			SetValueString("filename", fullpath);
		else
		{
			VTLOG("Couldn't load model from file '%s'\n", filename);
			return false;
		}
	}
	const char *itemname = GetValueString("itemname", false, true);
	if (itemname)
	{
		// Use ContentManager to create the structure, using the
		//  terrain's specific content manager
		m_pModel = pTerr->m_Content.CreateNodeFromItemname(itemname);

		// Also try the global content manager
		if (!m_pModel)
			m_pModel = vtGetContent().CreateNodeFromItemname(itemname);

		if (!m_pModel)
			return false;
	}
	if (!m_pContainer)
	{
		// constructing for the first time
		m_pContainer = new vtTransform();
		m_pContainer->SetName2("instance container");
	}
	m_pContainer->AddChild(m_pModel);

	float sc;
	if (GetValueFloat("scale", sc))
		m_fScale = sc;

	UpdateTransform(pTerr->GetHeightField());
	return true;
}

void vtStructInstance3d::DeleteNode()
{
	if (m_pContainer)
	{
		if (m_pModel)
		{
			m_pContainer->RemoveChild(m_pModel);
			m_pModel->Release();
			m_pModel = NULL;
		}
		m_pContainer->Release();
		m_pContainer = NULL;
	}
}

double vtStructInstance3d::DistanceToPoint(const DPoint2 &p, float fMaxRadius) const
{
	if (!m_pModel)
		return 1E9;	// Ignore instances that have no model

	if (m_pContainer && m_pModel)
	{
		// If we have the 3D model already loaded, we can return distance
		//  from the given point to the edge of the bounding sphere.  This
		//  makes objects easier to select, because their selectable zone
		//  is larger for larger objects.  This is a little messy, because
		//  it's a world-coord operation applied to a earth-coord result.
		FSphere sphere;
		m_pModel->GetBoundSphere(sphere);
		FPoint3 trans = m_pContainer->GetTrans();
		sphere.center += trans;
		if (sphere.radius < fMaxRadius)
		{
			DPoint2 ecenter;
			DPoint2 evector;
			g_Conv.ConvertToEarth(sphere.center.x, sphere.center.z, ecenter);
			g_Conv.ConvertVectorToEarth(sphere.radius, 0, evector);
			double dist = (ecenter - p).Length();
			return (dist - evector.x);
		}
		else
			return 1E9;	// Ignore instances with such a large radius
	}
	// otherwise, simple distance from the origin of this instance to the given point
	return vtStructInstance::DistanceToPoint(p, fMaxRadius);
}

///////////////////////////////////////////////////////////////////////
//
// vtStructureArray3d
//

vtStructureArray3d::vtStructureArray3d() : vtStructureArray()
{
	m_pTerrain = NULL;
}

vtBuilding *vtStructureArray3d::NewBuilding()
{
	// Make sure that subsequent operations on this building are done in with
	// the correct local coordinate system
	vtBuilding::s_Conv.Setup(m_proj.GetUnits(), DRECT(0,1,1,0));

	return new vtBuilding3d;
}

vtFence *vtStructureArray3d::NewFence()
{
	return new vtFence3d;
}

vtStructInstance *vtStructureArray3d::NewInstance()
{
	return new vtStructInstance3d;
}

vtStructure3d *vtStructureArray3d::GetStructure3d(int i)
{
	vtStructure *str = GetAt(i);

	// Due to the somewhat complicated structure of the multiple inheritance
	// here, we must do a double-cast: first cast down to the object's true
	// type, then back up to vtStructure3d.

	if (str->GetType() == ST_BUILDING)
		return (vtStructure3d *) (vtBuilding3d *) str;

	if (str->GetType() == ST_LINEAR)
		return (vtStructure3d *) (vtFence3d *) str;

	if (str->GetType() == ST_INSTANCE)
		return (vtStructure3d *) (vtStructInstance3d *) str;

	return NULL;
}

bool vtStructureArray3d::ConstructStructure(vtStructure3d *str)
{
	return str->CreateNode(m_pTerrain);
}

bool vtStructureArray3d::ConstructStructure(int index)
{
	vtStructure3d *str = GetStructure3d(index);
	if (str)
		return str->CreateNode(m_pTerrain);
	return false;
}

void vtStructureArray3d::OffsetSelectedStructures(const DPoint2 &offset)
{
	vtStructure *str;
	for (unsigned int i = 0; i < GetSize(); i++)
	{
		str = GetAt(i);
		if (!str->IsSelected())
			continue;
		if (str->GetType() == ST_BUILDING)
		{
			vtBuilding3d *bld = GetBuilding(i);
			bld->Offset(offset);
			bld->AdjustHeight(m_pTerrain->GetHeightField());

			// Should really move the building to a new cell in the LOD
			// Grid, but unless it's moving really far we don't need to
			// worry about this.
		}
		if (str->GetType() == ST_LINEAR)
		{
			// vtFence3d *fen = GetFence(i);
			// TODO: implement moving of fences?
		}
		if (str->GetType() == ST_INSTANCE)
		{
			vtStructInstance3d *inst = GetInstance(i);
			inst->Offset(offset);
			inst->UpdateTransform(m_pTerrain->GetHeightField());
		}
	}
}

void vtStructureArray3d::VisualDeselectAll()
{
	for (unsigned int i = 0; i < GetSize(); i++)
	{
		vtStructure *str = (vtStructure *) GetAt(i);
		vtStructure3d *str3d = GetStructure3d(i);

		str->Select(false);
		str3d->ShowBounds(false);
	}
}

void vtStructureArray3d::SetEnabled(bool bTrue)
{
	for (unsigned int j = 0; j < GetSize(); j++)
	{
		vtStructure3d *str3d = GetStructure3d(j);
		if (str3d)
		{
			vtNode *pThing = str3d->GetContained();
			if (pThing)
				pThing->SetEnabled(bTrue);
		}
	}
}

//
// Be informed of edit hightlighting
//
void vtStructureArray3d::SetEditedEdge(vtBuilding *bld, int lev, int edge)
{
	vtStructure3d *str1, *str2;

	if (m_pEditBuilding && m_pEditBuilding != bld)
	{
		m_pEditBuilding->RemoveTag("level");
		m_pEditBuilding->RemoveTag("edge");
		str1 = (vtStructure3d *) (vtBuilding3d *) m_pEditBuilding;
		ConstructStructure(str1);
	}

	vtStructureArray::SetEditedEdge(bld, lev, edge);

	if (m_pEditBuilding)
	{
		m_pEditBuilding->SetValueInt("level", m_iEditLevel);
		m_pEditBuilding->SetValueInt("edge", m_iEditEdge);
		str2 = (vtStructure3d *) (vtBuilding3d *) m_pEditBuilding;
		ConstructStructure(str2);
	}
}


void vtStructureArray3d::DestroyStructure(int i)
{
	// Need to destroy the 3D geometry for this structure
	vtStructure3d *st3d = GetStructure3d(i);
	st3d->DeleteNode();
}


/////////////////////////////////////////////////////////////////////////////
// Methods for vtMaterialDescriptorArray3d
//

vtMaterialDescriptorArray3d::vtMaterialDescriptorArray3d()
{
	m_pMaterials = NULL;
	m_bMaterialsCreated = false;
}

void vtMaterialDescriptorArray3d::InitializeMaterials()
{
	if (m_pMaterials != NULL)	// already initialized
		return;

	VTLOG("Initializing MaterialDescriptorArray3d\n");

	int i, j, k;
	RGBf color;
	int count = 0;
	int divisions = 6;
	float start = .25f;
	float step = (1.0f-start)/(divisions-1);

	// set up colour spread
	for (i = 0; i < divisions; i++) {
		for (j = 0; j < divisions; j++) {
			for (k = 0; k < divisions; k++) {
				m_Colors[count++].Set(start+i*step, start+j*step, start+k*step);
			}
		}
	}

	m_pMaterials = new vtMaterialArray();
	m_pMaterials->SetMaxSize(500);

	// Create internal materials (only needed by vtlib, not vtdata)
	m_hightlight1 = m_pMaterials->AddRGBMaterial1(RGBf(1,1,1), false, false, true);
	m_hightlight2 = m_pMaterials->AddRGBMaterial1(RGBf(1,0,0), false, false, true);
}

void vtMaterialDescriptorArray3d::CreateMaterials()
{
	VTLOG("Creating Materials:\n");
	clock_t clock1 = clock();

	m_bMaterialsCreated = true;

	vtMaterial *pMat;
	int i, j, iSize = GetSize();

	for (j = 0; j < iSize; j++)
	{
		vtMaterialDescriptor *descriptor = GetAt(j);
		VTLOG(" %s,", (const char *) descriptor->GetName());

		switch (descriptor->GetColorable())
		{
		case VT_MATERIAL_COLOURED:
			for (i = 0; i < COLOR_SPREAD; i++)
			{
				pMat = MakeMaterial(descriptor, m_Colors[i]);
				if (i == 0)
					descriptor->SetMaterialIndex(m_pMaterials->AppendMaterial(pMat));
				else
					m_pMaterials->AppendMaterial(pMat);
			}
			break;

		case VT_MATERIAL_SELFCOLOURED_TEXTURE:
			CreateSelfColoredMaterial(descriptor);
			break;

		case VT_MATERIAL_COLOURABLE_TEXTURE:
			CreateColorableMaterial(descriptor);
			break;
		}
	}
	clock_t clock2 = clock();
	VTLOG(" done in %.3f seconds.\n", (float)(clock2-clock1)/CLOCKS_PER_SEC);
}

void vtMaterialDescriptorArray3d::CreateSelfColoredMaterial(vtMaterialDescriptor *descriptor)
{
	RGBf color(1.0f, 1.0f, 1.0f);
	vtMaterial *pMat = MakeMaterial(descriptor, color);

	vtString path = FindFileOnPaths(vtGetDataPath(), descriptor->GetSourceName());
	pMat->SetTexture2(path);
	pMat->SetClamp(false);	// material needs to repeat

	if (descriptor->GetBlending())
		pMat->SetTransparent(true);

	descriptor->SetMaterialIndex(m_pMaterials->AppendMaterial(pMat));
}

void vtMaterialDescriptorArray3d::CreateColorableMaterial(vtMaterialDescriptor *descriptor)
{
	vtString source = descriptor->GetSourceName();
	vtString path = FindFileOnPaths(vtGetDataPath(), source);
	if (path == "")
	{
		VTLOG("\n\tMissing texture: %s\n", (const char *) source);
		return;
	}

	for (int i = 0; i < COLOR_SPREAD; i++)
	{
		vtMaterial *pMat = MakeMaterial(descriptor, m_Colors[i]);
		pMat->SetTexture2(path);
		pMat->SetMipMap(true);
		pMat->SetClamp(false);
		int index = m_pMaterials->AppendMaterial(pMat);
		if (i == 0)
			descriptor->SetMaterialIndex(index);
	}
}


//
// Takes the building material and color, and tries to find the closest
// existing vtMaterial.
//
int vtMaterialDescriptorArray3d::FindMatIndex(const vtString& Material,
											  const RGBf &inputColor)
{
	if (!m_bMaterialsCreated)
	{
		// postpone material creation until the first time they're needed
		CreateMaterials();
	}

	// handle special case of internal materials
	if (Material == "Highlight")
	{
		// Choose the correct highlight
		if (inputColor == RGBf(1,1,1))
			return m_hightlight1;
		else
			return m_hightlight2;
	}

	const vtMaterialDescriptor  *pMaterialDescriptor;
	pMaterialDescriptor = FindMaterialDescriptor(Material, inputColor);

	if (pMaterialDescriptor == NULL)
		return -1;
	int iIndex = pMaterialDescriptor->GetMaterialIndex();
	vtMaterialColorEnum Type = pMaterialDescriptor->GetColorable();

	if (Type == VT_MATERIAL_SELFCOLOURED_TEXTURE)
		return iIndex;

	// otherwise, it is of type VT_MATERIAL_COLOURED or VT_MATERIAL_COLOURABLE_TEXTURE
	// match the closest color.
	float bestError = 1E8;
	int bestMatch = -1;
	float error;

	for (int i = 0; i < COLOR_SPREAD; i++)
	{
		error = ColorDiff(m_Colors[i], inputColor);
		if (error < bestError)
		{
			bestMatch  = iIndex + i;
			bestError = error;
		}
	}
	return bestMatch;
}

vtMaterialDescriptor *vtMaterialDescriptorArray3d::FindMaterialDescriptor(const vtString& MaterialName,
																		  const RGBf &color)
{
	if (&MaterialName == NULL)
		return NULL;

	float bestError = 1E8;
	int bestMatch = -1;
	float error;

	vtMaterialDescriptor *desc;
	int i, iSize = GetSize();
	for (i = 0; i < iSize; i++)
	{
		desc = GetAt(i);
		if (desc->GetName() != MaterialName)
			continue;

		// look for matching name with closest color
		const RGBi rgb = desc->GetRGB();
		error = ColorDiff(rgb, color);
		if (error < bestError)
		{
			bestMatch  = i;
			bestError = error;
		}
	}
	if (bestMatch != -1)
		return GetAt(bestMatch);
	return NULL;
}

void vtMaterialDescriptorArray3d::ReleaseMaterials()
{
	if (m_pMaterials)
	{
		m_pMaterials->Release();
		m_pMaterials = NULL;
		// do not free them - they were not dynamically allocated
		// FreeGlobalMaterials();
	}
}


/////////////////////////////////////////////////////////////////////////////
// Methods for vtStructure3d
//

bool vtStructure3d::s_bMaterialsLoaded = false;

void vtStructure3d::InitializeMaterialArrays()
{
	if (!s_bMaterialsLoaded)
	{
		s_bMaterialsLoaded = true;

		s_MaterialDescriptors.InitializeMaterials();

		// Now load external materials (user-modifiable, user-extendable)
		s_MaterialDescriptors.LoadExternalMaterials(vtGetDataPath());

		SetGlobalMaterials(&s_MaterialDescriptors);
	}
}

void vtStructure3d::ReleaseSharedMaterials()
{
	s_MaterialDescriptors.ReleaseMaterials();
	vtFence3d::s_FenceMats.ReleaseMaterials();
}

//
// Helper to make a material
//
vtMaterial *vtMaterialDescriptorArray3d::MakeMaterial(vtMaterialDescriptor *desc,
													  const RGBf &color)
{
	vtMaterial *pMat = new vtMaterial();
	if (desc->GetAmbient())
	{
		// a purely ambient material
		pMat->SetDiffuse(0,0,0);
		pMat->SetAmbient(1,1,1);
	}
	else
	{
		pMat->SetDiffuse1(color * 0.7f);
		pMat->SetAmbient1(color * 0.4f);
	}
	pMat->SetSpecular2(0.0f);
	pMat->SetCulling(!desc->GetTwoSided());
	pMat->SetLighting(true);
	return pMat;
}


