//
// Building3d.cpp
//
// The vtBuilding3d class extends vtBuilding with the ability to procedurally
// create 3d geometry of the buildings.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/HeightField.h"
#include "vtdata/Triangulate.h"
#include "Building3d.h"
#include "Terrain.h"

// An array of materials, shared by all buildings.
// This is done to save memory.  For a list of 16000+ buildings, this can
//  save about 200MB of RAM.
vtMaterialArray *vtBuilding3d::s_Materials = NULL;
RGBf vtBuilding3d::s_Colors[COLOR_SPREAD];

#define PLAIN_MAT_START		0	// start index for plain colors
#define PLAIN_MAT_END		PLAIN_MAT_START + COLOR_SPREAD - 1
#define SIDING_MAT_START	PLAIN_MAT_END + 1
#define SIDING_MAT_END		SIDING_MAT_START + COLOR_SPREAD - 1
#define WINDOW_MAT			SIDING_MAT_END + 1
#define DOOR_MAT			WINDOW_MAT + 1
#define WOOD_MAT			DOOR_MAT + 1
#define CEMENT_MAT			WOOD_MAT + 1
#define BRICK_MAT1			CEMENT_MAT + 1
#define BRICK_MAT2			BRICK_MAT1 + 1
#define PBRICK_MAT_START	BRICK_MAT2 + 1
#define PBRICK_MAT_END		PBRICK_MAT_START + COLOR_SPREAD - 1
#define WINDOWWALL_MAT_START PBRICK_MAT_END + 1
#define WINDOWWALL_MAT_END WINDOWWALL_MAT_START + COLOR_SPREAD - 1

//
// Helper to make a material
//
vtMaterial *makeMaterial(RGBf &color, bool culling)
{
	vtMaterial *pMat = new vtMaterial();
	pMat->SetDiffuse1(color * 0.7f);
	pMat->SetAmbient1(color * 0.4f);
	pMat->SetSpecular2(0.0f);
	pMat->SetCulling(culling);
	pMat->SetLighting(true);
	return pMat;
}

/////////////////////////////////////////////////////////////////////////////

vtBuilding3d::vtBuilding3d() : vtBuilding()
{
	m_pContainer = NULL;
	m_pGeom = NULL;
	m_pHighlight = NULL;
}

vtBuilding3d::~vtBuilding3d()
{
	for (int i = 0; i < m_Mesh.GetSize(); i++)
		delete m_Mesh[i].m_pMesh;
}

vtBuilding3d &vtBuilding3d::operator=(const vtBuilding &v)
{
	// just call the copy method of the parent class
	*((vtBuilding*)this) = v;
	return *this;
}

