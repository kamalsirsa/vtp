//
// TerrainSurface : class of geometry for regular rectangular grids
//
// Supports simple terrain following
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINSURFACEH
#define TERRAINSURFACEH

#include "LocalGrid.h"
#include "HeightField.h"
#include "TerrainPatch.h"

class vtTextureCoverage
{
public:
	char m_szFilename[80];	// file of source texture
	float m_xmin, m_xmax;	// area of coverage
	float m_zmin, m_zmax;
	int m_appidx;
	int m_usecount;

	vtTextureCoverage *m_pNext;
};


class vtTerrainGeom : public vtGeom, public vtHeightFieldGrid
{
public:
	vtTerrainGeom();
	~vtTerrainGeom();

	bool CreateFromLocalGrid(vtLocalGrid *pGrid, int VtxType,
		 int iEveryX = 1, int iEveryZ = 1,
		 int largest_block_size = 16, int texture_patches = 2,
		 bool bSuppressLand = false, float fOceanDepth = 0.0f,
		 bool bLighting = true);

	// overrides for vtHeightField
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude, FPoint3 *vNormal);
	void GetEarthExtents(DRECT &ext);
	void GetTerrainExtents(FRECT &ext);

	TerrainPatch *GetPatch(int x, int z);
	void SetPatch(int x, int z, TerrainPatch *pPatch);

	FPoint3 *ComputeNormals(FPoint3 *pLocations);
	bool DrapeTextureUV();
	bool DrapeTextureUVTiled(vtTextureCoverage *cover);

protected:
	void AllocatePatches();

	int		m_iXPatches;
	int		m_iZPatches;
	int		m_iChopx[200];
	int		m_iChopz[200];
	float	m_fChopx[200];
	float	m_fChopz[200];

	TerrainPatch **m_pPatches;
};

#endif // TERRAINSURFACEH
