//
// CustomTerrain class : Dynamically rendering terrain
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CUSTOMTERRAINH
#define CUSTOMTERRAINH

#include "DynTerrain.h"

class CustomTerrain : public vtDynTerrainGeom
{
public:
	CustomTerrain();

	// initialization
	bool Init(vtElevationGrid *pGrid, float fZScale,
			  float fOceanDepth, int &iError);
	void Destroy();

	// overrides
	void DoRender();
	void DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov);
	float GetElevation(int iX, int iZ) const;
	void GetLocation(int iX, int iZ, FPoint3 &p) const;

protected:
	// rendering
	void RenderSurface();
	void RenderPass();
	void LoadSingleMaterial();

private:
	float *m_pData;			// the elevation height array
	float m_fZScale;
};

#endif
