//
// BuilderView.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/DataPath.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/vtLog.h"

#include "Options.h"
#include "BuilderView.h"
#include "Builder.h"
#include "MenuEnum.h"
#include "Helper.h"
#include "VTBuilder_UI.h"
// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
// Dialogs
#include "vtui/DistanceDlg.h"
#include "vtui/Helper.h"	// for GuessZoneFromGeo

#include "cpl_error.h"
#include <float.h>

#define BOUNDADJUST 5

#include "App.h"
DECLARE_APP(BuilderApp)


////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(BuilderView, vtScaledView)
EVT_LEFT_DOWN(BuilderView::OnLeftDown)
EVT_LEFT_UP(BuilderView::OnLeftUp)
EVT_LEFT_DCLICK(BuilderView::OnLeftDoubleClick)
EVT_MIDDLE_DOWN(BuilderView::OnMiddleDown)
EVT_MIDDLE_UP(BuilderView::OnMiddleUp)
EVT_RIGHT_DOWN(BuilderView::OnRightDown)
EVT_RIGHT_UP(BuilderView::OnRightUp)
EVT_MOTION(BuilderView::OnMouseMove)
EVT_MOUSEWHEEL(BuilderView::OnMouseWheel)

EVT_KEY_DOWN(BuilderView::OnKeyDown)
EVT_CHAR(BuilderView::OnChar)
EVT_IDLE(BuilderView::OnIdle)
EVT_SIZE(BuilderView::OnSize)
EVT_ERASE_BACKGROUND(BuilderView::OnEraseBackground)

EVT_SCROLLWIN_THUMBTRACK(BuilderView::OnBeginScroll)
EVT_SCROLLWIN_THUMBRELEASE(BuilderView::OnEndScroll)

EVT_SCROLLWIN_LINEUP(BuilderView::OnOtherScrollEvents)
EVT_SCROLLWIN_LINEDOWN(BuilderView::OnOtherScrollEvents)
EVT_SCROLLWIN_PAGEUP(BuilderView::OnOtherScrollEvents)
EVT_SCROLLWIN_PAGEDOWN(BuilderView::OnOtherScrollEvents)
//EVT_SCROLLWIN_CHANGED(BuilderView::OnScrollChanged)
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////

BuilderView::BuilderView(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, const wxString& name) :
		vtScaledView(parent, id, pos, size, 0, name )
{
	VTLOG(" Constructing BuilderView\n");

	m_bSkipNextRefresh = false;
	m_bGotFirstIdle = false;

	m_bCrossSelect = false;
	m_bShowMap = true;
	m_bScaleBar = false;
	m_bShowUTMBounds = false;

	m_bMouseMoved = false;
	m_bPanning = false;
	m_bScrolling = false;
	m_bBoxing = false;
	m_iDragSide = 0;
	m_bMouseCaptured = false;
	m_bShowGridMarks = false;
	m_pMapToCurrent = NULL;
	m_pCurrentToMap = NULL;

	m_ui.m_bRubber = false;
	m_ui.mode = LB_None;
	m_ui.m_bLMouseButton = m_ui.m_bMMouseButton = m_ui.m_bRMouseButton = false;
	m_ui.m_pEditingRoad = NULL;
	m_ui.m_iEditingPoint = -1;
	m_ui.m_pCurBuilding = NULL;
	m_ui.m_pCurLinear = NULL;
	m_ui.m_bDistanceToolMode = false;

	m_distance_p1.Set(0,0);
	m_distance_p2.Set(0,0);

	// Cursors are a little messy, since support is not even across platforms
#if defined(__WXMSW__)
	m_pCursorPan = new wxCursor(_T("cursors/panhand.cur"), wxBITMAP_TYPE_CUR);
#else
	// the predefined "hand" cursor isn't quite correct, since it is a image
	// of a hand with a pointing finger, not a closed, grasping hand.
	m_pCursorPan = new wxCursor(wxCURSOR_HAND);
#endif

	// world map SHP file
	m_iEntities = 0;
	m_bAttemptedLoad = false;
}

BuilderView::~BuilderView()
{
	delete m_pCursorPan;
	if (m_pMapToCurrent)
		delete m_pMapToCurrent;
	if (m_pCurrentToMap)
		delete m_pCurrentToMap;
}

////////////////////////////////////////////////////////////
// Operations

