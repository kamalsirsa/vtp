//
// vtLocalGrid
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAIN_LOCALGRIDH
#define TERRAIN_LOCALGRIDH

#include "vtdata/ElevationGrid.h"
#include "vtdata/vtDIB.h"
#include "LocalProjection.h"

class vtLocalGrid : public vtElevationGrid
{
public:
	vtLocalGrid();
	vtLocalGrid(const DRECT &area, int iColumns, int iRows, bool bFloat,
		vtProjection &proj, float fVerticalExag = 1.0f);

	void SetGlobalProjection();
	float GetWorldValue(int i, int j);
	void GetWorldLocation(int i, int j, FPoint3 &loc);

	void ShadeDibFromElevation(vtDIB *pDIB, FPoint3 light_dir,float light_adj);
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude, FPoint3 *vNormal = NULL);
	DPoint2 GetWorldSpacing();

	void SetupConversion(float fVerticalExag);
	vtLocalConversion	m_Conversion;

protected:
	float	m_fXStep, m_fZStep;
};

#endif
