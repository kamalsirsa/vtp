//
// Name:	 ItemGroup.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "frame.h"
#include "ItemGroup.h"


ItemGroup::ItemGroup(vtItem *pItem)
{
	m_pItem = pItem;
}

void ItemGroup::CreateNodes()
{
	m_pAxes = NULL;
	m_pRulers = NULL;
	m_pGroup = new vtGroup();
	m_pLOD = new vtLOD();
	m_pTop = new vtGroup;
	m_pTop->SetName2("ItemGroupTop");
	m_pTop->AddChild(m_pLOD);
	m_pTop->AddChild(m_pGroup);
}

void ItemGroup::AttemptToLoadModels()
{
	int i, num_models = m_pItem->NumModels();

	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		vtTransform *trans = GetMainFrame()->m_nodemap[mod];
		if (!trans && !mod->m_attempted_load)
		{
			// haven't tried to load it yet
			GetMainFrame()->AttemptLoad(mod);
		}
	}
}

void ItemGroup::AttachModels(vtFont *font)
{
	// Undo previous attachments
	vtNode *pNode;
	while (pNode = m_pLOD->GetChild(0))
		m_pLOD->RemoveChild(pNode);
	while (pNode = m_pGroup->GetChild(0))
		m_pGroup->RemoveChild(pNode);

	// re-attach
	int i, num_models = m_pItem->NumModels();
	FSphere sph(FPoint3(0,0,0), 1.0f);
	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		vtNode *node = GetMainFrame()->m_nodemap[mod];
		if (node)
		{
			m_pGroup->AddChild(node);
			m_pLOD->AddChild(node);
			node->GetBoundSphere(sph);
		}
	}
	UpdateCrosshair(sph);
	UpdateRulers(font, sph);
}

void ItemGroup::UpdateCrosshair(const FSphere &sph)
{
	// Update origin crosshair
	if (m_pAxes)
	{
		m_pTop->RemoveChild(m_pAxes);
		m_pAxes->Release();
	}
	float size = sph.radius * 2;
	m_pAxes = Create3DCursor(size, size/100, 0.4f);
	m_pAxes->SetName2("Origin Axes");
	m_pTop->AddChild(m_pAxes);
}

void ItemGroup::UpdateRulers(vtFont *font, const FSphere &sph)
{
	// Update rulers
	if (m_pRulers)
	{
		m_pTop->RemoveChild(m_pRulers);
		m_pRulers->Release();
	}
	float size = sph.radius * 2;
	m_pRulers = CreateRulers(font, size);
	m_pRulers->SetName2("Rulers");
	m_pTop->AddChild(m_pRulers);
}

void ItemGroup::ShowOrigin(bool bShow)
{
	m_pAxes->SetEnabled(bShow);
}

void ItemGroup::ShowRulers(bool bShow)
{
	m_pRulers->SetEnabled(bShow);
}

void ItemGroup::SetRanges()
{
	// Now set the LOD ranges for each model
	int i, num_models = m_pItem->NumModels();
	if (!num_models)
		return;

	// LOD documentation: For N children, you must have N+1 range values.
	// "Note that the last child (n) does not implicitly have a maximum
	//  distance value of infinity.  You must add a n+1'st range value to
	//  specify its maximum distance.  Otherwise, "bad things" will happen."

	m_ranges[0] = 0.0f;
	if (num_models == 1)
		m_ranges[1] = 10000000.0f;
	else
	{
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			m_ranges[i+1] = mod->m_distance;
		}
	}
	m_pLOD->SetRanges(m_ranges, num_models+1);
}

void ItemGroup::ShowLOD(bool bTrue)
{
	m_pLOD->SetEnabled(bTrue);
	m_pGroup->SetEnabled(!bTrue);
	if (bTrue)
	{
		// LOD requires all models to be enabled
		int i, num_models = m_pItem->NumModels();
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			vtTransform *trans = GetMainFrame()->m_nodemap[mod];
			if (trans)
				trans->SetEnabled(true);
		}
	}
	else
	{
		// Group requires all models to be (initially) disabled
		int i, num_models = m_pItem->NumModels();
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			vtTransform *trans = GetMainFrame()->m_nodemap[mod];
			if (trans)
				trans->SetEnabled(false);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Ruler geometry

vtGeom *CreateRulers(vtFont *font, float fSize)
{
	int i, j, start;
	vtMesh *mesh;
	vtTextMesh *text;

	vtGeom *pGeom = new vtGeom();
	vtMaterialArray *pMats = new vtMaterialArray();
	pMats->AddRGBMaterial1(RGBf(1.0f, 1.0f, 1.0f), false, false, false);
	pGeom->SetMaterials(pMats);
	pMats->Release();

	int up = 0;
	float interval = 0.001f;
	while (fSize / interval > 22)
	{
		if (up == 0)
			interval *= 2;
		if (up == 1)
			interval *= 2.5;
		if (up == 1)
			interval *= 2;
		up++;
		if (up > 2) up = 0;
	}
	int ticks = fSize / interval;

	vtString str;
	FPoint3 p;
	float *wide;
	float *thin;
	for (i = 0; i < 3; i++)
	{
		p.Set(0,0,0);

		mesh = new vtMesh(GL_LINES, VT_Normals, 24);

		if (i == 0) { wide = &p.x; thin = &p.z; }
		if (i == 1) { wide = &p.y; thin = &p.x; }
		if (i == 2) { wide = &p.z; thin = &p.x; }

		*wide = -fSize;
		mesh->AddVertex(p);
		*wide =  fSize;
		mesh->AddVertex(p);
		mesh->AddLine(0, 1);

		for (j = 1; j <= ticks; j++)
		{
			*wide = j * interval;
			*thin = -interval/2;
			start = mesh->AddVertex(p);
			*thin =  interval/2;
			mesh->AddVertex(p);
			mesh->AddLine(start, start+1);
		}
		pGeom->AddMesh(mesh, 0);
		mesh->Release();
	}
	for (i = 0; i < 3; i++)
	{
		p.Set(0,0,0);

		if (i == 0) { wide = &p.x; thin = &p.z; }
		if (i == 1) { wide = &p.y; thin = &p.x; }
		if (i == 2) { wide = &p.z; thin = &p.x; }

		for (j = 1; j <= ticks; j++)
		{
			*wide = j * interval;
			*thin = interval/2;

			if (font)
			{
				str.Format("%g", j * interval);
				text = new vtTextMesh(font, interval/2, false);
				text->SetPosition(p);
				if (i == 0)
					text->SetAlignment(0);
				else
					text->SetAlignment(1);
				text->SetText(str);
				pGeom->AddTextMesh(text, 0);
				text->Release();
			}
		}
	}
	return pGeom;
}

