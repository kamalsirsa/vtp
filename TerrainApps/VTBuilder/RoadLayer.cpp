//
// RoadLayer.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
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
#include "BuilderView.h"

bool vtRoadLayer::m_bDrawNodes = false;
bool vtRoadLayer::m_bShowDirection = false;
bool vtRoadLayer::m_bDrawWidth = false;

//////////////////////////////////////////////////////////////////////

vtRoadLayer::vtRoadLayer() : vtLayer(LT_ROAD)
{
	m_strFilename = _T("Untitled.rmf");
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
	if (NumNodes() == 0 && NumLinks() == 0)
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
	return WriteRMF(m_strFilename.mb_str());
}

bool vtRoadLayer::OnLoad()
{
	bool success = ReadRMF(m_strFilename.mb_str(), true, true, true);
	if (!success)
		return false;

	// Set visual properties
	for (NodeEdit *pN = GetFirstNode(); pN; pN = pN->GetNext())
	{
		pN->DetermineVisualFromLinks();
	}

	// Pre-process some road attributes
	for (LinkEdit *pR = GetFirstLink(); pR; pR = pR->GetNext())
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
	Link *r = pFrom->GetFirstLink();
	while (r)
	{
		Link *next = r->m_pNext;
		r->m_pNext = m_pFirstLink;
		m_pFirstLink = r;
		r = next;
	}
	pFrom->m_pFirstLink = NULL;
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

int vtRoadLayer::GetSelectedLinks()
{
	int count = 0;
	for (LinkEdit *n = GetFirstLink(); n; n = n->GetNext())
		if (n->IsSelected()) count++;
	return count;
}

void vtRoadLayer::ToggleLinkDirection(LinkEdit *pLink)
{
	switch (pLink->m_iFlags & (RF_FORWARD|RF_REVERSE))
	{
		case RF_FORWARD:
			pLink->m_iFlags &= ~RF_FORWARD;
			pLink->m_iFlags |= RF_REVERSE;
			break;
		case RF_REVERSE:
			pLink->m_iFlags |= RF_FORWARD;
			break;
		case (RF_FORWARD|RF_REVERSE):
			pLink->m_iFlags &= ~RF_REVERSE;
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

	LinkEdit *r;
	NodeEdit *n;
	for (r = GetFirstLink(); r; r=r->GetNext())
	{
		for (int i = 0; i < r->GetSize(); i++)
			trans->Transform(1, &(r->GetAt(i).x), &(r->GetAt(i).y));
	}
	for (n = GetFirstNode(); n; n=n->GetNext())
		trans->Transform(1, &(n->m_p.x), &(n->m_p.y));

	delete trans;

	// recompute road extents
	for (r = GetFirstLink(); r; r=r->GetNext())
		r->ComputeExtent();

	// set the vtRoadMap projection
	m_proj = proj_new;

	m_bValidExtents = false;
	return true;
}

void vtRoadLayer::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

void vtRoadLayer::Offset(const DPoint2 &p)
{
	for (LinkEdit *link = GetFirstLink(); link; link=link->GetNext())
	{
		for (int i = 0; i < link->GetSize(); i++)
		{
			link->GetAt(i) += p;
		}
		link->m_bSidesComputed = false;
	}
	for (Node *n = GetFirstNode(); n; n=n->m_pNext)
		n->m_p += p;

	// recompute road extents
	for (LinkEdit *r2 = GetFirstLink(); r2; r2=r2->GetNext())
		r2->ComputeExtent();

	m_bValidExtents = false;
}

void vtRoadLayer::GetPropertyText(wxString &strIn)
{
	strIn += _T("Network of links.\n");

	wxString str;
	str.Printf(_T("Nodes: %d, selected: %d\n"), NumNodes(), NumSelectedNodes());
	strIn += str;
	str.Printf(_T("Links: %d, selected: %d\n"), NumLinks(), NumSelectedLinks());
	strIn += str;
}

void vtRoadLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_LinkEdit && ui.m_pEditingRoad)
	{
		double closest = 1E8;
		int closest_i;
		for (int i = 0; i < ui.m_pEditingRoad->GetSize(); i++)
		{
			DPoint2 diff = ui.m_DownLocation - ui.m_pEditingRoad->GetAt(i);
			double dist = diff.Length();
			if (dist < closest)
			{
				closest = dist;
				closest_i = i;
			}
		}
		int pixels = pView->sdx(closest);
		if (pixels < 8)
		{
			// begin dragging point
			ui.m_iEditingPoint = closest_i;
			return;
		}
		else
			ui.m_iEditingPoint = -1;
	}
	if (ui.mode == LB_Dir)
	{
		LinkEdit *pLink = FindLink(ui.m_DownLocation, pView->odx(5));
		if (pLink)
		{
			ToggleLinkDirection(pLink);
			pView->RefreshRoad(pLink);
		}
	}
	if (ui.mode == LB_LinkEdit)
	{
		// see if there is a road or node at m_DownPoint
		float error = pView->odx(5);
		bool redraw = false;

		LinkEdit *pRoad = FindLink(ui.m_DownLocation, error);
		if (pRoad != ui.m_pEditingRoad)
		{
			if (ui.m_pEditingRoad)
			{
				pView->RefreshRoad(ui.m_pEditingRoad);
				ui.m_pEditingRoad->m_bDrawPoints = false;
			}
			ui.m_pEditingRoad = pRoad;
			if (ui.m_pEditingRoad)
			{
				pView->RefreshRoad(ui.m_pEditingRoad);
				ui.m_pEditingRoad->m_bDrawPoints = true;
			}
		}
	}
	if (ui.mode == LB_LinkExtend)
	{
		pView->OnLButtonClickElement(this);
	}
}

