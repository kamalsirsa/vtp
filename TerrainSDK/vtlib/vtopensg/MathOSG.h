//
// A handy set of inline functions to convert between the VTP and OpenSG
// math classes.  v2s() converts from VTP to OpenSG, s2v() goes the other way.
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MATHH
	#define VTOSG_MATHH

	#include <OpenSG/OSGVector.h>
	#include <OpenSG/OSGMatrix.h>
	#include <OpenSG/OSGBoxVolume.h>
	#include <OpenSG/OSGSphereVolume.h>

typedef osg::Real32 osg_matrix_value;

///////////////////////
// math helpers
inline void v2s(const FPoint2 &f, osg::Vec2f &s) { s[0] = f.x; s[1] = f.y;}
inline void v2s(const FPoint2 &f, osg::Pnt2f &s) { s[0] = f.x; s[1] = f.y;}
inline void v2s(const FPoint3 &f, osg::Vec3f &s) { s[0] = f.x; s[1] = f.y; s[2] = f.z;}
inline void v2s(const FPoint3 &f, osg::Pnt3f &s) { s[0] = f.x; s[1] = f.y; s[2] = f.z;}
inline void v2s(const RGBf &f, osg::Vec3f &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b;}
inline void v2s(const RGBAf &f, osg::Vec4f &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = f.a;}
inline void v2s(const RGBf &f, osg::Color3f &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b;}

inline osg::Vec3f v2s(const FPoint3 &f)
{
	osg::Vec3f s;
	s[0] = f.x; s[1] = f.y; s[2] = f.z;
	return s;
}

inline osg::Vec4f v2s(const RGBf &f)
{
	osg::Vec4f s;
	s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = 1.0f;
	return s;
}

inline osg::Vec4f v2s(const RGBAf &f)
{
	osg::Vec4f s;
	s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = f.a;
	return s;
}

inline void s2v(const osg::Vec3f &s, FPoint3 &f) { f.x = s[0]; f.y = s[1]; f.z = s[2];}
inline void s2v(const osg::Vec2f &s, FPoint2 &f) { f.x = s[0]; f.y = s[1];}
inline void s2v(const osg::Vec3f &s, RGBf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2];}
inline void s2v(const osg::Vec4f &s, RGBAf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2]; f.a = s[3];}

inline void s2v(const osg::SphereVolume &bs, FSphere &sph)
{
	s2v( bs.getCenter(), sph.center);
	sph.radius = static_cast<float> (bs.getRadius());
}

inline void s2v(const osg::BoxVolume &bs, FBox3 &box)
{
	bs.getBounds( box.min.x, box.min.y, box.min.z,
				  box.max.x, box.max.y, box.max.z );
}

inline FPoint3 s2v(const osg::Vec3f &s)
{
	FPoint3 f;
	f.x = s[0]; f.y = s[1]; f.z = s[2];
	return f;
}

inline RGBf s2v(const osg::Vec4f &s)
{
	RGBf f;
	f.r = s[0]; f.g = s[1]; f.b = s[2];
	return f;
}

inline void ConvertMatrix4(const osg::Matrix4f *mat_osg, FMatrix4 *mat)
{
	const osg::Real32 *ptr = mat_osg->getValues(); 
	int i, j;
	for( i = 0; i < 4; i++ )
		for( j = 0; j < 4; j++ ) {
			mat->Set(j, i, ptr[(i<<2)+j] );
		}
}

inline void ConvertMatrix4(const FMatrix4 *mat, osg::Matrix4f *mat_osg)
{
	osg::Real32 *ptr = mat_osg->getValues();
	int i, j;
	for( i = 0; i < 4; i++ )
		for( j = 0; j < 4; j++ ) {
			ptr[(i<<2)+j] = (osg::Real32)mat->Get(j, i);
		}
}

#endif	// VTOSG_MATHH

