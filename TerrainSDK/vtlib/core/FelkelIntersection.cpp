//
// FelkelIntersection.cpp: implementation of the CIntersection class.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Straight skeleton algorithm and original implementation
// courtesy of Petr Felkel and Stepan Obdrzalek (petr.felkel@tiani.com)
// Re-implemented for the Virtual Terrain Project (vterrain.org)
// by Roger James (www.beardandsandals.co.uk)
//

#include "FelkelIntersection.h"

//
// Construction/Destruction
//
CIntersection :: CIntersection (CVertexList &vl, CVertex &v)
{
	assert (v.m_prevVertex == NULL || v.m_leftLine.FacingTowards (v.m_prevVertex -> m_rightLine));
	assert (v.m_nextVertex == NULL || v.m_rightLine.FacingTowards (v.m_nextVertex -> m_leftLine));

	CVertex &l = *v.m_prevVertex;
	CVertex &r = *v.m_nextVertex;

	assert (v.m_leftLine.m_Angle == v.m_leftVertex -> m_leftLine.m_Angle);
	assert (v.m_rightLine.m_Angle == v.m_rightVertex -> m_rightLine.m_Angle);

	CNumber al = v.m_axis.m_Angle - l.m_axis.m_Angle;
	al.NormalizeAngle();

	CNumber ar = v.m_axis.m_Angle - r.m_axis.m_Angle;
	ar.NormalizeAngle();

	C2DPoint i1 = v.m_axis.FacingTowards(l.m_axis) ? C2DPoint(INFINITY, INFINITY) : v.m_axis.Intersection(l.m_axis);
	C2DPoint i2 = v.m_axis.FacingTowards (r.m_axis) ? C2DPoint(INFINITY, INFINITY) : v.m_axis.Intersection(r.m_axis);

	CNumber d1 = v.m_point.Dist(i1);
	CNumber d2 = v.m_point.Dist(i2);

	CVertex *leftPointer, *rightPointer;
	C2DPoint p;
	CNumber d3 = INFINITY;
	CNumber av = v.m_leftLine.m_Angle - v.m_rightLine.m_Angle;

	av.NormalizeAngle();
	if (av > 0.0 && (v.m_leftLine.Intersection(v.m_rightLine) == v.m_point || v.m_leftLine.Intersection(v.m_rightLine) == C2DPoint(INFINITY, INFINITY)))
		d3 = v.NearestIntersection(vl, &leftPointer, &rightPointer, p);

	if (d1 <= d2 && d1 <= d3) { m_leftVertex = &l; m_rightVertex = &v; m_poi = i1; m_type = CONVEX; m_height = v.m_leftLine.Dist(i1); }
	else if (d2 <= d1 && d2 <= d3) { m_leftVertex = &v; m_rightVertex = &r; m_poi = i2; m_type = CONVEX; m_height = v.m_rightLine.Dist(i2); }
	else if (d3 <= d1 && d3 <= d2) { m_poi = p; m_leftVertex = m_rightVertex = &v; m_type = NONCONVEX; m_height = d3; }

	if (m_poi == C2DPoint (INFINITY, INFINITY)) m_height = INFINITY;
	if (m_type == NONCONVEX && v.InvalidIntersection (vl, *this)) m_height = INFINITY;
}

