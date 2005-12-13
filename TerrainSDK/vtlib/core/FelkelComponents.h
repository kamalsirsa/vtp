//
// FelkelComponents.h
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Straight skeleton algorithm and original implementation
// courtesy of Petr Felkel and Stepan Obdrzalek (petr.felkel@tiani.com)
// Re-implemented for the Virtual Terrain Project (vterrain.org)
// by Roger James (www.beardandsandals.co.uk)
//

#ifndef FELKELCOMPONENTSH
#define FELKELCOMPONENTSH

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <math.h>
#include <assert.h>
#include <vector>
#include <list>

#include "vtdata/vtLog.h"

#ifdef _MSC_VER
#pragma warning(disable: 4786)	// prevent common warning about templates
#include <limits>
#endif

#ifdef EPS
#include <fstream>
#endif

using namespace std;

#define CN_PI			((CNumber ) 3.14159265358979323846)
#ifdef _MSC_VER
//#define CN_INFINITY		((CNumber ) std::numeric_limits<double>::infinity())
#define CN_INFINITY		((CNumber ) DBL_MAX)
#else
#define CN_INFINITY		((CNumber ) 1.797693E+308)
#endif
#define CN_SLOPE_MAX	((CNumber ) CN_PI * 89 /180)
#define CN_SLOPE_MIN	((CNumber ) CN_PI / 180)

#define MIN_DIFF		0.00005	// For vtp this gives about a 5 cm resolution
#define MIN_ANGLE_DIFF	0.00005

#define SIMILAR(a,b) ((a)-(b) < MIN_DIFF && (b)-(a) < MIN_DIFF)
#define ANGLE_SIMILAR(a,b) (a.NormalizedAngle() - b.NormalizedAngle() < MIN_ANGLE_DIFF && b.NormalizedAngle() - a.NormalizedAngle() < MIN_ANGLE_DIFF)


class CNumber
{
public:
	// Constructor
	CNumber (double x = 0.0);
	// Operator overloads
	//  operator const double& (void) const { return n; }
	CNumber& operator = (const CNumber &x) { m_n = x.m_n; return *this; }
	CNumber& operator = (const double &x) { m_n = x; return *this; }
	operator double& (void) const { return (double &) m_n; }
	bool operator == (const CNumber &x) const { return SIMILAR (m_n, x.m_n); }
	bool operator != (const CNumber &x) const { return !SIMILAR (m_n, x.m_n); }
	bool operator <= (const CNumber &x) const { return m_n < x.m_n || *this == x; }
	bool operator >= (const CNumber &x) const { return m_n > x.m_n || *this == x; }
	bool operator <  (const CNumber &x) const { return m_n < x.m_n && *this != x; }
	bool operator >  (const CNumber &x) const { return m_n > x.m_n && *this != x; }

	bool operator == (const double x) const { return *this == CNumber (x); }
	bool operator != (const double x) const { return *this != CNumber (x); }
	bool operator <= (const double x) const { return *this <= CNumber (x); }
	bool operator >= (const double x) const { return *this >= CNumber (x); }
	bool operator <  (const double x) const { return *this <  CNumber (x); }
	bool operator >  (const double x) const { return *this >  CNumber (x); }
	// Functions
	CNumber &NormalizeAngle();
	CNumber NormalizedAngle();
private:
	double m_n;
};

// This is not really a 3D class beware !!!!!!
// Most things only work on x and z co-ords

// This is not really a 3D class beware !!!!!!
// Most things only work on x and z co-ords

class C3DPoint
{
public:
	C3DPoint (void) : m_x(0), m_y(0), m_z(0) { }
	C3DPoint (CNumber X, CNumber Y, CNumber Z) : m_x (X), m_y (Y), m_z(Z) { }
	bool operator == (const C3DPoint &p) const { return m_x == p.m_x && m_z == p.m_z; }
	bool operator != (const C3DPoint &p) const { return m_x != p.m_x || m_z != p.m_z; }
	C3DPoint operator - (const C3DPoint &p) const {return C3DPoint(m_x - p.m_x, m_y, m_z - p.m_z);}
	C3DPoint operator * (const CNumber &n) const { return C3DPoint (n*m_x, m_y, n*m_z); }
	bool IsInfiniteXZ (void) { return *this == C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY) ? true : false; }
	CNumber DistXZ(const C3DPoint &p) const {return sqrt ((m_x-p.m_x)*(m_x-p.m_x) + (m_z - p.m_z)*(m_z - p.m_z));}
	CNumber LengthXZ() {return sqrt (m_x * m_x + m_z * m_z);}
	CNumber DotXZ(const C3DPoint &p) const {return m_x * p.m_x + m_z * p.m_z; }
	CNumber CrossXZ(const C3DPoint &p) {return (m_x * p.m_z - m_z * p.m_x);} // 2D pseudo cross

	CNumber m_x, m_y, m_z;
};

