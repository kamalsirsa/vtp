//
// Basic data type definitions shared by all the VTP software.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file MathTypes.h */

#ifndef VTMATHTYPESH
#define VTMATHTYPESH

#include <math.h>
#include <locale.h>
#include <vector>
#include <string>
#include "Array.h"

// willemsn: this was taken from OSG's Math include file.
// 04/29/03
#ifdef __DARWIN_OSX__
	#include <float.h>

	// PJA MAC OSX
	// This appears to be the simplest way to get these defined under MACOSX
	// where they arent in math.h
	#ifndef acosf
	#define acosf (float)acos
	#endif

	#ifndef asinf
	#define asinf (float)asin
	#endif

	#ifndef atan2f
	#define atan2f (float)atan2
	#endif

	#ifndef cosf
	#define cosf (float)cos
	#endif

	#ifndef sinf
	#define sinf (float)sin
	#endif

	#ifndef tanf
	#define tanf (float)tan
	#endif

	#ifndef logf
	#define logf (float)log
	#endif

	#ifndef powf
	#define powf (float)pow
	#endif

	#ifndef sqrtf
	#define sqrtf (float)sqrt
	#endif

	#ifndef fabsf
	#define fabsf (float)fabs
	#endif
#endif

#ifndef PId
 #define PId	3.14159265358979323846264338
 #define PIf 	3.14159265358979323846264338f
 #define PI2d 	6.28318530717958647692528676
 #define PI2f 	6.28318530717958647692528676f
 #define PID2d	1.57079632679489661923132169
 #define PID2f	1.57079632679489661923132169f
 #define PID3d	1.04719755119659774615421446
#endif

#ifndef NULL
#define NULL	0
#endif

class DPoint3;

/**
 * A 3-component vector class, single-precision (float).
 */
class FPoint3
{
public:
	FPoint3() { x = y = z = 0.0f; }
	FPoint3(float fx, float fy, float fz) { x=fx; y=fy; z=fz; }
	FPoint3(const DPoint3 &p);

	float Length() const { return sqrtf(x*x+y*y+z*z); }
	float LengthSquared() const { return x*x+y*y+z*z; }
	FPoint3 &Normalize() { float s = 1.0f/Length(); x*=s; y*=s; z*=s; return (*this); }
	void Set(float fx, float fy, float fz) { x=fx; y=fy; z=fz; }
	float Dot(const FPoint3 &rhs) const
	{
		return x*rhs.x+y*rhs.y+z*rhs.z;
	}
	float Dot(const float *fp) const
	{
		return x*fp[0]+y*fp[1]+z*fp[2];
	}
	FPoint3 Cross(const FPoint3 &v) const
	{
		return FPoint3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}

	// assignment
	FPoint3 &operator=(const FPoint3 &v) { x = v.x; y = v.y; z = v.z; return *this; }
	FPoint3 &operator=(const DPoint3 &v);

	// operators
	FPoint3 operator +(const FPoint3 &v) const { return FPoint3(x+v.x, y+v.y, z+v.z); }
	FPoint3 operator -(const FPoint3 &v) const { return FPoint3(x-v.x, y-v.y, z-v.z); }
	FPoint3 operator *(float s) const { return FPoint3(x*s, y*s, z*s); }
	FPoint3 operator /(float s) const { return FPoint3(x/s, y/s, z/s); }
	FPoint3 operator -() { return FPoint3(-x, -y, -z); }
	bool operator==(const FPoint3 &v2)
	{ return (x == v2.x && y == v2.y && z == v2.z); }
	bool operator!=(const FPoint3 &v2)
	{ return (x != v2.x || y != v2.y || z != v2.z); }

	void operator +=(const FPoint3 &v) { x+=v.x; y+=v.y; z+=v.z; }
	void operator -=(const FPoint3 &v) { x-=v.x; y-=v.y; z-=v.z; }
	void operator *=(float s) { x*=s; y*=s; z*=s; }
	void operator /=(float s) { x/=s; y/=s; z/=s; }
	FPoint3 operator -() const { return FPoint3 (-x, -y, -z); }

	float x, y, z;
};

/**
 * A 3-component vector class, double-precision.
 */
class DPoint3
{
public:
	DPoint3() { x = y = z = 0.0f; }
	DPoint3(double fx, double fy, double fz) { x=fx; y=fy; z=fz; }
	DPoint3(const FPoint3 &p);

	double Length() const { return sqrt(x*x+y*y+z*z); }
	double LengthSquared() const { return x*x+y*y+z*z; }
	DPoint3 &Normalize()
	{
		double s = 1.0/Length();
		x*=s; y*=s; z*=s;
		return (*this);
	}
	void Set(double fx, double fy, double fz) { x=fx; y=fy; z=fz; }
	double Dot(const DPoint3 &rhs) const
	{
		return x*rhs.x+y*rhs.y+z*rhs.z;
	}
	DPoint3 Cross(const DPoint3 &v) const
	{
		return DPoint3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}
	// assignment
	DPoint3 &operator=(const DPoint3 &v) { x = v.x; y = v.y; z = v.z; return *this; }
	DPoint3 &operator=(const FPoint3 &v);