void BuilderView::OnDraw(wxDC& dc)  // overridden to draw this view
{
	static bool bFirstDraw = true;
	if (bFirstDraw)
	{
		bFirstDraw = false;
		VTLOG("First View OnDraw\n");
	}

	// no point in drawing until the Idle events have made it to the splitter
	if (!m_bGotFirstIdle)
		return;

	if (g_bld->DrawDisabled())
		return;

	vtLayerPtr lp;
	int i, iLayers = g_bld->NumLayers();

	// Draw 'interrupted projection outline' for current projection
	vtProjection proj;
	g_bld->GetProjection(proj);
	if (proj.IsDymaxion())
	{
		DrawDymaxionOutline(&dc);
	}

	// Draw the world map SHP file of country outline polys in latlon
	if (m_bShowMap)
		DrawWorldMap(&dc);

	// Draw the solid layers first
	for (i = 0; i < iLayers; i++)
	{
		lp = g_bld->GetLayer(i);
		if (lp->GetType() != LT_IMAGE && lp->GetType() != LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(&dc, this);
	}
	// Then the poly/vector/point layers
	for (i = 0; i < iLayers; i++)
	{
		lp = g_bld->GetLayer(i);
		if (lp->GetType() == LT_IMAGE || lp->GetType() == LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(&dc, this);
	}
	vtLayer *curr = g_bld->GetActiveLayer();
	if (curr && (curr->GetType() == LT_ELEVATION || curr->GetType() == LT_IMAGE))
	{
		DRECT rect;
		curr->GetAreaExtent(rect);
		HighlightArea(&dc, rect);
	}

	if (m_bShowUTMBounds)
		DrawUTMBounds(&dc);

	DrawAreaTool(&dc, g_bld->GetAtArea());

	if (m_bShowGridMarks)
		DrawGridMarks(dc);	// erase

	DrawDistanceTool(&dc);

	if (m_bScaleBar && !m_bPanning && !m_bScrolling)
		DrawScaleBar(&dc);
}

void BuilderView::DrawDymaxionOutline(wxDC *pDC)
{
	DLine2Array polys;

	m_icosa.GetDymaxEdges(polys);

	for (unsigned int i = 0; i < polys.size(); i++)
	{
		DrawPolyLine(pDC, polys[i], true);
	}
}

void BuilderView::GetMouseLocation(DPoint2 &p)
{
	p.x = ox(m_ui.m_CurPoint.x);
	p.y = oy(m_ui.m_CurPoint.y);
}

void BuilderView::SetMode(LBMode m)
{
	if (m_ui.mode == LB_Dir && m != LB_Dir)
	{
		vtRoadLayer::SetShowDirection(false);
		Refresh();
	}

	m_ui.mode = m;
	SetCorrectCursor();

	switch (m_ui.mode)
	{
		case LB_Dir:
			vtRoadLayer::SetShowDirection(true);
			Refresh();
			break;
		case LB_Node:
			if (!vtRoadLayer::GetDrawNodes()) {
				vtRoadLayer::SetDrawNodes(true);
				Refresh();
			}
			break;
	}

	g_bld->OnSetMode(m);

	if (m_ui.mode != LB_LinkEdit)
	{
		if (m_ui.m_pEditingRoad)
		{
			m_ui.m_pEditingRoad->m_bDrawPoints = false;
			RefreshRoad(m_ui.m_pEditingRoad);
		}
		m_ui.m_pEditingRoad = NULL;
	}
}

void BuilderView::DrawUTMBounds(wxDC *pDC)
{
	wxPen orange(wxColor(255,128,0), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(orange);

	vtProjection proj;
	g_bld->GetProjection(proj);

	int width, height;
	GetClientSize(&width, &height);

	DPoint2 ll, utm;
	wxPoint sp, array[4000];
	int zone, j;

	if (proj.IsGeographic())
	{
		for (zone = 0; zone < 60; zone++)
		{
			ll.x = -180.0f + zone * 6.0;

			ll.y = -70.0;
			screen(ll, array[0]);
			ll.y = +70.0;
			screen(ll, array[1]);

			pDC->DrawLines(2, array);
		}
	}
	else
	{
		int zone_start = 0;
		int zone_end = 60;
		int zone;
		DPoint2 proj_point;

		vtProjection geo;
		CreateSimilarGeographicProjection(proj, geo);

		OCT *trans = CreateCoordTransform(&proj, &geo);

		// try to speed up a bit by avoiding zones off the screen
		object(wxPoint(0, height/2), proj_point);
		trans->Transform(1, &proj_point.x, &proj_point.y);
		zone = GuessZoneFromGeo(proj_point);
		if (zone-1 > zone_start) zone_start = zone-1;

		object(wxPoint(width, height/2), proj_point);
		trans->Transform(1, &proj_point.x, &proj_point.y);
		zone = GuessZoneFromGeo(proj_point);
		if (zone+1 < zone_end) zone_end = zone+1;

		delete trans;

		// Now convert the longitude lines (boundaries between the UTM zones)
		// to the current projection
		trans = CreateCoordTransform(&geo, &proj);

		for (zone = zone_start; zone < zone_end; zone++)
		{
			ll.x = -180.0f + zone * 6.0;
			j = 0;
			for (ll.y = -70.0; ll.y <= 70.0; ll.y += 0.1)
			{
				proj_point = ll;
				trans->Transform(1, &proj_point.x, &proj_point.y);
				screen(proj_point, sp);
				if (sp.y < -8000 || sp.y > 8000)
					continue;
				array[j++] = sp;
			}
			pDC->DrawLines(j, array);
		}
		delete trans;
	}
}

//////////////////////////////////////////////////////////
// Added capability to read & display world map, poly SHP file.

// Get data out of SHP into WMPolys
bool BuilderView::ImportWorldMap()
{
	SHPHandle	hSHP;
	int			nShapeType, nShapeCount;
	unsigned int i;
	int j, k;

	vtString fname = FindFileOnPaths(vtGetDataPath(), "WorldMap/gnv19.shp");
	if (fname == "")
		return false;

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	// Open SHP file
	hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return false;

	VTLOG(" Opened, reading worldmap.\n");
	SHPGetInfo(hSHP, &nShapeCount, &nShapeType, NULL, NULL);
	if (nShapeType != SHPT_POLYGON)
		return false;
	unsigned int iShapeCount = nShapeCount;

	// Copy SHP data into World Map Poly data
	WMPoly.reserve(iShapeCount * 11 / 10);

	int points, start, stop;

	for (i = 0; i < iShapeCount; i++)
	{
		DPoint2 p;

		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		if (psShape->nParts > 1)
			p.Set(0,0);

		for (j = 0; j < psShape->nParts; j++)
		{
			start = psShape->panPartStart[j];
			if (j < psShape->nParts - 1)
				stop = psShape->panPartStart[j+1];
			else
				stop = psShape->nVertices;
			points = stop - start;

			DLine2 current;
			current.SetMaxSize(points);

			for (k = start; k < stop; k++)
			{
				p.x = psShape->padfX[k];
				p.y = psShape->padfY[k];
				current.Append(p);
			}
			WMPoly.push_back(current);
		}
		SHPDestroyObject(psShape);
	}

	// Close SHP file
	SHPClose(hSHP);

	// Initialize the drawn World Map WMPolyDraw to original (latlon)
	m_iEntities = (unsigned int)WMPoly.size();
	WMPolyDraw.resize(m_iEntities);
	for (i = 0; i < m_iEntities; i++)
		WMPolyDraw[i] = WMPoly[i];

	return true;
}

void BuilderView::SetWMProj(const vtProjection &proj)
{
	unsigned int i, j;

	if (WMPoly.size() == 0)
		return;

	const char *proj_name = proj.GetProjectionNameShort();
	if (!strcmp(proj_name, "Geo") || !strcmp(proj_name, "Unknown"))
	{
		// the data is already in latlon so just use WMPoly
		for (i = 0; i < m_iEntities; i++)
			WMPolyDraw[i] = WMPoly[i];
		return;
	}

	// Otherwise, must convert from Geo to whatever project is desired
	vtProjection Source;
	CreateSimilarGeographicProjection(proj, Source);

#if VTDEBUG
	// Check projection text
	char *str1, *str2;
	Source.exportToWkt(&str1);
	proj.exportToWkt(&str2);
	VTLOG("World Map projection, converting:\n");
	VTLOG(" From: ");
	VTLOG(str1);
	VTLOG("\n   To: ");
	VTLOG(str2);
	OGRFree(str1);
	OGRFree(str2);

	// Check texts in PROJ4
	char *str3, *str4;
	Source.exportToProj4(&str3);
	VTLOG("\n From: ");
	VTLOG(str3);
	proj.exportToProj4(&str4);
	VTLOG("\n   To: ");
	VTLOG(str4);
	VTLOG("\n");
	OGRFree(str3);
	OGRFree(str4);
#endif

	// Create conversion object
	if (m_pMapToCurrent)
	{
		delete m_pMapToCurrent;
		m_pMapToCurrent = NULL;
	}
	if (m_pCurrentToMap)
	{
		delete m_pCurrentToMap;
		m_pCurrentToMap = NULL;
	}
	m_pMapToCurrent = CreateCoordTransform(&Source, &proj);
	m_pCurrentToMap = CreateCoordTransform(&proj, &Source);

	if (!m_pMapToCurrent)
	{
		m_bShowMap = false;
		return;
	}

	DPoint2 point;
	WMPolyExtents.resize(m_iEntities);
	for (i = 0; i < m_iEntities; i++)
	{
		// gather an extent bound for each original feature
		WMPolyExtents[i].SetRect(1E9, -1E9, -1E9, 1E9);
		WMPolyExtents[i].GrowToContainLine(WMPoly[i]);

		// and project into current CRS
		WMPolyDraw[i].Empty();
		for (j = 0; j < WMPoly[i].GetSize(); j++)
		{
			point = WMPoly[i].GetAt(j);

			int converted = m_pMapToCurrent->Transform(1, &point.x, &point.y);

			if (converted == 1)
				WMPolyDraw[i].Append(point);
		}
	}
}

void BuilderView::DrawWorldMap(wxDC *pDC)
{
	if (m_iEntities == 0 && !m_bAttemptedLoad)
	{
		m_bAttemptedLoad = true;
		if (ImportWorldMap())
		{
			vtProjection proj;
			g_bld->GetProjection(proj);
			SetWMProj(proj);
		}
		else
		{
			m_bShowMap = false;
			return;
		}
	}

	// Set pen options
	wxPen WMPen(wxColor(0,0,0), 1, wxSOLID);  //solid black pen
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(WMPen);

	// Don't draw polys that are outside the window bounds; convert the bounds
	//  from the current CRS to Geo, so that we can test
	bool bHaveBounds = false;
	DRECT bounds(1E9, -1E9, -1E9, 1E9);
	vtProjection &proj = g_bld->GetAtProjection();
	if (!proj.IsGeographic() && m_pCurrentToMap != NULL)
	{
		wxSize size = GetClientSize();
		wxPoint pix[4];
		CalcUnscrolledPosition(0, 0,			&pix[0].x, &pix[0].y);
		CalcUnscrolledPosition(size.x, 0,		&pix[1].x, &pix[1].y);
		CalcUnscrolledPosition(size.x, size.y,	&pix[2].x, &pix[2].y);
		CalcUnscrolledPosition(0, size.y,		&pix[3].x, &pix[3].y);

		for (int i = 0; i < 4; i++)
		{
			DPoint2 p;
			// convert canvas -> earth
			object(pix[i], p);
			// convert earth -> map geo
			if (m_pCurrentToMap->Transform(1, &p.x, &p.y) == 1)
			{
				bounds.GrowToContainPoint(p);
				bHaveBounds = true;
			}
		}
	}

#if 1
	// Draw each poly in WMPolyDraw
	for (unsigned int i = 0; i < m_iEntities; i++)
	{
		if (bHaveBounds)
		{
			if (!bounds.OverlapsRect(WMPolyExtents[i]))
				continue;
		}
		DrawPolyLine(pDC, WMPolyDraw[i], true);
	}
#else
	// Draw each poly in WMPolyDraw
	for (unsigned int i = 0; i < m_iEntities; i++)
	{
		if (bHaveBounds)
		{
			// reject entity if it doesn't overlap view extents
			if (!bounds.OverlapsRect(WMPolyExtents[i]))
				continue;

			// attempt to reject a few more entities
			DPoint2 p1(WMPolyExtents[i].left,WMPolyExtents[i].bottom),p2(WMPolyExtents[i].right,WMPolyExtents[i].top);

			float size_x_orig = p2.x-p1.x;
			float size_y_orig = p2.y-p1.y;

			if (m_pMapToCurrent->Transform(1, &p1.x, &p1.y) == 1)
				if (m_pMapToCurrent->Transform(1, &p2.x, &p2.y) == 1)
				{
					// reject entity if it is mirrored
					if (p1.x>=p2.x || p1.y>=p2.y) continue;

					float size_x_proj = p2.x-p1.x;
					float size_y_proj = p2.y-p1.y;

					static const float distortion=0.5f; // allow 50% distortion

					// reject entity if the distortion threshold is exceeded
					if (size_x_proj>0 && size_y_proj>0)
						if ( fabs( (size_x_orig/size_x_proj) / (size_y_orig/size_y_proj) - 1) > distortion )
							continue;
				}
		}

		DrawLine(pDC, WMPolyDraw[i], true);
	}
#endif
}

void BuilderView::DrawScaleBar(wxDC * p_DC)
{
	VTLOG1("DrawScaleBar\n");
#ifndef _SCALE

#define _SCALE
#define _SCALEBOTTOM	20	//merge to bottom
#define _SCALERIGHT		160 //merge to right side
#define _POSTEXT		20	//merge between text and scale
#define _BARHIGHT		3	//hight of side-delimitor of scale
#define _MARGIN			10	//margin

#endif //_SCALE

	int xx, yy, w,  h, ww,hh;
	int scaleLen = 180; //default scale lenght
	wxString str;

	// Set pen options
	wxPen WMPen(wxColor(0,0,0), 1, wxSOLID);  //solid black pen
	p_DC->SetLogicalFunction(wxCOPY);
	p_DC->SetPen(WMPen);

	GetClientSize(&w,&h);

	//right bottom corner
	CalcUnscrolledPosition(w, h, &ww, &hh);

	vtProjection &proj = g_bld->GetAtProjection();
	LinearUnits lu = proj.GetUnits();
	wxString unit_str;

	double val = scaleLen/GetScale();
	if (lu == LU_DEGREES)
	{
		str.Printf(_T("%.6lg"), val);
		unit_str = wxString("°", wxConvUTF8);
	}
	else if (lu == LU_FEET_INT || lu == LU_FEET_US)
	{
		if (val > 5280)
		{
			val /= 5280;
			str.Printf(_T("%.2lf "), val);
			unit_str = _("miles");
		}
		else
		{
			str.Printf(_T("%.2lf"), val);
			unit_str = _T("'");
		}
	}
	else if (lu == LU_METERS)
	{
		if (val > 1000)
		{
			val /= 1000;
			str.Printf(_T("%.2lf"), val);
			unit_str = _T("km");
		}
		else
		{
			str.Printf(_T("%.2lf"), val);
			unit_str = _T("m");
		}
	}
	else
		return;		// unknown units

	str += unit_str;

	// height and width of the string on right side
	wxFont font(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	p_DC->SetFont(font);
	wxCoord textsize_x, textsize_y;
	p_DC->GetTextExtent(str, &textsize_x, &textsize_y);

	int width = scaleLen + textsize_x + 2 * _MARGIN;
	int height = 30;

	// coord of the round corner rectangle (in unscrolled coordinates)
	xx = ww - width;
	yy = hh - height;

	// Scale Bar Area in scrolled coordinates
	m_ScaleBarArea = wxRect(w - width, h - height, width, height);
	p_DC->DrawRoundedRectangle(xx, yy, width, height, 5);

	p_DC->DrawText(_T("0"),xx + _MARGIN,yy + 5);
	p_DC->DrawText(str, xx + scaleLen, yy + 5);

	p_DC->DrawLine(xx + _MARGIN,
				   yy + 20,
				   xx + scaleLen + _MARGIN,
				   yy + 20);
	p_DC->DrawLine(xx + _MARGIN,
				   yy + 20 + _BARHIGHT,
				   xx + _MARGIN,
				   yy + 20 - _BARHIGHT);
	p_DC->DrawLine(xx + scaleLen + _MARGIN,
				   yy + 20 + _BARHIGHT,
				   xx + scaleLen + _MARGIN,
				   yy + 20 - _BARHIGHT);
}


//////////////////////////////////////////////////////////
// Pan handlers

void BuilderView::BeginPan()
{
	VTLOG1("BeginPan\n");

	m_bPanning = true;
	SetCursor(*m_pCursorPan);

	// hide scale bar while panning
	if (m_bScaleBar)
		RefreshRect(m_ScaleBarArea);
}

void BuilderView::EndPan()
{
	VTLOG1("EndPan\n");

	m_bPanning = false;
	SetCorrectCursor();

	// redraw scale bar when done panning
	if (m_bScaleBar)
		RefreshRect(m_ScaleBarArea);
}

void BuilderView::DoPan(wxPoint point)
{
	wxPoint diff;
	diff = point - m_DownClient;
	m_DownClient = point;

	// update picture to reflect the changes
	Scroll(m_xScrollPosition - diff.x,
		m_yScrollPosition - diff.y);
}


//////////////////////////////////////////////////////////
// Box handlers

void BuilderView::InvertRect(wxDC *pDC, const wxRect &r, bool bDashed)
{
	InvertRect(pDC, wxPoint(r.x, r.y),
		wxPoint(r.x + r.width, r.y + r.height), bDashed);
}

void BuilderView::InvertRect(wxDC *pDC, const wxPoint &one,
	const wxPoint &two, bool bDashed)
{
	wxPen pen(*wxBLACK_PEN);
	if (bDashed)
		pen.SetStyle(wxSHORT_DASH);
	pDC->SetPen(pen);
	pDC->SetLogicalFunction(wxINVERT);

	wxPoint points[5];
	points[0].x = one.x;
	points[0].y = one.y;
	points[1].x = one.x;
	points[1].y = two.y;
	points[2].x = two.x;
	points[2].y = two.y;
	points[3].x = two.x;
	points[3].y = one.y;
	points[4].x = one.x;
	points[4].y = one.y;

	pDC->DrawLines(5, points);
}

void BuilderView::BeginBox()
{
	m_bBoxing = true;
}

void BuilderView::EndBox(const wxMouseEvent& event)
{
	m_bBoxing = false;

	if (!m_bMouseMoved)
		return;

	wxClientDC dc(this);
	PrepareDC(dc);

	InvertRect(&dc, m_ui.m_DownPoint, m_ui.m_LastPoint);

	wxRect rect = PointsToRect(m_ui.m_DownPoint, m_ui.m_LastPoint);
	m_world_rect = CanvasToWorld(rect);
	switch (m_ui.mode)
	{
		case LB_Mag:
			if (event.AltDown())
				ZoomOutToRect(m_world_rect);
			else
				ZoomToRect(m_world_rect, 0.0f);
			break;
		case LB_Box:
			DrawAreaTool(&dc, g_bld->GetAtArea());
			g_bld->SetArea(m_world_rect);
			DrawAreaTool(&dc, g_bld->GetAtArea());
			break;
		case LB_Node:
		case LB_Link:
			{
			// select everything in the highlighted box.
				vtRoadLayer *pRL = g_bld->GetActiveRoadLayer();
				if (pRL->SelectArea(m_world_rect, (m_ui.mode == LB_Node),
							m_bCrossSelect))
				{
					rect = WorldToWindow(m_world_rect);
					IncreaseRect(rect, 5);
					if (m_bCrossSelect)
						Refresh();
					else
						Refresh(TRUE, &rect);
				}
				else
					DeselectAll();
			}
			break;
		case LB_Move:
			Refresh();
			break;
		case LB_FSelect:
			EndBoxFeatureSelect(event);
	}
}

void BuilderView::EndBoxFeatureSelect(const wxMouseEvent& event)
{
	VTLOG1("EndBoxFeatureSelect:");

	vtLayer *pL = g_bld->GetActiveLayer();
	if (!pL) return;

	wxString verb;
	SelectionType st;
	// operation may be select, add (shift), subtract (alt), toggle (ctrl)
	if (event.ShiftDown())
	{
		st = ST_ADD;
		verb = _("Added");
	}
	else if (event.AltDown())
	{
		st = ST_SUBTRACT;
		verb = _("Subtracted");
	}
	else if (event.ControlDown())
	{
		st = ST_TOGGLE;
		verb = _("Toggled");
	}
	else
	{
		st = ST_NORMAL;
		verb = _("Selected");
	}

	int changed=0, selected=0;
	if (pL->GetType() == LT_STRUCTURE)
	{
		VTLOG1(" Doing structure box select\n");
		vtStructureLayer *pSL = (vtStructureLayer *)pL;
		changed = pSL->DoBoxSelect(m_world_rect, st);
		selected = pSL->NumSelected();
	}
	if (pL->GetType() == LT_RAW)
	{
		VTLOG1(" Doing raw box select\n");
		vtRawLayer *pRL = (vtRawLayer *)pL;
		changed = pRL->GetFeatureSet()->DoBoxSelect(m_world_rect, st);
		selected = pRL->GetFeatureSet()->NumSelected();
	}
	wxString msg;
	if (changed == 1)
		msg.Printf(_("1 entity, %d total selected"), selected);
	else
		msg.Printf(_("%d entities, %d total selected"), changed, selected);
	verb += _T(" ");
	verb += msg;
	if (g_bld->m_pParentWindow)
		g_bld->m_pParentWindow->SetStatusText(verb);

	// Log it also
	VTLOG((const char *)verb.mb_str(wxConvUTF8));

	g_bld->OnSelectionChanged();
	Refresh(false);
}

void BuilderView::DoBox(wxPoint point)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	InvertRect(&dc, m_ui.m_DownPoint, m_ui.m_LastPoint);
	InvertRect(&dc, m_ui.m_DownPoint, point);
}

void BuilderView::DrawAreaTool(wxDC *pDC, const DRECT &area)
{
	if (area.IsEmpty())
		return;

	int d = 3;
	wxRect r = WorldToCanvas(area);

	// dashed-line rectangle
	InvertRect(pDC, r, true);

	// four small rectangles, for the handles at each corner
	InvertRect(pDC, wxPoint(r.x-d, r.y-d), wxPoint(r.x+d, r.y+d));
	InvertRect(pDC, wxPoint(r.x+r.width-d, r.y-d), wxPoint(r.x+r.width+d, r.y+d));
	InvertRect(pDC, wxPoint(r.x-d, r.y+r.height-d), wxPoint(r.x+d, r.y+r.height+d));
	InvertRect(pDC, wxPoint(r.x+r.width-d, r.y+r.height-d), wxPoint(r.x+r.width+d, r.y+r.height+d));
}


////////////////////////////////////////////////////////////

void BuilderView::OnBeginScroll(wxScrollWinEvent & event)
{
	if (!m_bScrolling)
	{
		VTLOG1("BeginScroll\n");
		m_bScrolling = true;
		// hide scale bar while scrolling
		if (m_bScaleBar)
			RefreshRect(m_ScaleBarArea);
	}
	event.Skip();
}

void BuilderView::OnEndScroll(wxScrollWinEvent & event)
{
	VTLOG1("EndScroll\n");
	m_bScrolling = false;

	// redraw scale bar when done scrolling
	if (m_bScaleBar)
		RefreshRect(m_ScaleBarArea);

	event.Skip();
}

void BuilderView::OnOtherScrollEvents(wxScrollWinEvent & event)
{
	VTLOG1("OnOtherScrollEvents\n");
	if (m_bScaleBar)
		RefreshRect(m_ScaleBarArea);
	event.Skip();
}


////////////////////////////////////////////////////////////

void BuilderView::DrawDistanceTool(wxDC *pDC)
{
	pDC->SetPen(wxPen(*wxBLACK_PEN));
	pDC->SetLogicalFunction(wxINVERT);

	if (m_ui.m_bDistanceToolMode)
	{
		// Path mode
		// draw the polyline
		DrawPolyLine(pDC, m_distance_path, false);

		// draw small crosshairs
		unsigned int i, len = m_distance_path.GetSize();
		for (i = 0; i < len; i++)
		{
			wxPoint p1 = g_screenbuf[i];
			pDC->DrawLine(p1.x-4, p1.y, p1.x+4+1, p1.y);
			pDC->DrawLine(p1.x, p1.y-4, p1.x, p1.y+4+1);
		}
	}
	else
	{
		// Line mode
		wxPoint p1, p2;
		screen(m_distance_p1, p1);
		screen(m_distance_p2, p2);

		// draw small crosshairs
		pDC->DrawLine(p1.x-4, p1.y, p1.x+4+1, p1.y);
		pDC->DrawLine(p1.x, p1.y-4, p1.x, p1.y+4+1);
		pDC->DrawLine(p2.x-4, p2.y, p2.x+4+1, p2.y);
		pDC->DrawLine(p2.x, p2.y-4, p2.x, p2.y+4+1);
		// and the line itself
		pDC->DrawLine(p1.x, p1.y, p2.x, p2.y);
	}
}

void BuilderView::DrawDistanceTool()
{
	wxClientDC dc(this);
	PrepareDC(dc);
	DrawDistanceTool(&dc);
}

void BuilderView::ClearDistanceTool()
{
	DrawDistanceTool();	// erase

	SetDistancePoints(DPoint2(0,0), DPoint2(0,0));
	SetDistancePath(DLine2());
//	Refresh();
}

void BuilderView::UpdateDistance()
{
	if (m_ui.m_bDistanceToolMode)
		g_bld->UpdateDistance(m_distance_path);
	else
		g_bld->UpdateDistance(m_distance_p1, m_distance_p2);
}


////////////////////////////////////////////////////////////
// Elevation

void BuilderView::CheckForTerrainSelect(const DPoint2 &loc)
{
	// perhaps the user clicked on a terrain
	bool bFound = false;
	DRECT rect;
	for (int l = 0; l < g_bld->NumLayers(); l++)
	{
		vtLayerPtr lp = g_bld->GetLayer(l);
		if (lp->GetType() != LT_ELEVATION) continue;
		vtElevLayer *t = (vtElevLayer *)lp;

		t->GetExtent(rect);
		if (rect.ContainsPoint(loc))
		{
			SetActiveLayer(t);
			bFound = true;
			break;
		}
	}
}

void BuilderView::DrawInvertedLine(const DPoint2 &ep1, const DPoint2 &ep2)
{
	wxClientDC dc(this);
	PrepareDC(dc);

	dc.SetPen(wxPen(*wxBLACK_PEN));
	dc.SetLogicalFunction(wxINVERT);

	wxPoint p1, p2;
	screen(ep1, p1);
	screen(ep2, p2);
	dc.DrawLine(p1.x, p1.y, p2.x, p2.y);
}

//
// The view needs to be notified of the new active layer to update
// the selection marks drawn around the active elevation layer.
//
void BuilderView::SetActiveLayer(vtLayerPtr lp)
{
	vtLayer *last = g_bld->GetActiveLayer();

	LayerType prev_type = last ? last->GetType() : LT_UNKNOWN;
	LayerType curr_type = lp ? lp->GetType() : LT_UNKNOWN;
	bool bNeedErase = (prev_type == LT_ELEVATION || prev_type == LT_IMAGE);
	bool bNeedRedraw = (curr_type == LT_ELEVATION || curr_type == LT_IMAGE);

	if (lp != last)
	{
		if (bNeedErase || bNeedRedraw)
		{
			// Erase previous highlight, change layer, then draw highlight
			wxClientDC DC(this), *pDC = &DC;
			PrepareDC(DC);
			DRECT rect;

			if (bNeedErase)
			{
				last->GetAreaExtent(rect);
				HighlightArea(pDC, rect);
			}

			g_bld->SetActiveLayer(lp, true);

			if (bNeedRedraw)
			{
				lp->GetAreaExtent(rect);
				HighlightArea(pDC, rect);
			}
		}
		else
			// Simply change the current layer
			g_bld->SetActiveLayer(lp, false);
	}
}

void BuilderView::HighlightArea(wxDC *pDC, const DRECT &rect)
{
	wxPen bgPen(wxColor(255,255,255), 3, wxSOLID);
	pDC->SetPen(bgPen);
	pDC->SetLogicalFunction(wxINVERT);

	wxRect sr = WorldToCanvas(rect);
	int sx = sr.width / 3;
	int sy = sr.height / 3;
	int left = sr.x, right = sr.x+sr.width,
	top = sr.y, bottom = sr.y+sr.height;
	int d=2,e=4;

	//
	pDC->DrawLine(left - e, top - d,
		left - e, top + sy);

	pDC->DrawLine(left - d, top - e,
		left + sx, top - e);

	//
	pDC->DrawLine(right - sx, top - e,
		right + e, top - e);

	pDC->DrawLine(right + e, top - d,
		right + e, top + sy);

	//
	pDC->DrawLine(right + e, bottom - sy,
		right + e, bottom + d);

	pDC->DrawLine(right - sx, bottom + e,
		right + e, bottom + e);

	//
	pDC->DrawLine(left - e, bottom - sy,
		left - e, bottom + d);

	pDC->DrawLine(left + sx, bottom + e,
		left - e, bottom + e);
}

////////////////////////////////////////////////////////////
// Cursor

void BuilderView::SetCorrectCursor()
{
	switch (m_ui.mode)
	{
	case LB_None:	// none
	case LB_Link:	// select/edit links
	case LB_Node:	// select/edit nodes
	case LB_Move:	// move selected nodes
		SetCursor(wxCURSOR_ARROW); break;
	case LB_Pan:	// pan the view
		SetCursor(*m_pCursorPan); break;
	case LB_Dist:	// measure distance
		SetCursor(wxCURSOR_CROSS); break;
	case LB_Mag:	// zoom into rectangle
		SetCursor(wxCURSOR_MAGNIFIER); break;
	case LB_TowerAdd: // add a tower to the location
		SetCursor(wxCURSOR_CROSS);break;
	case LB_TrimTIN:
		SetCursor(wxCURSOR_CROSS); break;
	case LB_Dir:		// show/change road direction
	case LB_LinkEdit:	// edit road points
	case LB_LinkExtend: //extend a road selection
	case LB_TSelect:
	case LB_Box:
	default:
		SetCursor(wxCURSOR_ARROW); break;
	}
}

///////////////////////////////////////////////////////////////////////
// Distance tool

void BuilderView::BeginDistance()
{
	wxClientDC dc(this);
	PrepareDC(dc);

	DrawDistanceTool(&dc);	// erase
	if (m_ui.m_bDistanceToolMode)
	{
		// Path mode - set initial segment
		int len = m_distance_path.GetSize();
		if (len == 0)
		{
			// begin new path
			m_distance_path.Append(m_ui.m_DownLocation);
		}
		// default: add point to the path
		m_distance_path.Append(m_ui.m_DownLocation);

		g_bld->UpdateDistance(m_distance_path);
	}
	else
	{
		// Line mode - set initial points
		m_distance_p1 = m_ui.m_DownLocation;
		m_distance_p2 = m_ui.m_DownLocation;
	}
	DrawDistanceTool(&dc);	// redraw
}

/////////////////////////////////////////////////////////////

void BuilderView::BeginArea()	// in canvas coordinates
{
	DRECT area = g_bld->GetAtArea();
	if (area.IsEmpty())
	{
		BeginBox();
		return;
	}

	// check to see if they've clicked near one of the sides of the area
	m_iDragSide = 0;
	int eps = 10;	// epsilon in pixels
	wxRect r = WorldToCanvas(area);

	int d0 = abs(m_ui.m_CurPoint.x - r.x);
	int d1 = abs(m_ui.m_CurPoint.x - (r.x+r.width));
	int d2 = abs(m_ui.m_CurPoint.y - r.y);
	int d3 = abs(m_ui.m_CurPoint.y - (r.y+r.height));

	if (d0 < eps) m_iDragSide |= 1;
	if (d1 < eps) m_iDragSide |= 2;
	if (d2 < eps) m_iDragSide |= 4;
	if (d3 < eps) m_iDragSide |= 8;

	if (!m_iDragSide)
	{
		// if they click inside the box, drag it
		if (r.Contains(m_ui.m_CurPoint.x, m_ui.m_CurPoint.y))
			m_iDragSide = 15;

		// if they didn't click near the box, start a new one
		else
			BeginBox();
	}
}

void BuilderView::DoArea(wxPoint delta)	// in canvas coordinates
{
	wxClientDC dc(this);
	PrepareDC(dc);

	DrawAreaTool(&dc, g_bld->GetAtArea());	// erase
	if (m_iDragSide & 1)
		g_bld->GetAtArea().left += odx(delta.x);
	if (m_iDragSide & 2)
		g_bld->GetAtArea().right += odx(delta.x);
	if (m_iDragSide & 4)
		g_bld->GetAtArea().top += ody(delta.y);
	if (m_iDragSide & 8)
		g_bld->GetAtArea().bottom += ody(delta.y);
	DrawAreaTool(&dc, g_bld->GetAtArea());	// redraw
}

void BuilderView::InvertAreaTool(const DRECT &rect)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	DrawAreaTool(&dc, rect);
}

void BuilderView::ShowGridMarks(const DRECT &area, int cols, int rows,
								int active_col, int active_row)
{
	wxClientDC dc(this);
	PrepareDC(dc);

	if (m_bShowGridMarks)
		DrawGridMarks(dc);	// erase

	if (cols < 1 || rows < 1)
		return;

	m_GridArea = area;
	m_iGridCols = cols;
	m_iGridRows = rows;
	m_iActiveCol = active_col;
	m_iActiveRow = active_row;
	m_bShowGridMarks = true;
	DrawGridMarks(dc);		// draw
}

void BuilderView::HideGridMarks()
{
	if (m_bShowGridMarks)
	{
		wxClientDC dc(this);
		PrepareDC(dc);
		DrawGridMarks(dc);	// erase
	}
	m_bShowGridMarks = false;
}

void BuilderView::DrawGridMarks(wxDC &dc)
{
	dc.SetPen(wxPen(*wxBLACK_PEN));
	dc.SetBrush(wxBrush(*wxBLACK_BRUSH));
	dc.SetLogicalFunction(wxINVERT);

	DPoint2 p;
	int *wx = new int[m_iGridCols+1];
	int *wy = new int[m_iGridRows+1];

	for (int x = 0; x <= m_iGridCols; x++)
		wx[x] = sx(m_GridArea.left + (m_GridArea.Width()/m_iGridCols) * x);
	for (int y = 0; y <= m_iGridRows; y++)
		wy[y] = sy(m_GridArea.bottom + (m_GridArea.Height()/m_iGridRows) * y);

	for (int x = 0; x <= m_iGridCols; x++)
		dc.DrawLine(wx[x], wy[0], wx[x], wy[m_iGridRows]);

	for (int y = 0; y <= m_iGridRows; y++)
		dc.DrawLine(wx[0], wy[y], wx[m_iGridCols], wy[y]);

	if (m_iActiveCol != -1)
	{
		dc.DrawRectangle(wx[m_iActiveCol], wy[m_iActiveRow+1],
			wx[1]-wx[0], wy[0]-wy[1]);
	}
	delete [] wx;
	delete [] wy;
}

void BuilderView::DeselectAll()
{
	vtRoadLayer *pRL = g_bld->GetActiveRoadLayer();
	if (pRL)
	{
		DRECT* world_bounds;
		wxRect bound;
		int n;

		// caller is responsible for deleting the array returned from DeselectAll
		world_bounds = pRL->DeSelectAll(n);
		if (n > 100)
		{
			// too many deleted for quick refresh
			Refresh(TRUE);
		}
		else
		{
			n = n -1;
			while (n >=0) {
				bound = WorldToWindow(world_bounds[n]);
				IncreaseRect(bound, BOUNDADJUST);
				Refresh(TRUE, &bound);
				n--;
			}
		}
		delete world_bounds;
	}
	vtStructureLayer *pSL = g_bld->GetActiveStructureLayer();
	if (pSL)
	{
		pSL->DeselectAll();
		Refresh(TRUE);
	}
	vtRawLayer *pRawL = g_bld->GetActiveRawLayer();
	if (pRawL)
	{
		pRawL->GetFeatureSet()->DeselectAll();
		Refresh(TRUE);
		g_bld->OnSelectionChanged();
	}
}

void BuilderView::DeleteSelected(vtRoadLayer *pRL)
{
	int nDeleted;

	// delete the items, which returns an array of extents,
	// one for each deleted item
	DRECT *world_bounds = pRL->DeleteSelected(nDeleted);

	if (pRL->RemoveUnusedNodes() != 0)
		pRL->ComputeExtents();

	if (nDeleted > 100)
	{
		// too many deleted for quick refresh
		Refresh(TRUE);
	}
	else if (nDeleted > 0  && world_bounds != NULL)
	{
		wxRect bound;
		for (int n = nDeleted - 1; n >= 0; n--)
		{
			bound = WorldToWindow(world_bounds[n]);
			IncreaseRect(bound, BOUNDADJUST);
			Refresh(TRUE, &bound);
		}
		pRL->SetModified(true);
	}
	delete world_bounds;
}

void BuilderView::MatchZoomToElev(vtElevLayer *pEL)
{
	if (!pEL || !pEL->GetGrid())
		return;

	DPoint2 spacing = pEL->GetGrid()->GetSpacing();
	SetScale(1.0f / spacing.x);

	DPoint2 center;
	DRECT area;
	pEL->GetExtent(area);
	area.GetCenter(center);
	ZoomToPoint(center);

	Refresh();
}
void BuilderView::MatchZoomToImage(vtImageLayer *pIL)
{
	if (!pIL)
		return;

	DPoint2 spacing = pIL->GetSpacing();
	SetScale(1.0f / spacing.x);

	DPoint2 center;
	DRECT area;
	pIL->GetExtent(area);
	area.GetCenter(center);
	ZoomToPoint(center);

	Refresh();
}

void BuilderView::SetShowMap(bool bShow)
{
	if (bShow)
		m_bAttemptedLoad = false;
	m_bShowMap = bShow;
}

void BuilderView::SetShowScaleBar(bool bShow)
{
	m_bScaleBar = bShow;
	Refresh();
}

void BuilderView::SetDistanceToolMode(bool bPath)
{
	m_ui.m_bDistanceToolMode = bPath;
}

bool BuilderView::GetDistanceToolMode()
{
	return m_ui.m_bDistanceToolMode;
}


/////////////////////////////////////////////////////////////
// Mouse handlers

void BuilderView::OnLeftDown(wxMouseEvent& event)
{
	m_ui.m_bLMouseButton = true;
	m_bMouseMoved = false;

	// save the point where the user clicked
	m_DownClient = event.GetPosition();
	GetCanvasPosition(event, m_ui.m_DownPoint);

	m_ui.m_CurPoint = m_ui.m_DownPoint;
	m_ui.m_LastPoint = m_ui.m_DownPoint;

	// "points" are in window pixels, "locations" are in the current CRS
	object(m_ui.m_DownPoint, m_ui.m_DownLocation);

	// Remember modifier key state
	m_ui.m_bShift = event.ShiftDown();
	m_ui.m_bControl = event.ControlDown();
	m_ui.m_bAlt = event.AltDown();

	// We must 'capture' the mouse in order to receive button-up events
	// in the case where the cursor leaves the window.
	if (!m_bMouseCaptured)
	{
		CaptureMouse();
		m_bMouseCaptured = true;
	}

	vtLayerPtr pL = g_bld->GetActiveLayer();
	switch (m_ui.mode)
	{
		case LB_TSelect:
			CheckForTerrainSelect(m_ui.m_DownLocation);
			break;

		case LB_Pan:
			BeginPan();
			break;

		case LB_Mag:
		case LB_Node:
		case LB_Link:
		case LB_FSelect:
			BeginBox();
			break;

		case LB_Box:
			BeginArea();
			break;

		case LB_Dist:
			BeginDistance();
			break;
	}
	// Dispatch for layer-specific handling
	if (pL)
		pL->OnLeftDown(this, m_ui);

	// Allow wxWindows to pass the event along.  This is important because
	//  otherwise (with wx>2.4) we may not receive keyboard focus.
	event.Skip();
}

void BuilderView::OnLeftUp(wxMouseEvent& event)
{
	if (m_bMouseCaptured)
	{
		ReleaseMouse();
		m_bMouseCaptured = false;
	}

	if (!m_bMouseMoved)
		OnLButtonClick(event);

	OnLButtonDragRelease(event);

	// Dispatch for layer-specific handling
	vtLayerPtr pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnLeftUp(this, m_ui);

	m_ui.m_bLMouseButton = false;	// left mouse button no longer down
}

void BuilderView::OnLeftDoubleClick(wxMouseEvent& event)
{
	GetCanvasPosition(event, m_ui.m_DownPoint);
	m_ui.m_CurPoint = m_ui.m_LastPoint = m_ui.m_DownPoint;
	object(m_ui.m_DownPoint, m_ui.m_DownLocation);

	vtLayer *pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnLeftDoubleClick(this, m_ui);
}

void BuilderView::OnLButtonClick(wxMouseEvent& event)
{
	vtLayerPtr pL = g_bld->GetActiveLayer();
	if (!pL) return;

	GetCanvasPosition(event, m_ui.m_DownPoint);
	m_ui.m_CurPoint = m_ui.m_LastPoint = m_ui.m_DownPoint;
	DPoint2 point(ox(m_ui.m_CurPoint.x), oy(m_ui.m_CurPoint.y));

	if (pL->GetType() == LT_ROAD)
	{
		switch (m_ui.mode)
		{
			case LB_Link:
			case LB_Node:
				OnLButtonClickElement((vtRoadLayer *)pL);
				break;
		}
	}
	if (m_ui.mode == LB_FSelect)
		OnLButtonClickFeature(pL);
}

void BuilderView::OnLButtonDragRelease(wxMouseEvent& event)
{
	if (m_bPanning)
		EndPan();
	if (m_bBoxing)
		EndBox(event);

	if (m_iDragSide)
	{
		g_bld->GetAtArea().Sort();
		m_iDragSide = 0;
	}
}

void BuilderView::OnDragDistance()
{
	if (m_ui.m_bDistanceToolMode)
	{
		// Path mode
		int len = m_distance_path.GetSize();
		m_distance_path[len-1] = m_ui.m_CurLocation;
	}
	else
	{
		// Line mode
		m_distance_p2 = m_ui.m_CurLocation;
	}
	UpdateDistance();
}

void BuilderView::OnLButtonClickElement(vtRoadLayer *pRL)
{
	DRECT world_bound;

	// error is how close to the road/node can we be off by?
	float error = odx(5);

	bool returnVal = false;
	if (m_ui.mode == LB_Node)
		returnVal = pRL->SelectNode(m_ui.m_DownLocation, error, world_bound);
	else if (m_ui.mode == LB_Link)
		returnVal = pRL->SelectLink(m_ui.m_DownLocation, error, world_bound);
	else if (m_ui.mode == LB_LinkExtend)
		returnVal = pRL->SelectAndExtendLink(m_ui.m_DownLocation, error, world_bound);

	wxString str;
	if (returnVal)
	{
		wxRect screen_bound = WorldToWindow(world_bound);
		IncreaseRect(screen_bound, BOUNDADJUST);
		Refresh(TRUE, &screen_bound);
		if (m_ui.mode == LB_Node)
			str.Printf(_("Selected 1 Node (%d total)"), pRL->GetSelectedNodes());
		else
			str.Printf(_("Selected 1 Road (%d total)"), pRL->GetSelectedLinks());
	}
	else
	{
		DeselectAll();
		str = _("Deselected all");
	}
	if (g_bld->m_pParentWindow)
		g_bld->m_pParentWindow->SetStatusText(str);
}


void BuilderView::OnLButtonClickLinkEdit(vtRoadLayer *pRL)
{
}

void BuilderView::RefreshRoad(LinkEdit *pRoad)
{
	DRECT world_bound = pRoad->m_extent;
	wxRect screen_bound = WorldToWindow(world_bound);
	IncreaseRect(screen_bound, BOUNDADJUST);
	Refresh(TRUE, &screen_bound);
}

void BuilderView::OnLButtonClickFeature(vtLayerPtr pL)
{
	if (pL->GetType() == LT_STRUCTURE)
	{
		vtStructureLayer *pSL = (vtStructureLayer *)pL;

		// first do a deselect-all
		pSL->DeselectAll();

		// see if there is a building at m_ui.m_DownPoint
		int building;
		double distance;
		bool found = pSL->FindClosestStructure(m_ui.m_DownLocation, odx(5),
				building, distance);
		if (found)
		{
			vtStructure *str = pSL->GetAt(building);
			str->Select(!str->IsSelected());
		}
		Refresh(false);
	}
	else if (pL->GetType() == LT_UTILITY)
	{
		// TODO? single click selection of utility features
		//vtUtilityLayer *pTL = (vtUtilityLayer *)pL;
	}
	else if (pL->GetType() == LT_RAW)
	{
		// TODO? single click selection of raw features
		//vtRawLayer *pRL = (vtRawLayer *)pL;
	}
}

////////////////

void BuilderView::OnMiddleDown(wxMouseEvent& event)
{
	m_ui.m_bMMouseButton = true;
	m_bMouseMoved = false;

	// save the point where the user clicked
	m_DownClient = event.GetPosition();

	GetCanvasPosition(event, m_ui.m_DownPoint);
	m_ui.m_CurPoint = m_ui.m_DownPoint;
	if (!m_bMouseCaptured)
	{
		CaptureMouse();
		m_bMouseCaptured = true;
	}

	BeginPan();
}

void BuilderView::OnMiddleUp(wxMouseEvent& event)
{
	if (m_bPanning)
		EndPan();

	if (m_bMouseCaptured)
	{
		ReleaseMouse();
		m_bMouseCaptured = false;
	}
}

void BuilderView::OnRightDown(wxMouseEvent& event)
{
	m_ui.m_bRMouseButton = true;
	if (!m_bMouseCaptured)
	{
		CaptureMouse();
		m_bMouseCaptured = true;
	}

	// Dispatch to the layer
	vtLayer *pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnRightDown(this, m_ui);
}

void BuilderView::OnRightUp(wxMouseEvent& event)
{
	m_ui.m_bRMouseButton = false;	//right mouse button no longer down
	if (m_bMouseCaptured)
	{
		ReleaseMouse();
		m_bMouseCaptured = false;
	}

	if (m_ui.mode == LB_Dist)
	{
		wxMenu *popmenu = new wxMenu;
		wxMenuItem *item;
		item = popmenu->Append(ID_DISTANCE_CLEAR, _("Clear Distance Tool"));
		wxPoint point = event.GetPosition();
		PopupMenu(popmenu, point);
		delete popmenu;
		return;
	}

	vtLayer *pL = g_bld->GetActiveLayer();
	if (!pL)
		return;

	// Dispatch to the layer
	pL->OnRightUp(this, m_ui);

	if (pL->GetType() == LT_STRUCTURE)
		OnRightUpStructure((vtStructureLayer *)pL);
}

void BuilderView::OnRightUpStructure(vtStructureLayer *pSL)
{
	pSL->EditBuildingProperties();
}

void BuilderView::OnMouseMove(wxMouseEvent& event)
{
	wxPoint point = event.GetPosition();
//	VTLOG("MouseMove(%d %d)\n", point.x, point.y);
	static wxPoint lastpoint;

	if (point == lastpoint)
		return;

	GetCanvasPosition(event, m_ui.m_CurPoint);
	object(m_ui.m_CurPoint, m_ui.m_CurLocation);

	if (m_ui.m_bLMouseButton || m_ui.m_bMMouseButton || m_ui.m_bRMouseButton)
	{
		wxPoint diff = m_ui.m_CurPoint - m_ui.m_DownPoint;
		int mag = abs(diff.x) + abs(diff.y);
		if (mag > 2 && !m_bMouseMoved)
			m_bMouseMoved = true;
	}

	if (m_bPanning)
		DoPan(point);

	// left button click and drag
	if (m_ui.m_bLMouseButton)
	{
		if (m_bBoxing)
			DoBox(m_ui.m_CurPoint);
		if (m_iDragSide)
			DoArea(point - lastpoint);
		if (m_ui.mode == LB_Dist)
		{
			wxClientDC dc(this);
			PrepareDC(dc);

			DrawDistanceTool(&dc);	// erase
			OnDragDistance();		// update
			DrawDistanceTool(&dc);	// redraw
		}
		else if (m_ui.mode == LB_BldEdit && m_ui.m_bRubber)
		{
		}
	}

	// Dispatch for layer-specific handling
	vtLayerPtr pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnMouseMove(this, m_ui);

	// update new mouse coordinates, etc. in status bar
	g_bld->RefreshStatusBar();

	m_ui.m_LastPoint = m_ui.m_CurPoint;
	m_ui.m_PrevLocation = m_ui.m_CurLocation;

	lastpoint = point;
}

void BuilderView::OnMouseWheel(wxMouseEvent& event)
{
	if (event.m_wheelRotation > 0)
		SetScale(GetScale() * sqrt(2.0));
	else
		SetScale(GetScale() / sqrt(2.0));

	// update scale in status bar
	g_bld->RefreshStatusBar();
}

void BuilderView::OnIdle(wxIdleEvent& event)
{
	if (!m_bGotFirstIdle)
	{
		m_bGotFirstIdle = true;
		VTLOG("First View Idle\n");
		g_bld->ZoomAll();
		Refresh();
		// wxGetApp().Exit();	// handy for testing memleaks
	}

	int i, iLayers = g_bld->NumLayers();

	// Check to see if any elevation layers needs drawing
	bool bNeedDraw = false;
	bool bDrew = false;
	for (i = 0; i < iLayers; i++)
	{
		vtLayer *lp = g_bld->GetLayer(i);
		if (lp->GetType() == LT_ELEVATION)
		{
			vtElevLayer *pEL = (vtElevLayer *)lp;
			if (pEL->m_draw.m_bShowElevation && pEL->NeedsDraw())
				bNeedDraw = true;
		}
	}
	if (bNeedDraw)
	{
		OpenProgressDialog2(_("Rendering elevation layers"), false);
		for (i = 0; i < iLayers; i++)
		{
			vtLayer *lp = g_bld->GetLayer(i);
			UpdateProgressDialog2(i * 99 / iLayers, 0, lp->GetLayerFilename());
			if (lp->GetType() == LT_ELEVATION)
			{
				vtElevLayer *pEL = (vtElevLayer *)lp;
				if (pEL->m_draw.m_bShowElevation && pEL->NeedsDraw())
				{
					pEL->RenderBitmap();
				}
			}
		}
		CloseProgressDialog2();
		Refresh(true);
	}
}

void BuilderView::OnSize(wxSizeEvent& event)
{
	// Attempt to avoid unnecessary redraws on shrinking the window.
	// Unfortunately using Skip() alone appears to have no effect,
	//  we still get the Refresh-Draw event.
	wxSize size = GetSize();
	//VTLOG("View OnSize %d, %d\n", size.x, size.y);
	if (size == m_previous_size)
		event.Skip(true);	// allow event to be handled normally
	else
	{
		if (m_bScaleBar)
			RefreshRect(m_ScaleBarArea);
		if (size.x <= m_previous_size.x && size.y <= m_previous_size.y && m_bGotFirstIdle)
		{
			// "prevent additional event handlers from being called and control
			// will be returned to the sender of the event immediately after the
			// current handler has finished."
			event.Skip(false);

			// Since that doesn't work, we use our own logic
			m_bSkipNextRefresh = true;
		}
		else
			event.Skip(true);	// allow event to be handled normally
	}
	m_previous_size = size;
}


//////////////////
// Keyboard shortcuts

//#include <wx/Dir.h>
//#include <wx/File.h>
//#include "vtdata/TripDub.h"
//#include "vtdata/vtDIB.h"
#include <map>

void BuilderView::OnChar(wxKeyEvent& event)
{
#if VTDEBUG
	VTLOG("Char %d (%c) ctrl:%d\n", event.GetKeyCode(), event.GetKeyCode(), event.ControlDown());
#endif

	bool ctrl = event.ControlDown();
	int code = event.GetKeyCode();

	if (code == ' ')
	{
		SetMode(LB_Pan);
		SetCorrectCursor();
	}
	else if (code == WXK_ADD && ctrl)
	{
		SetScale(GetScale() * sqrt(2.0));
	}
	else if (code == WXK_SUBTRACT && ctrl)
	{
		SetScale(GetScale() / sqrt(2.0));
	}
	else
		event.Skip();
}

void BuilderView::RunTest()
{
	// a place to put quick hacks and tests
#if 0
	vtRoadLayer *pR = (vtRoadLayer *)g_bld->FindLayerOfType(LT_ROAD);
	vtElevLayer *pE = (vtElevLayer *)g_bld->FindLayerOfType(LT_ELEVATION);
	pR->CarveRoadway(pE, 2.0);
#endif
#if 0
	vtElevLayer *pE = (vtElevLayer *)g_bld->FindLayerOfType(LT_ELEVATION);
	if (pE)
	{
		vtElevationGrid *g = pE->m_pGrid;
		int xs, zs;
		g->GetDimensions(xs, zs);
		for (int i = 0; i < xs; i++)
			for (int j = 0; j < zs; j++)
			{
				float val = g->GetFValue(i, j);
				val += (cos(j*0.02) + cos(i*0.02))*5;
				g->SetFValue(i, j, val);
			}
			g->ComputeHeightExtents();
			pE->SetModified(true);
			pE->ReRender();
	}
#endif
#if 0
	vtString dir = "E:/Data-Distro/Culture/UtilityStructures";
	for (dir_iter it((const char *)dir); it != dir_iter(); ++it)
	{
		if (it.is_directory())
			continue;
		vtString name = it.filename().c_str();
		if (name.Find(".obj") == -1)
			continue;
		FILE *in = vtFileOpen(dir + "/" + name, "rb");
		FILE *out = vtFileOpen(dir + "/" + name+"2", "wb");
		if (!in || !out)
			continue;
		char buf[99];
		double x, y, z;
		while (fgets(buf, 99, in))
		{
			if (buf[0] == 'v')
			{
				sscanf(buf, "v %lf %lf %lf", &x, &y, &z);
				fprintf(out, "v %lf %lf %lf\n", x, z, -y);
			}
			else
				fputs(buf, out);
		}
		fclose(out);
		fclose(in);
	}
#endif
#if 0
	{
		// create grid of polygons
		vtFeatureSetPolygon set;
		vtProjection proj;
		proj.SetWellKnownGeogCS("NAD83");
		proj.SetUTMZone(5);
		set.SetProjection(proj);
		DPoint2 base(215500, 2213000), spacing(1000,1000);
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				DLine2 dline;
				dline.Append(base + DPoint2(i*spacing.x, j*spacing.y));
				dline.Append(base + DPoint2((i+1)*spacing.x, j*spacing.y));
				dline.Append(base + DPoint2((i+1)*spacing.x, (j+1)*spacing.y));
				dline.Append(base + DPoint2(i*spacing.x, (j+1)*spacing.y));
				DPolygon2 poly;
				poly.push_back(dline);
				set.AddPolygon(poly);
			}
		}
		set.SaveToSHP("C:/Temp/waimea_quads.shp");
	}
#endif
#if 0
	DLine2 dline;
	dline.Append(DPoint2(0,0));
	dline.Append(DPoint2(1,0));
	dline.Append(DPoint2(1,1));
	dline.Append(DPoint2(0,1));
	vtStructureArray str;
	vtBuilding *bld = str.NewBuilding();
	bld->SetFootprint(0, dline);
	bld->SetStories(2);
	bld->SetRoofType(RT_HIP);
	bld->SetColor(BLD_BASIC, RGBi(255,0,0))
		bld->SetColor(BLD_ROOF, RGBi(255,255,255))
#endif
#if 0
		double left=0.00000000000000000;
	double top=0.0052590002305805683;
	double right=0.0070670000277459621;
	double bottom=0.00000000000000000;

	double ScaleX = vtProjection::GeodesicDistance(DPoint2(left,bottom),DPoint2(right,bottom));
	double foo = ScaleX;
#endif
#if 0
	wxString pname = _T("G:/Data-Charsettest/Temp");
	wxString filename;
	wxDir dir(pname);
	dir.GetFirst(&filename);
	bool result = wxFile::Access(pname + _T("/") + filename, wxFile::read);
	if (result)
		VTLOG("success\n");
#endif
#if 0
	ReqContext con;

	IPoint2 base(8838, 7430);
	IPoint2 size(50, 30);
	vtDIB output;
	output.Create(size.x*258, size.y*258, 24);
	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			int xx = base.x + x;
			int yy = base.y + y;
			vtBytes data;
			vtString url;
			url.Format("http://us.maps3.yimg.com/aerial.maps.yimg.com/tile?v=1.4&t=a&x=%d&y=%d&z=1",
				xx, yy);
			VTLOG1(url + "\n");
			bool result = con.GetURL(url, data);
			if (!result)
				continue;

			vtString fname;
			fname.Format("c:/temp/tile_%04d_%04d.jpg", xx, yy);
			FILE *fp = fopen(fname, "wb");
			fwrite(data.Get(), data.Len(), 1, fp);
			fclose(fp);

			vtDIB tile;
			if (tile.ReadJPEG(fname))
				tile.BlitTo(output, x * 258, (size.y - 1 - y) * 258);
		}
	}
	VTLOG1("Writing output\n");
	output.WriteBMP("c:/temp/output.bmp");
