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
// Implementation of the CRidgeLine class.
//

CRidgeLine::CRidgeLine(const C3DPoint &p, const C3DPoint &q, const CNumber &Slope, const bool IsRidgeLine)
          : m_Origin(p)
{
	m_Angle = atan2 (q.m_z - p.m_z, q.m_x - p.m_x);
	m_Slope = Slope;
	m_IsRidgeLine = IsRidgeLine;
}

CRidgeLine CRidgeLine::AngleAxis (const C3DPoint &b, const C3DPoint &a, const CNumber &sa, const C3DPoint &c, const CNumber &sc)
{
	CNumber theta;
	CNumber theta1;
	CNumber theta2;
	CRidgeLine ba (b, a, -1);
	CRidgeLine bc (b, c, -1);


	theta = bc.m_Angle - ba.m_Angle;


	theta1 = atan(sin(theta)/(cos(theta) + tan(sa)/tan(sc)));
	theta1 += ba.m_Angle;

	if (ba.m_Angle > bc.m_Angle)
		ba.m_Angle = ba.m_Angle - 2*CN_PI;

	if (fabs(bc.m_Angle - ba.m_Angle) > CN_PI)
		// Reflex angle flip the angle
		theta1 += CN_PI;

	theta2 = theta1 - ba.m_Angle;
	theta2 = atan(sin(theta2) * tan(sa));

	return CRidgeLine (b, theta1, theta2, true);
}

C3DPoint CRidgeLine::Intersection(const CRidgeLine &a)
{
	if (m_Origin == a.m_Origin)
		return m_Origin;
	if (PointOnRidgeLine(a.m_Origin) && a.PointOnRidgeLine(m_Origin))
	{
		if (m_IsRidgeLine && a.m_IsRidgeLine)
		{
			CNumber Ratio = tan(m_Slope) / tan(a.m_Slope);
			return C3DPoint (m_Origin.m_x + Ratio * (a.m_Origin.m_x - m_Origin.m_x) / (Ratio + 1),
							0,
							m_Origin.m_z + Ratio * (a.m_Origin.m_z - m_Origin.m_z) / (Ratio + 1));
		}
		else
			return C3DPoint ((m_Origin.m_x + a.m_Origin.m_x)/2, 0, (m_Origin.m_z + a.m_Origin.m_z)/2);
	}
	if (PointOnRidgeLine (a.m_Origin))
		return a.m_Origin;
	if (a.PointOnRidgeLine (m_Origin))
		return m_Origin;
	if (Colinear(a))
		return C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY);

	CNumber sa = sin (m_Angle);
	CNumber sb = sin (a.m_Angle);
	CNumber ca = cos (m_Angle);
	CNumber cb = cos (a.m_Angle);
	CNumber x = sb*ca - sa*cb;

	if (x == 0.0)
		return C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY);
	CNumber u = (cb*(m_Origin.m_z - a.m_Origin.m_z) - sb*(m_Origin.m_x - a.m_Origin.m_x))/x;
	if (u != 0.0 && u < 0.0)
		return C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY);
	if ((ca*(a.m_Origin.m_z - m_Origin.m_z) - sa*(a.m_Origin.m_x - m_Origin.m_x))/x > 0)
		return C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY);
	return C3DPoint (m_Origin.m_x + u*ca, 0, m_Origin.m_z + u*sa);
};

