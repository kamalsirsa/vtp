//
// RoadMapEdit.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RoadMapEdit.h"
#include "assert.h"
#include "NodeDlg.h"
#include "RoadDlg.h"
#include "RoadLayer.h"
#include "Builder.h"
#include "BuilderView.h"

#define NODE_RADIUS 5

wxPen LinkPen[12];
wxPen NodePen[VIT_TOTAL];
static bool g_bInitializedPens = false;

#define RP_HIGHWAY 0
// 1 through 7 are SURFT_GRAVEL through SURFT_STONE
#define RP_SELECTION 8
#define RP_DIRECTION 9
#define RP_CROSSES	10
#define RP_CENTERLINE 11

NodeEdit::NodeEdit() : TNode()
{
	m_bSelect = false;
	m_iPathIndex = 0;
	m_pPrevPathNode = NULL;
	m_pPrevPathLink = NULL;
	m_iVisual = VIT_UNKNOWN;
}

//
// Equality operator
//
bool NodeEdit::operator==(NodeEdit &ref)
{
	if (! ((*((TNode *)this)) == ref))
		return false;

	return (m_iVisual == ref.m_iVisual);
}

void NodeEdit::Copy(NodeEdit* node)
{
	TNode::Copy(node);
	m_bSelect = node->m_bSelect;
	m_iVisual = node->m_iVisual;
}

//
// draw a node as a circle
//
bool NodeEdit::Draw(wxDC *pDC, vtScaledView *pView)
{
	pDC->SetLogicalFunction(wxCOPY);
	assert(m_iVisual >= VIT_UNKNOWN && m_iVisual <= VIT_STOPSIGN);
	pDC->SetPen(NodePen[m_iVisual]);
	pDC->SetBrush(wxBrush(wxColour(0,0,0), wxTRANSPARENT));
	wxRect box;

	int x = pView->sx(m_p.x);
	int y = pView->sy(m_p.y);
	box.x = x - NODE_RADIUS;
	box.y = y - NODE_RADIUS;
	box.width = NODE_RADIUS << 1;
	box.height = NODE_RADIUS << 1;
	pDC->DrawEllipse(box);
	if (m_bSelect)
	{
		pDC->SetLogicalFunction(wxINVERT);
		pDC->SetPen(NodePen[5]);
		pDC->DrawEllipse(box);
	}
	return true;
}

//
// bring up dialog box to edit node properties.
//
bool NodeEdit::EditProperties(vtScaledView *pView, vtRoadLayer *pLayer)
{
	NodeDlg dlg(NULL, -1, _("Node Properties"));

	dlg.SetNode(this, pLayer);

	float fScale = pView->GetScale();
	dlg.SetScale(fScale);

	return (dlg.ShowModal() == wxID_OK);
}

void NodeEdit::Translate(const DPoint2 &offset)
{
	m_p += offset;

	// update the endpoints of all the links that meet here
	for (int i = 0; i < NumLinks(); i++)
	{
		TLink *pL = m_connect[i];
		if (pL->GetNode(0) == this)
			pL->SetAt(0, m_p);
		if (pL->GetNode(1) == this)
			pL->SetAt(pL->GetSize()-1, m_p);
	}
}

void NodeEdit::DetermineVisualFromLinks()
{
	IntersectionType it;

	int nlights = 0, nstops = 0;

	for (int i = 0; i < NumLinks(); i++)
	{
		it = GetIntersectType(i);
		if (it == IT_LIGHT) nlights++;
		if (it == IT_STOPSIGN) nstops++;
	}
	if (nlights == NumLinks())
		m_iVisual = VIT_ALLLIGHTS;
	else if (nstops == NumLinks())
		m_iVisual = VIT_ALLSTOPS;
	else if (nlights > 0)
		m_iVisual = VIT_LIGHTS;
	else if (nstops > 0)
		m_iVisual = VIT_STOPSIGN;
	else
		m_iVisual = VIT_NONE;
}


///////////////////////////////////////////////////////////////////

