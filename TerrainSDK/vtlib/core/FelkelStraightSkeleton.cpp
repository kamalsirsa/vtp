//
// FelkelStraightSkeleton.cpp: implementation of the CStraightSkeleton class.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Straight skeleton algorithm and original implementation
// courtesy of Petr Felkel and Stepan Obdrzalek (petr.felkel@tiani.com)
// Re-implemented for the Virtual Terrain Project (vterrain.org)
// by Roger James (www.beardandsandals.co.uk)
//

#include "FelkelStraightSkeleton.h"

#ifdef EPS
#include <fstream>
#include <limits>
using namespace std;
#endif

#define EPSILON numeric_limits<double>epsilon()

//
// Construction/Destruction
//
CStraightSkeleton::CStraightSkeleton()
{
}

CStraightSkeleton::~CStraightSkeleton()
{
}

CSkeleton& CStraightSkeleton::MakeSkeleton(ContourVector &contours)
{
	while (m_iq.size ())
		m_iq.pop (); 	
	m_vl.erase(m_vl.begin(), m_vl.end());
	m_skeleton.erase(m_skeleton.begin(), m_skeleton.end());
	m_boundaryedges.erase(m_boundaryedges.begin(), m_boundaryedges.end());

	for (size_t ci = 0; ci < contours.size(); ci++)
	{
		Contour &points = contours[ci];

		Contour::iterator first = points.begin();
		if (first == points.end())
			break;

		Contour::iterator next = first;

		while (++next != points.end ())
		{
			if (*first == *next)
				points.erase (next);
			else
				first = next;
			next = first;
		}

		int s = points.size();
		CVertexList::iterator start = m_vl.end();
		CVertexList::iterator from = start;
		CVertexList::iterator to = start;

		for (int f = 0; f <= s; f++)
		{
			if (0 == f)
			{
				m_vl.push_back(CVertex(points[f], points[(s+f-1)%s], points[(s+f+1)%s]));
				to = m_vl.end();
				to--;
				start = to;
			}
			else if (f == s)
			{
				from = to;
				to = start;
				m_boundaryedges.push_front(CSkeletonLine(*from, *to));
			}
			else
			{
				from = to;
				m_vl.push_back(CVertex(points[f], points[(s+f-1)%s], points[(s+f+1)%s]));
				to = m_vl.end();
				to--;
				m_boundaryedges.push_front(CSkeletonLine(*from, *to));
			}
		}
	}

	m_NumberOfBoundaryVertices = m_vl.size();

	if (m_vl.size() < 3)
		return m_skeleton;

	CVertexList::iterator i;

	size_t vn = 0, cn = 0;

	CVertexList::iterator contourBegin;

	for (i = m_vl.begin (); i != m_vl.end (); i++)
	{
		(*i).m_prevVertex = &*m_vl.prev(i);
		(*i).m_nextVertex = &*m_vl.next(i);
		(*i).m_leftVertex = &*i;
		(*i).m_rightVertex = &*i;
		if (vn == 0)
			contourBegin = i;
		if (vn == contours [cn].size () - 1)
		{
			(*i).m_nextVertex = &*contourBegin;
			(*contourBegin).m_prevVertex = &*i;
			vn = 0;
			cn ++;
		}
		else
			vn ++;
	}


#ifdef EPS
	extern ostream *epsStream;
	
	*epsStream << "%Hranice" << endl;
	for (i = m_vl.begin (); i != m_vl.end (); i++)
	{
		if (!(*i).m_done)
		{
			*epsStream << (*i).m_point.m_x << ' ' << (*i).m_point.m_y << " moveto ";
			*epsStream << (*i).m_nextVertex ->m_point.m_x << ' ' << (*i).m_nextVertex->m_point.m_y << " lineto\n";
		}
	}
	*epsStream << "%Vnitrek" << endl;
#endif

	for (i = m_vl.begin(); i != m_vl.end (); i++)
	{
		if (!(*i).m_done)
		{
			CIntersection is(m_vl, *i);
			if (is.m_height != INFINITY)
				m_iq.push(is);
		}
	}

	while (m_iq.size ())
	{
		CIntersection i = m_iq.top ();

		m_iq.pop ();

		if (i.m_leftVertex->m_done && i.m_rightVertex->m_done)
			continue;
		if (i.m_leftVertex->m_done || i.m_rightVertex->m_done)
		{
			if (!i.m_leftVertex->m_done)
				m_iq.push(CIntersection (m_vl, *i.m_leftVertex));
			if (!i.m_rightVertex->m_done)
				m_iq.push(CIntersection (m_vl, *i.m_rightVertex));
			continue;
		}

		assert(i.m_leftVertex->m_prevVertex != i.m_rightVertex);
		assert(i.m_rightVertex->m_nextVertex != i.m_leftVertex);
		if (i.m_type == CIntersection::CONVEX)
		if (i.m_leftVertex->m_prevVertex->m_prevVertex == i.m_rightVertex || i.m_rightVertex->m_nextVertex->m_nextVertex == i.m_leftVertex)
			i.ApplyLast3(m_skeleton, m_vl);
		else i.ApplyConvexIntersection(m_skeleton, m_vl, m_iq);
		if (i.m_type == CIntersection :: NONCONVEX)
			i.ApplyNonconvexIntersection(m_skeleton, m_vl, m_iq);
	}
	return m_skeleton;
}

