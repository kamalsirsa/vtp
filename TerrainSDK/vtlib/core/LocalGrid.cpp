//
// vtLocalGrid class:
//
// a subclass of ElevationGrid, cast into local coordinates, with
//		the ability to do terrain following
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/LocalConversion.h"
#include "LocalGrid.h"
#include "HeightField.h"

vtLocalGrid::vtLocalGrid() : vtElevationGrid()
{
	m_fXStep = m_fZStep = 0;
}

vtLocalGrid::vtLocalGrid(const DRECT &area, int iColumns, int iRows,
						 bool bFloat, vtProjection &proj, float fVerticalExag) :
	vtElevationGrid(area, iColumns, iRows, bFloat, proj)
{
	SetupConversion(fVerticalExag);
}

void vtLocalGrid::SetGlobalProjection()
{
	// copy projection and conversion to global variable
	g_Conv = m_Conversion;
}

void vtLocalGrid::SetupConversion(float fVerticalExag)
{
	m_Conversion.Setup(m_proj.GetUnits(), m_area);
	m_Conversion.m_fVerticalScale = fVerticalExag;

	// determine step size between each height sample
	m_fXStep = m_WorldExtents.Width() / (m_iColumns - 1);
	m_fZStep = -m_WorldExtents.Height() / (m_iRows - 1);
}

DPoint2 vtLocalGrid::GetWorldSpacing()
{
	return DPoint2(m_fXStep, m_fZStep);
}