LinkEdit::LinkEdit() : TLink(), Selectable()
{
	m_extent.SetRect(0,0,0,0);
	m_iPriority = 3;
	m_fLength = 0.0f;
	m_bDrawPoints = false;
	m_iHighlightPoint = -1;
	m_bSidesComputed = false;
}

//
// Equality operator
//
bool LinkEdit::operator==(LinkEdit &ref)
{
	if (! ((*((TLink *)this)) == ref))
		return false;

	return (m_iPriority == ref.m_iPriority &&
		m_fLength == ref.m_fLength);
}

void LinkEdit::ComputeExtent()
{
	int size = GetSize();

	DPoint2 p;
	p = GetAt(0);
	m_extent.SetRect(p.x, p.y, p.x, p.y);
	for (int i = 1; i < size; i++)
		m_extent.GrowToContainPoint(GetAt(i));
}

void LinkEdit::ComputeDisplayedLinkWidth(const DPoint2 &ToMeters)
{
	// also refresh the parallel left and right link edges
	EstimateWidth();
	double half_left_width = m_fLeftWidth;
	double half_right_width = m_fRightWidth;

	uint size = GetSize();
	m_LeftOffset.SetSize(size);
	m_RightOffset.SetSize(size);

	DPoint2 norm, prev, offset;

	for (uint i = 0; i < size; i++)
	{
		prev = norm;

		DPoint2 p = GetAt(i);
		if (i < size-1)
		{
			// beware duplicate points, which can cause bad normals; if we
			//  encounter them, just use the existing normal
			const DPoint2 &next = GetAt(i+1);
			if (p != next)
			{
				DPoint2 vec = next - p;
				norm.x = -vec.y;
				norm.y = vec.x;
				norm.Normalize();
			}
		}
		if (i == 0)		// first point
			offset = norm;
		else if (i > 0 && i < size-1)
		{
			// vector which bisects this point is the combination of both normals
			const DPoint2 bisect = (norm + prev).Normalize();

			// compute angle between the vectors
			double dot = prev.Dot(-norm);
			if (dot <= -0.97 || dot >= 0.97)
			{
				// simple case: close enough to colinear
				offset = bisect;
			}
			else
			{
				double angle = acos(dot);

				// factor to widen this corner is proportional to the angle
				double wider = 1.0 / sin(angle / 2);
				offset = bisect * wider;
			}
		}
		else if (i == size-1)	// last point
			offset = prev;
		
		offset.x /= ToMeters.x;			// convert (potentially) to degrees
		offset.y /= ToMeters.y;

		m_LeftOffset[i] = offset * - half_left_width;
		m_RightOffset[i] = offset * half_right_width;
	}
}

DRECT LinkEdit::GetExtentWithWidth()
{
	DRECT ext = m_extent;
	// TODO
	return ext;
}

bool LinkEdit::OverlapsExtent(const DRECT &target)
{
	return (target.left < m_extent.right && target.right > m_extent.left &&
		target.top > m_extent.bottom && target.bottom < m_extent.top);
}

bool LinkEdit::WithinExtent(const DPoint2 &p)
{
	return (p.x > m_extent.left && p.x < m_extent.right &&
			p.y > m_extent.bottom && p.y < m_extent.top);
}

//is extent of the link in "bound"
bool LinkEdit::InBounds(const DRECT &bound)
{
	//eliminate easy cases.
	if (m_extent.top < bound.bottom ||
			m_extent.bottom > bound.top ||
			m_extent.right < bound.left ||
			m_extent.left > bound.right) {
		return false;
	}

	//simple correct case:
	if ((m_extent.top < bound.top) &&
			(m_extent.bottom > bound.bottom) &&
			(m_extent.right < bound.right) &&
			(m_extent.left > bound.left)) {
		return true;
	}

	return false;
}

//is extent of the link in "bound"
bool LinkEdit::PartiallyInBounds(const DRECT &bound)
{
	//eliminate easy cases.
	if (m_extent.top < bound.bottom ||
			m_extent.bottom > bound.top ||
			m_extent.right < bound.left ||
			m_extent.left > bound.right) {
		return false;
	}

	//simple correct case:
	for (uint i = 0; i < GetSize(); i++)
	{
		DPoint2 point = GetAt(i);
		if (point.x > bound.left && point.x < bound.right &&
				point.y > bound.bottom && point.y < bound.top)
		{
			return true;
		}
	}

	return false;
}

