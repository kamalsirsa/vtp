//
// FelkelComponents.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Straight skeleton algorithm and original implementation
// courtesy of Petr Felkel and Stepan Obdrzalek (petr.felkel@tiani.com)
// Re-implemented for the Virtual Terrain Project (vterrain.org)
// by Roger James (www.beardandsandals.co.uk)
//

#include "FelkelComponents.h"
#include "FelkelIntersection.h"

//
// Implementation of the CBisector class.
//

CBisector::CBisector(const C2DPoint &p, const C2DPoint &q)
: m_Origin(p)
{
	double d = atan2 (q.m_y - p.m_y, q.m_x - p.m_x);
	m_Angle = d;
}

CBisector CBisector::AngleAxis (const C2DPoint &b, const C2DPoint &a,
	const C2DPoint &c)
{
	CBisector ba (b, a);
	CBisector bc (b, c);
	CNumber theta1;

	if (ba.m_Angle > bc.m_Angle) ba.m_Angle = ba.m_Angle - 2*CN_PI;

	theta1 = (ba.m_Angle + bc.m_Angle) / 2;

	return CBisector (b, theta1);
}

C2DPoint CBisector::Intersection(const CBisector &a)
{
	if (m_Origin == a.m_Origin) return m_Origin;
	if (PointOnBisector (a.m_Origin) && a.PointOnBisector(m_Origin))
		return C2DPoint ((m_Origin.m_x + a.m_Origin.m_x)/2, (m_Origin.m_y + a.m_Origin.m_y)/2);
	if (PointOnBisector (a.m_Origin)) return a.m_Origin;
	if (a.PointOnBisector (m_Origin)) return m_Origin;
	if (Colinear(a)) return C2DPoint(CN_INFINITY, CN_INFINITY);

	CNumber sa = sin (m_Angle);
	CNumber sb = sin (a.m_Angle);
	CNumber ca = cos (m_Angle);
	CNumber cb = cos (a.m_Angle);
	CNumber x = sb*ca - sa*cb;

	if (x == 0.0) return C2DPoint (CN_INFINITY, CN_INFINITY);
	CNumber u = (cb*(m_Origin.m_y - a.m_Origin.m_y) - sb*(m_Origin.m_x - a.m_Origin.m_x))/x;
	if (u != 0.0 && u < 0.0) return C2DPoint (CN_INFINITY, CN_INFINITY);
	if ((ca*(a.m_Origin.m_y - m_Origin.m_y) - sa*(a.m_Origin.m_x - m_Origin.m_x))/x > 0) return C2DPoint (CN_INFINITY, CN_INFINITY);
	return C2DPoint (m_Origin.m_x + u*ca, m_Origin.m_y + u*sa);
};

C2DPoint CBisector::IntersectionAnywhere (const CBisector& a) const
{
	CBisector OpaqueBisector1, OpaqueBisector2;

	if (m_Origin == a.m_Origin) return m_Origin;
	if (PointOnBisector (a.m_Origin) && a.PointOnBisector(m_Origin))
		return C2DPoint ((m_Origin.m_x + a.m_Origin.m_x)/2, (m_Origin.m_y + a.m_Origin.m_y)/2);
	if (PointOnBisector (a.m_Origin)) return a.m_Origin;
	if (a.PointOnBisector (m_Origin)) return m_Origin;
	if (Colinear(a)) return C2DPoint(CN_INFINITY, CN_INFINITY);

	OpaqueBisector1 = Opaque();
	OpaqueBisector2 = a.Opaque();

	if (OpaqueBisector1.PointOnBisector(a.m_Origin) && OpaqueBisector2.PointOnBisector(m_Origin))
		return C2DPoint ((m_Origin.m_x + a.m_Origin.m_x)/2, (m_Origin.m_y + a.m_Origin.m_y)/2);

	CNumber sa = sin (m_Angle);
	CNumber sb = sin (a.m_Angle);
	CNumber ca = cos (m_Angle);
	CNumber cb = cos (a.m_Angle);
	CNumber x = sb*ca - sa*cb;

	if (x == 0.0) return C2DPoint (CN_INFINITY, CN_INFINITY);
	CNumber u = (cb*(m_Origin.m_y - a.m_Origin.m_y) - sb*(m_Origin.m_x - a.m_Origin.m_x))/x;
	return C2DPoint (m_Origin.m_x + u*ca, m_Origin.m_y + u*sa);
};


bool CBisector::Colinear(const CBisector &a) const
{
	CNumber aa = m_Angle;
	CNumber ba = a.m_Angle;
	CNumber aa2 = m_Angle + CN_PI;

	aa.NormalizeAngle();
	ba.NormalizeAngle();
	aa2.NormalizeAngle();
	return (ba == aa || ba == aa2) ? true : false;
}

