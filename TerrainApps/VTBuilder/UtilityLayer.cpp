//
// TowerLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Frame.h"
#include "UtilityLayer.h"
#include "ScaledView.h"

wxPen greenPen;
wxPen bluePen;
static bool g_bInitializedPens = false;

//////////////////////////////////////////////////////////////////////////

vtUtilityLayer::vtUtilityLayer() : vtLayer(LT_UTILITY)
{
	m_strFilename = "Untitled.utl";

	if (!g_bInitializedPens)
	{
		g_bInitializedPens = true;
		greenPen.SetColour(25,128,0);
		bluePen.SetColour(25,25,200);
	}
}

bool vtUtilityLayer::GetExtent(DRECT &rect)
{
	if (m_Poles.IsEmpty())
		return false;

	GetPoleExtents(rect);

	// expand by 2 meters
	rect.left -= 2.0f;
	rect.right += 2.0f;
	rect.bottom -= 2.0f;
	rect.top += 2.0f;

	return true;
}

void vtUtilityLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	int i, j;
	int npoles = m_Poles.GetSize();
	int nlines = m_Lines.GetSize();

	if (!npoles)
		return;

	pDC->SetPen(greenPen);
	pDC->SetBrush(*wxTRANSPARENT_BRUSH);
	bool bSel = false;

	m_size = pView->sdx(20);
	if (m_size > 5) m_size = 5;
	if (m_size < 1) m_size = 1;

	pDC->SetPen(bluePen);
	for (i = 0; i < npoles; i++)
	{
		// draw each Pole
		vtPole *pole = m_Poles.GetAt(i);

/*		if (pole->IsSelected())
		{
			if (!bSel)
			{
				pDC->SetPen(bluePen);
				bSel = true;
			}
		}
		else
		{
			if (bSel)
			{
				pDC->SetPen(greenPen);
				bSel = false;
			}
		} */
		DrawPole(pDC, pView, pole);
	}
	pDC->SetPen(greenPen);
	for (i = 0; i < nlines; i++)
	{
		vtLine *line = m_Lines.GetAt(i);
		for (j = 0; j < line->GetSize(); j++)
			pView->screen(line->GetAt(j), g_screenbuf[j]);

		pDC->DrawLines(j, g_screenbuf);
	}
}


void vtUtilityLayer::DrawPole(wxDC* pDC, vtScaledView *pView, vtPole *pole)
{
	wxPoint center;
	pView->screen(pole->m_p, center);

	pDC->DrawLine(center.x-m_size, center.y, center.x+m_size+1, center.y);
	pDC->DrawLine(center.x, center.y-m_size, center.x, center.y+m_size+1);
}

bool vtUtilityLayer::OnSave()
{
	wxString strExt = m_strFilename.AfterLast('.');

//	if (!strExt.CmpNoCase("utl"))
//		return WriteUTL(m_strFilename);
	return false;
}

bool vtUtilityLayer::OnLoad()
{
	wxString strExt = m_strFilename.AfterLast('.');

//	if (!strExt.CmpNoCase("utl"))
//		return ReadUTL(m_strFilename);
	return false;
}

void vtUtilityLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtUtilityLayer::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

bool vtUtilityLayer::ConvertProjection(vtProjection &proj)
{
	if (proj == m_proj)
		return true;

	// Create conversion object
	vtProjection Source;
	GetProjection(Source);
	OCT *trans = OGRCreateCoordinateTransformation(&Source, &proj);
	if (!trans)
		return false;		// inconvertible projections

/*	vtTower *tower;
	DPoint2 loc;
	int i;
	int count = GetSize();
	for (i = 0; i < count; i++)
	{
		tower = GetAt(i);
		loc = tower->GetLocation();
		trans->Transform(1, &loc.x, &loc.y);
		tower->SetLocation(loc);
	}
*/
	delete trans;

	m_proj = proj;

	return false;
}

bool vtUtilityLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_UTILITY)
		return false;

	vtUtilityLayer *pFrom = (vtUtilityLayer *)pL;

/*	int count = pFrom->GetSize();
	for (int i = 0; i < count; i++)
	{
		vtTower *tower = pFrom->GetAt(i);
		Append(tower);
	} */
	return true;
}

void vtUtilityLayer::Offset(DPoint2 p)
{
/*
	int npoints = GetSize();
	if (!npoints)
		return;

	DPoint2 temp;
	for (int i = 0; i < npoints; i++)
	{
		vtTower *bld = GetAt(i);
		bld->Offset(p);
	}
*/
}

//
// inverts Selected value of tower within error or utmCoord
//
/*bool vtUtilityLayer::SelectTower(DPoint2 utmCoord, float error, DRECT &bound)
{
	NodeEdit* tower = SelectNode(utmCoord, error, true, false);
	if (node) {
		bound.left = bound.right = node->m_p.x;
		bound.top = bound.bottom = node->m_p.y;
		return true;
	} else
		return false;
}

//returns appropriate node at utmCoord within error
//toggle:	toggles the select value
//selectVal: what to assign the select value.
// toggle has precendence over selectVal.
NodeEdit* vtUtilityLayer::SelectNode(DPoint2 point, float error, bool toggle, bool selectVal)
{
	NodeEdit* bestSoFar;
	float dist = (float)error;
	float result;
	bool found = false;

	//a backwards rectangle, to provide greater flexibility for finding the node
	DRECT target(point.x-error, point.y+error, point.x+error, point.y-error);
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (curNode->WithinExtent(target))
		{
			result = curNode->DistanceToPoint(point);
			if (result < dist) {
				bestSoFar = curNode;
				dist = result;
				found = true;
			}
		}
	}

	if (found)
	{
		if (toggle)
			bestSoFar->ToggleSelect();
		else
			bestSoFar->Select(selectVal);
		return bestSoFar;
	}
	else
		return NULL;
}*/


void vtUtilityLayer::DeselectAll()
{
}

void vtUtilityLayer::InvertSelection()
{
}


