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
#include "vtlib/core/HeightField.h"
#include "vtdata/Triangulate.h"
#include "Building3d.h"
#include "Terrain.h"

//an array of materials.  buildings pull their colors from a common list of materials.
//this is done to save memory.  for a list of 16000+ buildings, this can save about 200MB of RAM.
vtMaterialArray *vtBuilding3d::s_Materials = NULL;

#define PLAIN_MATS	216	//216 plain colors
#define SIDING_MATS 216 //216 colors with siding texture
#define ROOF_MATS	216	//216 roof colors (separate from plain because roofs do not have back face culling.)
#define WINDOW_MATS 2	//2 different windows
#define DOOR_MATS 1		//1 door
#define PLAIN_MAT_START		0	//start index for plain colors
#define PLAIN_MAT_END		PLAIN_MAT_START + PLAIN_MATS - 1	//end index for plain colors
#define SIDING_MAT_START	PLAIN_MAT_START + PLAIN_MATS		//start index for siding texture
#define SIDING_MAT_END		SIDING_MAT_START + SIDING_MATS - 1	//end index for siding texture
#define ROOF_MAT_START		SIDING_MAT_START + SIDING_MATS		//start index for roof material
#define ROOF_MAT_END		ROOF_MAT_START + ROOF_MATS - 1		//end index for roof material
#define WINDOW_MAT_START	ROOF_MAT_START + ROOF_MATS			//start index for window material
#define WINDOW_MAT_END		WINDOW_MAT_START + WINDOW_MATS - 1	//end index for window material
#define DOOR_MAT_START		WINDOW_MAT_START + WINDOW_MATS		//start index for door material
#define DOOR_MAT_END		DOOR_MAT_START + DOOR_MATS - 1		//end index for door material

//helper to make a material
vtMaterial *makeMaterial(RGBf &color, bool culling)
{
	vtMaterial *pMat = new vtMaterial();
	pMat->SetDiffuse1(color);
	pMat->SetAmbient1(color/4.0f);
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

	for (int i = 0; i < BM_TOTAL; i++)
		m_pMesh[i] = NULL;
}

vtBuilding3d::~vtBuilding3d()
{
	for (int i = 0; i < BM_TOTAL; i++)
		delete m_pMesh[i];
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

	//create plain materials
	divisions = 6;
	start = .25f;
	step = (1.0f-start)/(divisions-1);
	for (i = 0; i < divisions; i++) {
		for (j = 0; j < divisions; j++) {
			for (k = 0; k < divisions; k++) {
				color.Set(start+i*step, start+j*step, start+k*step);
				pMat = makeMaterial(color, true);
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
				s_Materials->AppendMaterial(pMat);
			}
		}
	}

	//create roof materials (custom)
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
	int expectedtotal = PLAIN_MATS + SIDING_MATS + ROOF_MATS + WINDOW_MATS + DOOR_MATS;
	if (total != expectedtotal) {
		assert(false);
	}
}

void vtBuilding3d::FindMaterialIndices()
{
	if (s_Materials == NULL)
		CreateSharedMaterials();

	m_iMatIdx[BM_WALL] = FindMatIndex(BAP_PLAIN, m_Color);
	m_iMatIdx[BM_ROOF] = FindMatIndex(BAP_ROOF, m_RoofColor);
	m_iMatIdx[BM_TRIM] = FindMatIndex(BAP_PLAIN, m_MouldingColor);
	m_iMatIdx[BM_WINDOW] = FindMatIndex(BAP_WINDOW);
	m_iMatIdx[BM_DOOR] = FindMatIndex(BAP_DOOR);
}


// helper
void rotate_vector2d(FPoint2 &vec, double angle)
{
	float x = vec.x;
	float y = vec.y;
	vec.x = (float)(cos(angle) * x - sin(angle) * y);
	vec.y = (float)(sin(angle) * x + cos(angle) * y);
}