	// operators
	DPoint3 operator +(const DPoint3 &v) const { return DPoint3(x+v.x, y+v.y, z+v.z); }
	DPoint3 operator -(const DPoint3 &v) const { return DPoint3(x-v.x, y-v.y, z-v.z); }
	DPoint3 operator *(double s) const { return DPoint3(x*s, y*s, z*s); }
	DPoint3 operator /(double s) const { return DPoint3(x/s, y/s, z/s); }
	bool operator==(const DPoint3 &v2)
	{ return (x == v2.x && y == v2.y && z == v2.z); }
	bool operator!=(const DPoint3 &v2)
	{ return (x != v2.x || y != v2.y || z != v2.z); }

	// dot product
	double operator *(const DPoint3 &v) const { return x*v.x + y*v.y + z*v.z; }

	void operator +=(const DPoint3 &v) { x+=v.x; y+=v.y; z+=v.z; }
	void operator -=(const DPoint3 &v) { x-=v.x; y-=v.y; z-=v.z; }
	void operator *=(double s) { x*=s; y*=s; z*=s; }
	void operator /=(double s) { x/=s; y/=s; z/=s; }

	double x, y, z;
};

// Conversion
inline FPoint3::FPoint3(const DPoint3 &v) { x = (float) v.x; y = (float) v.y; z = (float) v.z; }
inline DPoint3::DPoint3(const FPoint3 &v) { x = v.x; y = v.y; z = v.z; }
inline FPoint3 &FPoint3::operator=(const DPoint3 &v) { x = (float) v.x; y = (float) v.y; z = (float) v.z; return *this; }
inline DPoint3 &DPoint3::operator=(const FPoint3 &v) { x = v.x; y = v.y; z = v.z; return *this; }

/////////////////////////////////////////////

/**
 * A class representing an infinite plane, single-precision (float).
 */
class FPlane : public FPoint3
{
public:
	FPlane() {}
	FPlane(float a, float b, float c, float d) { x = a; y = b; z = c; w = d; }
	const FPlane &operator=(const FPlane &rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		w = rhs.w;
		return *this;
	}
	void Set(const FPoint3& p, const FPoint3& n)
	{
		x = n.x;
		y = n.y;
		z = n.z;
		w = -n.Dot(p);
	}
	void Set(float a, float b, float c, float d) { x = a; y = b; z = c; w = d; }
	float Distance(const FPoint3 &v) const
	{
		return Dot(v) + w;
	}

	float w;
};

////////////////////////////////////////////////////////////////

class FPoint2;
class DPoint2;

/**
 * A 2-component vector class, single-precision (float).
 */
class FPoint2 {
public:
	FPoint2() { x = y = 0.0f; }
	FPoint2(int ix, int iy) { x=(float)ix; y=(float)iy; }
	FPoint2(float fx, float fy) { x=fx; y=fy; }
	FPoint2(double dx, double dy) { x=(float)dx; y=(float)dy; }
	FPoint2(const DPoint2 &d);

	float Length() const { return sqrtf(x*x+y*y); }
	float LengthSquared() const { return x*x+y*y; }
	FPoint2 &Normalize() { float s = 1.0f/Length(); x*=s; y*=s; return (*this); }
	void Set(float fx, float fy) { x=fx; y=fy; }
	float Dot(const FPoint2 &rhs) const
	{
		return x*rhs.x+y*rhs.y;
	}

	// assignment
	FPoint2 &operator=(const FPoint2 &v) { x = v.x; y = v.y; return *this; }
	FPoint2 &operator=(const class DPoint2 &v);

	FPoint2 operator +(const FPoint2 &v) const { return FPoint2(x+v.x, y+v.y); }
	FPoint2 operator -(const FPoint2 &v) const { return FPoint2(x-v.x, y-v.y); }
	FPoint2 operator *(float s) const { return FPoint2(x*s, y*s); }
	FPoint2 operator /(float s) const { return FPoint2(x/s, y/s); }

	void operator +=(const FPoint2 &v) { x+=v.x; y+=v.y; }
	void operator -=(const FPoint2 &v) { x-=v.x; y-=v.y; }
	void operator *=(float s) { x*=s; y*=s; }
	void operator /=(float s) { x/=s; y/=s; }

	bool operator==(const FPoint2 &v) { return (x == v.x && y == v.y); }
	bool operator!=(const FPoint2 &v) { return (x != v.x || y != v.y); }

	float x, y;
};

/**
 * A 2-component vector class, double-precision.
 */
class DPoint2 {
public:
	DPoint2() { x = y = 0.0f; }
	DPoint2(int ix, int iy) { x=(double)ix; y=(double)iy; }
	DPoint2(float fx, float fy) { x=fx; y=fy; }
	DPoint2(double fx, double fy) { x=fx; y=fy; }
	DPoint2(const FPoint2 &f);

