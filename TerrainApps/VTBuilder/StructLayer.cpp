//
// StructLayer.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/DLG.h"
#include "vtdata/Fence.h"
#include "vtdata/ElevationGrid.h"
#include "ogrsf_frmts.h"

#include "Frame.h"
#include "StructLayer.h"
#include "ElevLayer.h"
#include "BuilderView.h"
//#include "ScaledView.h"
#include "vtui/BuildingDlg.h"
#include "Helper.h"
#include "ImportStructDlg.h"

wxPen orangePen;
wxPen yellowPen;
wxPen thickPen;
static bool g_bInitializedPens = false;

//////////////////////////////////////////////////////////////////////////

vtStructureLayer::vtStructureLayer() : vtLayer(LT_STRUCTURE)
{
	m_strFilename = _T("Untitled.vtst");

	if (!g_bInitializedPens)
	{
		g_bInitializedPens = true;

		orangePen.SetColour(255,128,0);
		yellowPen.SetColour(255,255,0);

		thickPen.SetColour(255,255,255);
		thickPen.SetWidth(3);
	}
}

bool vtStructureLayer::GetExtent(DRECT &rect)
{
	if (IsEmpty())
		return false;

	GetExtents(rect);

	// expand by 10 meters (TODO: is this correct if units are degrees?)
	rect.left -= 10.0f;
	rect.right += 10.0f;
	rect.bottom -= 10.0f;
	rect.top += 10.0f;

	return true;
}

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

	int i;
	for (i = 0; i < structs; i++)
	{
		// draw each building
		vtStructure *str = GetAt(i);
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
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			DrawBuilding(pDC, pView, bld);

		vtFence *fen = str->GetFence();
		if (fen)
			DrawLinear(pDC, pView, fen);

		vtStructInstance *inst = str->GetInstance();
		if (inst)
		{
			wxPoint origin;
			pView->screen(inst->m_p, origin);

			pDC->DrawLine(origin.x-m_size, origin.y, origin.x+m_size+1, origin.y);
			pDC->DrawLine(origin.x, origin.y-m_size, origin.x, origin.y+m_size+1);
		}
	}
	DrawBuildingHighlight(pDC, pView);
}

void vtStructureLayer::DrawBuildingHighlight(wxDC* pDC, vtScaledView *pView)
{
	if (m_pEditBuilding)
	{
		pDC->SetLogicalFunction(wxINVERT);
		pDC->SetPen(thickPen);

		DLine2 &dl = m_pEditBuilding->GetFootprint(m_iEditLevel);
		int sides = dl.GetSize();
		int j = m_iEditEdge;
		wxPoint p[2];
		pView->screen(dl.GetAt(j), p[0]);
		pView->screen(dl.GetAt((j+1)%sides), p[1]);
		pDC->DrawLines(2, p);
	}
}

#define MAX_SIDES	500
static wxPoint array[MAX_SIDES];

void vtStructureLayer::DrawBuilding(wxDC* pDC, vtScaledView *pView,
									vtBuilding *bld)
{
	DPoint2 corner[4];
	int i, j;

	wxPoint origin;
	pView->screen(bld->GetLocation(), origin);

	// crosshair at building center
	pDC->DrawLine(origin.x-m_size, origin.y, origin.x+m_size+1, origin.y);
	pDC->DrawLine(origin.x, origin.y-m_size, origin.x, origin.y+m_size+1);

	// draw building footprint for all levels
	int levs = bld->GetNumLevels();

	// unless we're XORing, in which case multiple overlapping level would
	// cancel each other out
	if (pDC->GetLogicalFunction() == wxINVERT)
		levs = 1;

	for (i = 0; i < levs; i++)
	{
		DLine2 &dl = bld->GetFootprint(i);
		int sides = dl.GetSize();
		if (sides == 0)
			return;
		for (j = 0; j < sides && j < MAX_SIDES-1; j++)
			pView->screen(dl.GetAt(j), array[j]);
		pView->screen(dl.GetAt(0), array[j++]);

		pDC->DrawLines(j, array);
	}
}

