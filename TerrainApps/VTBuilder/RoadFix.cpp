//
// RoadFix.cpp
//
// contains methods of RoadMapEdit used for fixing and
//  cleaning the roadmap topology
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"

#include "RoadMapEdit.h"
#include "assert.h"


//helper
float fmin(float a, float b, float c, float d)
{
	if (a < b && a < c && a < d)
		return a;
	if (b < c && b < d)
		return b;
	if (c < d)
		return c;
	return d;
}

//makes sure road end points same as node point
void NodeEdit::EnforceLinkEndpoints()
{
	//for the roads that now end in pN2, move it's end point as well.
	for (int k = 0; k < m_iLinks; k++)
	{
		TLink *r = GetLink(k);
		if (r->GetNode(0) == this)
			r->SetAt(0, m_p);
		else if (r->GetNode(1) == this)
			r->SetAt(r->GetSize() - 1, m_p);
		else
			assert(0);	// bad case!

		((LinkEdit*)r)->m_fLength = r->Length();
	}
}


// merge nodes less than 8m apart
#define TOLERANCE_METERS (8.0f)
#define TOLERANCE_DEGREES (TOLERANCE_METERS/110000)

//
// Since the original data is scattered over many source files,
// any road which crosses a DLG file boundary will be split
// by two nodes, one on each edge of the two files.
//
// This routine will merge any two nodes which are sufficiently
// close together.
//
// Warning: some degerate roads may result.
//
// Return the number removed.
//
int RoadMapEdit::MergeRedundantNodes(bool bDegrees, bool progress_callback(int))
{
	NodeEdit *prev = NULL, *next;
	DPoint2 diff;
	int removed = 0;

	int nodes = NumNodes();
	float total = (float)nodes * nodes / 2;
	int count1 = 0, count = 0, count_tick, count_last_tick = 0;
	float tick_size = (float)total / 100;
	double tolerance, tolerance_squared;

	if (bDegrees)
		tolerance = TOLERANCE_DEGREES;
	else
		tolerance = TOLERANCE_METERS;
	tolerance_squared = tolerance * tolerance;

	NodeEdit *pN, *pN2;

	for (pN = GetFirstNode(); pN && pN->m_pNext; pN = next)
	{
		count1++;
		next = pN->GetNext();
		bool remove = false;
		for (pN2 = next; pN2; pN2 = pN2->GetNext())
		{
			diff = pN2->m_p - pN->m_p;
			if (diff.LengthSquared() < tolerance_squared)
			{
				remove = true;
				break;
			}
		}
		count += (nodes - count1);
		count_tick = (int) (count / tick_size);
		if (count_tick > count_last_tick)
		{
			count_last_tick = count_tick;
			progress_callback(count_tick);
		}
		if (remove)
		{
			// we've got a pair that need to be merged
			//new point is placed between the 2 original points
			pN2->m_p.x = (pN2->m_p.x + pN->m_p.x) / 2;
			pN2->m_p.y = (pN2->m_p.y + pN->m_p.y) / 2;

			// we're going to remove the "pN" node
			// inform any roads which may have referenced it
			ReplaceNode(pN, pN2);

			// to remove pN, link around it
			if (prev)
				prev->m_pNext = next;
			else
				m_pFirstNode = next;
			delete pN;

			// for the roads that now end in pN2, move their end points
			pN2->EnforceLinkEndpoints();
			removed++;
		}
		else
			prev = pN;
	}
	VTLOG(" Removed %i nodes\n", removed);
	return removed;
}


//
// Remove links which start and end at the same node, yet don't
// have enough points (less than 3) to be a valid loop
//
// There are many other ways a link can be degenerate, but we
// don't check for them (yet).
//
// Returns the number of links removed.
//
int RoadMapEdit::RemoveDegenerateLinks()
{
	int count = 0;

	LinkEdit *prevR = NULL, *nextR;

	for (LinkEdit *pR = GetFirstLink(); pR; pR = nextR)
	{
		nextR = pR->GetNext();
		if (pR->GetNode(0) == pR->GetNode(1) && pR->GetSize() < 3)
		{
			count++;
			// remove it
			if (prevR)
				prevR->m_pNext = nextR;
			else
				m_pFirstLink = nextR;

			// notify the nodes that the road is gone
			pR->GetNode(0)->DetachLink(pR);
			pR->GetNode(1)->DetachLink(pR);

			delete pR;
		}
		else
			prevR = pR;
	}
	VTLOG(" Removed %i degenerate roads.\n", count);
	return count;
}

