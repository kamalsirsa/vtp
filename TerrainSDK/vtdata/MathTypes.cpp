//
// Implementation of methods for the basic data classes
//
// Copyright (c) 2001 Virtual Terrain Project
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

void DLine2::RemovePoint(int i)
{
	int size = GetSize();
	for (int x=i; x < size-1; x++)
		SetAt(x, GetAt(x+1));
	SetSize(GetSize()-1);
}

bool DLine2::ContainsPoint(const DPoint2 &p) const
{
	if (GetData() != NULL)
		return CrossingsTest(GetData(), GetSize(), p);
	else
		return false;
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

//
// DMatrix methods
//

static double Dot3f(const double *d1, const double *d2)
{
	return d1[0]*d2[0]+d1[1]*d2[1]+d1[2]*d2[2];
}

void DMatrix3::Transform(const DPoint3 &tmp, DPoint3 &dst) const
{
	dst.x = Dot3f(&tmp.x, data[0]);
	dst.y = Dot3f(&tmp.x, data[1]);
	dst.z = Dot3f(&tmp.x, data[2]);
}

void DMatrix3::SetByMatrix4(const DMatrix4 &m)
{
	data[0][0] = m.Get(0,0); data[0][1] = m.Get(0,1); data[0][2] = m.Get(0,2);
	data[1][0] = m.Get(1,0); data[1][1] = m.Get(1,1); data[1][2] = m.Get(1,2);
	data[2][0] = m.Get(2,0); data[2][1] = m.Get(2,1); data[2][2] = m.Get(2,2);
}

/////////////////////////////////////////////////////////////////////
// DMatrix4

void DMatrix4::Identity()
{
	data[0][0] = 1.0;
	data[0][1] = 0.0;
	data[0][2] = 0.0;
	data[0][3] = 0.0;
	data[1][0] = 0.0;
	data[1][1] = 1.0;
	data[1][2] = 0.0;
	data[1][3] = 0.0;
	data[2][0] = 0.0;
	data[2][1] = 0.0;
	data[2][2] = 1.0;
	data[2][3] = 0.0;
	data[3][0] = 0.0;
	data[3][1] = 0.0;
	data[3][2] = 0.0;
	data[3][3] = 1.0;
}

void DMatrix4::AxisAngle(const DPoint3 &vec, double theta)
{
	double cost = cos(theta), sint = sin(theta);

	double a2, b2, c2, abm, acm, bcm;
	double mcos, asin, bsin, csin;
	mcos = 1.0f - cost;
	a2 = vec.x * vec.x;
	b2 = vec.y * vec.y;
	c2 = vec.z * vec.z;
	abm = vec.x * vec.y * mcos;
	acm = vec.x * vec.z * mcos;
	bcm = vec.y * vec.z * mcos;
	asin = vec.x * sint;
	bsin = vec.y * sint;
	csin = vec.z * sint;
	data[0][0] = a2 * mcos + cost;
	data[0][1] = abm - csin;
	data[0][2] = acm + bsin;
	data[1][0] = abm + csin;
	data[1][1] = b2 * mcos + cost;
	data[1][2] = bcm - asin;
	data[2][0] = acm - bsin;
	data[2][1] = bcm + asin;
	data[2][2] = c2 * mcos + cost;
}

static void Full_Inverse_Xform3(const double b[4][4], double a[4][4])
{
	long indxc[4], indxr[4], ipiv[4];
	long i, icol, irow, j, ir, ic;
	double big, dum, pivinv, temp, bb;
	ipiv[0] = -1;
	ipiv[1] = -1;
	ipiv[2] = -1;
	ipiv[3] = -1;
	a[0][0] = b[0][0];
	a[1][0] = b[1][0];
	a[2][0] = b[2][0];
	a[3][0] = b[3][0];
	a[0][1] = b[0][1];
	a[1][1] = b[1][1];
	a[2][1] = b[2][1];
	a[3][1] = b[3][1];
	a[0][2] = b[0][2];
	a[1][2] = b[1][2];
	a[2][2] = b[2][2];
	a[3][2] = b[3][2];
	a[0][3] = b[0][3];
	a[1][3] = b[1][3];
	a[2][3] = b[2][3];
	a[3][3] = b[3][3];
	for (i = 0; i < 4; i++) {
		big = 0.0f;
		for (j = 0; j < 4; j++) {
			if (ipiv[j] != 0) {
				if (ipiv[0] == -1) {
					if ((bb = (double) fabs(a[j][0])) > big) {
						big = bb;
						irow = j;
						icol = 0;
					}
				} else if (ipiv[0] > 0) {
					return;
				}
				if (ipiv[1] == -1) {
					if ((bb = (double) fabs((double) a[j][1])) > big) {
						big = bb;
						irow = j;
						icol = 1;
					}
				} else if (ipiv[1] > 0) {
					return;
				}
				if (ipiv[2] == -1) {
					if ((bb = (double) fabs((double) a[j][2])) > big) {
						big = bb;
						irow = j;
						icol = 2;
					}
				} else if (ipiv[2] > 0) {
					return;
				}
				if (ipiv[3] == -1) {
					if ((bb = (double) fabs((double) a[j][3])) > big) {
						big = bb;
						irow = j;
						icol = 3;
					}
				} else if (ipiv[3] > 0) {
					return;
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol) {
			temp = a[irow][0];
			a[irow][0] = a[icol][0];
			a[icol][0] = temp;
			temp = a[irow][1];
			a[irow][1] = a[icol][1];
			a[icol][1] = temp;
			temp = a[irow][2];
			a[irow][2] = a[icol][2];
			a[icol][2] = temp;
			temp = a[irow][3];
			a[irow][3] = a[icol][3];
			a[icol][3] = temp;
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol][icol] == 0.0) {
			return;
		}
		pivinv = 1.0f / a[icol][icol];
		a[icol][icol] = 1.0f;
		a[icol][0] *= pivinv;
		a[icol][1] *= pivinv;
		a[icol][2] *= pivinv;
		a[icol][3] *= pivinv;
		if (icol != 0) {
			dum = a[0][icol];
			a[0][icol] = 0.0f;
			a[0][0] -= a[icol][0] * dum;
			a[0][1] -= a[icol][1] * dum;
			a[0][2] -= a[icol][2] * dum;
			a[0][3] -= a[icol][3] * dum;
		}
		if (icol != 1) {
			dum = a[1][icol];
			a[1][icol] = 0.0f;
			a[1][0] -= a[icol][0] * dum;
			a[1][1] -= a[icol][1] * dum;
			a[1][2] -= a[icol][2] * dum;
			a[1][3] -= a[icol][3] * dum;
		}
		if (icol != 2) {
			dum = a[2][icol];
			a[2][icol] = 0.0f;
			a[2][0] -= a[icol][0] * dum;
			a[2][1] -= a[icol][1] * dum;
			a[2][2] -= a[icol][2] * dum;
			a[2][3] -= a[icol][3] * dum;
		}
		if (icol != 3) {
			dum = a[3][icol];
			a[3][icol] = 0.0f;
			a[3][0] -= a[icol][0] * dum;
			a[3][1] -= a[icol][1] * dum;
			a[3][2] -= a[icol][2] * dum;
			a[3][3] -= a[icol][3] * dum;
		}
	}
	if (indxr[3] != indxc[3]) {
		ir = indxr[3];
		ic = indxc[3];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[2] != indxc[2]) {
		ir = indxr[2];
		ic = indxc[2];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[1] != indxc[1]) {
		ir = indxr[1];
		ic = indxc[1];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[0] != indxc[0]) {
		ir = indxr[0];
		ic = indxc[0];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
}

static double det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3)
{
		return (double) (a1 * (b2 * c3 - b3 * c2) - b1 * (a2 * c3 - a3 * c2) + c1 * (a2 * b3 - a3 * b2));
}
#define SMALL_NUMBER		1.e-12f

static void Down_triangle_Inverse_Xform(const double src[4][4], double dst[4][4])
{
	double det1, o33;
	double a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = 1.0f / det1;
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[3][0] = -(src[3][0] * dst[0][0] + src[3][1] * dst[1][0] + src[3][2] * dst[2][0]) * o33;
		dst[3][1] = -(src[3][0] * dst[0][1] + src[3][1] * dst[1][1] + src[3][2] * dst[2][1]) * o33;
		dst[3][2] = -(src[3][0] * dst[0][2] + src[3][1] * dst[1][2] + src[3][2] * dst[2][2]) * o33;
		dst[0][3] = 0.0f;
		dst[1][3] = 0.0f;
		dst[2][3] = 0.0f;
		dst[3][3] = o33;
	}
}

static void Up_triangle_Inverse_Xform(const double src[4][4], double dst[4][4])
{
	double det1, o33;
	double a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = (double) (1.0f / det1);
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[0][3] = -(src[0][3] * dst[0][0] + src[1][3] * dst[0][1] + src[2][3] * dst[0][2]) * o33;
		dst[1][3] = -(src[0][3] * dst[1][0] + src[1][3] * dst[1][1] + src[2][3] * dst[1][2]) * o33;
		dst[2][3] = -(src[0][3] * dst[2][0] + src[1][3] * dst[2][1] + src[2][3] * dst[2][2]) * o33;
		dst[3][0] = 0.0f;
		dst[3][1] = 0.0f;
		dst[3][2] = 0.0f;
		dst[3][3] = o33;
	}
}