void CIntersection::ApplyNonconvexIntersection(CSkeleton &skeleton, CVertexList &vl, IntersectionQueue &iq)
{
	assert (m_leftVertex == m_rightVertex);

	CVertex *leftPointer, *rightPointer;
	C2DPoint p;
	CNumber d3 = INFINITY;

	d3 = m_leftVertex->NearestIntersection(vl, &leftPointer, &rightPointer, p);
	if (d3 == INFINITY) return;
							
	if (p != m_poi) return;
							
	CVertex v1 (p, *rightPointer, *m_rightVertex);
	CVertex v2 (p, *m_leftVertex, *leftPointer);

	assert (v1.m_point != C2DPoint(INFINITY, INFINITY));
	assert (v2.m_point != C2DPoint(INFINITY, INFINITY));

	m_leftVertex->m_done = true;
	//  i.rightVertex -> done = true;

	CVertex *newNext1 = m_rightVertex->m_nextVertex;
	CVertex *newPrev1 = leftPointer->Highest();
	v1.m_prevVertex = newPrev1;
	v1.m_nextVertex = newNext1;
	vl.push_back(v1);

	CVertex *v1Pointer = &vl.back();

	newPrev1->m_nextVertex = v1Pointer;
	newNext1->m_prevVertex = v1Pointer;
	m_rightVertex->m_higher = v1Pointer;

	CVertex *newNext2 = rightPointer->Highest();
	CVertex *newPrev2 = m_leftVertex->m_prevVertex;
	v2.m_prevVertex = newPrev2;
	v2.m_nextVertex = newNext2;
	vl.push_back(v2);

	CVertex *v2Pointer = &vl.back();

	newPrev2->m_nextVertex = v2Pointer;
	newNext2->m_prevVertex = v2Pointer;
	m_leftVertex->m_higher = v2Pointer;

	skeleton.push_back(CSkeletonLine(*m_rightVertex, *v1Pointer));

	CSkeletonLine *linePtr = &skeleton.back();

	skeleton.push_back(CSkeletonLine(*v1Pointer, *v2Pointer));

	CSkeletonLine *auxLine1Ptr = &skeleton.back ();

	skeleton.push_back(CSkeletonLine(*v2Pointer, *v1Pointer));

	CSkeletonLine *auxLine2Ptr = &skeleton.back();

	linePtr->m_lower.m_right = m_leftVertex->m_leftSkeletonLine;
	linePtr->m_lower.m_left = m_leftVertex->m_rightSkeletonLine;

	v1Pointer->m_rightSkeletonLine = v2Pointer->m_leftSkeletonLine = linePtr;
	v1Pointer->m_leftSkeletonLine = auxLine1Ptr;
	v2Pointer->m_rightSkeletonLine = auxLine2Ptr;

	auxLine1Ptr->m_lower.m_right = auxLine2Ptr;
	auxLine2Ptr->m_lower.m_left = auxLine1Ptr;

	if (m_leftVertex->m_leftSkeletonLine)
		m_leftVertex->m_leftSkeletonLine ->m_higher.m_left = linePtr;
	if (m_leftVertex->m_rightSkeletonLine)
		m_leftVertex->m_rightSkeletonLine->m_higher.m_right = linePtr;
	m_leftVertex->m_advancingSkeletonLine = linePtr;

	if (newNext1 == newPrev1)
	{
		v1Pointer->m_done = true;
		newNext1->m_done = true;
		skeleton.push_back(CSkeletonLine(*v1Pointer, *newNext1));
		CSkeletonLine *linePtr = &skeleton.back();
		linePtr->m_lower.m_right  = v1Pointer->m_leftSkeletonLine;
		linePtr->m_lower.m_left   = v1Pointer->m_rightSkeletonLine;
		linePtr->m_higher.m_right = newNext1->m_leftSkeletonLine;
		linePtr->m_higher.m_left  = newNext1->m_rightSkeletonLine;

		if (v1Pointer->m_leftSkeletonLine)
			v1Pointer->m_leftSkeletonLine->m_higher.m_left  = linePtr;
		if (v1Pointer->m_rightSkeletonLine)
			v1Pointer->m_rightSkeletonLine->m_higher.m_right = linePtr;
		if (newNext1->m_leftSkeletonLine)
			newNext1->m_leftSkeletonLine->m_higher.m_left  = linePtr;
		if (newNext1->m_rightSkeletonLine)
			newNext1->m_rightSkeletonLine->m_higher.m_right = linePtr;
	}
	else
	{
		CIntersection i1(vl, *v1Pointer);
		if (m_height != INFINITY) iq.push (i1);
	}
	if (newNext2 == newPrev2)
	{
		v2Pointer->m_done = true;
		newNext2 ->m_done = true;
		skeleton.push_back(CSkeletonLine (*v2Pointer, *newNext2));
		CSkeletonLine *linePtr = &skeleton.back();
		linePtr->m_lower.m_right = v2Pointer->m_leftSkeletonLine;
		linePtr->m_lower.m_left = v2Pointer->m_rightSkeletonLine;
		linePtr->m_higher.m_right = newNext2->m_leftSkeletonLine;
		linePtr->m_higher.m_left  = newNext2->m_rightSkeletonLine;

		if (v2Pointer->m_leftSkeletonLine)
			v2Pointer->m_leftSkeletonLine->m_higher.m_left  = linePtr;
		if (v2Pointer->m_rightSkeletonLine)
			v2Pointer->m_rightSkeletonLine->m_higher.m_right = linePtr;
		if (newNext2->m_leftSkeletonLine)
			newNext2 ->m_leftSkeletonLine->m_higher.m_left  = linePtr;
		if (newNext2->m_rightSkeletonLine)
			newNext2->m_rightSkeletonLine->m_higher.m_right = linePtr;
	}
	else
	{
		CIntersection i2 (vl, *v2Pointer);
		if (i2.m_height != INFINITY)
			iq.push(i2);
	}
}

