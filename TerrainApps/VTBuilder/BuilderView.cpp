//
// BuilderView.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
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
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
// Dialogs
#include "DistanceDlg.h"

#include "cpl_error.h"

#define BOUNDADJUST 5

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

	EVT_CHAR(BuilderView::OnChar)
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////

BuilderView::BuilderView(wxWindow* parent, wxWindowID id, const wxPoint& pos,
						 const wxSize& size, const wxString& name) :
	vtScaledView(parent, id, pos, size, name )
{
	VTLOG(" Constructing BuilderView\n");
	m_bPanning = false;
	m_bBoxing = false;
	m_iDragSide = 0;
	m_bCrossSelect = false;
	m_bMouseMoved = false;
	m_bShowMap = true;
	m_bShowUTMBounds = false;

	m_ui.m_bRubber = false;
	m_ui.mode = LB_None;
	m_ui.m_bLMouseButton = m_ui.m_bMMouseButton = m_ui.m_bRMouseButton = false;
	m_ui.m_pEditingRoad = NULL;
	m_ui.m_iEditingPoint = -1;
	m_ui.m_pCurBuilding = NULL;
	m_ui.m_pCurLinear = NULL;

	// Cursors are a little messy, since support is not even across platforms
#if __WXMSW__
	m_pCursorPan = new wxCursor("cursors/panhand.cur", wxBITMAP_TYPE_CUR);
//#elif __WXMOTIF__
//	m_pCursorPan = new wxCursor("cursors/panhand.xbm", wxBITMAP_TYPE_XBM);
#else
	// the predefined "hand" cursor isn't quite correct, since it is a image
	// of a hand with a pointing finger, not a closed, grasping hand.
	m_pCursorPan = new wxCursor(wxCURSOR_HAND);
#endif

	// Import world map SHP file
	WMPoly = NULL;
	WMPolyDraw = NULL;
	m_NoLines = NULL;
	ImportWorldMap();
}

BuilderView::~BuilderView()
{
	delete [] WMPoly;
	delete [] WMPolyDraw;
	delete [] m_NoLines;
}

////////////////////////////////////////////////////////////
// Operations