	double Length() const { return sqrt(x*x+y*y); }
	double LengthSquared() const { return (x*x+y*y); }
	DPoint2 &Normalize() { double s = 1.0f/Length(); x*=s; y*=s; return (*this); }
	void Set(double fx, double fy) { x=fx; y=fy; }
	double Dot(const DPoint2 &rhs) const
	{
		return x*rhs.x + y*rhs.y;
	}
	void Rotate(double radians)
	{
		double tempx = x;
		x = x * cos(radians) - y * sin(radians);
		y = tempx * sin(radians) + y * cos(radians);
	}
	// so-called "2D cross product" is really the dot-product with the
	// perpendicular vector
	double Cross(const DPoint2 &rhs)
	{
		return (x*rhs.y - y*rhs.x);
	}

	// assignment
	DPoint2 &operator=(const DPoint2 &v) { x = v.x; y = v.y; return *this; }
	DPoint2 &operator=(const class FPoint2 &v);

	DPoint2 operator +(const DPoint2 &v) const { return DPoint2(x+v.x, y+v.y); }
	DPoint2 operator -(const DPoint2 &v) const { return DPoint2(x-v.x, y-v.y); }
	DPoint2 operator *(double s) const { return DPoint2(x*s, y*s); }
	DPoint2 operator /(double s) const { return DPoint2(x/s, y/s); }

	void operator +=(const DPoint2 &v) { x+=v.x; y+=v.y; }
	void operator -=(const DPoint2 &v) { x-=v.x; y-=v.y; }
	void operator *=(double s) { x*=s; y*=s; }
	void operator /=(double s) { x/=s; y/=s; }

	bool operator==(const DPoint2 &v) const { return (x == v.x && y == v.y); }
	bool operator!=(const DPoint2 &v) const { return (x != v.x || y != v.y); }

	double x, y;
};


// copy constructors FPoint2 <> DPoint2
inline FPoint2::FPoint2(const DPoint2 &d) { x=(float)d.x; y=(float)d.y; }
inline DPoint2::DPoint2(const FPoint2 &f) { x=f.x; y=f.y; }


/**
 * A 2-component vector class, integer.  Useful for representing discrete
 * coordinates and sizes such as pixels and grids.
 */
class IPoint2 {
public:
	IPoint2() {}
	IPoint2(int ix, int iy) { x=ix; y=iy; }

	float Length() const { return sqrtf((float)x*x + (float)y*y); }
	void Set(int ix, int iy) { x=ix; y=iy; }
	IPoint2 &operator=(const IPoint2 &v) { x = v.x; y = v.y; return *this; }

	IPoint2 operator +(const IPoint2 &v) const { return IPoint2(x+v.x, y+v.y); }
	IPoint2 operator -(const IPoint2 &v) const { return IPoint2(x-v.x, y-v.y); }
	IPoint2 operator *(int s) const { return IPoint2(x*s, y*s); }
	IPoint2 operator *(float f) const { return IPoint2((int)(x*f), (int)(y*f)); }

	void operator +=(const IPoint2 &v) { x+=v.x; y+=v.y; }
	void operator -=(const IPoint2 &v) { x-=v.x; y-=v.y; }
	void operator *=(int s) { x*=s; y*=s; }
	void operator *=(float f) { x=(int)(x*f); y=(int)(y*f); }

	bool operator==(const IPoint2 &v) const { return (x == v.x && y == v.y); }
	bool operator!=(const IPoint2 &v) const { return (x != v.x || y != v.y); }

	int x, y;
};

inline FPoint2 &FPoint2::operator=(const class DPoint2 &v)
{
	x = (float) v.x;
	y = (float) v.y;
	return *this;
}

inline DPoint2 &DPoint2::operator=(const class FPoint2 &v)
{
	x = v.x;
	y = v.y;
	return *this;
}

/**
 * A series of 2D points.  This is useful for representing either a series of
 * points, line segments or a simple closed polygon.  Double-precision.
 */
class DLine2 : public Array<DPoint2>
{
public:
	DLine2() {}
	// copy constructor
	DLine2(const DLine2 &ref) : Array<DPoint2>() { *this = ref; }

	// assignment
	DLine2 &operator=(const class DLine2 &v);
	DLine2 &operator=(const class FLine2 &v);

	void Add(const DPoint2 &p);

	void InsertPointAfter(int iInsertAfter, const DPoint2 &Point);
	void RemovePoint(int i);
	bool ContainsPoint(const DPoint2 &p) const;
	double SegmentLength(unsigned int i) const;
	double NearestSegment(const DPoint2 &Point, int &iIndex, DPoint2 &Intersection) const;
	double NearestPoint(const DPoint2 &Point, int &iIndex) const;
	DPoint2 GetSafePoint(int index) const;
	void SetSafePoint(int index, const DPoint2 &p);
	double Length() const;
};

/**
 * A series of 2D points.  This is useful for representing either a series of
 * points, line segments or a closed polygon.  Single-precision.
 */
class FLine2 : public Array<FPoint2>
{
public:
	FLine2() {}
	// copy constructor
	FLine2(const FLine2 &ref) : Array<FPoint2>() { *this = ref; }

