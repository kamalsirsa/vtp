//
// Structure3d.cpp
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <fstream>

#include "xmlhelper/easyxml.hpp"
#include "vtlib/vtlib.h"
#include "vtdata/vtlog.h"

#include "Structure3d.h"
#include "Building3d.h"
#include "Fence3d.h"
#include "Terrain.h"

const vtMaterialName BMAT_NAME_HIGHLIGHT = _T("Highlight");

// Static memebers
vtMaterialArray vtStructure3d::s_Materials;
vtMaterialDescriptorArray vtStructure3d::s_MaterialDescriptors;
bool vtStructure3d::s_MaterialArraysInitialised = false;

// There is a single array of materials, shared by all buildings.
// This is done to save memory.  For a list of 16000+ buildings, this can
//  save about 200MB of RAM.
RGBf vtStructure3d::s_Colors[COLOR_SPREAD];

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

	// try to work around 3DS coordinate axes difference problem
	vtString fname2 = GetValue("filename");
	int dot = fname2.ReverseFind('.');
	if (dot > 0)
	{
		vtString ext = fname2.Mid(dot+1);
		if (ext.CompareNoCase("3ds") == 0)
		{
			// Must rotate by 90 degrees for 3DS MAX -> OpenGL
			m_pContainer->Rotate2(FPoint3(1.0f, 0.0f, 0.0f), -PID2f);
		}
	}

	if (m_fRotation != 0.0f)
		m_pContainer->Rotate2(FPoint3(0,1,0), m_fRotation);

	FPoint3 surface_pos;
	pHeightField->ConvertEarthToSurfacePoint(m_p, surface_pos);
	m_pContainer->SetTrans(surface_pos);
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
	if (!m_pContainer)
	{
		// constructing for the first time
		m_pContainer = new vtTransform();
	}

	// if previously created, destroy to re-create
	if (m_pModel)
	{
		m_pModel->Destroy();
		m_pModel = NULL;
	}

	const char *filename = GetValue("filename");
	if (filename)
	{
		// relative path: look on the standards data paths
		vtString fullpath = FindFileOnPaths(vtTerrain::m_DataPaths, filename);
		if (fullpath != "")
		{
			m_pModel = vtLoadModel(fullpath);
			if (m_pModel)
				SetValue("filename", fullpath);
		}
		if (!m_pModel)
			return false;
	}
	const char *itemname = GetValue("itemname");
	if (itemname)
	{
		// Use ContentManager to create the structure
		vtItem *pItem = vtTerrain::s_Content.FindItemByName(itemname);
		if (pItem)
		{
			m_pModel = vtTerrain::s_Content.CreateInstanceOfItem(pItem);
		}
		if (!m_pModel)
			return false;
	}
	m_pContainer->AddChild(m_pModel);

	const char *scale = GetValue("scale");
	if (scale)
	{
		double sc = atof(scale);
		if (sc != 0.0)
			m_fScale = sc;
	}

	UpdateTransform(pTerr->GetHeightField());
	return true;
}


///////////////////////////////////////////////////////////////////////
//
// vtStructureArray3d
//

vtBuilding *vtStructureArray3d::NewBuilding()
{
	// Make sure that subsequent operations on this building are done in with
	// the correct local coordinate system
	vtBuilding::s_Conv.Setup(m_proj.GetUnits(), DPoint2(0, 0));

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
	return NULL;
}

void vtStructureArray3d::OffsetSelectedStructures(const DPoint2 &offset)
{
	vtStructure *str;
	for (int i = 0; i < GetSize(); i++)
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
			vtFence3d *fen = GetFence(i);
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
	for (int i = 0; i < GetSize(); i++)
	{
		vtStructure *str = (vtStructure *) GetAt(i);
		vtStructure3d *str3d = GetStructure3d(i);

		str->Select(false);
		str3d->ShowBounds(false);
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
		m_pEditBuilding->SetValue("level", m_iEditLevel);
		m_pEditBuilding->SetValue("edge", m_iEditEdge);
		str2 = (vtStructure3d *) (vtBuilding3d *) m_pEditBuilding;
		ConstructStructure(str2);
	}
}