C3DPoint CRidgeLine::IntersectionAnywhere (const CRidgeLine& a) const
{
	CRidgeLine OpaqueRidgeLine1 , OpaqueRidgeLine2;

	if (m_Origin == a.m_Origin)
		return m_Origin;
	if (PointOnRidgeLine (a.m_Origin) && a.PointOnRidgeLine(m_Origin))
	{
		if (m_IsRidgeLine && a.m_IsRidgeLine)
		{
			CNumber Ratio = tan(m_Slope) / tan(a.m_Slope);
			return C3DPoint (m_Origin.m_x + Ratio * (a.m_Origin.m_x - m_Origin.m_x) / (Ratio + 1),
							0,
							m_Origin.m_z + Ratio * (a.m_Origin.m_z - m_Origin.m_z) / (Ratio + 1));
		}
		else
			return C3DPoint ((m_Origin.m_x + a.m_Origin.m_x)/2, 0, (m_Origin.m_z + a.m_Origin.m_z)/2);
	}
	if (PointOnRidgeLine (a.m_Origin))
		return a.m_Origin;
	if (a.PointOnRidgeLine (m_Origin))
		return m_Origin;
	if (Colinear(a))
		return C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY);

	OpaqueRidgeLine1 = Opaque();
	OpaqueRidgeLine2 = a.Opaque();

	if (OpaqueRidgeLine1.PointOnRidgeLine(a.m_Origin) && OpaqueRidgeLine2.PointOnRidgeLine(m_Origin))
	{
		if (m_IsRidgeLine && a.m_IsRidgeLine)
		{
			CNumber Ratio = tan(m_Slope) / tan(a.m_Slope);
			return C3DPoint (m_Origin.m_x + Ratio * (a.m_Origin.m_x - m_Origin.m_x) / (Ratio + 1),
							0,
							m_Origin.m_z + Ratio * (a.m_Origin.m_z - m_Origin.m_z) / (Ratio + 1));
		}
		else
			return C3DPoint ((m_Origin.m_x + a.m_Origin.m_x)/2, 0, (m_Origin.m_z + a.m_Origin.m_z)/2);
	}

	CNumber sa = sin (m_Angle);
	CNumber sb = sin (a.m_Angle);
	CNumber ca = cos (m_Angle);
	CNumber cb = cos (a.m_Angle);
	CNumber x = sb*ca - sa*cb;

	if (x == 0.0)
		return C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY);
	CNumber u = (cb*(m_Origin.m_z - a.m_Origin.m_z) - sb*(m_Origin.m_x - a.m_Origin.m_x))/x;
	return C3DPoint (m_Origin.m_x + u*ca, 0, m_Origin.m_z + u*sa);
};


bool CRidgeLine::Colinear(const CRidgeLine &a) const
{
	CNumber aa = m_Angle;
	CNumber ba = a.m_Angle;
	CNumber aa2 = m_Angle + CN_PI;

	aa.NormalizeAngle();
	ba.NormalizeAngle();
	aa2.NormalizeAngle();
	return (ba == aa || ba == aa2) ? true : false;
}

