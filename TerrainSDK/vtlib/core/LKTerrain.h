//
// LKTerrain class :
// Implementation of the Lindstrom-Koller terrain LOD algorithm
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LKTERRAINH
#define LKTERRAINH

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "DynTerrain.h"

#define DO_BOUNDS_CHECK		0	// debug: bounds checking
#define BUCKETS				4
#define DENSE_ARRAY			1

#define VERTEX_ON		0x01
#define VERTEX_EVALED	0x02

typedef struct {
//	unsigned int eval_pass;		// frame number when last evaluated
	float delta2;				// delta squared
							// maximum delta of this and child vertices

	short xmin, xmax;		// INDICES of extents
	short zmin, zmax;		// INDICES of extents
	float ymin, ymax;		// ACTUAL z extents
	float y;
	short level;

	int parent0, parent1;
} Vertex;


#define clamp(a,b,c) (b < a ? a : b > c ? c : b)

#if DENSE_ARRAY
#define TURN_ON(v) pFlagSet[(v)>>4] |= (1 << ((v) & 0xf))
#define TURN_OFF(v) pFlagSet[(v)>>4] &= ~(1 << ((v) & 0xf))
#define IS_ON(v) (pFlagSet[(v)>>4] & (1 << ((v) & 0xf)))
#define SET_EVALUATED(v) pFlagEval[(v)>>4] |= (1 << ((v) & 0xf))
#define UNSET_EVALUATED(v) pFlagEval[(v)>>4] &= ~(1 << ((v) & 0xf))
#define IS_EVALUATED(v) (pFlagEval[(v)>>4] & (1 << ((v) & 0xf)))
#else
#define TURN_ON(v) pFlags[v] |= VERTEX_ON
#define TURN_OFF(v) pFlags[v] &= ~VERTEX_ON
#define IS_ON(v) (pFlags[v] & VERTEX_ON)
#define SET_EVALUATED(v) pFlags[v] |= VERTEX_EVALED
#define UNSET_EVALUATED(v) pFlags[v] &= ~VERTEX_EVALED
#define IS_EVALUATED(v) (pFlags[v] & VERTEX_EVALED)
#endif

class Bucket : public Array<int>
{
public:
	// additional info
	int base_i, base_j;
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*!
	The LKTerrain class implements the Siggraph 1996 paper "Real-Time,
	Continuous LOD Rendering of Height Fields" by Peter Lindstrom and David
	Koller.  It was implemented with great help from Koller himself.
	\par
	This algorithm requires that the input heightfield is a square regular
	grid of dimension 2^n+1, eg. 513, 1025, or 2049.  Storage requirements
	are rather large, but the algorithm can very quickly determine exactly
	which vertices are to be visible to maintain a given error bound.  The
	error metric is sophisticated and take both distance and angle into
	consideration.
	<h4>Differences</h4>
	This implementation differs from the published Siggraph paper in the
	following ways:
	 - A single massive array is used, instead of the "block" approach
	   described in the paper.  This is because the paper does not describe
	   how to maintain continuity on block boundaries.
	 - The culling algorithm proceeds top-down, rather than bottom-up.
 */
class LKTerrain : public vtDynTerrainGeom
{
public:
	LKTerrain();
	~LKTerrain();

	/// initialization
	bool Init(vtLocalGrid *pGrid, float fZScale, float fOceanDepth, int &iError);
	static int MemoryRequired(int iDimension);

	// overrides
	void DoRender();
	void DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov);
	void GetLocation(int iX, int iZ, FPoint3 &p);

protected:
	// setup and initialization
	void compute_delta(int left_index, int mid_index, int right_index);
	void compute_bbox(int i, int j);
	void ComputeLevelsAndDeltas();
	void DumpOffsets();

	// visibility determination and vertex tree processing
	void evaluate_vertex(int index);
	void turn_on_parents(int index);
	int child_index(int mid_index, int child_num);

	// Tristrip rendering
	void render_vertex(int index_l, int index_mid, int index_r, int level);
	void terrain_render_tristrips();
	void terrain_render_tristrips2();

	// Non-tristrip rendering
	void render_triangle(int index_l, int index_mid, int index_r, int level);
	void terrain_render_simple();
	void terrain_render_simple2();
	void flush_buffer(bool bStrips);

	void print_culling_results();

private:
	// for Buckets
	void empty_buckets();
	void terrain_render_buckets();
	void determine_bucket(int index_l, int index_r, int index_mid,
		int &a, int &b);

#if DO_BOUNDS_CHECK
	int offset(int x, int y) {
		if (x < 0 || x >= m_iDim || y < 0 || y >= m_iDim)
		{
			TRACE("Yow!\n");
		}
		int result = (((y) << (m_iLevels)) + (y) + (x));
		if (result < 0 || result >= m_iTotalSize)
		{
			TRACE("Yow!\n");
		}
		return result;
	}
#endif

	// the single, square vertex array
	Vertex	*pVertex;

	// indices of the corners of the height array
	int		m_sw, m_nw, m_ne, m_se, m_center;

	int		m_iLevels;
	int		m_iDim;
	int		m_iTotalSize;

	// tables for quick conversion from x,y index to output X,Z coordinates
//	float	*fXLookup, *fYLookup;

#if DENSE_ARRAY
	unsigned short	*pFlagSet;
	unsigned short	*pFlagEval;
	size_t	m_statsize;		// size of status buffer, in shorts
#else
	char	*pFlags;
#endif

	// parameters affecting the error metric
	float	m_kappa2;
	FPoint3 m_eyepos;

	// for Buckets
	bool	m_bUseBuckets;
	Bucket	m_Bucket[BUCKETS][BUCKETS];		// index arrays
};

#endif