void vtRoadLayer::OnRightUp(BuilderView *pView, UIContext &ui)
{
	//if we are not clicked close to a single item, edit all selected items.
	bool status;
	if (ui.mode == LB_Node)
		status = EditNodesProperties();
	else
		status = EditLinksProperties();
	if (status)
	{
		SetModified(true);
		pView->Refresh();
		GetMainFrame()->RefreshTreeStatus();
	}
}

void vtRoadLayer::OnLeftDoubleClick(BuilderView *pView, UIContext &ui)
{
	DRECT world_bound, bound2;

	// epsilon is how close to the link/node can we be off by?
	float epsilon = pView->odx(5);
	bool bRefresh = false;

	if (ui.mode == LB_Node)
	{
		SelectNode(ui.m_DownLocation, epsilon, bound2);
		EditNodeProperties(ui.m_DownLocation, epsilon, world_bound);
		bRefresh = true;
	}
	else if (ui.mode == LB_Link)
	{
		SelectLink(ui.m_DownLocation, epsilon, bound2);
		EditLinkProperties(ui.m_DownLocation, epsilon, world_bound);
		bRefresh = true;
	}
	if (bRefresh)
	{
		wxRect screen_bound = pView->WorldToWindow(world_bound);
		IncreaseRect(screen_bound, 5);
		pView->Refresh(TRUE, &screen_bound);
	}
}

bool vtRoadLayer::EditNodeProperties(const DPoint2 &point, float epsilon,
									 DRECT &bound)
{
	NodeEdit *node = (NodeEdit *) FindNodeAtPoint(point, epsilon);
	if (node)
	{
		DPoint2 p = node->m_p;
		bound.SetRect(p.x-epsilon, p.y+epsilon, p.x+epsilon, p.y-epsilon);
		return node->EditProperties(this);
	}
	return false; 
}

bool vtRoadLayer::EditLinkProperties(const DPoint2 &point, float error,
									 DRECT &bound)
{
	LinkEdit* bestRSoFar = NULL;
	float dist = (float)error;
	float result;
	bool RFound = false;

	DRECT target(point.x-error, point.y+error, point.x+error, point.y-error);

	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink->WithinExtent(target))
		{
			result = curLink->DistanceToPoint(point);
			if (result < dist)
			{
				bestRSoFar = curLink;
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

	for (NodeEdit* n = GetFirstNode(); n; n = n->GetNext())
	{
		if (!n->IsSelected())
			continue;
		count++;
		node = n;
	}
	if (count == 0)
		return false;

	NodeDlg dlg(NULL, -1, _T("Node Properties"));
	if (count == 1)
		dlg.SetNode(node, this);
	else
		dlg.SetNode(NULL, this);
	return (dlg.ShowModal() == wxID_OK);
}

bool vtRoadLayer::EditLinksProperties()
{
	int count = 0;
	LinkEdit* link;

	//only bring up dialog is there is a selected road.
	for (LinkEdit* r = GetFirstLink(); r; r = r->GetNext())
	{
		if (!r->IsSelected())
			continue;
		count++;
		link = r;
	}
	if (count == 0)
		return false;

	RoadDlg dlg(NULL, -1, _T("Link Properties"));
	if (count == 1)
		dlg.SetRoad(link, this);	//only one road found
	else
		dlg.SetRoad(NULL, this);
	return (dlg.ShowModal() == wxID_OK);
}

bool vtRoadLayer::SelectArea(const DRECT &box, bool nodemode, bool crossSelect)
{
	bool ret = false;
	int selected;
	if (nodemode)
	{
		selected = SelectNodes(box, true);
		wxString str = wxString::Format(_T("Selected %d node%s"), selected,
			selected == 1 ? "" : "s");
		if (selected) SetMessageText(str);
		return (selected != 0);
	}
	else
	{
		if (crossSelect)
			selected = CrossSelectLinks(box, true);
		else
			selected = SelectLinks(box, true);

		wxString str = wxString::Format(_T("Selected %d road%s"), selected,
			selected == 1 ? "" : "s");
		if (selected) SetMessageText(str);
		return (selected != 0);
	}
	return ret;
}