CSkeleton& CStraightSkeleton::MakeSkeleton(Contour &points)
{
	ContourVector vv;

	vv.push_back (points);

	return MakeSkeleton(vv);
}

CSkeleton& CStraightSkeleton::CompleteWingedEdgeStructure(ContourVector &contours)
{
	int iCount = 0;

	// Add boundary edges to the skeleton
	m_skeleton.splice(m_skeleton.begin(), m_boundaryedges);

	// Merge duplicate points
	CVertexList::iterator v1 = m_vl.begin();

	for (int i = 0; v1 != m_vl.end(); i++)
	{
		if (i >= m_NumberOfBoundaryVertices)
		{
			CVertexList::iterator v2 = v1;
			v2++;
			while(v2 != m_vl.end())
			{
				if (*v1 == *v2)
				{
					for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
					{
						if ((*s1).m_lower.VertexID() == (*v2).m_ID)
							(*s1).m_lower.m_vertex = &(*v1);
						if ((*s1).m_higher.VertexID() == (*v2).m_ID)
							(*s1).m_higher.m_vertex = &(*v1);
					}
					// dup vertex
					CVertexList::iterator temp = v2;
					v2--;
					m_vl.erase(temp);
				}
				v2++;
			}
		}
		v1++;
	}

	// Remove zero length edges
	for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
	{
		if ((*s1).m_lower.VertexID() == (*s1).m_higher.VertexID())
		{
			if (s1 == m_skeleton.begin())
			{
				m_skeleton.erase(s1);
				s1 = m_skeleton.begin();
			}
			else
			{
				CSkeleton::iterator temp = s1;
				s1--;
				m_skeleton.erase(temp);
			}
		}
		else
		{
			// Remove wing information
			(*s1).m_lower.m_left = NULL;
			(*s1).m_lower.m_right = NULL;
			(*s1).m_higher.m_left = NULL;
			(*s1).m_higher.m_right = NULL;
			(*s1).m_ID = iCount;
			iCount++;
		}
	}

	for (size_t ci = 0; ci < contours.size(); ci++)
	{
		if (!FixSkeleton(contours[ci]))
		{
			m_skeleton.erase(m_skeleton.begin(), m_skeleton.end());
			return m_skeleton;
		}
	}

	return m_skeleton;
}

bool CStraightSkeleton::FixSkeleton(Contour& points)
{
	CSkeletonLine* pNextEdge;
	CSkeletonLine* pPrevEdge;
	bool bPrevReversed;

	for (size_t pi = 0; pi < points.size(); pi++)
	{
		bool bReversed = false;
		C2DPoint& p1 = points[pi];
		C2DPoint& p2 = points[(pi+1)%points.size()];

		for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
			if (((*s1).m_lower.m_vertex->m_point == p1) && ((*s1).m_higher.m_vertex->m_point == p2))
				break;
		pNextEdge = &(*s1);
		// Circumnavigate the right face
		do
		{
			bPrevReversed = bReversed;
			// Find next boundary edge
			pPrevEdge = pNextEdge;

			pNextEdge = FindNextRightEdge(pPrevEdge, &bReversed);
//			assert (pNextEdge != NULL);
			if (NULL == pNextEdge)
				return false;
			// Join up the edges if needed
			if (bReversed)
			{
				if (bPrevReversed)
				{
					// Joining lower to higher
//					assert((NULL == pPrevEdge->m_lower.m_left) && (NULL == pNextEdge->m_higher.m_left));
					if ((NULL != pPrevEdge->m_lower.m_left) || (NULL != pNextEdge->m_higher.m_left))
						return false;
					pPrevEdge->m_lower.m_left = pNextEdge;
					pNextEdge->m_higher.m_left = pPrevEdge;
				}
				else
				{
					// Joing higher to higher
//					assert((NULL == pPrevEdge->m_higher.m_right) && (NULL == pNextEdge->m_higher.m_left));
					if ((NULL != pPrevEdge->m_higher.m_right) || (NULL != pNextEdge->m_higher.m_left))
						return false;
					pPrevEdge->m_higher.m_right = pNextEdge;
					pNextEdge->m_higher.m_left = pPrevEdge;
				}
			}
			else
			{
				if (bPrevReversed)
				{
					// Joining lower to lower
//					assert((NULL == pPrevEdge->m_lower.m_left) && (NULL == pNextEdge->m_lower.m_right));
					if ((NULL != pPrevEdge->m_lower.m_left) || (NULL != pNextEdge->m_lower.m_right))
						return false;
					pPrevEdge->m_lower.m_left = pNextEdge;
					pNextEdge->m_lower.m_right = pPrevEdge;
				}
				else
				{
					// Joining higher to lower
//					assert((NULL == pPrevEdge->m_higher.m_right) && (NULL == pNextEdge->m_lower.m_right));
					if ((NULL != pPrevEdge->m_higher.m_right) || (NULL != pNextEdge->m_lower.m_right))
						return false;
					pPrevEdge->m_higher.m_right = pNextEdge;
					pNextEdge->m_lower.m_right = pPrevEdge;
				}
			}
		}
		while (bReversed ? p2 != pNextEdge->m_lower.m_vertex->m_point : p2 != pNextEdge->m_higher.m_vertex->m_point);
	}
	return true;
}

