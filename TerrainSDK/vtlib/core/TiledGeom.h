//
// vtTiledGeom : Renders tiled heightfields using Roettger's libMini library
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TILEDGEOMH
#define TILEDGEOMH

#include "vtdata/HeightField.h"

/** \addtogroup dynterr */
/*@{*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
class TiledDatasetDescription
{
public:
	bool Read(const char *ini_fname);
	bool GetCorners(DLine2 &line, bool bGeo) const;

	int cols, rows;
	int lod0size;
	DRECT earthextents;
	vtProjection proj;
};
#endif

#include "mini.h"
#include "miniload.hpp"
#include "minicache.hpp"

typedef unsigned char *ucharptr;

/**
 * This class represents a textured terrain heightfield, which is drawn using
 * the terrain paging capabilities of the libMini.  It is rendered directly
 * using OpenGL, instead of going through whichever scene graph vtlib is
 * built on.
 */
class vtTiledGeom : public vtDynGeom, public vtHeightField3d
{
public:
	vtTiledGeom();
	~vtTiledGeom();

	bool ReadTileList(const char *dataset_fname_elev, const char *dataset_fname_image);

	// overrides for vtDynGeom
	void DoRender();
	void DoCalcBoundBox(FBox3 &box);
	void DoCull(const vtCamera *pCam);

	// overrides for vtHeightField
	bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue = false) const;

	// overrides for vtHeightField3d
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, bool bIncludeCulture = false,
		FPoint3 *vNormal = NULL) const;
	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;

	int cols, rows;
	float coldim, rowdim;
	int lod0size;
	ucharptr *hfields, *textures;
	float exaggeration;
	float res;
	float minres;

	IPoint2 m_window_size;
	FPoint3 m_eyepos_ogl;
	float m_fFOVY;
	float m_fAspect;
	float m_fNear, m_fFar;
	FPoint3 eye_up, eye_forward;
	FPoint3 center;

	// the terrain and its cache
	class miniload *m_pMiniLoad;
	class minitile *m_pMiniTile;
	class minicache cache;

	// CRS of this tileset
	vtProjection m_proj;

protected:
	void SetupMiniLoad();
};

/*@}*/	// Group dynterr

#endif // TILEDGEOMH