	// assignment
	FLine2 &operator=(const class FLine2 &v);
	FLine2 &operator=(const class DLine2 &v);
};

inline DLine2 &DLine2::operator=(const class DLine2 &v)
{
	int size = v.GetSize();
	SetSize(size);
	for (int i = 0; i < size; i++)
		SetAt(i, v.GetAt(i));

	return *this;
}

inline DLine2 &DLine2::operator=(const class FLine2 &v)
{
	int size = v.GetSize();
	SetSize(size);

	FPoint2 p1;
	DPoint2 p2;
	for (int i = 0; i < size; i++)
	{
		p1 = v.GetAt(i);
		p2 = p1;
		SetAt(i, p2);
	}
	return *this;
}

inline FLine2 &FLine2::operator=(const class FLine2 &v)
{
	int size = v.GetSize();
	SetSize(size);
	for (int i = 0; i < size; i++)
		SetAt(i, v.GetAt(i));

	return *this;
}

inline FLine2 &FLine2::operator=(const class DLine2 &v)
{
	int size = v.GetSize();
	SetSize(size);

	DPoint2 p1;
	FPoint2 p2;
	for (int i = 0; i < size; i++)
	{
		p1 = v.GetAt(i);
		p2 = p1;
		SetAt(i, p2);
	}
	return *this;
}

/////////////////////////////////////

/**
 * A series of 3D points.  This is useful for representing either a series of
 * points, line segments or a closed polygon.  Double-precision.
 */
class DLine3 : public Array<DPoint3>
{
public:
	DLine3() {}
	// copy constructor
	DLine3(const DLine3 &ref) : Array<DPoint3>() { *this = ref; }

	// assignment
	DLine3 &operator=(const class DLine3 &v);
//	DLine3 &operator=(const class FLine3 &v);

	void RemovePoint(int i);
};

/**
 * A series of 3D points.  This is useful for representing either a series of
 * points, line segments or a closed polygon.  Single-precision.
 */
class FLine3 : public Array<FPoint3>
{
public:
	FLine3() {}
	FLine3(int size) { SetSize(size); }
	// copy constructor
	FLine3(const FLine3 &ref) : Array<FPoint3>() { *this = ref; }

	// assignment
	FLine3 &operator=(const class FLine3 &v);
//	FLine3 &operator=(const class DLine3 &v);
};

inline DLine3 &DLine3::operator=(const class DLine3 &v)
{
	int size = v.GetSize();
	SetSize(size);
	for (int i = 0; i < size; i++)
		SetAt(i, v.GetAt(i));

	return *this;
}

inline FLine3 &FLine3::operator=(const class FLine3 &v)
{
	int size = v.GetSize();
	SetSize(size);
	for (int i = 0; i < size; i++)
		SetAt(i, v.GetAt(i));

	return *this;
}

/////////////////////////////////////

/**
 * A 3d box.  Useful for representing an rectangular space such as extents
 * or a bounding box.  Single-precision.
 */
class FBox3
{
public:
	FBox3() {}
	FBox3(const FPoint3 &min1, const FPoint3 &max1) { min = min1; max = max1; }

	void InsideOut()
	{
		min.Set(1E10f, 1E10f, 1E10f);
		max.Set(-1E10f, -1E10f, -1E10f);
	}
	void Set(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		min.Set(x1, y1, z1);
		max.Set(x2, y2, z2);
	}
	FPoint3 Center() const { return ((min + max) * 0.5); }
	void GrowToContainPoint(const FPoint3 &p)
	{
		if (p.x < min.x) min.x = p.x;
		if (p.y < min.y) min.y = p.y;
		if (p.z < min.z) min.z = p.z;
		if (p.x > max.x) max.x = p.x;
		if (p.y > max.y) max.y = p.y;
		if (p.z > max.z) max.z = p.z;
	}

	FPoint3	min, max;
};

/////////////////////////////////////////////

/**
 * A sphere, represented as a center and radius.  Single-precision.
 */
class FSphere
{
public:
	FSphere() {}
	FSphere(const FBox3 &src)
	{
		center = src.Center();
		radius = (center - src.min).Length();
	}
	FSphere(const FPoint3& p, float fRadius)
	{
		Set(p, fRadius);
	}
	const FSphere &operator=(const FSphere &rhs)
	{
		center = rhs.center;
		radius = rhs.radius;
		return *this;
	}
	void Set(const FPoint3& p, float fRadius)
	{
		center = p;
		radius = fRadius;
	}
	void Empty() { center.Set(0,0,0); radius = 0; }

	FPoint3 center;
	float radius;
};

/////////////////////////////////////

typedef std::vector<DLine2> DLine2Array;

/**
 * We represent a polygon as a collection of closed rings, each of which
 * is represented by a DLine2.  The first DLine2 is the 'outside' ring,
 * any subsequent DLine2 are 'inside' rings, which are holes.
 */
class DPolygon2 : public DLine2Array
{
public:
	bool ContainsPoint(const DPoint2 &p) const;
	void Add(const DPoint2 &p);