#endif
#if 0
	#include "C:/Dev/TMK-Process/TMK.cpp"
#endif
#if 0
	{
		// create grid of points over current layer
		vtFeatureSetPoint2D set;
		vtProjection proj;
		set.SetProjection(g_bld->GetAtProjection());
		DRECT area = g_bld->m_area;
		set.AddField("filename", FT_String, 30);
		set.AddField("rotation", FT_Float);

		DPoint2 spacing(area.Width()/21, area.Height()/21);
		for (int i = 0; i < 22; i++)
		{
			for (int j = 0; j < 22; j++)
			{
				DPoint2 p;
				p.x = area.left + i*spacing.x;
				p.y = area.bottom + j*spacing.y;
				int rec = set.AddPoint(p);
				set.SetValue(rec, 0, "C:/temp/triangle.osg");
				set.SetValue(rec, 1, 110 + (i * 5) - (j * 2));
			}
		}
		set.SaveToSHP("C:/Temp/PearlRiverPoints.shp");
	}
#endif
#if 0
	{
		vtStructureArray *sa = new vtStructureArray();
		sa->m_proj.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
		// 1557 buildings
		sa->ReadXML("G:/Data-USA/Data-Hawaii/BuildingData/stage5.vtst");
		//sa->ReadXML("G:/Data-USA/Data-Hawaii/BuildingData/one_building.vtst");
	}