bool LinkEdit::Draw(wxDC *pDC, vtScaledView *pView, bool bShowDirection,
	bool bShowWidth)
{
	// base link color on type of link
	pDC->SetLogicalFunction(wxCOPY);
	if (m_iHwy != -1)
		pDC->SetPen(LinkPen[RP_HIGHWAY]);
	else
		pDC->SetPen(LinkPen[m_Surface]);

	const int size = GetSize();
	if (bShowWidth)
		pView->DrawDoubleLine(pDC, *this, m_LeftOffset, m_RightOffset);
	else
		pView->DrawPolyLine(pDC, *this, false);

	if (bShowWidth && m_iLanes > 1)
	{
		pDC->SetPen(LinkPen[RP_CENTERLINE]);
		pView->DrawPolyLine(pDC, *this, false);
	}

	if (m_bSelect)
	{
		pDC->SetLogicalFunction(wxINVERT);
		pDC->SetPen(LinkPen[RP_SELECTION]);
		pView->DrawPolyLine(pDC, *this, false);
	}
	if (bShowDirection)
	{
		int mid = (GetSize() == 2) ? 0 : GetSize() / 2;

		FPoint2 p0, p1, diff, center;
		FPoint2 fw, side;
		int r = 0;

		diff.x = diff.y = 0;
		while (fabs(diff.x) < 2 && fabs(diff.y) < 2)
		{
			p0.x = g_screenbuf[mid-r].x;
			p0.y = g_screenbuf[mid-r].y;
			p1.x = g_screenbuf[mid+r+1].x;
			p1.y = g_screenbuf[mid+r+1].y;
			diff = p1 - p0;
			r++;
		}
		center = p0 + (diff * 0.5f);

		fw.x = diff.x;
		fw.y = diff.y;
		fw.Normalize();
		side.x = -fw.y;
		side.y = fw.x;
		pDC->SetPen(LinkPen[RP_DIRECTION]);
		if (m_iFlags & RF_FORWARD)
		{
			pDC->DrawLine((int) (center.x - side.x * 5.0f),
				(int) (center.y - side.y * 5.0f),
				(int) (center.x + fw.x * 6.0f),
				(int) (center.y + fw.y * 6.0f));
			pDC->DrawLine((int) (center.x + fw.x * 6.0f),
				(int) (center.y + fw.y * 6.0f),
				(int) (center.x + side.x * 5.0f),
				(int) (center.y + side.y * 5.0f));
		}
		if (m_iFlags & RF_REVERSE)
		{
			pDC->DrawLine((int) (center.x - side.x * 5.0f),
				(int) (center.y - side.y * 5.0f),
				(int) (center.x - fw.x * 6.0f),
				(int) (center.y - fw.y * 6.0f));
			pDC->DrawLine((int) (center.x - fw.x * 6.0f),
				(int) (center.y - fw.y * 6.0f),
				(int) (center.x + side.x * 5.0f),
				(int) (center.y + side.y * 5.0f));
		}
	}
	if (m_bDrawPoints)
	{
		// Put a crosshair at every point on the line.
		if (bShowWidth)
			pView->ProjectPolyline(pDC, *this, false);	// Use the centerline

		const int xhair_size = 4;
		pDC->SetPen(LinkPen[RP_CROSSES]);
		for (int c = 0; c < size && c < SCREENBUF_SIZE; c++)
		{
			pDC->DrawLine(g_screenbuf[c].x - xhair_size, g_screenbuf[c].y,
				g_screenbuf[c].x + xhair_size, g_screenbuf[c].y);
			pDC->DrawLine(g_screenbuf[c].x, g_screenbuf[c].y - xhair_size,
				g_screenbuf[c].x, g_screenbuf[c].y + xhair_size);
		}

		// We may highlight a single point
		if (m_iHighlightPoint != -1)
		{
			const int radius = 5;
			pDC->SetPen(LinkPen[RP_CROSSES]);
			pDC->SetBrush(wxBrush(wxColour(0,0,0), wxTRANSPARENT));
			pDC->DrawCircle(g_screenbuf[m_iHighlightPoint], radius);
		}
	}
	return true;
}