void vtBuilding3d::CreateSharedMaterials()
{
	vtString path;
	int i, j, k;
	RGBf color;
	vtMaterial *pMat;
	s_Materials = new vtMaterialArray();
	s_Materials->SetMaxSize(500);

	int count = 0;
	int divisions = 6;
	float start = .25f;
	float step = (1.0f-start)/(divisions-1);
	for (i = 0; i < divisions; i++) {
		for (j = 0; j < divisions; j++) {
			for (k = 0; k < divisions; k++) {
				s_Colors[count++].Set(start+i*step, start+j*step, start+k*step);
			}
		}
	}

	// create plain materials
	for (i = 0; i < COLOR_SPREAD; i++)
	{
		pMat = makeMaterial(s_Colors[i], false);
		s_Materials->AppendMaterial(pMat);
	}

	//create siding materials (bright colors only)
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/siding64.bmp");
	vtImage *pImageSiding = new vtImage(path);
	divisions = 6;
	start = .25f;
	step = (1.0f-start)/(divisions-1);
	for (i = 0; i < COLOR_SPREAD; i++)
	{
		pMat = makeMaterial(s_Colors[i], true);
		pMat->SetTexture(pImageSiding);
		pMat->SetClamp(false);
		s_Materials->AppendMaterial(pMat);
	}

	// others are literal textures - use white for diffuse
	color.Set(1.0f, 1.0f, 1.0f);

	// window material
	pMat = makeMaterial(color, false);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/window.bmp");
	pMat->SetTexture2(path);
	pMat->SetClamp(false);
	s_Materials->Append(pMat);

	// door material
	pMat = makeMaterial(color, false);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/door.bmp");
	pMat->SetTexture2(path);
	s_Materials->Append(pMat);

	// wood material
	pMat = makeMaterial(color, false);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/wood1_256.bmp");
	pMat->SetTexture2(path);
	pMat->SetClamp(false);
	s_Materials->Append(pMat);

	// cement block material
	pMat = makeMaterial(color, false);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/cement_block1_256.bmp");
	pMat->SetTexture2(path);
	pMat->SetClamp(false);
	s_Materials->Append(pMat);

	// brick material 1
	// measured average brick color: 159, 100, 83 (reddish medium brown)
	pMat = makeMaterial(color, false);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/brick1_256.bmp");
	pMat->SetTexture2(path);
	pMat->SetClamp(false);
	s_Materials->Append(pMat);

	// brick material 2
	// measured average brick color: 183, 178, 171 (slightly pinkish grey)
	pMat = makeMaterial(color, false);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/brick2_256.bmp");
	pMat->SetTexture2(path);
	pMat->SetClamp(false);
	s_Materials->Append(pMat);

	// painted brick material: no brick color, can be colorized for any shade
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/brick_mono_256.bmp");
	vtImage *pPaintedBrick = new vtImage(path);
	for (i = 0; i < COLOR_SPREAD; i++)
	{
		pMat = makeMaterial(s_Colors[i], false);
		pMat->SetTexture(pPaintedBrick);
		pMat->SetClamp(false);
		s_Materials->Append(pMat);
	}

	// create window-wall materials (bright colors only)
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/window_wall128.bmp");
	vtImage *pImageWindowWall = new vtImage(path);
	for (i = 0; i < COLOR_SPREAD; i++)
	{
		pMat = makeMaterial(s_Colors[i], true);
		pMat->SetTexture(pImageWindowWall);
		pMat->SetClamp(false);
		s_Materials->AppendMaterial(pMat);
	}

	int total = s_Materials->GetSize();
	// window, door, wood, cement_block, windowwall
	int expectedtotal = COLOR_SPREAD + COLOR_SPREAD +	// plain, siding
		1 + 1 + 1 + 1 +		// window, door, wood, cement
		1 + 1 +				// brick1, brick2
		COLOR_SPREAD + COLOR_SPREAD;	// painted brick, window-wall
	assert(total == expectedtotal);
}

void vtBuilding3d::FindMaterialIndices()
{
	if (s_Materials == NULL)
		CreateSharedMaterials();
}