void CIntersection::ApplyConvexIntersection(CSkeleton &skeleton, CVertexList &vl, IntersectionQueue &iq)
{
	CVertex vtx (m_poi, *m_leftVertex, *m_rightVertex);
	assert(vtx.m_point != C2DPoint(INFINITY, INFINITY));

	CVertex *newNext = m_rightVertex->m_nextVertex;
	CVertex *newPrev = m_leftVertex->m_prevVertex;

	vtx.m_prevVertex = newPrev;
	vtx.m_nextVertex = newNext;

	vl.push_back (vtx);

	CVertex *vtxPointer = &vl.back();

	newPrev->m_nextVertex = vtxPointer;
	newNext->m_prevVertex = vtxPointer;
	m_leftVertex->m_higher = vtxPointer;
	m_rightVertex->m_higher = vtxPointer;

	m_leftVertex->m_done = true;
	m_rightVertex->m_done = true;

	CIntersection newI(vl, *vtxPointer);

	if (newI.m_height != INFINITY)
		iq.push(newI);

	skeleton.push_back(CSkeletonLine(*m_leftVertex, *vtxPointer));

	CSkeletonLine *lLinePtr = &skeleton.back();

	skeleton.push_back(CSkeletonLine (*m_rightVertex, *vtxPointer));

	CSkeletonLine *rLinePtr = &skeleton.back();

	lLinePtr->m_lower.m_right = m_leftVertex->m_leftSkeletonLine;
	lLinePtr->m_lower.m_left = m_leftVertex->m_rightSkeletonLine;
	lLinePtr->m_higher.m_right = rLinePtr;
	rLinePtr->m_lower.m_right = m_rightVertex->m_leftSkeletonLine;
	rLinePtr->m_lower.m_left = m_rightVertex->m_rightSkeletonLine;
	rLinePtr->m_higher.m_left = lLinePtr;

	if (m_leftVertex->m_leftSkeletonLine)
		m_leftVertex->m_leftSkeletonLine->m_higher.m_left = lLinePtr;
	if (m_leftVertex->m_rightSkeletonLine)
		m_leftVertex->m_rightSkeletonLine->m_higher.m_right = lLinePtr;

	if (m_rightVertex->m_leftSkeletonLine)
		m_rightVertex->m_leftSkeletonLine->m_higher.m_left = rLinePtr;
	if (m_rightVertex->m_rightSkeletonLine)
		m_rightVertex->m_rightSkeletonLine->m_higher.m_right = rLinePtr;

	vtxPointer->m_leftSkeletonLine = lLinePtr;
	vtxPointer->m_rightSkeletonLine = rLinePtr;

	m_leftVertex->m_advancingSkeletonLine = lLinePtr;
	m_rightVertex->m_advancingSkeletonLine = rLinePtr;
}