void vtStructureArray3d::DestroyStructure(int i)
{
	// Need to destroy the 3d geometry for this structure
	vtStructure3d *st3d = GetStructure3d(i);
	st3d->DeleteNode();
}

// Methods for vtStructure3d
void vtStructure3d::InitializeMaterialArrays()
{
	vtString path;
	int i, j, k;
	RGBf color;
	vtMaterial *pMat;
	int count = 0;
	int divisions = 6;
	float start = .25f;
	float step = (1.0f-start)/(divisions-1);
	int iSize;
	vtImage *pImage;
	//
	if (s_MaterialArraysInitialised)
		return;


	// set up colour spread
	for (i = 0; i < divisions; i++) {
		for (j = 0; j < divisions; j++) {
			for (k = 0; k < divisions; k++) {
				s_Colors[count++].Set(start+i*step, start+j*step, start+k*step);
			}
		}
	}

	s_Materials.SetMaxSize(500);
/*
	// Uncooment this to make a default textures file
	// remember to create an empty file in the correct place first (for FindFileOnPaths)

	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_PLAIN),
									"",
									VT_MATERIAL_COLOURED,
									1.0f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_SIDING),
									"BuildingModels/siding64.jpg",
									VT_MATERIAL_COLOURABLE_TEXTURE,
									1.0f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_WINDOW),
									"BuildingModels/window.jpg",
									VT_MATERIAL_SELFCOLOURED_TEXTURE,
									1.0f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_DOOR),
									"BuildingModels/door.jpg",
									VT_MATERIAL_SELFCOLOURED_TEXTURE,
									1.0f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_WOOD),
									"BuildingModels/wood1_256.jpg",
									VT_MATERIAL_SELFCOLOURED_TEXTURE,
									0.6f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_CEMENT),
									"BuildingModels/cement_block1_256.jpg",
									VT_MATERIAL_SELFCOLOURED_TEXTURE,
									1.0f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_BRICK),
									"BuildingModels/brick1_256.jpg",
									VT_MATERIAL_BRICK,
									0.8f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_BRICK),
									"BuildingModels/brick2_256.jpg",
									VT_MATERIAL_BRICK,
									0.8f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_PAINTED_BRICK),
									"BuildingModels/brick_mono_256.jpg",
									VT_MATERIAL_COLOURABLE_TEXTURE,
									0.8f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_ROLLED_ROOFING),
									"BuildingModels/roofing1_256.jpg",
									VT_MATERIAL_COLOURABLE_TEXTURE,
									1.0f));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_WINDOWWALL),
									"BuildingModels/window_wall128.jpg",
									VT_MATERIAL_COLOURABLE_TEXTURE,
									1.0f,
									false));
	s_MaterialDescriptors.Append(new vtMaterialDescriptor(g_MaterialNames.FindOrAppendMaterialName(BMAT_NAME_HIGHLIGHT),
									"",
									VT_MATERIAL_HIGHLIGHT,
									1.0f,
									false));

	{
		std::ofstream ops(FindFileOnPaths(vtTerrain::m_DataPaths, "GeoTypical/textures.xml"));

		ops << s_MaterialDescriptors;

		ops.close();
	}
*/

	s_MaterialDescriptors.Load(FindFileOnPaths(vtTerrain::m_DataPaths, "GeoTypical/textures.xml"));

	iSize = s_MaterialDescriptors.GetSize();

	for (j = 0; j < iSize; j++)
	{
		vtMaterialDescriptor& Descriptor = *s_MaterialDescriptors.GetAt(j);

		switch(Descriptor.GetMaterialType())
		{
			case VT_MATERIAL_COLOURED:
				for (i = 0; i < COLOR_SPREAD; i++)
				{
					pMat = MakeMaterial(s_Colors[i], true);
					if (i == 0)
						Descriptor.SetMaterialIndex(s_Materials.AppendMaterial(pMat));
					else
						s_Materials.AppendMaterial(pMat);
				}
				break;

			case VT_MATERIAL_SELFCOLOURED_TEXTURE:
			case VT_MATERIAL_BRICK:
				color.Set(1.0f, 1.0f, 1.0f);
				pMat = MakeMaterial(color, true);
				path = FindFileOnPaths(vtTerrain::m_DataPaths, Descriptor.GetSourceName());
				pMat->SetTexture2(path);
				pMat->SetClamp(false);
				Descriptor.SetMaterialIndex(s_Materials.AppendMaterial(pMat));
				break;

			case VT_MATERIAL_COLOURABLE_TEXTURE:
				path = FindFileOnPaths(vtTerrain::m_DataPaths, Descriptor.GetSourceName());
				pImage = new vtImage(path);
				if (NULL == pImage)
					throw "Out of memory";
				divisions = 6;
				start = .25f;
				step = (1.0f-start)/(divisions-1);
				for (i = 0; i < COLOR_SPREAD; i++)
				{
					pMat = MakeMaterial(s_Colors[i], true);
					pMat->SetTexture(pImage);
					pMat->SetClamp(false);
					if (i == 0)
						Descriptor.SetMaterialIndex(s_Materials.AppendMaterial(pMat));
					else
						s_Materials.AppendMaterial(pMat);
				}
				break;
			case VT_MATERIAL_HIGHLIGHT:
				Descriptor.SetMaterialIndex(s_Materials.AddRGBMaterial1(RGBf(1,1,1), false, false, true));
				s_Materials.AddRGBMaterial1(RGBf(1,0,0), false, false, true);
				break;
		}
	}
/*
	int total = s_Materials.GetSize();
	// window, door, wood, cement_block, windowwall
	int expectedtotal = COLOR_SPREAD + COLOR_SPREAD +	// plain, siding
		1 + 1 + 1 + 1 +		// window, door, wood, cement
		1 + 1 +				// brick1, brick2
		COLOR_SPREAD + COLOR_SPREAD +	// painted brick, window-wall
		COLOR_SPREAD +		// rolled roofing
		1 + 1;				// highlight colors
	assert(total == expectedtotal);
*/
	s_MaterialArraysInitialised = true;
}

