//
// Building.cpp
//
// Implements the vtBuilding class which represents a single built structure.
// This is can be a single building, or any single artificial structure
// such as a wall or fence.
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Building.h"
#include "LocalConversion.h"

// Defaults
#define STORY_HEIGHT	3.0f
#define WINDOW_WIDTH	1.3f
#define WINDOW_BOTTOM	0.4f
#define WINDOW_TOP		0.8f
#define DOOR_WIDTH		1.0f
#define DOOR_TOP		0.7f

vtLocalConversion vtBuilding::s_Conv;

/////////////////////////////////////

vtEdgeFeature::vtEdgeFeature()
{
	SetDefaults();
}

vtEdgeFeature::vtEdgeFeature(int code, float width, float vf1, float vf2)
{
	m_code = code;
	m_width = width;
	m_vf1 = vf1;
	m_vf2 = vf2;
}

void vtEdgeFeature::SetDefaults()
{
	m_code = WFC_WALL;
	m_width = -1.0f;
	m_vf1 = 0.0f;
	m_vf2 = 1.0f;
}


/////////////////////////////////////

vtEdge::vtEdge()
{
	m_Color.Set(255,0,0);		// default color: red
	m_Material = BMAT_PLAIN;	// default material: plain
	m_iSlope = 90;		// vertical
	m_fEaveLength = 0.0f;
}

vtEdge::~vtEdge()
{
}

vtEdge::vtEdge(const vtEdge &lhs)
{
	m_Color = lhs.m_Color;
	m_Material = lhs.m_Material;
	for (int i = 0; i < lhs.m_Features.GetSize(); i++)
		m_Features.Append(lhs.m_Features[i]);
}

void vtEdge::Set(int iDoors, int iWindows, BldMaterial material)
{
	vtEdgeFeature wall, window, door;

	window.m_code = WFC_WINDOW;
	window.m_width = WINDOW_WIDTH;
	window.m_vf1 = WINDOW_BOTTOM;
	window.m_vf2 = WINDOW_TOP;

	door.m_code = WFC_DOOR;
	door.m_width = DOOR_WIDTH;
	door.m_vf1 = 0.0f;
	door.m_vf2 = DOOR_TOP;

	int num_Edgess = iDoors + iWindows + 1;

	m_Features.Empty();
	m_Features.SetMaxSize((iDoors + iWindows) * 2 + 1);
	m_Features.Append(wall);

	bool do_door, do_window, flip = false;
	while (iDoors || iWindows)
	{
		do_door = do_window = false;
		if (iDoors && iWindows)
		{
			if (flip)
				do_door = true;
			else
				do_window = true;
		}
		else if (iDoors)
			do_door = true;
		else if (iWindows)
			do_window = true;

		if (do_door)
		{
			m_Features.Append(door);
			iDoors--;
		}
		if (do_window)
		{
			m_Features.Append(window);
			iWindows--;
		}
		m_Features.Append(wall);
	}
	m_Material = material;
}

void vtEdge::AddFeature(int code, float width, float vf1, float vf2)
{
	m_Features.Append(vtEdgeFeature(code, width, vf1, vf2));
}

int vtEdge::NumFeaturesOfCode(int code)
{
	int i, count = 0, size = m_Features.GetSize();
	for (i = 0; i < size; i++)
	{
		if (m_Features[i].m_code == code)
			count++;
	}
	return count;
}

float vtEdge::FixedFeaturesWidth()
{
	float width = 0.0f, fwidth;
	int size = m_Features.GetSize();
	for (int i = 0; i < size; i++)
	{
		fwidth = m_Features[i].m_width;
		if (fwidth > 0)
			width += fwidth;
	}
	return width;
}

float vtEdge::ProportionTotal()
{
	float width = 0.0f, fwidth;
	int size = m_Features.GetSize();
	for (int i = 0; i < size; i++)
	{
		fwidth = m_Features[i].m_width;
		if (fwidth < 0)
			width += fwidth;
	}
	return width;
}

/////////////////////////////////////

vtLevel::vtLevel()
{
	m_iStories = 0;
	m_fStoryHeight = STORY_HEIGHT;
}

vtLevel::~vtLevel()
{
	DeleteEdges();
}