void DMatrix4::Invert(const DMatrix4 &src)
{
	if ((src.data[0][3] == 0) && (src.data[1][3] == 0) && (src.data[2][3] == 0)) {
		Down_triangle_Inverse_Xform(src.data, data);
		return;
	} else if ((src.data[3][0] == 0) && (src.data[3][1] == 0) && (src.data[3][2] == 0)) {
		Up_triangle_Inverse_Xform(src.data, data);
		return;
	} else {
		Full_Inverse_Xform3(src.data, data);
		return;
	}
}


//////////////////////////////////////////////////////////////
// FMatrix4

bool FMatrix4::IsIdentity() const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i == j)
			{
				if (data[i][j] != 1.0f) return false;
			}
			else
			{
				if (data[i][j] != 0.0f) return false;
			}
		}
	}
	return true;
}

void FMatrix4::AxisAngle(const FPoint3 &vec, double theta)
{
	double cost = cos(theta), sint = sin(theta);

	double a2, b2, c2, abm, acm, bcm;
	double mcos, asin, bsin, csin;
	mcos = 1.0f - cost;
	a2 = vec.x * vec.x;
	b2 = vec.y * vec.y;
	c2 = vec.z * vec.z;
	abm = vec.x * vec.y * mcos;
	acm = vec.x * vec.z * mcos;
	bcm = vec.y * vec.z * mcos;
	asin = vec.x * sint;
	bsin = vec.y * sint;
	csin = vec.z * sint;
	data[0][0] = (float) (a2 * mcos + cost);
	data[0][1] = (float) (abm - csin);
	data[0][2] = (float) (acm + bsin);
	data[1][0] = (float) (abm + csin);
	data[1][1] = (float) (b2 * mcos + cost);
	data[1][2] = (float) (bcm - asin);
	data[2][0] = (float) (acm - bsin);
	data[2][1] = (float) (bcm + asin);
	data[2][2] = (float) (c2 * mcos + cost);
}