#define DOOR_WIDTH		1.0f
#define WINDOW_WIDTH	1.3f
#define MOULDING_WIDTH	0.2f
#define CEILING_HEIGHT	2.8f
#define DOOR_HEIGHT		2.3f
#define WINDOW_HEIGHT	1.3f
#define WINDOW_BOTTOM	1.0f

void vtBuilding3d::DetermineBaseCorners(vtHeightField *pHeightField,
										FLine3 &base_corner)
{
	int i;

	switch (m_BldShape)
	{
	case SHAPE_RECTANGLE:
		RectToPoly();
		break;
	}
	int corners = m_Footprint.GetSize();

	// find where the corner points are on the ground
	FPoint3 p3;
	FLine3 corner3d;
	corner3d.SetSize(corners);

	for (i = 0; i < corners; i++)
	{
		pHeightField->ConvertEarthToSurfacePoint(m_Footprint[i], p3);
		corner3d.SetAt(i, p3);
	}

	// determine the highest point of the corners
	float ymax = -10000.0f, ymin = 10000.0f;
	int highest, lowest;
	for (i = 0; i < corners; i++)
	{
		if (corner3d[i].y > ymax)
		{
			ymax = corner3d[i].y;
			highest = i;
		}
		if (corner3d[i].y < ymin)
		{
			ymin = corner3d[i].y;
			lowest = i;
		}
	}
	if ((ymax - ymin) > (1.0f * WORLD_SCALE))	// it's on a slope
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
	// The base corners are expressed relative to the origin of the building
	// (they are in the building's local coordinate system)
	//
	base_corner.SetSize(corners);
	for (i = 0; i < corners; i++)
	{
		p3 = corner3d[i] - m_center;
		p3.y = 0.0f;
		base_corner.SetAt(i, p3);
	}
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
	FLine3 base_corner;
	DetermineBaseCorners(pHeightField, base_corner);
	m_pContainer->SetTrans(m_center);
}

