//
// Fences.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCESH
#define FENCESH

#include <stdio.h>
#include "MathTypes.h"
#include "Structure.h"

enum FenceType
{
	FT_WIRE,
	FT_CHAINLINK,
	FT_HEDGEROW,
	FT_DRYSTONE,
	FT_PRIVET,
	FT_BEECH,
	FT_STONE
};

#define FENCE_DEFAULT_HEIGHT	1.2f
#define FENCE_DEFAULT_SPACING	2.5f

/**
 * An encapsulation of the various paramters used to create a linear structure.
 */
struct LinStructOptions
{
	FenceType eType;
	float fHeight;
	float fSpacing;
};

class vtFence : public vtStructure
{
public:
	vtFence();
	vtFence(FenceType type, float fHeight, float fSpacing);

	// copy operator
	vtFence &operator=(const vtFence &v);

	void AddPoint(const DPoint2 &epos);
	void SetOptions(const LinStructOptions &opt);
	void SetPostSize(FPoint3 size ) { m_PostSize = size; }

	void SetFenceType(const FenceType type);
	FenceType GetFenceType() { return m_FenceType; }

	DLine2 &GetFencePoints() { return m_pFencePts; }
	void GetClosestPoint(const DPoint2 &point, DPoint2 &closest);
	double GetDistanceToLine(const DPoint2 &point);

	void SetHeight(float meters) { m_fHeight = meters; }
	float GetHeight() { return m_fHeight; }

	void SetSpacing(float meters) { m_fSpacing = meters; }
	float GetSpacing() { return m_fSpacing; }

	bool GetExtents(DRECT &rect) const;

	void WriteXML_Old(FILE *fp, bool bDegrees);
	void WriteXML(GZOutput &out, bool bDegrees);

	bool IsContainedBy(const DRECT &rect) const;

protected:
	DLine2		m_pFencePts;	// in earth coordinates
	FenceType	m_FenceType;
	FPoint3		m_PostSize;

	float	m_fHeight;
	float	m_fSpacing;
};

#endif
