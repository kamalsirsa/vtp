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
	for (Link *r = GetFirstLink(); r; r=r->m_pNext)
	{
		for (int i = 0; i < r->GetSize(); i++)
			r->GetAt(i) += p;
	}
	for (Node *n = GetFirstNode(); n; n=n->m_pNext)
		n->m_p += p;

	// recompute road extents
	for (LinkEdit *r2 = GetFirstLink(); r2; r2=r2->GetNext())
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

	NodeDlg dlg(NULL, -1, "Node Properties");
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

	RoadDlg dlg(NULL, -1, "Link Properties");
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
		wxString str = wxString::Format("Selected %d node%s", selected,
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

		wxString str = wxString::Format("Selected %d road%s", selected,
			selected == 1 ? "" : "s");
		if (selected) SetMessageText(str);
		return (selected != 0);
	}
	return ret;
}


