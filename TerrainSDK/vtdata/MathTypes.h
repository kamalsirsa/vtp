//
// Basic data type definitions shared by all the VTP software.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATATYPESH
#define VTDATATYPESH

#include <math.h>
#include "Array.h"
#include "Array.inl"

#ifndef PI
#define PI		3.141592653589793238
#endif
#ifndef PI2
#define PIf 	3.141592653589793238f
#define PI2 	6.283185307179586476
#define PI2f 	6.283185307179586476f
#define PID2 	1.570796326794896619
#define PID2f 	1.570796326794896619f
#endif

#ifndef NULL
#define NULL    0
#endif

class DPoint3;

class FPoint3 {
public:
	FPoint3() { x = y = z = 0.0f; }
	FPoint3(float fx, float fy, float fz) { x=fx; y=fy; z=fz; }

	float Length() const { return sqrtf(x*x+y*y+z*z); }
	float LengthSquared() const { return x*x+y*y+z*z; }
	void Normalize() { float s = 1.0f/Length(); x*=s; y*=s; z*=s; }
	void Set(float fx, float fy, float fz) { x=fx; y=fy; z=fz; }
	float Dot(const FPoint3 &rhs) const
	{
		return x*rhs.x+y*rhs.y+z*rhs.z;
	}
	float Dot(const float *fp) const
	{
		return x*fp[0]+y*fp[1]+z*fp[2];
	}
	FPoint3 Cross(const FPoint3 &v)
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

	float x, y, z;
};

class DPoint3 {
public:
	DPoint3() { x = y = z = 0.0f; }
	DPoint3(double fx, double fy, double fz) { x=fx; y=fy; z=fz; }

	double Length() const { return sqrt(x*x+y*y+z*z); }
	void Normalize() { double s = 1.0f/Length(); x*=s; y*=s; z*=s; }
	void Set(double fx, double fy, double fz) { x=fx; y=fy; z=fz; }
	DPoint3 Cross(const DPoint3 &v)
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

	// dot product
	double operator *(const DPoint3 &v) const { return x*v.x + y*v.y + z*v.z; }

	void operator +=(const DPoint3 &v) { x+=v.x; y+=v.y; z+=v.z; }
	void operator -=(const DPoint3 &v) { x-=v.x; y-=v.y; z-=v.z; }
	void operator *=(double s) { x*=s; y*=s; z*=s; }
	void operator /=(double s) { x/=s; y/=s; z/=s; }

	double x, y, z;
};

// Conversion
inline FPoint3 &FPoint3::operator=(const DPoint3 &v) { x = (float) v.x; y = (float) v.y; z = (float) v.z; return *this; }
inline DPoint3 &DPoint3::operator=(const FPoint3 &v) { x = v.x; y = v.y; z = v.z; return *this; }

/////////////////////////////////////////////

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

class FPoint2 {
public:
	FPoint2() { x = y = 0.0f; }
	FPoint2(int ix, int iy) { x=(float)ix; y=(float)iy; }
	FPoint2(float fx, float fy) { x=fx; y=fy; }
	FPoint2(double dx, double dy) { x=(float)dx; y=(float)dy; }
	FPoint2(const DPoint2 &d);

	float Length() const { return sqrtf(x*x+y*y); }
	void Normalize() { float s = 1.0f/Length(); x*=s; y*=s; }
	void Set(float fx, float fy) { x=fx; y=fy; }

	// assignment
	FPoint2 &operator=(const FPoint2 &v) { x = v.x; y = v.y; return *this; }
	FPoint2 &operator=(const class DPoint2 &v);

	FPoint2 operator +(const FPoint2 &v) const { return FPoint2(x+v.x, y+v.y); }
	FPoint2 operator -(const FPoint2 &v) const { return FPoint2(x-v.x, y-v.y); }
	FPoint2 operator *(float s) const { return FPoint2(x*s, y*s); }

	void operator +=(const FPoint2 &v) { x+=v.x; y+=v.y; }
	void operator -=(const FPoint2 &v) { x-=v.x; y-=v.y; }
	void operator *=(float s) { x*=s; y*=s; }

	bool operator==(const FPoint2 &v) { return (x == v.x && y == v.y); }
	bool operator!=(const FPoint2 &v) { return (x != v.x || y != v.y); }

	float x, y;
};

class DPoint2 {
public:
	DPoint2() { x = y = 0.0f; }
	DPoint2(int ix, int iy) { x=(double)ix; y=(double)iy; }
	DPoint2(float fx, float fy) { x=fx; y=fy; }
	DPoint2(double fx, double fy) { x=fx; y=fy; }
	DPoint2(const FPoint2 &f);

