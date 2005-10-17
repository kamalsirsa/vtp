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

// Simple cache of tiles loaded from disk
typedef unsigned char *ucharptr;
struct CacheEntry { unsigned char *data; int size; };
typedef std::map<std::string, CacheEntry> TileCache;

/**
 * This class represents a tiled textured terrain heightfield, which is drawn
 * using the tiled paging capabilities of Roettger's libMini.  It is rendered
 * directly using OpenGL, instead of going through whichever scene graph vtlib is
 * built on.
 */
class vtTiledGeom : public vtDynGeom, public vtHeightField3d
{
public:
	vtTiledGeom();
	~vtTiledGeom();

	bool ReadTileList(const char *dataset_fname_elev, const char *dataset_fname_image);
	void SetVerticalExag(float fExag);
	float GetVerticalExag() { return m_fDrawScale; }
	void SetVertexTarget(int iVertices);
	int GetVertexTarget() { return m_iVertexTarget; }
	FPoint2 GetWorldSpacingAtPoint(const DPoint2 &p);

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

	// Tile cache methods
	unsigned char *FetchAndCacheTile(const char *fname);
	void EmptyCache();

	// Values used to initialize miniload
	int cols, rows;
	float coldim, rowdim;
	int lod0size;
	FPoint3 center;
	ucharptr *hfields, *textures;

	// Values used to render each frame
	IPoint2 m_window_size;
	FPoint3 m_eyepos_ogl;
	float m_fFOVY;
	float m_fAspect;
	float m_fNear, m_fFar;
	FPoint3 eye_up, eye_forward;

	// vertical scale (exaggeration)
	float m_fMaximumScale;
	float m_fHeightScale;
	float m_fDrawScale;

	// detail level and vertex target
	float m_fResolution;
	float m_fHResolution;
	float m_fLResolution;
	int m_iVertexTarget;

	// the libMini objects
	class miniload *m_pMiniLoad;
	class minitile *m_pMiniTile;
	class minicache *m_pMiniCache;	// This is cache of OpenGL primitives to be rendered

	// CRS of this tileset
	vtProjection m_proj;

	// Tile cache in host RAM, to reduce loading from disk
	TileCache m_Cache;

protected:
	void SetupMiniLoad();
};

/*@}*/	// Group dynterr

#endif // TILEDGEOMH

