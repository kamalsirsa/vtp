//
// PolyChecker.h: interface for the PolyChecker class by Roger James.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef POLYCHECKER_H
#define POLYCHECKER_H

#include "MathTypes.h"

// I really need to stop defining this!
#define  POLYEPSILON 0.000001

class PolyChecker
{
public:
	PolyChecker();
	virtual ~PolyChecker();

	bool Xor(bool x, bool y);
	bool Intersect(DPoint2 a, DPoint2 b, DPoint2 c, DPoint2 d);
	bool IntersectProp(DPoint2 a, DPoint2 b, DPoint2 c, DPoint2 d);
	bool Between(DPoint2 a, DPoint2 b, DPoint2 c);
	bool Left(DPoint2 a, DPoint2 b, DPoint2 c);
	bool LeftOn(DPoint2 a, DPoint2 b, DPoint2 c);
	bool Collinear(DPoint2 a, DPoint2 b, DPoint2 c);
	double Area2(DPoint2 a, DPoint2 b, DPoint2 c);
	int AreaSign(DPoint2 a, DPoint2 b, DPoint2 c);
	bool IsSimplePolygon(DLine2 &vertices);
	bool IsSimplePolygon(DLine3 &vertices);
	bool Intersect(FPoint2 a, FPoint2 b, FPoint2 c, FPoint2 d);
	bool IntersectProp(FPoint2 a, FPoint2 b, FPoint2 c, FPoint2 d);
	bool Between(FPoint2 a, FPoint2 b, FPoint2 c);
	bool Left(FPoint2 a, FPoint2 b, FPoint2 c);
	bool LeftOn(FPoint2 a, FPoint2 b, FPoint2 c);
	bool Collinear(FPoint2 a, FPoint2 b, FPoint2 c);
	float Area2(FPoint2 a, FPoint2 b, FPoint2 c);
	int AreaSign(FPoint2 a, FPoint2 b, FPoint2 c);
	bool IsSimplePolygon(FLine2 &vertices);
	bool IsSimplePolygon(FLine3 &vertices);
};

#endif // POLYCHECKER_H