void vtLevel::DeleteEdges()
{
	for (int i = 0; i < m_Edges.GetSize(); i++)
		delete m_Edges.GetAt(i);
	m_Edges.SetSize(0);
}

vtLevel &vtLevel::operator=(const vtLevel &v)
{
	DeleteEdges();
	m_Edges.SetSize(v.m_Edges.GetSize());
	for (int i = 0; i < v.m_Edges.GetSize(); i++)
	{
		vtEdge *pnew = new vtEdge(*v.m_Edges[i]);
		m_Edges.SetAt(i, pnew);
	}
	m_Footprint = v.m_Footprint;
	return *this;
}

void vtLevel::SetFootprint(const DLine2 &dl)
{
	int prev = m_Footprint.GetSize();
	int curr = dl.GetSize();

	m_Footprint = dl;

	if (curr != prev)
		SetWalls(curr);
}

void vtLevel::SetEdgeMaterial(BldMaterial bm)
{
	for (int i = 0; i < m_Edges.GetSize(); i++)
		m_Edges[i]->m_Material = bm;
}

void vtLevel::SetEdgeColor(RGBi color)
{
	for (int i = 0; i < m_Edges.GetSize(); i++)
		m_Edges[i]->m_Color = color;
}

float vtLevel::GetEdgeLength(int i)
{
	int edges = GetNumEdges();
	int j = i+1;
	if (j == edges)
		j = 0;
	return (float) (m_Footprint[j] - m_Footprint[i]).Length();
}

void vtLevel::SetWalls(int n)
{
	DeleteEdges();
	for (int i = 0; i < n; i++)
	{
		vtEdge *pnew = new vtEdge;
		pnew->Set(0, 0, BMAT_PLAIN);
		m_Edges.Append(pnew);
	}
}

bool vtLevel::HasSlopedEdges()
{
	for (int i = 0; i < m_Edges.GetSize(); i++)
	{
		if (m_Edges[i]->m_iSlope != 90)
			return true;
	}
	return false;
}

bool vtLevel::IsHorizontal()
{
	for (int i = 0; i < m_Edges.GetSize(); i++)
	{
		if (m_Edges[i]->m_iSlope == 0)
			return true;
	}
	return false;
}

void vtLevel::SetRoofType(RoofType rt, int iSlope)
{
	int i, edges = GetNumEdges();

	if (rt == ROOF_FLAT)
	{
		// all edges are horizontal
		for (i = 0; i < edges; i++)
			m_Edges[i]->m_iSlope = 0;
		m_fStoryHeight = 0.0f;
	}
	if (rt == ROOF_SHED)
	{
		// all edges are vertical
		for (i = 0; i < edges; i++)
			m_Edges[i]->m_iSlope = 90;
		// except for the first edge
		m_Edges[0]->m_iSlope = iSlope;

		DetermineHeightFromSlopes();
	}
	if (rt == ROOF_GABLE)
	{
		// Algorithm for guessing which edges makes up the gable roof:
		if (GetNumEdges() == 4)
		{
			// In the case of a rectangular footprint, assume that the
			// shorter edge has the gable
			if (GetEdgeLength(1) > GetEdgeLength(0))
			{
				m_Edges[0]->m_iSlope = 90;
				m_Edges[1]->m_iSlope = iSlope;
				m_Edges[2]->m_iSlope = 90;
				m_Edges[3]->m_iSlope = iSlope;
			}
			else
			{
				m_Edges[0]->m_iSlope = iSlope;
				m_Edges[1]->m_iSlope = 90;
				m_Edges[2]->m_iSlope = iSlope;
				m_Edges[3]->m_iSlope = 90;
			}
		}
		else
		{
			// Assume that only convex edges can be gables, and no more than
			// one edge in a row is a gable.  All other edges are hip.
			bool last_gable = false;
			for (i = 0; i < edges; i++)
			{
				if (IsEdgeConvex(i) && !last_gable)
				{
					m_Edges[i]->m_iSlope = 90;
					last_gable = true;
				}
				else
				{
					m_Edges[i]->m_iSlope = iSlope;
					last_gable = false;
				}
			}
		}
		DetermineHeightFromSlopes();
	}
	if (rt == ROOF_HIP)
	{
		for (i = 0; i < edges; i++)
			m_Edges[i]->m_iSlope = iSlope;

		DetermineHeightFromSlopes();
	}
}

