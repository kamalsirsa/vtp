//
// Fence3d.cpp
//
// Creates fence geometry, drapes on a terrain
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Light.h"
#include "Terrain.h"
#include "Fence3d.h"
#include "TerrainScene.h"

// statics
vtMaterialDescriptorArray3d *vtFence3d::s_pFenceMats = NULL;
int vtFence3d::s_mi_wire;


vtFence3d::vtFence3d() : vtFence()
{
	Init();
}

void vtFence3d::Init()
{
	m_bBuilt = false;
	m_pFenceGeom = NULL;
}

void vtFence3d::CreateMaterials()
{
	s_pFenceMats = new vtMaterialDescriptorArray3d();
	s_pFenceMats->InitializeMaterials();

	// Materials for Posts

	// wood fence post
	s_pFenceMats->Append(new vtMaterialDescriptor("wood",
		"Culture/fencepost_64.jpg", VT_MATERIAL_SELFCOLOURED_TEXTURE, -1, -1));

	// create wirefence post textured material (0)
	s_pFenceMats->Append(new vtMaterialDescriptor("steel",
		"Culture/chainpost32.jpg", VT_MATERIAL_SELFCOLOURED_TEXTURE, -1, -1));

	// Materials for Connections

	// manually add wire material
	s_mi_wire = s_pFenceMats->GetMatArray()->AddRGBMaterial(RGBf(0.0f, 0.0f, 0.0f), // diffuse
		RGBf(0.5f, 0.5f, 0.5f),	// ambient
		false, true, false,		// culling, lighting, wireframe
		0.6f);					// alpha

	// chainlink material: twosided, ambient, and alpha-blended
	s_pFenceMats->Append(new vtMaterialDescriptor("chain-link",
		"Culture/chain128-4.png", VT_MATERIAL_SELFCOLOURED_TEXTURE, 0.5f, 0.5f, true, true, true));

	// create drystone textured material
	s_pFenceMats->Append(new vtMaterialDescriptor("drystone",
		"Culture/drystone_wall_512.jpg", VT_MATERIAL_SELFCOLOURED_TEXTURE, 2.4f, 1.2f));
	s_pFenceMats->Append(new vtMaterialDescriptor("stone",
		"Culture/stone256.jpg", VT_MATERIAL_SELFCOLOURED_TEXTURE, 2.5f, -1));

	// create privet textured material
	s_pFenceMats->Append(new vtMaterialDescriptor("privet",
		"Culture/privet_256.jpg", VT_MATERIAL_SELFCOLOURED_TEXTURE, 1.2f, 1.2f, true));

	// create railing textured materials: twosided, ambient, and alpha-blended
	s_pFenceMats->Append(new vtMaterialDescriptor("railing_pipe",
		"Culture/railing_pipe.png", VT_MATERIAL_SELFCOLOURED_TEXTURE, 1, -1, true, true, true));
	s_pFenceMats->Append(new vtMaterialDescriptor("railing_wire",
		"Culture/railing_wire.png", VT_MATERIAL_SELFCOLOURED_TEXTURE, 1, -1, true, true, true));
	s_pFenceMats->Append(new vtMaterialDescriptor("railing_eu",
		"Culture/railing_eu.png", VT_MATERIAL_SELFCOLOURED_TEXTURE, 1, -1, true, true, true));

	// add red material for display of unknown material
	s_pFenceMats->Append(new vtMaterialDescriptor("unknown", "",
		VT_MATERIAL_COLOURED, 1, 1, true, true, false, RGBi(255,0,0)));

	s_pFenceMats->CreateMaterials();
}

void vtFence3d::AddFencepost(const FPoint3 &p1, vtMaterialDescriptor *desc)
{
	// create fencepost block
	vtMesh *pPostMesh = new vtMesh(vtMesh::TRIANGLE_FAN, VT_Normals | VT_TexCoords, 20);

	FPoint3 PostSize(m_Params.m_fPostWidth, m_Params.m_fPostHeight, m_Params.m_fPostDepth);
	pPostMesh->CreateOptimizedBlock(PostSize);

	// scoot over and upwards to put it above ground
	FMatrix4 t;
	t.Identity();
	t.Translate(p1);
	pPostMesh->TransformVertices(t);

	m_pFenceGeom->AddMesh(pPostMesh, desc->GetMaterialIndex());
	pPostMesh->Release();	// pass ownership
}

