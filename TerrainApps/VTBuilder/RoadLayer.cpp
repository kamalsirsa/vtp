//
// RoadLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RoadLayer.h"
#include "NodeDlg.h"
#include "RoadDlg.h"
#include "Frame.h"
#include "Helper.h"

bool vtRoadLayer::m_bDrawNodes = false;
bool vtRoadLayer::m_bShowDirection = false;
bool vtRoadLayer::m_bDrawWidth = false;

//////////////////////////////////////////////////////////////////////

vtRoadLayer::vtRoadLayer() : vtLayer(LT_ROAD)
{
	m_strFilename = "Untitled.rmf";
}

vtRoadLayer::~vtRoadLayer()
{
	int foo = 1;
}

///////////////////////////////////////////////////////////////////////
// Standard layer overrides
//

bool vtRoadLayer::GetExtent(DRECT &rect)
{
	if (NumNodes() == 0 && NumRoads() == 0)
		return false;

	rect = GetMapExtent();
	return true;
}

void vtRoadLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	Draw(pDC, pView, m_bDrawNodes);
}

bool vtRoadLayer::OnSave()
{
	return WriteRMF(m_strFilename);
}

bool vtRoadLayer::OnLoad()
{
	bool success = ReadRMF(m_strFilename, true, true, true);
	if (!success)
		return false;

	// Set visual properties
	for (NodeEdit *pN = GetFirstNode(); pN; pN = pN->GetNext())
	{
		pN->DetermineVisualFromRoads();
	}

	// Pre-process some road attributes
	for (RoadEdit *pR = GetFirstRoad(); pR; pR = pR->GetNext())
	{
		pR->m_fLength = pR->Length();

		//set the bounding box for the road
		pR->ComputeExtent();		

		// clean up road direction info
		if ((pR->m_iFlags & (RF_FORWARD|RF_REVERSE)) == 0)
			pR->m_iFlags |= (RF_FORWARD|RF_REVERSE);
	}
	return true;
}

void vtRoadLayer::GetProjection(vtProjection &proj)
{
	proj = vtRoadMap::GetProjection();
}

bool vtRoadLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_ROAD)
		return false;

	vtRoadLayer *pFrom = (vtRoadLayer *)pL;

	// add nodes to our list
	Node *n = pFrom->GetFirstNode();
	while (n)
	{
		Node *next = n->m_pNext;
		n->m_pNext = m_pFirstNode;
		m_pFirstNode = n;
		n = next;
	}
	// add roads to our list
	Road *r = pFrom->GetFirstRoad();
	while (r)
	{
		Road *next = r->m_pNext;
		r->m_pNext = m_pFirstRoad;
		m_pFirstRoad = r;
		r = next;
	}
	pFrom->m_pFirstRoad = NULL;
	pFrom->m_pFirstNode = NULL;

	ComputeExtents();

	return true;
}

///////////////////////////////////////////////////////////////////////

int vtRoadLayer::GetSelectedNodes()
{
	int count = 0;
	for (NodeEdit *n = GetFirstNode(); n; n = n->GetNext())
		if (n->IsSelected()) count++;
	return count;
}

int vtRoadLayer::GetSelectedRoads()
{
	int count = 0;
	for (RoadEdit *n = GetFirstRoad(); n; n = n->GetNext())
		if (n->IsSelected()) count++;
	return count;
}

void vtRoadLayer::ToggleRoadDirection(RoadEdit *pRoad)
{
	switch (pRoad->m_iFlags & (RF_FORWARD|RF_REVERSE))
	{
		case RF_FORWARD:
			pRoad->m_iFlags &= ~RF_FORWARD;
			pRoad->m_iFlags |= RF_REVERSE;
			break;
		case RF_REVERSE:
			pRoad->m_iFlags |= RF_FORWARD;
			break;
		case (RF_FORWARD|RF_REVERSE):
			pRoad->m_iFlags &= ~RF_REVERSE;
			break;
	}
	SetModified(true);
}

void vtRoadLayer::MoveSelectedNodes(const DPoint2 &offset)
{
	for (NodeEdit *n = GetFirstNode(); n; n=n->GetNext())
	{
		if (n->IsSelected())
			n->Translate(offset);
	}
}

bool vtRoadLayer::ConvertProjection(vtProjection &proj_new)
{
	// Create conversion object
	vtProjection Source;
	GetProjection(Source);
	OCT *trans = OGRCreateCoordinateTransformation(&Source, &proj_new);
	if (!trans)
		return false;		// inconvertible projections

	RoadEdit *r;
	NodeEdit *n;
	for (r = GetFirstRoad(); r; r=r->GetNext())
	{
		for (int i = 0; i < r->GetSize(); i++)
			trans->Transform(1, &(r->GetAt(i).x), &(r->GetAt(i).y));
	}
	for (n = GetFirstNode(); n; n=n->GetNext())
		trans->Transform(1, &(n->m_p.x), &(n->m_p.y));

	delete trans;

	// recompute road extents
	for (r = GetFirstRoad(); r; r=r->GetNext())
		r->ComputeExtent();

	// set the vtRoadMap projection
	m_proj = proj_new;

	m_bValidExtents = false;
	return true;
}

