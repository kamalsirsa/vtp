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
void NodeEdit::EnforceRoadEndpoints()
{
	//for the roads that now end in pN2, move it's end point as well.
	for (int k = 0; k < m_iRoads; k++)
	{
		Road *r = GetRoad(k);
		if (r->GetNode(0) == this)
			r->SetAt(0, m_p);
		else if (r->GetNode(1) == this)
			r->SetAt(r->GetSize() - 1, m_p);
		else
			assert(0);	// bad case!

		((RoadEdit*)r)->m_fLength = r->Length();
	}
}


// merge nodes less than 8m apart
#define TOLERANCE (8.0f)

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
int RoadMapEdit::MergeRedundantNodes(void progress_callback(int))
{
	NodeEdit *prev = NULL, *next;
	DPoint2 diff;
	int removed = 0;

	int nodes = NumNodes();
	int total = nodes * nodes / 2;
	int count = 0;

	NodeEdit *pN, *pN2;

	for (pN = GetFirstNode(); pN && pN->m_pNext; pN = next)
	{
		next = pN->GetNext();
		bool remove = false;
		for (pN2 = next; pN2; pN2 = pN2->GetNext())
		{
			count++;
			if ((count % 1000) == 0)
				progress_callback(count * 100 / total);

			diff = pN2->m_p - pN->m_p;
			if (diff.Length() < TOLERANCE)
			{
				remove = true;
				break;
			}
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
			pN2->EnforceRoadEndpoints();
			removed++;
		}
		else
			prev = pN;
	}
//	wxLogMessage("Removed %i nodes\n", removed);
	return removed;
}