	void GetAsDLine2(DLine2 &dline) const;
};

/**
 * A collection of DPolygon2 objects.
 */
class DPolyArray : public std::vector<DPolygon2>
{
public:
	DPolyArray() { s_previous_poly = -1; }

	int FindPoly(const DPoint2 &p) const;

	// for speed, remember the polygon that we found last time
	static int s_previous_poly;
};


/**
 * DRECT / FRECT
 *
 * These rectangle classes are meant to be used for geographical areas,
 * or texture coordinates, which assume that the vertical axis increases
 * upward, not like typical screen coordinates which increase downwards.
 *
 * This means that the Height() method returns top-bottom, not bottom-top
 * as would be the case with downward-increasing coordinates.
 */
class DRECT
{
public:
	DRECT() { left = top = right = bottom = 0.0; }
	DRECT(double l, double t, double r, double b) { left = l; top = t; right = r; bottom = b; }

	void SetRect(double l, double t, double r, double b) { left = l; top = t; right = r; bottom = b; }
	// retrieves the width
	double Width() const { return right - left; }
	// returns the height, assumes upward increasing coordinate system
	double Height() const { return top - bottom; };
	// return true if null (all 0)
	bool IsNull() const { return (left == 0.0 && top == 0.0 && right == 0.0 && bottom == 0.0); }
	// return true if empty
	bool IsEmpty() const { return (left == right && top == bottom); }
	void Empty() { left = top = right = bottom = 0.0; }
	void Sort()
	{
		double tmp;
		if (left > right)
		{
			tmp = left;
			left = right;
			right = tmp;
		}
		if (bottom > top)
		{
			tmp = bottom;
			bottom = top;
			top = tmp;
		}
	}
	void GetCenter(DPoint2 &p) const
	{
		p.x = (left + right) / 2.0;
		p.y = (bottom + top) / 2.0;
	}
	bool ContainsPoint(const DPoint2 &p) const
	{
		return (p.x > left && p.x < right && p.y > bottom && p.y < top);
	}
	bool ContainsLine(const DLine2 &line) const;
	bool ContainsRect(const DRECT &r2) const
	{
		return (r2.left >= left && r2.right <= right &&
				r2.bottom >= bottom && r2.top <= top);
	}
	void Grow(double x, double y)
	{
		left -= x;
		right += x;
		bottom -= y;
		top += y;
	}
	void GrowToContainPoint(const DPoint2 &p)
	{
		if (p.x < left)		left = p.x;
		if (p.x > right)	right = p.x;
		if (p.y < bottom)	bottom = p.y;
		if (p.y > top)		top = p.y;
	}
	void GrowToContainLine(const DLine2 &line)
	{
		DPoint2 p;
		int size = line.GetSize();
		for (int i = 0; i < size; i++)
		{
			p = line.GetAt(i);
			if (p.x < left)		left = p.x;
			if (p.x > right)	right = p.x;
			if (p.y < bottom)	bottom = p.y;
			if (p.y > top)		top = p.y;
		}
	}
	// expand geographical rectangle r1 to include rectangle r2
	void GrowToContainRect(const DRECT &r2)
	{
		if (r2.left < left)		left = r2.left;
		if (r2.right > right)	right = r2.right;
		if (r2.top > top)		top = r2.top;
		if (r2.bottom < bottom)	bottom = r2.bottom;
	}
	bool operator==(const DRECT &v)
	{
		return (left == v.left && top == v.top && right == v.right && bottom == v.bottom);
	}
	bool operator!=(const DRECT &v)
	{
		return (left != v.left || top != v.top || right != v.right || bottom != v.bottom);
	}

	double	left;
	double	top;
	double	right;
	double	bottom;
};

/**
 * DRECT / FRECT
 *
 * These rectangle classes are meant to be used for geographical areas,
 * or texture coordinates, which assume that the vertical axis increases
 * upward, not like typical screen coordinates which increase downwards.
 *
 * This means that the Height() method returns top-bottom, not bottom-top
 * as would be the case with downward-increasing coordinates.
 */
class FRECT
{
public:
	FRECT() { left = top = right = bottom = 0.0; }
	FRECT(float l, float t, float r, float b)
	{
		left = l; top = t; right = r; bottom = b;
	}
	void SetRect(float l, float t, float r, float b)
	{
		left = l; top = t; right = r; bottom = b;
	}
	// retrieves the width
	float Width() const { return right - left; }
	// returns the height
	float Height() const { return top - bottom; };
	bool ContainsPoint(float x, float y)
	{
		return (x > left && x < right && y > bottom && y < top);
	}
	void Center(FPoint2 &center)
	{
		center.x = (left + right)/2;
		center.y = (bottom + top)/2;
	}
	void operator+=(const FPoint2 &delta)
	{
		left += delta.x; top += delta.y;
		right += delta.x; bottom += delta.y;
	}

	float	left;
	float	top;
	float	right;
	float	bottom;
};