//
// Takes the building material and color, and tries to find the closest
// existing vtMaterial.
//
int vtStructure3d::FindMatIndex(const vtMaterialName& Material, RGBi inputColor)
{
	if (!s_MaterialArraysInitialised)
		return -1;
	
	vtMaterialDescriptor const *pMaterialDescriptor = FindMaterialDescriptor(Material);
	if (pMaterialDescriptor == NULL)
		return -1;
	int iIndex = pMaterialDescriptor->GetMaterialIndex();
	vtMaterialTypeEnum Type = pMaterialDescriptor->GetMaterialType();

	if (Type == VT_MATERIAL_SELFCOLOURED_TEXTURE)
		return iIndex;

	if (Type == VT_MATERIAL_BRICK)
	{
		// choose one of our (currently 2) unpainted brick textures
		RGBi b1(159, 100, 83);	// (reddish medium brown)
		RGBi b2(183, 178, 171);	// (slightly pinkish grey)
		if (ColorDiff(inputColor, b1) < ColorDiff(inputColor, b2))
			return iIndex;
		else
			return iIndex + 1;
	}

	if (Type == VT_MATERIAL_HIGHLIGHT)
	{
		// Choose the correct highlight
		if (inputColor == RGBi(255,255,255))
			return iIndex;
		else
			return iIndex + 1;
	}

	// match the closest color.
	float bestError = 1E8;
	int bestMatch = -1;
	float error;

	for (int i = 0; i < COLOR_SPREAD; i++)
	{
		error = ColorDiff(s_Colors[i], inputColor);
		if (error < bestError)
		{
			bestMatch  = iIndex + i;
			bestError = error;
		}
	}
	return bestMatch;
}

