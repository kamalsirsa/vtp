//
// Fences.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCESH
#define FENCESH

#include <stdio.h>
#include "MathTypes.h"
#include "Structure.h"

enum vtLinearStyle
{
	FS_WOOD_POSTS_WIRE,
	FS_METAL_POSTS_WIRE,
	FS_CHAINLINK,
	FS_DRYSTONE,
	FS_STONE,
	FS_PRIVET,
	FS_RAILING_ROW,
	FS_RAILING_CABLE,
	FS_RAILING_EU,
	FS_TOTAL
};

#define FENCE_DEFAULT_HEIGHT	1.2f
#define FENCE_DEFAULT_SPACING	2.5f

class vtLinearParams
{
public:
	void Defaults();
	void Blank();
	void ApplyStyle(vtLinearStyle style);
	void WriteXML(GZOutput &out) const;

	// Posts
	vtString	m_PostType;	// wood, steel, none
	float	m_fPostHeight;
	float	m_fPostSpacing;
	float	m_fPostWidth;
	float	m_fPostDepth;

	// Connect
	vtString	m_ConnectType;	// wire, chain-link, privet, drystone, none
	float	m_fConnectTop;
	float	m_fConnectBottom;
	float	m_fConnectWidth;
	// ConnectWidth assumed same at top and bottom
};

/**
 * The vtFence class represents any linear structure.  This can be a fence,
 * a wall, a railing, or any other kind of build structure which follows
 * a line on the ground.
 */
class vtFence : public vtStructure
{
public:
	vtFence();

	// copy operator
	vtFence &operator=(const vtFence &v);

	// geometry methods
	void AddPoint(const DPoint2 &epos);
	DLine2 &GetFencePoints() { return m_pFencePts; }
	void GetClosestPoint(const DPoint2 &point, DPoint2 &closest);
	double GetDistanceToLine(const DPoint2 &point);
	bool IsContainedBy(const DRECT &rect) const;
	bool GetExtents(DRECT &rect) const;

	// IO
	void WriteXML(GZOutput &out, bool bDegrees) const;

	// style
	void ApplyStyle(vtLinearStyle style);
	vtLinearParams &GetParams() { return m_Params; }
	void SetParams(const vtLinearParams &params) { m_Params = params; }

protected:
	DLine2		m_pFencePts;	// in earth coordinates

	vtLinearParams	m_Params;
};

#endif	// FENCESH

