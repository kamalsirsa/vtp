//
// Structure3d.cpp
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "Structure3d.h"
#include "Building3d.h"
#include "Fence3d.h"
#include "Terrain.h"


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
		m_pModel = vtLoadModel(filename);
		if (!m_pModel)
		{
			// try again, looking on the standards data paths
			vtString fullpath = FindFileOnPaths(vtTerrain::m_DataPaths, filename);
			if (fullpath != "")
			{
				// try again
				m_pModel = vtLoadModel(fullpath);
				if (m_pModel)
					SetValue("filename", fullpath);
			}
			if (!m_pModel)
				return false;
		}
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

