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
#include "HeightField.h"
#include "vtdata/Triangulate.h"
#include "Building3d.h"
#include "Terrain.h"

//an array of materials.  buildings pull their colors from a common list of materials.
//this is done to save memory.  for a list of 16000+ buildings, this can save about 200MB of RAM.
vtMaterialArray *vtBuilding3d::s_Materials = NULL;

#define PLAIN_MATS	216	//216 plain colors
#define SIDING_MATS 216 //216 colors with siding texture
#define WINDOW_MATS 2	//2 different windows
#define DOOR_MATS 1		//1 door
#define PLAIN_MAT_START		0	//start index for plain colors
#define PLAIN_MAT_END		PLAIN_MAT_START + PLAIN_MATS - 1	//end index for plain colors
#define SIDING_MAT_START	PLAIN_MAT_START + PLAIN_MATS		//start index for siding texture
#define SIDING_MAT_END		SIDING_MAT_START + SIDING_MATS - 1	//end index for siding texture
#define WINDOW_MAT_START	SIDING_MAT_START + SIDING_MATS			//start index for window material
#define WINDOW_MAT_END		WINDOW_MAT_START + WINDOW_MATS - 1	//end index for window material
#define DOOR_MAT_START		WINDOW_MAT_START + WINDOW_MATS		//start index for door material
#define DOOR_MAT_END		DOOR_MAT_START + DOOR_MATS - 1		//end index for door material

//helper to make a material
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

//////////////////////////////////////////////////////////////////////////////////

vtBuilding3d::vtBuilding3d()
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
	//create the shared materials if they haven't been already.
	int i, j, k, divisions;
	float start, step;
	RGBf color;
	vtMaterial *pMat;
	s_Materials = new vtMaterialArray();
	s_Materials->SetMaxSize(500);

	// create plain materials
	divisions = 6;
	start = .25f;
	step = (1.0f-start)/(divisions-1);
	for (i = 0; i < divisions; i++) {
		for (j = 0; j < divisions; j++) {
			for (k = 0; k < divisions; k++) {
				color.Set(start+i*step, start+j*step, start+k*step);
				pMat = makeMaterial(color, false);
				s_Materials->AppendMaterial(pMat);
			}
		}
	}

	//create siding materials (bright colors only)
	vtString path;
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/siding64.bmp");
	vtImage *pImageSiding = new vtImage(path);
	divisions = 6;
	start = .25f;
	step = (1.0f-start)/(divisions-1);
	for (i = 0; i < divisions; i++) {
		for (j = 0; j < divisions; j++) {
			for (k = 0; k < divisions; k++) {
				color.Set(start+i*step, start+j*step, start+k*step);
				pMat = makeMaterial(color, true);
				pMat->SetTexture(pImageSiding);
				pMat->SetClamp(false);
				s_Materials->AppendMaterial(pMat);
			}
		}
	}

	// window material
	color.Set(1.0f,1.0f,1.0f);
	pMat = makeMaterial(color, true);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/window.bmp");
	pMat->SetTexture2(path);
	pMat->SetClamp(false);
	//window stays on at night
	pMat->SetLighting(false);
	s_Materials->Append(pMat);

	color.Set(1.0f,1.0f,1.0f);
	pMat = makeMaterial(color, true);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/window.bmp");
	pMat->SetTexture2(path);
	pMat->SetClamp(false);
	s_Materials->Append(pMat);

	// door material
	color.Set(1.0f, 1.0f, 1.0f);
	pMat = makeMaterial(color, true);
	path = FindFileOnPaths(vtTerrain::m_DataPaths, "BuildingModels/door.bmp");
	pMat->SetTexture2(path);
	s_Materials->Append(pMat);

	int total = s_Materials->GetSize();
	int expectedtotal = PLAIN_MATS + SIDING_MATS + WINDOW_MATS + DOOR_MATS;
	if (total != expectedtotal) {
		assert(false);
	}
}

void vtBuilding3d::FindMaterialIndices()
{
	if (s_Materials == NULL)
		CreateSharedMaterials();
}


