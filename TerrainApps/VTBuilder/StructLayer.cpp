//
// StructLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Frame.h"
#include "StructLayer.h"
#include "ScaledView.h"
#include "BuildingDlg.h"

#ifndef _MSC_VER	// for non-MSVC
#include <string.h>
#define stricmp strcasecmp
#endif

wxPen orangePen;
wxPen yellowPen;
static bool g_bInitializedPens = false;

//////////////////////////////////////////////////////////////////////////

vtStructureLayer::vtStructureLayer() : vtLayer(LT_STRUCTURE)
{
	m_strFilename = "Untitled.xml";

	if (!g_bInitializedPens)
	{
		g_bInitializedPens = true;
		orangePen.SetColour(255,128,0);
		yellowPen.SetColour(255,255,0);
	}
}

bool vtStructureLayer::GetExtent(DRECT &rect)
{
	if (IsEmpty())
		return false;

	GetExtents(rect);

	// expand by 10 meters
	rect.left -= 10.0f;
	rect.right += 10.0f;
	rect.bottom -= 10.0f;
	rect.top += 10.0f;

	return true;
}

#define BLENGTH 5

void vtStructureLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	int structs = GetSize();
	if (!structs)
		return;

	pDC->SetPen(orangePen);
	pDC->SetBrush(*wxTRANSPARENT_BRUSH);
	bool bSel = false;

	m_size = pView->sdx(20);
	if (m_size > 5) m_size = 5;
	if (m_size < 1) m_size = 1;

	for (int i = 0; i < structs; i++)
	{
		// draw each building
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
		{
			if (str->IsSelected())
			{
				if (!bSel)
				{
					pDC->SetPen(yellowPen);
					bSel = true;
				}
			}
			else
			{
				if (bSel)
				{
					pDC->SetPen(orangePen);
					bSel = false;
				}
			}
			DrawBuilding(pDC, pView, bld);
		}
		else
		{
			vtFence *fen = str->GetFence();
			if (fen)
			{
				// TODO
			}
		}
	}
}

#define MAX_SIDES	100
static wxPoint array[MAX_SIDES];

void vtStructureLayer::DrawBuilding(wxDC* pDC, vtScaledView *pView, vtBuilding *bld)
{
	DPoint2 corner[4];
	float fWidth, fDepth, fRotation;
	int j;

	wxPoint origin;
	pView->screen(bld->GetLocation(), origin);

	pDC->DrawLine(origin.x-m_size, origin.y, origin.x+m_size+1, origin.y);
	pDC->DrawLine(origin.x, origin.y-m_size, origin.x, origin.y+m_size+1);

	if (bld->GetShape() == SHAPE_POLY)
	{
		DLine2 &dl = bld->GetFootprint();
		int size = dl.GetSize();
		for (j = 0; j < size && j < MAX_SIDES-1; j++)
			pView->screen(dl.GetAt(j), array[j]);
		pView->screen(dl.GetAt(0), array[j++]);

		pDC->DrawLines(j, array);
	}
	if (bld->GetShape() == SHAPE_CIRCLE)
	{
		int size = pView->sdx(bld->GetRadius());
		pDC->DrawEllipse(origin.x-size, origin.y-size,
			size<<1, size<<1);
	}
	if (bld->GetShape() == SHAPE_RECTANGLE)
	{
		bld->GetRectangle(fWidth, fDepth);
		bld->GetRotation(fRotation);
		if (fRotation == -1.0f) fRotation = 0.0f;
		DPoint2 pt(fWidth / 2.0, fDepth / 2.0);
		corner[0].Set(-pt.x, pt.y);
		corner[1].Set(pt.x, pt.y);
		corner[2].Set(pt.x, -pt.y);
		corner[3].Set(-pt.x, -pt.y);
		corner[0].Rotate(fRotation);
		corner[1].Rotate(fRotation);
		corner[2].Rotate(fRotation);
		corner[3].Rotate(fRotation);
		array[0] = origin + pView->screen_delta(corner[0]);
		array[1] = origin + pView->screen_delta(corner[1]);
		array[2] = origin + pView->screen_delta(corner[2]);
		array[3] = origin + pView->screen_delta(corner[3]);
		array[4] = array[0];
		pDC->DrawLines(5, array);
	}
}

bool vtStructureLayer::OnSave()
{
	return WriteXML(m_strFilename);
}

bool vtStructureLayer::OnLoad()
{
	return ReadXML(m_strFilename);
}

void vtStructureLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtStructureLayer::SetProjection(vtProjection &proj)
{
	m_proj = proj;
}

bool vtStructureLayer::ConvertProjection(vtProjection &proj)
{
	if (proj == m_proj)
		return true;

	// Create conversion object
	vtProjection Source;
	GetProjection(Source);
	OCT *trans = OGRCreateCoordinateTransformation(&Source, &proj);
	if (!trans)
		return false;		// inconvertible projections

	DPoint2 loc;
	int i;
	int count = GetSize();
	for (i = 0; i < count; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
		{
			loc = bld->GetLocation();
			trans->Transform(1, &loc.x, &loc.y);
			bld->SetLocation(loc);
		}
		vtFence *fen = str->GetFence();
		if (fen)
		{
			// TODO
		}
	}

	// set the projection
	m_proj = proj;

	delete trans;
	return true;
}

bool vtStructureLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_STRUCTURE)
		return false;

	vtStructureLayer *pFrom = (vtStructureLayer *)pL;

	int count = pFrom->GetSize();
	for (int i = 0; i < count; i++)
	{
		vtStructure *str = pFrom->GetAt(i);
		Append(str);
	}
	return true;
}

