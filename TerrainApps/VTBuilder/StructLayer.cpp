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

#include "vtdata/DLG.h"
#include "vtdata/Fence.h"
#include "ogrsf_frmts.h"

#include "Frame.h"
#include "StructLayer.h"
#include "BuilderView.h"
//#include "ScaledView.h"
#include "vtui/BuildingDlg.h"
#include "Helper.h"

wxPen orangePen;
wxPen yellowPen;
wxPen thickPen;
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

	int i, j;
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
		{
			DLine2 &pts = fen->GetFencePoints();
			for (j = 0; j < pts.GetSize(); j++)
				pView->screen(pts.GetAt(j), g_screenbuf[j]);
			pDC->DrawLines(j, g_screenbuf);
		}
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

	str.Printf("Number of structures: %d\n", size);
	strIn += str;

	int bld = 0, lin = 0, ins = 0;
	for (i = 0; i < size; i++)
	{
		vtStructure *sp = GetAt(i);
		if (sp->GetBuilding()) bld++;
		else if (sp->GetFence()) lin++;
		else if (sp->GetInstance()) ins++;
	}
	str.Printf("\t%d Buildings (procedural)\n", bld);
	strIn += str;
	str.Printf("\t%d Linear (fences/walls)\n", lin);
	strIn += str;
	str.Printf("\t%d Instances (imported models)\n", ins);
	strIn += str;

	str.Printf("Number of selected structures: %d\n", NumSelected());
	strIn += str;
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
	dlg.Setup(this, bld_selected);

	dlg.ShowModal();
	return true;
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

bool vtStructureLayer::AddElementsFromSHP(const char *filename,
										  vtProjection &proj, DRECT rect)
{
	wxString choices[3];
	choices[0] = "Buildings (parametric by center or footprint)";
	choices[1] = "Linear Structures (fences)";
	choices[2] = "Instances (external model references)";

	wxSingleChoiceDialog dialog(NULL, "Please indicate the type of structures in this SHP file:",
		"SHP File Import",
		3, (const wxString *)choices);
	if (dialog.ShowModal() != wxID_OK)
		return false;
	vtStructureType type = (vtStructureType) dialog.GetSelection();

	bool bRestrict = false;
	if (!rect.IsEmpty())
	{
		int res = wxMessageBox("Would you like to restrict the imported "
			"features\nto exclude those outside your Area tool?",
			"SHP File Import", wxYES_NO);
		if (res == wxNO)
			rect.Empty();
	}

	int nShapeType = GetSHPType(filename);

	bool bFlip = false;
	if (type == ST_BUILDING && nShapeType == SHPT_POLYGON)
	{
		int res = wxMessageBox("Would you like to flip the direction of "
			"the footprint polygons?", "SHP File Import", wxYES_NO);
		if (res == wxYES)
			bFlip = true;
	}

//	OpenProgressDialog("SHP File Import");
	bool success = ReadSHP(filename, type, rect, bFlip, progress_callback);
//	CloseProgressDialog();
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