///////////////////////////////////////////////////////////////////////
// Matrix classes

class DMatrix3;
class DMatrix4;

/**
 * A 3x3 matrix class, double-precision.
 */
class DMatrix3
{
public:
	void  Set(int i, int j, double v) { data[i][j] = v; }
	double Get(int i, int j) const { return data[i][j]; }
	double operator()(int i, int j) const { return data[i][j]; }

	void Identity();
	void AxisAngle(const DPoint3 &vec, double theta);
	void Transform(const DPoint3 &src, DPoint3 &dst) const;
	void SetByMatrix4(const DMatrix4 &m);

protected:
	double data[3][3];
};

/**
 * A 4x4 matrix class, double-precision.
 */
class DMatrix4
{
public:
	void  Set(int i, int j, double v) { data[i][j] = v; }
	double Get(int i, int j) const { return data[i][j]; }
	double operator()(int i, int j) const { return data[i][j]; }

	void Identity();
	void AxisAngle(const DPoint3 &vec, double theta);
	void Invert(const DMatrix4 &src);

protected:
	double data[4][4];
};

//////////////

class FMatrix4;

/**
 * A 3x3 matrix class, single-precision.
 */
class FMatrix3
{
public:
	FMatrix3() {}
	FMatrix3(const FMatrix4 &mat) { SetFromMatrix4(mat); }

	void  Set(int col, int row, float v) { data[col][row] = v; }
	float Get(int col, int row) const { return data[col][row]; }
	void SetRow(int row, float f0, float f1, float f2);

	void Identity();
	bool IsIdentity() const;
	void AxisAngle(const FPoint3 &vec, double theta);
	void SetFromVectors(const FPoint3 &forward, const FPoint3 &up);

	void Transform(const FPoint3 &src, FPoint3 &dst) const;
	void SetFromMatrix4(const FMatrix4 &mat);

	// operators
	float operator()(int col, int row) const { return data[col][row]; }
	FMatrix3 &operator=(const FMatrix4 &mat);

protected:
	float data[3][3];
};

typedef float FMatrix4Data[4][4];

/**
 * A 4x4 matrix class, single-precision.
 */
class FMatrix4
{
public:
	FMatrix4() {}
	FMatrix4(const FMatrix3 &mat) { SetFromMatrix3(mat); }

	void  Set(int col, int row, float v) { data[col][row] = v; }
	float Get(int col, int row) const { return data[col][row]; }
	void SetRow(int row, float f0, float f1, float f2, float f3);

	void  SetData(FMatrix4Data data_in) { memcpy(data, data_in, 64); }
	const FMatrix4Data &GetData() const { return data; }

	// set/modify
	void Identity();
	bool IsIdentity() const;
	void AxisAngle(const FPoint3 &vec, double theta);
	void Translate(const FPoint3 &vec);
	void Invert(const FMatrix4 &src);
	FPoint3 GetTrans() const;
	void SetTrans(FPoint3 pos);
	void SetFromVectors(const FPoint3 &pos, const FPoint3 &forward,
		const FPoint3 &up);
	void SetFromMatrix3(const FMatrix3 &mat);

	void PreMult(const FMatrix4 &mat);
	void PostMult(const FMatrix4 &mat);

	// apply to vectors
	void Transform(const FPoint3 &src, FPoint3 &dst) const;
	void TransformVector(const FPoint3 &tmp, FPoint3 &dst) const;

	// operators
	float operator()(int col, int row) const { return data[col][row]; }
	FMatrix4 &operator=(const FMatrix3 &mat);

protected:
	float data[4][4];
};

inline FMatrix3 &FMatrix3::operator=(const FMatrix4 &mat)
{
	SetFromMatrix4(mat);
	return (*this);
}
inline FMatrix4 &FMatrix4::operator=(const FMatrix3 &mat)
{
	SetFromMatrix3(mat);
	return (*this);
}

///////////////////////////////////////////////////////////////////////
// Quaternion class

/**
 * Single-precision quaternion class.  Quaternions are very useful for
 * efficiently representing an orientations/rotation and operations
 * such as interpolating between orientations.
 */
class FQuat
{
public:
	FQuat() {}
	FQuat(float qx, float qy, float qz, float qw) { x = qx; y = qy; z = qz; w = qw; }
	FQuat(const FQuat &q) { x = q.x; y = q.y; z = q.z; w = q.w; }
	FQuat(const FPoint3 &axis, float angle) { AxisAngle(axis, angle); }

	void Init() { x = 0; y = 0; z = 0; w = 1; }
	void Set(float qx, float qy, float qz, float qw) { x = qx; y = qy; z = qz; w = qw; }
	void SetFromMatrix(const FMatrix3 &matrix);
	void SetFromVectors(const FPoint3 &forward, const FPoint3 &up);
	void AxisAngle(const FPoint3 &axis, float angle);
	void GetMatrix(FMatrix3 &matrix) const;