#define MOULDING_WIDTH	0.2f

//
// Convert the footprint for each level into world coordinates
//
void vtBuilding3d::DetermineWorldFootprints(vtHeightField *pHeightField)
{
	int i, j;
	int levs = GetNumLevels();
	FPoint3 p3;

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
		// do something about it - like deform the ground or build
		//  some posts on which to set the building
	}
	float base_level;
	if (m_bElevated)
		base_level = ymax;
	else
		// embed the building in the ground
		base_level = ymin;

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
}

vtTransform *vtBuilding3d::GetTransform()
{
	return m_pContainer;
}

void vtBuilding3d::AdjustHeight(vtHeightField *pHeightField)
{
	DetermineWorldFootprints(pHeightField);
	m_pContainer->SetTrans(m_center);
}

void vtBuilding3d::CreateGeometry(vtHeightField *pHeightField, bool bDoRoof,
								   bool bDoWalls, bool details)
{
	int i, j, k;

	// make sure we've got materials first
	FindMaterialIndices();

	DetermineWorldFootprints(pHeightField);
	int corners = m_lfp[0]->GetSize();

	// Potential Optimization: should calculate how many vertices the building
	// will take.  Even the simplest building will use 20 vertices, for now just
	// use 40 as a reasonable starting point for each component mesh.

/*	// ready to start creating geometry
	m_pMesh[BM_WALL] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);
	m_pMesh[BM_TRIM] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);
	m_pMesh[BM_DOOR] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);
	m_pMesh[BM_WINDOW] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);

	// assume roof outline same as base footprint (TODO - make general)
	if (m_Levels[0]->GetFootprint().GetSize() > 4)
		mesh = new vtMesh(GL_TRIANGLES, VT_Normals | VT_TexCoords, 40);
	else
		mesh = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);
*/

	// create the walls
	float fHeight = 0.0f;
	int iLevels = GetNumLevels();
	if (bDoWalls)
	{
		for (i = 0; i < iLevels; i++)
		{
			vtLevel *lev = m_Levels[i];
			for (j = 0; j < lev->m_iStories; j++)
			{
				for (k = 0; k < corners; k++)
					CreateWallGeometry(*m_lfp[i], lev, fHeight, k, details);
				fHeight += lev->m_fStoryHeight;
			}
		}
	}

	FLine3 *roof = m_lfp[iLevels-1];	// roof: top level
	vtLevel *roof_lev = m_Levels[iLevels-1];
	float roof_height = (roof_lev->m_fStoryHeight * roof_lev->m_iStories);

	// create the roof
	if (bDoRoof)
	{
		switch (m_RoofType)
		{
			case ROOF_FLAT:
				AddFlatRoof(*roof, roof_height);
				break;
			case ROOF_SHED:
				AddShedRoof(*roof, roof_height);
				break;
			case ROOF_GABLE:
				AddGableRoof(*roof, roof_height);
				break;
			case ROOF_HIP:
				AddHipRoof(*roof, roof_height);
				break;
		}
	}

	// wrap in a shape and set materials
	m_pGeom = new vtGeom();
	m_pGeom->SetName2("building-geom");
	m_pGeom->SetMaterials(s_Materials);

	for (i = 0; i < m_Mesh.GetSize(); i++)
	{
		m_pGeom->AddMesh(m_Mesh[i].m_pMesh, m_Mesh[i].m_iMatIdx);
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


////////////////////////////////////////////////////////////////////////////

vtMesh *vtBuilding3d::FindMatMesh(BldMaterial bm, RGBi color, bool bFans)
{
	int mi = FindMatIndex(bm, color);

	int i, size = m_Mesh.GetSize();
	for (i = 0; i < size; i++)
	{
		if (m_Mesh[i].m_iMatIdx == mi)
			return m_Mesh[i].m_pMesh;
	}
	// did find it, so we need to make it
	MatMesh mm;
	mm.m_iMatIdx = mi;
	mm.m_pMesh = new vtMesh(bFans ? GL_TRIANGLE_FAN : GL_TRIANGLES,
		VT_Normals | VT_TexCoords, 40);
	i = m_Mesh.Append(mm);
	return mm.m_pMesh;
}

//walls are created in panels (sections)
void vtBuilding3d::CreateWallGeometry(Array<FPoint3> &corners, vtLevel *pLev,
									  float fBase, int iWall, bool details)
{
	int num_walls = pLev->m_Wall.GetSize();
	int i = iWall, j = (i+1)%num_walls;

	FPoint3 point[30];

	vtWall	*pWall = pLev->m_Wall[iWall];

	// start with the whole wall section
	point[0] = corners[i];
	point[0].y = fBase;
	point[1] = corners[j];
	point[1].y = fBase;

	// figure out how many panels we have (walls sections, windows sections,
	// doors, moulding)
	int totalfeatures = pWall->NumFeatures();

	// length of the wall
	FPoint3 direction = point[1] - point[0];
	float total_length = direction.Length();
	direction.Normalize();

	// How wide should each feature be?
	// Determine how much space we have for the proportional features after
	// accounting for the fixed-width features
	float fixed_width = pWall->FixedFeaturesWidth();
	float total_prop = pWall->ProportionTotal();
	float dyn_width = total_length - fixed_width;

	// build the wall features.
	// point[0] is the first starting point of a panel.
	for (i = 0; i < pWall->NumFeatures(); i++)
	{
		vtWallFeature &feat = pWall->m_Features[i];

		// determine real width
		float meter_width = 0.0f;
		if (feat.m_width >= 0)
			meter_width = feat.m_width;
		else
			meter_width = (feat.m_width / total_prop) * dyn_width;
		point[1] = point[0] + direction*meter_width;

		if (feat.m_code == WFC_WALL)
		{
			AddWallNormal(pLev, pWall, &feat, point[0], point[1]);
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
			AddWindowSection(pLev, pWall, &feat, point[0], point[1]);
		}
		if (feat.m_code == WFC_DOOR)
		{
			AddDoorSection(pLev, pWall, &feat, point[0], point[1]);
		}
		point[0] = point[1];
	}
}

/**
 * Builds a wall, given material index, starting and end points, height, and
 * starting height.
 */
void vtBuilding3d::AddWallSection(vtLevel *pLev, vtWall *pWall,
								  vtWallFeature *pFeat,
								  FPoint3 p0, FPoint3 p1,
								  float h1, float h2)
{
	// determine 2 points at top of wall
	FPoint3 p2 = p1, p3 = p0;
	p0.y += h1;
	p1.y += h1;
	p2.y += h2;
	p3.y += h2;

	vtMesh *mesh = FindMatMesh(pWall->m_Material, m_Color, true);

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0, p1, p2);

	int start =
	mesh->AddVertexNUV(p0, norm, FPoint2(1.0f, p0.y));
	mesh->AddVertexNUV(p1, norm, FPoint2(0.0f, p1.y));
	mesh->AddVertexNUV(p2, norm, FPoint2(0.0f, p2.y));
	mesh->AddVertexNUV(p3, norm, FPoint2(1.0f, p3.y));

	mesh->AddFan(start, start+1, start+2, start+3);
}

void vtBuilding3d::AddWallNormal(vtLevel *pLev, vtWall *pWall,
								 vtWallFeature *pFeat,
								 const FPoint3 &p0, const FPoint3 &p1)
{
	float h1 = (pFeat->m_vf1 * pLev->m_fStoryHeight);
	float h2 = (pFeat->m_vf2 * pLev->m_fStoryHeight);
	AddWallSection(pLev, pWall, pFeat, p0, p1, h1, h2);
}

/**
 * Builds a door section.  will also build the wall above the door to ceiling
 * height.
 */
void vtBuilding3d::AddDoorSection(vtLevel *pLev, vtWall *pWall,
								  vtWallFeature *pFeat,
								  const FPoint3 &p0, const FPoint3 &p1)
{
	float h1 = 0;
	float h2 = (pFeat->m_vf2 * pLev->m_fStoryHeight);
	float top = pLev->m_fStoryHeight;

	// determine 2 points at top of wall
	FPoint3 p2 = p1, p3 = p0;
	p2.y += h2;
	p3.y += h2;

	vtMesh *mesh = FindMatMesh(BMAT_DOOR, m_Color, true);

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0, p1, p2);

	int start =
	mesh->AddVertexNUV(p0, norm, FPoint2(1.0f, 1.0f));
	mesh->AddVertexNUV(p1, norm, FPoint2(0.0f, 1.0f));
	mesh->AddVertexNUV(p2, norm, FPoint2(0.0f, 0.0f));
	mesh->AddVertexNUV(p3, norm, FPoint2(1.0f, 0.0f));

	mesh->AddFan(start, start+1, start+2, start+3);

	//add wall above door
	AddWallSection(pLev, pWall, pFeat, p0, p1, h2, top);
}

