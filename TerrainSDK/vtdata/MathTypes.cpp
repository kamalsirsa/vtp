//
// Implementation of methods for the basic data classes
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "MathTypes.h"

/**
 * Return the index of the polygon at a specified point, or -1 if
 * there is no polygon there.
 *
 * For speed, it first test the polygon which was found last time.
 * For spatially linear testing, this can be a 10x speedup.
 */
int DPolyArray2::FindPoly(const DPoint2 &p)
{
	if (m_previous_poly != -1)
	{
		if (GetAt(m_previous_poly)->ContainsPoint(p))
			return m_previous_poly;
	}
	int size = GetSize();
	for (int i = 0; i < size; i++)
	{
		DLine2 *poly = GetAt(i);
#if 0
		// possible further speed test: first test against extents
		if (world_x < poly->xmin || world_x > poly->xmax ||
			world_z < poly->zmin || world_z > poly->zmax)
			continue;
#endif
		if (poly->ContainsPoint(p))
		{
			// found
			m_previous_poly = i;
			return i;
		}
	}
	// not found
	m_previous_poly = -1;
	return -1;
}

//
// DLine2 methods
//

void DLine2::Add(const DPoint2 &p)
{
	int size = GetSize();
	for (int i=0; i < size; i++)
		GetAt(i) += p;
}

void  DLine2::InsertPointAfter(int iInsertAfter, DPoint2 &Point)
{
	int iNumPoints = GetSize();
	int iIndex;
	if (iInsertAfter == iNumPoints - 1)
		Append(Point);
	else
	{
		for (iIndex = iNumPoints - 1; iIndex > iInsertAfter ; iIndex--)
			SetAt(iIndex + 1, GetAt(iIndex));
		SetAt(iInsertAfter + 1, Point);
	}
}

void DLine2::RemovePoint(int i)
{
//	int size = GetSize();
//	for (int x=i; x < size-1; x++)
//		SetAt(x, GetAt(x+1));
//	SetSize(GetSize()-1);
	RemoveAt(i);
}

bool DLine2::ContainsPoint(const DPoint2 &p) const
{
	if (GetData() != NULL)
		return CrossingsTest(GetData(), GetSize(), p);
	else
		return false;
}

/**
 * Returns the index of the first point of the nearest line segment
 * that has a normal to the specified point
 */
double DLine2::NearestSegment(const DPoint2 &Point, int& iIndex, DPoint2 &Intersection)
{
	int iNumPoints = GetSize();
	int i;
	double dMagnitude;
	double dDistance;
	double dMinDistance = 1E9;
	double dU;
	DPoint2 p0, p1, p2;

	for (i = 0; i < iNumPoints; i++)
	{
		p0 = GetAt(i);
		p1 = GetAt((i + 1) % iNumPoints);
		dMagnitude = SegmentLength(i);
		// Calcualate U for standard line equation
		// values of U between 0.0 and +1.0 mean normal intersects segment
		dU = ((( Point.x - p0.x) * (p1.x - p0.x)) +
				((Point.y - p0.y) * (p1.y - p0.y))) / (dMagnitude * dMagnitude);
		if ((dU < 0.0) || (dU > 1.0))
			continue;
		p2.x = p0.x + dU * (p1.x - p0.x);
		p2.y = p0.y + dU * (p1.y - p0.y);
		dDistance = DPoint2(Point - p2).Length();
		if (dDistance < dMinDistance)
		{
			dMinDistance = dDistance;
			iIndex = i;
			Intersection = p2;
		}
	}
	if (dMinDistance != 1E9)
		return dMinDistance;
	else
		return -1;
}

/**
 * Return the nearest point
 */
double DLine2::NearestPoint(const DPoint2 &Point, int &iIndex)
{
	int iNumPoints = GetSize();
	int i;
	double dMinDistance = 1E9;
	double dDistance;
	DPoint2 p0;

	for (i = 0; i < iNumPoints; i++)
	{
		p0 = GetAt(i);
		dDistance = DPoint2(Point - p0).Length();
		if (dDistance < dMinDistance)
		{
			dMinDistance = dDistance;
			iIndex = i;
		}
	}
	if (dMinDistance != 1E9)
		return dMinDistance;
	else
		return -1;
}

/**
 * Return the length of the N'th segment in the line, which is the distance
 * between points N and N+1.  If the length of the last segment is requested,
 * a closed polygon is assumed.
 */
double DLine2::SegmentLength(int i) const
{
	int j = (i < GetSize()-1) ? i+1 : 0;
	return (GetAt(j) - GetAt(i)).Length();
}

