//
// SMTerrain class : Dynamically rendering terrain
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef SMTERRAINH
#define SMTERRAINH

#include <limits.h>
#include "DynTerrain.h"

#define INTEGER_HEIGHT		1
#define STORE_FRUSTUM		0
#define STORE_DISTANCE		1
#define FAST_ALLOC			0	// use quick (macro, no assert) triangle stack

#if INTEGER_HEIGHT
typedef short HeightType;
typedef unsigned char VarianceType;
typedef int MathType;		// use this type to perform arithmetic on
							// the heights and variances
#else
typedef float HeightType;
typedef float VarianceType;
typedef float MathType;
#endif

#if STORE_FRUSTUM
#define SetFrust(t, i) (t)->m_frust = i
#else
#define SetFrust(t, i)
#endif

#define FRUST_OUT		0
#define FRUST_PARTIN	1
#define FRUST_ALLIN		2

class BinTri
{
public:
	BinTri *LeftChild;
	BinTri *RightChild;
	BinTri *LeftNeighbor;
	BinTri *RightNeighbor;
	BinTri *BottomNeighbor;
	// to add: some goop for geomorphs
#if STORE_FRUSTUM
	int m_frust;
#endif
#if STORE_DISTANCE
	float m_distance;
#endif
};

class Block
{
public:
	// the two root triangles of the block
	BinTri *root[2];
	int num[2];
	int v0[2];
	int v1[2];
	int va[2];
};
typedef Block *BlockPtr;

/*
Seumas says:

  "I use bytes for my variance info, since my vertical height map
  resolution is 25cm. Since the variance tree can get pretty big,
  I'd suggest using bytes anyway, perhaps with a non-linear scaling
  if you need larger dynamic range."

  "I believe my implicit variance tree is max-4 levels deep, so imagine
  the finest tessellation possible, then step back 4 levels from there."
*/

class SMTerrain : public vtDynTerrainGeom
{
public:
	SMTerrain();
	~SMTerrain();

	// initialization
	bool Init(vtLocalGrid *pGrid, float fZScale,
				float fOceanDepth, int &iError);
	void Init2();
	void AllocatePool();
	static int MemoryRequired(int iDimension);

	// overrides
	void DoRender();
	void DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov);
	void GetLocation(int iX, int iZ, FPoint3 &p);

protected:
	// initialization
	void ComputeVariances();
	MathType ComputeTriangleVariance(int num, int v0, int v1, int va, int level);
	void SetupBlocks();

	// per-frame
	void AdjustQualityConstant();
	void SplitToBlocks(int num, BinTri *tri, int v0, int v1, int va);
	void SplitIfNeeded(int num, BinTri *tri, int v0, int v1, int va, bool bEntirelyInFrustum, int level);
	void Split(BinTri *tri);
	void Split2(BinTri *tri);
#if !FAST_ALLOC
	inline BinTri *AllocateBinTri();
#endif

	// rendering
	void RenderSurface();
	void RenderBlock(BlockPtr block, bool bFans);
	bool BlockIsVisible(BlockPtr block);
	void LoadSingleMaterial();
	void LoadBlockMaterial(int a, int b);
	void render_triangle_as_fan(BinTri *pTri, int v0, int v1, int va,
		bool even, bool right);
	void render_triangle_single(BinTri *pTri, int v0, int v1, int va);
	void flush_buffer(int type);
	inline float DistanceToTriangle(int num);

private:
	HeightType *m_pData;		// the elevation height array
	VarianceType *m_pVariance;	// the variance implicit binary tree
	float *m_HypoLength;		// table of hypotenuse lengths

	// indices of the corners of the height array
	int m_sw, m_nw, m_ne, m_se;

	int m_iDim;				// dimension: same as width
	int m_n;				// the power-of-2 which defines the grid
	int m_iLevels;			// triangle tree's number of levels deep
	int m_iNodes;			// triangle tree's number of nodes
	int m_iUsedNodes;		// number of nodes needed for variance tree
	int m_iSplitCutoff;
	FPoint3 m_Eyepos;		// world position of the viewpoint
	float m_fZScale;		// converts from values in height field to world scale

	float m_fQualityConstant;

	BlockPtr *m_pBlockArray;
	int m_iBlockN;
	int m_iBlockLevel;
	int m_iBlockCutoff;
	int m_iBlockPoolEnd;
	int m_iBlockArrayDim;

	// The triangle pool use to hold rendered triangles
	int		m_iTriPoolSize;
	BinTri *m_TriPool;
	int		m_NextBinTriPool;

	// the two root triangles of the binary triangle trees
	BinTri *m_pNWTri;
	BinTri *m_pSETri;

	bool hack_detail_pass;
};

#endif