CNumber CRidgeLine::Dist(const C3DPoint &p)
{
	CNumber a = m_Angle - CRidgeLine(m_Origin, p, -1).m_Angle;
	CNumber d = sin (a) * m_Origin.DistXZ(p);
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

bool CNumber::Similar(const CNumber &x)
{
	// TBD  put some code here to check exp and mantissa separately
	// for similarity at least significant point
	return true;
}
//
// Implementation of the CVertex class.
//

//
// Constructor
//
CVertex :: CVertex (const C3DPoint &p, CVertex &left, CVertex &right) 
				: m_point (p), m_done (false), m_higher (NULL), m_ID (-1), m_leftSkeletonLine (NULL), m_rightSkeletonLine (NULL),
				  m_advancingSkeletonLine (NULL)
{
	// This constructor links the new vertex in the current active contour
	m_leftLine = left.m_leftLine; 
	m_rightLine = right.m_rightLine;
	m_leftVertex = &left;
	m_rightVertex = &right;

	// Create the associated ridgeline for this vertex
#ifdef FELKELDEBUG
	{
		CNumber n1 = m_leftLine.Dist(m_point) * tan(m_leftLine.m_Slope);
		CNumber n2 = m_rightLine.Dist(m_point) * tan(m_rightLine.m_Slope);
//		assert (n1 == n2);
	}
#endif
	C3DPoint i = m_leftLine.Intersection (m_rightLine); 
	if (i.m_x == CN_INFINITY)
	{                                                               
		assert (i.m_z == CN_INFINITY);
		i = m_leftLine.IntersectionAnywhere (m_rightLine);
		if (i.m_x == CN_INFINITY)
		{
			assert (i.m_z == CN_INFINITY);
			m_axis = CRidgeLine(m_point, m_leftLine.m_Angle, -1);
		}
		else 
		{
			m_axis = CRidgeLine(m_point, i);
			m_axis.m_Angle = m_axis.m_Angle + CN_PI;  
		}
	}
	else
	{
		m_axis = CRidgeLine(m_point, i);
	}
}

C3DPoint CVertex::CoordinatesOfAnyIntersectionOfTypeB (const CVertex &left, const CVertex &right)
{
	C3DPoint p1 = m_rightLine.IntersectionAnywhere(right.m_leftLine);
	C3DPoint p2 = m_leftLine.IntersectionAnywhere(left.m_rightLine);
	C3DPoint poi(CN_INFINITY, CN_INFINITY, CN_INFINITY);
	CNumber Slope;

	assert(right.m_leftLine.m_Slope == left.m_rightLine.m_Slope);

	if (p1 != C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY) && p2 != C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY))
	{
		if (m_rightLine.PointOnRidgeLine(p1)) return C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY);
		if (m_leftLine.PointOnRidgeLine(p2))  return C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY);
		poi = CRidgeLine::AngleAxis(p1, p2, right.m_leftLine.m_Slope, m_point, m_rightLine.m_Slope).IntersectionAnywhere (m_axis);
	}
	else //if (p1 != Point (CN_INFINITY, CN_INFINITY))                             
	{
		poi = left.m_rightLine.IntersectionAnywhere(m_axis);
		if (p1 == C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY))
			Slope = m_rightLine.m_Slope;
		else
			Slope = m_leftLine.m_Slope;

		poi.m_x = (((m_point.m_x - poi.m_x) * tan(Slope)) / (tan(left.m_rightLine.m_Slope) + tan(Slope))) + poi.m_x;
		poi.m_z = (((m_point.m_z - poi.m_z) * tan(Slope)) / (tan(left.m_rightLine.m_Slope) + tan(Slope))) + poi.m_z;

	}
	return poi;
}

C3DPoint CVertex::IntersectionOfTypeB (const CVertex &left, const CVertex &right)
{
	assert (m_prevVertex == NULL || m_leftLine.FacingTowards(m_prevVertex->m_rightLine));
	assert (m_nextVertex == NULL || m_rightLine.FacingTowards(m_nextVertex->m_leftLine));
	assert (left.m_prevVertex == NULL || left.m_leftLine.FacingTowards(left.m_prevVertex->m_rightLine));
	assert (left.m_nextVertex == NULL || left.m_rightLine.FacingTowards(left.m_nextVertex->m_leftLine));
	assert (right.m_prevVertex == NULL || right.m_leftLine.FacingTowards(right.m_prevVertex->m_rightLine));
	assert (right.m_nextVertex == NULL || right.m_rightLine.FacingTowards(right.m_nextVertex->m_leftLine));

	C3DPoint pl(m_axis.Intersection(left.m_rightLine));
	C3DPoint pr(m_axis.Intersection(right.m_leftLine));
	if (pl == C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY) && pr == C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY))
		return C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY);

	C3DPoint p;
	if (pl != C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY)) p = pl;
	if (pr != C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY)) p = pr;
	assert (p != C3DPoint(CN_INFINITY, CN_INFINITY, CN_INFINITY));
// Dont know why this triggers......Needs fixing some day!!!!!!!!
//	assert (pl == C2DPoint(CN_INFINITY, CN_INFINITY) || pr == C2DPoint(CN_INFINITY, CN_INFINITY) || pl == pr);

	C3DPoint poi = CoordinatesOfAnyIntersectionOfTypeB(left, right); 
	CNumber al = left.m_axis.m_Angle - left.m_rightLine.m_Angle;
	CNumber ar = right.m_axis.m_Angle - right.m_leftLine.m_Angle; 

	CNumber alp = CRidgeLine(left.m_point, poi).m_Angle - left.m_rightLine.m_Angle;
	CNumber arp = CRidgeLine(right.m_point, poi).m_Angle - right.m_leftLine.m_Angle;

	al.NormalizeAngle(); ar.NormalizeAngle(); alp.NormalizeAngle(); arp.NormalizeAngle();
	assert (al <= 0.0);
	assert (ar >= 0.0 || ar == -CN_PI);

	if ((alp > 0.0 || alp < al) && !ANGLE_SIMILAR(alp, CNumber(0)) && !ANGLE_SIMILAR(alp, al))
		return C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY);
	if ((arp < 0.0 || arp > ar) && !ANGLE_SIMILAR(arp, CNumber(0)) && !ANGLE_SIMILAR(arp, ar))
		return C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY);
	return poi;
}

