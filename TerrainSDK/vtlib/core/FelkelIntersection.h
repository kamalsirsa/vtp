//
// FelkelIntersection.h: interface for the CIntersection class.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Straight skeleton algorithm and original implementation
// courtesy of Petr Felkel and Stepan Obdrzalek (petr.felkel@tiani.com)
// Re-implemented for the Virtual Terrain Project (vterrain.org)
// by Roger James (www.beardandsandals.co.uk)
//

#ifndef FELKELINTERSECTIONH
#define FELKELINTERSECTIONH

#include <queue>

#include "FelkelComponents.h"
class CSkeleton;

typedef priority_queue <CIntersection, deque <CIntersection>, greater <CIntersection> > IntersectionQueue;

class CIntersection
{
public:
	CIntersection (void) { };
	CIntersection (CVertexList &vl, CVertex &v);

	void ApplyNonconvexIntersection(CSkeleton &skeleton, CVertexList &vl, IntersectionQueue &iq);
	void ApplyConvexIntersection(CSkeleton &skeleton, CVertexList &vl, IntersectionQueue &iq);
	void ApplyLast3(CSkeleton &skeleton, CVertexList &vl);

	C2DPoint m_poi;
	CVertex *m_leftVertex, *m_rightVertex;
	CNumber m_height;
	enum Type { CONVEX, NONCONVEX } m_type;

	operator < (const CIntersection &i) const { return m_height < i.m_height; }  // pro usporadani v prioritni fronte
	operator > (const CIntersection &i) const { return m_height > i.m_height; }  // pro usporadani v prioritni fronte
	operator == (const CIntersection &i) const { return m_poi == i.m_poi; }
};

#endif // FELKELINTERSECTIONH