//
// Convert the footprint for each level into world coordinates
//
void vtBuilding3d::DetermineWorldFootprints(vtHeightField *pHeightField)
{
	int i, j;
	int levs = GetNumLevels();
	FPoint3 p3;

	m_lfp.Empty();
	for (i = 0; i < levs; i++)
	{
		vtLevel *lev = m_Levels[i];
		DLine2 &foot = lev->GetFootprint();
		int size = foot.GetSize();

		// find where the footprints points are on the ground
		FLine3 *foot3d = new FLine3();
		foot3d->SetSize(size);

		for (j = 0; j < size; j++)
		{
			pHeightField->ConvertEarthToSurfacePoint(foot[j], p3);
			foot3d->SetAt(j, p3);
		}
		m_lfp.Append(foot3d);
	}
	FLine3 &base = *m_lfp[0];
	int base_corners = base.GetSize();

	// determine the highest point of the base corners
	float ymax = -10000.0f, ymin = 10000.0f;
	int highest, lowest;
	for (i = 0; i < base_corners; i++)
	{
		if (base[i].y > ymax)
		{
			ymax = base[i].y;
			highest = i;
		}
		if (base[i].y < ymin)
		{
			ymin = base[i].y;
			lowest = i;
		}
	}
	if ((ymax - ymin) > 1.0f)	// it's on a slope
	{
		// Could do something about it - like deform the ground or build
		//  some posts on which to set the building?
	}
	// Embed the building in the ground such that the lowest corner of its
	// lowest level is at ground level.
	float base_level = ymin;

	// Find the center of the building in world coordinates (the origin of
	// the building's local coordinate system)
	pHeightField->ConvertEarthToSurfacePoint(m_EarthPos, m_center);
	m_center.y = base_level;

	//
	// The final footprints are expressed relative to the origin of the
	// building (they are in the building's local coordinate system)
	//
	float height = 0.0f;
	for (i = 0; i < levs; i++)
	{
		int size = m_lfp[i]->GetSize();
		for (j = 0; j < size; j++)
		{
			p3 = m_lfp[i]->GetAt(j) - m_center;
			p3.y = height;
			m_lfp[i]->SetAt(j, p3);
		}
		height += (m_Levels[i]->m_fStoryHeight * m_Levels[i]->m_iStories);
	}
}

float vtBuilding3d::GetHeightOfStories()
{
	float height = 0.0f;

	int levs = m_Levels.GetSize();
	for (int i = 0; i < levs; i++)
		height += m_Levels[i]->m_iStories * m_Levels[i]->m_fStoryHeight;

	return height;
}


/**
 * Creates the geometry for the building.
 * Capable of several levels of detail (defaults to full detail).
 * If the geometry was already built previously, it is destroyed and re-created.
 *
 * \param pHeightField The heightfield on which to plant the building.
 * \param bDoRoof Construct a roof.
 * \param bDoWalls Construct the walls.
 * \param bDetails Construct all the little details, like mouding and windows.
 */
void vtBuilding3d::CreateShape(vtHeightField *pHeightField, bool bDoRoof,
							   bool bDoWalls, bool bDetails)
{
	if (m_pContainer)
	{
		// was build before; re-build geometry
		DestroyGeometry();
	}
	else
	{
		// constructing for the first time
		m_pContainer = new vtTransform();
		m_pContainer->SetName2("building");
	}
	CreateGeometry(pHeightField, bDoRoof, bDoWalls, bDetails);
	m_pContainer->AddChild(m_pGeom);
	m_pContainer->SetTrans(m_center);
}

void vtBuilding3d::DestroyGeometry()
{
	m_pContainer->RemoveChild(m_pGeom);
	delete m_pGeom;
	for (int i = 0; i < m_Mesh.GetSize(); i++)
		delete m_Mesh[i].m_pMesh;
	m_Mesh.Empty();
}

void vtBuilding3d::AdjustHeight(vtHeightField *pHeightField)
{
	DetermineWorldFootprints(pHeightField);
	m_pContainer->SetTrans(m_center);
}

