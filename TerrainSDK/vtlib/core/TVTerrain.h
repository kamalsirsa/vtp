//
// TVTerrain class : Dynamically rendering terrain
//
// Original authors: Will Evans and Gregg Townsend of the University of Arizona
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TVTERRAINH
#define TVTERRAINH

/** \addtogroup dynterr */
/*@{*/

#include <limits.h>
#include "DynTerrain.h"

/*
 * =========== Common Constants =============
 */
#ifndef PI
#define PI 3.1415926535897932385
#endif

/*
 * =========== Common Macros =============
 */
#ifndef ABS
#define ABS(x) (((x)<0)?(-(x)):(x))
#endif

#ifndef ROUND
#define ROUND(a) ((int)((a)+0.5))
#endif

/*
 * ============ Common Structures ============
 */

typedef int Coord2d[2];

/*
 * ============== Heirarchy ===============
 */
#define MAXERR USHRT_MAX	/* maximum scaled triangle error */

/* triangle info flag bits */
#define SMALLER1	  1	/* Is neighbor 1 smaller? */
#define SMALLER2	  2	/* Is neighbor 2 smaller? */
#define BIGGER3		  4	/* Is neighbor 3 larger? */
#define SURFACE		  8	/* on surface? */
#define ALLINFOV	 16	/* completely in field of view? */
#define PARTINFOV	 32	/* partially in field of view? */
#define ALLINROD	 64	/* completely in rectangle of definition? */
#define PARTINROD	128	/* partially in rectangle of definition? */

/* info field access macros */
#define BORDER(t) (m_info[(t)->s] & PARTINROD)
#define INROD(t) (m_info[(t)->s] & ALLINROD)
#define PUTONSURFACE(t) \
	m_info[(t)->s] = (m_info[(t)->s]&(ALLINROD|PARTINROD))|SURFACE
#define ONSURFACE(t) (m_info[(t)->s] & SURFACE)
#define INFOV(t) (m_info[(t)->s] & (ALLINFOV | PARTINFOV))
#define SMALLN1(t) (m_info[(t)->s] & SMALLER1)
#define SMALLN2(t) (m_info[(t)->s] & SMALLER2)
#define BIGN3(t) (m_info[(t)->s] & BIGGER3)

/* these macros also take TriIndex arguments */
#define ISLEAF(t) ((t)->depth >= m_depth)
#define IAMLEFTCHILD(t) ((((t)->depth&1)&&(((t)->orient^(t)->x)&1)) || \
			 ((((t)->depth&1)==0)&&(((t)->orient^(t)->x^(t)->y)&1)))
#define TRIEQUATE(a,b)  (a)->x=(b)->x; \
			(a)->y=(b)->y; \
			(a)->depth=(b)->depth; \
			(a)->orient=(b)->orient; \
			(a)->s=(b)->s

typedef struct {	/* triangle index */
  int x,y;			/* coords of square containing tri at depth */
  int depth;		/* depth of triangle */
  int orient;		/* orientation */
  int s;			/* physical address */
} TriIndex;

/* initial values */
#define IERRTHRESH		0.001
#define IERRTHRESHINC	0.00005
#define IDISTTHRESH		700.0
#define IDISTTHRESHINC	50.0
#define IVEXAGGER		1.0
#define IVEXAGGERINC	0.1


/*!
	\par
	The TVTerrain class implements the
	<a href="http://www.cs.arizona.edu/topovista/">TopoVista</a>
	algorithm for regular-grid terrain LOD.
	\par
	This algorithm requires that the input heightfield is a square regular
	grid, of any size.  Storage requirements are around 14 bytes/vertex.
	The data is represented in memory by a binary tree of faces.
	\par
	The VTP implementation consists of an adaptation of the author's own
	source code, made to work in the vtDynTerrainGeom framework.
	\par
	The algorithm is promising, but needs a lot of work:
	 - Doesn't currently address lighting or texturing
	 - Doesn't utilize triangle strips
	 - Uses a very simple error metric based only on ground (2D) distance
	\par
	So far i have extended the algorithm in the following ways:
	 - Added full 6-plane view-volume culling (instead of the previously
	   simple 2D culling).   This required implementing some
	   <a href="http://vterrain.org/LOD/culling.html">3d triangle culling</a>
	   which was mysteriously absent from the computer graphics literature.
 */
