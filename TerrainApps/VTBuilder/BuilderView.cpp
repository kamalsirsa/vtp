//
// BuilderView.cpp
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/vtLog.h"

#include "BuilderView.h"
#include "Frame.h"
#include "MenuEnum.h"
#include "Helper.h"
#include "VTBuilder_wdr.h"
// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
// Dialogs
#include "vtui/DistanceDlg.h"
#include "vtui/ProfileDlg.h"

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
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////

BuilderView::BuilderView(wxWindow* parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, const wxString& name) :
		vtScaledView(parent, id, pos, size, 0, name )
{
	VTLOG(" Constructing BuilderView\n");

	m_bSkipNextDraw = false;
	m_bSkipNextRefresh = false;
	m_bGotFirstIdle = false;

	m_bCrossSelect = false;
	m_bShowMap = true;
	m_bShowUTMBounds = false;

	m_bMouseMoved = false;
	m_bPanning = false;
	m_bBoxing = false;
	m_iDragSide = 0;
	m_bMouseCaptured = false;

	m_ui.m_bRubber = false;
	m_ui.mode = LB_None;
	m_ui.m_bLMouseButton = m_ui.m_bMMouseButton = m_ui.m_bRMouseButton = false;
	m_ui.m_pEditingRoad = NULL;
	m_ui.m_iEditingPoint = -1;
	m_ui.m_pCurBuilding = NULL;
	m_ui.m_pCurLinear = NULL;

	m_distance_p1.Set(0,0);
	m_distance_p2.Set(0,0);

	// Cursors are a little messy, since support is not even across platforms
#if __WXMSW__
	m_pCursorPan = new wxCursor(_T("cursors/panhand.cur"), wxBITMAP_TYPE_CUR);
//#elif __WXMOTIF__
//	m_pCursorPan = new wxCursor("cursors/panhand.xbm", wxBITMAP_TYPE_XBM);
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

	if (m_bSkipNextDraw)
	{
		m_bSkipNextDraw = false;
		return;
	}

	MainFrame *pFrame = GetMainFrame();
	if (pFrame->DrawDisabled())
		return;

	vtLayerPtr lp;
	int i, iLayers = pFrame->NumLayers();

	// Draw 'interrupted projection outline' for current projection
	vtProjection proj;
	GetMainFrame()->GetProjection(proj);
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
		lp = pFrame->GetLayer(i);
		if (lp->GetType() != LT_IMAGE && lp->GetType() != LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(&dc, this);
	}
	// Then the poly/vector/point layers
	for (i = 0; i < iLayers; i++)
	{
		lp = pFrame->GetLayer(i);
		if (lp->GetType() == LT_IMAGE || lp->GetType() == LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(&dc, this);
	}
	if (pFrame->GetActiveElevLayer())
		HighlightTerrain(&dc, pFrame->GetActiveElevLayer());

	if (m_bShowUTMBounds)
		DrawUTMBounds(&dc);

	DrawAreaTool(&dc, pFrame->m_area);
	DrawDistanceTool(&dc);
}

void BuilderView::DrawDymaxionOutline(wxDC *pDC)
{
	DLine2Array polys;

	m_icosa.GetDymaxEdges(polys);

	for (unsigned int i = 0; i < polys.size(); i++)
	{
		DrawLine(pDC, polys[i], true);
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

	// Show this dialog only in AddLinear mode
	GetMainFrame()->ShowLinearStructureDlg(m_ui.mode == LB_AddLinear);

	// Show this dialog only in AddInstance mode
	GetMainFrame()->ShowInstanceDlg(m_ui.mode == LB_AddInstance);

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
	GetMainFrame()->GetProjection(proj);

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

	const char *filename = "WorldMap/gnv19.shp";
	VTLOG(" Attempting to open %s\n", filename);

	// Open SHP file
	hSHP = SHPOpen(filename, "rb");
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
	m_iEntities = WMPoly.size();
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

#if DEBUG
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
	OCT *trans = CreateCoordTransform(&Source, &proj);

	if (!trans)
	{
		m_bShowMap = false;
		return;
	}

	DPoint2 point;
	for (i = 0; i < m_iEntities; i++)
	{
		WMPolyDraw[i].Empty();
		for (j = 0; j < WMPoly[i].GetSize(); j++)
		{
			point = WMPoly[i].GetAt(j);

			int converted = trans->Transform(1, &point.x, &point.y);

			if (converted == 1)
				WMPolyDraw[i].Append(point);
		}
	}
	delete trans;
}

void BuilderView::DrawWorldMap(wxDC *pDC)
{
	if (m_iEntities == 0 && !m_bAttemptedLoad)
	{
		m_bAttemptedLoad = true;
		if (ImportWorldMap())
		{
			vtProjection proj;
			GetMainFrame()->GetProjection(proj);
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

	// Draw each poly in WMPolyDraw
	for (unsigned int i = 0; i < m_iEntities; i++)
		DrawLine(pDC, WMPolyDraw[i], true);
}

//////////////////////////////////////////////////////////
// Pan handlers

void BuilderView::BeginPan()
{
	m_bPanning = true;
	SetCursor(*m_pCursorPan);
}

void BuilderView::EndPan()
{
	m_bPanning = false;
	SetCorrectCursor();
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
	MainFrame *frame = GetMainFrame();

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
			DrawAreaTool(&dc, frame->m_area);
			frame->m_area = m_world_rect;
			DrawAreaTool(&dc, frame->m_area);
			break;
		case LB_Node:
		case LB_Link:
			{
			// select everything in the highlighted box.
				vtRoadLayer *pRL = GetMainFrame()->GetActiveRoadLayer();
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
	vtLayer *pL = GetMainFrame()->GetActiveLayer();
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
		vtStructureLayer *pSL = (vtStructureLayer *)pL;
		changed = pSL->DoBoxSelect(m_world_rect, st);
		selected = pSL->NumSelected();
	}
	if (pL->GetType() == LT_RAW)
	{
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
	GetMainFrame()->SetStatusText(verb);

	GetMainFrame()->OnSelectionChanged();
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

void BuilderView::DrawDistanceTool(wxDC *pDC)
{
	wxPoint p1, p2;
	screen(m_distance_p1, p1);
	screen(m_distance_p2, p2);

	pDC->SetPen(wxPen(*wxBLACK_PEN));
	pDC->SetLogicalFunction(wxINVERT);

	// draw small crosshairs
	pDC->DrawLine(p1.x-4, p1.y, p1.x+4+1, p1.y);
	pDC->DrawLine(p1.x, p1.y-4, p1.x, p1.y+4+1);
	pDC->DrawLine(p2.x-4, p2.y, p2.x+4+1, p2.y);
	pDC->DrawLine(p2.x, p2.y-4, p2.x, p2.y+4+1);
	// and the line itself
	pDC->DrawLine(p1.x, p1.y, p2.x, p2.y);
}

void BuilderView::BeginLine()
{
}

////////////////////////////////////////////////////////////
// Elevation

void BuilderView::CheckForTerrainSelect(const DPoint2 &loc)
{
	MainFrame *pFrame = GetMainFrame();

	// perhaps the user clicked on a terrain
	bool bFound = false;
	DRECT rect;
	for (int l = 0; l < pFrame->NumLayers(); l++)
	{
		vtLayerPtr lp = pFrame->GetLayer(l);
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

//
// The view needs to be notified of the new active layer to update
// the selection marks drawn around the active elevation layer.
//
void BuilderView::SetActiveLayer(vtLayerPtr lp)
{
	MainFrame *pFrame = GetMainFrame();
	vtElevLayer *last = pFrame->GetActiveElevLayer();
	if (lp != last)
	{
		wxClientDC DC(this), *pDC = &DC;
		PrepareDC(DC);

		if (last)
			HighlightTerrain(pDC, last);

		pFrame->SetActiveLayer(lp, true);

		if (pFrame->GetActiveElevLayer())
			HighlightTerrain(pDC, pFrame->GetActiveElevLayer());
	}
}

void BuilderView::HighlightTerrain(wxDC* pDC, vtElevLayer *t)
{
	wxPen bgPen(wxColor(255,255,255), 3, wxSOLID);
	pDC->SetPen(bgPen);
	pDC->SetLogicalFunction(wxINVERT);

	DRECT rect;
	t->GetAreaExtent(rect);
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
	case LB_Link:	// select/edit roads
	case LB_Node:	// select/edit nodes
	case LB_Move:	// move selected nodes
		SetCursor(wxCURSOR_ARROW); break;
	case LB_Pan:	// pan the view
		SetCursor(*m_pCursorPan); break;
	case LB_Dist:	// measure distance
		SetCursor(wxCURSOR_CROSS); break;
	case LB_Mag:	// zoom into rectangle
		SetCursor(wxCURSOR_MAGNIFIER); break;
	case LB_TowerAdd:
		SetCursor(wxCURSOR_CROSS);break; // add a tower to the location
	case LB_Dir:		// show/change road direction
	case LB_LinkEdit:	// edit road points
	case LB_LinkExtend: //extend a road selection
	case LB_TSelect:
	case LB_Box:
	default:
		SetCursor(wxCURSOR_ARROW); break;
	}
}


/////////////////////////////////////////////////////////////

void BuilderView::BeginArea()	// in canvas coordinates
{
	DRECT area = GetMainFrame()->m_area;
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
		if (r.Inside(m_ui.m_CurPoint.x, m_ui.m_CurPoint.y))
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

	MainFrame *frame = GetMainFrame();

	DrawAreaTool(&dc, frame->m_area);	// erase
	if (m_iDragSide & 1)
		frame->m_area.left += odx(delta.x);
	if (m_iDragSide & 2)
		frame->m_area.right += odx(delta.x);
	if (m_iDragSide & 4)
		frame->m_area.top += ody(delta.y);
	if (m_iDragSide & 8)
		frame->m_area.bottom += ody(delta.y);
	DrawAreaTool(&dc, frame->m_area);	// redraw
}

void BuilderView::InvertAreaTool(const DRECT &rect)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	DrawAreaTool(&dc, rect);
}

void BuilderView::DeselectAll()
{
	vtRoadLayer *pRL = GetMainFrame()->GetActiveRoadLayer();
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
	vtStructureLayer *pSL = GetMainFrame()->GetActiveStructureLayer();
	if (pSL)
	{
		pSL->DeselectAll();
		Refresh(TRUE);
	}
	vtRawLayer *pRawL = GetMainFrame()->GetActiveRawLayer();
	if (pRawL)
	{
		pRawL->GetFeatureSet()->DeselectAll();
		Refresh(TRUE);
		GetMainFrame()->OnSelectionChanged();
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
	if (!pEL || !pEL->m_pGrid)
		return;

	DPoint2 spacing = pEL->m_pGrid->GetSpacing();
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

	// "points" are in window pixels, "locations" are in current projection
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

	vtLayerPtr pL = GetMainFrame()->GetActiveLayer();
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
			BeginLine();
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
	vtLayerPtr pL = GetMainFrame()->GetActiveLayer();
	if (pL)
		pL->OnLeftUp(this, m_ui);

	m_ui.m_bLMouseButton = false;	// left mouse button no longer down
}

void BuilderView::OnLeftDoubleClick(wxMouseEvent& event)
{
	GetCanvasPosition(event, m_ui.m_DownPoint);
	m_ui.m_CurPoint = m_ui.m_LastPoint = m_ui.m_DownPoint;
	object(m_ui.m_DownPoint, m_ui.m_DownLocation);

	vtLayer *pL = GetMainFrame()->GetActiveLayer();
	if (pL)
		pL->OnLeftDoubleClick(this, m_ui);
}

void BuilderView::OnLButtonClick(wxMouseEvent& event)
{
	vtLayerPtr pL = GetMainFrame()->GetActiveLayer();
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
		GetMainFrame()->m_area.Sort();
		m_iDragSide = 0;
	}
}

void BuilderView::OnDragDistance()
{
	object(m_ui.m_DownPoint, m_distance_p1);
	object(m_ui.m_LastPoint, m_distance_p2);

	GetMainFrame()->UpdateDistance(m_distance_p1, m_distance_p2);
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

	if (returnVal)
	{
		wxRect screen_bound = WorldToWindow(world_bound);
		IncreaseRect(screen_bound, BOUNDADJUST);
		Refresh(TRUE, &screen_bound);
		wxString str;
		if (m_ui.mode == LB_Node)
			str.Format(_("Selected 1 Node (%d total)"), pRL->GetSelectedNodes());
		else
			str.Format(_("Selected 1 Road (%d total)"), pRL->GetSelectedLinks());
		GetMainFrame()->SetStatusText(str);
	}
	else
	{
		DeselectAll();
		GetMainFrame()->SetStatusText(_("Deselected all"));
	}
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
	vtLayer *pL = GetMainFrame()->GetActiveLayer();
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

	vtLayer *pL = GetMainFrame()->GetActiveLayer();
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
	vtLayerPtr pL = GetMainFrame()->GetActiveLayer();
	if (pL)
		pL->OnMouseMove(this, m_ui);

	// update new mouse coordinates, etc. in status bar
	GetMainFrame()->RefreshStatusBar();

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
	GetMainFrame()->RefreshStatusBar();
}

void BuilderView::OnIdle(wxIdleEvent& event)
{
	if (!m_bGotFirstIdle)
	{
		m_bGotFirstIdle = true;
		VTLOG("First View Idle\n");
		GetMainFrame()->ZoomAll();
		Refresh();
		// wxGetApp().Exit();	// handy for testing memleaks
	}

	MainFrame *pFrame = GetMainFrame();
	int i, iLayers = pFrame->NumLayers();

	// Check to see if any elevation layers needs drawing
	bool bDrew = false;
	for (i = 0; i < iLayers; i++)
	{
		vtLayer *lp = pFrame->GetLayer(i);
		if (lp->GetType() == LT_ELEVATION)
		{
			vtElevLayer *pEL = (vtElevLayer *)lp;
			if (pEL->NeedsDraw())
			{
				pEL->RenderBitmap();
				bDrew = true;
			}
		}
	}
	if (bDrew)
		Refresh(true);
}

void BuilderView::OnSize(wxSizeEvent& event)
{
	// Attempt to avoid unnecessary redraws on shrinking the window.
	// Unfortunately using Skip() alone appears to have no effect,
	//  we still get the Refresh-Draw event.
	wxSize size = GetSize();
	if (size == m_previous_size)
		event.Skip(true);	// allow event to be handled normally
	else if (size.x <= m_previous_size.x && size.y <= m_previous_size.y && m_bGotFirstIdle)
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
	m_previous_size = size;
}


//////////////////
// Keyboard shortcuts

#include "vtui/wxString2.h"

void BuilderView::OnChar(wxKeyEvent& event)
{
#if DEBUG
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
	else if (code == WXK_F10)
	{
		// a place to put quick hacks and tests
#if 0
		vtRawLayer *pRaw = GetMainFrame()->GetActiveRawLayer();
		if (!pRaw) return;
		pRaw->ReadGeoURL();
		Refresh();
#endif
#if 0
		wxString str = wxGetTextFromUser(_T("Test Message"),
			_T("Test Caption"), _T(""), this);

		const char *from = str.mb_str();
		VTLOG("from wxString's mb_str: %s\n", from);

		wxString2 wx2 = str.c_str();

		const char *from2 = wx2.mb_str();
		VTLOG("from wxString2's mb_str: %s\n", from2);

		wstring2 str2 = str.c_str();
		const char *utf8 = str2.to_utf8();
		VTLOG("as UTF-8: %s\n", utf8);

		// back again
		wstring2 str3;
		size_t result = str3.from_utf8(utf8);

		utf8 = str3.to_utf8();
		VTLOG("Twice converted: %s\n", utf8);
#endif
#if 0
		vtRoadLayer *pR = (vtRoadLayer *)GetMainFrame()->FindLayerOfType(LT_ROAD);
		vtElevLayer *pE = (vtElevLayer *)GetMainFrame()->FindLayerOfType(LT_ELEVATION);
		pR->CarveRoadway(pE, 2.0);
#endif
#if 0
		DBFHandle db = DBFOpen("E:/GreenMaps/Locations/map_locations_v4.dbf", "rb");
		DBFHandle db2 = DBFCreate("E:/GreenMaps/Locations/map_locations_v4d.dbf");
		if (db != NULL && db2 != NULL)
		{
			int iEntities = DBFGetRecordCount(db);
			int iFields = DBFGetFieldCount(db);

			int f1 = DBFAddField(db2, "X", FTDouble, 12, 6);
			int f2 = DBFAddField(db2, "Y", FTDouble, 12, 6);

			DBFFieldType type;
			char fieldname[80];
			int width, decimals;
			for (int field = 2; field < iFields; field++)
			{
				type = DBFGetFieldInfo(db, field, fieldname, &width, &decimals);
				DBFAddField(db2, fieldname, type, width, decimals);
			}

			int deg, min;
			double val;
			const char *sz;
			for (int i = 0; i < iEntities; i++)
			{
				for (int field = 0; field < iFields; field++)
				{
					sz = DBFReadStringAttribute(db, i, field);

					if (field == 0)
					{
						deg = min = 0;
						sscanf(sz, "%d %d", &deg, &min);
						val = abs(deg) + (double)min / 60.0;
						if (deg < 0) val = -val;
						DBFWriteDoubleAttribute(db2, i, f1, val);
					}
					else if (field == 1)
					{
						deg = min = 0;
						sscanf(sz, "%d %d", &deg, &min);
						val = abs(deg) + (double)min / 60.0;
						if (deg < 0) val = -val;
						DBFWriteDoubleAttribute(db2, i, f2, val);
					}
					else
					{
						DBFWriteStringAttribute(db2, i, field, sz);
					}
				}
			}
			DBFClose(db);
			DBFClose(db2);
		}
#endif
#if 0
		vtElevLayer *pE = (vtElevLayer *)GetMainFrame()->FindLayerOfType(LT_ELEVATION);
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
			FILE *in = fopen(dir + "/" + name, "rb");
			FILE *out = fopen(dir + "/" + name+"2", "wb");
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
	}
	else
		event.Skip();
}

void BuilderView::OnKeyDown(wxKeyEvent& event)
{
	int code = event.GetKeyCode();
	bool ctrl = event.ControlDown();
#if DEBUG
//	VTLOG("KeyDown %d (%c) ctrl:%d\n", code, event.GetKeyCode(), ctrl);
#endif

	wxCommandEvent dummy;
	MainFrame *pFrame = GetMainFrame();

	// Some accelerators aren't caught properly (at least on Windows)
	//  So, explicitly check for them here.
	if (code == 43 && ctrl)
		pFrame->OnViewZoomIn(dummy);
	else if (code == 45 && ctrl)
		pFrame->OnViewZoomOut(dummy);

	else
		event.Skip();
}

void BuilderView::OnEraseBackground( wxEraseEvent& event )
{
	// there are some erase events we don't need, such as when sizing the
	//  window smaller
	if (m_bSkipNextRefresh)
	{
		event.Skip(false);
		m_bSkipNextDraw = true;
	}
	else
		event.Skip(true);
	m_bSkipNextRefresh = false;
}