vtMaterialDescriptor *vtStructure3d::FindMaterialDescriptor(const vtMaterialName& MaterialName)
{
	int iIndex;
	int iSize = s_MaterialDescriptors.GetSize();

	for (iIndex = 0;  iIndex < iSize; iIndex++)
	{
		vtMaterialDescriptor *ptr = s_MaterialDescriptors.GetAt(iIndex);
		if (ptr->GetName() == MaterialName)
			return ptr;
	}
	return NULL; 
}


//
// Helper to make a material
//
vtMaterial *vtStructure3d::MakeMaterial(RGBf &color, bool culling)
{
	vtMaterial *pMat = new vtMaterial();
	pMat->SetDiffuse1(color * 0.7f);
	pMat->SetAmbient1(color * 0.4f);
	pMat->SetSpecular2(0.0f);
	pMat->SetCulling(culling);
	pMat->SetLighting(true);
	return pMat;
}

// Linear distance in RGB space
float vtStructure3d::ColorDiff(const RGBi &c1, const RGBi &c2)
{
	FPoint3 diff;
	diff.x = (c1.r - c2.r);
	diff.y = (c1.g - c2.g);
	diff.z = (c1.b - c2.b);
	return diff.Length();
}

// XML parser for MaterialDescriptorArray

class MaterialDescriptorArrayVisitor : public XMLVisitor
{
public:
	MaterialDescriptorArrayVisitor(vtMaterialDescriptorArray *MDA) : m_state(0), m_pMDA(MDA) {}
	void startXML() { m_state = 0; }
	void endXML() { m_state = 0; }
	void startElement(const char *name, const XMLAttributes &atts);

private:
	int m_state;

	vtMaterialDescriptorArray *m_pMDA;
};

void MaterialDescriptorArrayVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	if (m_state == 0 && !strcmp(name, "MaterialDescriptorArray"))
		m_state = 1;
	else if (m_state == 1)
	{
		if (!strcmp(name, "MaterialDescriptor"))
		{
			vtMaterialDescriptor *pDescriptor = new vtMaterialDescriptor;
			if (NULL == pDescriptor)
				throw "Out of memory";
			attval = atts.getValue("Name");
			if (attval)
				pDescriptor->SetName(g_MaterialNames.FindOrAppendMaterialName(vtMaterialName(attval)));
			attval = atts.getValue("Type");
			if (attval)
				pDescriptor->SetMaterialType((vtMaterialTypeEnum)atoi(attval));
			attval = atts.getValue("Source");
			if (attval)
				pDescriptor->SetSourceName(attval);
			attval = atts.getValue("Scale");
			if (attval)
				pDescriptor->SetUVScale((float)atof(attval));
			attval = atts.getValue("UIVisible");
			if (attval)
				pDescriptor->SetUIVisible(0 == atoi(attval)? false : true);
			attval = atts.getValue("RGB");
			if (attval)
			{
				short r, g, b;
				sscanf(attval, "%hu %hu %hu", &r, &g, &b);
				pDescriptor->SetRGB(RGBi(r, g, b));
			}
			m_pMDA->Append(pDescriptor);
		}
	}
}

// Methods for MaterialDescriptorArray

bool vtMaterialDescriptorArray::Load(const char *FileName)
{
	MaterialDescriptorArrayVisitor Visitor(this);
	try
	{
		readXML(FileName, Visitor);
	}
	catch (xh_exception &e)
	{
		// TODO: would be good to pass back the error message.
		VTLOG("vtMaterialDescriptorArray::Load xml error %s\n", e.getMessage());
		return false;
	}
	return true;
}