/*RemoveUnnecessaryNodes
  eliminates unnecessary nodes:
	-has 2 roads intersecting it
	-both roads are the same type
	-intersection is uncontrolled
*/
int RoadMapEdit::RemoveUnnecessaryNodes()
{
	TLink *aLink;
	TLink *bLink;
	TNode *curNode = m_pFirstNode;
	TNode *prevNode = NULL;
	int count = 0;
	int total = NumNodes();
	while (curNode != NULL)
	{
		if (curNode->m_iLinks == 2 && !curNode->IsControlled())
		{
			aLink = curNode->GetLink(0);
			bLink = curNode->GetLink(1);
			//see if the 2 roads have equivalent characteristics.
			//and the roads are NOT the same!
			if (aLink != bLink &&
				aLink->m_fWidth == bLink->m_fWidth &&
				aLink->m_iLanes == bLink->m_iLanes &&
				aLink->m_Surface == bLink->m_Surface &&
				aLink->m_iHwy == bLink->m_iHwy &&
				aLink->m_iFlags == bLink->m_iFlags)
			{
				
				//merge the 2 roads into one.
				LinkEdit* newLink = new LinkEdit();
				//make the new road the same as the old roads...
				newLink->m_fWidth = bLink->m_fWidth;
				newLink->m_iLanes = bLink->m_iLanes;
				newLink->m_Surface = bLink->m_Surface;
				newLink->m_iHwy = bLink->m_iHwy;
				newLink->m_iFlags = bLink->m_iFlags;
				newLink->SetSize(aLink->GetSize() + bLink->GetSize() - 1);  //subtract one for overlapping middle point

				//extract road coordinates
				//where/how do we connect road a to road b?
				DPoint2 diff1, diff2, diff3, diff4;
				double dist, dist1, dist2, dist3, dist4;

				//find distance between endpoints
				diff1 = aLink->GetAt(0) - bLink->GetAt(0);
				dist1 = diff1.Length();
				diff2 = aLink->GetAt(0) - bLink->GetAt(bLink->GetSize()-1);
				dist2 = diff2.Length();
				diff3 = aLink->GetAt(aLink->GetSize()-1) - bLink->GetAt(0);
				dist3 = diff3.Length();
				diff4 = aLink->GetAt(aLink->GetSize()-1) - bLink->GetAt(bLink->GetSize()-1);
				dist4 = diff4.Length();

				//shortest distance is connection point
				dist = fmin(dist1, dist2, dist3, dist4);
				
				int i;
				int aFirst=0, aLast=0, bFirst=0, bLast=0, aStep=0, bStep=0;
				if (dist == dist1) {
					//connection: aLink->m_p[0] - bLink->m_p[0];
					aFirst = aLink->GetSize()-1;
					aLast = 1;
					aStep = -1;
					bFirst = 0;
					bLast = bLink->GetSize()-1;
					bStep = 1;
				} else if (dist == dist2) {
					//connection: aLink->m_p[0] - bLink->m_p[bLink->GetSize()-1];
					aFirst = aLink->GetSize()-1;
					aLast = 1;
					aStep = -1;
					bFirst = bLink->GetSize()-1;
					bLast = 0;
					bStep = -1;
				} else if (dist == dist3) {
					//connection: m_p[aLink->GetSize()-1] - bLink->m_p[0];
					aFirst = 0;
					aLast = aLink->GetSize()-2;
					aStep = 1;
					bFirst = 0;
					bLast = bLink->GetSize()-1;
					bStep = 1;
				} else if (dist == dist4) {
					//connection: m_p[aLink->GetSize()-1] - bLink->m_p[bLink->GetSize()-1];
					aFirst = 0;
					aLast = aLink->GetSize()-2;
					aStep = 1;
					bFirst = bLink->GetSize()-1;
					bLast = 0;
					bStep = -1;
				}

				int index = 0;
				//which direction to traverse the first road?
				if (aStep < 0) {
					for (i = aFirst; i >= aLast; i--) {
						newLink->SetAt(index, aLink->GetAt(i));
						index++;
					}
				} else {
					for (i = aFirst; i <= aLast; i++) {
						newLink->SetAt(index, aLink->GetAt(i));
						index++;
					}
				}

				//which direction to traverse the second road?
				if (bStep < 0) {
					for (i = bFirst; i >= bLast; i--) {
						newLink->SetAt(index, bLink->GetAt(i));
						index++;
					}
				} else {
					for (i = bFirst; i <= bLast; i++) {
						newLink->SetAt(index, bLink->GetAt(i));
						index++;
					}
				}

				TNode *tmpNode;
				//replace roads at endpoints with new road
				if (aLink->GetNode(0) == curNode) {
					index = 1;
				} else {
					index = 0;
				}
				tmpNode = aLink->GetNode(index);
				newLink->SetNode(0, tmpNode);
				tmpNode->AddLink(newLink);
				tmpNode->SetIntersectType(newLink,tmpNode->GetIntersectType(aLink));
				aLink->GetNode(index)->DetachLink(aLink);
				if (bLink->GetNode(0) == curNode) {
					index = 1;
				} else {
					index = 0;
				}
				tmpNode = bLink->GetNode(index);
				newLink->SetNode(1, tmpNode);
				tmpNode->AddLink(newLink);
				tmpNode->SetIntersectType(newLink,tmpNode->GetIntersectType(bLink));
				tmpNode->DetachLink(bLink);

				newLink->ComputeExtent();
				((LinkEdit*)newLink)->m_fLength = newLink->Length();

				//insert new road...
				newLink->m_pNext = m_pFirstLink;
				m_pFirstLink = newLink;

				//remove old roads...
				TLink* curLink = m_pFirstLink->m_pNext;
				TLink* prevLink = m_pFirstLink;
				while (curLink) {
					if (curLink == bLink) {
						prevLink->m_pNext = curLink->m_pNext;
						curLink = curLink->m_pNext;
						delete bLink;
					} else if (curLink == aLink) {
						prevLink->m_pNext = curLink->m_pNext;
						curLink = curLink->m_pNext;
						delete aLink;
					} else {
						prevLink = curLink;	
						curLink = curLink->m_pNext;
					}
				}

				tmpNode = curNode;
				//delete the current node - it is no longer needed.
				if (prevNode) {
					curNode = curNode->m_pNext;
					prevNode->m_pNext = curNode;
				} else {
					curNode = curNode->m_pNext;
					m_pFirstNode = curNode;
				}
				count++;
				delete tmpNode;
				//newLink->m_bTest = true;
			} else {
				prevNode = curNode;
				curNode = (NodeEdit*)(curNode->m_pNext);
			}
		} else {
			prevNode = curNode;
			curNode = (NodeEdit*)(curNode->m_pNext);
		}
	}

	VTLOG(" Eliminated %i of %i nodes\n", count, total);
	VTLOG(" There are now %i nodes and %i roads\n", NumNodes(),NumLinks());
	return count;
}