//builds a window section.  builds the wall below and above a window too.
void vtBuilding3d::AddWindowSection(vtLevel *pLev, vtWall *pWall,
									vtWallFeature *pFeat,
									FPoint3 p0, FPoint3 p1)
{
	float h1 = (pFeat->m_vf1 * pLev->m_fStoryHeight);
	float h2 = (pFeat->m_vf2 * pLev->m_fStoryHeight);
	float top = pLev->m_fStoryHeight;

	// build wall to base of window.
	AddWallSection(pLev, pWall, pFeat, p0, p1, 0, h1);

	// build wall above window
	AddWallSection(pLev, pWall, pFeat, p0, p1, h2, top);

	// determine 2 points at top of wall
	FPoint3 p2 = p1, p3 = p0;
	p0.y += h1;
	p1.y += h1;
	p2.y += h2;
	p3.y += h2;

	vtMesh *mesh = FindMatMesh(BMAT_WINDOW, m_Color, true);

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0,p1,p2);

	int start =
	mesh->AddVertexNUV(p0, norm, FPoint2(1.0f, 1.0f));
	mesh->AddVertexNUV(p1, norm, FPoint2(0.0f, 1.0f));
	mesh->AddVertexNUV(p2, norm, FPoint2(0.0f, 0.0f));
	mesh->AddVertexNUV(p3, norm, FPoint2(1.0f, 0.0f));

	mesh->AddFan(start, start+1, start+2, start+3);
}