bool LinkEdit::EditProperties(vtRoadLayer *pLayer)
{
	RoadDlg dlg(NULL, -1, _("Road Properties"));
	dlg.SetRoad(this, pLayer);
	return (dlg.ShowModal() == wxID_OK);
}

// override because we need to update width when flags change
void LinkEdit::SetFlag(int flag, bool value)
{
	const int flags = RF_SIDEWALK_LEFT | RF_SIDEWALK_RIGHT |
					  RF_PARKING_LEFT | RF_PARKING_RIGHT | RF_MARGIN;
	const int before = m_iFlags & flags;
	TLink::SetFlag(flag, value);
	const int after = m_iFlags & flags;
	if (before != after)
		m_bSidesComputed = false;
}

// call whenever the link's goemetry is changed
void LinkEdit::Dirtied()
{
	ComputeExtent();
	m_bSidesComputed = false;
}

////////////////////////////////////////////////////////////////////////////////

RoadMapEdit::RoadMapEdit() : vtRoadMap()
{
	// create Pens for drawing links
	if (!g_bInitializedPens)
	{
		g_bInitializedPens = true;

		LinkPen[RP_HIGHWAY].SetColour(128,0,0);	// 0: dark red highways
		LinkPen[RP_HIGHWAY].SetWidth(2);

		LinkPen[SURFT_GRAVEL].SetColour(128,128,128);	// 1

		LinkPen[SURFT_TRAIL].SetColour(130,100,70);		// 2
		LinkPen[SURFT_TRAIL].SetStyle(wxDOT);

		LinkPen[SURFT_2TRACK].SetColour(130,100,70);	// 3

		LinkPen[SURFT_DIRT].SetColour(130,100,70);		// 4

		LinkPen[SURFT_PAVED].SetColour(0,0,0);			// 5

		LinkPen[SURFT_RAILROAD].SetColour(0,0,0);		// 6
		LinkPen[SURFT_RAILROAD].SetStyle(wxSHORT_DASH);	// 7

		LinkPen[RP_SELECTION].SetColour(255,255,255);	// for selection
		LinkPen[RP_SELECTION].SetWidth(3);

		LinkPen[RP_DIRECTION].SetColour(0,180,0);	// for direction
		LinkPen[RP_DIRECTION].SetWidth(2);

		LinkPen[RP_CROSSES].SetColour(128,0,128);	// for edit crosses

		LinkPen[RP_CENTERLINE].SetColour(255,255,255);	// for centers of multi-lanes
		LinkPen[RP_CENTERLINE].SetStyle(wxSHORT_DASH);

		NodePen[VIT_UNKNOWN].SetColour(255,0,255);

		NodePen[VIT_NONE].SetColour(0,128,255);

		NodePen[VIT_STOPSIGN].SetColour(128,0,0);
		NodePen[VIT_STOPSIGN].SetStyle(wxDOT);

		NodePen[VIT_ALLSTOPS].SetColour(128,0,0);

		NodePen[VIT_LIGHTS].SetColour(0,128,0);
		NodePen[VIT_LIGHTS].SetStyle(wxDOT);

		NodePen[VIT_ALLLIGHTS].SetColour(0,128,0);

		NodePen[VIT_SELECTED].SetColour(255,255,255);  //for selection
		NodePen[VIT_SELECTED].SetWidth(3);
	}
}

RoadMapEdit::~RoadMapEdit()
{
}

