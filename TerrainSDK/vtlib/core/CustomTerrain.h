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
	~CustomTerrain();

	// initialization
	bool Init(vtLocalGrid *pGrid, float fZScale,
				float fOceanDepth, int &iError);

	// overrides
	void DoRender();
	void DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov);
	void GetLocation(int iX, int iZ, FPoint3 &p);

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