void vtBuilding3d::CreateGeometry(vtHeightField *pHeightField, bool bDoRoof,
								   bool bDoWalls, bool details)
{
	int i, j;

	// make sure we've got materials first
	FindMaterialIndices();

	FLine3 base_corner;
	DetermineBaseCorners(pHeightField, base_corner);
	int corners = base_corner.GetSize();

	// Potential Optimization: should calculate how many vertices the building
	// will take.  Even the simplest building will use 20 vertices, for now just
	// use 40 as a reasonable starting point for each component mesh.

	// ready to start creating geometry
	m_pMesh[BM_WALL] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);
	m_pMesh[BM_TRIM] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);
	m_pMesh[BM_DOOR] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);
	m_pMesh[BM_WINDOW] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);

	m_bPolyRoof = false;
	if (m_BldShape == SHAPE_POLY && m_Footprint.GetSize() > 4)
		m_bPolyRoof = true;

	if (m_bPolyRoof)
		m_pMesh[BM_ROOF] = new vtMesh(GL_TRIANGLES, VT_Normals | VT_TexCoords, 40);
	else
		m_pMesh[BM_ROOF] = new vtMesh(GL_TRIANGLE_FAN, VT_Normals | VT_TexCoords, 40);

	// create the walls
	if (bDoWalls)
	{
		for (i = 0; i < m_Story.GetSize(); i++)
		{
			for (j = 0; j < corners; j++)
				CreateWallGeometry(base_corner, i, j, details);
		}
	}

	float roof_height = (CEILING_HEIGHT * m_Story.GetSize()) * WORLD_SCALE;

	// create the roof
	if (bDoRoof)
	{
		switch (m_RoofType)
		{
			case ROOF_FLAT:
				AddFlatRoof(base_corner, roof_height);
				break;
			case ROOF_SHED:
				AddShedRoof(base_corner, roof_height);
				break;
			case ROOF_GABLE:
				AddGableRoof(base_corner, roof_height);
				break;
			case ROOF_HIP:
				AddHipRoof(base_corner, roof_height);
				break;
		}
	}

	// wrap in a shape and set materials
	m_pGeom = new vtGeom();
	m_pGeom->SetName2("building-geom");
	m_pGeom->SetMaterials(s_Materials);

	for (i = 0; i < BM_TOTAL; i++)
	{
		if (m_pMesh[i]->GetNumPrims() != 0)
			m_pGeom->AddMesh(m_pMesh[i], m_iMatIdx[i]);
		else
		{
			delete m_pMesh[i];
			m_pMesh[i] = NULL;
		}
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

//walls are created in panels (sections)
void vtBuilding3d::CreateWallGeometry(Array<FPoint3> &corners, int iStory,
									  int iWall, bool details)
{
	float fFloor = (iStory * CEILING_HEIGHT) * WORLD_SCALE;
	float fCeiling = ((iStory+1) * CEILING_HEIGHT) * WORLD_SCALE;

	int num_walls = m_Story[iStory]->m_Wall.GetSize();

	int i = iWall, j = (i+1)%num_walls;

	FPoint3 point[30];

	vtWall	*pWall = m_Story[iStory]->m_Wall[iWall];

	int numDoors = 0;
	int numWindows = 0;
	bool bMoulding = false;
	if (details) {
		bMoulding = m_bMoulding;
		numDoors = pWall->m_iDoors;
		numWindows = pWall->m_iWindows;
	}
	// start with the simple wall
	point[0] = corners[i];
	point[0].y = fFloor;
	point[1] = corners[j];
	point[1].y = fFloor;

	//choose wall material
	switch (pWall->m_Type)
	{
	case WALL_FLAT:
		m_iMatIdx[BM_WALL] = FindMatIndex(BAP_PLAIN, m_Color); break;
	case WALL_SIDING:
		m_iMatIdx[BM_WALL] = FindMatIndex(BAP_SIDING, m_Color); break;
	case WALL_GLASS:
		m_iMatIdx[BM_WALL] = FindMatIndex(BAP_SIDING, m_Color); break;  //unsupported for now
	}

	// figure out how many panels we have (walls sections, windows sections,
	// doors, moulding)
	int totalfeatures = numDoors + numWindows;
	FPoint3 direction = point[1] - point[0];
	// length of the wall
	float dist = direction.Length();
	direction.Normalize();

	// how wide should each wall section be?
	// this is how much space we have for the walls after accounting for
	// doors and windows
	float sectionLength = dist - (numDoors * DOOR_WIDTH * WORLD_SCALE +
								numWindows * WINDOW_WIDTH * WORLD_SCALE);

	if (sectionLength < 0) {
//		assert(false);  //uh oh!  our features are wider than the wall.
	}

	int totalsections;
	if (sectionLength < 0)
	{
		numDoors = numWindows = 0;
		sectionLength = dist;
		totalsections = 1;
	}
	else
	{
		if (bMoulding) {
			//account for moulding.
			sectionLength -= MOULDING_WIDTH * 2.0f * WORLD_SCALE;
			//now sectionLength equals length per wall panel
			sectionLength = sectionLength/(totalfeatures+1);
			totalsections = totalfeatures * 2 + 3;
		} else {
			//now sectionLength equals length per wall panel
			sectionLength = sectionLength/(totalfeatures+1);
			totalsections = totalfeatures * 2 + 1;
		}
	}

	// boolean used within loop to say whether the next panel should be a
	// door or not.
	bool door;
	// second boolean used to actually signal whether to build a door or not,
	// looking at the variable door and other factors.
	bool makedoor;
	int doorcount = 0;
	int windowcount = 0;
	bool madefirstfeature = false;
	// build the wall.  point[0] is the first starting point of a panel.
	for (i = 0; i < totalsections; i++)
	{
		//add moulding if necessary (only at beginning or end)
		if (bMoulding && (i == 0 || i == totalsections - 1))
		{
			point[1] = point[0] + direction*MOULDING_WIDTH*WORLD_SCALE;
			AddWallSection(BM_TRIM, point[0], point[1], CEILING_HEIGHT * WORLD_SCALE, 0);
			continue;
		}

		// alternate between wall panel and a feature (window or door)
		if ((i%2 == 0 && !bMoulding) ||
			(i%2 == 1 && bMoulding))
		{
			point[1] = point[0] + direction*sectionLength;
			AddWallSection(BM_WALL, point[0], point[1], CEILING_HEIGHT * WORLD_SCALE, 0);
		}
		else
		{
			// figure out whether to make a door or a window.
			if (madefirstfeature) {
				//make a door IF:
				//	it's the turn to make a door and we have doors left
				//  to build OR out of windows to build.
				makedoor = ((door && doorcount < numDoors) ||
					windowcount > numWindows);
			}
			else
			{
				makedoor = (pWall->m_iDoors > numWindows);
				madefirstfeature = true;
			}

			if (makedoor)
			{
				point[1] = point[0] + direction*DOOR_WIDTH*WORLD_SCALE;
				AddDoorSection(point[0], point[1], CEILING_HEIGHT * WORLD_SCALE);
				doorcount++;
				//made a door.  door should not be next
				door = false;
			} else {
				point[1] = point[0] + direction*WINDOW_WIDTH*WORLD_SCALE;
				AddWindowSection(point[0], point[1], CEILING_HEIGHT * WORLD_SCALE);
				windowcount++;
				//made a window.  door should be next
				door = true;
			}
		}
		point[0] = point[1];
	}
}

/**
 * Builds a wall, given material index, starting and end points, height, and
 * starting height.
 */
void vtBuilding3d::AddWallSection(BuildingMesh bm, FPoint3 &p0, FPoint3 &p1, float height, float base_height)
{
	// determine 2 points at top of door
	FPoint3 p2 = p1, p3 = p0;
	p2.y += height;
	p3.y += height;

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0,p1,p2);

	//make the texture map look right.  adjust it so it matches other walls.
	base_height = (CEILING_HEIGHT*WORLD_SCALE) - (base_height + height);
	float f1 = base_height/(CEILING_HEIGHT * WORLD_SCALE);
	float f2 = (base_height + height)/(CEILING_HEIGHT * WORLD_SCALE);

	int start =
	m_pMesh[bm]->AddVertexNUV(p0, norm, FPoint2(1.0f, f2));
	m_pMesh[bm]->AddVertexNUV(p1, norm, FPoint2(0.0f, f2));
	m_pMesh[bm]->AddVertexNUV(p2, norm, FPoint2(0.0f, f1));
	m_pMesh[bm]->AddVertexNUV(p3, norm, FPoint2(1.0f, f1));

	m_pMesh[bm]->AddFan(start, start+1, start+2, start+3);
}