void BuilderView::OnDraw(wxDC& dc)  // overridden to draw this view
{
	MainFrame *pFrame = GetMainFrame();
	vtLayerPtr lp;
	int i, iLayers = pFrame->m_Layers.GetSize();

	// Draw the world map SHP file of country outline polys in latlon
	if (m_bShowMap)
		DrawWorldMap(&dc, this);

	// Draw the solid layers first
	for (i = 0; i < iLayers; i++)
	{
		lp = pFrame->m_Layers.GetAt(i);
		if (lp->GetType() != LT_IMAGE && lp->GetType() != LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(&dc, this);
	}
	// Then the poly/vector/point layers
	for (i = 0; i < iLayers; i++)
	{
		lp = pFrame->m_Layers.GetAt(i);
		if (lp->GetType() == LT_IMAGE || lp->GetType() == LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(&dc, this);
	}
	if (pFrame->GetActiveElevLayer())
		HighlightTerrain(&dc, pFrame->GetActiveElevLayer());

	if (m_bShowUTMBounds)
		DrawUTMBounds(&dc);

	DrawArea(&dc);
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

		OCT *trans = OGRCreateCoordinateTransformation(&proj, &geo);

		// try to speed up a bit by avoiding zones off the screen
		object(wxPoint(0, height/2), proj_point);
		trans->Transform(1, &proj_point.x, &proj_point.y);
		zone = GuessZoneFromLongitude(proj_point.x);
		if (zone-1 > zone_start) zone_start = zone-1;

		object(wxPoint(width, height/2), proj_point);
		trans->Transform(1, &proj_point.x, &proj_point.y);
		zone = GuessZoneFromLongitude(proj_point.x);
		if (zone+1 < zone_end) zone_end = zone+1;

		delete trans;

		// Now convert the longitude lines between the UTM zones to the current projection
		trans = OGRCreateCoordinateTransformation(&geo, &proj);

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
void BuilderView::ImportWorldMap()
{
	SHPHandle	hSHP;
	int			nShapeType;
	double		adfMinBound[4], adfMaxBound[4];
	int			i, j;

	// Open SHP file
	hSHP = SHPOpen( "WorldMap/gnv19.shp", "rb");
	if (hSHP == NULL) return;

	SHPGetInfo(hSHP, &m_iEntities, &nShapeType, adfMinBound, adfMaxBound);
	if (nShapeType != SHPT_POLYGON) return;

	// Copy SHP data into World Map Poly data
	WMPoly = new DLine2[m_iEntities];
	WMPolyDraw = new DLine2[m_iEntities];
	m_NoLines = new BoolArray[m_iEntities];
	m_bHidden = false;

	for (i = 0; i < m_iEntities; i++)
	{
		DLine2 &current = WMPoly[i];
		BoolArray &noline = m_NoLines[i];

		DPoint2 p;

		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		int points = psShape->nVertices;

		current.SetMaxSize(points);
		noline.SetMaxSize(points);

		for (j = 0; j < points; j++)
		{
			p.x = psShape->padfX[j];
			p.y = psShape->padfY[j];
			current.Append(p);
		}
		SHPDestroyObject(psShape);

		for (j = 0; j < points; j++)
			noline[j] = false;
	}

	// Close SHP file
	SHPClose(hSHP);

	// Initialize the drawn World Map WMPolyDraw to original (latlon)
	for (i = 0; i < m_iEntities; i++)
		WMPolyDraw[i] = WMPoly[i];
}

//
// post-process WMPoly to determine lines not to draw 
//
void BuilderView::HideWorldMapEdges()
{
	int i, points;

	for (i = 0; i < m_iEntities; i++)
	{
		DLine2 &current = WMPoly[i];
		BoolArray &noline = m_NoLines[i];
		points = current.GetSize();
		// must have at least 8 points to worry about internal edges
		if (points < 8 || points > 8000)
			continue;

		// detect and turn off internal edges
		int a, b;
		for (a = 0; a < points-2; a++)
		{
			for (b = a+1; b < points-1; b++)
			{
				if (current[a] == current[b])
				{
					// This is an internal edge
					noline[a+1] = true;
					noline[b] = true;
					break;
				}
			}
		}
	}
	m_bHidden = true;
}


void myErrorHandler(CPLErr err, int i, const char*str)
{
	int foo = err;
	wxLogDebug(str);
}

void BuilderView::SetWMProj(const vtProjection &proj)
{
	int i, j;

	if (!WMPoly)
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
	((vtProjection &)proj).exportToWkt(&str2);
	wxLogDebug(str1);
	wxLogDebug(str2);

	// Check texts in PROJ4
	char *str3, *str4;
	Source.exportToProj4(&str3);
	wxLogDebug(str3);
	proj.exportToProj4(&str4);
	wxLogDebug(str4);
#endif

	CPLPushErrorHandler(myErrorHandler);
	// Create conversion object
	OCT *trans = OGRCreateCoordinateTransformation(&Source, &proj);
	CPLPopErrorHandler();

	if (!trans)
	{
		m_bShowMap = false;
		return;
	}

	DPoint2 point;
	for (i = 0; i < m_iEntities; i++)
	{
		for (j = 0; j < WMPoly[i].GetSize(); j++)
		{
			point = WMPoly[i][j];
			trans->Transform(1, &point.x, &point.y);
			WMPolyDraw[i][j] = point;
		}
	}
}


#define MAXPOINTS 8000
static wxPoint wmbuf[MAXPOINTS];
// All poly have less than 8000 points except ocean (the one we don't want
// to draw)

void BuilderView::DrawWorldMap(wxDC* pDC, vtScaledView *pView)
{
	// Set pen options
	wxPen WMPen(wxColor(0,0,0), 1, wxSOLID);  //solid black pen
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(WMPen);

	if (!m_bHidden)
	{
		VTLOG("Preparing World Map...");
	    wxDialog dialog(this, -1, "Preparing World Map...", wxDefaultPosition,
			wxSize(400, 50), wxDEFAULT_DIALOG_STYLE | wxDIALOG_MODELESS);
		dialog.Show(true);
		HideWorldMapEdges();
		dialog.Show(false);
		Refresh();
		VTLOG(" Done.\n");
	}

	// Draw each poly in WMPolyDraw
	for (int i = 0; i < m_iEntities; i++)
	{
		int wmbuflen = 0;
		if (WMPolyDraw[i].GetSize() > MAXPOINTS)
			continue;

		for (int j = 0; j < WMPolyDraw[i].GetSize(); j++)
		{
			wmbuf[j].x = pView->sx(WMPolyDraw[i][j].x);
			wmbuf[j].y = pView->sy(WMPolyDraw[i][j].y);
			wmbuflen += 1;
			if (m_NoLines[i][j] == true)
			{
				if (wmbuflen > 2) pDC->DrawLines(wmbuflen - 1, wmbuf);
				wmbuflen = 0;
			}
		}
		if (wmbuflen > 1) pDC->DrawLines(wmbuflen, wmbuf);
		wmbuflen = 0;
	}
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
		DrawArea(&dc);
		GetMainFrame()->m_area = m_world_rect;
		DrawArea(&dc);
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

	const char *verb;
	SelectionType st;
	// operation may be select, add (shift), subtract (alt), toggle (ctrl)
	if (event.ShiftDown())
	{
		st = ST_ADD;
		verb = "Added";
	}
	else if (event.AltDown())
	{
		st = ST_SUBTRACT;
		verb = "Subtracted";
	}
	else if (event.ControlDown())
	{
		st = ST_TOGGLE;
		verb = "Toggled";
	}
	else
	{
		st = ST_NORMAL;
		verb = "Selected";
	}

	int changed, selected;
	if (pL->GetType() == LT_STRUCTURE)
	{
		vtStructureLayer *pSL = (vtStructureLayer *)pL;
		changed = pSL->DoBoxSelect(m_world_rect, st);
		selected = pSL->NumSelected();
	}
	if (pL->GetType() == LT_RAW)
	{
		vtRawLayer *pRL = (vtRawLayer *)pL;
		changed = pRL->DoBoxSelect(m_world_rect, st);
		selected = pRL->NumSelected();
	}
	wxString msg;
	msg.Printf("%s %d entit%s, %d total selected", verb, changed,
		changed == 1 ? "y" : "ies", selected);
	GetMainFrame()->SetStatusText(msg);
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

void BuilderView::DrawArea(wxDC *pDC)
{
	DRECT area = GetMainFrame()->m_area;
	if (!area.IsEmpty())
	{
		int d = 3;
		wxRect r = WorldToCanvas(area);
		InvertRect(pDC, r, true);
		InvertRect(pDC, wxPoint(r.x-d, r.y-d), wxPoint(r.x+d, r.y+d));
		InvertRect(pDC, wxPoint(r.x+r.width-d, r.y-d), wxPoint(r.x+r.width+d, r.y+d));
		InvertRect(pDC, wxPoint(r.x-d, r.y+r.height-d), wxPoint(r.x+d, r.y+r.height+d));
		InvertRect(pDC, wxPoint(r.x+r.width-d, r.y+r.height-d), wxPoint(r.x+r.width+d, r.y+r.height+d));
	}
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
	for (int l = 0; l < pFrame->m_Layers.GetSize(); l++)
	{
		vtLayerPtr lp = pFrame->m_Layers.GetAt(l);
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
	t->GetExtent(rect);
	wxRect sr = WorldToCanvas(rect);
	int sx = sr.width / 3;
	int sy = sr.height / 3;
	int left = sr.x, right = sr.x+sr.width,
		top = sr.y, bottom = sr.y+sr.height;
	int d=3,e=6;

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
	int diff;
	diff = abs(m_ui.m_CurPoint.x - r.x);
	if (diff < eps)
		m_iDragSide |= 1;
	diff = abs(m_ui.m_CurPoint.x - (r.x+r.width));
	if (diff < eps)
		m_iDragSide |= 2;
	diff = abs(m_ui.m_CurPoint.y - r.y);
	if (diff < eps)
		m_iDragSide |= 4;
	diff = abs(m_ui.m_CurPoint.y - (r.y+r.height));
	if (diff < eps)
		m_iDragSide |= 8;

	// if they didn't click near the box, start a new one
	if (!m_iDragSide)
		BeginBox();
}

void BuilderView::DoArea(wxPoint point)	// in canvas coordinates
{
	wxClientDC dc(this);
	PrepareDC(dc);
	DrawArea(&dc);
	if (m_iDragSide & 1)
		GetMainFrame()->m_area.left = ox(point.x);
	if (m_iDragSide & 2)
		GetMainFrame()->m_area.right = ox(point.x);
	if (m_iDragSide & 4)
		GetMainFrame()->m_area.top = oy(point.y);
	if (m_iDragSide & 8)
		GetMainFrame()->m_area.bottom = oy(point.y);
	DrawArea(&dc);
}

void BuilderView::AreaStretch()
{
	wxClientDC dc(this);
	PrepareDC(dc);
	DrawArea(&dc);
	GetMainFrame()->StretchArea();
	DrawArea(&dc);
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
		pRawL->DeselectAll();
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

	FPoint2 center;
	DRECT area;
	pEL->GetExtent(area);
	center.x = (float) (area.left + area.right) / 2;
	center.y = (float) (area.bottom + area.top) / 2;
	ZoomToPoint(center);

	Refresh();
}

/////////////////////////////////////////////////////////////
// Mouse handlers

void BuilderView::OnLeftDown(const wxMouseEvent& event)
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
	CaptureMouse();

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
}

void BuilderView::OnLeftUp(const wxMouseEvent& event)
{
	ReleaseMouse();

	if (!m_bMouseMoved)
		OnLButtonClick(event);

	OnLButtonDragRelease(event);

	// Dispatch for layer-specific handling
	vtLayerPtr pL = GetMainFrame()->GetActiveLayer();
	if (pL)
		pL->OnLeftUp(this, m_ui);

	m_ui.m_bLMouseButton = false;	// left mouse button no longer down
}

void BuilderView::OnLeftDoubleClick(const wxMouseEvent& event)
{
	GetCanvasPosition(event, m_ui.m_DownPoint);
	m_ui.m_CurPoint = m_ui.m_LastPoint = m_ui.m_DownPoint;
	object(m_ui.m_DownPoint, m_ui.m_DownLocation);

	vtLayer *pL = GetMainFrame()->GetActiveLayer();
	if (pL)
		pL->OnLeftDoubleClick(this, m_ui);
}

void BuilderView::OnLButtonClick(const wxMouseEvent& event)
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

void BuilderView::OnLButtonDragRelease(const wxMouseEvent& event)
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

	if (m_ui.mode == LB_LinkEdit)
		OnDragLinkEdit();
}

void BuilderView::OnDragDistance()
{
	DPoint2 p1, p2;
	object(m_ui.m_DownPoint, p1);
	object(m_ui.m_LastPoint, p2);

	DistanceDlg *pDlg = GetMainFrame()->ShowDistanceDlg();
	pDlg->SetPoints(p1, p2);
}

void BuilderView::OnDragLinkEdit()
{
	if (m_ui.m_pEditingRoad != NULL && m_ui.m_iEditingPoint >= 0)
	{
		RefreshRoad(m_ui.m_pEditingRoad);
		DPoint2 p = m_ui.m_pEditingRoad->GetAt(m_ui.m_iEditingPoint);
		p += (m_ui.m_CurLocation - m_ui.m_DownLocation);
		m_ui.m_pEditingRoad->SetAt(m_ui.m_iEditingPoint, p);
		m_ui.m_pEditingRoad->ComputeExtent();
		RefreshRoad(m_ui.m_pEditingRoad);
	}
	m_ui.m_iEditingPoint = -1;
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
		wxString str = wxString::Format("Selected 1 %s (%d total)",
			m_ui.mode == LB_Node ? "Node" : "Road",
			m_ui.mode == LB_Node ? pRL->GetSelectedNodes() : pRL->GetSelectedLinks());
		GetMainFrame()->SetStatusText(str);
	}
	else
	{
		DeselectAll();
		GetMainFrame()->SetStatusText("Deselected all");
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
		vtUtilityLayer *pTL = (vtUtilityLayer *)pL;
		// TODO? single click selection of utility features
	}
	else if (pL->GetType() == LT_RAW)
	{
		vtRawLayer *pRL = (vtRawLayer *)pL;
		// TODO? single click selection of raw features
	}
}

////////////////

void BuilderView::OnMiddleDown(const wxMouseEvent& event)
{
	m_ui.m_bMMouseButton = true;
	m_bMouseMoved = false;	

	// save the point where the user clicked
	m_DownClient = event.GetPosition();

	GetCanvasPosition(event, m_ui.m_DownPoint);
	m_ui.m_CurPoint = m_ui.m_DownPoint;
	CaptureMouse();			// capture mouse

	BeginPan();
}

void BuilderView::OnMiddleUp(const wxMouseEvent& event)
{
	if (m_bPanning)
		EndPan();

	ReleaseMouse();
}

void BuilderView::OnRightDown(const wxMouseEvent& event)
{
	m_ui.m_bRMouseButton = true;
	CaptureMouse();			// capture mouse

	// Dispatch to the layer
	vtLayer *pL = GetMainFrame()->GetActiveLayer();
	if (pL)
		pL->OnRightDown(this, m_ui);
}

void BuilderView::OnRightUp(const wxMouseEvent& event)
{
	m_ui.m_bRMouseButton = false;	//right mouse button no longer down
	ReleaseMouse();

	vtLayer *pL = GetMainFrame()->GetActiveLayer();
	if (!pL)
		return;

	// Dispatch to the layer
	pL->OnRightUp(this, m_ui);

	switch (pL->GetType())
	{
	case LT_STRUCTURE:
		OnRightUpStructure((vtStructureLayer *)pL);
		break;
	}
}

void BuilderView::OnRightUpStructure(vtStructureLayer *pSL) 
{
	pSL->EditBuildingProperties();
}

void BuilderView::OnMouseMove(const wxMouseEvent& event)
{
//	VTLOG("MouseMove\n");

	wxPoint point = event.GetPosition();
	static wxPoint lastpoint;

	if (point == lastpoint)
		return;
	lastpoint = point;

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
			DoArea(m_ui.m_CurPoint);
		if (m_ui.mode == LB_Dist)
		{
			wxClientDC dc(this);
			PrepareDC(dc);
			wxPen pen(*wxBLACK_PEN);
			dc.SetPen(pen);
			dc.SetLogicalFunction(wxINVERT);

			dc.DrawLine(m_ui.m_DownPoint, m_ui.m_LastPoint);
			dc.DrawLine(m_ui.m_DownPoint, m_ui.m_CurPoint);

			OnDragDistance();
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
}

//////////////////
// Keyboard shortcuts

void BuilderView::OnChar(wxKeyEvent& event)
{
	bool ctrl = event.ControlDown();
	int code = event.KeyCode();

	if (code == ' ')
	{
		SetMode(LB_Pan);
		SetCorrectCursor();
	}
	else if (code == WXK_ADD && ctrl)
	{
		SetScale(GetScale() * 1.2);
	}
	else if (code == WXK_SUBTRACT && ctrl)
	{
		SetScale(GetScale() / 1.2);
	}
	else
		vtScaledView::OnChar(event);
}
