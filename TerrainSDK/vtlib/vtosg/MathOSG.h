//
// A handy set of inline functions to convert between the VTP and OSG
// math classes.  v2s() converts from VTP to OSG, s2v() goes the other way.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MATHH
#define VTOSG_MATHH

#if OSG_USE_DOUBLE_MATRICES
#error "You should build OSG to use float, not double, for matrices.  If you REALLY want to, you can bypass this error, but it's not recommended."
typedef double osg_matrix_value;
#else
typedef float osg_matrix_value;
#endif

///////////////////////
// math helpers

inline void v2s(const FPoint2 &f, osg::Vec2 &s) { s[0] = f.x; s[1] = f.y; }
inline void v2s(const FPoint3 &f, osg::Vec3 &s) { s[0] = f.x; s[1] = f.y; s[2] = f.z; }
inline void v2s(const RGBf &f, osg::Vec4 &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = 1.0f; }

inline osg::Vec3 v2s(const FPoint3 &f)
{
	osg::Vec3 s;
	s[0] = f.x; s[1] = f.y; s[2] = f.z;
	return s;
}

inline osg::Vec4 v2s(const RGBf &f)
{
	osg::Vec4 s;
	s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = 1.0f;
	return s;
}

inline void s2v(const osg::Vec3 &s, FPoint3 &f) { f.x = s[0]; f.y = s[1]; f.z = s[2]; }
inline void s2v(const osg::Vec2 &s, FPoint2 &f) { f.x = s[0]; f.y = s[1]; }
inline void s2v(const osg::Vec4 &s, RGBf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2]; }

inline void s2v(const osg::BoundingSphere &bs, FSphere &sph)
{
	s2v(bs._center, sph.center);
	sph.radius = bs._radius;
}

inline void s2v(const osg::BoundingBox &bs, FBox3 &box)
{
	box.min.x = bs._min[0];
	box.min.y = bs._min[1];
	box.min.z = bs._min[2];

	box.max.x = bs._max[0];
	box.max.y = bs._max[1];
	box.max.z = bs._max[2];
}

inline FPoint3 s2v(const osg::Vec3 &s)
{
	FPoint3 f;
	f.x = s[0]; f.y = s[1]; f.z = s[2];
	return f;
}

inline RGBf s2v(const osg::Vec4 &s)
{
	RGBf f;
	f.r = s[0]; f.g = s[1]; f.b = s[2];
	return f;
}

inline void ConvertMatrix4(const osg::Matrix *mat_osg, FMatrix4 *mat)
{
	const osg_matrix_value *ptr = mat_osg->ptr();
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mat->Set(j, i, ptr[(i<<2)+j]);
		}
}

inline void ConvertMatrix4(const FMatrix4 *mat, osg::Matrix *mat_osg)
{
	osg_matrix_value *ptr = mat_osg->ptr();
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			ptr[(i<<2)+j] = mat->Get(j, i);
		}
}

#endif	// VTOSG_MATHH