	double Length() const { return sqrt(x*x+y*y); }
	double LengthSquared() const { return (x*x+y*y); }
	void Normalize() { double s = 1.0/Length(); x*=s; y*=s; }
	void Set(double fx, double fy) { x=fx; y=fy; }
	void Rotate(double radians)
	{
		double tempx = x;
		x = x * cos(radians) - y * sin(radians);
		y = tempx * sin(radians) + y * cos(radians);
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

	bool operator==(const DPoint2 &v) { return (x == v.x && y == v.y); }

	double x, y;
};


// copy constructors FPoint2 <> DPoint2
inline FPoint2::FPoint2(const DPoint2 &d) { x=(float)d.x; y=(float)d.y; }
inline DPoint2::DPoint2(const FPoint2 &f) { x=f.x; y=f.y; }


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

//
// a series of points, which can represent either a series of line
//  segments or a closed polygon
//
class DLine2 : public Array<DPoint2>
{
public:
	DLine2() {}
	// copy constructor
	DLine2(DLine2 &ref) { *this = ref; }

	// assignment
	DLine2 &operator=(const class DLine2 &v);
	DLine2 &operator=(const class FLine2 &v);

	void RemovePoint(int i);
	bool ContainsPoint(const DPoint2 &p);
};

class FLine2 : public Array<FPoint2>
{
public:
	FLine2() {}
	// copy constructor
	FLine2(FLine2 &ref) { *this = ref; }

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

class DLine3 : public Array<DPoint3>
{
public:
	DLine3() {}
	// copy constructor
	DLine3(DLine3 &ref) { *this = ref; }

	// assignment
	DLine3 &operator=(const class DLine3 &v);
//	DLine3 &operator=(const class FLine3 &v);

	void RemovePoint(int i);
};

class FLine3 : public Array<FPoint3>
{
public:
	FLine3() {}
	// copy constructor
	FLine3(FLine3 &ref) { *this = ref; }

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

class FBox3
{
public:
	FBox3() {}
	FBox3(FPoint3 &min1, FPoint3 &max1) { min = min1; max = max1; }

	void Set(float x1, float y1, float z1, float x2, float y2, float z2)
	{
		min.Set(x1, y1, z1);
		max.Set(x2, y2, z2);
	}
	FPoint3 Center() const { return ((min + max) * 0.5); }

	FPoint3	min, max;
};

/////////////////////////////////////////////

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

//
// a collection of polygons
//
class DPolyArray2 : public Array<DLine2 *>
{
public:
	DPolyArray2() { m_previous_poly = -1; }

	int FindPoly(DPoint2 p);

	// for speed, remember the polygon that we found last time
	int m_previous_poly;
};


//
// DRECT / FRECT
//
// These rectangle classes are meant to be used for geographical areas,
// or texture coordinates, which assume that the vertical axis increases
// upward, not like typical screen coordinates which increase downwards.
//
class DRECT
{
public:
	DRECT() { left = top = right = bottom = 0.0; }
	DRECT(double l, double t, double r, double b) { left = l; top = t; right = r; bottom = b; }

	void SetRect(double l, double t, double r, double b) { left = l; top = t; right = r; bottom = b; }
	// retrieves the width
	double Width() const { return right - left; }
	// returns the height
	double Height() const { return top - bottom; };
	// return true if null (all 0)
	bool IsNull() const { return (left == 0.0 && top == 0.0 && right == 0.0 && bottom == 0.0); }
	// return true if empty
	bool IsEmpty() const { return (left == right && top == bottom); }
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

class FRECT
{
public:
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

	float	left;
	float	top;
	float	right;
	float	bottom;
};

/////////////////////////////////////////////////////////////
// Matrix classes

class DMatrix3;
class DMatrix4;

class DMatrix3
{
public:
	void  Set(int i1, int i2, double v) { data[i1][i2] = v; }
	double Get(int i1, int i2) const { return data[i1][i2]; }

	void Transform(const DPoint3 &src, DPoint3 &dst) const;
    void SetByMatrix4(const DMatrix4 &m);

protected:
	double data[3][3];
};
class DMatrix4
{
public:
	void  Set(int i1, int i2, double v) { data[i1][i2] = v; }
	double Get(int i1, int i2) const { return data[i1][i2]; }

