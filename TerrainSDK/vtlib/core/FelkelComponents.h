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

#include <math.h>
#include <assert.h>
#include <vector>
#include <list>

#ifdef _MSC_VER
#pragma warning(disable: 4786)	// prevent common warning about templates
#endif

#ifdef EPS
#include <fstream>
#endif

using namespace std;

#define CN_PI			((CNumber ) 3.14159265358979323846)
#define CN_INFINITY		((CNumber ) 1.797693E+308)

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

// N.B. This is only meant to be a 2D class
// the z coordinate is for convenience

class C2DPoint
{
public:
	C2DPoint (void) : m_x(0), m_y(0), m_z(0) { }
	C2DPoint (CNumber X, CNumber Y) : m_x (X), m_y (Y), m_z(0) { }
	bool operator == (const C2DPoint &p) const { return m_x == p.m_x && m_y == p.m_y; }
	bool operator != (const C2DPoint &p) const { return m_x != p.m_x || m_y != p.m_y; }
	C2DPoint operator - (const C2DPoint &p) const {return C2DPoint(m_x - p.m_x, m_y - p.m_y);}
	C2DPoint operator * (const CNumber &n) const { return C2DPoint (n*m_x, n*m_y); }
	bool IsInfinite (void) { return *this == C2DPoint (CN_INFINITY, CN_INFINITY) ? true : false; }
	CNumber Dist(const C2DPoint &p) {return sqrt ((m_x-p.m_x)*(m_x-p.m_x) + (m_y - p.m_y)*(m_y - p.m_y));}
	CNumber Length() {return sqrt (m_x * m_x + m_y * m_y);}
	CNumber Dot(const C2DPoint &p) const {return m_x * p.m_x + m_y * p.m_y; }
	CNumber Cross(const C2DPoint &p) {return (m_x * p.m_y - m_y * p.m_x);} // 2D pseudo cross

	CNumber m_x, m_y, m_z;
};

typedef vector <C2DPoint> Contour;
typedef vector <Contour> ContourVector;

class CBisector
{
public:
	CBisector(const C2DPoint &p = C2DPoint (0, 0), const C2DPoint &q = C2DPoint(0, 0));
	CBisector(const C2DPoint &p, const CNumber &a) : m_Origin (p), m_Angle (a) { };
	CBisector Opaque(void) const { return CBisector (m_Origin, m_Angle + CN_PI); }
	static CBisector AngleAxis (const C2DPoint &b, const C2DPoint &a, const C2DPoint &c);
	C2DPoint Intersection(const CBisector &a);
	C2DPoint IntersectionAnywhere (const CBisector& a) const;
	bool Colinear(const CBisector &a) const;
	inline bool PointOnBisector (const C2DPoint &p) const { return (p == m_Origin || CBisector(m_Origin, p).m_Angle == m_Angle) ? true : false; }
	inline bool FacingTowards (const CBisector &a) const { return (a.PointOnBisector(m_Origin) && PointOnBisector(a.m_Origin) && !(m_Origin == a.m_Origin)) ? true : false; }
	CNumber Dist(const C2DPoint &p);

	C2DPoint m_Origin;
	CNumber m_Angle;
};

class CVertexList;
class CIntersection;
class CSkeletonLine;

class CVertex
{
public:
	CVertex (void) : m_ID (-1) { };
	CVertex (const C2DPoint &p, const C2DPoint &prev = C2DPoint (), const C2DPoint &next = C2DPoint ())
	: m_point (p), m_axis (CBisector::AngleAxis (p, prev, next)), m_leftLine (p, prev), m_rightLine (p, next), m_higher (NULL),
	m_leftVertex (NULL), m_rightVertex (NULL), m_nextVertex (NULL), m_prevVertex (NULL), m_done (false), m_ID (-1),
	m_leftSkeletonLine (NULL), m_rightSkeletonLine (NULL), m_advancingSkeletonLine (NULL) { }
	CVertex (const C2DPoint &p, CVertex &left, CVertex &right);
	CVertex *Highest (void) { return m_higher ? m_higher -> Highest () : this; }
	bool AtContour (void) const { return m_leftVertex == this && m_rightVertex == this; }
	bool operator == (const CVertex &v) const { return m_point == v.m_point; }
	bool operator < (const CVertex &) const { assert (false); return false; }
//	bool IntersectionFromLeft (const CBisector &l);
//	bool IntersectionFromRight (const CBisector &l);
	C2DPoint CoordinatesOfAnyIntersectionOfTypeB(const CVertex &left, const CVertex &right);
	C2DPoint IntersectionOfTypeB(const CVertex &left, const CVertex &right);
	CNumber NearestIntersection (CVertexList &vl, CVertex **left, CVertex **right, C2DPoint &p);
	bool InvalidIntersection(CVertexList &vl, const CIntersection &is);
	// data
	C2DPoint m_point;
	CBisector m_axis;
	CBisector m_leftLine, m_rightLine;
	CVertex *m_leftVertex, *m_rightVertex;
	CVertex *m_nextVertex, *m_prevVertex;
	CVertex *m_higher;
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
		assert (x.m_prevVertex == NULL || x.m_leftLine.FacingTowards (x.m_prevVertex -> m_rightLine));
		assert (x.m_nextVertex == NULL || x.m_rightLine.FacingTowards (x.m_nextVertex -> m_leftLine));
		((CVertex &)x).m_ID = size ();	   // automaticke cislovani
		list <CVertex> :: push_back (x);
	}
};

class CSegment
{
public:
	CSegment (const C2DPoint &p = C2DPoint(), const C2DPoint &q = C2DPoint()) : m_a(p), m_b(q) { };
	CSegment (CNumber x1, CNumber y1, CNumber x2, CNumber y2) : m_a (x1, y1), m_b (x2, y2) { }
	CNumber Dist(const C2DPoint &p);

private:
	C2DPoint m_a;
	C2DPoint m_b;
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
	bool operator < (const CSkeletonLine &) const { assert (false); return false; }
	int m_ID;
};


class CSkeleton : public list <CSkeletonLine>
{
public:
	void push_back (const CSkeletonLine &x)
	{
#ifdef EPS
		extern ostream *epsStream;

		*epsStream << x.m_lower.m_vertex -> m_point.m_x << ' ' << x.m_lower.m_vertex -> m_point.m_y << " moveto ";
		*epsStream << x.m_higher.m_vertex -> m_point.m_x << ' ' << x.m_higher.m_vertex -> m_point.m_y << " lineto\n";
#endif
		((CSkeletonLine &)x).m_ID = size ();	 // automaticke cislovani
		list <CSkeletonLine> :: push_back (x);
	}
};

#endif // FELKELCOMPONENTSH