void vtStructureLayer::Offset(const DPoint2 &p)
{
	int npoints = GetSize();
	if (!npoints)
		return;

	DPoint2 temp;
	for (int i = 0; i < npoints; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			bld->Offset(p);
		vtFence *fen = str->GetFence();
		if (fen)
		{
			// TODO
		}
	}
}

//
// Locate the closest building that is no more than epsilon units in either
// dimension away from point.
//
// Return NULL if there is no building.
//
vtStructure *vtStructureLayer::FindBuilding(DPoint2 &point, double epsilon)
{
	int count = GetSize();
	DPoint2 loc, diff;
	double dist2, closest = 1.0E10f, eps2 = epsilon * epsilon;

	vtStructure *best_bld = NULL;

	for (int i = 0; i < count; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		loc = bld->GetLocation();
		diff = loc - point;
		dist2 = diff.x*diff.x + diff.y*diff.y;
		if (dist2 < eps2)
		{
			if (dist2 < closest)
			{
				closest = dist2;
				best_bld = str;
			}
		}
	}
	return best_bld;
}

bool vtStructureLayer::EditBuildingProperties()
{
	int count = 0;
	vtBuilding *bld_selected;

	int size = GetSize();
	for (int i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld && str->IsSelected())
		{
			count++;
			bld_selected = bld;
		}
	}
	if (count != 1)
		return false;

	// for now, assume they will change something (pessimistic)
	SetModified(true);

	BuildingDlg dlg(NULL, -1, "Building Properties", wxDefaultPosition);
		dlg.Setup(bld_selected);
	return (dlg.ShowModal() == wxID_OK);
}

void vtStructureLayer::InvertSelection()
{
	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		str->Select(!str->IsSelected());
	}
}

//
// Helper: find the index of a field in a DBF file, given the name of the field.
// Returns -1 if not found.
//
int FindDBField(DBFHandle db, const char *field_name)
{
	int count = DBFGetFieldCount(db);
	for (int i = 0; i < count; i++)
	{
		int pnWidth, pnDecimals;
		char pszFieldName[80];
		DBFFieldType fieldtype = DBFGetFieldInfo(db, i,
			pszFieldName, &pnWidth, &pnDecimals );
		if (!stricmp(field_name, pszFieldName))
			return i;
	}
	return -1;
}

void vtStructureLayer::AddElementsFromSHP(const char *filename, vtProjection &proj)
{
	//Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(filename, "rb");
	if (hSHP == NULL)
		return;

	//  Get number of polys (m_iNumPolys) and type of data (nShapeType)
	int		nElem;
	int		nShapeType;
    double	adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

	//  Check Shape Type, Building outlines should be Poly data
	if (nShapeType == SHPT_POINT)
		AddElementsFromSHPPoints(hSHP, nElem);
	else if (nShapeType == SHPT_POLYGON)
		AddElementsFromSHPPolygons(filename, hSHP, nElem);

	m_proj = proj;	// Set projection

	SHPClose(hSHP);
}

void vtStructureLayer::AddElementsFromSHPPoints(SHPHandle hSHP, int nElem)
{
	// Initialize arrays
	SetMaxSize(nElem);

	// Read Points from SHP
	int i;
	SHPObject *psShape;
	for (i = 0; i < nElem; i++)
	{
		// Get the i-th Point in the SHP file
		psShape = SHPReadObject(hSHP, i);

		vtBuilding *new_bld = new vtBuilding();
		new_bld->SetShape(SHAPE_RECTANGLE);
		new_bld->SetLocation(DPoint2(psShape->padfX[0], psShape->padfY[0]));
		new_bld->SetStories(1);
		AddBuilding(new_bld);

		SHPDestroyObject(psShape);
	}
}

void vtStructureLayer::AddElementsFromSHPPolygons(const char *filename,
												 SHPHandle hSHP, int nElem)
{
	// Open DBF File & Get DBF Info:
	DBFHandle db = DBFOpen(filename, "rb");
	if (db == NULL)
		return;

	// Check for field with number of stories
	int stories_field = FindDBField(db, "Stories");

	// Initialize arrays
	SetMaxSize(nElem);

	// Read Polys from SHP
	int i;
	SHPObject *psShape;
	for (i = 0; i < nElem; i++)
	{
		// Get the i-th Poly in the SHP file
		psShape = SHPReadObject(hSHP, i);

		// Store each SHP Poly as Building Footprint

		// The SHP appears to repeat the first point as the last, so ignore
		// the last point.
		int num_points = psShape->nVertices-1;

		vtBuilding *new_bld = new vtBuilding();
		new_bld->SetShape(SHAPE_POLY);

		DLine2 foot;
		foot.SetSize(num_points);

		int j, k;
		for (j = 0; j < num_points; j++)
		{
			// The SHP polygons appear to be clockwise - but the vtBuilding
			//  convention is counter-clockwise, so reverse the order.
			k = num_points-1-j;

			foot.SetAt(j, DPoint2(psShape->padfX[k], psShape->padfY[k]));
		}
		new_bld->SetFootprint(foot);
		new_bld->SetCenterFromPoly();

		int num_stories = 1;
		if (stories_field != -1)
		{
			// attempt to get number of stories from the DBF
			num_stories = DBFReadIntegerAttribute(db, i, stories_field);
			if (num_stories < 1)
				num_stories = 1;
		}
		new_bld->SetStories(num_stories);

		AddBuilding(new_bld);

		SHPDestroyObject(psShape);
	}
	DBFClose(db);
}