void vtStructureLayer::DrawLinear(wxDC* pDC, vtScaledView *pView, vtFence *fen)
{
	int j;
	DLine2 &pts = fen->GetFencePoints();
	for (j = 0; j < pts.GetSize(); j++)
		pView->screen(pts.GetAt(j), g_screenbuf[j]);
	pDC->DrawLines(j, g_screenbuf);
	for (j = 0; j < pts.GetSize(); j++)
	{
		pDC->DrawLine(g_screenbuf[j].x-2, g_screenbuf[j].y,
			g_screenbuf[j].x+2, g_screenbuf[j].y);
		pDC->DrawLine(g_screenbuf[j].x, g_screenbuf[j].y-2,
			g_screenbuf[j].x, g_screenbuf[j].y+2);
	}
}

bool vtStructureLayer::OnSave()
{
	return WriteXML(m_strFilename.mb_str());
}

bool vtStructureLayer::OnLoad()
{
	return ReadXML(m_strFilename.mb_str());
}

void vtStructureLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtStructureLayer::SetProjection(const vtProjection &proj)
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
	int i, j;
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
			DLine2 line = fen->GetFencePoints();
			for (j = 0; j < line.GetSize(); j++)
			{
				loc = line[j];
				trans->Transform(1, &loc.x, &loc.y);
				line[j] = loc;
			}
		}
		vtStructInstance *inst = str->GetInstance();
		if (inst)
		{
			loc = inst->m_p;
			trans->Transform(1, &loc.x, &loc.y);
			inst->m_p = loc;
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
	// tell the source layer that it has no structures (we have taken them)
	pFrom->SetSize(0);

	return true;
}

void vtStructureLayer::Offset(const DPoint2 &delta)
{
	int npoints = GetSize();
	if (!npoints)
		return;

	int i, j;
	DPoint2 temp;
	for (i = 0; i < npoints; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			bld->Offset(delta);
		vtFence *fen = str->GetFence();
		if (fen)
		{
			DLine2 line = fen->GetFencePoints();
			for (j = 0; j < line.GetSize(); j++)
				line.GetAt(j) += delta;
		}
		vtStructInstance *inst = str->GetInstance();
		if (inst)
			inst->m_p += delta;
	}
}

void vtStructureLayer::GetPropertyText(wxString &strIn)
{
	wxString str;

	int i, size = GetSize();

	str.Printf(_T("Number of structures: %d\n"), size);
	strIn += str;

	int bld = 0, lin = 0, ins = 0;
	for (i = 0; i < size; i++)
	{
		vtStructure *sp = GetAt(i);
		if (sp->GetBuilding()) bld++;
		else if (sp->GetFence()) lin++;
		else if (sp->GetInstance()) ins++;
	}
	str.Printf(_T("\t%d Buildings (procedural)\n"), bld);
	strIn += str;
	str.Printf(_T("\t%d Linear (fences/walls)\n"), lin);
	strIn += str;
	str.Printf(_T("\t%d Instances (imported models)\n"), ins);
	strIn += str;

	str.Printf(_T("Number of selected structures: %d\n"), NumSelected());
	strIn += str;
}

void vtStructureLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	switch (ui.mode)
	{
	case LB_AddLinear:
		if (ui.m_pCurLinear == NULL)
		{
			ui.m_pCurLinear = NewFence();
			ui.m_pCurLinear->SetOptions(GetMainFrame()->m_LSOptions);
			Append(ui.m_pCurLinear);
			ui.m_bRubber = true;
		}
		ui.m_pCurLinear->AddPoint(ui.m_CurLocation);
		pView->Refresh(TRUE);
		break;
	case LB_BldEdit:
		OnLeftDownEditBuilding(pView, ui);
		break;
	case LB_EditLinear:
		OnLeftDownEditLinear(pView, ui);
		// TODO - find nearest point on nearest linear
		break;
	}
}

