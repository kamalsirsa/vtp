//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPSM_MATHH
#define VTPSM_MATHH

///////////////////////
// math helpers

//
// From VTLIB to PSM
//
inline void v2p(const FPoint2 &v, PSVec2 &p) { p.x = v.x; p.y = v.y; }
inline void v2p(const FPoint3 &v, PSVec3 &p) { p.x = v.x; p.y = v.y; p.z = v.z; }
inline void v2p(const FBox3 &v, PSBox3 &p)
{
	p.Set(v.min.x, v.min.y, v.min.z,
		  v.max.x, v.max.y, v.max.z);
}
inline void v2p(const RGBf &v, PSColor &p)
{
	p.r = (unsigned char) (v.r * 255.0f);
	p.g = (unsigned char) (v.g * 255.0f);
	p.b = (unsigned char) (v.b * 255.0f);
	p.a = (unsigned char) 255;
}
inline void v2p(const FSphere &v, PSSphere &p)
{
	v2p(v.center, p.Center);
	p.Radius = v.radius;
}

inline void ConvertMatrix4(const FMatrix4 *mat, PSMatrix *mat_psm)
{
	const FMatrix4Data &data = mat->GetData();
	// class data has identical layout in memory
	mat_psm->SetData(& data);
}

// Single-argument varieties
inline PSVec3 v2p(const FPoint3 &v) { return PSVec3(v.x, v.y, v.z); }
inline PSBox3 v2p(const FBox3 &v)
{
	return PSBox3(v.min.x, v.min.y, v.min.z,
				  v.max.x, v.max.y, v.max.z);
}
inline PSColor v2p(const RGBf &v)
{
	return PSColor((unsigned char) (v.r * 255.0f), (unsigned char) (v.g * 255.0f),
				   (unsigned char) (v.b * 255.0f), (unsigned char) 255);
}

//
// From PSM to VTLIB
//
inline void p2v(const PSVec2 &p, FPoint2 &v) { v.x = p.x; v.y = p.y; }
inline void p2v(const PSVec3 &p, FPoint3 &v) { v.x = p.x; v.y = p.y; v.z = p.z; }
inline void p2v(const PSBox3 &p, FBox3 &v) { v.Set(p.min.x, p.min.y, p.min.z, p.max.x, p.max.x, p.max.z); }
//inline void p2v(PSVec4 &p, RGBf &v) { v.r = p.x; v.g = p.y; v.b = p.z; }

inline void p2v(const PSSphere &p, FSphere &v)
{
	p2v(p.Center, v.center);
	v.radius = p.Radius;
}
inline void p2v(const PSColor &p, RGBf &v)
{
	v.r = ((float)p.r / 255.0f);
	v.g = ((float)p.g / 255.0f);
	v.b = ((float)p.b / 255.0f);
}

inline void ConvertMatrix4(const PSMatrix *mat_psm, FMatrix4 *mat)
{
	MatrixData *ptr = mat_psm->GetData();
	// class data has identical layout in memory
	mat->SetData(*ptr);
}

// Single-argument varieties
inline FBox3 p2v(const PSBox3 &p)
{
	FBox3 v;
	v.Set(p.min.x, p.min.y, p.min.z,
		  p.max.x, p.max.y, p.max.z);
	return v;
}
inline FPoint3 p2v(const PSVec3 &p) { return FPoint3(p.x, p.y, p.z); }
inline RGBf p2v(const PSColor &p)
{
	return RGBf((float)p.r / 255.0f, (float)p.g / 255.0f, (float)p.b / 255.0f);
}

#endif	// VTPSM_MATHH