void vtBuilding3d::CreateUpperPolygon(vtLevel *lev, FLine3 &poly, FLine3 &poly2)
{
	int i, prev, next;

	poly2 = poly;
	int edges = lev->m_Edges.GetSize();
	for (i = 0; i < edges; i++)
	{
		prev = (i-1 < 0) ? edges-1 : i-1;
		next = (i+1 == edges) ? 0 : i+1;

		FPoint3 p = poly[i];

		int islope1 = lev->m_Edges[prev]->m_iSlope;
		int islope2 = lev->m_Edges[i]->m_iSlope;
		if (islope1 == 90 && islope2 == 90)
		{
			// easy case
			p.y += lev->m_fStoryHeight;
		}
		else
		{
			float slope1 = (islope1 / 180.0f * PIf);
			float slope2 = (islope2 / 180.0f * PIf);

			// get edge vectors
			FPoint3 vec1 = poly[prev] - poly[i];
			FPoint3 vec2 = poly[next] - poly[i];
			vec1.Normalize();
			vec2.Normalize();

			// get perpendicular (upward pointing) vectors
			FPoint3 perp1, perp2;
			perp1.Set(0, 1, 0);
			perp2.Set(0, 1, 0);

			// create rotation matrices to rotate them upward
			FMatrix4 mat1, mat2;
			mat1.Identity();
			mat1.AxisAngle(vec1, -slope1);
			mat2.Identity();
			mat2.AxisAngle(vec2, slope2);

			// create normals
			FPoint3 norm1, norm2;
			mat1.TransformVector(perp1, norm1);
			mat2.TransformVector(perp2, norm2);

			// vector of plane intersection is cross product of their normals
			FPoint3 inter = norm1.Cross(norm2);
			inter.Normalize();
			inter *= (lev->m_fStoryHeight / inter.y);

			p += inter;
		}
		poly2[i] = p;
	}
}

void vtBuilding3d::CreateGeometry(vtHeightField *pHeightField, bool bDoRoof,
								   bool bDoWalls, bool details)
{
	int i, j, k;

	// make sure we've got materials first
	FindMaterialIndices();

	DetermineWorldFootprints(pHeightField);

	// create the edges (walls and roof)
	float fHeight = 0.0f;
	int iLevels = GetNumLevels();
	if (bDoWalls)
	{
		for (i = 0; i < iLevels; i++)
		{
			vtLevel *lev = m_Levels[i];
			if (lev->IsHorizontal())
			{
				// make flat roof
				AddFlatRoof(*m_lfp[i], lev);
				continue;
			}
			else if (lev->IsUniform())
			{
				CreateUniformLevel(i, fHeight);
				fHeight += lev->m_iStories * lev->m_fStoryHeight;
				continue;
			}
			else
				// 'flat roof' for the floor
				AddFlatRoof(*m_lfp[i], lev);

			FLine3 poly = *m_lfp[i];
			FLine3 poly2;

			for (j = 0; j < lev->m_iStories; j++)
			{
				int edges = lev->m_Edges.GetSize();
				for (k = 0; k < edges; k++)
				{
					poly[k].y = fHeight;
				}
				CreateUpperPolygon(lev, poly, poly2);
				for (k = 0; k < edges; k++)
				{
					CreateEdgeGeometry(lev, poly, poly2, k, details);
				}
				fHeight += lev->m_fStoryHeight;
			}
		}
	}

	FLine3 *roof = m_lfp[iLevels-1];	// roof: top level
	vtLevel *roof_lev = m_Levels[iLevels-1];
	float roof_height = (roof_lev->m_fStoryHeight * roof_lev->m_iStories);

	// wrap in a shape and set materials
	m_pGeom = new vtGeom();
	m_pGeom->SetName2("building-geom");
	m_pGeom->SetMaterials(s_Materials);

	for (i = 0; i < m_Mesh.GetSize(); i++)
	{
		vtMesh *mesh = m_Mesh[i].m_pMesh;
		int index = m_Mesh[i].m_iMatIdx;
		m_pGeom->AddMesh(mesh, index);
	}

	// resize bounding box (TODO: more elegant implementation for this)
	if (m_pHighlight)
	{
		m_pContainer->RemoveChild(m_pHighlight);
		FSphere sphere;
		m_pGeom->GetBoundSphere(sphere);
		m_pHighlight = CreateBoundSphereGeom(sphere);
		m_pContainer->AddChild(m_pHighlight);
	}
}


////////////////////////////////////////////////////////////////////////////

