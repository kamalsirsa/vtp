//
// ScaledView.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "ScaledView.h"
#include "Builder.h"
#include "vtdata/Triangulate.h"

// global useful buffer for forming lines
wxPoint g_screenbuf[SCREENBUF_SIZE];


///////////////////////////////////////////////////////////////////////

vtScaledView::vtScaledView(wxWindow *parent, wxWindowID id, const wxPoint& pos,
						 const wxSize& size, long style, const wxString& name) :
	wxScrolledWindow(parent, id, pos, size, style, name )
{
	m_limits.x = m_limits.y = -200;
	m_limits.width = m_limits.height = 100;
	m_fScale = 1.0f;
}

void vtScaledView::ZoomToRect(const DRECT &geo_rect, float margin)
{
	VTLOG(" ZoomToRect LRTB(%lg, %lg, %lg, %lg)\n", geo_rect.left, geo_rect.right, geo_rect.top, geo_rect.bottom);

	wxRect client;
	GetClientSize(&client.width, &client.height);

	// safety check: if the data is a single point, back out slightly
	DRECT rect = geo_rect;
	if (rect.Width() == 0)
		rect.Grow(0.25, 0);
	if (rect.Height() == 0)
		rect.Grow(0, 0.25);

	// scale is pixels/coordinate unit
	DPoint2 scale;
	scale.x = (float) client.GetWidth() / rect.Width();
	scale.y = (float) client.GetHeight() / rect.Height();
	m_fScale = (scale.x < scale.y ? scale.x : scale.y);		// min
	m_fScale *= (1.0f - margin);
	UpdateRanges();

	DPoint2 center;
	rect.GetCenter(center);
	ZoomToPoint(center);
}

void vtScaledView::ZoomOutToRect(const DRECT &geo_rect)
{
	// Get current earth extents of the view
	DPoint2 p1, p2;
	wxSize size = GetClientSize();
	wxPoint pixel_size(size.x, size.y);

	DRECT outer_rect = GetWorldRect();

	DPoint2 scale;
	scale.x = geo_rect.Width() / outer_rect.Width();
	scale.y = geo_rect.Height() / outer_rect.Height();
	float delta = (scale.x < scale.y ? scale.x : scale.y);	// min

	DPoint2 center1, center2, diff;
	geo_rect.GetCenter(center1);
	outer_rect.GetCenter(center2);
	diff = center2 - center1;
	diff /= delta;

	m_fScale *= delta;
	UpdateRanges();

	DPoint2 new_center = center1 + diff;
	ZoomToPoint(new_center);
}

void vtScaledView::ZoomToPoint(const DPoint2 &p)
{
	wxPoint offset;

	offset.x = sdx(p.x);
	offset.y = sdy(p.y);

	int w, h;
	GetClientSize(&w, &h);
	offset.x -= (w / 2);
	offset.y -= (h / 2);

//	Scroll(offset.x - m_limits.x, offset.y - m_limits.y);

	// this avoids the calls to ScrollWindow which cause undesireable
	//  extra redrawing
	m_xScrollPosition = offset.x - m_limits.x;
	m_yScrollPosition = offset.y - m_limits.y;
	SetScrollPos( wxHORIZONTAL, m_xScrollPosition, TRUE );
	SetScrollPos( wxVERTICAL, m_yScrollPosition, TRUE );
	Refresh();
}

wxRect vtScaledView::WorldToCanvas(const DRECT &r)
{
	wxRect sr;

	sr.x = sx(r.left);
	sr.y = sy(r.top);
	sr.width = sx(r.right) - sr.x;
	sr.height = sy(r.bottom) - sr.y;

	return sr;
}

wxRect vtScaledView::WorldToWindow(const DRECT &r)
{
	wxRect sr;
	int right, bottom;

	CalcScrolledPosition(sx(r.left), sy(r.top), &sr.x, &sr.y);
	CalcScrolledPosition(sx(r.right), sy(r.bottom), &right, &bottom);
	sr.width = right - sr.x;
	sr.height = bottom - sr.y;

	return sr;
}