//
// draw the road network in window, given center and size or drawing area
//
void RoadMapEdit::Draw(wxDC *pDC, vtScaledView *pView, bool bNodes)
{
	if (bNodes)
	{
		for (NodeEdit *curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
			curNode->Draw(pDC, pView);
	}

	DPoint2 center;
	DPoint2 ToMeters(1.0, 1.0);	// convert (estimate) width to meters
	bool bGeo = (m_proj.IsGeographic() != 0);

	bool bShowWidth = vtRoadLayer::GetDrawWidth();
	bool bShowDir = vtRoadLayer::GetShowDirection();

	for (LinkEdit *curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (!curLink->m_bSidesComputed)
		{
			if (bGeo)
			{
				curLink->m_extent.GetCenter(center);
				ToMeters.x = EstimateDegreesToMeters(center.y);
				ToMeters.y = METERS_PER_LATITUDE;
			}
			curLink->ComputeDisplayedLinkWidth(ToMeters);
			curLink->m_bSidesComputed = true;
		}
		curLink->Draw(pDC, pView, bShowDir, bShowWidth);
	}
}

//
// delete all selected links
//
DRECT *RoadMapEdit::DeleteSelected(int &nDeleted)
{
	nDeleted = NumSelectedLinks();
	if (nDeleted == 0)
		return NULL;

	DRECT* array = new DRECT[nDeleted];

	LinkEdit *prevLink = NULL;
	LinkEdit *tmpLink;
	LinkEdit *curLink = GetFirstLink();
	TNode *tmpNode;
	int n = 0;

	while (curLink)
	{
		tmpLink = curLink;
		curLink = curLink->GetNext();
		if (tmpLink->IsSelected())
		{
			//delete the link
			array[n] = tmpLink->m_extent;
			n++;

			if (prevLink)
				prevLink->SetNext(curLink);
			else
				m_pFirstLink = curLink;

			tmpNode = tmpLink->GetNode(0);
			if (tmpNode)
				tmpNode->DetachLink(tmpLink);
			tmpNode = tmpLink->GetNode(1);
			if (tmpNode)
				tmpNode->DetachLink(tmpLink);
			delete tmpLink;
		}
		else
			prevLink = tmpLink;
	}
	m_bValidExtents = false;

	return array;
}

bool RoadMapEdit::SelectLink(const DPoint2 &point, float error, DRECT &bound)
{
	LinkEdit *link = FindLink(point, error);
	if (link)
	{
		link->ToggleSelect();
		bound = link->m_extent;
		return true;
	}
	return false;
}

int RoadMapEdit::SelectLinks(const DRECT &bound, bool bval)
{
	int found = 0;
	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink->InBounds(bound)) {
			curLink->Select(bval);
			found++;
		}
	}
	return found;
}

bool RoadMapEdit::SelectAndExtendLink(DPoint2 point, float error, DRECT &bound)
{
	LinkEdit *originalLink = FindLink(point, error);
	if (originalLink == NULL)
		return false;

	originalLink->Select(true);
	bound = originalLink->m_extent;
	//extend the given link
	NodeEdit* node;
	LinkEdit* link = originalLink;
	//run twice.  once in node 0 direction.  once in node 1 direction
	node = (NodeEdit*) originalLink->GetNode(0);
	bool search;
	for (int i = 0; i < 2; i++) {
		search = true;
		while (search) {
			//ignore if there is only the one link.
			int index = -1;
			int j;
			float bestAngle = PI2f;
			int bestLinkIndex = -1;
			if (node->NumLinks() > 1) {
				node->SortLinksByAngle();
				//find index for current link
				for (j=0; j < node->NumLinks(); j++) {
					if (link == node->GetLink(j)) {
						index = j;
					}
				}

				//compare index with all the other links at the node.
				for (j = 0; j < node->NumLinks(); j++) {
					if (j != index) {
						float newAngle  = node->GetLinkAngle(j) - (node->GetLinkAngle(index) + PIf);
						//adjust to value between 180 and -180 degrees
						while (newAngle > PIf) {
							newAngle -= PI2f;
						}
						while (newAngle < -PIf) {
							newAngle += PI2f;
						}
						newAngle = fabsf(newAngle);

						//same highway number
						if (link->m_iHwy > 0 && link->m_iHwy == node->GetLink(j)->m_iHwy) {
							bestLinkIndex = j;
							bestAngle = 0;
							break;
						}
						if (newAngle < bestAngle) {
							bestAngle = newAngle;
							bestLinkIndex = j;
						}
					}
				}
				//wxLogMessage("best angle:%f, link: %i\n", bestAngle, bestLinkIndex);
				//ignore result if angle is too big
				if (bestAngle > PIf/6 && node->NumLinks() > 2) {
					bestLinkIndex = -1;
				} else if (link->m_iHwy > 0 && link->m_iHwy != node->GetLink(bestLinkIndex)->m_iHwy) {
					//highway must match with same highway number
					bestLinkIndex = -1;
				} else if (link->m_iHwy < 0 && node->GetLink(bestLinkIndex)->m_iHwy > 0) {
					//non-highway can't pair with a highway
					bestLinkIndex = -1;
				}
				if (bestLinkIndex != -1) {
					//select the link
					link = node->GetLink(bestLinkIndex);
					if (node == link->GetNode(0))
						node = link->GetNode(1);
					else
						node = link->GetNode(0);

					link->Select(true);
					//increase the size of the extent
					if (link->m_extent.left < bound.left) bound.left = link->m_extent.left;
					if (link->m_extent.bottom < bound.bottom) bound.bottom = link->m_extent.bottom;
					if (link->m_extent.right > bound.right) bound.right = link->m_extent.right;
					if (link->m_extent.top > bound.top) bound.top = link->m_extent.top;
					if (link == originalLink)
						bestLinkIndex = -1;
				}
			}
			if (bestLinkIndex == -1) {
				//wxLogMessage("Stop!\n");
				search = false;
			}
		}
		//search in node(1) direction.
		node = (NodeEdit*) originalLink->GetNode(1);
		link = originalLink;
	}
	return true;
}