CNumber CBisector::Dist(const C2DPoint &p)
{
	CNumber a = m_Angle - CBisector(m_Origin, p).m_Angle;
	CNumber d = sin (a) * m_Origin.Dist(p);
	if (d < 0.0) return -d;
	return d;
}

//
// implementation of the CNumber class.
//

CNumber::CNumber(double x)
{
	m_n = x;
}

CNumber& CNumber::NormalizeAngle()
{
	if (m_n >=  CN_PI)
	{
		m_n = m_n - 2*CN_PI;
		return NormalizeAngle();
	}
	if (m_n < -CN_PI)
	{
		m_n = m_n + 2*CN_PI;
		return NormalizeAngle();
	}
	return *this;
}

CNumber CNumber::NormalizedAngle()
{
	CNumber temp = *this;

	temp.NormalizeAngle();

	return temp;
}

//
// Implementation of the CVertex class.
//

//
// Constructor
//
CVertex::CVertex (const C2DPoint &p, CVertex &left, CVertex &right)
: m_point (p), m_done (false), m_higher (NULL), m_ID (-1),
m_leftSkeletonLine (NULL), m_rightSkeletonLine (NULL),
m_advancingSkeletonLine (NULL)
{
	m_leftLine = left.m_leftLine;
	m_rightLine = right.m_rightLine;
	m_leftVertex = &left;
	m_rightVertex = &right;

//	assert (m_leftLine.Dist(m_point) == m_rightLine.Dist(m_point));
	C2DPoint i = m_leftLine.Intersection (m_rightLine);
	if (i.m_x == CN_INFINITY)
	{
		assert (i.m_y == CN_INFINITY);
		i = m_leftLine.IntersectionAnywhere (m_rightLine);
		if (i.m_x == CN_INFINITY)
		{
			assert (i.m_y == CN_INFINITY);
			m_axis = CBisector(m_point, m_leftLine.m_Angle);
		}
		else
		{
			m_axis = CBisector(m_point, i);
			m_axis.m_Angle = m_axis.m_Angle + CN_PI;
		}
	}
	else
	{
		m_axis = CBisector(m_point, i);
	}
}
/*
bool CVertex::IntersectionFromLeft (const CBisector &l)
{
	if (m_axis.Intersection(l) != C2DPoint(CN_INFINITY, CN_INFINITY)) return false;
	if (m_rightVertex == this) return false;
	if (m_rightVertex->m_axis.Intersection(l) != C2DPoint(CN_INFINITY, CN_INFINITY)) return true;
	return m_rightVertex->IntersectionFromLeft(l);
}

bool CVertex::IntersectionFromRight (const CBisector &l)
{
	if (m_axis.Intersection(l) != C2DPoint(CN_INFINITY, CN_INFINITY)) return false;
	if (m_leftVertex == this) return false;
	if (m_leftVertex->m_axis.Intersection(l) != C2DPoint(CN_INFINITY, CN_INFINITY)) return true;
	return m_leftVertex->IntersectionFromRight(l);
}
*/
C2DPoint CVertex::CoordinatesOfAnyIntersectionOfTypeB (const CVertex &left, const CVertex &right)
{
	C2DPoint p1 = m_rightLine.IntersectionAnywhere(right.m_leftLine);
	C2DPoint p2 = m_leftLine.IntersectionAnywhere(left.m_rightLine);
	C2DPoint poi (CN_INFINITY, CN_INFINITY);

	if (p1 != C2DPoint(CN_INFINITY, CN_INFINITY) && p2 != C2DPoint(CN_INFINITY, CN_INFINITY))
	{
		if (m_rightLine.PointOnBisector(p1)) return C2DPoint(CN_INFINITY, CN_INFINITY);
		if (m_leftLine.PointOnBisector(p2))  return C2DPoint(CN_INFINITY, CN_INFINITY);
		poi = CBisector::AngleAxis(p1, p2, m_point).IntersectionAnywhere (m_axis);
	}
	else //if (p1 != Point (CN_INFINITY, CN_INFINITY))
	{
		poi = left.m_rightLine.IntersectionAnywhere(m_axis);
		poi.m_x = (poi.m_x + m_point.m_x) / 2;
		poi.m_y = (poi.m_y + m_point.m_y) / 2;
	}
	return poi;
}

