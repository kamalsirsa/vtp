//
// CustomTerrain class : Dynamically rendering terrain
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CUSTOMTERRAINH
#define CUSTOMTERRAINH

#include "DynTerrain.h"

/**
 * This class provides an example of how to add a CLOD implementation to
 * the vtlib library.  It contains the bare skeleton of a terrain rendering
 * algorithm, taking an elevation grid as input and rendering all the
 * geometry via OpenGL each frame.
 *
 * To add, or wrap, your own terrain rendering algorithm, simply make a
 * copy of CustomTerrain and rename it, then fill in the methods with your
 * own functionality.
 */
class CustomTerrain : public vtDynTerrainGeom
{
public:
	CustomTerrain();

	// initialization
	bool Init(vtElevationGrid *pGrid, float fZScale,
			  float fOceanDepth, int &iError);

	// overrides
	void DoRender();
	void DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov);
	float GetElevation(int iX, int iZ) const;
	void GetWorldLocation(int iX, int iZ, FPoint3 &p) const;

protected:
	virtual ~CustomTerrain();

	// rendering
	void RenderSurface();
	void RenderPass();
	void LoadSingleMaterial();

private:
	float *m_pData;			// the elevation height array
	float m_fZScale;
};

#endif
