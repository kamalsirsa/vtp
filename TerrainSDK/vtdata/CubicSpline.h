//
// (Natural) Cubic Spline class.
//
// Originally adapted from some LGPL code found around the net.
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __CUBIC_SPLINE_H__
#define __CUBIC_SPLINE_H__

#include <list>
#include "MathTypes.h"

class CubicSpline
{
public:
	CubicSpline();

	virtual ~CubicSpline();

	/** re-initialization */
	void Cleanup();

	/** accumlate interporation point */
	int AddPoint(const DPoint3 &vec);

	/** generate spline coeffs */
	bool Generate();

	/** perform interpolation */
	bool Interpolate(double f, DPoint3 *vec,
					DPoint3 *dvec = NULL, DPoint3 *ddvec = NULL);

	int getPoints() const { return m_nPoints; }

private:
	std::list<DPoint3> m_veclist;

	int m_iPoints;
	DPoint3 *m_pCoeff0;
	DPoint3 *m_pCoeff1;
	DPoint3 *m_pCoeff2;
	DPoint3 *m_pCoeff3;
};

#endif	// __CUBIC_SPLINE_H__