/**
 * Builds a door section.  will also build the wall above the door to ceiling
 * height.
 */
void vtBuilding3d::AddDoorSection(FPoint3 &p0, FPoint3 &p1, float height)
{
	// determine 2 points at top of wall
	FPoint3 p2 = p1, p3 = p0;
	p2.y += DOOR_HEIGHT * WORLD_SCALE;
	p3.y += DOOR_HEIGHT * WORLD_SCALE;

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0,p1,p2);

	int start =
	m_pMesh[BM_DOOR]->AddVertexNUV(p0, norm, FPoint2(1.0f, 1.0f));
	m_pMesh[BM_DOOR]->AddVertexNUV(p1, norm, FPoint2(0.0f, 1.0f));
	m_pMesh[BM_DOOR]->AddVertexNUV(p2, norm, FPoint2(0.0f, 0.0f));
	m_pMesh[BM_DOOR]->AddVertexNUV(p3, norm, FPoint2(1.0f, 0.0f));

	m_pMesh[BM_DOOR]->AddFan(start, start+1, start+2, start+3);

	//add wall above door
	AddWallSection(BM_WALL, p3, p2, (CEILING_HEIGHT - DOOR_HEIGHT)*WORLD_SCALE, DOOR_HEIGHT);
}

//builds a window section.  builds the wall below and above a window too.
void vtBuilding3d::AddWindowSection(FPoint3 p0, FPoint3 p1, float height)
{
	// build wall to base of window.
	AddWallSection(BM_WALL, p0, p1, WINDOW_BOTTOM*WORLD_SCALE, 0);
	// determine 2 points at top of wall
	p0.y += WINDOW_BOTTOM*WORLD_SCALE;
	p1.y += WINDOW_BOTTOM*WORLD_SCALE;
	FPoint3 p2 = p1, p3 = p0;
	p2.y += WINDOW_HEIGHT*WORLD_SCALE;
	p3.y += WINDOW_HEIGHT*WORLD_SCALE;

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0,p1,p2);

	int start =
	m_pMesh[BM_WINDOW]->AddVertexNUV(p0, norm, FPoint2(1.0f, 1.0f));
	m_pMesh[BM_WINDOW]->AddVertexNUV(p1, norm, FPoint2(0.0f, 1.0f));
	m_pMesh[BM_WINDOW]->AddVertexNUV(p2, norm, FPoint2(0.0f, 0.0f));
	m_pMesh[BM_WINDOW]->AddVertexNUV(p3, norm, FPoint2(1.0f, 0.0f));

	m_pMesh[BM_WINDOW]->AddFan(start, start+1, start+2, start+3);

	// build wall above window
	AddWallSection(BM_WALL, p3, p2, (CEILING_HEIGHT - WINDOW_HEIGHT - WINDOW_BOTTOM)*WORLD_SCALE, (WINDOW_HEIGHT + WINDOW_BOTTOM)*WORLD_SCALE);
}