void vtBuilding3d::AddFlatRoof(Array<FPoint3> &pp, float height)
{
	FPoint3 up(0.0f, 1.0f, 0.0f);	// vector pointing up
	int corners = pp.GetSize();
	int i, j;

	vtMesh *mesh = FindMatMesh(m_RoofMaterial, m_RoofColor, false);

	if (m_Levels[0]->GetFootprint().GetSize() > 4)
	{
		// roof consists of a polygon which must be split into triangles
		FLine2 roof;
		for (i = 0; i < corners; i++)
			roof.Append(FPoint2(pp[i].x, pp[i].z));

		float roof_y = pp[0].y + height;

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
			p.y += height;
			idx[i] = mesh->AddVertexN(p, up);
		}
		mesh->AddTri(idx[0], idx[1], idx[2]);
		mesh->AddTri(idx[2], idx[3], idx[0]);
	}
}

void vtBuilding3d::AddShedRoof(Array<FPoint3> &pp, float height)
{
	vtLevel *pTopLev = m_Levels[GetNumLevels()-1];
	if (pTopLev->GetFootprint().GetSize() != 4)
	{
		AddFlatRoof(pp, height);
		return;
	}

	vtMesh *mesh;
	mesh = FindMatMesh(m_RoofMaterial, m_RoofColor, false);

	FPoint3 norm;
	int corners = pp.GetSize();
	int idx[4];
	FPoint3 p[6];
	for (int i = 0; i < 4; i++)
	{
		p[i] = pp[i];
		p[i].y += height;
		if (i < 2)
		{
			p[i+4] = p[i];
			p[i+4].y += (height / 3.0f);
		}
	}
	// Add top
	norm = Normal(p[4], p[5], p[2]);
	idx[0] = mesh->AddVertexN(p[4], norm);
	idx[1] = mesh->AddVertexN(p[5], norm);
	idx[2] = mesh->AddVertexN(p[2], norm);
	idx[3] = mesh->AddVertexN(p[3], norm);
	mesh->AddFan(idx, 4);

	vtWall *pWall;
	pWall = pTopLev->m_Wall[0];
	mesh = FindMatMesh(pWall->m_Material, m_Color, false);

	// Add front
	norm = Normal(p[0], p[1], p[5]);
	idx[0] = mesh->AddVertexNUV(p[0], norm, FPoint2(0.0f, p[0].y));
	idx[1] = mesh->AddVertexNUV(p[1], norm, FPoint2(1.0f, p[1].y));
	idx[2] = mesh->AddVertexNUV(p[5], norm, FPoint2(1.0f, p[5].y));
	idx[3] = mesh->AddVertexNUV(p[4], norm, FPoint2(0.0f, p[4].y));
	mesh->AddFan(idx, 4);

	pWall = pTopLev->m_Wall[3];
	mesh = FindMatMesh(pWall->m_Material, m_Color, false);

	// Add left
	norm = Normal(p[3], p[0], p[4]);
	idx[0] = mesh->AddVertexNUV(p[3], norm, FPoint2(0.0f, p[3].y));
	idx[1] = mesh->AddVertexNUV(p[0], norm, FPoint2(1.0f, p[0].y));
	idx[2] = mesh->AddVertexNUV(p[4], norm, FPoint2(1.0f, p[4].y));
	mesh->AddFan(idx, 3);

	pWall = pTopLev->m_Wall[1];
	mesh = FindMatMesh(pWall->m_Material, m_Color, false);

	// Add right
	norm = Normal(p[1], p[2], p[5]);
	idx[0] = mesh->AddVertexNUV(p[1], norm, FPoint2(0.0f, p[1].y));
	idx[1] = mesh->AddVertexNUV(p[2], norm, FPoint2(1.0f, p[2].y));
	idx[2] = mesh->AddVertexNUV(p[5], norm, FPoint2(0.0f, p[5].y));
	mesh->AddFan(idx, 3);
}

