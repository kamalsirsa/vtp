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

/////////////////////////////////////

void vtWall::Set(int iDoors, int iWindows, WallMaterial Type)
{
	m_iDoors = iDoors;
	m_iWindows = iWindows;
	m_Type = Type;
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
		vtWall *pnew = new vtWall;
		*pnew = *v.m_Wall[i];
		m_Wall.SetAt(i, pnew);
	}
	return *this;
}
void vtLevel::SetWalls(int n)
{
	DeleteWalls();
	for (int i = 0; i < n; i++)
	{
		vtWall *pnew = new vtWall;
		pnew->Set(0, 0, vtWall::UNKNOWN);
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

	// size / position
	SetRectangle(10.0f, 10.0f);
}

vtBuilding::~vtBuilding()
{
	DeleteStories();
	//m_Footprint.Empty(); don't need to do this, it happens automatically
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

	m_Footprint.SetSize(v.m_Footprint.GetSize());
	int i;
	for (i = 0; i < v.m_Footprint.GetSize(); i++)
		m_Footprint.SetAt(i, v.m_Footprint.GetAt(i));

	SetStories(v.GetStories());

	return *this;
}

void vtBuilding::SetRectangle(float fWidth, float fDepth, float fRotation)
{
	RectToPoly(fWidth, fDepth, fRotation);
}

bool vtBuilding::GetRectangle(float &fWidth, float &fDepth) const
{
	if (m_Footprint.GetSize() == 4)
	{
		fWidth = (float) m_Footprint.SegmentLength(0);
		fDepth = (float) m_Footprint.SegmentLength(1);
		return true;
	}
	return false;
}

void vtBuilding::SetRadius(float fRad)
{
	m_Footprint.Empty();
	int i;
	for (i = 0; i < 20; i++)
	{
		double angle = i * PI2d / 20;
		DPoint2 vec(cos(angle) * fRad, sin(angle) * fRad);
		m_Footprint.Append(m_EarthPos + vec);
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

void vtBuilding::SetFootprint(DLine2 &dl)
{
	int size_new = dl.GetSize();
	int size_old = m_Footprint.GetSize();

	m_Footprint = dl;

	if (size_new != size_old)
	{
		// must rebuild stories and walls
//		delete m_pStory;
//		m_pStory = new vtLevel[m_iStories];
		RebuildWalls();
	}
};

void vtBuilding::RebuildWalls()
{
	for (int s = 0; s < m_Levels.GetSize(); s++)
	{
		m_Levels[s]->SetWalls(m_Footprint.GetSize());
		for (int w = 0; w < m_Levels[s]->m_Wall.GetSize(); w++)
		{
			// Doors, Windows, WallType
			m_Levels[s]->m_Wall[w]->Set(0, 2, vtWall::SIDING);
		}
	}
}

void vtBuilding::SetCenterFromPoly()
{
	DPoint2 p;

	int size = m_Footprint.GetSize();
	for (int i = 0; i < size; i++)
	{
		p += m_Footprint.GetAt(i);
	}
	p *= (1.0f / size);
	SetLocation(p);
}

void vtBuilding::Offset(const DPoint2 &p)
{
	m_EarthPos += p;

	for (int j = 0; j < m_Footprint.GetSize(); j++)
		m_Footprint[j] += p;
}

//
// Get an extent rectangle around the building.
// It doesn't need to be exact.
//
bool vtBuilding::GetExtents(DRECT &rect) const
{
	int size = m_Footprint.GetSize();
	if (size == 0)
		return false;
	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	for (int j = 0; j < size; j++)
		rect.GrowToContainPoint(m_Footprint[j]);
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

	m_Footprint.Empty();
	m_Footprint.Append(m_EarthPos + corner[0]);
	m_Footprint.Append(m_EarthPos + corner[1]);
	m_Footprint.Append(m_EarthPos + corner[2]);
	m_Footprint.Append(m_EarthPos + corner[3]);
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

	int points = m_Footprint.GetSize();
	fprintf(fp, "\t\t\t<poly num=\"%d\" coords=\"", points);
	for (int i = 0; i < points; i++)
	{
		DPoint2 p = m_Footprint.GetAt(i);
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