class TVTerrain : public vtDynTerrainGeom
{
public:
	TVTerrain();

	/// initialization
	DTErr Init(const vtElevationGrid *pGrid, float fZScale);
	static int MemoryRequired(int iDimension);

	// overrides
	void DoRender();
	void DoCulling(const vtCamera *pCam);
	float GetElevation(int iX, int iZ, bool bTrue = false) const;
	void GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue = false) const;
	virtual float GetVerticalExag() const { return m_fZScale; }

protected:
	void mkscale(const vtElevationGrid *pGrid);
	int calcErr(const vtElevationGrid *pGrid, Coord2d p1, Coord2d p2, Coord2d p3);
	int inFOV(Coord2d p1, Coord2d p2, Coord2d p3);
	int triInFOV(TriIndex *t);
	int inROD(Coord2d p1, Coord2d p2, Coord2d p3);
	void makeDFS(const vtElevationGrid *pGrid, TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3);
	int init_ntriabove(int depth);
	void getVerts(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3);

	void parent(TriIndex *t, TriIndex *p);
	void bro1(TriIndex *t, TriIndex *b1);
	void bro2(TriIndex *t, TriIndex *b2);
	void bro3(TriIndex *t, TriIndex *b3);
	void nbr1(TriIndex *t, TriIndex *n1);
	void nbr2(TriIndex *t, TriIndex *n2);
	void nbr3(TriIndex *t, TriIndex *n3);
	void kidsOnSurf(TriIndex *t);
	void split(TriIndex *t);
	void rodSplit(TriIndex *t);
	void eyeSplit(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3);
	void baseSurface();

	int tooCoarse(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3);
	void errSplit(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3);
	void errSurface();
	void buildSurface(FPoint3 &eyepos_ogl);
	void emitSurface();
	void emitDFS(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3);
	void emitTri(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3);
	void getNormal(int,int,double,int,int,double,int,int,double,double*,double*,double*);

protected:
	// cleanup
	virtual ~TVTerrain();

private:
	float	m_fXScale, m_fYScale, m_fZScale;

/* heirarchy data */
	int m_depth;			/* maximum depth */
	int m_size;				/* number of triangles in hierarchy */
	int m_numSurfTri;		/* number of triangles on surface */
	unsigned short *m_err;	/* error of triangles */
	uchar *m_info;	/* surface,neighbor,infov of triangles */
	int m_iLevels;

	double x_per_y, y_per_x, errPerEm;

/* terrain data */
	float *m_pVertex;

	double xmeters, ymeters;	/* dimensions of data in meters */
	double xscale, yscale;		/* scales grid distance to meters */
	double m2grid;				/* scales vertical meters to grid units */
	double err2grid;			/* scales 0..MAXERR to grid units */
	Coord2d se, sw, ne, nw;
	TriIndex rootL, rootR;		/* initial Triangles */

/* Triangle err/distance from eye must be < errThresh */
	double errThresh;

/* Triangles < k*distThresh meters from eye must have size < 2^k */
	double distThresh;

/* error parameters, derived from errThresh (above) */
	double etGrid, dtGrid;

/* current viewpoint */
	double eyeEmx, eyeEmy, eyeEmz;	/* eye location */
	int numDisplayTri;		/* number of displayed triangles */

// quick conversion from x,y index to output X,Y coordinates
//	float	*fXLookup, *fYLookup;

	Coord2d eyeP;
};

/*@}*/	// Group dynterr

#endif	// TVTERRAINH

