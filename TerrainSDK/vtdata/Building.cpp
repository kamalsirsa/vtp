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

// Defaults
#define STORY_HEIGHT	3.0f
#define WINDOW_WIDTH	1.3f
#define WINDOW_BOTTOM	0.4f
#define WINDOW_TOP		0.8f
#define DOOR_WIDTH		1.0f
#define DOOR_TOP		0.7f

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
	m_Material = BMAT_PLAIN;
}

vtEdge::~vtEdge()
{
}

vtEdge::vtEdge(const vtEdge &lhs)
{
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

	int num_walls = iDoors + iWindows + 1;
	int num_feat = iDoors + iWindows + num_walls;

	m_Features.Empty();
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
			m_Features.Append(door);
		if (do_window)
			m_Features.Append(window);
		m_Features.Append(wall);
	}
	m_Material = material;
}

void vtEdge::AddFeature(int code, float width, float vf1, float vf2)
{
	m_Features.Append(vtEdgeFeature(code, width, vf1, vf2));
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
	DeleteWalls();
}

void vtLevel::DeleteWalls()
{
	for (int i = 0; i < m_Wall.GetSize(); i++)
		delete m_Wall.GetAt(i);
	m_Wall.SetSize(0);
}

vtLevel &vtLevel::operator=(const vtLevel &v)
{
	DeleteWalls();
	m_Wall.SetSize(v.m_Wall.GetSize());
	for (int i = 0; i < v.m_Wall.GetSize(); i++)
	{
		vtEdge *pnew = new vtEdge(*v.m_Wall[i]);
		m_Wall.SetAt(i, pnew);
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

void vtLevel::SetWallMaterial(BldMaterial bm)
{
	for (int i = 0; i < m_Wall.GetSize(); i++)
		m_Wall[i]->m_Material = bm;
}

void vtLevel::SetWalls(int n)
{
	DeleteWalls();
	for (int i = 0; i < n; i++)
	{
		vtEdge *pnew = new vtEdge;
		pnew->Set(0, 0, BMAT_PLAIN);
		m_Wall.Append(pnew);
	}
}

/////////////////////////////////////

vtBuilding::vtBuilding()
{
	m_RoofType = ROOF_FLAT;			// default roof
	m_bMoulding = false;
	m_bElevated = false;			// default placement

	// default colors
	m_Color.Set(255,0,0);			// red
	m_RoofColor.Set(160,255,160);	// lime green
	m_MouldingColor.Set(255,255,255);	// white
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

	m_Color = v.m_Color;
	m_RoofColor = v.m_RoofColor;
	m_MouldingColor = v.m_MouldingColor;

	m_EarthPos = v.m_EarthPos;

	int i;
	for (i = 0; i < v.m_Levels.GetSize(); i++)
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
	switch (which)
	{
	case BLD_BASIC: m_Color = col; break;
	case BLD_ROOF: m_RoofColor = col; break;
	case BLD_MOULDING: m_MouldingColor = col; break;
	}
}

RGBi vtBuilding::GetColor(BldColor which) const
{
	switch (which)
	{
	case BLD_BASIC: return m_Color; break;
	case BLD_ROOF: return m_RoofColor; break;
	case BLD_MOULDING: return m_MouldingColor; break;
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
};

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