void vtStructureLayer::OnLeftUp(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_BldEdit && ui.m_bRubber)
	{
		DRECT extent_old, extent_new;
		ui.m_pCurBuilding->GetExtents(extent_old);
		ui.m_EditBuilding.GetExtents(extent_new);
		wxRect screen_old = pView->WorldToWindow(extent_old);
		wxRect screen_new = pView->WorldToWindow(extent_new);
		screen_old.Inflate(1);
		screen_new.Inflate(1);

		pView->Refresh(TRUE, &screen_old);
		pView->Refresh(TRUE, &screen_new);

		// copy back from temp building to real building
		*ui.m_pCurBuilding = ui.m_EditBuilding;
		ui.m_bRubber = false;
		GetMainFrame()->GetActiveLayer()->SetModified(true);
		ui.m_pCurBuilding = NULL;
	}
	if (ui.mode == LB_EditLinear && ui.m_bRubber)
	{
		DRECT extent_old, extent_new;
		ui.m_pCurLinear->GetExtents(extent_old);
		ui.m_EditLinear.GetExtents(extent_new);
		wxRect screen_old = pView->WorldToWindow(extent_old);
		wxRect screen_new = pView->WorldToWindow(extent_new);

		pView->Refresh(TRUE, &screen_old);
		pView->Refresh(TRUE, &screen_new);

		// copy back from temp building to real building
		*ui.m_pCurLinear = ui.m_EditLinear;
		ui.m_bRubber = false;
		GetMainFrame()->GetActiveLayer()->SetModified(true);
		ui.m_pCurLinear = NULL;
	}
}

/*void ResolveClosest(bool &valid1, bool &valid2, bool &valid3,
					double dist1, double dist2, double dist3)
{
	if (valid1 && valid2)
	{
		if (dist1 < dist2)
			valid2 = false;
		else
			valid1 = false;
	}
	if (valid1 && valid3)
	{
		if (dist1 < dist3)
			valid3 = false;
		else
			valid1 = false;
	}
	if (valid2 && valid3)
	{
		if (dist2 < dist3)
			valid3 = false;
		else
			valid2 = false;
	}
}*/

void vtStructureLayer::OnLeftDownEditBuilding(BuilderView *pView, UIContext &ui)
{
	double epsilon = pView->odx(6);  // 6 pixels as world coord

	int building1, building2,  corner;
	double dist1, dist2;

	bool found1 = FindClosestBuildingCenter(ui.m_DownLocation, epsilon, building1, dist1);
	bool found2 = FindClosestBuildingCorner(ui.m_DownLocation, epsilon, building2, corner, dist2);

	if (found1 && found2)
	{
		// which was closer?
		if (dist1 < dist2)
			found2 = false;
		else
			found1 = false;
	}
	if (found1)
	{
		// closest point is a building center
		ui.m_pCurBuilding = GetAt(building1)->GetBuilding();
		ui.m_bDragCenter = true;
	}
	if (found2)
	{
		// closest point is a building corner
		ui.m_pCurBuilding = GetAt(building2)->GetBuilding();
		ui.m_bDragCenter = false;
		ui.m_iCurCorner = corner;
		ui.m_bRotate = ui.m_bControl;
	}
	if (found1 || found2)
	{
		ui.m_bRubber = true;

		// make a copy of the building, to edit and display while dragging
		ui.m_EditBuilding = *ui.m_pCurBuilding;
	}
}

void vtStructureLayer::OnLeftDownEditLinear(BuilderView *pView, UIContext &ui)
{
	double epsilon = pView->odx(6);  // 6 pixels as world coord

	int structure, corner;
	double dist1;

	bool found1 = FindClosestLinearCorner(ui.m_DownLocation, epsilon,
		structure, corner, dist1);

	if (found1)
	{
		// closest point is a building center
		ui.m_pCurLinear = GetAt(structure)->GetFence();
		ui.m_iCurCorner = corner;
		ui.m_bRubber = true;

		// make a copy of the linear, to edit and display while dragging
		ui.m_EditLinear = *ui.m_pCurLinear;
	}
}

void vtStructureLayer::OnRightDown(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_AddLinear && ui.m_pCurLinear != NULL)
	{
		ui.m_pCurLinear->AddPoint(ui.m_CurLocation);
		pView->Refresh(TRUE);
		ui.m_pCurLinear = NULL;
		ui.m_bRubber = false;
	}
}

