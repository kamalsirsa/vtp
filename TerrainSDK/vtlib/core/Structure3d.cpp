//
// Structure3d.cpp
//

#include "vtlib/vtlib.h"

#include "Structure3d.h"
#include "Building3d.h"
#include "Fence3d.h"


///////////////////////////////////////////////////////////////////////

vtStructInstance3d::vtStructInstance3d() : vtStructInstance()
{
	m_pContainer = NULL;
	m_pHighlight = NULL;
	m_pModel = NULL;
}

bool vtStructInstance3d::CreateShape(vtHeightField *pHeightField)
{
	if (!m_pContainer)
	{
		// constructing for the first time
		m_pContainer = new vtTransform();
	}

	const char *filename = GetValue("filename");
	if (filename)
	{
		m_pModel = vtLoadModel(filename);
		if (!m_pModel)
			return false;
		m_pContainer->AddChild(m_pModel);
	}
	const char *itemname = GetValue("itemname");
	if (itemname)
	{
		// TODO: use ContentManager to create model
	}

	UpdateTransform(pHeightField);
	return true;
}

void vtStructInstance3d::UpdateTransform(vtHeightField *pHeightField)
{
	m_pContainer->Identity();
	m_pContainer->Scale3(WORLD_SCALE, WORLD_SCALE, WORLD_SCALE);

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


///////////////////////////////////////////////////////////////////////

vtStructure3d::vtStructure3d() : vtStructure()
{
	m_pNode = NULL;
}

bool vtStructure3d::CreateNode(vtHeightField *hf, const char *options)
{
	if (m_type == ST_BUILDING)
	{
		vtBuilding3d *bld = (vtBuilding3d *) m_pBuilding;
		bool roof = (strstr(options, "roof") != NULL);
		bool walls = (strstr(options, "walls") != NULL);
		bool details = (strstr(options, "detail") != NULL);
		bld->CreateShape(hf, roof, walls, details);
		return true;
	}
	if (m_type == ST_FENCE)
	{
		vtFence3d *fen = (vtFence3d *) m_pFence;
		return fen->CreateShape(hf);
	}
	if (m_type == ST_INSTANCE)
	{
		vtStructInstance3d *inst = (vtStructInstance3d *) m_pInstance;
		bool bSuccess = inst->CreateShape(hf);
		if (!bSuccess && options != NULL)
		{
			// try again with the supplied path
			vtTag *tag = inst->FindTag("filename");
			if (tag)
			{
				vtString path = options;
				tag->value = path + tag->value;
			}
			// try again
			bSuccess = inst->CreateShape(hf);
		}
		return bSuccess;
	}
	return false;
}

vtTransform *vtStructure3d::GetTransform()
{
	if (m_type == ST_BUILDING)
	{
		vtBuilding3d *bld = (vtBuilding3d *) m_pBuilding;
		return bld->GetTransform();
	}
	if (m_type == ST_INSTANCE)
	{
		vtStructInstance3d *inst = (vtStructInstance3d *) m_pInstance;
		return inst->GetTransform();
	}
	return NULL;
}

vtGeom *vtStructure3d::GetGeom()
{
	if (m_type == ST_FENCE)
	{
		vtFence3d *fen = (vtFence3d *) m_pFence;
		return fen->GetGeom();
	}
	return NULL;
}

void vtStructure3d::DeleteNode()
{
	// not needed for buildings - those are automatically deleted when needed
	// is needed for fences:
	if (m_type == ST_FENCE)
	{
		vtFence3d *fen = (vtFence3d *) m_pFence;
		fen->DestroyGeometry();
	}
}

vtBuilding3d *vtStructure3d::GetBuilding()
{
	return (vtBuilding3d *) vtStructure::GetBuilding();
}

vtFence3d *vtStructure3d::GetFence()
{
	return (vtFence3d *) vtStructure::GetFence();
}

vtStructInstance3d *vtStructure3d::GetInstance()
{
	return (vtStructInstance3d *) vtStructure::GetInstance();
}

/////////////////////////////////////////////////////////////////////

vtBuilding *vtStructureArray3d::NewBuilding()
{
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

bool vtStructureArray3d::ConstructStructure(vtStructure3d *str,
													const char *options)
{
	return str->CreateNode(m_pHeightField, options);
}

void vtStructureArray3d::ReConstructStructure(vtStructure3d *str,
											  const char *options)
{
	str->DeleteNode();
	str->CreateNode(m_pHeightField, options);
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
			bld->AdjustHeight(m_pHeightField);

			// Should really move the building to a new cell in the LOD
			// Grid, but unless it's moving really far we don't need to
			// worry about this.
		}
		if (str->GetType() == ST_FENCE)
		{
			vtFence3d *fen = GetFence(i);
			// TODO
		}
		if (str->GetType() == ST_INSTANCE)
		{
			vtStructInstance3d *inst = GetInstance(i);
			inst->Offset(offset);
			inst->UpdateTransform(m_pHeightField);
		}
	}
}

void vtStructureArray3d::VisualDeselectAll()
{
	for (int i = 0; i < GetSize(); i++)
	{
		vtStructure3d *str = (vtStructure3d *) GetAt(i);
		str->Select(false);

		vtBuilding3d *bld = str->GetBuilding();
		if (bld)
			bld->ShowBounds(false);

		// TODO: visual highlighting of fences

		vtStructInstance3d *inst = str->GetInstance();
		if (inst)
			inst->ShowBounds(false);
	}
}

void vtStructureArray3d::VisualSelect(vtStructure3d *str)
{
	str->Select(true);

	vtBuilding3d *bld = str->GetBuilding();
	if (bld)
		bld->ShowBounds(true);

	// TODO: visual highlighting of fences

	vtStructInstance3d *inst = str->GetInstance();
	if (inst)
		inst->ShowBounds(true);
}