wxRect vtScaledView::PointsToRect(const wxPoint &p1, const wxPoint &p2)
{
	wxRect rect;
	rect.x = p1.x;
	rect.y = p1.y;
	rect.width = p2.x - p1.x;
	rect.height = p2.y - p1.y;
	return rect;
}

DRECT vtScaledView::CanvasToWorld(const wxRect &r)
{
	DRECT rect;
	rect.left = ox(r.x);
	rect.top = oy(r.y);
	rect.right = ox(r.x + r.width);
	rect.bottom = oy(r.y + r.height);
	rect.Sort();
	return rect;
}

void vtScaledView::SetScale(double scale)
{
	// don't lose track of where we're looking
	// convert window pixel center to lat-lon
	wxPoint size;
	GetClientSize(&size.x, &size.y);
	wxPoint center1(size.x / 2, size.y / 2), center2;

	CalcUnscrolledPosition(center1.x, center1.y, &center2.x, &center2.y);
	FPoint2 midscreen_coordinate(ox(center2.x), oy(center2.y));

	m_fScale = scale;
	UpdateRanges();

	ZoomToPoint(midscreen_coordinate);
}

double vtScaledView::GetScale()
{
	return m_fScale;
}

DRECT vtScaledView::GetWorldRect()
{
	wxPoint size;
	wxRect rect;

	GetClientSize(&size.x, &size.y);
	CalcUnscrolledPosition(0, 0, &rect.x, &rect.y);
	rect.width = size.x;
	rect.height = size.y;

	return CanvasToWorld(rect);
}

void vtScaledView::UpdateRanges()
{
	int w, h;
	GetClientSize(&w, &h);

	DRECT extents = g_bld->GetExtents();

	m_limits.x = sdx(extents.left);
	m_limits.width = sdx(extents.right) - m_limits.x;
	m_limits.y = sdy(extents.top);
	m_limits.height = sdy(extents.bottom) - m_limits.y;

	m_limits.x -= (w/2);
	m_limits.y -= (h/2);

	m_limits.width += w;
	m_limits.height += h;

	int h_range = m_limits.GetWidth();
	int v_range = m_limits.GetHeight();
	SetScrollbars(1, 1, h_range, v_range, 0, 0, TRUE);
}

void vtScaledView::GetCanvasPosition(const wxMouseEvent &event, wxPoint &pos)
{
	wxPoint p = event.GetPosition();
	CalcUnscrolledPosition(p.x, p.y, &pos.x, &pos.y);
}

int vtScaledView::ProjectPolyline(wxDC *pDC, const DLine2 &dline, bool bClose)
{
	const int size = dline.GetSize();
	if (size < 2)
		return 0;

	int i;
	for (i = 0; i < size && i < SCREENBUF_SIZE-1; i++)
		screen(dline[i], g_screenbuf[i]);
	if (bClose)
	{
		screen(dline[0], g_screenbuf[i]);
		i++;
	}
	return i;
}

void vtScaledView::DrawLine(wxDC *pDC, const DPoint2 &p0, const DPoint2 &p1)
{
	screen(p0, g_screenbuf[0]);
	screen(p1, g_screenbuf[1]);
	pDC->DrawLines(2, g_screenbuf);
}

void vtScaledView::DrawPolyLine(wxDC *pDC, const DLine2 &dline, bool bClose)
{
	const int num = ProjectPolyline(pDC, dline, bClose);
	pDC->DrawLines(num, g_screenbuf);
}

