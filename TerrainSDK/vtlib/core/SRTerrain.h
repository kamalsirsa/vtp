//
// SRTerrain class : a subclass of vtDynTerrainGeom which exposes
//  Stefan Roettger's CLOD algorithm.
//
// utilizes: Roettger's MINI library implementation
// http://wwwvis.informatik.uni-stuttgart.de/~roettger
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef SRTERRAINH
#define SRTERRAINH

#include "DynTerrain.h"

#include "mini.h"
#include "ministub.hpp"

/*!
	The SRTerrain class implements Stefan Roettger's algorithm for
	regular-grid terrain LOD.  It was adapted directly from his sample
	implementation and correspondence with him.
*/
class SRTerrain : public vtDynTerrainGeom
{
public:
	SRTerrain();

	// initialization
	bool Init(vtElevationGrid *pGrid, float fZScale,
				float fOceanDepth, int &iError);

	// overrides
	void DoRender();
	void DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov);
	float GetElevation(int iX, int iZ) const;
	void GetWorldLocation(int iX, int iZ, FPoint3 &p) const;

	void LoadSingleMaterial();
	void LoadBlockMaterial(int a, int b);

	int		m_iBlockSize;

protected:
	// rendering
	void RenderSurface();
	void RenderPass();

	// cleanup
	virtual ~SRTerrain();

private:
	ministub *m_pMini;

	IPoint2 m_window_size;
	FPoint3 m_eyepos_ogl;
	float m_fFOVY;

	float m_fResolution;
	float m_fHeightScale;
};

#endif	// SRTerrain