void vtStructureLayer::OnMouseMove(BuilderView *pView, UIContext &ui)
{
	// create rubber (xor) pen
	wxClientDC dc(pView);
	pView->PrepareDC(dc);
	wxPen pen(*wxBLACK_PEN);
	dc.SetPen(pen);
	dc.SetLogicalFunction(wxINVERT);

	if (ui.m_bLMouseButton && ui.mode == LB_BldEdit && ui.m_bRubber)
	{
		// rubber-band a building
		DrawBuilding(&dc, pView, &ui.m_EditBuilding);

		if (ui.m_bDragCenter)
			UpdateMove(ui);
		else if (ui.m_bRotate)
			UpdateRotate(ui);
		else
			UpdateResizeScale(ui);

		DrawBuilding(&dc, pView, &ui.m_EditBuilding);
	}
	if (ui.mode == LB_AddLinear && ui.m_bRubber)
	{
		wxPoint p1, p2;
		DLine2 &pts = ui.m_pCurLinear->GetFencePoints();
		pView->screen(pts.GetAt(pts.GetSize()-1), p1);
		dc.DrawLine(p1, ui.m_LastPoint);
		dc.DrawLine(p1, ui.m_CurPoint);
	}
	if (ui.mode == LB_EditLinear && ui.m_bRubber)
	{
		// rubber-band a linear
		DrawLinear(&dc, pView, &ui.m_EditLinear);

		ui.m_EditLinear.GetFencePoints().SetAt(ui.m_iCurCorner, ui.m_CurLocation);

		DrawLinear(&dc, pView, &ui.m_EditLinear);
	}
}

void vtStructureLayer::UpdateMove(UIContext &ui)
{
	DPoint2 p;
	DPoint2 moved_by = ui.m_CurLocation - ui.m_DownLocation;

	int i, levs = ui.m_pCurBuilding->GetNumLevels();
	for (i = 0; i < levs; i++)
	{
		DLine2 dl = ui.m_pCurBuilding->GetFootprint(i);
		dl.Add(moved_by);
		ui.m_EditBuilding.SetFootprint(i, dl);
	}
}

void vtStructureLayer::UpdateRotate(UIContext &ui)
{
	DPoint2 origin = ui.m_pCurBuilding->GetLocation();
	DPoint2 original_vector = ui.m_DownLocation - origin;
	double length1 = original_vector.Length();
	double angle1 = atan2(original_vector.y, original_vector.x);

	DPoint2 cur_vector = ui.m_CurLocation - origin;
	double length2 = cur_vector.Length();
	double angle2 = atan2(cur_vector.y, cur_vector.x);

	double angle_diff = angle2 - angle1;

	DPoint2 p;
	int i, j, levs = ui.m_pCurBuilding->GetNumLevels();
	for (i = 0; i < levs; i++)
	{
		DLine2 dl = ui.m_pCurBuilding->GetFootprint(i);
		for (j = 0; j < dl.GetSize(); j++)
		{
			p = dl.GetAt(j);
			p -= origin;
			p.Rotate(angle_diff);
			p += origin;
			dl.SetAt(j, p);
		}
		ui.m_EditBuilding.SetFootprint(i, dl);
	}
}

void vtStructureLayer::UpdateResizeScale(UIContext &ui)
{
	DPoint2 moved_by = ui.m_CurLocation - ui.m_DownLocation;

	if (ui.m_bShift)
		int foo = 1;

	DPoint2 origin = ui.m_pCurBuilding->GetLocation();
	DPoint2 diff1 = ui.m_DownLocation - origin;
	DPoint2 diff2 = ui.m_CurLocation - origin;
	float fScale = diff2.Length() / diff1.Length();

	DPoint2 p;
	if (ui.m_bShift)
	{
		// Scale evenly
		int i, j, levs = ui.m_pCurBuilding->GetNumLevels();
		for (i = 0; i < levs; i++)
		{
			DLine2 dl = ui.m_pCurBuilding->GetFootprint(i);
			for (j = 0; j < dl.GetSize(); j++)
			{
				p = dl.GetAt(j);
				p -= origin;
				p *= fScale;
				p += origin;
				dl.SetAt(j, p);
			}
			ui.m_EditBuilding.SetFootprint(i, dl);
		}
	}
	else
	{
		DLine2 dl = ui.m_pCurBuilding->GetFootprint(0);
		// drag individual corner points
		p = dl.GetAt(ui.m_iCurCorner);
		p += moved_by;
		dl.SetAt(ui.m_iCurCorner, p);
		ui.m_EditBuilding.SetFootprint(0, dl);
	}
}


/////////////////////////////////////////////////////////////////////////////

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

	// for now, assume they will use the dialog to change something about
	// the building (pessimistic)
	SetModified(true);

	BuildingDlg dlg(NULL, -1, _T("Building Properties"), wxDefaultPosition);
	dlg.Setup(this, bld_selected);

	dlg.ShowModal();
	return true;
}