void vtScaledView::DrawDoubleLine(wxDC *pDC, const DLine2 &line, const DLine2 &width)
{
	uint i, size = line.GetSize();
	if (size < 2)
		return;

	for (i = 0; i < size && i < SCREENBUF_SIZE-1; i++)
		screen(line[i] + width[i], g_screenbuf[i]);
	pDC->DrawLines(i, g_screenbuf);

	for (i = 0; i < size && i < SCREENBUF_SIZE-1; i++)
		screen(line[i] - width[i], g_screenbuf[i]);
	pDC->DrawLines(i, g_screenbuf);
}

void vtScaledView::DrawPolygon(wxDC *pDC, const DPolygon2 &poly, bool bFill)
{
	if (bFill)
	{
		// inefficient temporary array.  TODO: make this more efficient.
		DLine2 dline;
		poly.GetAsDLine2(dline);

		int i, size = dline.GetSize();

		for (i = 0; i < size && i < SCREENBUF_SIZE-1; i++)
			screen(dline.GetAt(i), g_screenbuf[i]);

		pDC->DrawPolygon(i, g_screenbuf);
	}
	else
	{
#if 0
		// just draw each ring
		for (uint ring = 0; ring < poly.size(); ring++)
			DrawPolyLine(pDC, poly[ring], true);
#else
		// Draw outer ring solid
		DrawPolyLine(pDC, poly[0], true);
		if (poly.size() > 0)
		{
			// Draw holes as dotted lines
			wxPen pen1(pDC->GetPen());	// save
			wxPen pen2(pen1);			// copy
			pen2.SetStyle(wxDOT);		// modify
			pDC->SetPen(pen2);			// use
			for (uint ring = 1; ring < poly.size(); ring++)
				DrawPolyLine(pDC, poly[ring], true);
			pDC->SetPen(pen1);			// restore
		}
#endif
	}
}

void vtScaledView::DrawOGRLinearRing(wxDC *pDC, const OGRLinearRing *line, bool bCircles)
{
	int i, size = line->getNumPoints();
	if (size < 2)
		return;

	OGRPoint op;
	for (i = 0; i < size && i < SCREENBUF_SIZE-1; i++)
	{
		line->getPoint(i, &op);
		screen(&op, g_screenbuf[i]);
	}
	// Close
	line->getPoint(0, &op);
	screen(&op, g_screenbuf[i]);

	pDC->DrawLines(i+1, g_screenbuf);

	if (bCircles)
	{
		for (int j = 0; j < i+1; j++)
			pDC->DrawCircle(g_screenbuf[j], 3);
	}
}

void vtScaledView::DrawOGRPolygon(wxDC *pDC, const OGRPolygon &poly, bool bFill,
								  bool bCircles)
{
	if (bFill)
	{
		// TODO
	}
	else
	{
		// just draw each ring
		DrawOGRLinearRing(pDC, poly.getExteriorRing(), bCircles);

		for (int ring = 0; ring < poly.getNumInteriorRings(); ring++)
			DrawOGRLinearRing(pDC, poly.getInteriorRing(ring), bCircles);
	}
}

void vtScaledView::DrawDPolygon2(wxDC *pDC, const DPolygon2 &poly, bool bFill,
								  bool bCircles)
{
	if (bFill)
	{
		// tessellate.  we could also draw these as solid triangles.
		DLine2 result;
		CallTriangle(poly, result);
		int tcount = result.GetSize()/3;
		for (int j = 0; j < tcount; j++)
		{
			const DPoint2 &p1 = result[j*3+0];
			const DPoint2 &p2 = result[j*3+1];
			const DPoint2 &p3 = result[j*3+2];
			DrawLine(pDC, p1, p2);
			DrawLine(pDC, p2, p3);
			DrawLine(pDC, p3, p1);
		}
	}
	else
	{
		// just draw each ring
		for (uint ring = 0; ring < poly.size(); ring++)
		{
			DrawPolyLine(pDC, poly[ring], true);
			if (bCircles)
			{
				int size = poly[ring].GetSize();
				for (int j = 0; j < size; j++)
					pDC->DrawCircle(g_screenbuf[j], 3);
			}
		}
	}
}