CSkeletonLine* CStraightSkeleton::FindNextRightEdge(CSkeletonLine* pEdge, bool *bReversed)
{
	CSkeletonLine* pNextEdge = NULL;
	C2DPoint OldPoint;
	C2DPoint OldEdgeVector;
	C2DPoint NewEdgeVector;
	CNumber CosTheta;
	CNumber HighestCosTheta = 0;
	
	if(*bReversed)
	{
		OldPoint = pEdge->m_lower.m_vertex->m_point;
		OldEdgeVector = OldPoint - pEdge->m_higher.m_vertex->m_point;
	}
	else
	{
		OldPoint = pEdge->m_higher.m_vertex->m_point;
		OldEdgeVector = OldPoint - pEdge->m_lower.m_vertex->m_point;
	}

	for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
	{
#ifdef _DEBUG
		CSkeletonLine& db = (*s1);
		double Cross, Dot, l1, l2;
#endif
		if ((*s1).m_ID != pEdge->m_ID)
		{
			if (((*s1).m_lower.m_vertex->m_point == OldPoint) || ((*s1).m_higher.m_vertex->m_point == OldPoint))
			{
				bool bTemp;

				if ((*s1).m_lower.m_vertex->m_point == OldPoint)
				{
					// matched the lower vertex of an edge
					NewEdgeVector = (*s1).m_higher.m_vertex->m_point - OldPoint;
					bTemp = false;
				}	
				else
				{
					NewEdgeVector = (*s1).m_lower.m_vertex->m_point - OldPoint;
					bTemp = true;
				}
				CosTheta = OldEdgeVector.Dot(NewEdgeVector)/(OldEdgeVector.Length() * NewEdgeVector.Length());
				if ((double)OldEdgeVector.Cross(NewEdgeVector) < 0)
					CosTheta = 3 - CosTheta;
				else
					CosTheta += 1;
				if (CosTheta > HighestCosTheta)
				{
					HighestCosTheta = CosTheta;
					pNextEdge = &(*s1);
					*bReversed = bTemp;
				}
#ifdef _DEBUG
				Cross = (double)OldEdgeVector.Cross(NewEdgeVector);
				Dot = (double)OldEdgeVector.Dot(NewEdgeVector);
				l1 = (double)OldEdgeVector.Length();
				l2 = (double)NewEdgeVector.Length();
#endif
			}
		}
	}
	return pNextEdge;
}

// Thought I might need this but I dont at the moment :-)
bool CStraightSkeleton::IsClockwise(Contour& points)
{
	// Cannot remember if this works for all Jordan
	double Area = 0;

	for (size_t pi = 0; pi < points.size(); pi++)
	{
		C2DPoint& p1 = points[pi];
		C2DPoint& p2 = points[(pi+1)%points.size()];

		Area += (p2.m_x - p1.m_x) * (p2.m_y + p1.m_y);
	}

	if (Area > 0)
		return true;
	else
		return false;
}

