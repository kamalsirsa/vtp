//
// Building.cpp
//
// Implements the vtBuilding class which represents a single built structure.
// This is can be a single building, or any single artificial structure
// such as a wall or fence.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Building.h"

/////////////////////////////////////

void vtWall::Set(int iDoors, int iWindows, WallType Type)
{
	m_iDoors = iDoors;
	m_iWindows = iWindows;
	m_Type = Type;
}

/////////////////////////////////////

vtStory::vtStory()
{
}

vtStory::~vtStory()
{
	DeleteWalls();
}

void vtStory::DeleteWalls()
{
	for (int i = 0; i < m_Wall.GetSize(); i++)
		delete m_Wall.GetAt(i);
	m_Wall.SetSize(0);
}

vtStory &vtStory::operator=(const vtStory &v)
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
void vtStory::SetWalls(int n)
{
	DeleteWalls();
	for (int i = 0; i < n; i++)
	{
		vtWall *pnew = new vtWall;
		pnew->Set(0, 0, WALL_FLAT);
		m_Wall.Append(pnew);
	}
}

/////////////////////////////////////

vtBuilding::vtBuilding()
{
	m_BldShape = SHAPE_RECTANGLE;	// default shape
	m_RoofType = ROOF_FLAT;			// default roof
	m_bMoulding = false;
	m_bElevated = false;			// default placement

	// default colors
	m_Color.Set(255,0,0);			// red
	m_RoofColor.Set(160,255,160);	// lime green
	m_MouldingColor.Set(255,255,255);	// white

	// size / position
	SetRectangle(10.0f, 10.0f);
	m_fRadius = 10.0f;
	m_fRotation = -1.0f;	// unknown rotation
}

vtBuilding::~vtBuilding()
{
	DeleteStories();
	//m_Footprint.Empty(); don't need to do this, it happens automatically
}

void vtBuilding::DeleteStories()
{
	for (int i = 0; i < m_Story.GetSize(); i++)
		delete m_Story.GetAt(i);
	m_Story.SetSize(0);
}

vtBuilding &vtBuilding::operator=(const vtBuilding &v)
{
	m_BldShape = v.m_BldShape;
	m_RoofType = v.m_RoofType;
	m_bMoulding = v.m_bMoulding;
	m_bElevated = v.m_bElevated;

	m_Color = v.m_Color;
	m_RoofColor = v.m_RoofColor;
	m_MouldingColor = v.m_MouldingColor;

	m_EarthPos = v.m_EarthPos;
	m_fRotation = v.m_fRotation;
	m_fWidth = v.m_fWidth;
	m_fDepth = v.m_fDepth;
	m_fRadius = v.m_fRadius;

	m_Footprint.SetSize(v.m_Footprint.GetSize());
	int i;
	for (i = 0; i < v.m_Footprint.GetSize(); i++)
		m_Footprint.SetAt(i, v.m_Footprint.GetAt(i));

	SetStories(v.GetStories());

	return *this;
}

void vtBuilding::SetLocation(double utm_x, double utm_y)
{
	m_EarthPos.x = utm_x;
	m_EarthPos.y = utm_y;
}

void vtBuilding::SetRectangle(float fWidth, float fDepth)
{
	m_fWidth = fWidth;
	m_fDepth = fDepth;
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
RGBi vtBuilding::GetColor(BldColor which)
{
	switch (which)
	{
	case BLD_BASIC: return m_Color; break;
	case BLD_ROOF: return m_RoofColor; break;
	case BLD_MOULDING: return m_MouldingColor; break;
	}
	return RGBi(0,0,0);
}

//sets rotation of the building.  (may break the lighting of the building!)
void vtBuilding::SetRotation(float fRadians)
{
	m_fRotation = fRadians;
}

void vtBuilding::SetStories(int iStories)
{
	int s;

	int previous = m_Story.GetSize();
	if (previous == iStories)
		return;

	m_Story.SetSize(iStories);
	if (iStories > previous)
	{
		// added some stories
		for (s = previous; s < iStories; s++)
			m_Story[s] = new vtStory();
	}
	if (previous)
	{
		for (s = previous; s < iStories; s++)
			*m_Story[s] = *m_Story[previous-1];
	}
	else
		RebuildWalls();
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
//		m_pStory = new vtStory[m_iStories];
		RebuildWalls();
	}
};

