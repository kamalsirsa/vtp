//
// RoadMapEdit.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
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
#include "Frame.h"

#define NODE_RADIUS 5

wxPen RoadPen[10];
wxPen NodePen[6];
static bool g_bInitializedPens = false;


NodeEdit::NodeEdit() : Node()
{
	m_bSelect = false;
}

//
// Equality operator
//
bool NodeEdit::operator==(NodeEdit &ref)
{
	if (! ((*((Node *)this)) == ref))
		return false;

	return (m_iVisual == ref.m_iVisual);
}

void NodeEdit::Copy(NodeEdit* node)
{
	Node::Copy(node);
	m_bSelect = node->m_bSelect;
	m_iVisual = node->m_iVisual;
}

//
// draw a node as a circle
//
bool NodeEdit::Draw(wxDC* pDC, vtScaledView *pView)
{
	pDC->SetLogicalFunction(wxCOPY);
	assert(m_iVisual >= 0 && m_iVisual < 6);
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
bool NodeEdit::EditProperties(vtLayer *pLayer)
{
	CNodeDlg dlg(this, pLayer);
	bool success = dlg.LoadFromResource(GetMainFrame(), "dialog4");
	//popup the dialog
	return (dlg.ShowModal() == wxID_OK);
}

//is target inside the extent
bool NodeEdit::WithinExtent(DRECT target)
{
	return (m_p.x > target.left && m_p.x < target.right && 
			m_p.y > target.bottom && m_p.y < target.top);
}

//is the node in "bound"
bool NodeEdit::InBounds(DRECT bound)
{
	//eliminate easy cases.
	if (m_p.y < bound.bottom || 
		m_p.y > bound.top ||
		m_p.x < bound.left ||
		m_p.x > bound.right) {
		return false;
	}

	//simple correct case:
	if ((m_p.y < bound.top) &&
		(m_p.y > bound.bottom) &&
		(m_p.x < bound.right) &&
		(m_p.x > bound.left)) {
		return true;
	}

	return false;
}

void NodeEdit::Translate(DPoint2 offset)
{
	m_p += offset;

	// also move the endpoint of all the roads that end here
	for (int i = 0; i < m_iRoads; i++)
	{
		Road *pR = m_r[i];
		if (pR->GetNode(0) == this)
			pR->SetAt(0, m_p);
		else
			pR->SetAt(pR->GetSize()-1, m_p);
	}
}

void NodeEdit::DetermineVisualFromRoads()
{
	IntersectionType it;

	int nlights = 0, nstops = 0;

	for (int i = 0; i < m_iRoads; i++)
	{
		it = GetIntersectType(i);
		if (it == IT_LIGHT) nlights++;
		if (it == IT_STOPSIGN) nstops++;
	}
	if (nlights == m_iRoads)
		m_iVisual = VIT_ALLLIGHTS;
	else if (nstops == m_iRoads)
		m_iVisual = VIT_ALLSTOPS;
	else if (nlights > 0)
		m_iVisual = VIT_LIGHTS;
	else if (nstops > 0)
		m_iVisual = VIT_STOPSIGN;
	else
		m_iVisual = VIT_NONE;
}


///////////////////////////////////////////////////////////////////

RoadEdit::RoadEdit() : Road()
{
	m_extent.SetRect(0,0,0,0);
	m_bSelect = false;
	m_bDrawPoints = false;
}

//
// Equality operator
//
bool RoadEdit::operator==(RoadEdit &ref)
{
	if (! ((*((Road *)this)) == ref))
		return false;

	return (m_iPriority == ref.m_iPriority &&
			m_fLength == ref.m_fLength);
}

void RoadEdit::ComputeExtent()
{
	int i, size = GetSize();

	DPoint2 p;
	p = GetAt(0);
	m_extent.SetRect(p.x, p.y, p.x, p.y);
	for (i = 1; i < size; i++)
	{
		m_extent.GrowToContainPoint(GetAt(i));
//		p = GetAt(i);
	}

	// also refresh the parallel left and right road edges
	DPoint2 p2, vec, norm, p3;
	m_fWidth = EstimateWidth();
	float half_width = m_fWidth / 2;	// Warning: assumes meters-based
	for (i = 0; i < size; i++)
	{
		p = GetAt(i);
		if (i < size-1)
		{
			p2 = GetAt(i+1);
			vec = p2 - p;
			norm.x = -vec.y;
			norm.y = vec.x;
			norm.Normalize();
		}
		p3 = p + (norm * half_width);
		m_Left.SetAt(i, p3);
		p3 = p - (norm * half_width);
		m_Right.SetAt(i, p3);
	}
}

bool RoadEdit::WithinExtent(DRECT target)
{
	return (target.left < m_extent.right && target.right > m_extent.left && 
		target.top > m_extent.bottom && target.bottom < m_extent.top);
}

//is extent of the road in "bound"
bool RoadEdit::InBounds(DRECT bound)
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

//is extent of the road in "bound"
bool RoadEdit::PartiallyInBounds(DRECT bound)
{
	//eliminate easy cases.
	if (m_extent.top < bound.bottom || 
		m_extent.bottom > bound.top ||
		m_extent.right < bound.left ||
		m_extent.left > bound.right) {
		return false;
	}

	//simple correct case:
	for (int i = 0; i < GetSize(); i++)
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

#define MAXPOINTS 8000
static wxPoint roadbuf[MAXPOINTS];

bool RoadEdit::Draw(wxDC* pDC, vtScaledView *pView, bool bShowDirection) 
{
	// base road color on type of road
	pDC->SetLogicalFunction(wxCOPY);
	if (m_iHwy != -1)
		pDC->SetPen(RoadPen[0]);
	else
		pDC->SetPen(RoadPen[m_Surface]);

	int c, size = GetSize();
	if (pView->sdx(m_fWidth) < 3)
	{
		for (c = 0; c < size && c < MAXPOINTS; c++)
			pView->screen(GetAt(c), roadbuf[c]);

		pDC->DrawLines(c, roadbuf);
	}
	else
	{
		for (c = 0; c < size && c < MAXPOINTS; c++)
			pView->screen(m_Left.GetAt(c), roadbuf[c]);
		pDC->DrawLines(c, roadbuf);
		for (c = 0; c < size && c < MAXPOINTS; c++)
			pView->screen(m_Right.GetAt(c), roadbuf[c]);
		pDC->DrawLines(c, roadbuf);
	}
	if (m_bSelect)
	{
		pDC->SetLogicalFunction(wxINVERT);
		pDC->SetPen(RoadPen[7]);
		pDC->DrawLines(GetSize(), roadbuf);
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
			p0.x = roadbuf[mid-r].x;
			p0.y = roadbuf[mid-r].y;
			p1.x = roadbuf[mid+r+1].x;
			p1.y = roadbuf[mid+r+1].y;
			diff = p1 - p0;
			r++;
		}
		center = p0 + (diff * 0.5f);

		fw.x = diff.x;
		fw.y = diff.y;
		fw.Normalize();
		side.x = -fw.y;
		side.y = fw.x;
		pDC->SetPen(RoadPen[8]);
		if (m_iFlags & RF_FORWARD)
		{
			pDC->DrawLine(center.x - side.x * 5.0f, center.y - side.y * 5.0f,
						  center.x + fw.x * 6.0f, center.y + fw.y * 6.0f);
			pDC->DrawLine(center.x + fw.x * 6.0f, center.y + fw.y * 6.0f,
						  center.x + side.x * 5.0f, center.y + side.y * 5.0f);
		}
		if (m_iFlags & RF_REVERSE)
		{
			pDC->DrawLine(center.x - side.x * 5.0f, center.y - side.y * 5.0f,
						  center.x - fw.x * 6.0f, center.y - fw.y * 6.0f);
			pDC->DrawLine(center.x - fw.x * 6.0f, center.y - fw.y * 6.0f,
						  center.x + side.x * 5.0f, center.y + side.y * 5.0f);
		}
	}
	if (m_bDrawPoints)
	{
		pDC->SetPen(RoadPen[9]);
		for (c = 0; c < size && c < MAXPOINTS; c++)
		{
			pDC->DrawLine(roadbuf[c].x-3, roadbuf[c].y,
						  roadbuf[c].x+3, roadbuf[c].y);
			pDC->DrawLine(roadbuf[c].x, roadbuf[c].y-3,
						  roadbuf[c].x, roadbuf[c].y+3);
		}
	}
	return true;
}

bool RoadEdit::EditProperties(vtLayer *pLayer)
{
	CRoadDlg dlg(this, pLayer);
	bool success = dlg.LoadFromResource(GetMainFrame(), "dialog5");
	//popup the dialog
	return (dlg.ShowModal() == wxID_OK);
}


////////////////////////////////////////////////////////////////////////////////

RoadMapEdit::RoadMapEdit() : vtRoadMap()
{
	// create Pens for drawing roads
	if (!g_bInitializedPens)
	{
		g_bInitializedPens = true;

		RoadPen[0].SetColour(128,0,0);
		RoadPen[0].SetWidth(2);

		RoadPen[ST_GRAVEL].SetColour(128,128,128);

		RoadPen[ST_TRAIL].SetColour(130,100,70);
		RoadPen[ST_TRAIL].SetStyle(wxDOT);

		RoadPen[ST_2TRACK].SetColour(130,100,70);

		RoadPen[ST_DIRT].SetColour(130,100,70);

		RoadPen[ST_PAVED].SetColour(0,0,0);

		RoadPen[ST_RAILROAD].SetColour(0,0,0);
		RoadPen[ST_RAILROAD].SetStyle(wxSHORT_DASH);

		RoadPen[7].SetColour(255,255,255);	// for selection
		RoadPen[7].SetWidth(3);

		RoadPen[8].SetColour(0,180,0);	// for direction
		RoadPen[8].SetWidth(2);

		RoadPen[9].SetColour(128,0,128);	// for edit crosses

		NodePen[VIT_NONE].SetColour(0,128,255);

		NodePen[VIT_STOPSIGN].SetColour(128,0,0);
		NodePen[VIT_STOPSIGN].SetStyle(wxDOT);

		NodePen[VIT_ALLSTOPS].SetColour(128,0,0);

		NodePen[VIT_LIGHTS].SetColour(0,128,0);
		NodePen[VIT_LIGHTS].SetStyle(wxDOT);

		NodePen[VIT_ALLLIGHTS].SetColour(0,128,0);

		NodePen[5].SetColour(255,255,255);  //for selection
		NodePen[5].SetWidth(3);
	}
}

RoadMapEdit::~RoadMapEdit() 
{
	int foo = 1;
}

//merges the 2 selected nodes
bool RoadMapEdit::Merge2Nodes() {
	NodeEdit *prev = NULL;
	NodeEdit *pNprev = NULL;
	NodeEdit *pN = NULL;
	NodeEdit *pN2 = NULL;
	NodeEdit *curNode = GetFirstNode();
	IPoint2 diff;
	bool removed = false;
	int count = 0;

	while (curNode && count <=2) {
		if (curNode->IsSelected()) {
			count++;
			if (pN == NULL) {
				pN = curNode;
				pNprev = prev;

			} else if (pN2 == NULL) {
				pN2 = curNode;
			}
		}
		prev = curNode;
		curNode = curNode->GetNext();
	}

	if (count == 2) {
		// we've got a pair that need to be merged
		//new point is placed between the 2 original points
		pN2->m_p.x = (pN2->m_p.x + pN->m_p.x) / 2;
		pN2->m_p.y = (pN2->m_p.y + pN->m_p.y) / 2;
		// we're going to remove the "pN" node
		// inform any roads which may have referenced it
		ReplaceNode(pN, pN2);
		// to remove pN, link around it
		if (pNprev)
			pNprev->m_pNext = pN->m_pNext;
		else
			m_pFirstNode = pN->m_pNext;
		delete pN;

		// for the roads that now end in pN2, move their end points
		pN2->EnforceRoadEndpoints();
		removed = true;
	}
	return removed;
}

//
// draw the road network in window, given center and size or drawing area
//
void RoadMapEdit::Draw(wxDC* pDC, vtScaledView *pView, bool bNodes)
{
	if (bNodes)
	{
		for (NodeEdit *curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
			curNode->Draw(pDC, pView);
	}

	for (RoadEdit *curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
		curRoad->Draw(pDC, pView, vtRoadLayer::GetShowDirection());
}

//
// delete all selected roads
//
DRECT *RoadMapEdit::DeleteSelected(int &nBounds)
{
	int n = NumSelectedRoads();

	DRECT* array = new DRECT[n];
	nBounds = n;
	if (nBounds == 0) {
		return NULL;
	}
	n = 0;

	RoadEdit *prevRoad = NULL;
	RoadEdit *tmpRoad;
	RoadEdit *curRoad = GetFirstRoad();

	while (curRoad)
	{
		tmpRoad = curRoad;
		curRoad = curRoad->GetNext();
		if (tmpRoad->IsSelected())
		{
			//delete the road
			array[n] = tmpRoad->m_extent;
			n++;

			if (prevRoad)
				prevRoad->m_pNext = curRoad;
			else
				m_pFirstRoad = curRoad;

			tmpRoad->GetNode(0)->DetachRoad(tmpRoad);
			tmpRoad->GetNode(1)->DetachRoad(tmpRoad);
			delete tmpRoad;
		}
		else
			prevRoad = tmpRoad;
	}
	m_bValidExtents = false;

	return array;
}

bool RoadMapEdit::SelectRoad(DPoint2 point, float error, DRECT &bound)
{
	RoadEdit* road = SelectRoad(point, error, true, false);
	if (road)
	{
		bound = road->m_extent;
//		wxLogMessage("road has %i.  there are %i roads.\n", road->GetSize(), NumRoads());
		return true;
	}
	return false;
}

int RoadMapEdit::SelectRoads(DRECT bound, bool bval)
{
	int found = 0;
	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
	{
		if (curRoad->InBounds(bound)) {
			curRoad->Select(bval);
			found++;
		}
	}
	return found;
}

bool RoadMapEdit::SelectAndExtendRoad(DPoint2 utmCoord, float error, DRECT &bound)
{
	RoadEdit* originalRoad = SelectRoad(utmCoord, error, false, true);
	if (originalRoad == NULL) {
		return false;
	}
	bound = originalRoad->m_extent;
	//extend the given road
	NodeEdit* node;
	RoadEdit* road = originalRoad;
	//run twice.  once in node 0 direction.  once in node 1 direction
	node = (NodeEdit*) originalRoad->GetNode(0);
	bool search;
	for (int i = 0; i < 2; i++) {
		search = true;
		while (search) {
			//ignore if there is only the one road.
			int index = -1;
			int j;
			float bestAngle = PI2f;
			int bestRoadIndex = -1;
			if (node->m_iRoads > 1) {
				node->SortRoadsByAngle();
				//find index for current road
				for (j=0; j < node->m_iRoads; j++) {
					if (road == node->GetRoad(j)) {
						index = j;	
					}
				}
				
				//compare index with all the other roads at the node.
				for (j = 0; j < node->m_iRoads; j++) {
					if (j != index) {
						float newAngle  = node->m_fRoadAngle[j] - (node->m_fRoadAngle[index] + PIf);
						//adjust to value between 180 and -180 degrees
						while (newAngle > PIf) {
							newAngle -= PI2f;
						}
						while (newAngle < -PIf) {
							newAngle += PI2f;
						}
						newAngle = fabsf(newAngle);

						//same highway number
						if (road->m_iHwy > 0 && road->m_iHwy == node->GetRoad(j)->m_iHwy) {
							bestRoadIndex = j;
							bestAngle = 0;
							break;
						}
						if (newAngle < bestAngle) {
							bestAngle = newAngle;
							bestRoadIndex = j;
						}
					}
				}
				//wxLogMessage("best angle:%f, road: %i\n", bestAngle, bestRoadIndex);
				//ignore result if angle is too big
				if (bestAngle > PIf/6 && node->m_iRoads > 2) {
					bestRoadIndex = -1;
				} else if (road->m_iHwy > 0 && road->m_iHwy != node->GetRoad(bestRoadIndex)->m_iHwy) {
					//highway must match with same highway number
					bestRoadIndex = -1;
				} else if (road->m_iHwy < 0 && node->GetRoad(bestRoadIndex)->m_iHwy > 0) {
					//non-highway can't pair with a highway
					bestRoadIndex = -1;
				}
				if (bestRoadIndex != -1) {
					//select the road
					road = node->GetRoad(bestRoadIndex);
					if (node == road->GetNode(0)) {
						node = road->GetNode(1);
					} else {
						node = road->GetNode(0);
					}
					road->Select(true);
					//increase the size of the extent
					if (road->m_extent.left < bound.left) bound.left = road->m_extent.left;
					if (road->m_extent.bottom < bound.bottom) bound.bottom = road->m_extent.bottom;
					if (road->m_extent.right > bound.right) bound.right = road->m_extent.right;
					if (road->m_extent.top > bound.top) bound.top = road->m_extent.top;
					if (road == originalRoad) {
						bestRoadIndex = -1;
					}
				} 
			}
			if (bestRoadIndex == -1) {
				//wxLogMessage("Stop!\n");
				search = false;
			}
		}
		//search in node(1) direction.
		node = (NodeEdit*) originalRoad->GetNode(1);
		road = originalRoad;
	}
	return true;
}

bool RoadMapEdit::SelectHwyNum(int num)
{
	bool found = false;
	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
	{
		if (curRoad->m_iHwy == num) {
			curRoad->Select(true);
			found = true;
		}
	}
	return found;
}

bool RoadMapEdit::CrossSelectRoads(DRECT bound, bool bval)
{
	bool found = false;
	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
	{
		if (curRoad->PartiallyInBounds(bound)) {
			curRoad->Select(bval);
			found = true;
		}
	}
	return found;
}

void RoadMapEdit::InvertSelection()
{
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
		curNode->Select(!curNode->IsSelected());

	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
		curRoad->Select(!curRoad->IsSelected());
}

//
// inverts Selected value of node within error or utmCoord
//
bool RoadMapEdit::SelectNode(DPoint2 utmCoord, float error, DRECT &bound)
{
	NodeEdit* node = SelectNode(utmCoord, error, true, false);
	if (node) {
		bound.left = bound.right = node->m_p.x;
		bound.top = bound.bottom = node->m_p.y;
		return true;
	} else
		return false;
}

//if bval true, select nodes within bound.  otherwise deselect nodes
int RoadMapEdit::SelectNodes(DRECT bound, bool bval)
{
	int found = 0;
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (curNode->InBounds(bound)) {
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

int RoadMapEdit::NumSelectedRoads()
{
	int n = 0;
	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
		if (curRoad->IsSelected())
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
	n += NumSelectedRoads();
	numRegions = n;

	// make an array large enough to hold them all
	DRECT* array = new DRECT[n];

	// fill the array with the element's extents, and deselect them
	n = 0;
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (curNode->IsSelected()) {
			array[n++] = DRECT(curNode->m_p.x, curNode->m_p.y, curNode->m_p.x, curNode->m_p.y);
			curNode->Select(false);
		}
	}
	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
	{
		if (curRoad->IsSelected()) {
			array[n++] = curRoad->m_extent;
			curRoad->Select(false);
		}
	}
	return array;
}

//returns appropriate node at utmCoord within error
//toggle:	toggles the select value
//selectVal: what to assign the select value.
// toggle has precendence over selectVal.
NodeEdit* RoadMapEdit::SelectNode(DPoint2 point, float error, bool toggle, bool selectVal)
{
	NodeEdit* bestSoFar;
	float dist = (float)error;
	float result;
	bool found = false;

	//a backwards rectangle, to provide greater flexibility for finding the node
	DRECT target(point.x-error, point.y+error, point.x+error, point.y-error);
	for (NodeEdit* curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (curNode->WithinExtent(target))
		{
			result = curNode->DistanceToPoint(point);
			if (result < dist) {
				bestSoFar = curNode;
				dist = result;
				found = true;
			}
		}
	}

	if (found)
	{
		if (toggle)
			bestSoFar->ToggleSelect();
		else
			bestSoFar->Select(selectVal);
		return bestSoFar;
	}
	else
		return NULL;
}


RoadEdit *RoadMapEdit::FindRoad(DPoint2 point, float error)
{
	RoadEdit *bestSoFar = NULL;
	float dist = (float)error;
	float result;

	//a backwards rectangle, to provide greater flexibility for finding the road.
	DRECT target(point.x-error, point.y+error, point.x+error, point.y-error);
	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
	{
		if (curRoad->WithinExtent(target)) {
			result = curRoad->DistanceToPoint(point);
			if (result < dist) {
				bestSoFar = curRoad;
				dist = result;
			}
		}
	}
	return bestSoFar;
}

//returns appropriate road at utmCoord within error
//toggle:	toggles the select value
//selectVal: what to assign the select value.
// toggle has precendence over selectVal.
RoadEdit* RoadMapEdit::SelectRoad(DPoint2 point, float error, bool toggle, bool selectVal)
{
	RoadEdit *pPickedRoad = FindRoad(point, error);
	if (pPickedRoad)
	{
		if (toggle)
			pPickedRoad->ToggleSelect();
		else
			pPickedRoad->Select(selectVal);
	}
	return pPickedRoad;
}


NodeEdit* RoadMapEdit::GetNode(DPoint2 point, float error)
{
	return SelectNode(point, error, false, true);
}

RoadEdit* RoadMapEdit::GetRoad(DPoint2 point, float error)
{
	return SelectRoad(point, error, false, true);
}

void RoadMapEdit::DeleteSingleRoad(RoadEdit *pDeleteRoad)
{
	RoadEdit *prev = NULL;
	for (RoadEdit* curRoad = GetFirstRoad(); curRoad; curRoad = curRoad->GetNext())
	{
		if (curRoad == pDeleteRoad)
		{
			if (prev)
				prev->m_pNext = curRoad->GetNext();
			else
				m_pFirstRoad = curRoad->GetNext();
			curRoad->GetNode(0)->DetachRoad(curRoad);
			curRoad->GetNode(1)->DetachRoad(curRoad);
			delete curRoad;
			return;
		}
		prev = curRoad;
	}
}

void RoadMapEdit::ReplaceNode(NodeEdit *pN, NodeEdit *pN2)
{
	bool lights = false;
	IntersectionType type = IT_NONE;
	for (Road *pR = m_pFirstRoad; pR; pR=pR->m_pNext)
	{
		if (pR->GetNode(0) == pN)
		{
			pR->SetNode(0, pN2);
			pN2->AddRoad(pR);
			type = pN->GetIntersectType(pR);
			pN2->SetIntersectType(pR, type);
			if (type == IT_LIGHT) {
				lights = true;
			}
		}
		if (pR->GetNode(1) == pN)
		{
			pR->SetNode(1, pN2);
			pN2->AddRoad(pR);
			type = pN->GetIntersectType(pR);
			pN2->SetIntersectType(pR, type);
			if (type == IT_LIGHT) {
				lights = true;
			}
		}
	}
	if (lights) {
		pN2->AdjustForLights();
	}
}