	void Slerp(const FQuat &from, const FQuat &to, float f);
	float LengthSquared() const { return x*x + y*y + z*z + w*w; }
	const FQuat Inverse() const
	{
		float l2 = LengthSquared();
		return FQuat( -x / l2, -y / l2, -z / l2, w / l2);
	}
	void Invert();

	// operators
	const FQuat operator*(const FQuat &q) const;
	FQuat& operator*=(const FQuat &q);
	const FQuat operator/(const FQuat &q) const;
	FQuat& operator/=(const FQuat &q);
	FQuat &operator=(const FQuat &q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }

	float x, y, z, w;
};


///////////////////////////////////////////////////////////////////////
// PQ class (Position + Quaternion)

/**
 * Single-precision class that combines a position and orientation.
 */
class FPQ
{
public:
	FPQ() {}
	void FromMatrix(const FMatrix4 &matrix);
	void ToMatrix(FMatrix4 &matrix);
	void Interpolate(const FPQ &from, const FPQ &to, float f);

	// operators
	FPQ &operator=(const class FMatrix4 &matrix)
	{
		FromMatrix(matrix);
		return (*this);
	}

	FPoint3 p;
	FQuat q;
};


///////////////////////////////////////////////////////////////////////
// Color classes

/**
 * An RGB class for handling color operations.
 *
 * This one handles colors with integer components (0-255)
 */
class RGBi
{
public:
	RGBi() {}
	RGBi(short _r, short _g, short _b) { r = _r; g = _g; b = _b; }
	RGBi(const class RGBf &v) { *this = v; }

	void Set(short _r, short _g, short _b) { r = _r; g = _g; b = _b; }
	RGBi operator +(const RGBi &v) const { return RGBi(r+v.r, g+v.g, b+v.b); }
	RGBi operator +(const class RGBAi &v) const;
	RGBi operator -(const RGBi &v) const { return RGBi(r-v.r, g-v.g, b-v.b); }
	RGBi operator *(float s) const { return RGBi((short)(r*s), (short)(g*s), (short)(b*s)); }
	RGBi operator /(float s) const { return RGBi((short)(r/s), (short)(g/s), (short)(b/s)); }
	void operator *=(float s) { r=(short)(r*s); g=(short)(g*s); b=(short)(b*s); }
	void operator /=(float s) { r=(short)(r/s); g=(short)(g/s); b=(short)(b/s); }

	void operator +=(const RGBi &v) { r+=v.r; g+=v.g; b+=v.b; }
	void operator +=(const class RGBAi &v);

	void Crop();

	// assignment
	RGBi &operator=(const RGBi &v) { r = v.r; g = v.g; b = v.b; return *this; }
	RGBi &operator=(const class RGBf &v);

	bool operator==(const RGBi &v) const { return (r == v.r && g == v.g && b == v.b); }
	bool operator!=(const RGBi &v) const { return (r != v.r || g != v.g || b != v.b); }

	short r, g, b;
};

/**
 * An RGBA class for handling color operations with alpha support.
 *
 * This one handles colors with integer components (0-255)
 */
class RGBAi
{
public:
	RGBAi() {}
	RGBAi(short _r, short _g, short _b, short _a = 255) { r = _r; g = _g; b = _b; a = _a; }
	RGBAi(const class RGBi &v) { *this = v; }

	void Set(short _r, short _g, short _b, short _a = 255) { r = _r; g = _g; b = _b; a = _a; }
	RGBAi operator +(const RGBAi &v) const { return RGBAi(r+v.r, g+v.g, b+v.b, a+v.a); }
	RGBAi operator -(const RGBAi &v) const { return RGBAi(r-v.r, g-v.g, b-v.b, a+v.a); }
	RGBAi operator *(float s) const { return RGBAi((short)(r*s), (short)(g*s), (short)(b*s), (short)(a*s)); }
	RGBAi operator /(float s) const { return RGBAi((short)(r/s), (short)(g/s), (short)(b/s), (short)(a/s)); }
	void operator *=(float s) { r=(short)(r*s); g=(short)(g*s); b=(short)(b*s); a=(short)(a*s); }
	void operator /=(float s) { r=(short)(r/s); g=(short)(g/s); b=(short)(b/s); a=(short)(a/s); }
	void Crop();

	// assignment
	RGBAi &operator=(const RGBi &v) { r = v.r; g = v.g; b = v.b; a = 255; return *this; }

	short r, g, b, a;
};

inline void RGBi::operator +=(const class RGBAi &v) { r+=v.r; g+=v.g; b+=v.b; }
inline RGBi RGBi::operator +(const class RGBAi &v) const { return RGBi(r+v.r, g+v.g, b+v.b); }

/**
 * An RGB class for handling color operations.
 *
 * This one handles colors with float components (0.0 - 1.0)
 */
class RGBf
{
public:
	RGBf() {}
	RGBf(float _r, float _g, float _b) { r = _r; g = _g; b = _b; }
	RGBf(const class RGBi &v) { *this = v; }
	RGBf(const class RGBAf &v) { *this = v; }