//
// Since each set of primitives with a specific material requires its own
// mesh, this method looks up or creates the mesh as needed.
//
vtMesh *vtBuilding3d::FindMatMesh(BldMaterial bm, RGBi color, bool bFans)
{
	int mi = FindMatIndex(bm, color);

	int i, size = m_Mesh.GetSize();
	for (i = 0; i < size; i++)
	{
		if (m_Mesh[i].m_iMatIdx == mi && m_Mesh[i].m_bFans == bFans)
			return m_Mesh[i].m_pMesh;
	}
	// didn't find it, so we need to make it
	MatMesh mm;
	mm.m_iMatIdx = mi;
	mm.m_bFans = bFans;

	// Potential Optimization: should calculate how many vertices the building
	// will take.  Even the simplest building will use 20 vertices, for now
	// just use 40 as a reasonable starting point for each component mesh.

	mm.m_pMesh = new vtMesh(bFans ? GL_TRIANGLE_FAN : GL_TRIANGLES,
		VT_Normals | VT_TexCoords, 40);
	m_Mesh.Append(mm);
	return mm.m_pMesh;
}

//
// Walls/edges are created in panels (sections)
//
void vtBuilding3d::CreateEdgeGeometry(vtLevel *pLev, FLine3 &poly1, FLine3 &poly2,
									  int iWall, bool details)
{
	int num_Edgess = pLev->m_Edges.GetSize();
	int i = iWall, j = (i+1)%num_Edgess;

	FLine3 quad(4);

	vtEdge	*pEdge = pLev->m_Edges[iWall];

	// start with the whole wall section
	quad[0] = poly1[i];
	quad[1] = poly1[j];
	quad[2] = poly2[i];
	quad[3] = poly2[j];

	// figure out how many features we have
	int totalfeatures = pEdge->NumFeatures();

	// length of the edge
	FPoint3 dir1 = quad[1] - quad[0];
	FPoint3 dir2 = quad[3] - quad[2];
	float total_length1 = dir1.Length();
	float total_length2 = dir2.Length();
	if (total_length1 > 0.0f)
		dir1.Normalize();
	if (total_length2 > 0.0f)
		dir2.Normalize();

	// How wide should each feature be?
	// Determine how much space we have for the proportional features after
	// accounting for the fixed-width features
	float fixed_width = pEdge->FixedFeaturesWidth();
	float total_prop = pEdge->ProportionTotal();
	float dyn_width = total_length1 - fixed_width;

	// build the edge features.
	// point[0] is the first starting point of a panel.
	int features = pEdge->NumFeatures();
	for (i = 0; i < pEdge->NumFeatures(); i++)
	{
		vtEdgeFeature &feat = pEdge->m_Features[i];

		// determine real width
		float meter_width = 0.0f;
		if (feat.m_width >= 0)
			meter_width = feat.m_width;
		else
			meter_width = (feat.m_width / total_prop) * dyn_width;
		quad[1] = quad[0] + dir1 * meter_width;
		quad[3] = quad[2] + dir2 * (meter_width * total_length2 / total_length1);

		if (feat.m_code == WFC_WALL)
		{
			AddWallNormal(pEdge, &feat, quad);
		}
		if (feat.m_code == WFC_GAP)
		{
			// do nothing
		}
		if (feat.m_code == WFC_POST)
		{
			// TODO
		}
		if (feat.m_code == WFC_WINDOW)
		{
			AddWindowSection(pEdge, &feat, quad);
		}
		if (feat.m_code == WFC_DOOR)
		{
			AddDoorSection(pEdge, &feat, quad);
		}
		quad[0] = quad[1];
		quad[2] = quad[3];
	}
}

/**
 * Builds a wall, given material index, starting and end points, height, and
 * starting height.
 */
