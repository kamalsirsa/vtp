//
// DemeterTerrain class
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef DEMETERTERRAINH
#define DEMETERTERRAINH

#include "DynTerrain.h"
namespace Demeter { class Terrain; class DemeterDrawable; }

/** \addtogroup dynterr */
/*@{*/

/**
 * This class wraps the Demeter CLOD terrain rendering algorithm for use in
 * the vtlib library.
 *
 * This implementation is unfinished and disabled.
 */
class DemeterTerrain : public vtDynTerrainGeom
{
public:
	DemeterTerrain();
	~DemeterTerrain();

	// initialization
	DTErr Init(const vtElevationGrid *pGrid, float fZScale);

	// overrides
	void DoRender();
	void DoCulling(const vtCamera *pCam);
	void GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue = false) const;

protected:
	// rendering
	void RenderSurface();
	void RenderPass();
	void LoadSingleMaterial();

private:
	Demeter::Terrain	*m_pTerrain;
	Demeter::DemeterDrawable *m_pDrawable;

	float *m_pData;			// the elevation height array
	float m_fZScale;
};

/*@}*/	// Group dynterr

#endif	// DEMETERTERRAINH