//
// Remove roads which start and end at the same node, yet don't
// have enough points (less than 3) to be a valid loop
//
// There are many other ways a road can be degenerate, but we
// don't check for them (yet).
//
// Returns the number of roads removed.
//
int RoadMapEdit::RemoveDegenerateRoads()
{
	int count = 0;

	RoadEdit *prevR = NULL, *nextR;

	for (RoadEdit *pR = GetFirstRoad(); pR; pR = nextR)
	{
		nextR = pR->GetNext();
		if (pR->GetNode(0) == pR->GetNode(1) && pR->GetSize() < 3)
		{
			count++;
			// remove it
			if (prevR)
				prevR->m_pNext = nextR;
			else
				m_pFirstRoad = nextR;

			// notify the nodes that the road is gone
			pR->GetNode(0)->DetachRoad(pR);
			pR->GetNode(1)->DetachRoad(pR);

			delete pR;
		}
		else
			prevR = pR;
	}
//	wxLogMessage("Removed %i degenerate roads.\n", count);
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
	Road* aRoad;
	Road* bRoad;
	Node *curNode = m_pFirstNode;
	Node *prevNode = NULL;
	int count = 0;
	int total = NumNodes();
	while (curNode != NULL)
	{
		if (curNode->m_iRoads == 2 && !curNode->IsControlled())
		{
			aRoad = curNode->GetRoad(0);
			bRoad = curNode->GetRoad(1);
			//see if the 2 roads have equivalent characteristics.
			//and the roads are NOT the same!
			if (aRoad != bRoad &&
				aRoad->m_fWidth == bRoad->m_fWidth &&
				aRoad->m_iLanes == bRoad->m_iLanes &&
				aRoad->m_Surface == bRoad->m_Surface &&
				aRoad->m_iHwy == bRoad->m_iHwy &&
				aRoad->m_iFlags == bRoad->m_iFlags)
			{
				
				//merge the 2 roads into one.
				RoadEdit* newRoad = new RoadEdit();
				//make the new road the same as the old roads...
				newRoad->m_fWidth = bRoad->m_fWidth;
				newRoad->m_iLanes = bRoad->m_iLanes;
				newRoad->m_Surface = bRoad->m_Surface;
				newRoad->m_iHwy = bRoad->m_iHwy;
				newRoad->m_iFlags = bRoad->m_iFlags;
				newRoad->SetSize(aRoad->GetSize() + bRoad->GetSize() - 1);  //subtract one for overlapping middle point

				//extract road coordinates
				//where/how do we connect road a to road b?
				DPoint2 diff1, diff2, diff3, diff4;
				double dist, dist1, dist2, dist3, dist4;

				//find distance between endpoints
				diff1 = aRoad->GetAt(0) - bRoad->GetAt(0);
				dist1 = diff1.Length();
				diff2 = aRoad->GetAt(0) - bRoad->GetAt(bRoad->GetSize()-1);
				dist2 = diff2.Length();
				diff3 = aRoad->GetAt(aRoad->GetSize()-1) - bRoad->GetAt(0);
				dist3 = diff3.Length();
				diff4 = aRoad->GetAt(aRoad->GetSize()-1) - bRoad->GetAt(bRoad->GetSize()-1);
				dist4 = diff4.Length();

				//shortest distance is connection point
				dist = fmin(dist1, dist2, dist3, dist4);
				
				int i;
				int aFirst, aLast, bFirst, bLast, aStep, bStep;
				if (dist == dist1) {
					//connection: aRoad->m_p[0] - bRoad->m_p[0];
					aFirst = aRoad->GetSize()-1;
					aLast = 1;
					aStep = -1;
					bFirst = 0;
					bLast = bRoad->GetSize()-1;
					bStep = 1;
				} else if (dist == dist2) {
					//connection: aRoad->m_p[0] - bRoad->m_p[bRoad->GetSize()-1];
					aFirst = aRoad->GetSize()-1;
					aLast = 1;
					aStep = -1;
					bFirst = bRoad->GetSize()-1;
					bLast = 0;
					bStep = -1;
				} else if (dist == dist3) {
					//connection: m_p[aRoad->GetSize()-1] - bRoad->m_p[0];
					aFirst = 0;
					aLast = aRoad->GetSize()-2;
					aStep = 1;
					bFirst = 0;
					bLast = bRoad->GetSize()-1;
					bStep = 1;
				} else if (dist == dist4) {
					//connection: m_p[aRoad->GetSize()-1] - bRoad->m_p[bRoad->GetSize()-1];
					aFirst = 0;
					aLast = aRoad->GetSize()-2;
					aStep = 1;
					bFirst = bRoad->GetSize()-1;
					bLast = 0;
					bStep = -1;
				}

				int index = 0;
				//which direction to traverse the first road?
				if (aStep < 0) {
					for (i = aFirst; i >= aLast; i--) {
						newRoad->SetAt(index, aRoad->GetAt(i));
						index++;
					}
				} else {
					for (i = aFirst; i <= aLast; i++) {
						newRoad->SetAt(index, aRoad->GetAt(i));
						index++;
					}
				}

				//which direction to traverse the second road?
				if (bStep < 0) {
					for (i = bFirst; i >= bLast; i--) {
						newRoad->SetAt(index, bRoad->GetAt(i));
						index++;
					}
				} else {
					for (i = bFirst; i <= bLast; i++) {
						newRoad->SetAt(index, bRoad->GetAt(i));
						index++;
					}
				}

				Node* tmpNode;
				//replace roads at endpoints with new road
				if (aRoad->GetNode(0) == curNode) {
					index = 1;
				} else {
					index = 0;
				}
				tmpNode = aRoad->GetNode(index);
				newRoad->SetNode(0, tmpNode);
				tmpNode->AddRoad(newRoad);
				tmpNode->SetIntersectType(newRoad,tmpNode->GetIntersectType(aRoad));
				aRoad->GetNode(index)->DetachRoad(aRoad);
				if (bRoad->GetNode(0) == curNode) {
					index = 1;
				} else {
					index = 0;
				}
				tmpNode = bRoad->GetNode(index);
				newRoad->SetNode(1, tmpNode);
				tmpNode->AddRoad(newRoad);
				tmpNode->SetIntersectType(newRoad,tmpNode->GetIntersectType(bRoad));
				tmpNode->DetachRoad(bRoad);

				newRoad->ComputeExtent();
				((RoadEdit*)newRoad)->m_fLength = newRoad->Length();

				//insert new road...
				newRoad->m_pNext = m_pFirstRoad;
				m_pFirstRoad = newRoad;

				//remove old roads...
				Road* curRoad = m_pFirstRoad->m_pNext;
				Road* prevRoad = m_pFirstRoad;
				while (curRoad) {
					if (curRoad == bRoad) {
						prevRoad->m_pNext = curRoad->m_pNext;
						curRoad = curRoad->m_pNext;
						delete bRoad;
					} else if (curRoad == aRoad) {
						prevRoad->m_pNext = curRoad->m_pNext;
						curRoad = curRoad->m_pNext;
						delete aRoad;
					} else {
						prevRoad = curRoad;	
						curRoad = curRoad->m_pNext;
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
				//newRoad->m_bTest = true;
			} else {
				prevNode = curNode;
				curNode = (NodeEdit*)(curNode->m_pNext);
			}
		} else {
			prevNode = curNode;
			curNode = (NodeEdit*)(curNode->m_pNext);
		}
	}

//	wxLogMessage("Eliminated %i of %i nodes\n", count, total);
//	wxLogMessage("There are now %i nodes and %i roads\n", NumNodes(),NumRoads());
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
int RoadMapEdit::CleanRoadPoints()
{
	int count = 0;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->m_pNext; pN = pN->GetNext())
		pN->EnforceRoadEndpoints();

	for (RoadEdit *pR = GetFirstRoad(); pR; pR = pR->GetNext())
	{
		for (int i = 1; i < pR->GetSize(); i++)
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
int RoadMapEdit::DeleteDanglingRoads()
{
	NodeEdit *pN1, *pN2;
	RoadEdit *pR = GetFirstRoad();
	RoadEdit *prev = NULL, *next;
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
				m_pFirstRoad = next;

			pN1->DetachRoad(pR);
			pN2->DetachRoad(pR);
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
int RoadMapEdit::FixOverlappedRoads()
{
	int fixed = 0, roads;
	DPoint2 p0, p1, diff;
	int i, j;
	RoadEdit *pR1, *pR2;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->m_pNext; pN = pN->GetNext())
	{
		roads = pN->m_iRoads;

		if (roads < 2) continue;

		bool bad = false;
		for (i = 0; i < roads-1 && !bad; i++)
		{
			pR1 = pN->GetRoad(i);
			p0 = pN->find_adjacent_roadpoint2d(pR1);
			for (j = i+1; j < roads; j++)
			{
				pR2 = pN->GetRoad(j);

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
		((RoadEdit*)pR1)->m_fLength = pR1->Length();
		((RoadEdit*)pR2)->m_fLength = pR2->Length();

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
	RoadEdit *pR1, *pR2;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->m_pNext; pN = pN->GetNext())
	{
		roads = pN->m_iRoads;

		if (roads < 3) continue;

		bool bad = false;
		pN->DetermineRoadAngles();
		for (i = 0; i < roads-1 && !bad; i++)
		{
			pR1 = pN->GetRoad(i);
			for (j = i+1; j < roads; j++)
			{
				pR2 = pN->GetRoad(j);
				if (pR2 == pR1) break;		// ignore loops
				float diff = angle_diff(pN->m_fRoadAngle[i], pN->m_fRoadAngle[j]);
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
				leads_to[0] = pR1->GetNode(1)->m_iRoads;
			else
				leads_to[0] = pR1->GetNode(0)->m_iRoads;
			if (pR2->GetNode(0) == pN)
				leads_to[1] = pR2->GetNode(1)->m_iRoads;
			else
				leads_to[1] = pR2->GetNode(0)->m_iRoads;
			if (leads_to[0] == 1 && leads_to[1] > 1)
			{
				// delete R1
				DeleteSingleRoad(pR1);
				removed++;
			}
			else if (leads_to[0] > 1 && leads_to[1] == 1)
			{
				// delete R2;
				DeleteSingleRoad(pR2);
				removed++;
			}
			else
			{
				// dangerous case: both road look valid, yet meet at
				// a very close angle
				int foo = 1;
			}
		}
	}
	return removed;
}

//if a road starts and ends at the same node, split it into 2 roads (at the middle point.)
//will create a new uncontrolled node in the middle.
int RoadMapEdit::SplitLoopingRoads() {
	int count = 0;
	RoadEdit *curRoad = GetFirstRoad();
	NodeEdit *curNode;
	RoadEdit *roadA, *roadB, *tmpRoad;

	while (curRoad) {
		if (curRoad->GetNode(0) == curRoad->GetNode(1)) {
			//we have a looping road.  split it in half.
			curNode = curRoad->GetNode(0);

			//create 2 new roads.
			int switchPoint = curRoad->GetSize()/2 + 1;
			int i = 0;
			roadA = new RoadEdit();
			roadA->SetSize(switchPoint);
			roadB = new RoadEdit();
			roadB->SetSize(curRoad->GetSize() - switchPoint + 1);
			for (; i < switchPoint; i++)
				roadA->SetAt(i, curRoad->GetAt(i));

			int j = 0;
			i--;
			for (; i < curRoad->GetSize(); i++)
			{
				roadB->SetAt(j, curRoad->GetAt(i));
				j++;
			}
			//equal characteristics
			roadA->m_iFlags = curRoad->m_iFlags;
			roadA->m_iHwy = curRoad->m_iHwy;
			roadA->m_iLanes = curRoad->m_iLanes;
			roadA->m_Surface = curRoad->m_Surface;
			roadA->ComputeExtent();
			roadA->m_fLength = roadA->Length();
			
			roadB->m_iFlags = curRoad->m_iFlags;
			roadB->m_iHwy = curRoad->m_iHwy;
			roadB->m_iLanes = curRoad->m_iLanes;
			roadB->m_Surface = curRoad->m_Surface;
			roadB->ComputeExtent();
			roadB->m_fLength = roadB->Length();

			NodeEdit *node = new NodeEdit();
			node->m_p = roadB->GetAt(0);

			//add roads to node;
			roadA->SetNode(0, curNode);
			curNode->AddRoad(roadA);
			roadA->SetNode(1, node);
			node->AddRoad(roadA);

			roadB->SetNode(1, curNode);
			curNode->AddRoad(roadB);
			roadB->SetNode(0, node);
			node->AddRoad(roadB);

			node->SetIntersectType(0, IT_NONE);
			node->SetIntersectType(1, IT_NONE);

			//add node to road map
			node->m_pNext = m_pFirstNode;
			m_pFirstNode = node;
			//add roads to road map
			roadA->m_pNext = m_pFirstRoad;
			m_pFirstRoad = roadA;
			roadB->m_pNext = m_pFirstRoad;
			m_pFirstRoad = roadB;

			//set traffic behavior
//			node->m_Behavior = IT_NONE;
			node->SetVisual(VIT_NONE);

			curNode->SetIntersectType(roadA, curNode->GetIntersectType(curRoad));
			curNode->SetIntersectType(roadB, curNode->GetIntersectType(curRoad));

			//delete the old road
			tmpRoad = curRoad;
			curRoad = (RoadEdit*) curRoad->m_pNext;
			//delete the old road.
			DeleteSingleRoad(tmpRoad);

			//adjust traffic signals as necessary
			curNode->AdjustForLights();			

			count++;
		} else {
			curRoad = (RoadEdit*) curRoad->m_pNext;
		}
	}
	return count;
}