vtMaterialDescriptor *vtFence3d::FindDescriptor(const vtString &type)
{
	RGBf dummy;
	vtMaterialDescriptor *desc = s_pFenceMats->FindMaterialDescriptor(type, dummy);
	if (!desc)
		desc = s_pFenceMats->FindMaterialDescriptor("unknown", dummy);
	return desc;
}

FPoint3 SidewaysVector(const FPoint3 &p0, const FPoint3 &p1)
{
	FPoint3 diff = p1 - p0;
	FPoint3 up(0,1,0);
	FPoint3 cross = diff.Cross(up);
	cross.Normalize();
	return cross;
}

void vtFence3d::AddFenceMeshes(vtHeightField3d *pHeightField)
{
	unsigned int i, j;
	unsigned int numfencepts = m_pFencePts.GetSize();

	DLine2 posts;
	FLine3 p3;

	DPoint2 diff, dp;
	FPoint3 PostSize(m_Params.m_fPostWidth, m_Params.m_fPostHeight,
		m_Params.m_fPostDepth);

	if (m_Params.m_PostType != "none")
	{
		// has posts
		// determine where the fence posts go
		for (i = 0; i < numfencepts; i++)
		{
			if (i == numfencepts-1)
			{
				posts.Append(m_pFencePts[i]);
				continue;
			}
			// get start and end group points for this section
			DPoint2 epos1 = m_pFencePts[i];
			DPoint2 epos2 = m_pFencePts[i+1];

			diff = epos2 - epos1;
			double distance = diff.Length();
			unsigned int segments = (unsigned int) (distance / m_Params.m_fPostSpacing);
			if (segments < 1) segments = 1;
			DPoint2 diff_per_segment = diff / segments;

			for (j = 0; j < segments; j++)
			{
				dp = epos1 + (diff_per_segment * j);
				posts.Append(dp);
			}
		}

		// convert post positions to world-coordinate ground locations
		FPoint3 pout;
		p3.SetSize(posts.GetSize());
		for (i = 0; i < posts.GetSize(); i++)
		{
			pHeightField->ConvertEarthToSurfacePoint(posts[i], pout);

			if (i > 0 && i < posts.GetSize()-1)
			{
				// randomly offset by up to 4% of fence spacing, for "realism"
				pout.x += random_offset(0.04f * m_Params.m_fPostSpacing);
				pout.z += random_offset(0.04f * m_Params.m_fPostSpacing);
			}
			p3[i] = pout;
		}
		// generate the posts
		vtMaterialDescriptor *desc = FindDescriptor(m_Params.m_PostType);
		for (i = 0; i < posts.GetSize(); i++)
			AddFencepost(p3[i], desc);
	}
	else
	{
		// no posts, just project earth to world
		p3.SetSize(numfencepts);
		for (i = 0; i < numfencepts; i++)
			pHeightField->ConvertEarthToSurfacePoint(m_pFencePts[i], p3[i]);
	}

	// if not enough points, nothing connections to create
	unsigned int npoints = p3.GetSize();
	if (npoints < 2)
		return;

	if (m_Params.m_ConnectType == "none")
	{
		// nothing to do
	}
	else if (m_Params.m_ConnectType == "wire")
	{
		// the 3 wires
		if (npoints > 1)
		{
			float wire_height[3] = { 0.42f, 0.66f, 0.91f };

			vtMesh *pWireMesh = new vtMesh(vtMesh::LINE_STRIP, 0, npoints);
			int vidx = 0;
			for (j = 0; j < 3; j++)
			{
				int start = vidx;
				for (i = 0; i < npoints; i++)
				{
					pWireMesh->AddVertex(p3[i] + FPoint3(0, (PostSize.y * wire_height[j]), 0));
					vidx++;
				}
				pWireMesh->AddStrip2(npoints, start);
			}
			m_pFenceGeom->AddMesh(pWireMesh, s_mi_wire);
			pWireMesh->Release();	// pass ownership
		}
	}
	else if (m_Params.m_fConnectWidth == 0.0f)
	{
		vtMesh *pMesh = new vtMesh(vtMesh::TRIANGLE_STRIP, VT_TexCoords, 100);

		// A single thin strip polygon with a single texture.
		vtMaterialDescriptor *desc = FindDescriptor(m_Params.m_ConnectType);
		FPoint2 uvscale = desc->GetUVScale();

		float u = 0.0f;
		float vertical_meters = m_Params.m_fConnectTop - m_Params.m_fConnectBottom;
		float v_top;
		if (uvscale.y == -1)
			v_top = 1.0f;
		else
			v_top = vertical_meters / uvscale.y;

		int vidx = 0;
		for (i = 0; i < npoints; i++)
		{
			pMesh->SetVtxPUV(vidx++, p3[i] + FPoint3(0, m_Params.m_fConnectBottom, 0), u, 0.0);
			pMesh->SetVtxPUV(vidx++, p3[i] + FPoint3(0, m_Params.m_fConnectTop, 0), u, v_top);

			if (i < npoints-1)
			{
				// increment u based on the length of each fence segment
				float length_meters = (p3[i+1] - p3[i]).Length();
				u += (length_meters / desc->GetUVScale().x);
			}
		}
		pMesh->AddStrip2(npoints * 2, 0);

		m_pFenceGeom->AddMesh(pMesh, desc->GetMaterialIndex());
		pMesh->Release();	// pass ownership
	}
	else if (m_Params.m_fConnectWidth > 0.0f)
	{
		vtMesh *pMesh = new vtMesh(vtMesh::TRIANGLE_STRIP, VT_TexCoords | VT_Normals, 100);

		// a solid block, with top/left/right sides, made of 3 strips
		vtMaterialDescriptor *desc = FindDescriptor(m_Params.m_ConnectType);
		FPoint2 uvscale = desc->GetUVScale();
		float vertical_meters = m_Params.m_fConnectTop - m_Params.m_fConnectBottom;

		float u = 0.0f;
		float v1, v2;
		for (int i = 0; i < 3; i++)
		{
			float y1, y2;
			float z1, z2;
			FPoint3 pos, sideways, normal;

			// determine v texture coordinate
			switch (i)
			{
			case 0:
				v1 = 0.0f;
				if (uvscale.y == -1)
					v2 = 1.0f;
				else
					v2 = vertical_meters / uvscale.y;
				break;
			case 1:
				v1 = 0.0f;
				if (uvscale.y == -1)
					v2 = 1.0f;
				else
					v2 = m_Params.m_fConnectWidth / uvscale.y;
				break;
			case 2:
				v2 = 0.0f;
				if (uvscale.y == -1)
					v1 = 1.0f;
				else
					v1 = vertical_meters / uvscale.y;
				break;
			}

			// determine Y and Z values
			switch (i)
			{
			case 0:
				y1 = m_Params.m_fConnectBottom;
				y2 = m_Params.m_fConnectTop;
				z1 = m_Params.m_fConnectWidth / 2;
				z2 = m_Params.m_fConnectWidth / 2;
				break;
			case 1:
				y1 = m_Params.m_fConnectTop;
				y2 = m_Params.m_fConnectTop;
				z1 = m_Params.m_fConnectWidth / 2;
				z2 = -m_Params.m_fConnectWidth / 2;
				break;
			case 2:
				y1 = m_Params.m_fConnectTop;
				y2 = m_Params.m_fConnectBottom;
				z1 = -m_Params.m_fConnectWidth / 2;
				z2 = -m_Params.m_fConnectWidth / 2;
				break;
			}

			int vidx;
			int start = pMesh->GetNumVertices();
			for (j = 0; j < npoints; j++)
			{
				if (j < npoints-1)
				{
					// determine normal (used for shading and thickness)
					sideways = SidewaysVector(p3[j], p3[j+1]);
					switch (i)
					{
					case 0: normal = sideways; break;
					case 1: normal.Set(0,1,0); break;	// up
					case 2: normal = -sideways; break;
					}
				}

				pos = p3[j];
				pos.y += y2;
				pos += (sideways * z2);
				vidx = pMesh->AddVertex(pos);
				pMesh->SetVtxTexCoord(vidx, FPoint2(u, v2));
				pMesh->SetVtxNormal(vidx, normal);

				pos = p3[j];
				pos.y += y1;
				pos += (sideways * z1);
				vidx = pMesh->AddVertex(pos);
				pMesh->SetVtxTexCoord(vidx, FPoint2(u, v1));
				pMesh->SetVtxNormal(vidx, normal);

				if (j < npoints-1)
				{
					// increment u based on the length of each fence segment
					float length_meters = (p3[j+1] - p3[j]).Length();
					u += (length_meters / desc->GetUVScale().x);
				}
			}
			pMesh->AddStrip2(npoints * 2, start);
		}

		// add cap at beginning
		u = m_Params.m_fConnectWidth / desc->GetUVScale().x;
		v2 = vertical_meters / uvscale.y;

		int start =
		pMesh->AddVertex(pMesh->GetVtxPos(npoints*2*2+1));
		pMesh->AddVertex(pMesh->GetVtxPos(npoints*2*2));
		pMesh->AddVertex(pMesh->GetVtxPos(0));
		pMesh->AddVertex(pMesh->GetVtxPos(1));
		pMesh->SetVtxNormal(start+0, p3[0] - p3[1]);
		pMesh->SetVtxNormal(start+1, p3[0] - p3[1]);
		pMesh->SetVtxNormal(start+2, p3[0] - p3[1]);
		pMesh->SetVtxNormal(start+3, p3[0] - p3[1]);
		pMesh->SetVtxTexCoord(start+0, FPoint2(u, 0.0f));
		pMesh->SetVtxTexCoord(start+1, FPoint2(u, v2));
		pMesh->SetVtxTexCoord(start+2, FPoint2(0.0f, 0.0f));
		pMesh->SetVtxTexCoord(start+3, FPoint2(0.0f, v2));
		pMesh->AddStrip2(4, start);

		// add cap at end
		start =
		pMesh->AddVertex(pMesh->GetVtxPos(npoints*2-2));
		pMesh->AddVertex(pMesh->GetVtxPos(npoints*2-1));
		pMesh->AddVertex(pMesh->GetVtxPos(npoints*3*2-1));
		pMesh->AddVertex(pMesh->GetVtxPos(npoints*3*2-2));
		pMesh->SetVtxNormal(start+0, p3[npoints-1] - p3[npoints-2]);
		pMesh->SetVtxNormal(start+1, p3[npoints-1] - p3[npoints-2]);
		pMesh->SetVtxNormal(start+2, p3[npoints-1] - p3[npoints-2]);
		pMesh->SetVtxNormal(start+3, p3[npoints-1] - p3[npoints-2]);
		pMesh->SetVtxTexCoord(start+0, FPoint2(0.0f, 0.0f));
		pMesh->SetVtxTexCoord(start+1, FPoint2(0.0f, v2));
		pMesh->SetVtxTexCoord(start+2, FPoint2(u, 0.0f));
		pMesh->SetVtxTexCoord(start+3, FPoint2(u, v2));
		pMesh->AddStrip2(4, start);

		m_pFenceGeom->AddMesh(pMesh, desc->GetMaterialIndex());
		pMesh->Release();	// pass ownership
	}
}

