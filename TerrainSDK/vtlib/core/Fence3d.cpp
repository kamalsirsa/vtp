//
// Fence3d.cpp
//
// Creates fence geometry, drapes on a terrain
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Light.h"
#include "Terrain.h"
#include "Fence3d.h"

// statics
vtMaterialArray *vtFence3d::s_pFenceMats;
float vtFence3d::s_fFenceScale;	// fence size is exaggerated by this amount

vtFence3d::vtFence3d() : vtFence()
{
	Init();
}

vtFence3d::vtFence3d(FenceType type, float fHeight, float fSpacing) : vtFence(type, fHeight, fSpacing)
{
	Init();
}

void vtFence3d::Init()
{
	m_bBuilt = false;
	m_pFenceGeom = NULL;
}

int vtFence3d::m_mi_woodpost;
int vtFence3d::m_mi_wire;
int vtFence3d::m_mi_chainlink;
int vtFence3d::m_mi_metalpost;
int vtFence3d::m_mi_hedgerow;
int vtFence3d::m_mi_drystone;
int vtFence3d::m_mi_privet;
int vtFence3d::m_mi_stone;
int vtFence3d::m_mi_beech;

void vtFence3d::CreateMaterials()
{
	s_pFenceMats = new vtMaterialArray();

	// create wirefence post textured material (0)
	vtString fname;

	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/fencepost_64.bmp");
	m_mi_woodpost = s_pFenceMats->AddTextureMaterial2(fname,
		true, true, false, false,
		TERRAIN_AMBIENT,
		TERRAIN_DIFFUSE,
		1.0f,		// alpha
		TERRAIN_EMISSIVE);

	// add wire material (1)
	m_mi_wire = s_pFenceMats->AddRGBMaterial(RGBf(0.0f, 0.0f, 0.0f), // diffuse
		RGBf(0.5f, 0.5f, 0.5f),	// ambient
		false, true, false,		// culling, lighting, wireframe
		0.6f);					// alpha

	// chainlink material(2)
	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/chain128-4.png");
	m_mi_chainlink = s_pFenceMats->AddTextureMaterial2(fname,
		false, true, true, false,	// cull, light, transp, add
		1.0f,	// ambient
		0.0f,	// diffuse
		1.0f,	// alpha
		TERRAIN_EMISSIVE);

	// create chainfence post textured material (3)
	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/chainpost32.bmp");
	m_mi_metalpost = s_pFenceMats->AddTextureMaterial2(fname,
		true, true, false, false,	// cull, light, transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);

	// create hedgerow textured material
	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/hedgerow256.png");
	m_mi_hedgerow = s_pFenceMats->AddTextureMaterial2(fname,
		false, true, true, false,	// cull, light, transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);

	// create drystone textured material
	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/drystone256.png");
	m_mi_drystone = s_pFenceMats->AddTextureMaterial2(fname,
		false, true, true, false,	// cull, light, transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);

	// create privet textured material
	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/privet256.png");
	m_mi_privet = s_pFenceMats->AddTextureMaterial2(fname,
		false, true, true, false,	// cull, light, transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);

	// create stone textured material
	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/stone256.png");
	m_mi_stone = s_pFenceMats->AddTextureMaterial2(fname,
		false, true, true, false,	// cull, light, transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);

	// create beech textured material
	fname = FindFileOnPaths(vtTerrain::m_DataPaths, "Culture/beech256.png");
	m_mi_beech = s_pFenceMats->AddTextureMaterial2(fname,
		false, true, true, false,	// cull, light, transp, add
		TERRAIN_AMBIENT, TERRAIN_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);
}

void vtFence3d::AddFencepost(FPoint3 &p1, int iMatIdx)
{
	// create fencepost block
	vtMesh *pPostMesh = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 20);

	FPoint3 PostSizeScaled = m_PostSize * s_fFenceScale;
	pPostMesh->CreateOptimizedBlock(PostSizeScaled );

	// scoot over and upwards to put it above ground
	FMatrix4 t;
	t.Identity();
	t.Translate(p1);
	pPostMesh->TransformVertices(t);

	m_pFenceGeom->AddMesh(pPostMesh, iMatIdx);
}


void vtFence3d::AddFenceMeshes(vtHeightField *pHeightField)
{
	Array<DPoint2> posts;
	DPoint2 diff, dp;
	int i, j, nposts;

	int numfencepts = m_pFencePts.GetSize();
	float fCurrentSpacing = m_fSpacing * s_fFenceScale;
	FPoint3 PostSizeScaled = m_PostSize * s_fFenceScale;
	float fFenceHeightScaled = m_fHeight * s_fFenceScale;

	// first determine where the fence posts go, for this whole array
	// of fences
	for (i = 0; i < numfencepts; i++)
	{
		if (i == numfencepts-1)
		{
			posts.Append(m_pFencePts[i]);
			continue;
		}
		// get start and end group points for this section
		DPoint2 utm1 = m_pFencePts[i];
		DPoint2 utm2 = m_pFencePts[i+1];

		diff = utm2 - utm1;
		double distance = diff.Length();
		int segments = (int) (distance / fCurrentSpacing);
		if (segments < 1) segments = 1;
		DPoint2 diff_per_segment = diff / segments;

		for (j = 0; j < segments; j++)
		{
			dp = utm1 + (diff_per_segment * j);
			posts.Append(dp);
		}
	}

	// convert post positions to world-coordinate ground locations
	nposts = posts.GetSize();

	FPoint3 pout;
	FLine3 p3;
	p3.SetSize(nposts);
	for (i = 0; i < nposts; i++)
	{
		dp = posts[i];
		pHeightField->ConvertEarthToSurfacePoint(dp.x, dp.y, pout);

		if (i > 0 && i < nposts-1)
		{
			// randomly offset by up to 4% of fence spacing, for "realism"
			pout.x += random_offset(0.04f * fCurrentSpacing);
			pout.z += random_offset(0.04f * fCurrentSpacing);
		}
		p3[i] = pout;
	}

	if (m_FenceType == FT_WIRE)
	{
		// generate the posts
		for (i = 0; i < nposts; i++)
			AddFencepost(p3[i], m_mi_woodpost);

		// and the 3 wires
		if (nposts > 1)
		{
			float wire_height[3] = { 0.42f, 0.66f, 0.91f };

			vtMesh *pWireMesh = new vtMesh(GL_LINE_STRIP, 0, nposts);
			int vidx = 0;
			for (j = 0; j < 3; j++)
			{
				int start = vidx;
				for (i = 0; i < nposts; i++)
				{
					pWireMesh->AddVertex(p3[i] + FPoint3(0, (PostSizeScaled.y * wire_height[j]), 0));
					vidx++;
				}
				pWireMesh->AddStrip2(nposts, start);
			}
			m_pFenceGeom->AddMesh(pWireMesh, m_mi_wire);
		}
	}

	if (m_FenceType == FT_CHAINLINK)
	{
		float u = 0.0f;
		float fence_height_meters = m_PostSize.y;
		float v_top = fence_height_meters * 2.0f;

		// generate the posts
		for (i = 0; i < nposts; i++)
			AddFencepost(p3[i], m_mi_metalpost);

		if (nposts > 1)
		{
			vtMesh *pMesh = new vtMesh(GL_TRIANGLE_STRIP, VT_Normals | VT_TexCoords, nposts*2);
			int vidx = 0;
			for (i = 0; i < nposts; i++)
			{
				pMesh->SetVtxPUV(vidx++, p3[i], u, 0.0);
				pMesh->SetVtxPUV(vidx++, p3[i] + FPoint3(0, PostSizeScaled.y, 0), u, v_top);

				if (i < nposts+1)
				{
					// increment u based on the length of each fence segment
					float length = (posts[i+1] - posts[i]).Length();
					u += ((length / s_fFenceScale) * 2.0f);
				}
			}
			pMesh->AddStrip2(nposts * 2, 0);
			m_pFenceGeom->AddMesh(pMesh, m_mi_chainlink);
		}
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
bool vtFence3d::CreateNode(vtHeightField *hf, const vtTagArray &options)
{
	if (!m_pFenceGeom)
	{
		if (s_pFenceMats == NULL)
			CreateMaterials();

		m_pFenceGeom = new vtGeom;
		m_pFenceGeom->SetName2("Fence");
		m_pFenceGeom->SetMaterials(s_pFenceMats);
	}

	if (m_bBuilt)
		DestroyGeometry();

	// create surface and shape
	AddFenceMeshes(hf);

	m_bBuilt = true;
	return true;
}

vtGeom *vtFence3d::GetGeom()
{
	return m_pFenceGeom;
}

void vtFence3d::DeleteNode()
{
	if (m_pFenceGeom)
	{
		DestroyGeometry();
		delete m_pFenceGeom;
		m_pFenceGeom = NULL;
	}
}