class CEdge
{
public:
	CEdge (CNumber X, CNumber Y, CNumber Z, CNumber Slope) : m_Point(X, Y, Z), m_Slope(Slope) {};
	bool operator == (const CEdge &p) const { return m_Point == p.m_Point; }
	C3DPoint m_Point;
	CNumber m_Slope;
};

typedef vector <CEdge> Contour;
typedef vector <Contour> ContourVector;

class CRidgeLine
{
public:
	CRidgeLine(const C3DPoint &p = C3DPoint (0, 0, 0), const C3DPoint &q = C3DPoint(0, 0, 0), const CNumber &Slope = -1, const bool IsRidgeLine = false);
	CRidgeLine(const C3DPoint &p, const CNumber &a, const CNumber &Slope, const bool IsRidgeLine = true) : m_Origin (p), m_Angle (a), m_Slope(Slope), m_IsRidgeLine(IsRidgeLine)
	{
		m_Angle.NormalizeAngle();
	};
	CRidgeLine Opaque(void) const { return CRidgeLine (m_Origin, m_Angle + CN_PI, m_Slope); }
	static CRidgeLine AngleAxis (const C3DPoint &b, const C3DPoint &a, const CNumber &sa, const C3DPoint &c, const CNumber &sc);
	C3DPoint Intersection(const CRidgeLine &a);
	C3DPoint IntersectionAnywhere (const CRidgeLine& a) const;
	bool Colinear(const CRidgeLine &a) const;
	inline bool PointOnRidgeLine (const C3DPoint &p) const
	{
		return (p == m_Origin || CRidgeLine(m_Origin, p, -1).m_Angle == m_Angle) ? true : false;
	}
	inline bool FacingTowards (const CRidgeLine &a) const
	{
//		return (a.PointOnRidgeLine(m_Origin) && PointOnRidgeLine(a.m_Origin) && !(m_Origin == a.m_Origin)) ? true : false;
		if (m_IsRidgeLine != a.m_IsRidgeLine)
			VTLOG("%s %d m_IsRidgeLine %d\n", __FILE__, __LINE__, m_IsRidgeLine);
		if (m_IsRidgeLine)
			return (a.PointOnRidgeLine(m_Origin) && PointOnRidgeLine(a.m_Origin) && !(m_Origin == a.m_Origin) && SIMILAR(m_Slope, 0.0) && SIMILAR(m_Slope, 0.0)) ? true : false;
		else
			return (a.PointOnRidgeLine(m_Origin) && PointOnRidgeLine(a.m_Origin) && !(m_Origin == a.m_Origin)) ? true : false;
	}
	CNumber Dist(const C3DPoint &p) const;


	C3DPoint m_Origin;
	CNumber m_Angle;
	CNumber m_Slope;
	bool m_IsRidgeLine;
};

class CSkeletonLine;
class CVertexList;
class CIntersection;

class CVertex
{
public:
	CVertex (void) : m_ID (-1) { };
	CVertex (const C3DPoint &p, const C3DPoint &prev = C3DPoint (), const CNumber &prevslope = -1, const C3DPoint &next = C3DPoint (), const CNumber &nextslope = -1)
	: m_point (p), m_axis (CRidgeLine::AngleAxis (p, prev, prevslope,  next, nextslope)), m_leftLine (p, prev, prevslope), m_rightLine (p, next, nextslope), m_higher (NULL),
	m_leftVertex (NULL), m_rightVertex (NULL), m_nextVertex (NULL), m_prevVertex (NULL), m_done (false), m_ID (-1),
	m_leftSkeletonLine (NULL), m_rightSkeletonLine (NULL), m_advancingSkeletonLine (NULL) { }
	CVertex (const C3DPoint &p, CVertex &left, CVertex &right);
	CVertex *Highest (void) { return m_higher ? m_higher -> Highest () : this; }
	bool AtContour (void) const { return m_leftVertex == this && m_rightVertex == this; }
	bool operator == (const CVertex &v) const { return m_point == v.m_point; }
	bool operator < (const CVertex &) const { VTLOG("%s %d Assert failed\n", __FILE__, __LINE__); return false; }
	C3DPoint CoordinatesOfAnyIntersectionOfTypeB(const CVertex &left, const CVertex &right);
	C3DPoint IntersectionOfTypeB(const CVertex &left, const CVertex &right);
	CNumber NearestIntersection (CVertexList &vl, CVertex **left, CVertex **right, C3DPoint &p);
	bool InvalidIntersection(CVertexList &vl, const CIntersection &is);
	// data
	C3DPoint m_point;
	CRidgeLine m_axis;  // the axis (ridgeline) for this vertex
	CRidgeLine m_leftLine, m_rightLine; // vectors for the original left and right edge contour lines
	CVertex *m_leftVertex, *m_rightVertex; // Current contour chain (List of active vertices)
	CVertex *m_nextVertex, *m_prevVertex; // Overall vertex list
	CVertex *m_higher; // chain to next higher point in the skeleton (set when intersection is applied)
	bool m_done;
	int m_ID;
	CSkeletonLine *m_leftSkeletonLine, *m_rightSkeletonLine, *m_advancingSkeletonLine;
};

