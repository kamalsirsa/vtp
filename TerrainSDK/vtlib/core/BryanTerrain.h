//
// BryanTerrain class : Dynamically rendering terrain
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BRYANTERRAINH
#define BRYANTERRAINH

#include "DynTerrain.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Machine-specific typedefs
//   NOTE:  On UNIX, INT{8,16,32,64} are defined in X11/Xmd.h, and
//		  not necessarily signed
typedef unsigned long UINT_32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

// Pool Size
#define POOL_SIZE (4000+2)
#define EDGE_NODE (POOL_SIZE-1)

// Child Indexes...
#define CHILD_NW 0
#define CHILD_NE 1
#define CHILD_SW 2
#define CHILD_SE 3

// --------------------------------------------------------------
// Quad Tree structure with index linking
// --------------------------------------------------------------
struct QuadNodeStruc		// 16 Byte record for lookup efficiency
{
	UINT8 nBlend;			// Blend value [0..255] => [0.0 .. 1.0]
	UINT8 _Reserve1;		// Spacer
	UINT16 nChildren;		// Index to first child (all children are in a block)
	UINT16 nNorth, nSouth;	// Pointer to neighbors of same level
	UINT16 nEast, nWest;
	UINT16 nTexture;		// TextureID
	UINT16 _Reserve2;		// Spacer
};

// --------------------------------------------------------------
// Custom Terrain Class
// --------------------------------------------------------------
class BryanTerrain : public vtDynTerrainGeom
{
public:
	BryanTerrain();

	// initialization
	DTErr Init(const vtElevationGrid *pGrid, float fZScale);

	// overrides
	void DoRender();
	void DoCulling(const vtCamera *pCam);
	float GetElevation(int iX, int iZ, bool bTrue = false) const;
	void GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue = false) const;

protected:
	// rendering
	void RenderSurface();
	void RenderPass();
	void LoadSingleMaterial();

	// Quad Tree Engine
	void Split( UINT16 nNode );
	void SplitSelected( );
	void FreeChildren( UINT16 nNode );
	UINT8 SplitMetric( UINT16 nNode, UINT_32 NW_X, UINT_32 NW_Y, UINT_32 Quad_Width );

	void RecursTessellate( UINT16 nNode, UINT_32 NW_X, UINT_32 NW_Y, UINT_32 nQuadWidth, UINT8 nSortOrder );
	void RecursRender( UINT16 nNode, UINT_32 NW_X, UINT_32 NW_Y, UINT_32 Quad_Width );

	void MakeVertex( UINT_32 x, UINT_32 y, float height, UINT_32 NW_X, UINT_32 NW_Y );
	void MakeBlendedVertex( UINT_32 x, UINT_32 y, float heightOne, float heightTwo, float fBlend, UINT_32 NW_X, UINT_32 NW_Y );

	float HeightField( UINT_32 y, UINT_32 x );

protected:
	virtual ~BryanTerrain();

private:
	// Basic Engine Variables
	float *m_pData;			// the elevation height array
	float m_fZScale;


	// Quad Tree Engine Variables
	QuadNodeStruc	*m_aQuadPool;				// All Quads
	UINT16			m_nFirstFree;				// Index of first free block of nodes.
	float			m_fFrameVariance;
	FPoint3			m_eyepos_ogl;
	UINT_32			m_nPoints;
};

#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif
