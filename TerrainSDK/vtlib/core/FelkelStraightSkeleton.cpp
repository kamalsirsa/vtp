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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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
				m_vl.push_back(CVertex(points[0].m_Point, points[s - 1].m_Point, points[s - 1].m_Slope, points[1].m_Point, points[0].m_Slope));
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
				m_vl.push_back(CVertex(points[f].m_Point, points[f - 1].m_Point, points[f - 1].m_Slope, points[(f + 1) % s].m_Point, points[f].m_Slope));
				to = m_vl.end();
				to--;
				m_boundaryedges.push_front(CSkeletonLine(*from, *to));
			}
		}
	}

	m_NumberOfBoundaryVertices = m_vl.size();
	m_NumberOfBoundaryEdges = m_boundaryedges.size();

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


#ifdef FELKELDEBUG
	VTLOG("Building initial intersection queue\n");
#endif
	for (i = m_vl.begin(); i != m_vl.end (); i++)
	{
		if (!(*i).m_done)
		{
			CIntersection is(m_vl, *i);
			if (is.m_height != CN_INFINITY)
				m_iq.push(is);
		}
	}

#ifdef FELKELDEBUG
	VTLOG("Processing intersection queue\n");
#endif
	while (m_iq.size ())
	{
		CIntersection i = m_iq.top ();

		m_iq.pop ();

#ifdef FELKELDEBUG
		VTLOG("Processing %d %d left done %d right done %d\n",
			i.m_leftVertex->m_ID, i.m_rightVertex->m_ID, i.m_leftVertex->m_done, i.m_rightVertex->m_done);
#endif
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

#if VTDEBUG
		if (!(i.m_leftVertex->m_prevVertex != i.m_rightVertex))
			VTLOG("%s %d Assert failed\n", __FILE__, __LINE__);
		if (!(i.m_rightVertex->m_nextVertex != i.m_leftVertex))
			VTLOG("%s %d Assert failed\n", __FILE__, __LINE__);
#endif
		if (i.m_type == CIntersection::CONVEX)
			if (i.m_leftVertex->m_prevVertex->m_prevVertex == i.m_rightVertex || i.m_rightVertex->m_nextVertex->m_nextVertex == i.m_leftVertex)
				i.ApplyLast3(m_skeleton, m_vl);
			else
				i.ApplyConvexIntersection(m_skeleton, m_vl, m_iq);
		if (i.m_type == CIntersection :: NONCONVEX)
			i.ApplyNonconvexIntersection(m_skeleton, m_vl, m_iq);
	}
#ifdef FELKELDEBUG
	Dump();
#endif
	return m_skeleton;
}

CSkeleton& CStraightSkeleton::MakeSkeleton(Contour &points)
{
	ContourVector vv;

	vv.push_back (points);

	return MakeSkeleton(vv);
}

CSkeleton CStraightSkeleton::CompleteWingedEdgeStructure(ContourVector &contours)
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

#ifdef FELKELDEBUG
	Dump();
#endif

	for (size_t ci = 0; ci < contours.size(); ci++)
		if (!FixSkeleton(contours[ci]))
			return CSkeleton();

#ifdef FELKELDEBUG
	Dump();
#endif

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
		C3DPoint& p1 = points[pi].m_Point;
		C3DPoint& p2 = points[(pi+1)%points.size()].m_Point;

		CSkeleton::iterator s1;
		for (s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
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
			// Join up the edges if needed
			if (bReversed)
			{
				if (bPrevReversed)
				{
					// Joining lower to higher
#if VTDEBUG
					if (!((NULL == pPrevEdge->m_lower.m_left) && (NULL == pNextEdge->m_higher.m_left)))
						VTLOG("%s %d Assert failed prev lower left %x next higher left %x\n", __FILE__, __LINE__, pPrevEdge->m_lower.m_left, pNextEdge->m_higher.m_left);
#else
					if ((NULL != pPrevEdge->m_lower.m_left) || (NULL != pNextEdge->m_higher.m_left))
						return false;
#endif
					pPrevEdge->m_lower.m_left = pNextEdge;
					pNextEdge->m_higher.m_left = pPrevEdge;
				}
				else
				{
					// Joing higher to higher
#if VTDEBUG
					if (!((NULL == pPrevEdge->m_higher.m_right) && (NULL == pNextEdge->m_higher.m_left)))
						VTLOG("%s %d Assert failed prev higher right %x next higher left %x\n", __FILE__, __LINE__, pPrevEdge->m_higher.m_right, pNextEdge->m_higher.m_left);
#else
					if ((NULL != pPrevEdge->m_higher.m_right) || (NULL != pNextEdge->m_higher.m_left))
						return false;
#endif
					pPrevEdge->m_higher.m_right = pNextEdge;
					pNextEdge->m_higher.m_left = pPrevEdge;
				}
			}
			else
			{
				if (bPrevReversed)
				{
					// Joining lower to lower
#if VTDEBUG
					if (!((NULL == pPrevEdge->m_lower.m_left) && (NULL == pNextEdge->m_lower.m_right)))
						VTLOG("%s %d Assert failed prev lower left %x next lower right %x\n", __FILE__, __LINE__, pPrevEdge->m_lower.m_left, pNextEdge->m_lower.m_right);
#else
					if ((NULL != pPrevEdge->m_lower.m_left) || (NULL != pNextEdge->m_lower.m_right))
						return false;
#endif
					pPrevEdge->m_lower.m_left = pNextEdge;
					pNextEdge->m_lower.m_right = pPrevEdge;
				}
				else
				{
					// Joining higher to lower
#if VTDEBUG
					if (!((NULL == pPrevEdge->m_higher.m_right) && (NULL == pNextEdge->m_lower.m_right)))
						VTLOG("%s %d Assert failed prev higher right %x next lower right %x\n", __FILE__, __LINE__, pPrevEdge->m_higher.m_right, pNextEdge->m_lower.m_right);
#else
					if ((NULL != pPrevEdge->m_higher.m_right) || (NULL != pNextEdge->m_lower.m_right))
						return false;
#endif
					pPrevEdge->m_higher.m_right = pNextEdge;
					pNextEdge->m_lower.m_right = pPrevEdge;
				}
			}
		}
		while (bReversed ? p2 != pNextEdge->m_lower.m_vertex->m_point : p2 != pNextEdge->m_higher.m_vertex->m_point);
	}
	return true;
}

CSkeletonLine *CStraightSkeleton::FindNextRightEdge(CSkeletonLine* pEdge, bool *bReversed)
{
	CSkeletonLine *pNextEdge=NULL;
	C3DPoint OldPoint;
	CRidgeLine OldEdge;
	CRidgeLine NewEdge;
	CNumber Angle;
	CNumber LowestAngle = CN_INFINITY;
	bool bBoundaryEdge = pEdge->m_ID < m_NumberOfBoundaryEdges;

	if(*bReversed)
	{
		OldPoint = pEdge->m_lower.m_vertex->m_point;
		OldEdge = CRidgeLine(pEdge->m_higher.m_vertex->m_point, OldPoint);
	}
	else
	{
		OldPoint = pEdge->m_higher.m_vertex->m_point;
		OldEdge = CRidgeLine(pEdge->m_lower.m_vertex->m_point, OldPoint);
	}

	for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
	{
		if ((*s1).m_ID != pEdge->m_ID)
		{
			if ((((*s1).m_lower.m_vertex->m_point == OldPoint) || ((*s1).m_higher.m_vertex->m_point == OldPoint)) &&
				// If current edge is a boundary edge then skip any candidates that are also boundaries
				!(bBoundaryEdge && (*s1).m_ID < m_NumberOfBoundaryEdges))
			{
				// Current edge is not a boundary edge then skip any candidates were I am
				// considering the higher vertex of the boundary edge
				if (!bBoundaryEdge &&
						(*s1).m_ID < m_NumberOfBoundaryEdges &&
						(OldPoint == (*s1).m_higher.m_vertex->m_point))
					continue;

				bool bTemp;

				if ((*s1).m_lower.m_vertex->m_point == OldPoint)
				{
					// matched the lower vertex of an edge
					NewEdge = CRidgeLine(OldPoint, (*s1).m_higher.m_vertex->m_point);
					bTemp = false;
				}
				else
				{
					NewEdge = CRidgeLine(OldPoint, (*s1).m_lower.m_vertex->m_point);
					bTemp = true;
				}
				Angle = CNumber(NewEdge.m_Angle - OldEdge.m_Angle).NormalizedAngle() + CN_PI;
				if (Angle < LowestAngle)
				{
					LowestAngle = Angle;
					pNextEdge = &(*s1);
					*bReversed = bTemp;
				}
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
		C3DPoint& p1 = points[pi].m_Point;
		C3DPoint& p2 = points[(pi+1)%points.size()].m_Point;

		Area += (p2.m_x - p1.m_x) * (p2.m_z + p1.m_z);
	}

	if (Area > 0)
		return true;
	else
		return false;
}

CNumber CStraightSkeleton::CalculateNormal(const CSkeletonLine& Edge, const C3DPoint& Point)
{
	C3DPoint p1 = Edge.m_lower.m_vertex->m_point;
	C3DPoint p2 = Edge.m_higher.m_vertex->m_point;
	C3DPoint p3 = Point;
	C3DPoint pIntersection;
	CNumber SegmentLength = (p2 - p1).LengthXZ();
	CNumber U;

	U = (((p3.m_x - p1.m_x) * (p2.m_x - p1.m_x)) + ((p3.m_z - p1.m_z) * (p2.m_z - p1.m_z))) /
		(SegmentLength * SegmentLength);

	pIntersection.m_x = p1.m_x + U * (p2.m_x - p1.m_x);
	pIntersection.m_z = p1.m_z + U * (p2.m_z - p1.m_z);

	return (pIntersection - p3).LengthXZ();
}

#ifdef FELKELDEBUG
void CStraightSkeleton::Dump()
{
	int i;

	VTLOG("Skeleton:\n");

	i = 0;
	for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
	{
		CSkeletonLine& db = (*s1);
		VTLOG("ID: %d lower leftID %d rightID %d vertexID %d (%f %f %f)\nhigher leftID %d rightID %d vertexID %d (%f %f %f)\n",
			db.m_ID,
			db.m_lower.LeftID(),
			db.m_lower.RightID(),
			db.m_lower.VertexID(), db.m_lower.m_vertex->m_point.m_x, db.m_lower.m_vertex->m_point.m_y, db.m_lower.m_vertex->m_point.m_z,
			db.m_higher.LeftID(),
			db.m_higher.RightID(),
			db.m_higher.VertexID(), db.m_higher.m_vertex->m_point.m_x, db.m_higher.m_vertex->m_point.m_y, db.m_higher.m_vertex->m_point.m_z);
	}
}
#endif