static float Dot3f(const float *d1, const float *d2)
{
	return d1[0]*d2[0]+d1[1]*d2[1]+d1[2]*d2[2];
}

static void Full_Inverse_Xform3(const float b[4][4], float a[4][4])
{
	long indxc[4], indxr[4], ipiv[4];
	long i, icol, irow, j, ir, ic;
	float big, dum, pivinv, temp, bb;
	ipiv[0] = -1;
	ipiv[1] = -1;
	ipiv[2] = -1;
	ipiv[3] = -1;
	a[0][0] = b[0][0];
	a[1][0] = b[1][0];
	a[2][0] = b[2][0];
	a[3][0] = b[3][0];
	a[0][1] = b[0][1];
	a[1][1] = b[1][1];
	a[2][1] = b[2][1];
	a[3][1] = b[3][1];
	a[0][2] = b[0][2];
	a[1][2] = b[1][2];
	a[2][2] = b[2][2];
	a[3][2] = b[3][2];
	a[0][3] = b[0][3];
	a[1][3] = b[1][3];
	a[2][3] = b[2][3];
	a[3][3] = b[3][3];
	for (i = 0; i < 4; i++) {
		big = 0.0f;
		for (j = 0; j < 4; j++) {
			if (ipiv[j] != 0) {
				if (ipiv[0] == -1) {
					if ((bb = (float) fabs(a[j][0])) > big) {
						big = bb;
						irow = j;
						icol = 0;
					}
				} else if (ipiv[0] > 0) {
					return;
				}
				if (ipiv[1] == -1) {
					if ((bb = (float) fabs((float) a[j][1])) > big) {
						big = bb;
						irow = j;
						icol = 1;
					}
				} else if (ipiv[1] > 0) {
					return;
				}
				if (ipiv[2] == -1) {
					if ((bb = (float) fabs((float) a[j][2])) > big) {
						big = bb;
						irow = j;
						icol = 2;
					}
				} else if (ipiv[2] > 0) {
					return;
				}
				if (ipiv[3] == -1) {
					if ((bb = (float) fabs((float) a[j][3])) > big) {
						big = bb;
						irow = j;
						icol = 3;
					}
				} else if (ipiv[3] > 0) {
					return;
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol) {
			temp = a[irow][0];
			a[irow][0] = a[icol][0];
			a[icol][0] = temp;
			temp = a[irow][1];
			a[irow][1] = a[icol][1];
			a[icol][1] = temp;
			temp = a[irow][2];
			a[irow][2] = a[icol][2];
			a[icol][2] = temp;
			temp = a[irow][3];
			a[irow][3] = a[icol][3];
			a[icol][3] = temp;
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol][icol] == 0.0) {
			return;
		}
		pivinv = 1.0f / a[icol][icol];
		a[icol][icol] = 1.0f;
		a[icol][0] *= pivinv;
		a[icol][1] *= pivinv;
		a[icol][2] *= pivinv;
		a[icol][3] *= pivinv;
		if (icol != 0) {
			dum = a[0][icol];
			a[0][icol] = 0.0f;
			a[0][0] -= a[icol][0] * dum;
			a[0][1] -= a[icol][1] * dum;
			a[0][2] -= a[icol][2] * dum;
			a[0][3] -= a[icol][3] * dum;
		}
		if (icol != 1) {
			dum = a[1][icol];
			a[1][icol] = 0.0f;
			a[1][0] -= a[icol][0] * dum;
			a[1][1] -= a[icol][1] * dum;
			a[1][2] -= a[icol][2] * dum;
			a[1][3] -= a[icol][3] * dum;
		}
		if (icol != 2) {
			dum = a[2][icol];
			a[2][icol] = 0.0f;
			a[2][0] -= a[icol][0] * dum;
			a[2][1] -= a[icol][1] * dum;
			a[2][2] -= a[icol][2] * dum;
			a[2][3] -= a[icol][3] * dum;
		}
		if (icol != 3) {
			dum = a[3][icol];
			a[3][icol] = 0.0f;
			a[3][0] -= a[icol][0] * dum;
			a[3][1] -= a[icol][1] * dum;
			a[3][2] -= a[icol][2] * dum;
			a[3][3] -= a[icol][3] * dum;
		}
	}
	if (indxr[3] != indxc[3]) {
		ir = indxr[3];
		ic = indxc[3];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[2] != indxc[2]) {
		ir = indxr[2];
		ic = indxc[2];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[1] != indxc[1]) {
		ir = indxr[1];
		ic = indxc[1];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[0] != indxc[0]) {
		ir = indxr[0];
		ic = indxc[0];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
}

static float det3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
		return (float) (a1 * (b2 * c3 - b3 * c2) - b1 * (a2 * c3 - a3 * c2) + c1 * (a2 * b3 - a3 * b2));
}
#define SMALL_NUMBER		1.e-12f

static void Down_triangle_Inverse_Xform(const float src[4][4], float dst[4][4])
{
	float det1, o33;
	float a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = 1.0f / det1;
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[3][0] = -(src[3][0] * dst[0][0] + src[3][1] * dst[1][0] + src[3][2] * dst[2][0]) * o33;
		dst[3][1] = -(src[3][0] * dst[0][1] + src[3][1] * dst[1][1] + src[3][2] * dst[2][1]) * o33;
		dst[3][2] = -(src[3][0] * dst[0][2] + src[3][1] * dst[1][2] + src[3][2] * dst[2][2]) * o33;
		dst[0][3] = 0.0f;
		dst[1][3] = 0.0f;
		dst[2][3] = 0.0f;
		dst[3][3] = o33;
	}
}