#endif
#if 0
	{
		vtRawLayer *ab = g_bld->GetActiveRawLayer();
		vtFeatureSetLineString3D *fe3;
		fe3 = dynamic_cast<vtFeatureSetLineString3D*>(ab->GetFeatureSet());
		if (fe3)
		{
			DLine3 &line = fe3->GetPolyLine(0);
			for (int i = 0; i < line.GetSize(); i++)
			{
				DPoint3 p = line[i];
				p.z -= 2000;
				line[i] = p;
			}
		}
	}
#endif
#if 0
	{
		vtProjection proj;
		vtElevationGrid grid(DRECT(0, 1, 1, 0), 5, 5, true, proj);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				if (i == 4 || j == 4)
					grid.SetFValue(i, j, 1);
				else
					grid.SetFValue(i, j, 0);
			}
		}

		vtBitmap bmp;
		bmp.Allocate(4, 4, 24);

		std::vector<RGBi> table;
		table.push_back(RGBi(0,0,0));
		table.push_back(RGBi(255,255,255));

		grid.ColorDibFromTable(&bmp, table, 0, 1);
	}
#endif
#if 0
	{
		vtStructureLayer *pL = (vtStructureLayer *)g_bld->FindLayerOfType(LT_STRUCTURE);
		if (pL)
		{
			pL->DeselectAll();
			vtStructure *str = pL->GetAt(868);
			if (str)
			{
				str->Select(true);
				DRECT r;
				str->GetExtents(r);
				ZoomToRect(r, 0.1f);
			}
		}
	}