void vtLevel::SetEaveLength(float fMeters)
{
	int i, edges = GetNumEdges();
	
	for (i = 0; i < edges; i++)
	{
		vtEdge *edge = m_Edges[i];
		float rise = m_fStoryHeight;
		// sin(slope) = rise/length
		// length = rise/sin(slope)
		float length = rise / sinf(edge->m_iSlope / 180.0f * PIf);
		edge->m_Features[0].m_vf1 = -(fMeters / length);
	}
}

bool vtLevel::IsEdgeConvex(int i)
{
	// get the 2 corner indices of this edge
	int edges = GetNumEdges();
	int c1 = i;
	int c2 = (i+1 == edges) ? 0 : i+1;

	return (IsCornerConvex(c1) && IsCornerConvex(c2));
}

bool vtLevel::IsCornerConvex(int i)
{
	// get the 2 adjacent corner indices
	int edges = GetNumEdges();
	int c1 = (i-1 < 0) ? edges-1 : i-1;
	int c2 = i;
	int c3 = (i+1 == edges) ? 0 : i+1;

	// get edge vectors
	DPoint2 v1 = m_Footprint[c2] - m_Footprint[c1];
	DPoint2 v2 = m_Footprint[c3] - m_Footprint[c2];

	// if dot product is positive, it's convex
	double xprod = v1.Cross(v2);
	return (xprod > 0);
}

/**
 * Returns true if this level consists of edges with identical,
 * evenly spaced windows.
 */
bool vtLevel::IsUniform()
{
	int i, edges = GetNumEdges();
	for (i = 0; i < edges; i++)
	{
		vtEdge *edge = m_Edges[i];
		int windows = edge->NumFeaturesOfCode(WFC_WINDOW);
		int doors = edge->NumFeaturesOfCode(WFC_DOOR);
		int walls = edge->NumFeaturesOfCode(WFC_WALL);
		if (doors > 0)
			return false;
		if (walls != (windows + 1))
			return false;
		if (edge->m_iSlope != 90)
			return false;
//		if (edge->m_Color != RGBi(255, 255, 255))
//			return false;
	}
	return true;
}

void vtLevel::DetermineLocalFootprint(float fHeight)
{
	DPoint2 p;
	FPoint3 lp;

	int i, edges = m_Footprint.GetSize();
	m_LocalFootprint.SetSize(edges);
	for (i = 0; i < edges; i++)
	{
		p = m_Footprint.GetAt(i);
		vtBuilding::s_Conv.ConvertFromEarth(p, lp.x, lp.z);
		lp.y = fHeight;
		m_LocalFootprint.SetAt(i, lp);
	}
}

void vtLevel::GetEdgePlane(int i, FPlane &plane)
{
	vtEdge *edge = m_Edges[i];
	int edges = m_Edges.GetSize();

	int islope = edge->m_iSlope;
	float slope = (islope / 180.0f * PIf);
	int next = (i+1 == edges) ? 0 : i+1;

	// get edge vector
	FPoint3 vec = m_LocalFootprint[next] - m_LocalFootprint[i];
	vec.Normalize();

	// get perpendicular (upward pointing) vector
	FPoint3 perp;
	perp.Set(0, 1, 0);

	// create rotation matrix to rotate it upward
	FMatrix4 mat;
	mat.Identity();
	mat.AxisAngle(vec, slope);

	// create normal
	FPoint3 norm;
	mat.TransformVector(perp, norm);

	plane.Set(m_LocalFootprint[i], norm);
}

