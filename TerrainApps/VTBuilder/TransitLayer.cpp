//
// TransitLayer.cpp
//
// This layer wraps a transit system; it is only useful to those people
// using the 'PRTSim' package.  (If you do, set SUPPORT_TRANSIT=1).
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "TransitLayer.h"
#include "ScaledView.h"
#include "Helper.h"

#if SUPPORT_TRANSIT
#include "../5-16-joe-mods-15/portable/file-parser/file-parser.h"
#include "../5-16-joe-mods-15/portable/sim-engine/sim-engine.h"

//////////////////////////////////////////////////////////////////////////

vtTransitLayer::vtTransitLayer() : vtLayer(LT_TRANSIT)
{
	m_strFilename = "Untitled.hyd";
}

vtTransitLayer::~vtTransitLayer()
{
}

bool vtTransitLayer::OnSave(bool progress_callback(int))
{
	char *errMsg = NULL;

	KludgeRefPosApply(false);
	bool success = g_sim.SaveGuideway(m_strFilename, errMsg);
	KludgeRefPosApply(true);

	return true;
}

bool vtTransitLayer::OnLoad()
{
	OpenProgressDialog("Loading Transit Layer");

	char *errMsg = NULL;
	bool success = g_sim.LoadGuideway(m_strFilename, errMsg);
	if (!success)
	{
		CloseProgressDialog();
		return false;
	}

	KludgeRefPosApply(true);

	m_proj.SetUTMZone(10);

	CloseProgressDialog();
	return success;
}

void vtTransitLayer::KludgeRefPosApply(bool bForward)
{
	int i;
	double xoff, yoff;
	CName name;

	bool success = g_sim.GetReferencePosition(name, xoff, yoff);
	if (!success)
		return;

	if (bForward)
	{
		for (i = 1; i <= g_sim.NumSegments(); i++)
		{
			CSegment *ps = &g_sim.NthSegment(i);

			ps->m_y1 = -ps->m_y1;
			ps->m_y2 = -ps->m_y2;
			ps->m_ym = -ps->m_ym;
			ps->m_yb = -ps->m_yb;

			ps->m_x1 += xoff;
			ps->m_y1 += yoff;
			ps->m_x2 += xoff;
			ps->m_y2 += yoff;
			ps->m_xm += xoff;
			ps->m_ym += yoff;
			ps->m_xb += xoff;
			ps->m_yb += yoff;
		}
	}
	else
	{
		for (i = 1; i <= g_sim.NumSegments(); i++)
		{
			CSegment *ps = &g_sim.NthSegment(i);

			ps->m_x1 -= xoff;
			ps->m_y1 -= yoff;
			ps->m_x2 -= xoff;
			ps->m_y2 -= yoff;
			ps->m_xm -= xoff;
			ps->m_ym -= yoff;
			ps->m_xb -= xoff;
			ps->m_yb -= yoff;

			ps->m_y1 = -ps->m_y1;
			ps->m_y2 = -ps->m_y2;
			ps->m_ym = -ps->m_ym;
			ps->m_yb = -ps->m_yb;
		}
	}
}

bool vtTransitLayer::TransformCoords(vtProjection &proj_new)
{
	// unimplemented
	return false;
}


void vtTransitLayer::DrawLayer(wxDC *pDC, vtScaledView *pView)
{
	wxPen TransitPen(wxColor(255,0,0), 2, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(TransitPen);

	for (int i = 1; i <= g_sim.NumSegments(); i++)
	{
		const CSegment *ps = &g_sim.NthSegment(i);
		drawSeg(pDC, pView, ps->m_x1, ps->m_y1, ps->m_x2, ps->m_y2);

		double switchpos = ps->m_fBranchPos / ps->m_fLength;

		double xswitch = ps->m_x1 * (1-switchpos) + ps->m_x2 * switchpos;
		double yswitch = ps->m_y1 * (1-switchpos) + ps->m_y2 * switchpos;

		if (ps->m_segMerge)
			drawSeg(pDC, pView, ps->m_xm, ps->m_ym, xswitch, yswitch);

		if (ps->m_segNextBranch)
			drawSeg(pDC, pView, xswitch, yswitch, ps->m_xb, ps->m_yb);
	}
}

void vtTransitLayer::drawSeg(wxDC *pDC, vtScaledView *pView, double x1, double y1, double x2, double y2)
{
	// convert coordinates, then draw
	pDC->DrawLine(pView->sx(x1), pView->sy(y1), pView->sx(x2), pView->sy(y2));
}


bool vtTransitLayer::GetExtent(DRECT &rect)
{
	int size = g_sim.NumSegments();
	if (size == 0)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (int i = 1; i <= size; i++)
	{
		const CSegment *ps = &g_sim.NthSegment(i);
		rect.GrowToContainPoint(DPoint2(ps->m_x1, ps->m_y1));
		rect.GrowToContainPoint(DPoint2(ps->m_x2, ps->m_y2));
	}
	return true;
}

bool vtTransitLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_TRANSIT)
		return;

	vtTransitLayer *pFrom = (vtTransitLayer *)pL;
}

void vtTransitLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtTransitLayer::Offset(const DPoint2 &p)
{
	for (int i = 1; i <= g_sim.NumSegments(); i++)
	{
		CSegment *ps = &g_sim.NthSegment(i);

		ps->m_x1 += p.x;
		ps->m_y1 += p.y;
		ps->m_x2 += p.x;
		ps->m_y2 += p.y;
		ps->m_xm += p.x;
		ps->m_ym += p.y;
		ps->m_xb += p.x;
		ps->m_yb += p.y;
	}
}

#ifdef _DEBUG
#include <stdarg.h>
int gDebugLevel = 1;

void DebugOut(int level, LPCTSTR pFormat, ...)
{
	if (level > gDebugLevel)
		return;

	va_list va;
	va_start(va, pFormat);

	char ach[255];
	_vsntprintf(ach, 255, pFormat, va);

	OutputDebugString(ach);
}
#endif

#endif	// SUPPORT_TRANSIT