void vtBuilding3d::AddHipRoof(Array<FPoint3> &pp, float height)
{
	if (m_Levels[0]->GetFootprint().GetSize() != 4)
	{
		AddFlatRoof(pp, height);
		return;
	}

	FPoint3 v[10];
	float roofheight;
	float adjust;
	FPoint3 norm;
	int i;
	int corners = pp.GetSize();

	//copy points to array...
	for (i = 0; i < corners; i++)
	{
		v[i] = pp[i];
		v[i].y += height;
	}

	//assume 4 points:
	//the longer side is either 0->1 or 0->3
	//points arranged:
	//	3.----2. 3.------2.
	//	| \5./|  | \    /|
	//  |  |  |  |  4.-5.|
	//  | /4.\|  | /    \|
	//	0.----1. 0.------1.

	int bottom_left;
	int top_left;
	int top_right;
	int bottom_right;
	int bottom_middle = 4;
	int top_middle = 5;

	FPoint3 long_edge, short_edge;
	FPoint3 edge1 = v[1] - v[0];
	FPoint3 edge2 = v[3] - v[0];
	float len1 = edge1.Length();
	float len2 = edge2.Length();

	if (len1 > len2)
	{
		bottom_left = 3;
		bottom_right = 0;
		top_right = 1;
		top_left = 2;
	}
	else
	{
		bottom_left = 0;
		bottom_right = 1;
		top_right = 2;
		top_left = 3;
	}
	long_edge = v[top_left] - v[bottom_left];
	short_edge = v[bottom_right] - v[bottom_left];
	float short_len = short_edge.Length();
	long_edge.Normalize();
	short_edge.Normalize();
	roofheight = short_len/4;

	//add top points of roof.
	adjust = short_len/2;

	v[bottom_middle] = (v[bottom_left] + v[bottom_right])/2;
	v[bottom_middle].y += roofheight;
	v[bottom_middle] += (long_edge * adjust);
	v[top_middle] = (v[top_left] + v[top_right])/2;
	v[top_middle].y += roofheight;
	v[top_middle] -= (long_edge * adjust);

#if 1
	//adjust points for roof overhang
	adjust = .6f;
	FPoint3 offset;

	offset = - (short_edge * adjust) - (long_edge * adjust);
	v[bottom_left] += offset;

	offset =   (short_edge * adjust) - (long_edge * adjust);
	v[bottom_right] += offset;

	offset = - (short_edge * adjust) + (long_edge * adjust);
	v[top_left] += offset;

	offset =   (short_edge * adjust) + (long_edge * adjust);
	v[top_right] += offset;

	//subtract from y:
	adjust = adjust / 2;
	v[bottom_left].y -= adjust;
	v[bottom_right].y -= adjust;
	v[top_left].y -= adjust;
	v[top_right].y -= adjust;
#endif

	//construct the roof
	BuildRoofPanel(v, 3, bottom_left, bottom_right, bottom_middle);
	BuildRoofPanel(v, 3, top_middle, top_right, top_left);
	BuildRoofPanel(v, 4, top_middle, top_left, bottom_left, bottom_middle);
	BuildRoofPanel(v, 4, bottom_middle, bottom_right, top_right, top_middle);
}