#endif
#if 1
	vtRawLayer *pRaw = g_bld->GetActiveRawLayer();
	if (!pRaw) return;
	vtFeatureSetPolygon *fsp = (vtFeatureSetPolygon*) pRaw->GetFeatureSet();
	int fixed = fsp->FixGeometry(0.05);	// 5 cm
	Refresh();
#endif
}

void BuilderView::OnKeyDown(wxKeyEvent& event)
{
	int code = event.GetKeyCode();
	bool ctrl = event.ControlDown();
#if VTDEBUG
//	VTLOG("View: KeyDown %d (%c) ctrl:%d\n", code, event.GetKeyCode(), ctrl);
#endif

#if 0
	wxCommandEvent dummy;

	// Some accelerators aren't caught properly (at least on Windows)
	//  So, explicitly check for them here.
	if (code == 43 && ctrl)
		g_bld->OnViewZoomIn(dummy);
	else if (code == 45 && ctrl)
		g_bld->OnViewZoomOut(dummy);

	else
		event.Skip();
#endif
}

void BuilderView::OnEraseBackground( wxEraseEvent& event )
{
	// there are some erase events we don't need, such as when sizing the
	//  window smaller
	if (m_bSkipNextRefresh)
		event.Skip(false);
	else
		event.Skip(true);
	m_bSkipNextRefresh = false;
}