//
// Does two cleanup steps on the points of the road:
//  1. if the road's endpoint does not match the coordinate of
//		that node, then add a point to make it so
//  2. if the road has any two redundant points (same coordinate)
//		then remove one of them.
// Returns the number of road points affected.
//
int RoadMapEdit::CleanLinkPoints()
{
	int count = 0;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->m_pNext; pN = pN->GetNext())
		pN->EnforceLinkEndpoints();

	for (LinkEdit *pR = GetFirstLink(); pR; pR = pR->GetNext())
	{
		for (unsigned int i = 1; i < pR->GetSize(); i++)
		{
			if (pR->GetAt(i-1) == pR->GetAt(i))
			{
				// the point is redudant and should be removed
				pR->RemovePoint(i-1);
				count++;
				pR->m_fLength = pR->Length();
				break;
			}
		}	
	}
	return count;
}


//
// deletes roads that either:
//		have the same start and end nodes and have less than 4 points
//		have less than 2 points, regardless of start or end nodes.
//
int RoadMapEdit::DeleteDanglingLinks()
{
	NodeEdit *pN1, *pN2;
	LinkEdit *pR = GetFirstLink();
	LinkEdit *prev = NULL, *next;
	int count = 0;
	while (pR)
	{
		next = pR->GetNext();
		pN1 = pR->GetNode(0);
		pN2 = pR->GetNode(1);
		if ((pN1 == pN2 && pR->GetSize() <4) ||
			(pR->GetSize() < 2))
		{
			//delete the road!
			if (prev)
				prev->m_pNext = next;
			else
				m_pFirstLink = next;

			pN1->DetachLink(pR);
			pN2->DetachLink(pR);
			delete pR;
			count++;
		}
		else
			prev = pR;

		pR = next;
	}
	return count;
}


