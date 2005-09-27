//
// DynTerrain class : Dynamically rendering terrain
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef DYNTERRAINH
#define DYNTERRAINH

#include "vtdata/ElevationGrid.h"
#include "vtdata/HeightField.h"

/** \defgroup dynterr Dynamic Terrain (CLOD)
 * These classes are used for dynamically rendered terrain surfaces, including
 * implementations of Continuous Level of Detail (CLOD).
 */
/*@{*/

// A set of errors that may be returned from the dynamic terrain classes.
enum DTErr
{
	DTErr_OK,
	DTErr_EMPTY_EXTENTS,
	DTErr_NOTSQUARE,
	DTErr_NOTPOWER2,
	DTErr_NOMEM
};

/**
 * This class provides a framework for implementing any kind of dynamic
 * geometry for a heightfield terrain grid.  It is the parent class which
 * contains common fuctionality used by each of the terrain CLOD
 * implementations.
 */
class vtDynTerrainGeom : public vtDynGeom, public vtHeightFieldGrid3d
{
public:
	vtDynTerrainGeom();

	virtual DTErr Init(const vtElevationGrid *pGrid, float fZScale) = 0;
	virtual void Init2();
	DTErr BasicInit(const vtElevationGrid *pGrid);
	void SetOptions(bool bUseTriStrips, int iBlockArrayDim, int iTextureSize);
	virtual void SetVerticalExag(float fExag) {}

	void SetPixelError(float fPixelError);
	float GetPixelError();
	virtual void SetPolygonCount(int iPolygonCount);
	int GetPolygonCount();

	int GetNumDrawnTriangles();

	void SetDetailMaterial(vtMaterial *pApp, float fTiling, float fDistance);
	void EnableDetail(bool bOn);
	bool GetDetail() { return m_bDetailTexture; }
	void SetupTexGen(float fTiling);
	void SetupBlockTexGen(int a, int b);
	void DisableTexGen();

	// overrides for vtDynGeom
	void DoCalcBoundBox(FBox3 &box);
	void DoCull(const vtCamera *pCam);

	// overrides for HeightField
	bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue = false) const;

	// overrides for HeightField3d
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, bool bIncludeCulture = false,
		FPoint3 *vNormal = NULL) const;

	// overridables
	virtual void DoCulling(const vtCamera *pCam) = 0;

	// control
	void SetCull(bool bOnOff);
	void CullOnce();

	void PreRender() const;
	void PostRender() const;

	int		m_iTPatchDim;
	int		m_iTPatchSize;		// size of each texture patch in texels

protected:
	// tables for quick conversion from x,y index to output X,Z coordinates
	float	*m_fXLookup, *m_fZLookup;

	// these determine the global level of detail
	// (implementation classes can choose which to obey)
	float	m_fPixelError;
	int		m_iPolygonTarget;

	// statistics
	int m_iTotalTriangles;
	int m_iDrawnTriangles;

	// flags
	bool m_bUseTriStrips;
	bool m_bCulleveryframe;
	bool m_bCullonce;

	// detail texture
	float m_fDetailTiling;
	float m_fDetailDistance;
	bool m_bDetailTexture;
	vtMaterial *m_pDetailMat;

protected:
	~vtDynTerrainGeom();
};

#include "mini.h"
#include "miniload.hpp"
#include "minicache.hpp"

typedef unsigned char *ucharptr;

class vtTiledGeom : public vtDynGeom, public vtHeightField3d
{
public:
	vtTiledGeom();

	bool ReadTileList(const char *fname);
	void SetupMiniLoad();
	void SetupMiniTile();

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
	ucharptr *hfields, *textures;
	float exaggeration;
	float exaggertrees;
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
};

/*@}*/	// Group dynterr

#endif	// DYNTERRAINH