	void Identity();
	void AxisAngle(const DPoint3 &vec, double theta);
	void Invert(const DMatrix4 &src);

protected:
	double data[4][4];
};

//////////////

class FMatrix3
{
public:
	void  Set(int i1, int i2, float v) { data[i1][i2] = v; }
	float Get(int i1, int i2) const { return data[i1][i2]; }

	void Transform(const FPoint3 &src, FPoint3 &dst) const;
    void SetByMatrix4(const class FMatrix4 &m);

protected:
	float data[3][3];
};

class FMatrix4
{
public:
	void  Set(int i1, int i2, float v) { data[i1][i2] = v; }
	float Get(int i1, int i2) const { return data[i1][i2]; }

	// set/modify
	void Identity();
	bool IsIdentity() const;
	void AxisAngle(const FPoint3 &vec, double theta);
	void Translate(const FPoint3 &vec);
	void Invert(const FMatrix4 &src);
	FPoint3 GetTrans() const;
	void SetTrans(FPoint3 pos);

	// apply to vectors
	void Transform(const FPoint3 &src, FPoint3 &dst) const;
	void TransformVector(const FPoint3 &tmp, FPoint3 &dst) const;

protected:
	float data[4][4];
};

//////////////////////////////////////////////////////////////
//
// rgb class for handling color operations
//
// this one handles colors with integer components (0-255)
//
class RGBi
{
public:
	RGBi() {}
	RGBi(short _r, short _g, short _b) { r = _r; g = _g; b = _b; }
	RGBi(const class RGBf &v) { *this = v; }

	void Set(short _r, short _g, short _b) { r = _r; g = _g; b = _b; }
	RGBi operator +(const RGBi &v) const { return RGBi(r+v.r, g+v.g, b+v.b); }
	RGBi operator -(const RGBi &v) const { return RGBi(r-v.r, g-v.g, b-v.b); }
	RGBi operator *(float s) const { return RGBi((short)(r*s), (short)(g*s), (short)(b*s)); }
	RGBi operator /(float s) const { return RGBi((short)(r/s), (short)(g/s), (short)(b/s)); }
	void operator *=(float s) { r=(short)(r*s); g=(short)(g*s); b=(short)(b*s); }

	// assignment
	RGBi &operator=(const RGBi &v) { r = v.r; g = v.g; b = v.b; return *this; }
	RGBi &operator=(const class RGBf &v);

	bool operator==(const RGBi &v) { return (r == v.r && g == v.g && b == v.b); }
	bool operator!=(const RGBi &v) { return (r != v.r || g != v.g || b != v.b); }

	short r, g, b;
};

//
// this one handles colors with float components (0-1)
//
class RGBf
{
public:
	RGBf() {}
	RGBf(float _r, float _g, float _b) { r = _r; g = _g; b = _b; }
	RGBf(const class RGBi &v) { *this = v; }

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

	bool operator==(const RGBf &v2)
	{ return (r == v2.r && g == v2.g && b == v2.b); }
	bool operator!=(const RGBf &v2)
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

//////////
// And one with alpha value support

class RGBAf
{
public:
	RGBAf() {}
	RGBAf(float _r, float _g, float _b) { r = _r; g = _g; b = _b; a = 1.0f; }
	RGBAf(float _r, float _g, float _b, float _a) { r = _r; g = _g; b = _b; a = _a; }
	RGBAf(const class RGBf &v) { *this = v; }

	void Set(float _r, float _g, float _b) { r = _r; g = _g; b = _b; }
	RGBAf operator +(const RGBAf &v) const { return RGBAf(r+v.r, g+v.g, b+v.b, a+v.a); }
	RGBAf operator -(const RGBAf &v) const { return RGBAf(r-v.r, g-v.g, b-v.b, a+v.a); }
	RGBAf operator *(float s) const { return RGBAf(r*s, g*s, b*s); }
	RGBAf operator /(float s) const { return RGBAf(r/s, g/s, b/s); }
	void operator *=(float s) { r*=s; g*=s; b*=s; }

	// assignment
	RGBAf &operator=(const RGBf &v) { r = v.r; g = v.g; b = v.b; a = 1.0f; return *this; }

	float r, g, b, a;
};

inline RGBf &RGBf::operator=(const class RGBAf &v)
{
	r = v.r;
	g = v.g;
	b = v.b;
	return *this;
}


//
// handy helper functions
//
float random_offset(float x);
float random(float x);
bool CrossingsTest(DPoint2 *pgon, int numverts, const DPoint2 &point);

#endif