C2DPoint CVertex::IntersectionOfTypeB (const CVertex &left, const CVertex &right)
{
	assert (m_prevVertex == NULL || m_leftLine.FacingTowards(m_prevVertex->m_rightLine));
	assert (m_nextVertex == NULL || m_rightLine.FacingTowards(m_nextVertex->m_leftLine));
	assert (left.m_prevVertex == NULL || left.m_leftLine.FacingTowards(left.m_prevVertex->m_rightLine));
	assert (left.m_nextVertex == NULL || left.m_rightLine.FacingTowards(left.m_nextVertex->m_leftLine));
	assert (right.m_prevVertex == NULL || right.m_leftLine.FacingTowards(right.m_prevVertex->m_rightLine));
	assert (right.m_nextVertex == NULL || right.m_rightLine.FacingTowards(right.m_nextVertex->m_leftLine));

	C2DPoint pl(m_axis.Intersection(left.m_rightLine));
	C2DPoint pr(m_axis.Intersection(right.m_leftLine));
	if (pl == C2DPoint(CN_INFINITY, CN_INFINITY) && pr == C2DPoint(CN_INFINITY, CN_INFINITY))
		return C2DPoint(CN_INFINITY, CN_INFINITY);

	C2DPoint p;
	if (pl != C2DPoint(CN_INFINITY, CN_INFINITY)) p = pl;
	if (pr != C2DPoint(CN_INFINITY, CN_INFINITY)) p = pr;
	assert (p != C2DPoint(CN_INFINITY, CN_INFINITY));
// Dont know why this triggers......Needs fixing some day!!!!!!!!
//	assert (pl == C2DPoint(CN_INFINITY, CN_INFINITY) || pr == C2DPoint(CN_INFINITY, CN_INFINITY) || pl == pr);

	C2DPoint poi = CoordinatesOfAnyIntersectionOfTypeB(left, right);
	CNumber al = left.m_axis.m_Angle - left.m_rightLine.m_Angle;
	CNumber ar = right.m_axis.m_Angle - right.m_leftLine.m_Angle;

	CNumber alp = CBisector(left.m_point, poi).m_Angle - left.m_rightLine.m_Angle;
	CNumber arp = CBisector(right.m_point, poi).m_Angle - right.m_leftLine.m_Angle;

	al.NormalizeAngle(); ar.NormalizeAngle(); alp.NormalizeAngle(); arp.NormalizeAngle();
//	assert (al <= 0.0);
//	assert (ar >= 0.0 || ar == -CN_PI);

	if ((alp > 0.0 || alp < al) && !ANGLE_SIMILAR(alp, CNumber(0)) && !ANGLE_SIMILAR(alp, al))
		return C2DPoint (CN_INFINITY, CN_INFINITY);
	if ((arp < 0.0 || arp > ar) && !ANGLE_SIMILAR(arp, CNumber(0)) && !ANGLE_SIMILAR(arp, ar))
		return C2DPoint (CN_INFINITY, CN_INFINITY);
	return poi;
}

CNumber CVertex::NearestIntersection (CVertexList &vl, CVertex **left, CVertex **right, C2DPoint &p)
{
	CNumber minDist = CN_INFINITY;
	CVertexList::iterator minI = vl.end ();
	CVertexList :: iterator i;
	for (i = vl.begin (); i != vl.end (); i++)
	{
		if ((*i).m_done) continue;
		if ((*i).m_nextVertex == NULL || (*i).m_prevVertex == NULL) continue;
		if (&*i == this || (*i).m_nextVertex == this) continue;
		assert ((*i).m_rightVertex != NULL);
		assert ((*i).m_leftVertex != NULL);
		C2DPoint poi = IntersectionOfTypeB((*i), *(*i).m_nextVertex);
		if (poi == C2DPoint (CN_INFINITY, CN_INFINITY)) continue;
		CNumber d = poi.Dist(m_point);
		if (d < minDist) { minDist = d; minI = i; }
	}
	if (minDist == CN_INFINITY)
		return CN_INFINITY;

	i = minI;
	C2DPoint poi = CoordinatesOfAnyIntersectionOfTypeB((*i), *(*i).m_nextVertex);

	CNumber d = m_leftLine.Dist(poi);
//	assert (d == m_rightLine.Dist(poi));
//	assert (d == (*i).m_rightLine.Dist(poi));
//	assert (d == (*i).m_nextVertex->m_leftLine.Dist(poi));

	p = poi;
	*left = (CVertex *) &*i;
	*right = (*i).m_nextVertex;

	return d;
}

bool CVertex::InvalidIntersection (CVertexList &vl, const CIntersection &is)
{
	for (CVertexList :: iterator i = vl.begin (); i != vl.end (); i++)
	{
		if ((*i).m_done) continue;
		if ((*i).m_nextVertex == NULL || (*i).m_prevVertex == NULL) continue;
		C2DPoint poi = m_axis.Intersection((*i).m_axis);
		if (poi == C2DPoint (CN_INFINITY, CN_INFINITY)) continue;
		if (&*i == is.m_leftVertex || &*i == is.m_rightVertex) continue;

		CNumber dv = m_leftLine.Dist(poi);
		CNumber dvx = m_rightLine.Dist(poi);
		//		assert (SIMILAR (dv, dist (poi, v.rightLine)));
//		assert (dv == dvx);
		if (dv > is.m_height) continue;

		CNumber di = (*i).m_leftLine.Dist(poi);
//		assert (di == (*i).m_rightLine.Dist(poi));
		if (di > dv + MIN_DIFF) continue;
		//		if (di > is.height) continue;

		return true;
	}
	return false;
}