//
// Look at the sloped edges to see if they meet at a particular point;
// if so, set that as the Level's story height.  Return true on success.
//
bool vtLevel::DetermineHeightFromSlopes()
{
	// In order to find a roof point, we need 3 adjacent edges whose
	// edges intersect.
	int i, edges = GetNumEdges();

	bool bFoundASolution = false;
	FPlane *planes = new FPlane[edges];
	float fMinHeight = 1E10;
	for (i = 0; i < edges; i++)
	{
		GetEdgePlane(i, planes[i]);
	}
	for (i = 0; i < edges; i++)
	{
		int i0 = i;
		int i1 = (i+1)%edges;
		int i2 = (i+2)%edges;
		vtEdge *edge0 = m_Edges[i0];
		vtEdge *edge1 = m_Edges[i1];
		vtEdge *edge2 = m_Edges[i2];
		if (edge0->m_iSlope == 90 &&
			edge1->m_iSlope == 90 &&
			edge2->m_iSlope == 90)
		{
			// skip this one; 3 vertical edges aren't useful
			continue;
		}
		FPoint3 point;
		bool valid = PlaneIntersection(planes[i0], planes[i1], planes[i2], point);
		if (valid)
		{
			// take this point as the height of the roof
			float fHeight = (point.y - m_LocalFootprint[0].y);
			if (fHeight < fMinHeight)
				fMinHeight = fHeight;
			bFoundASolution = true;
		}
	}
	if (bFoundASolution)
		m_fStoryHeight = fMinHeight;
	delete [] planes;
	return bFoundASolution;
}


/////////////////////////////////////

vtBuilding::vtBuilding()
{
	m_RoofType = ROOF_FLAT;			// default roof
	m_bMoulding = false;
	m_bElevated = false;			// default placement
}

vtBuilding::~vtBuilding()
{
	DeleteStories();
}

void vtBuilding::DeleteStories()
{
	for (int i = 0; i < m_Levels.GetSize(); i++)
		delete m_Levels.GetAt(i);
	m_Levels.SetSize(0);
}

vtBuilding &vtBuilding::operator=(const vtBuilding &v)
{
	m_RoofType = v.m_RoofType;
	m_bMoulding = v.m_bMoulding;
	m_bElevated = v.m_bElevated;
	m_EarthPos = v.m_EarthPos;

	DeleteStories();

	for (int i = 0; i < v.m_Levels.GetSize(); i++)
		m_Levels.Append(new vtLevel(* v.m_Levels.GetAt(i)));

	return *this;
}

void vtBuilding::SetRectangle(float fWidth, float fDepth, float fRotation)
{
	RectToPoly(fWidth, fDepth, fRotation);
}

bool vtBuilding::GetRectangle(float &fWidth, float &fDepth) const
{
	DLine2 &fp = m_Levels[0]->GetFootprint();
	if (fp.GetSize() == 4)
	{
		fWidth = (float) fp.SegmentLength(0);
		fDepth = (float) fp.SegmentLength(1);
		return true;
	}
	return false;
}

void vtBuilding::SetRadius(float fRad)
{
	DLine2 &fp = m_Levels[0]->GetFootprint();
	fp.Empty();
	int i;
	for (i = 0; i < 20; i++)
	{
		double angle = i * PI2d / 20;
		DPoint2 vec(cos(angle) * fRad, sin(angle) * fRad);
		fp.Append(m_EarthPos + vec);
	}
}

float vtBuilding::GetRadius() const
{
	DRECT rect;
	GetExtents(rect);
	return (float) rect.Width() / 2.0f;
}


//sets colors of the walls
void vtBuilding::SetColor(BldColor which, RGBi col)
{
	int i, levs = m_Levels.GetSize();
	for (i = 0; i < levs; i++)
	{
		vtLevel *pLev = m_Levels[i];
		int j, edges = pLev->GetNumEdges();
		for (j = 0; j < edges; j++)
		{
			vtEdge *edge = pLev->GetEdge(j);
			if (edge->m_iSlope < 90)
			{
				if (which == BLD_ROOF)
					edge->m_Color = col;
			}
			else
			{
				if (which == BLD_BASIC)
					edge->m_Color = col;
			}
		}
	}
}

RGBi vtBuilding::GetColor(BldColor which) const
{
	int i, levs = m_Levels.GetSize();
	for (i = 0; i < levs; i++)
	{
		vtLevel *pLev = m_Levels[i];
		int j, edges = pLev->GetNumEdges();
		for (j = 0; j < edges; j++)
		{
			vtEdge *edge = pLev->GetEdge(j);
			if (edge->m_iSlope < 90)
			{
				if (which == BLD_ROOF)
					return edge->m_Color;
			}
			else
			{
				if (which == BLD_BASIC)
					return edge->m_Color;
			}
		}
	}
	return RGBi(0,0,0);
}