void vtBuilding3d::AddWallSection(vtEdge *pEdge, BldMaterial bmat,
								  const FLine3 &quad,
								  float vf1, float vf2, float hf1)
{
	// determine 4 points at corners of wall section
	FPoint3 up1 = (quad[2] - quad[0]);
	FPoint3 up2 = (quad[3] - quad[1]);
	FPoint3 p0 = quad[0] + (up1 * vf1);
	FPoint3 p1 = quad[1] + (up2 * vf1);
	FPoint3 p3 = quad[0] + (up1 * vf2);
	FPoint3 p2 = quad[1] + (up2 * vf2);

	vtMesh *mesh = FindMatMesh(bmat, pEdge->m_Color, true);

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0, p1, p2);

	// special case for texture
	FPoint2 uv0, uv1, uv2, uv3;
	if (bmat == BMAT_WINDOWWALL)
	{
		uv0.Set(0, 0);
		uv1.Set(hf1, 0);
		uv2.Set(hf1, vf2);
		uv3.Set(0, vf2);
	}
	else
	{
		uv0.Set(0, p0.y);
		uv1.Set(1, p1.y);
		uv2.Set(1, p2.y);
		uv3.Set(0, p3.y);
	}

	int start =
	mesh->AddVertexNUV(p0, norm, uv0);
	mesh->AddVertexNUV(p1, norm, uv1);
	mesh->AddVertexNUV(p2, norm, uv2);
	mesh->AddVertexNUV(p3, norm, uv3);

	mesh->AddFan(start, start+1, start+2, start+3);
}

void vtBuilding3d::AddWallNormal(vtEdge *pEdge, vtEdgeFeature *pFeat,
								 const FLine3 &quad)
{
	float h1 = pFeat->m_vf1;
	float h2 = pFeat->m_vf2;
	AddWallSection(pEdge, pEdge->m_Material, quad, h1, h2);
}

/**
 * Builds a door section.  will also build the wall above the door to ceiling
 * height.
 */
void vtBuilding3d::AddDoorSection(vtEdge *pEdge, vtEdgeFeature *pFeat,
								  const FLine3 &quad)
{
	float vf1 = 0;
	float vf2 = pFeat->m_vf2;

	// determine 4 points at corners of section
	FPoint3 up1 = (quad[2] - quad[0]);
	FPoint3 up2 = (quad[3] - quad[1]);
	FPoint3 p0 = quad[0] + (up1 * vf1);
	FPoint3 p1 = quad[1] + (up2 * vf1);
	FPoint3 p3 = quad[0] + (up1 * vf2);
	FPoint3 p2 = quad[1] + (up2 * vf2);

	vtMesh *mesh = FindMatMesh(BMAT_DOOR, pEdge->m_Color, true);

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0, p1, p2);

	int start =
	mesh->AddVertexNUV(p0, norm, FPoint2(1.0f, 1.0f));
	mesh->AddVertexNUV(p1, norm, FPoint2(0.0f, 1.0f));
	mesh->AddVertexNUV(p2, norm, FPoint2(0.0f, 0.0f));
	mesh->AddVertexNUV(p3, norm, FPoint2(1.0f, 0.0f));

	mesh->AddFan(start, start+1, start+2, start+3);

	//add wall above door
	AddWallSection(pEdge, pEdge->m_Material, quad, vf2, 1.0f);
}

//builds a window section.  builds the wall below and above a window too.
void vtBuilding3d::AddWindowSection(vtEdge *pEdge, vtEdgeFeature *pFeat,
									const FLine3 &quad)
{
	float vf1 = pFeat->m_vf1;
	float vf2 = pFeat->m_vf2;

	// build wall to base of window.
	AddWallSection(pEdge, pEdge->m_Material, quad, 0, vf1);

	// build wall above window
	AddWallSection(pEdge, pEdge->m_Material, quad, vf2, 1.0f);

	// determine 4 points at corners of section
	FPoint3 up1 = (quad[2] - quad[0]);
	FPoint3 up2 = (quad[3] - quad[1]);
	FPoint3 p0 = quad[0] + (up1 * vf1);
	FPoint3 p1 = quad[1] + (up2 * vf1);
	FPoint3 p3 = quad[0] + (up1 * vf2);
	FPoint3 p2 = quad[1] + (up2 * vf2);

	vtMesh *mesh = FindMatMesh(BMAT_WINDOW, pEdge->m_Color, true);

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0,p1,p2);

	int start =
	mesh->AddVertexNUV(p0, norm, FPoint2(1.0f, 1.0f));
	mesh->AddVertexNUV(p1, norm, FPoint2(0.0f, 1.0f));
	mesh->AddVertexNUV(p2, norm, FPoint2(0.0f, 0.0f));
	mesh->AddVertexNUV(p3, norm, FPoint2(1.0f, 0.0f));

	mesh->AddFan(start, start+1, start+2, start+3);
}