//
// DRECT methods
//

bool DRECT::ContainsLine(const DLine2 &line) const
{
	for (int i = 0; i < line.GetSize(); i++)
	{
		if (!ContainsPoint(line[i]))
			return false;
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// FQuat

void FQuat::makeRotate(const FPoint3 &axis, float angle)
{
}


/////////////////////////////////////////////////////////////////////////////
// RGBi

void RGBi::Crop()
{
	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;
}

/////////////////////////////////////////////////////////////////////////////
// RGBAi

void RGBAi::Crop()
{
	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;
	if (a < 0) a = 0;
	else if (a > 255) a = 255;
}

///////////////////////////
// useful helper functions
//

float random_offset(float x)
{
	return (((float)rand()/RAND_MAX) - 0.5f) * x;
}

float random(float x)
{
	return ((float)rand()/RAND_MAX) * x;
}

/**
 * Compute base-2 logarithm of an integer.
 * There's probably an even simpler way to do this with sneaky
 * logical ops or without a loop, but this works.
 */
int vt_log2(int n)
{
	int temp = n, i;
	for (i = 0; temp > 1; i++)
		temp >>= 1;
	return i;
}



/*
 * ======= Crossings algorithm ============================================
 *
 * Adapted from: Graphics Gems IV
 *
 * Shoot a test ray along +X axis.  The strategy, from MacMartin, is to
 * compare vertex Y values to the testing point's Y and quickly discard
 * edges which are entirely to one side of the test ray.
 */
/**
 * Point-in-polygon test.
 *
 * Tests whether the 2D polygon \a pgon with \a numverts number of vertices
 * contains the test point \a point.
 *
 * \returns 1 if inside, 0 if outside.
 */
bool CrossingsTest(const DPoint2 *pgon, int numverts, const DPoint2 &point)
{
	register int	j, yflag0, yflag1, xflag0;
	register double ty, tx;
	register bool inside_flag;
	const DPoint2 *vtx0, *vtx1;

	tx = point.x;
	ty = point.y;

	vtx0 = pgon + (numverts-1);
	/* get test bit for above/below X axis */
	yflag0 = (vtx0->y >= ty);
	vtx1 = pgon;

	inside_flag = false;
	for (j = numverts+1; --j;)
	{
		yflag1 = (vtx1->y >= ty);
		/* check if endpoints straddle (are on opposite sides) of X axis
		 * (i.e. the Y's differ); if so, +X ray could intersect this edge.
		 */
		if (yflag0 != yflag1)
		{
			xflag0 = (vtx0->x >= tx);
			/* check if endpoints are on same side of the Y axis (i.e. X's
			 * are the same); if so, it's easy to test if edge hits or misses.
			 */
			if (xflag0 == (vtx1->x >= tx))
			{
				/* if edge's X values both right of the point, must hit */
				if (xflag0) inside_flag = !inside_flag;
			}
			else
			{
				/* compute intersection of pgon segment with +X ray, note
				 * if >= point's X; if so, the ray hits it.
				 */
				if ((vtx1->x - (vtx1->y-ty)*
					 (vtx0->x-vtx1->x)/(vtx0->y-vtx1->y)) >= tx) {
					inside_flag = !inside_flag;
				}
			}
		}
		/* move to next pair of vertices, retaining info as possible */
		yflag0 = yflag1;
		vtx0 = vtx1;
		vtx1 += 1;
	}

	return inside_flag;
}

/**
 * 2d point in triangle containment test.
 *
 * \return true if the point is inside the triangle, otherwise false.
 */
bool PointInTriangle(const FPoint2 &p, const FPoint2 &p1, const FPoint2 &p2,
					 const FPoint2 &p3)
{
	float fAB = (p.y-p1.y)*(p2.x-p1.x) - (p.x-p1.x)*(p2.y-p1.y);
	float fBC = (p.y-p2.y)*(p3.x-p2.x) - (p.x-p2.x)*(p3.y-p2.y);
	float fCA = (p.y-p3.y)*(p1.x-p3.x) - (p.x-p3.x)*(p1.y-p3.y);

	return (fAB * fBC >= 0) && (fBC * fCA >= 0);
}

/**
 * 2d point in triangle containment test.
 *
 * \return true if the point is inside the triangle, otherwise false.
 */
bool PointInTriangle(const DPoint2 &p, const DPoint2 &p1, const DPoint2 &p2,
					 const DPoint2 &p3)
{
	double fAB = (p.y-p1.y)*(p2.x-p1.x) - (p.x-p1.x)*(p2.y-p1.y);
	double fBC = (p.y-p2.y)*(p3.x-p2.x) - (p.x-p2.x)*(p3.y-p2.y);
	double fCA = (p.y-p3.y)*(p1.x-p3.x) - (p.x-p3.x)*(p1.y-p3.y);

	return (fAB * fBC >= 0) && (fBC * fCA >= 0);
}

/**
 * Compute the 3 barycentric coordinates of a 2d point in a 2d triangle.
 *
 * \return false if a problem was encountered (e.g. degenerate triangle),
 * otherwise true.
 */
bool BarycentricCoords(const FPoint2 &p1, const FPoint2 &p2,
					   const FPoint2 &p3, const FPoint2 &p, float fBary[3])
{
	FPoint2 vec13 = p1 - p3;
	FPoint2 vec23 = p2 - p3;
	FPoint2 vecp3 = p - p3;

	float m11 = vec13.Dot(vec13);
	float m12 = vec13.Dot(vec23);
	float m22 = vec23.Dot(vec23);
	float fR0 = vec13.Dot(vecp3);
	float fR1 = vec23.Dot(vecp3);
	float fDet = m11*m22 - m12*m12;
	if (fDet == 0.0f)
		return false;
	float fInvDet = 1.0f/fDet;

	fBary[0] = (m22*fR0 - m12*fR1)*fInvDet;
	fBary[1] = (m11*fR1 - m12*fR0)*fInvDet;
	fBary[2] = 1.0f - fBary[0] - fBary[1];
	return true;
}


/**
 * Compute the 3 barycentric coordinates of a 2d point in a 2d triangle.
 *
 * \return false if a problem was encountered (e.g. degenerate triangle),
 * otherwise true.
 */
bool BarycentricCoords(const DPoint2 &p1, const DPoint2 &p2,
					   const DPoint2 &p3, const DPoint2 &p, double fBary[3])
{
	DPoint2 vec13 = p1 - p3;
	DPoint2 vec23 = p2 - p3;
	DPoint2 vecp3 = p - p3;

	double m11 = vec13.Dot(vec13);
	double m12 = vec13.Dot(vec23);
	double m22 = vec23.Dot(vec23);
	double fR0 = vec13.Dot(vecp3);
	double fR1 = vec23.Dot(vecp3);
	double fDet = m11*m22 - m12*m12;
	if (fDet == 0.0f)
		return false;
	double fInvDet = 1.0f/fDet;

	fBary[0] = (m22*fR0 - m12*fR1)*fInvDet;
	fBary[1] = (m11*fR1 - m12*fR0)*fInvDet;
	fBary[2] = 1.0f - fBary[0] - fBary[1];
	return true;
}

/**
 * Find the intersection of 3 planes.
 *
 * \return true if there was a valid intersection point.  The function
 * will fail and return false for any degenerate case, e.g. when any two
 * of the planes are parallel.
 */
bool PlaneIntersection(const FPlane &plane1, const FPlane &plane2,
					   const FPlane &plane3, FPoint3 &result)
{
	FPoint3 n1 = plane1;
	FPoint3 n2 = plane2;
	FPoint3 n3 = plane3;
	float d1 = -plane1.w;
	float d2 = -plane2.w;
	float d3 = -plane3.w;

	// formula for result: P = (d1(n2 X n3) + d2(n3 X n1) + d3(n1 X n2)) / n1 . (n2 X n3)
	FPoint3 numer = n2.Cross(n3) * d1 + n3.Cross(n1) * d2 + n1.Cross(n2) * d3;
	float denom = n1.Dot(n2.Cross(n3));

	// The following epsilon value was chosen carefully from empirical
	// results.  If you have buildings whose roofs are being drawn
	// incorrectly, this value can be increased, but do so with care.
	if (fabsf(denom) < 1E-7)
		return false;

	result = numer / denom;
	return true;
}

/**
 * Find the closest distance from a point to a line segment.
 *
 * \param p1, p2	The points which define the line segment.
 * \param p3		The point to which to measure distance.
 */
double DistancePointToLine(const DPoint2 &p1, const DPoint2 &p2, const DPoint2 &p3)
{
	DPoint2 vec = (p2 - p1);
	double len2 = vec.LengthSquared();
	double u = ((p3.x - p1.x)*(p2.x - p1.x) + (p3.y - p1.y)*(p2.y - p1.y)) / len2;

	// constrain u to remain within the line segment
	if (u < 0) u = 0;
	if (u > 1) u = 1;

	DPoint2 closest = p1 + (vec * u);
	return (p3 - closest).Length();
}