void vtRoadLayer::SetProjection(vtProjection &proj)
{
	m_proj = proj;
}

void vtRoadLayer::Offset(const DPoint2 &p)
{
	for (Road *r = GetFirstRoad(); r; r=r->m_pNext)
	{
		for (int i = 0; i < r->GetSize(); i++)
			r->GetAt(i) += p;
	}
	for (Node *n = GetFirstNode(); n; n=n->m_pNext)
		n->m_p += p;

	// recompute road extents
	for (RoadEdit *r2 = GetFirstRoad(); r2; r2=r2->GetNext())
		r2->ComputeExtent();

	m_bValidExtents = false;
}

bool vtRoadLayer::EditNodeProperties(const DPoint2 &point, float error,
									 DRECT &bound)
{
	NodeEdit* bestNSoFar = NULL;
	float dist = (float)error;
	float result;
	bool NFound = false;

	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		result = curNode->DistanceToPoint(point);
		if (result < dist && result >= 0) {
			bestNSoFar = curNode;
			dist = result;
			NFound = true;
		}
	}

	if (NFound) {
		//wxLogMessage("FOUND A ROAD %i AT DISTANCE %f\n", bestSoFar->m_id, dist);
		DPoint2 p = bestNSoFar->m_p;
		bound.SetRect(p.x-error, p.y+error, p.x+error, p.y-error);
		return bestNSoFar->EditProperties(this);
	}
	return false; 
}

bool vtRoadLayer::EditRoadProperties(const DPoint2 &point, float error,
									 DRECT &bound)
{
	RoadEdit* bestRSoFar = NULL;
	float dist = (float)error;
	float result;
	bool RFound = false;

	DRECT target(point.x-error, point.y+error, point.x+error, point.y-error);

	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
	{
		if (curRoad->WithinExtent(target))
		{
			result = curRoad->DistanceToPoint(point);
			if (result < dist)
			{
				bestRSoFar = curRoad;
				dist = result;
				RFound = true;
			}
		}
	}

	if (RFound)
	{
		bound = bestRSoFar->m_extent;
		return bestRSoFar->EditProperties(this);
	}

	return false; 
}

bool vtRoadLayer::EditNodesProperties()
{
	int count = 0;
	NodeEdit* node;
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (curNode->IsSelected()) {
			count++;
			node = curNode;
		}
	}
	if (count == 0) {
		return false;
	}
	if (count == 1) {
		CNodeDlg dlg(node, this, false);
		bool success = dlg.LoadFromResource(GetMainFrame(), "dialog4");
		return (dlg.ShowModal() == wxID_OK);
	}
	else
	{
		CNodeDlg dlg(GetFirstNode(), this, true);
		bool success = dlg.LoadFromResource(GetMainFrame(), "dialog4");
		return (dlg.ShowModal() == wxID_OK);
	}
}

bool vtRoadLayer::EditRoadsProperties()
{
	int count = 0;
	RoadEdit* road;

	//only bring up dialog is there is a selected road.
	for (RoadEdit* r = GetFirstRoad(); r; r = r->GetNext())
	{
		if (r->IsSelected())
		{
			count++;
			road = r;
		}
	}
	if (count == 0)
		return false;

	//only one road found
	if (count == 1)
	{
		CRoadDlg dlg(road, this, false);
		bool success = dlg.LoadFromResource(GetMainFrame(), "dialog5");
		return (dlg.ShowModal() == wxID_OK);
	}
	else
	{
		CRoadDlg dlg((RoadEdit*)m_pFirstRoad, this, true);
		bool success = dlg.LoadFromResource(GetMainFrame(), "dialog5");
		return (dlg.ShowModal() == wxID_OK);
	}
}

bool vtRoadLayer::SelectArea(const DRECT &box, bool nodemode, bool crossSelect)
{
	bool ret = false;
	int selected;
	if (nodemode)
	{
		selected = SelectNodes(box, true);
		wxString str = wxString::Format("Selected %d nodes", selected);
		if (selected) SetMessageText(str);
		return (selected != 0);
	}
	else
	{
		if (crossSelect)
			selected = CrossSelectRoads(box, true);
		else
			selected = SelectRoads(box, true);

		wxString str = wxString::Format("Selected %d roads", selected);
		if (selected) SetMessageText(str);
		return (selected != 0);
	}
	return ret;
}