static void Up_triangle_Inverse_Xform(const float src[4][4], float dst[4][4])
{
	float det1, o33;
	float a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = (float) (1.0f / det1);
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[0][3] = -(src[0][3] * dst[0][0] + src[1][3] * dst[0][1] + src[2][3] * dst[0][2]) * o33;
		dst[1][3] = -(src[0][3] * dst[1][0] + src[1][3] * dst[1][1] + src[2][3] * dst[1][2]) * o33;
		dst[2][3] = -(src[0][3] * dst[2][0] + src[1][3] * dst[2][1] + src[2][3] * dst[2][2]) * o33;
		dst[3][0] = 0.0f;
		dst[3][1] = 0.0f;
		dst[3][2] = 0.0f;
		dst[3][3] = o33;
	}
}

void FMatrix4::Identity()
{
	data[0][0] = 1.0f;
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[0][3] = 0.0f;
	data[1][0] = 0.0f;
	data[1][1] = 1.0f;
	data[1][2] = 0.0f;
	data[1][3] = 0.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
	data[2][3] = 0.0f;
	data[3][0] = 0.0f;
	data[3][1] = 0.0f;
	data[3][2] = 0.0f;
	data[3][3] = 1.0f;
}

void FMatrix4::Invert(const FMatrix4 &src)
{
	if ((src.data[0][3] == 0) && (src.data[1][3] == 0) && (src.data[2][3] == 0)) {
		Down_triangle_Inverse_Xform(src.data, data);
		return;
	} else if ((src.data[3][0] == 0) && (src.data[3][1] == 0) && (src.data[3][2] == 0)) {
		Up_triangle_Inverse_Xform(src.data, data);
		return;
	} else {
		Full_Inverse_Xform3(src.data, data);
		return;
	}
}

void FMatrix4::Translate(const FPoint3 &vec)
{
	data[0][3] += vec.x;
	data[1][3] += vec.y;
	data[2][3] += vec.z;
}

FPoint3 FMatrix4::GetTrans() const
{
	return FPoint3(data[0][3], data[1][3], data[2][3]);
}

void FMatrix4::SetTrans(FPoint3 pos)
{
	data[0][3] = pos.x;
	data[1][3] = pos.y;
	data[2][3] = pos.z;
}

void FMatrix4::Transform(const FPoint3 &tmp, FPoint3 &dst) const
{
	dst.x = Dot3f(&tmp.x, data[0]) + data[0][3];
	dst.y = Dot3f(&tmp.x, data[1]) + data[1][3];
	dst.z = Dot3f(&tmp.x, data[2]) + data[2][3];
}

void FMatrix4::TransformVector(const FPoint3 &tmp, FPoint3 &dst) const
{
	dst.x = Dot3f(&tmp.x, data[0]);
	dst.y = Dot3f(&tmp.x, data[1]);
	dst.z = Dot3f(&tmp.x, data[2]);
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