bool RoadMapEdit::SelectHwyNum(int num)
{
	bool found = false;
	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink->m_iHwy == num) {
			curLink->Select(true);
			found = true;
		}
	}
	return found;
}

bool RoadMapEdit::CrossSelectLinks(DRECT bound, bool bval)
{
	bool found = false;
	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink->PartiallyInBounds(bound)) {
			curLink->Select(bval);
			found = true;
		}
	}
	return found;
}

void RoadMapEdit::InvertSelection()
{
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
		curNode->Select(!curNode->IsSelected());

	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
		curLink->Select(!curLink->IsSelected());
}

//
// Inverts selected value of node within epsilon of point
//
bool RoadMapEdit::SelectNode(const DPoint2 &point, float epsilon, DRECT &bound)
{
	NodeEdit *node = (NodeEdit *) FindNodeAtPoint(point, epsilon);
	if (node)
	{
		node->ToggleSelect();
		bound.left = bound.right = node->Pos().x;
		bound.top = bound.bottom = node->Pos().y;
		return true;
	}
	else
		return false;
}

//if bval true, select nodes within bound.  otherwise deselect nodes
int RoadMapEdit::SelectNodes(DRECT bound, bool bval)
{
	int found = 0;
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (bound.ContainsPoint(curNode->Pos()))
		{
			curNode->Select(bval);
			found++;
		}
	}
	return found;
}

int RoadMapEdit::NumSelectedNodes()
{
	int n = 0;
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
		if (curNode->IsSelected())
			n++;
	return n;
}

int RoadMapEdit::NumSelectedLinks()
{
	int n = 0;
	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
		if (curLink->IsSelected())
			n++;
	return n;
}

//
// caller is responsible for deleting the array returned!
//
DRECT *RoadMapEdit::DeSelectAll(int &numRegions)
{
	// count the number of regions (number of selected elements)
	int n = 0;
	n += NumSelectedNodes();
	n += NumSelectedLinks();
	numRegions = n;

	// make an array large enough to hold them all
	DRECT* array = new DRECT[n];

	// fill the array with the element's extents, and deselect them
	n = 0;
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (curNode->IsSelected()) {
			array[n++] = DRECT(curNode->Pos().x, curNode->Pos().y,
							   curNode->Pos().x, curNode->Pos().y);
			curNode->Select(false);
		}
	}
	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink->IsSelected()) {
			array[n++] = curLink->m_extent;
			curLink->Select(false);
		}
	}
	return array;
}