// fix when two different roads meet at the same node along the same path
int RoadMapEdit::FixOverlappedLinks(bool bDegrees)
{
	int fixed = 0, roads;
	DPoint2 p0, p1, diff;
	int i, j;
	LinkEdit *pR1=NULL, *pR2=NULL;

	double tolerance;
	if (bDegrees)
		tolerance = TOLERANCE_DEGREES/8;
	else
		tolerance = TOLERANCE_METERS/8;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->m_pNext; pN = pN->GetNext())
	{
		roads = pN->m_iLinks;

		if (roads < 2) continue;

		bool bad = false;
		for (i = 0; i < roads-1 && !bad; i++)
		{
			pR1 = pN->GetLink(i);
			p0 = pN->find_adjacent_roadpoint2d(pR1);
			for (j = i+1; j < roads; j++)
			{
				pR2 = pN->GetLink(j);

				if (pR1 == pR2) continue;	// don't worry about loops

				p1 = pN->find_adjacent_roadpoint2d(pR2);
				diff = (p1 - p0);
				if (fabs(diff.x) < 1.0f && fabs(diff.y) < 1.0f)
				{
					bad = true;
					break;
				}
			}
		}
		if (!bad) continue;

		// now fix it, by removing the two points of the 2 roads that
		// overlap as they approach the node
		if (pR1->GetNode(0) == pN)
			pR1->RemovePoint(1);			// roads starts here
		else
			pR1->RemovePoint(pR1->GetSize()-2);	// road ends here
		if (pR2->GetNode(0) == pN)
			pR2->RemovePoint(1);			// roads starts here
		else
			pR2->RemovePoint(pR2->GetSize()-2);	// road ends here
		fixed++;
		((LinkEdit*)pR1)->m_fLength = pR1->Length();
		((LinkEdit*)pR2)->m_fLength = pR2->Length();

	}
	return fixed;
}

//
// Returns the absolute difference between 2 angles.
//
float angle_diff(float a1, float a2)
{
	// transform both angles to [0, 2pi]
	while (a1 < 0.0f) a1 += PI2f;
	while (a1 > PI2f) a1 -= PI2f;
	while (a2 < 0.0f) a2 += PI2f;
	while (a2 > PI2f) a2 -= PI2f;

	float diff = a1 - a2;
	if (diff > PIf) diff -= PI2f;
	if (diff < -PIf) diff += PI2f;
	return fabsf(diff);
}

//
// deletes really close parallel (roughly) roads, where one of the roads go nowhere.
//
int RoadMapEdit::FixExtraneousParallels()
{
	int removed = 0, i, j, roads;
	LinkEdit *pR1=NULL, *pR2=NULL;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->m_pNext; pN = pN->GetNext())
	{
		roads = pN->m_iLinks;

		if (roads < 3) continue;

		bool bad = false;
		pN->DetermineLinkAngles();
		for (i = 0; i < roads-1 && !bad; i++)
		{
			pR1 = pN->GetLink(i);
			for (j = i+1; j < roads; j++)
			{
				pR2 = pN->GetLink(j);
				if (pR2 == pR1) break;		// ignore loops
				float diff = angle_diff(pN->m_fLinkAngle[i], pN->m_fLinkAngle[j]);
				if (diff < 0.3f)
				{
					// pR1 and pR2 are suspiciously close
					bad = true;
					break;
				}
			}
		}
		if (bad)
		{
			int leads_to[2];
			if (pR1->GetNode(0) == pN)
				leads_to[0] = pR1->GetNode(1)->m_iLinks;
			else
				leads_to[0] = pR1->GetNode(0)->m_iLinks;
			if (pR2->GetNode(0) == pN)
				leads_to[1] = pR2->GetNode(1)->m_iLinks;
			else
				leads_to[1] = pR2->GetNode(0)->m_iLinks;
			if (leads_to[0] == 1 && leads_to[1] > 1)
			{
				// delete R1
				DeleteSingleLink(pR1);
				removed++;
			}
			else if (leads_to[0] > 1 && leads_to[1] == 1)
			{
				// delete R2;
				DeleteSingleLink(pR2);
				removed++;
			}
			else
			{
				// dangerous case: both road look valid, yet meet at
				// a very close angle
				;	// a spot to put a breakpoint
			}
		}
	}
	return removed;
}