	void Set(float _r, float _g, float _b) { r = _r; g = _g; b = _b; }
	RGBf operator +(const RGBf &v) const { return RGBf(r+v.r, g+v.g, b+v.b); }
	RGBf operator -(const RGBf &v) const { return RGBf(r-v.r, g-v.g, b-v.b); }
	RGBf operator *(float s) const { return RGBf(r*s, g*s, b*s); }
	RGBf operator /(float s) const { return RGBf(r/s, g/s, b/s); }
	void operator *=(float s) { r*=s; g*=s; b*=s; }

	// assignment
	RGBf &operator=(const RGBf &v) { r = v.r; g = v.g; b = v.b; return *this; }
	RGBf &operator=(const class RGBi &v);
	RGBf &operator=(const class RGBAf &v);

	bool operator==(const RGBf &v2) const
	{ return (r == v2.r && g == v2.g && b == v2.b); }
	bool operator!=(const RGBf &v2) const
	{ return (r != v2.r || g != v2.g || b != v2.b); }

	float r, g, b;
};

inline RGBi &RGBi::operator=(const class RGBf &v)
{
	r = (int) (v.r * 255.999f);
	g = (int) (v.g * 255.999f);
	b = (int) (v.b * 255.999f);
	return *this;
}

inline RGBf &RGBf::operator=(const class RGBi &v)
{
	r = v.r / 255.0f;
	g = v.g / 255.0f;
	b = v.b / 255.0f;
	return *this;
}


/**
 * An RGBA class for handling color operations with alpha support.
 *
 * This one handles colors with float components (0.0 - 1.0)
 */
class RGBAf
{
public:
	RGBAf() {}
	RGBAf(float _r, float _g, float _b) { r = _r; g = _g; b = _b; a = 1.0f; }
	RGBAf(float _r, float _g, float _b, float _a) { r = _r; g = _g; b = _b; a = _a; }
	RGBAf(const class RGBf &v) { *this = v; }

	void Set(float _r, float _g, float _b, float _a) { r = _r; g = _g; b = _b; a = _a; }
	RGBAf operator +(const RGBAf &v) const { return RGBAf(r+v.r, g+v.g, b+v.b, a+v.a); }
	RGBAf operator -(const RGBAf &v) const { return RGBAf(r-v.r, g-v.g, b-v.b, a+v.a); }
	RGBAf operator *(float s) const { return RGBAf(r*s, g*s, b*s); }
	RGBAf operator /(float s) const { return RGBAf(r/s, g/s, b/s); }
	void operator *=(float s) { r*=s; g*=s; b*=s; }

	// assignment
	RGBAf &operator=(const RGBf &v) { r = v.r; g = v.g; b = v.b; a = 1.0f; return *this; }

	// comparison
	bool operator==(const RGBAf &v2) const
	{ return (r == v2.r && g == v2.g && b == v2.b && a == v2.a); }
	bool operator!=(const RGBAf &v2) const
	{ return (r != v2.r || g != v2.g || b != v2.b || a != v2.a); }

	float r, g, b, a;
};

inline RGBf &RGBf::operator=(const class RGBAf &v)
{
	r = v.r;
	g = v.g;
	b = v.b;
	return *this;
}

/**
 * The locale might be set to something strange, ala European. that
 * interprets '.' as ',' and vice versa.  That terribly breaks VTP's usage
 * of printf/sscanf/atof to encode and parse numbers.
 *
 * This handy class encapsulates overriding the locale.  Use it like this
 * for 'standard' numbers:
 *
 * \example
 *		LocaleWrap normal_numbers(LC_NUMERIC, "C");
 *
 * The locale will be restored when the LocaleWrap object goes out of scope.
 */
class LocaleWrap
{
public:
	/// See the C function setlocale() for an explanation of the arguments.
	LocaleWrap(int category, const char *locale_string);
	~LocaleWrap();

protected:
	std::string m_old_locale;
};


///////////////////////////////////////////////////////////////////////
// handy helper functions
//
float random_offset(float x);
float random(float x);
int vt_log2(int n);
bool CrossingsTest(const DPoint2 *pgon, int numverts, const DPoint2 &point);
bool PointInTriangle(const FPoint2 &p, const FPoint2 &p1, const FPoint2 &p2,
					 const FPoint2 &p3);
bool PointInTriangle(const DPoint2 &p, const DPoint2 &p1, const DPoint2 &p2,
					 const DPoint2 &p3);
bool BarycentricCoords(const FPoint2 &p1, const FPoint2 &p2,
					   const FPoint2 &p3, const FPoint2 &p, float fBary[3]);
bool BarycentricCoords(const DPoint2 &p1, const DPoint2 &p2,
					   const DPoint2 &p3, const DPoint2 &p, double fBary[3]);
bool PlaneIntersection(const FPlane &plane1, const FPlane &plane2,
					   const FPlane &plane3, FPoint3 &result);
double DistancePointToLine(const DPoint2 &p1, const DPoint2 &p2, const DPoint2 &p3);
void vtLogMatrix(const FMatrix4 &mat);
void vtLogMatrix(const FMatrix3 &mat);

#endif	// VTMATHTYPESH