void vtBuilding3d::AddFlatRoof(Array<FPoint3> &pp, float height)
{
	FPoint3 up(0.0f, 1.0f, 0.0f);	// vector pointing up
	int corners = pp.GetSize();
	int i, j;

	if (m_bPolyRoof)
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

		for (i=0; i<tcount; i++)
		{
			for (j = 0; j < 3; j++)
			{
				FPoint2 gp = result[i*3+j];
				FPoint3 p(gp.x, roof_y, gp.y);
				ind[j] = m_pMesh[BM_ROOF]->AddVertexN(p, up);
			}
			m_pMesh[BM_ROOF]->AddTri(ind[0], ind[1], ind[2]);
		}
		return;
	}
	if (m_BldShape == SHAPE_RECTANGLE || m_BldShape == SHAPE_POLY)
	{
		int idx[MAX_WALLS];
		for (i = 0; i < corners; i++)
		{
			FPoint3 p = pp[i];
			p.y += height;
			idx[i] = m_pMesh[BM_ROOF]->AddVertexN(p, up);
		}
		m_pMesh[BM_ROOF]->AddFan(idx, corners);
	}
	if (m_BldShape == SHAPE_CIRCLE)
	{
	}
}

void vtBuilding3d::AddShedRoof(Array<FPoint3> &pp, float height)
{
	if (m_BldShape != SHAPE_RECTANGLE)
		return;

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
	idx[0] = m_pMesh[BM_ROOF]->AddVertexN(p[4], norm);
	idx[1] = m_pMesh[BM_ROOF]->AddVertexN(p[5], norm);
	idx[2] = m_pMesh[BM_ROOF]->AddVertexN(p[2], norm);
	idx[3] = m_pMesh[BM_ROOF]->AddVertexN(p[3], norm);
	m_pMesh[BM_ROOF]->AddFan(idx, 4);

	// Add front
	norm = Normal(p[0], p[1], p[5]);
	idx[0] = m_pMesh[BM_WALL]->AddVertexNUV(p[0], norm, FPoint2(0.0f, 0.0f));
	idx[1] = m_pMesh[BM_WALL]->AddVertexNUV(p[1], norm, FPoint2(1.0f, 0.0f));
	idx[2] = m_pMesh[BM_WALL]->AddVertexNUV(p[5], norm, FPoint2(1.0f, 1.0f));
	idx[3] = m_pMesh[BM_WALL]->AddVertexNUV(p[4], norm, FPoint2(0.0f, 1.0f));
	m_pMesh[BM_WALL]->AddFan(idx, 4);

	// Add left
	norm = Normal(p[3], p[0], p[4]);
	idx[0] = m_pMesh[BM_WALL]->AddVertexNUV(p[3], norm, FPoint2(0.0f, 0.0f));
	idx[1] = m_pMesh[BM_WALL]->AddVertexNUV(p[0], norm, FPoint2(1.0f, 0.0f));
	idx[2] = m_pMesh[BM_WALL]->AddVertexNUV(p[4], norm, FPoint2(1.0f, 1.0f));
	m_pMesh[BM_WALL]->AddFan(idx, 3);

	// Add right
	norm = Normal(p[1], p[2], p[5]);
	idx[0] = m_pMesh[BM_WALL]->AddVertexNUV(p[1], norm, FPoint2(0.0f, 0.0f));
	idx[1] = m_pMesh[BM_WALL]->AddVertexNUV(p[2], norm, FPoint2(1.0f, 0.0f));
	idx[2] = m_pMesh[BM_WALL]->AddVertexNUV(p[5], norm, FPoint2(0.0f, 1.0f));
	m_pMesh[BM_WALL]->AddFan(idx, 3);
}