void vtBuilding3d::AddFlatRoof(FLine3 &pp, vtLevel *pLev)
{
	FPoint3 up(0.0f, 1.0f, 0.0f);	// vector pointing up
	int corners = pp.GetSize();
	int i, j;

	vtMesh *mesh = FindMatMesh(pLev->m_Edges[0]->m_Material,
		pLev->m_Edges[0]->m_Color, false);

	if (corners > 4)
	{
		// roof consists of a polygon which must be split into triangles
		FLine2 roof;
		roof.SetMaxSize(corners);
		for (i = 0; i < corners; i++)
			roof.Append(FPoint2(pp[i].x, pp[i].z));

		float roof_y = pp[0].y;

		// allocate a polyline to hold the answer.
		FLine2 result;

		//  Invoke the triangulator to triangulate this polygon.
		Triangulate_f::Process(roof, result);

		// use the results.
		int tcount = result.GetSize()/3;
		int ind[3];
		FPoint2 gp;
		FPoint3 p;

		for (i=0; i<tcount; i++)
		{
			for (j = 0; j < 3; j++)
			{
				gp = result[i*3+j];
				p.Set(gp.x, roof_y, gp.y);
				ind[j] = mesh->AddVertexN(p, up);
			}
			mesh->AddTri(ind[0], ind[1], ind[2]);
		}
	}
	else
	{
		int idx[MAX_WALLS];
		for (i = 0; i < corners; i++)
		{
			FPoint3 p = pp[i];
			idx[i] = mesh->AddVertexN(p, up);
		}
		if (corners > 2)
			mesh->AddTri(idx[0], idx[1], idx[2]);
		if (corners > 3)
			mesh->AddTri(idx[2], idx[3], idx[0]);
	}
}


void vtBuilding3d::CreateUniformLevel(int iLevel, float fHeight)
{
	vtLevel *pLev = m_Levels[iLevel];
	FLine3 poly1 = *m_lfp[iLevel];
	FLine3 poly2;

	int j;
	int edges = pLev->m_Edges.GetSize();
	for (j = 0; j < edges; j++)
		poly1[j].y = fHeight;

	poly2 = poly1;
	for (j = 0; j < edges; j++)
		poly2[j].y += pLev->m_fStoryHeight;

	for (j = 0; j < edges; j++)
	{
		int a = j, b = (a+1)%edges;

		FLine3 quad(4);

		vtEdge	*pWall = pLev->m_Edges[j];

		// do the whole wall section
		quad[0] = poly1[a];
		quad[1] = poly1[b];
		quad[2] = poly2[a];
		quad[3] = poly2[b];

		float h1 = 0.0f;
		float h2 = pLev->m_iStories;
		AddWallSection(pWall, BMAT_WINDOWWALL, quad, h1, h2,
			pWall->NumFeaturesOfCode(WFC_WINDOW));
	}
}

FPoint3 vtBuilding3d::Normal(const FPoint3 &p0, const FPoint3 &p1, const FPoint3 &p2)
{
	FPoint3 a = p0 - p1;
	FPoint3 b = p2 - p1;
	FPoint3 norm = b.Cross(a);
	norm.Normalize();
	return norm;
}