void vtStructureLayer::AddFoundations(vtElevLayer *pEL)
{
	// TODO: ask user if they want concrete, the same material as the
	//	building wall, or what.
	// TODO: ask user exactly how much slope (or depth) to tolerate
	//	without building a foundation.

	vtLevel *pLev, *pNewLev;
	int i, j, pts, built = 0;
	float fElev;

	int selected = NumSelected();
	int size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		if (selected > 0 && !str->IsSelected())
			continue;

		// Get the footprint of the lowest level
		pLev = bld->GetLevel(0);
		DLine2 &foot = pLev->GetFootprint();
		pts = foot.GetSize();

		float fMin = 1E9, fMax = -1E9;
		for (j = 0; j < pts; j++)
		{
			fElev = pEL->GetElevation(foot.GetAt(j));
			if (fElev == INVALID_ELEVATION)
				continue;

			if (fElev < fMin) fMin = fElev;
			if (fElev > fMax) fMax = fElev;
		}
		float fDiff = fMax - fMin;

		// if there's less than 50cm of depth, don't bother building
		// a foundation
		if (fDiff < 0.5f)
			continue;

		// Create and add a foundation level
		pNewLev = new vtLevel();
		pNewLev->m_iStories = 1;
		pNewLev->m_fStoryHeight = fDiff;
		bld->InsertLevel(0, pNewLev);
//		pNewLev->SetFootprint(foot);
		bld->SetFootprint(0, foot);
		pNewLev->SetEdgeMaterial(BMAT_CEMENT);
		pNewLev->SetEdgeColor(RGBi(255, 255, 255));
		built++;
	}
	wxString str;
	str.Printf(_T("Added a foundation level to %d buildings.\n"), built);
	wxMessageBox(str);
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

void vtStructureLayer::DeselectAll()
{
	int i, size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		str->Select(false);
	}
}

int vtStructureLayer::DoBoxSelect(const DRECT &rect, SelectionType st)
{
	int affected = 0;
	bool bWas;

	for (int i = 0; i < GetSize(); i++)
	{
		vtStructure *str = GetAt(i);

		bWas = str->IsSelected();
		if (st == ST_NORMAL)
			str->Select(false);

		if (!str->IsContainedBy(rect))
			continue;

		switch (st)
		{
		case ST_NORMAL:
			str->Select(true);
			affected++;
			break;
		case ST_ADD:
			str->Select(true);
			if (!bWas) affected++;
			break;
		case ST_SUBTRACT:
			str->Select(false);
			if (bWas) affected++;
			break;
		case ST_TOGGLE:
			str->Select(!bWas);
			affected++;
			break;
		}
	}
	return affected;
}


void vtStructureLayer::SetEditedEdge(vtBuilding *bld, int lev, int edge)
{
	vtScaledView *pView = GetMainFrame()->GetView();
	wxClientDC dc(pView);
	pView->PrepareDC(dc);

	DrawBuildingHighlight(&dc, pView);

	vtStructureArray::SetEditedEdge(bld, lev, edge);

	DrawBuildingHighlight(&dc, pView);

	// Trigger re-draw of the building
//	bound = WorldToWindow(world_bounds[n]);
//	IncreaseRect(bound, BOUNDADJUST);
//	Refresh(TRUE, &bound);
}


/////////////////////////////////////////////////////////////////////////////
// Import methods

bool vtStructureLayer::AddElementsFromSHP(const wxString2 &filename,
										  vtProjection &proj, DRECT rect)
{
	ImportStructDlg dlg(NULL, -1, _T("Import Structures"));

	dlg.SetFileName(filename);
	if (dlg.ShowModal() != wxID_OK)
		return false;

	if (dlg.m_iType == 0) dlg.m_opt.type = ST_BUILDING;
	if (dlg.m_iType == 1) dlg.m_opt.type = ST_BUILDING;
	if (dlg.m_iType == 2) dlg.m_opt.type = ST_LINEAR;
	if (dlg.m_iType == 3) dlg.m_opt.type = ST_INSTANCE;

	dlg.m_opt.rect = rect;

	bool success = ReadSHP(filename.mb_str(), dlg.m_opt, progress_callback);
	if (!success)
		return false;

	m_proj = proj;	// Set projection
	return true;
}