void vtBuilding3d::AddGableRoof(Array<FPoint3> &pp, float height)
{
#if 0
	if (m_Levels[0]->GetFootprint().GetSize() != 4)
	{
		AddFlatRoof(pp, height);
		return;
	}

	FPoint3 v[MAX_WALLS];
	int idx[MAX_WALLS];
	float roofheight;
	float adjust;
	FPoint3 norm, axis1, axis2;
	int i;
	int corners = pp.GetSize();

	//copy points to array...
	for (i = 0; i < corners; i++)
	{
		v[i] = pp[i];
		v[i].y += height;
	}

	axis1 = v[1] - v[0];
	axis1.Normalize();
	axis2 = v[3] - v[0];
	axis2.Normalize();

	float fWidth, fDepth;
	GetRectangle(fWidth, fDepth);

	roofheight = fWidth/4;
	//the longer side is 0->1, 2->3
	//the shorter side is 0->3, 1->2
	//add top points of roof.
	int bottom_left = 0;
	int top_left = 3;
	int top_right = 2;
	int bottom_right = 1;
	int bottom_middle = 4;
	int top_middle = 5;
	//points arranged:
	//	3.-5.-2.
	//	|  |  |
	//  |  |  |
	//  |  |  |
	//	0.-4.-1.

	v[bottom_middle] = (v[bottom_left] + v[bottom_right])/2;
	v[bottom_middle].y += roofheight;
	v[top_middle] = (v[top_left] + v[top_right])/2;
	v[top_middle].y += roofheight;

	//construct wall extensions.
	norm = Normal(v[bottom_left],v[bottom_right],v[bottom_middle]);
	idx[0] = m_pMesh[BM_WALL]->AddVertexNUV(v[bottom_left], norm, FPoint2(0.0f, v[bottom_left].y));
	idx[1] = m_pMesh[BM_WALL]->AddVertexNUV(v[bottom_right], norm, FPoint2(1.0f, v[bottom_right].y));
	idx[2] = m_pMesh[BM_WALL]->AddVertexNUV(v[bottom_middle], norm, FPoint2(0.0f, v[bottom_middle].y));
	m_pMesh[BM_WALL]->AddFan(idx, 3);

	norm = Normal(v[top_right],v[top_left],v[top_middle]);
	idx[0] = m_pMesh[BM_WALL]->AddVertexNUV(v[top_right], norm, FPoint2(0.0f, v[top_right].y));
	idx[1] = m_pMesh[BM_WALL]->AddVertexNUV(v[top_left], norm, FPoint2(1.0f, v[top_left].y));
	idx[2] = m_pMesh[BM_WALL]->AddVertexNUV(v[top_middle], norm, FPoint2(0.0f, v[top_middle].y));
	m_pMesh[BM_WALL]->AddFan(idx, 3);

#if 1
	//adjust points for roof overhang
	//add to right;
	adjust = .6f;
	v[top_right] += (axis1 * adjust);
	v[bottom_right] += (axis1 * adjust);
	//subtract from left;
	v[bottom_left] -= (axis1 * adjust);
	v[top_left] -= (axis1 * adjust);
	//add to top;
	v[top_left] += (axis2 * adjust);
	v[top_right] += (axis2 * adjust);
	v[top_middle] += (axis2 * adjust);
	//subtract from bottom;
	v[bottom_left] -= (axis2 * adjust);
	v[bottom_right] -= (axis2 * adjust);
	v[bottom_middle] -= (axis2 * adjust);
	//subtract from y:
	adjust = .6f * roofheight/(fWidth/2);
	v[bottom_left].y -= adjust;
	v[top_left].y -= adjust;
	v[top_right].y -= adjust;
	v[bottom_right].y -= adjust;
#endif

	//construct the roof
	BuildRoofPanel(v, 4, top_middle, top_left, bottom_left, bottom_middle);
	BuildRoofPanel(v, 4, bottom_middle, bottom_right, top_right, top_middle);
#endif
}