//
// Randomize buildings characteristics
//
void vtBuilding3d::Randomize(int iStories)
{
	RGBi color;

	color = GetColor(BLD_BASIC);
	if (color.r == -1 && color.g == -1 && color.b == -1)
	{
		// unset color
		// random pastel color
		unsigned char r, g, b;
		r = (unsigned char) (128 + random(127));
		g = (unsigned char) (128 + random(127));
		b = (unsigned char) (128 + random(127));
		SetColor(BLD_BASIC, RGBi(r, g, b));
	}

	color = GetColor(BLD_ROOF);
	if (color.r == -1 && color.g == -1 && color.b == -1)
	{
		// unset color
		// random roof color
		int r = rand() %5;
		switch (r) {
		case 0: color.Set(255, 255, 250); break;	//off-white
		case 1: color.Set(153, 51, 51); break;		//reddish
		case 2: color.Set(153, 153, 255); break;	//blue-ish
		case 3: color.Set(153, 255, 153); break;	//green-ish
		case 4: color.Set(178, 102, 51); break;		//brown
		}

		SetColor(BLD_ROOF, color);
	}
}


// Linear distance in RGB space
float ColorDiff(const RGBi &c1, const RGBi &c2)
{
	FPoint3 diff;
	diff.x = (c1.r - c2.r);
	diff.y = (c1.g - c2.g);
	diff.z = (c1.b - c2.b);
	return diff.Length();
}

//
// Takes the building material and color, and tries to find the closest
// existing vtMaterial.
//
int vtBuilding3d::FindMatIndex(BldMaterial bldMat, RGBi inputColor)
{
	if (s_Materials == NULL)
		return -1;

	if (bldMat == BMAT_WINDOW)	// only one kind of window
		return WINDOW_MAT;
	if (bldMat == BMAT_DOOR)	// only one door
		return DOOR_MAT;
	if (bldMat == BMAT_WOOD)	// only one wood
		return WOOD_MAT;
	if (bldMat == BMAT_CEMENT)	// only one cement
		return CEMENT_MAT;
	if (bldMat == BMAT_BRICK)
	{
		// choose one of our (currently 2) unpainted brick textures
		RGBi b1(159, 100, 83);	// (reddish medium brown)
		RGBi b2(183, 178, 171);	// (slightly pinkish grey)
		if (ColorDiff(inputColor, b1) < ColorDiff(inputColor, b2))
			return BRICK_MAT1;
		else
			return BRICK_MAT2;
	}

	// get the appropriate range in the indices
	int start = 0;
	int end = 0;
	switch (bldMat)
	{
	case BMAT_PLAIN:
		start = PLAIN_MAT_START;
		end = PLAIN_MAT_END;
		break;
	case BMAT_SIDING:
		start = SIDING_MAT_START;
		end = SIDING_MAT_END;
		break;
	case BMAT_WINDOWWALL:
		start = WINDOWWALL_MAT_START;
		end = WINDOWWALL_MAT_END;
		break;
	case BMAT_PAINTED_BRICK:
		start = PBRICK_MAT_START;
		end = PBRICK_MAT_END;
		break;
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
			bestMatch  = start + i;
			bestError = error;
		}
	}
	return bestMatch;
}


// implement vtStructure3d methods
bool vtBuilding3d::CreateNode(vtHeightField *hf, const char *options)
{
	bool roof = (strstr(options, "roof") != NULL);
	bool walls = (strstr(options, "walls") != NULL);
	bool details = (strstr(options, "detail") != NULL);
	CreateShape(hf, roof, walls, details);
	return true;
}

vtGeom *vtBuilding3d::GetGeom()
{
	return NULL;
}

void vtBuilding3d::DeleteNode()
{
	// not needed for buildings - those are automatically deleted when needed
}

/**
 * Display some bounding wires around the object to highlight it.
 */
void vtBuilding3d::ShowBounds(bool bShow)
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