void vtBuilding3d::AddHipRoof(Array<FPoint3> &pp, float height)
{
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

	switch (m_BldShape) {
	case SHAPE_RECTANGLE:
		{
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
			adjust = .6f*WORLD_SCALE;
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
			break;
		}
	}
}

void vtBuilding3d::AddGableRoof(Array<FPoint3> &pp, float height)
{
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

	switch (m_BldShape) {
	case SHAPE_RECTANGLE:
		{
			roofheight = m_fWidth*WORLD_SCALE/4;
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
			idx[0] = m_pMesh[BM_WALL]->AddVertexNUV(v[bottom_left], norm, FPoint2(0.0f, 0.0f));
			idx[1] = m_pMesh[BM_WALL]->AddVertexNUV(v[bottom_right], norm, FPoint2(1.0f, 0.0f));
			idx[2] = m_pMesh[BM_WALL]->AddVertexNUV(v[bottom_middle], norm, FPoint2(0.0f, 1.0f));
			m_pMesh[BM_WALL]->AddFan(idx, 3);

			norm = Normal(v[top_right],v[top_left],v[top_middle]);
			idx[0] = m_pMesh[BM_WALL]->AddVertexNUV(v[top_right], norm, FPoint2(0.0f, 0.0f));
			idx[1] = m_pMesh[BM_WALL]->AddVertexNUV(v[top_left], norm, FPoint2(1.0f, 0.0f));
			idx[2] = m_pMesh[BM_WALL]->AddVertexNUV(v[top_middle], norm, FPoint2(0.0f, 1.0f));
			m_pMesh[BM_WALL]->AddFan(idx, 3);

#if 1
			//adjust points for roof overhang
			//add to right;
			adjust = .6f*WORLD_SCALE;
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
			adjust = .6f * roofheight/(m_fWidth/2);
			v[bottom_left].y -= adjust;
			v[top_left].y -= adjust;
			v[top_right].y -= adjust;
			v[bottom_right].y -= adjust;
#endif

			//construct the roof
			BuildRoofPanel(v, 4, top_middle, top_left, bottom_left, bottom_middle);
			BuildRoofPanel(v, 4, bottom_middle, bottom_right, top_right, top_middle);
			break;
		}
	}
}