LinkEdit *RoadMapEdit::FindLink(const DPoint2 &point, float error)
{
	LinkEdit *bestSoFar = NULL;
	double dist = error;
	double b;

	// A buffer rectangle, to make it easier to click a link.
	DRECT target(point.x-error, point.y+error, point.x+error, point.y-error);

	for (LinkEdit *curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink->OverlapsExtent(target))
		{
			b = curLink->DistanceToPoint(point);
			if (b < dist)
			{
				bestSoFar = curLink;
				dist = b;
			}
		}
	}
	return bestSoFar;
}

void RoadMapEdit::DeleteSingleLink(LinkEdit *pDeleteLink)
{
	LinkEdit *prev = NULL;
	for (LinkEdit *curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink == pDeleteLink)
		{
			if (prev)
				prev->SetNext(curLink->GetNext());
			else
				m_pFirstLink = curLink->GetNext();
			curLink->GetNode(0)->DetachLink(curLink);
			curLink->GetNode(1)->DetachLink(curLink);
			delete curLink;
			return;
		}
		prev = curLink;
	}
}

void RoadMapEdit::ReplaceNode(NodeEdit *pN, NodeEdit *pN2)
{
	bool lights = false;

	for (int i = 0; i < pN->NumLinks(); i++)
	{
		TLink *link = pN->GetLink(i);

		IntersectionType itype = link->GetIntersectionType(pN);
		if (itype == IT_LIGHT)
			lights = true;

		if (link->GetNode(0) == pN)
			link->SetNode(0, pN2);
		if (link->GetNode(1) == pN)
			link->SetNode(1, pN2);

		int iNewLinkNum = pN2->AddLink(link);
		pN2->SetIntersectType(iNewLinkNum, itype);
	}
	while (TLink *pL = pN->GetLink(0))
		pN->DetachLink(pL);

	if (lights)
		pN2->AdjustForLights();
}

class LinkEdit *NodeEdit::GetLink(int n)
{
	if (n >= 0 && n < NumLinks())	// safety check
		return (LinkEdit *) m_connect[n];
	else
		return NULL;
}

LinkEdit *RoadMapEdit::AddRoadSegment(OGRLineString *pLineString)
{
	// Road: implicit nodes at start and end
	LinkEdit *link = AddNewLink();
	int num_points = pLineString->getNumPoints();
	for (int j = 0; j < num_points; j++)
	{
		link->Append(DPoint2(pLineString->getX(j),	pLineString->getY(j)));
	}
	TNode *n1 = AddNewNode();
	n1->SetPos(pLineString->getX(0), pLineString->getY(0));

	TNode *n2 = AddNewNode();
	n2->SetPos(pLineString->getX(num_points-1), pLineString->getY(num_points-1));

	link->ConnectNodes(n1, n2);

	//set bounding box for the link
	link->Dirtied();

	return link;
}

/**
 * Split a link into two separate links.
 *
 * \param link The link to split.
 * \param index The place to split it (from 0 to number of points in this link)
 * \param node The node to use at the place we're splitting.
 * \param link1, line2 Will be set to the two links that are produced.
 */
void RoadMapEdit::SplitLinkAtIndex(LinkEdit *link, int index, NodeEdit *node,
	LinkEdit **plink1, LinkEdit **plink2)
{
	// Split by making two new links.
	TNode *old_node0 = link->GetNode(0);
	TNode *old_node1 = link->GetNode(1);

	LinkEdit *link1 = AddNewLink();
	link1->CopyAttributesFrom(link);
	for (int j = 0; j <= index; j++)
		link1->Append(link->GetAt(j));
	link1->ConnectNodes(old_node0, node);
	link1->Dirtied();

	LinkEdit *link2 = AddNewLink();
	link2->CopyAttributesFrom(link);
	for (int j = index; j < (int) link->GetSize(); j++)
		link2->Append(link->GetAt(j));
	link2->ConnectNodes(node, old_node1);
	link2->Dirtied();

	// Inform caller
	if (plink1 != NULL)
		*plink1 = link1;
	if (plink2 != NULL)
		*plink2 = link2;

	// Remove and delete the now-split link
	DetachLink(link);
	RemoveLink(link);
}