//if a road starts and ends at the same node, split it into 2 roads (at the middle point.)
//will create a new uncontrolled node in the middle.
int RoadMapEdit::SplitLoopingLinks()
{
	int count = 0;
	LinkEdit *curLink = GetFirstLink();
	NodeEdit *curNode;
	LinkEdit *roadA, *roadB, *tmpLink;

	while (curLink)
	{
		if (curLink->GetNode(0) == curLink->GetNode(1))
		{
			//we have a looping road.  split it in half.
			curNode = curLink->GetNode(0);

			//create 2 new roads.
			unsigned int switchPoint = curLink->GetSize()/2 + 1;
			unsigned int i = 0;
			roadA = new LinkEdit();
			roadA->SetSize(switchPoint);
			roadB = new LinkEdit();
			roadB->SetSize(curLink->GetSize() - switchPoint + 1);
			for (; i < switchPoint; i++)
				roadA->SetAt(i, curLink->GetAt(i));

			unsigned int j = 0;
			i--;
			for (; i < curLink->GetSize(); i++)
			{
				roadB->SetAt(j, curLink->GetAt(i));
				j++;
			}
			//equal characteristics
			roadA->m_iFlags = curLink->m_iFlags;
			roadA->m_iHwy = curLink->m_iHwy;
			roadA->m_iLanes = curLink->m_iLanes;
			roadA->m_Surface = curLink->m_Surface;
			roadA->ComputeExtent();
			roadA->m_fLength = roadA->Length();
			
			roadB->m_iFlags = curLink->m_iFlags;
			roadB->m_iHwy = curLink->m_iHwy;
			roadB->m_iLanes = curLink->m_iLanes;
			roadB->m_Surface = curLink->m_Surface;
			roadB->ComputeExtent();
			roadB->m_fLength = roadB->Length();

			NodeEdit *node = new NodeEdit();
			node->m_p = roadB->GetAt(0);

			//add roads to node;
			roadA->SetNode(0, curNode);
			curNode->AddLink(roadA);
			roadA->SetNode(1, node);
			node->AddLink(roadA);

			roadB->SetNode(1, curNode);
			curNode->AddLink(roadB);
			roadB->SetNode(0, node);
			node->AddLink(roadB);

			node->SetIntersectType(0, IT_NONE);
			node->SetIntersectType(1, IT_NONE);

			//add node to road map
			node->m_pNext = m_pFirstNode;
			m_pFirstNode = node;
			//add roads to road map
			roadA->m_pNext = m_pFirstLink;
			m_pFirstLink = roadA;
			roadB->m_pNext = m_pFirstLink;
			m_pFirstLink = roadB;

			//set traffic behavior
//			node->m_Behavior = IT_NONE;
			node->SetVisual(VIT_NONE);

			curNode->SetIntersectType(roadA, curNode->GetIntersectType(curLink));
			curNode->SetIntersectType(roadB, curNode->GetIntersectType(curLink));

			//delete the old road
			tmpLink = curLink;
			curLink = (LinkEdit*) curLink->m_pNext;
			//delete the old road.
			DeleteSingleLink(tmpLink);

			//adjust traffic signals as necessary
			curNode->AdjustForLights();			

			count++;
		}
		else
		{
			curLink = (LinkEdit*) curLink->m_pNext;
		}
	}
	return count;
}
