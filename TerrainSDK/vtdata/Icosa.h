//
// Icosa.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ICOSAH
#define ICOSAH

#include "MathTypes.h"

struct uvpoly {
	Array<FPoint2> uv;
};

struct icoface {
	DPoint3 base;
	DPoint3 center;
	DPoint3 vec_a, vec_b, vec_c;	// normalized basis vectors for face

	DMatrix3 trans;

	double d;	// for plane equation

	// map data
	Array<uvpoly *> m_poly;
};

#endif