CSkeleton& CStraightSkeleton::MakeRoof(ContourVector &contours, double dSlopeRadians)
{
	CSkeletonLine* pNextEdge = NULL;
	CSkeletonLine* pPrevEdge;
	CSkeletonLine* pStartEdge;
	CSkeletonLine* pStopEdge;
	bool bNextReversed;
	bool bPrevReversed;
	CNumber z;
	CNumber Normal;

	// Compute the z values
	// Set proportionate to the distance from the edge normal
	// scale to get an appropriate slope
	for (size_t ci = 0; ci < contours.size(); ci++)
	{
		Contour& points = contours[ci];
		for (size_t pi = 0; pi < points.size(); pi++)
		{
			// For each boundary edge zip round the polygon
			// and fill in the z for eaqch non boundary point that
			// has not been calculated already
			C2DPoint& p1 = points[pi];
			C2DPoint& p2 = points[(pi+1)%points.size()];
			// Find the starting edge
			for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
				if (((*s1).m_lower.m_vertex->m_point == p1) && ((*s1).m_higher.m_vertex->m_point == p2))
					break;
			if (s1 == m_skeleton.end())
				goto ErrorExit;
			pStartEdge = &(*s1);
			pPrevEdge = pStartEdge;
			pStopEdge = pStartEdge->m_lower.m_right;
			if (NULL == pStopEdge)
				goto ErrorExit;
			bPrevReversed = false;
			do
			{
				bNextReversed = false;
				if (bPrevReversed)
				{
					pNextEdge = pPrevEdge->m_lower.m_left;
					if (NULL == pNextEdge)
						goto ErrorExit;
					if (pPrevEdge->m_lower.m_vertex->m_point != pNextEdge->m_lower.m_vertex->m_point)
						bNextReversed = true;
				}
				else
				{
					pNextEdge = pPrevEdge->m_higher.m_right;
					if (NULL == pNextEdge)
						goto ErrorExit;
					if (pPrevEdge->m_higher.m_vertex->m_point != pNextEdge->m_lower.m_vertex->m_point)
						bNextReversed = true;
				}
				if (pNextEdge != pStopEdge)
				{
					if (bNextReversed)
					{
						Normal = CalculateNormal(*pStartEdge, pNextEdge->m_lower.m_vertex->m_point);
						z = Normal * tan(dSlopeRadians);
						if (pNextEdge->m_lower.m_vertex->m_point.m_z != 0.0)
						{
//							assert(pNextEdge->m_lower.m_vertex->m_point.m_z == z);
						}
						else
						{
							pNextEdge->m_lower.m_vertex->m_point.m_z = z;
						}
					}
					else
					{
						Normal = CalculateNormal(*pStartEdge, pNextEdge->m_higher.m_vertex->m_point);
						z = Normal * tan(dSlopeRadians);
						if (pNextEdge->m_higher.m_vertex->m_point.m_z != 0.0)
						{
//							assert(pNextEdge->m_higher.m_vertex->m_point.m_z == z);
						}
						else
						{
							pNextEdge->m_higher.m_vertex->m_point.m_z = z;
						}
					}
					pPrevEdge = pNextEdge;
					bPrevReversed = bNextReversed;
				}
			}
			while (pNextEdge != pStopEdge);
		}
	}
	return m_skeleton;

ErrorExit:
	m_skeleton.erase(m_skeleton.begin(), m_skeleton.end());
	return m_skeleton;
}

CNumber CStraightSkeleton::CalculateNormal(const CSkeletonLine& Edge, const C2DPoint& Point)
{
	C2DPoint p1 = Edge.m_lower.m_vertex->m_point;
	C2DPoint p2 = Edge.m_higher.m_vertex->m_point;
	C2DPoint p3 = Point;
	C2DPoint pIntersection;
	CNumber SegmentLength = (p2 - p1).Length();
	CNumber U;

	U = (((p3.m_x - p1.m_x) * (p2.m_x - p1.m_x)) + ((p3.m_y - p1.m_y) * (p2.m_y - p1.m_y))) /
					(SegmentLength * SegmentLength);

	pIntersection.m_x = p1.m_x + U * (p2.m_x - p1.m_x);
	pIntersection.m_y = p1.m_y + U * (p2.m_y - p1.m_y);

	return (pIntersection - p3).Length();
}

#ifdef _DEBUG
#include "windows.h"

void CStraightSkeleton::Dump()
{
	char DebugString[1024];
	int i;

	sprintf(DebugString, "Skeleton:\n");
	OutputDebugString(DebugString);

	i = 0;
	for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
	{
		CSkeletonLine& db = (*s1);
		sprintf(DebugString, "ID: %d\tlwr lID %d rID %d vID %d (%f %f %f)\n\t\thgr lID %d rID %d vID %d (%f %f %f)\n",
							db.m_ID,
							db.m_lower.LeftID(),
							db.m_lower.RightID(),
							db.m_lower.VertexID(), db.m_lower.m_vertex->m_point.m_x, db.m_lower.m_vertex->m_point.m_y, db.m_lower.m_vertex->m_point.m_z,
							db.m_higher.LeftID(),
							db.m_higher.RightID(),
							db.m_higher.VertexID(), db.m_higher.m_vertex->m_point.m_x, db.m_higher.m_vertex->m_point.m_y, db.m_higher.m_vertex->m_point.m_z);
		OutputDebugString(DebugString);
	}
}
#endif



