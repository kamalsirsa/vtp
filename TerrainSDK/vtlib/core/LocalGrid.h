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
	void SetGlobalProjection();
	void GetWorldExtents(FRECT &rect);
	float GetLocalValue(int i, int j) {
		if (m_bFloatMode)
			return m_pFData[i*m_iRows+j] * m_fHeightScale;
		else
			return m_pData[i*m_iRows+j] * m_fHeightScale;
	}
	void GetLocation(int i, int j, FPoint3 &loc);

	void Setup(float fVerticalExag);

	void ColorDibFromElevation(vtDIB *pDIB, RGBi color_ocean);
	void ShadeDibFromElevation(vtDIB *pDIB, FPoint3 light_dir,float light_adj,
									int xPatch=0, int yPatch=0, int nPatches = 1);
	bool FindAltitudeAtPoint(FPoint3 &p3, float &fAltitude, FPoint3 *vNormal = NULL);

protected:
	// bounds in the application-specific coordinate scheme
	FRECT	m_WorldExtents;

	float	m_fXStep, m_fZStep;
	float	m_fHeightScale;
};

#endif
