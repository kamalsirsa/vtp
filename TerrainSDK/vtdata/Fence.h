//
// Fences.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCESH
#define FENCESH

#include <stdio.h>
#include "MathTypes.h"

enum FenceType
{
	FT_WIRE,
	FT_CHAINLINK
};

#define FENCE_DEFAULT_HEIGHT	1.2f
#define FENCE_DEFAULT_SPACING	2.5f


class vtFence
{
public:
	vtFence();
	vtFence(FenceType type, float fHeight, float fSpacing);

	void AddPoint(const DPoint2 &epos);
	void SetPostSize(FPoint3 size ) { m_PostSize = size; }

	void SetFenceType(FenceType type) { m_FenceType = type; }
	FenceType GetFenceType() { return m_FenceType; }

	DLine2 &GetFencePoints() { return m_pFencePts; }
	void GetClosestPoint(const DPoint2 &point, DPoint2 &closest);

	void SetHeight(float meters) { m_fHeight = meters; }
	float GetHeight() { return m_fHeight; }

	void SetSpacing(float meters) { m_fSpacing = meters; }
	float GetSpacing() { return m_fSpacing; }

	bool GetExtents(DRECT &rect);

	void WriteXML(FILE *fp, bool bDegrees);

protected:
	DLine2		m_pFencePts;	// in earth coordinates
	FenceType	m_FenceType;
	FPoint3		m_PostSize;

	float	m_fHeight;
	float	m_fSpacing;
};

#endif