void CIntersection::ApplyLast3(CSkeleton &skeleton, CVertexList &vl)
{
	assert(m_leftVertex->m_nextVertex == m_rightVertex);
	assert(m_rightVertex->m_prevVertex == m_leftVertex);
	assert(m_leftVertex->m_prevVertex->m_prevVertex == m_rightVertex);
	assert(m_rightVertex->m_nextVertex->m_nextVertex == m_leftVertex);

	CVertex &v1 = *m_leftVertex;
	CVertex &v2 = *m_rightVertex;
	CVertex &v3 = *m_leftVertex->m_prevVertex;

	v1.m_done = true;
	v2.m_done = true;
	v3.m_done = true;

	C2DPoint is1 = v1.m_axis.FacingTowards(v2.m_axis) ? C2DPoint(INFINITY, INFINITY) : v1.m_axis.Intersection(v2.m_axis);
	C2DPoint is2 = v2.m_axis.FacingTowards(v3.m_axis) ? C2DPoint(INFINITY, INFINITY) : v2.m_axis.Intersection(v3.m_axis);
	C2DPoint is3 = v3.m_axis.FacingTowards(v1.m_axis) ? C2DPoint(INFINITY, INFINITY) : v3.m_axis.Intersection(v1.m_axis);

	C2DPoint is = m_poi;
	//assert(is == is1 || is1 == C2DPoint(INFINITY, INFINITY));
	//assert(is == is2 || is2 == C2DPoint(INFINITY, INFINITY));
	//assert(is == is3 || is3 == C2DPoint(INFINITY, INFINITY));

	CVertex v(is);

	v.m_done = true;
	vl.push_back(v);
	CVertex *vtxPointer = &vl.back();

	skeleton.push_back (CSkeletonLine(v1, *vtxPointer));

	CSkeletonLine *line1Ptr = &skeleton.back();

	skeleton.push_back (CSkeletonLine(v2, *vtxPointer));

	CSkeletonLine *line2Ptr = &skeleton.back();

	skeleton.push_back (CSkeletonLine(v3, *vtxPointer));

	CSkeletonLine *line3Ptr = &skeleton.back ();

	line1Ptr->m_higher.m_right = line2Ptr; // zapojeni okridlenych hran
	line2Ptr->m_higher.m_right = line3Ptr;
	line3Ptr->m_higher.m_right = line1Ptr;

	line1Ptr->m_higher.m_left = line3Ptr;
	line2Ptr ->m_higher.m_left = line1Ptr;
	line3Ptr->m_higher.m_left = line2Ptr;

	line1Ptr->m_lower.m_left = v1.m_rightSkeletonLine;
	line1Ptr->m_lower.m_right = v1.m_leftSkeletonLine;

	line2Ptr->m_lower.m_left = v2.m_rightSkeletonLine;
	line2Ptr->m_lower.m_right = v2.m_leftSkeletonLine;

	line3Ptr->m_lower.m_left = v3.m_rightSkeletonLine;
	line3Ptr->m_lower.m_right = v3.m_leftSkeletonLine;

	if (v1.m_leftSkeletonLine)
		v1.m_leftSkeletonLine->m_higher.m_left = line1Ptr;
	if (v1.m_rightSkeletonLine)
		v1.m_rightSkeletonLine->m_higher.m_right = line1Ptr;

	if (v2.m_leftSkeletonLine)
		v2.m_leftSkeletonLine->m_higher.m_left = line2Ptr;
	if (v2.m_rightSkeletonLine)
		v2.m_rightSkeletonLine->m_higher.m_right = line2Ptr;

	if (v3.m_leftSkeletonLine)
		v3.m_leftSkeletonLine->m_higher.m_left = line3Ptr;
	if (v3.m_rightSkeletonLine)
		v3.m_rightSkeletonLine->m_higher.m_right = line3Ptr;

	v1.m_advancingSkeletonLine = line1Ptr;
	v2.m_advancingSkeletonLine = line2Ptr;
	v3.m_advancingSkeletonLine = line3Ptr;
}