void vtBuilding::RebuildWalls()
{
	for (int s = 0; s < m_Story.GetSize(); s++)
	{
		switch (m_BldShape)
		{
		case SHAPE_RECTANGLE:
			m_Story[s]->SetWalls(4);
			break;
		case SHAPE_CIRCLE:
			m_Story[s]->SetWalls(1);
			break;
		case SHAPE_POLY:
			m_Story[s]->SetWalls(m_Footprint.GetSize());
			break;
		}
		for (int w = 0; w < m_Story[s]->m_Wall.GetSize(); w++)
		{
			// Doors, Windows, WallType, Awning
			m_Story[s]->m_Wall[w]->Set(0, 2, WALL_SIDING);
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

void vtBuilding::Offset(DPoint2 &p)
{
	m_EarthPos += p;

	if (m_BldShape == SHAPE_POLY)
	{
		for (int j = 0; j < m_Footprint.GetSize(); j++)
		{
			m_Footprint[j] += p;
		}
	}
}

//
// Get an extent rectangle around the building.
// It doesn't need to be exact.
//
bool vtBuilding::GetExtents(DRECT &rect)
{
	if (m_BldShape == SHAPE_RECTANGLE)
	{
		float greater = m_fWidth > m_fDepth ? m_fWidth : m_fDepth;
		rect.SetRect(m_EarthPos.x - greater, m_EarthPos.y + greater,
					 m_EarthPos.x + greater, m_EarthPos.y - greater);
		return true;
	}

	if (m_BldShape == SHAPE_CIRCLE)
	{
		rect.SetRect(m_EarthPos.x - m_fRadius, m_EarthPos.y + m_fRadius,
					 m_EarthPos.x + m_fRadius, m_EarthPos.y - m_fRadius);
		return true;
	}

	if (m_BldShape == SHAPE_POLY)
	{
		rect.SetRect(1E9, -1E9, -1E9, 1E9);

		for (int j = 0; j < m_Footprint.GetSize(); j++)
			rect.GrowToContainPoint(m_Footprint[j]);
		return true;
	}
	return false;
}


void vtBuilding::RectToPoly()
{
	if (m_BldShape != SHAPE_RECTANGLE)
		return;

	DPoint2 corner[4];

	// if rotation is unset, default to none
	float fRotation = m_fRotation == -1.0f ? 0.0f : m_fRotation;

	DPoint2 pt(m_fWidth / 2.0, m_fDepth / 2.0);
	corner[0].Set(-pt.x, -pt.y);
	corner[1].Set(pt.x, -pt.y);
	corner[2].Set(pt.x, pt.y);
	corner[3].Set(-pt.x, pt.y);
	corner[0].Rotate(fRotation);
	corner[1].Rotate(fRotation);
	corner[2].Rotate(fRotation);
	corner[3].Rotate(fRotation);
	m_Footprint.SetAt(0, m_EarthPos + corner[0]);
	m_Footprint.SetAt(1, m_EarthPos + corner[1]);
	m_Footprint.SetAt(2, m_EarthPos + corner[2]);
	m_Footprint.SetAt(3, m_EarthPos + corner[3]);
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

	if (GetShape() == SHAPE_RECTANGLE)
	{
		fprintf(fp, "\t\t\t<rect ref_point=\"");
		fprintf(fp, coord_format, m_EarthPos.x);
		fprintf(fp, " ");
		fprintf(fp, coord_format, m_EarthPos.y);
		fprintf(fp, "\" size=\"%g, %g\"", m_fWidth, m_fDepth);
		if (m_fRotation != -1.0f)
			fprintf(fp, " rot=\"%f\"", m_fRotation);
		fprintf(fp, " />\n");
	}

	if (GetShape() == SHAPE_CIRCLE)
	{
		fprintf(fp, "\t\t\t<circle ref_point=\"");
		fprintf(fp, coord_format, m_EarthPos.x);
		fprintf(fp, " ");
		fprintf(fp, coord_format, m_EarthPos.y);
		fprintf(fp, "\" radius=\"%f\" />\n", m_fRadius);
	}

	if (GetShape() == SHAPE_POLY)
	{
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
	}
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


