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
	m_pTransform = NULL;
}

bool vtStructInstance3d::CreateShape(vtHeightField *pHeightField)
{
	FPoint3 surface_pos;
	pHeightField->ConvertEarthToSurfacePoint(m_p, surface_pos);

	const char *filename = this->GetValue("filename");
	if (filename)
	{
		vtNode *model = vtLoadModel(filename);
		if (!model)
			return false;
		m_pTransform = new vtTransform();
		m_pTransform->AddChild(model);
		if (m_fRotation != 0.0f)
			m_pTransform->Rotate2(FPoint3(0,1,0), m_fRotation);
		m_pTransform->SetTrans(surface_pos);
		return true;
	}
	const char *itemname = this->GetValue("itemname");
	if (itemname)
	{
		// TODO: use ContentManager to create model
	}
	return true;
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
		return bld->CreateShape(hf, roof, walls, details);
	}
	if (m_type == ST_FENCE)
	{
		vtFence3d *fen = (vtFence3d *) m_pFence;
		return fen->CreateShape(hf);
	}
	if (m_type == ST_INSTANCE)
	{
		vtStructInstance3d *inst = (vtStructInstance3d *) m_pInstance;
		return inst->CreateShape(hf);
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

void vtStructureArray3d::OffsetSelectedBuildings(DPoint2 &offset)
{
	vtStructure *str;
	for (int i = 0; i < GetSize(); i++)
	{
		str = GetAt(i);
		if (!str->IsSelected())
			continue;
		if (str->GetType() != ST_BUILDING)
			continue;

		vtBuilding3d *bld = GetBuilding(i);
		bld->Offset(offset);
		bld->AdjustHeight(m_pHeightField);

		// Should really move the building to a new cell in the LOD Grid,
		// but unless it's moving really far we don't need to worry about this.
	}
}