//
// given points, constructs a panel for the roof
//
void vtBuilding3d::BuildRoofPanel(FPoint3 *v, int n, ...)
{
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
		idx[i] = m_pMesh[BM_ROOF]->AddVertexN(v[va_arg(ap, int)], norm);
	va_end(ap);

	m_pMesh[BM_ROOF]->SetVtxTexCoord(idx[0], FPoint2(0.0f, 1.0f));
	m_pMesh[BM_ROOF]->SetVtxTexCoord(idx[1], FPoint2(0.0f, 0.0f));
	m_pMesh[BM_ROOF]->SetVtxTexCoord(idx[2], FPoint2(1.0f, 0.0f));
	if (n > 3)
		m_pMesh[BM_ROOF]->SetVtxTexCoord(idx[3], FPoint2(1.0f, 1.0f));
	m_pMesh[BM_ROOF]->AddFan(idx, n);
}

FPoint3 vtBuilding3d::Normal(FPoint3 &p0, FPoint3 &p1, FPoint3 &p2)
{
	FPoint3 a = p0 - p1;
	FPoint3 b = p2 - p1;
	FPoint3 norm = b.Cross(a);
	norm.Normalize();
	return norm;
}

//randomize buildings characteristics
void vtBuilding3d::Randomize(int iStories, bool bRotation)
{
	float angle;

	if (m_BldShape == SHAPE_RECTANGLE)
	{
		if (bRotation)
		{
			GetRotation(angle);
			if (angle == -1.0f)	// unset rotation
			{
				int r = rand() % 4;
				switch (r) {
				case 0:		angle = 0; break;
				case 1:		angle = PID2f; break;
				case 2:		angle = PI2f; break;
				case 3:		angle = -PID2f; break;
				}
				angle += random(.2f);
				SetRotation(angle);  // messes up lighting?
			}
		}

		m_RoofType = (RoofType) (rand() % NUM_ROOFTYPES);
		if (m_RoofType != ROOF_FLAT && m_RoofType != ROOF_GABLE && m_RoofType != ROOF_HIP) {
			m_RoofType = ROOF_GABLE;
		}
	}
	SetStories(iStories);

	int s, w;
	//whether or not to have siding.
	WallType walltype = rand() %4 ? WALL_SIDING : WALL_FLAT;
	for (s = 0; s < iStories; s++)
	{
		for (w = 0; w < m_Story[s]->m_Wall.GetSize(); w++) {
			m_Story[s]->m_Wall[w]->Set(0, 3, walltype);
		}
	}

	if (m_Story[0]->m_Wall.GetSize() > 3)
	{
		// put a door on the front and back of the first floor
		if (rand()%2) {
			w = 0;
			s = 2;
		} else {
			w = 1;
			s = 3;
		}
		m_Story[0]->m_Wall[w]->m_iDoors = 1;
		m_Story[0]->m_Wall[s]->m_iDoors = 1;
		m_Story[0]->m_Wall[w]->m_iWindows -= 1;
		m_Story[0]->m_Wall[s]->m_iWindows -= 1;
	}

	if (rand()%3)
		m_bMoulding = true;
	else
		m_bMoulding = false;

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
// choose appropriate color index.
// takes the building material and tries to match the closest color.
//
int vtBuilding3d::FindMatIndex(BuildingMaterial bldApp, RGBi inputColor)
{
	if (s_Materials == NULL) {
		return -1;
	}

	if (bldApp == BAP_WINDOW) {
		//only one kinda of window.
		//caller knows to choose between the 2 windows
		return WINDOW_MAT_START;
	} else if (bldApp == BAP_DOOR) {
		//only one door.
		return DOOR_MAT_START;
	}

	int start = 0;
	int end = 0;
	//get the appropriate range in the index
	switch (bldApp) {
	case BAP_PLAIN:
		start = PLAIN_MAT_START;
		end = PLAIN_MAT_END + 1;
		break;
	case BAP_SIDING:
		start = SIDING_MAT_START;
		end = SIDING_MAT_END + 1;
		break;
	case BAP_ROOF:
		start = ROOF_MAT_START;
		end = ROOF_MAT_END + 1;
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