void vtStructureLayer::AddElementsFromDLG(vtDLGFile *pDlg)
{
	// set projection
	m_proj = pDlg->GetProjection();

/*	TODO: similar code to import from SDTS-DLG
	NEEDED: an actual sample file containing building data in this format.
*/
}

void vtStructureLayer::AddElementsFromOGR(OGRDataSource *pDatasource,
									 void progress_callback(int))
{
	int i, j, feature_count, count;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRPoint		*pPoint;
	OGRPolygon		*pPolygon;
	vtBuilding		*pBld;
	DPoint2			point;

	// Assume that this data source is a USGS SDTS DLG
	//
	// Iterate through the layers looking for the ones we care about
	//
	int num_layers = pDatasource->GetLayerCount();
	for (i = 0; i < num_layers; i++)
	{
		pLayer = pDatasource->GetLayer(i);
		if (!pLayer)
			continue;

		feature_count = pLayer->GetFeatureCount();
  		pLayer->ResetReading();
		OGRFeatureDefn *defn = pLayer->GetLayerDefn();
		if (!defn)
			continue;

		const char *layer_name = defn->GetName();

		// Nodes
		if (!strcmp(layer_name, "NO01"))
		{
			// only 1 field: RCID - not enough to do anything useful
		}
		if (!strcmp(layer_name, "NE01"))
		{
			// get field indices
			int index_entity = defn->GetFieldIndex("ENTITY_LABEL");

			count = 0;
			while( (pFeature = pLayer->GetNextFeature()) != NULL )
			{
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPoint = (OGRPoint *) pGeom;
				pBld = NewBuilding();

				point.x = pPoint->getX();
				point.y = pPoint->getY();
				pBld->SetLocation(point);
				pBld->SetRectangle(point.x, point.y);
				pBld->SetStories(1);

				Append(pBld);

				count++;
			}
		}
		// Lines
		if (!strcmp(layer_name, "LE01"))
		{
			// only 3 field: RCID, SNID, ENID - not enough to do anything useful
		}
		// Areas (buildings, built-up areas, other areas like golf courses)
		if (!strcmp(layer_name, "PC01"))
		{
			// Get the projection (SpatialReference) from this layer
			OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
			if (pSpatialRef)
				m_proj.SetSpatialReference(pSpatialRef);

			// get field indices
			int index_entity = defn->GetFieldIndex("ENTITY_LABEL");

			count = 0;
			while( (pFeature = pLayer->GetNextFeature()) != NULL )
			{
				count++;
				progress_callback(count * 100 / feature_count);

				// Ignore non-entities
				if (!pFeature->IsFieldSet(index_entity))
					continue;

				// The "ENTITY_LABEL" contains the same information as the old
				// DLG classification.  First, try to use this field to guess
				// values such as number of lanes, etc.
				const char *str_entity = pFeature->GetFieldAsString(index_entity);
				int numEntity = atoi(str_entity);
				int iMajorAttr = numEntity / 10000;
				int iMinorAttr = numEntity % 10000;

				int num_stories = 1;
				pBld = NULL;
				switch (iMinorAttr)
				{
				case 123:	// golf course
				case 150:	// built-up area
				case 420:	// cemetary
					break;
				case 402:	// church
				case 403:	// school
				case 405:	// courthouse
				case 406:	// post office
				case 407:	// city/town hall
				case 408:	// hospital
				case 409:	// prison
				case 412:	// customs building
				case 413:	// capitol
				case 414:	// community center
				case 415:	// muesum
				case 418:	// library
					num_stories = 2;
				case 400:	// building (general case)
					pBld = NewBuilding();
					break;
				}
				if (!pBld)
					continue;
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPolygon = (OGRPolygon *) pGeom;

				OGRLinearRing *ring = pPolygon->getExteriorRing();
				int num_points = ring->getNumPoints();

				DLine2 foot;
				foot.SetSize(num_points);
				for (j = 0; j < num_points; j++)
					foot.SetAt(j, DPoint2(ring->getX(j),
						ring->getY(j)));

				pBld->SetFootprint(0, foot);
				pBld->SetCenterFromPoly();
				pBld->SetStories(num_stories);

				Append(pBld);
			}
		}
	}
}