class CVertexList : public list <CVertex>
{
public:
	CVertexList (void) { }
	iterator prev (const iterator &i) { iterator tmp (i); if (tmp == begin ()) tmp = end (); tmp --; return tmp; }
	iterator next (const iterator &i) { iterator tmp (i); tmp ++; if (tmp == end ()) tmp = begin (); return tmp; }
	void push_back (const CVertex& x)
	{
#if VTDEBUG
		if (!(x.m_prevVertex == NULL || x.m_leftLine.FacingTowards (x.m_prevVertex -> m_rightLine)))
			VTLOG("%s %d Assert failed\n", __FILE__, __LINE__);
		if (!(x.m_nextVertex == NULL || x.m_rightLine.FacingTowards (x.m_nextVertex -> m_leftLine)))
			VTLOG("%s %d Assert failed\n", __FILE__, __LINE__);
#endif
		((CVertex &)x).m_ID = size ();	// automatic ID generation
		list <CVertex> :: push_back (x);
#ifdef FELKELDEBUG
		VTLOG("Vertex %d x %f y %f z %f angle %f\n left %d right %d prev %d next %d added to list\n",
			((CVertex &)x).m_ID, ((CVertex &)x).m_point.m_x, ((CVertex &)x).m_point.m_y, ((CVertex &)x).m_point.m_z, ((CVertex &)x).m_axis.m_Angle,
			(((CVertex &)x).m_leftVertex == NULL) ? -999 : ((CVertex &)x).m_leftVertex->m_ID,
			(((CVertex &)x).m_rightVertex == NULL) ? -999 : ((CVertex &)x).m_rightVertex->m_ID,
			(((CVertex &)x).m_prevVertex == NULL) ? -999 : ((CVertex &)x).m_prevVertex->m_ID,
			(((CVertex &)x).m_nextVertex == NULL) ? -999 : ((CVertex &)x).m_nextVertex->m_ID);
#endif
	}
};

class CSegment
{
public:
	CSegment (const C3DPoint &p = C3DPoint(), const C3DPoint &q = C3DPoint()) : m_a(p), m_b(q) { };
	CSegment (CNumber x1, CNumber z1, CNumber x2, CNumber z2) : m_a (x1, 0, z1), m_b (x2, 0, z2) { }
	CNumber Dist(const C3DPoint &p);

private:
	C3DPoint m_a;
	C3DPoint m_b;
};

class CSkeletonLine
{
public:
	CSkeletonLine (void) : m_ID (-1) { }
	CSkeletonLine (const CVertex &l, const CVertex &h) : m_lower (l), m_higher (h), m_ID (-1) { };
	operator CSegment (void) { return CSegment (m_lower.m_vertex -> m_point, m_higher.m_vertex -> m_point); }

	struct SkeletonPoint
	{
		SkeletonPoint (const CVertex &v = CVertex (), CSkeletonLine *l = NULL, CSkeletonLine *r = NULL) : m_vertex (&v), m_left (l), m_right (r) { }
		const CVertex *m_vertex;
		CSkeletonLine *m_left, *m_right;
		int LeftID (void) const { if (!m_left) return -1; return m_left -> m_ID; }
		int RightID (void) const { if (!m_right) return -1; return m_right -> m_ID; }
		int VertexID (void) const { if (!m_vertex) return -1; return m_vertex -> m_ID; }
	} m_lower, m_higher;
	bool operator == (const CSkeletonLine &s) const
	{
		return m_higher.m_vertex -> m_ID == s.m_higher.m_vertex -> m_ID  && m_lower.m_vertex -> m_ID  == s.m_lower.m_vertex -> m_ID ;
	}
	bool operator < (const CSkeletonLine &) const { VTLOG("%s %d Assert failed\n", __FILE__, __LINE__); return false; }
	int m_ID;
};


class CSkeleton : public list <CSkeletonLine>
{
public:
	void push_back (const CSkeletonLine &x)
	{
		((CSkeletonLine &)x).m_ID = size ();	// automatically assign the ID number
#ifdef FELKELDEBUG
		{
			VTLOG("New skeleton line %d lower %d higher %d\n",
				((CSkeletonLine &)x).m_ID,
				((CSkeletonLine &)x).m_lower.m_vertex->m_ID,
				((CSkeletonLine &)x).m_higher.m_vertex->m_ID);
		}
#endif
		list <CSkeletonLine> :: push_back (x);
	}
};

#endif	// DOXYGEN_SHOULD_SKIP_THIS
#endif	// FELKELCOMPONENTSH