void vtBuilding::SetStories(int iStories)
{
	int previous = GetStories();
	if (previous == iStories)
		return;

	if (previous == 0)
		m_Levels.Append(new vtLevel());

	int levels = m_Levels.GetSize();

	// increase if necessary
	if (iStories > previous)
	{
		// get top level
		vtLevel *pTopLev = m_Levels[levels-1];
		// added some stories
		pTopLev->m_iStories += (iStories - previous);
	}
	// decrease if necessary
	while (GetStories() > iStories)
	{
		// get top level
		vtLevel *pTopLev = m_Levels[levels-1];
		pTopLev->m_iStories--;
		if (pTopLev->m_iStories == 0)
		{
			delete pTopLev;
			m_Levels.SetSize(levels-1);
			levels--;
		}
	}
}

int vtBuilding::GetStories() const
{
	int stories = 0;
	for (int i = 0; i < m_Levels.GetSize(); i++)
		stories += m_Levels[i]->m_iStories;
	return stories;
}

void vtBuilding::SetFootprint(int i, const DLine2 &dl)
{
	m_Levels[i]->SetFootprint(dl);

	// this new footprint make have altered the center of the building
	SetCenterFromPoly();

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

void vtBuilding::SetRoofType(RoofType rt)
{
	m_RoofType = rt;

	// if there is a roof level, attempt to set its edge angles to match
	// the desired roof type
	if (GetNumLevels() < 2)
		return;

	vtLevel *below = GetLevel(GetNumLevels()-2);
	vtLevel *pLev = GetLevel(GetNumLevels()-1);

	// provide default slopes for the roof sections
	int iSlope = 0;
	if (rt == ROOF_SHED)
		iSlope = 4;
	if (rt == ROOF_GABLE || rt == ROOF_HIP)
		iSlope = 15;

	pLev->SetRoofType(rt, iSlope);

	// all horizontal edges of the roof should default to the same material
	// as the wall section below them
	int i, edges = pLev->GetNumEdges();
	for (i = 0; i < edges; i++)
	{
		vtEdge *edge0 = below->GetEdge(i);
		vtEdge *edge1 = pLev->GetEdge(i);
		if (edge1->m_iSlope == 90)
		{
			edge1->m_Material = edge0->m_Material;
			edge1->m_Color = edge0->m_Color;
		}
	}
}

void vtBuilding::SetCenterFromPoly()
{
	DRECT rect;
	GetExtents(rect);
	DPoint2 p;
	rect.GetCenter(p);
	SetLocation(p);
}

void vtBuilding::Offset(const DPoint2 &p)
{
	int i;

	m_EarthPos += p;

	for (i = 0; i < m_Levels.GetSize(); i++)
	{
		vtLevel *lev = m_Levels[i];
		lev->GetFootprint().Add(p);
	}
}

//
// Get an extent rectangle around the building.
// It doesn't need to be exact.
//
bool vtBuilding::GetExtents(DRECT &rect) const
{
	int i, j;
	int levs = m_Levels.GetSize();
	if (levs == 0)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (i = 0; i < levs; i++)
	{
		vtLevel *lev = m_Levels[i];
		for (j = 0; j < lev->GetFootprint().GetSize(); j++)
			rect.GrowToContainPoint(lev->GetFootprint()[j]);
	}
	return true;
}

vtLevel *vtBuilding::CreateLevel(const DLine2 &footprint)
{
	vtLevel *pLev = new vtLevel();
	pLev->SetFootprint(footprint);
	m_Levels.Append(pLev);

	// this new footprint make have altered the center of the building
	SetCenterFromPoly();

	// keep 2d and 3d in synch
	DetermineLocalFootprints();

	return pLev;
}

void vtBuilding::RectToPoly(float fWidth, float fDepth, float fRotation)
{
	DPoint2 corner[4];

	// if rotation is unset, default to none
	if (fRotation == -1.0f)
		fRotation = 0.0f;

	DPoint2 pt(fWidth / 2.0, fDepth / 2.0);
	corner[0].Set(-pt.x, -pt.y);
	corner[1].Set(pt.x, -pt.y);
	corner[2].Set(pt.x, pt.y);
	corner[3].Set(-pt.x, pt.y);
	corner[0].Rotate(fRotation);
	corner[1].Rotate(fRotation);
	corner[2].Rotate(fRotation);
	corner[3].Rotate(fRotation);

	DLine2 dl;
	dl.Append(m_EarthPos + corner[0]);
	dl.Append(m_EarthPos + corner[1]);
	dl.Append(m_EarthPos + corner[2]);
	dl.Append(m_EarthPos + corner[3]);

	vtLevel *lev = m_Levels[0];
	lev->SetFootprint(dl);
}


void vtBuilding::WriteXML(FILE *fp, bool bDegrees)
{
	const char *coord_format;
	if (bDegrees)
		coord_format = "%lg";
	else
		coord_format = "%.2lf";

	fprintf(fp, "\t<structure type=\"building\">\n");

	int stories = GetStories();
	if (stories != 0)	
		fprintf(fp, "\t\t<height stories=\"%d\" />\n", stories);

	RGBi color;
	color = GetColor(BLD_BASIC);
	fprintf(fp, "\t\t<walls color=\"%d %d %d\" />\n", color.r, color.g, color.b);

	if (m_bMoulding)
	{
		color = GetColor(BLD_MOULDING);
		fprintf(fp, "\t\t<trim color=\"%d %d %d\" />\n", color.r, color.g, color.b);
	}
	fprintf(fp, "\t\t<shapes>\n");

	vtLevel *lev = m_Levels[0];
	DLine2 &foot = lev->GetFootprint();
	int points = foot.GetSize();
	fprintf(fp, "\t\t\t<poly num=\"%d\" coords=\"", points);
	for (int i = 0; i < points; i++)
	{
		DPoint2 p = foot.GetAt(i);
		fprintf(fp, coord_format, p.x);
		fprintf(fp, " ");
		fprintf(fp, coord_format, p.y);
		if (i != points-1)
			fprintf(fp, " ");
	}
	fprintf(fp, "\" />\n");
	fprintf(fp, "\t\t</shapes>\n");

	fprintf(fp, "\t\t<roof type=\"");
	switch (m_RoofType)
	{
	case ROOF_FLAT: fprintf(fp, "flat"); break;
	case ROOF_SHED: fprintf(fp, "shed"); break;
	case ROOF_GABLE: fprintf(fp, "gable"); break;
	case ROOF_HIP: fprintf(fp, "hip"); break;
	}
	color = GetColor(BLD_ROOF);
	fprintf(fp, "\" color=\"%d %d %d\" />\n", color.r, color.g, color.b);

	fprintf(fp, "\t</structure>\n");
}

void vtBuilding::AddDefaultDetails()
{
	// add some default windows/doors
	vtLevel *lev;
	vtEdge *edge;
	int i, j;
	int levs = m_Levels.GetSize();
	for (i = 0; i < levs; i++)
	{
		lev = m_Levels[i];
		int edges = lev->GetNumEdges();
		for (j = 0; j < edges; j++)
		{
			edge = lev->GetEdge(j);
			int doors = 0;
			int windows = (int) (lev->GetEdgeLength(j) / 6.0f);
			edge->Set(doors, windows, BMAT_SIDING);
		}
	}

	// add a roof level
	vtLevel *ground = m_Levels[0];
	vtLevel *roof = CreateLevel(ground->GetFootprint());
	roof->m_iStories = 1;
	int edges = roof->GetNumEdges();
	for (j = 0; j < edges; j++)
	{
		edge = roof->GetEdge(j);
		edge->m_iSlope = 0;		// flat roof
	}
	DetermineLocalFootprints();
}

void vtBuilding::DetermineLocalFootprints()
{
	s_Conv.SetOrigin(m_EarthPos);

	int i;
	int levs = m_Levels.GetSize();
	float fHeight = 0.0f;

	for (i = 0; i < levs; i++)
	{
		vtLevel *lev = m_Levels[i];
		lev->DetermineLocalFootprint(fHeight);
		fHeight += (lev->m_iStories * lev->m_fStoryHeight);
	}
}