CNumber CVertex::NearestIntersection (CVertexList &vl, CVertex **left, CVertex **right, C3DPoint &p)
{
	CNumber minDist = CN_INFINITY;
	CVertexList::iterator minI = vl.end (); 
	for (CVertexList :: iterator i = vl.begin (); i != vl.end (); i++)
	{
		if ((*i).m_done) continue;
		if ((*i).m_nextVertex == NULL || (*i).m_prevVertex == NULL) continue;
		if (&*i == this || (*i).m_nextVertex == this) continue;
		assert ((*i).m_rightVertex != NULL);
		assert ((*i).m_leftVertex != NULL);
		C3DPoint poi = IntersectionOfTypeB((*i), *(*i).m_nextVertex);
		if (poi == C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY)) continue;
		CNumber d = poi.DistXZ(m_point);
		if (d < minDist) { minDist = d; minI = i; }
	}
	if (minDist == CN_INFINITY) return CN_INFINITY;

	i = minI;
	C3DPoint poi = CoordinatesOfAnyIntersectionOfTypeB((*i), *(*i).m_nextVertex);

	// Calculate heights
	poi.m_y = m_leftLine.Dist(poi) * tan(m_leftLine.m_Slope);

/*
#ifdef FELKELDEBUG
	{
		CNumber db1 = m_rightLine.Dist(poi) * tan(m_rightLine.m_Slope);
		CNumber db2 = (*i).m_rightLine.Dist(poi) * tan((*i).m_rightLine.m_Slope);
		CNumber db3 = (*i).m_nextVertex->m_leftLine.Dist(poi) * tan((*i).m_nextVertex->m_leftLine.m_Slope);
		assert (SIMILAR(poi.m_y, db1));
		assert (SIMILAR(poi.m_y, db2));
		assert (SIMILAR(poi.m_y, db3));
	}
#endif
*/

	p = poi;
	*left = (CVertex *) &*i;
	*right = (*i).m_nextVertex;

	return poi.m_y;
}

bool CVertex::InvalidIntersection (CVertexList &vl, const CIntersection &is)
{
	for (CVertexList :: iterator i = vl.begin (); i != vl.end (); i++)
	{
		if ((*i).m_done)
			continue;
		if ((*i).m_nextVertex == NULL || (*i).m_prevVertex == NULL)
			continue;

		C3DPoint poi = m_axis.Intersection((*i).m_axis);
		if (poi == C3DPoint (CN_INFINITY, CN_INFINITY, CN_INFINITY))
			continue;
		if (&*i == is.m_leftVertex || &*i == is.m_rightVertex)
			continue;
		if (m_axis.PointOnRidgeLine((*i).m_axis.m_Origin) && (*i).m_axis.PointOnRidgeLine(m_axis.m_Origin))
			continue;

		// Calculate height of intersection
		CNumber dv = m_leftLine.Dist(poi) * tan(m_leftLine.m_Slope);
		CNumber dvx = m_rightLine.Dist(poi) * tan(m_rightLine.m_Slope);
//		assert (dv == dvx);
		if (dv > is.m_height)
			continue;

		CNumber di = (*i).m_leftLine.Dist(poi) * tan((*i).m_leftLine.m_Slope);
		CNumber dix = (*i).m_rightLine.Dist(poi) * tan((*i).m_rightLine.m_Slope);
//		assert (di == dix);
		if (di > dv + MIN_DIFF)
			continue;

		return true;
	}
	return false;
}