void vtFence3d::DestroyGeometry()
{
	// Destroy the meshes so they can be re-made
	while (m_pFenceGeom->GetNumMeshes())
	{
		vtMesh *pMesh = m_pFenceGeom->GetMesh(0);
		m_pFenceGeom->RemoveMesh(pMesh);
	}

	m_bBuilt = false;
}

/////////////////////////////////////
// Implement vtStructure3d methods

/**
 * Build (or rebuild) the geometry for a fence.
 */
bool vtFence3d::CreateNode(vtTerrain *pTerr)
{
	if (!m_pFenceGeom)
	{
		bool bFirstTime = false;
		if (s_pFenceMats == NULL)
		{
			bFirstTime = true;
			CreateMaterials();
		}

		m_pFenceGeom = new vtGeom;
		m_pFenceGeom->SetName2("Fence");
		m_pFenceGeom->SetMaterials(s_pFenceMats->GetMatArray());

		if (bFirstTime)
			s_pFenceMats->GetMatArray()->Release();
	}

	if (m_bBuilt)
		DestroyGeometry();

	// create surface and shape
	AddFenceMeshes(pTerr->GetHeightField());

	m_bBuilt = true;
	return true;
}

void vtFence3d::DeleteNode()
{
	if (m_pFenceGeom)
	{
		DestroyGeometry();
		m_pFenceGeom->Release();
		m_pFenceGeom = NULL;
	}
}