//
// Given points, constructs a panel for the roof.
// n is how many points are to be used.
// indices to the array should follow (...)
// points should be given in a counter clockwise order.
//
void vtBuilding3d::BuildRoofPanel(FPoint3 *v, int n, ...)
{
#if 0
	int idx[MAX_WALLS];
	int i;
	va_list ap;

	va_start(ap, n);
	int p0 = va_arg(ap, int);
	int p1 = va_arg(ap, int);
	int p2 = va_arg(ap, int);
	FPoint3 norm = Normal(v[p0], v[p1], v[p2]);
	va_end(ap);

	va_start(ap, n);
	for (i = 0; i < n; i++)
		idx[i] = mesh->AddVertexN(v[va_arg(ap, int)], norm);
	va_end(ap);

	mesh->SetVtxTexCoord(idx[0], FPoint2(0.0f, 1.0f));
	mesh->SetVtxTexCoord(idx[1], FPoint2(0.0f, 0.0f));
	mesh->SetVtxTexCoord(idx[2], FPoint2(1.0f, 0.0f));
	if (n > 3)
		mesh->SetVtxTexCoord(idx[3], FPoint2(1.0f, 1.0f));
	mesh->AddFan(idx, n);
#endif
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


//
// Takes the building material and color, and tries to find the closest
// existing vtMaterial.
//
int vtBuilding3d::FindMatIndex(BldMaterial bldMat, RGBi inputColor)
{
	if (s_Materials == NULL)
		return -1;

	if (bldMat == BMAT_WINDOW) {
		//only one kinda of window.
		//caller knows to choose between the 2 windows
		return WINDOW_MAT_START;
	} else if (bldMat == BMAT_DOOR) {
		//only one door.
		return DOOR_MAT_START;
	}

	int start = 0;
	int end = 0;
	//get the appropriate range in the index
	switch (bldMat) {
	case BMAT_PLAIN:
		start = PLAIN_MAT_START;
		end = PLAIN_MAT_END + 1;
		break;
	case BMAT_SIDING:
		start = SIDING_MAT_START;
		end = SIDING_MAT_END + 1;
		break;
	}

	// match the closest color.
	vtMaterial *app;
	RGBf colorD;
	RGBf colorA;
	RGBf color;

	RGBf color2 = inputColor;
	float bestError = 10000000.0f;
	int bestMatch = -1;
	float error;
	FPoint3 diff;

	for (int i = start; i < end; i++)
	{
		app = s_Materials->GetAt(i);
		color = app->GetDiffuse();
		diff.x = (color.r - color2.r)*100;
		diff.y = (color.g - color2.g)*100;
		diff.z = (color.b - color2.b)*100;
		error = diff.x*diff.x  + diff.y*diff.y + diff.z*diff.z;

		if (error < bestError) {
			bestMatch  = i;
			bestError = error;
		}
	}
	return bestMatch;
}

