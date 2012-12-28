//
// TVTerrain.cpp
//
// An adaptation of the TopoVista 2.0 algorithm
//
// Original authors: Will Evans and Gregg Townsend of the University of Arizona
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#define DO_VIEW_CULLING		1

#include "TVTerrain.h"

// this is how to produce an offset into the vertex array
#define offset(x, y)	(((y) << m_iLevels) + (y) + (x))

#define SOUTH	0	/* important that these 4 come before NE,SW,NW,SE */
#define WEST	1
#define NORTH	2
#define EAST	3
#define NW		4	/* important that low bits alternate SW,NE and NW,SE */
#define SE		5
#define NE		6
#define SW		7

/* SOUTH,WEST,NORTH,EAST,NW,SE,NE,SW */
static int lorient[8] = {SE,SW,NW,NE,WEST,EAST,NORTH,SOUTH};
static int rorient[8] = {SW,NW,NE,SE,NORTH,SOUTH,EAST,WEST};
static int b1orient[8] = {EAST,SOUTH,WEST,NORTH,SW,NE,NW,SE};
static int b2orient[8] = {WEST,NORTH,EAST,SOUTH,NE,SW,SE,NW};
static int b3orient[8] = {NORTH,EAST,SOUTH,WEST,SE,NW,SW,NE};

/* NW,SE,NE,SW (parity of x determines row) */
static int porient1[8] = {WEST,SOUTH,NORTH,WEST,
NORTH,EAST,EAST,SOUTH};
/* SOUTH,WEST,NORTH,EAST (parity of x+y determines row) */
static int porient0[8] = {SE,NW,NW,SE,
SW,SW,NE,NE};

/* SOUTH,WEST,NORTH,EAST,NW,SE,NE,SW */
static int lxinc[8] = {0,0,1,1,0,0,0,0};
static int lyinc[8] = {0,1,1,0,0,0,0,0};
static int rxinc[8] = {1,0,0,1,0,0,0,0};
static int ryinc[8] = {0,0,1,1,0,0,0,0};
static int b1xinc[8] = {0,0,0,0,0,0,+1,-1};
static int b1yinc[8] = {0,0,0,0,+1,-1,0,0};
static int b2xinc[8] = {0,0,0,0,-1,+1,0,0};
static int b2yinc[8] = {0,0,0,0,0,0,+1,-1};
static int b3xinc[8] = {0,-1,0,+1,0,0,0,0};
static int b3yinc[8] = {-1,0,+1,0,0,0,0,0};

static int xmax[64];
static int ymax[64];
static int ntriabove[64];

#define S1(t) 2*(t->y*xmax[t->depth]+t->y+t->x)+(t->orient&1)+ntriabove[t->depth]
#define S0(t) 4*(t->y*xmax[t->depth]+t->y+t->x)+t->orient+ntriabove[t->depth]


//
//
//
TVTerrain::TVTerrain() : vtDynTerrainGeom()
{
	errThresh = IERRTHRESH;
	distThresh = IDISTTHRESH;
}

TVTerrain::~TVTerrain()
{
	free(m_pVertex);
}

void leftChild(TriIndex *t, TriIndex *lt)
{
	lt->depth = t->depth+1;
	lt->orient = lorient[t->orient];
	if (t->depth&1) {
		lt->x = t->x;
		lt->y = t->y;
		lt->s = S0(lt);
	} else {
		lt->x = t->x<<1 | lxinc[t->orient];
		lt->y = t->y<<1 | lyinc[t->orient];
		if (lt->x > xmax[lt->depth] || lt->y > ymax[lt->depth])
			lt->s = 0;
		else
			lt->s = S1(lt);
	}
}

void rightChild(TriIndex *t, TriIndex *rt)
{
	rt->depth = t->depth+1;
	rt->orient = rorient[t->orient];
	if (t->depth&1) {
		rt->x = t->x;
		rt->y = t->y;
		rt->s = S0(rt);
	} else {
		rt->x = t->x<<1 | rxinc[t->orient];
		rt->y = t->y<<1 | ryinc[t->orient];
		if (rt->x > xmax[rt->depth] || rt->y > ymax[rt->depth])
			rt->s = 0;
		else
			rt->s = S1(rt);
	}
}

static int ceilingLog2(int x)
{
	int m = 1;
	int i = 0;

	while (m < x && m < INT_MAX) {
		m <<= 1;
		++i;
	}
	return i;
}

static int next2Power(int x)
{
	int m = 1;
	while (m < x && m < INT_MAX)
		m <<= 1;
	return m;
}

int TVTerrain::calcErr(const vtElevationGrid *pGrid, Coord2d p1, Coord2d p2, Coord2d p3)
{
	int x1, x2, x3, y1, y2, y3;
	double z1, z2, z3;
	double ystart, yend, xstart, xend;
	double deltaxstart, deltaxend, deltaystart, deltayend;
	int xmini, xmaxi, ymini, ymaxi;
	double z, zstart, zend, deltazstart, deltazend, deltaz;
	double max_eps, eps;
	int x, y;
	int xstarti, xendi, ystarti, yendi;

	x1 = p1[0];
	y1 = p1[1];
	z1 = pGrid->GetFValue(x1, y1);
	x2 = p2[0];
	y2 = p2[1];
	z2 = pGrid->GetFValue(x2, y2);
	x3 = p3[0];
	y3 = p3[1];
	z3 = pGrid->GetFValue(x3, y3);

	assert(x1 < m_iSize.x);
	assert(x2 < m_iSize.x);
	assert(x3 < m_iSize.x);
	assert(y1 < m_iSize.y);
	assert(y2 < m_iSize.y);
	assert(y3 < m_iSize.y);

	max_eps = 0.0;

	if (y1 == y2)
	{
		if (x1 > x2)
		{
			xmini = x2;
			xmaxi = x1;
			ymini = y3;
			ymaxi = y1;

			xstart = x3;
			xend = xstart;
			deltaxstart = -x_per_y;
			deltaxend = x_per_y;
			zstart = z3;
			zend = z3;
			deltazstart = (z2 - zstart) / (double)(y1 - y3);
			deltazend = (z1 - zend) / (double)(y1 - y3);
		}
		else
		{
			xmini = x1;
			xmaxi = x2;
			ymini = y1;
			ymaxi = y3;

			xstart = x1;
			xend = x2;
			deltaxstart = x_per_y;
			deltaxend = -x_per_y;
			zstart = z1;
			zend = z2;
			deltazstart = (z3 - zstart) / (double)(y3 - y1);
			deltazend = (z3 - zend) / (double)(y3 - y1);
		}
		for (y = ymini; y <= ymaxi; ++y)
		{
			xstarti = MAX(xmini, ROUND(xstart));
			xendi = MIN(xmaxi, ROUND(xend));
			z = zstart;
			if (xstarti != xendi) {
				deltaz = (zend - zstart) / (xendi - xstarti);
			} else {
				deltaz = 0.0;
			}
			for (x = xstarti; x <= xendi; ++x) {
				eps = ABS(pGrid->GetFValue(x, y) - z);
				if (eps > max_eps) {
					max_eps = eps;
				}
				z = z + deltaz;
			}
			zstart = zstart + deltazstart;
			zend = zend + deltazend;
			xstart = xstart + deltaxstart;
			xend = xend + deltaxend;
		}
	}
	else
	{
		if (x1 == x2)
		{
			if (y1 > y2)
			{
				xmini = x1;
				xmaxi = x3;
				ymini = y2;
				ymaxi = y1;

				ystart = y2;
				yend = y1;
				deltaystart = y_per_x;
				deltayend = -y_per_x;
				zstart = z2;
				zend = z1;
				deltazstart = (z3 - zstart) / (double)(x3 - x1);
				deltazend = (z3 - zend) / (double)(x3 - x1);
			}
			else
			{
				xmini = x3;
				xmaxi = x1;
				ymini = y1;
				ymaxi = y2;

				ystart = y3;
				yend = ystart;
				deltaystart = -y_per_x;
				deltayend = y_per_x;
				zstart = z3;
				zend = z3;
				deltazstart = (z1 - zstart) / (double)(x1 - x3);
				deltazend = (z2 - zend) / (double)(x1 - x3);
			}
		}
		else
		{
			if (x1 < x2)
			{
				if (y1 > y2)
				{
					xmini = x1;
					xmaxi = x2;
					ymini = y2;
					ymaxi = y3;

					ystart = y1;
					yend = ystart;
					deltaystart = -y_per_x;
					deltayend = 0.0;
					zstart = z1;
					zend = z1;
					deltazstart = (z2 - zstart) / (double)(x3 - x1);
					deltazend = (z3 - zend) / (double)(x3 - x1);
				}
				else
				{
					xmini = x1;
					xmaxi = x2;
					ymini = y1;
					ymaxi = y3;

					ystart = y1;
					yend = y3;
					deltaystart = y_per_x;
					deltayend = 0.0;
					zstart = z1;
					zend = z3;
					deltazstart = (z2 - zstart) / (double)(x2 - x3);
					deltazend = (z2 - zend) / (double)(x2 - x3);
				}
			}
			else
			{
				if (y1 > y2)
				{
					xmini = x2;
					xmaxi = x3;
					ymini = y3;
					ymaxi = y1;

					ystart = y2;
					yend = ystart;
					deltaystart = 0.0;
					deltayend = y_per_x;
					zstart = z2;
					zend = z2;
					deltazstart = (z3 - zstart) / (double)(x3 - x2);
					deltazend = (z1 - zend) / (double)(x3 - x2);
				}
				else
				{
					xmini = x3;
					xmaxi = x1;
					ymini = y3;
					ymaxi = y2;

					ystart = y3;
					yend = y2;
					deltaystart = 0.0;
					deltayend = -y_per_x;
					zstart = z3;
					zend = z2;
					deltazstart = (z1 - zstart) / (double)(x1 - x3);
					deltazend = (z1 - zend) / (double)(x1 - x3);
				}
			}
		}
		for (x = xmini; x <= xmaxi; ++x)
		{
			ystarti = MAX(ymini, ROUND(ystart));
			yendi = MIN(ymaxi, ROUND(yend));
			z = zstart;
			if (ystarti != yendi) {
				deltaz = (zend - zstart) / (yendi - ystarti);
			} else {
				deltaz = 0.0;
			}
			for (y = ystarti; y <= yendi; ++y)
			{
				eps = ABS(pGrid->GetFValue(x, y) - z);
				if (eps > max_eps) {
					max_eps = eps;
				}
				z = z + deltaz;
			}
			zstart = zstart + deltazstart;
			zend = zend + deltazend;
			ystart = ystart + deltaystart;
			yend = yend + deltayend;
		}
	}
	return ROUND(max_eps * errPerEm);
}

#define MAKE_XYZ(point) m_fXLookup[point[0]], m_pVertex[offset(point[0], point[1])] * m_fZScale, m_fZLookup[point[1]]
#define MAKE_XYZ2(x, y) m_fXLookup[x], m_pVertex[offset(x, y)] * m_fZScale, m_fZLookup[y]
#define MAKE_XYZ2_TRUE(x, y) m_fXLookup[x], m_pVertex[offset(x, y)], m_fZLookup[y]

#define emitVertex(p) glVertex3f(p[0], m_pVertex[offset(p[0], p[1])], p[1])


/* A triangle is in FOV iff one of its 3 vertices is in FOV
* or if the line of sight intersects one of its 3 segments.
*/
int TVTerrain::inFOV(Coord2d p1, Coord2d p2, Coord2d p3)
{
#if DO_VIEW_CULLING
	// find 3 points of 3d triangle
	FPoint3 v1(MAKE_XYZ(p1));
	FPoint3 v2(MAKE_XYZ(p2));
	FPoint3 v3(MAKE_XYZ(p3));

	// test the triangle against the view volume
	int ret = IsVisible(v1, v2, v3);
	if (ret == VT_AllVisible) return ALLINFOV;
	if (ret == VT_Visible) return PARTINFOV;
	return 0;
#else
	// no view culling (for simplified testing)
	return ALLINFOV;
#endif
}


/*  triInFOV(t) -- is triangle t in field of view?  */

int TVTerrain::triInFOV(TriIndex *t)
{
	Coord2d p1, p2, p3;

	getVerts(t, p1, p2, p3);
	return inFOV(p1, p2, p3);
}


int TVTerrain::inROD(Coord2d p1, Coord2d p2, Coord2d p3)
{
	int r;
	int txmax = MAX(p1[0], MAX(p2[0], p3[0]));
	int txmin = MIN(p1[0], MIN(p2[0], p3[0]));
	int tymax = MAX(p1[1], MAX(p2[1], p3[1]));
	int tymin = MIN(p1[1], MIN(p2[1], p3[1]));

	if (txmax > m_iSize.x - 1) {
		if (txmin >= m_iSize.x - 1) {
			r = 0;
		} else {
			r = PARTINROD;
		}
	} else if (tymax > m_iSize.y - 1) {
		if (tymin >= m_iSize.y - 1) {
			r = 0;
		} else {
			r = PARTINROD;
		}
	} else {
		r = ALLINROD;
	}

	return r;
}

/*  midpoint(p1, p2, m) -- set m to midpoint of segment between p1 and p2  */

void midpoint(const Coord2d p1, const Coord2d p2, Coord2d m)
{
	m[0] = (p1[0] + p2[0]) / 2;
	m[1] = (p1[1] + p2[1]) / 2;
	/*  m[2] = pGrid->GetFValue(m[0], m[1]); */
}

void TVTerrain::makeDFS(const vtElevationGrid *pGrid, TriIndex *t,
						Coord2d p1, Coord2d p2, Coord2d p3)
{
	TriIndex lt,rt;
	Coord2d mid;
	int r;

	r = inROD(p1, p2, p3);
	if (t->s) {
		m_info[t->s] = r;
		if (ISLEAF(t)) {
			m_err[t->s] = 0;
		} else {
			if (r == 0) {
				m_err[t->s] = 0;
			} else if (r == PARTINROD) {
				m_err[t->s] = MAXERR;
			} else {
				m_err[t->s] = calcErr(pGrid, p1, p2, p3);
			}
			midpoint(p1, p2, mid);
			leftChild(t,&lt);
			rightChild(t,&rt);
			makeDFS(pGrid, &lt, p3, p1, mid);
			makeDFS(pGrid, &rt, p2, p3, mid);
		}
	}
}

void TVTerrain::parent(TriIndex *t, TriIndex *p)
{
	p->depth = t->depth-1;
	if (t->depth&1) {
		p->orient = porient1[((t->x&1)<<2)|(t->orient&3)];
		p->x = t->x>>1;
		p->y = t->y>>1;
		p->s = S0(p);
	} else {
		p->orient = porient0[(((t->x^t->y)&1)<<2)|(t->orient&3)];
		p->x = t->x;
		p->y = t->y;
		p->s = S1(p);
	}
}

void TVTerrain::bro1(TriIndex *t, TriIndex *b1)
{
	b1->depth = t->depth;
	b1->x = t->x + b1xinc[t->orient];
	b1->y = t->y + b1yinc[t->orient];
	b1->orient = b1orient[t->orient];
	if (b1->x > xmax[t->depth] || b1->y > ymax[t->depth] ||
		b1->x < 0 || b1->y < 0)
	{
		b1->s = 0;			/* set b1 to be OUTOFBOUNDS */
	}
	else {
		if (t->depth&1)
			b1->s = S1(b1);
		else
			b1->s = S0(b1);
	}
}

void TVTerrain::bro2(TriIndex *t, TriIndex *b2)
{
	b2->depth = t->depth;
	b2->x = t->x + b2xinc[t->orient];
	b2->y = t->y + b2yinc[t->orient];
	b2->orient = b2orient[t->orient];
	if (b2->x > xmax[t->depth] || b2->y > ymax[t->depth] ||
		b2->x < 0 || b2->y < 0)
		b2->s = 0;			/* set b2 to be OUTOFBOUNDS */
	else {
		if (t->depth&1)
			b2->s = S1(b2);
		else
			b2->s = S0(b2);
	}
}

void TVTerrain::bro3(TriIndex *t, TriIndex *b3)
{
	b3->x = t->x+b3xinc[t->orient];
	b3->y = t->y+b3yinc[t->orient];
	b3->orient = b3orient[t->orient];
	b3->depth = t->depth;
	if (b3->x > xmax[t->depth] || b3->y > ymax[t->depth] ||
		b3->x < 0 || b3->y < 0)
		b3->s = 0;			/* set b3 to be OUTOFBOUNDS */
	else {
		if (t->depth&1)
			b3->s = S1(b3);
		else
			b3->s = S0(b3);
	}
}


void TVTerrain::nbr1(TriIndex *t, TriIndex *n1)
{
	TriIndex b1;

	bro1(t,&b1);

	if (b1.s && SMALLN1(t)) {
		leftChild(&b1,n1);
	} else {
		TRIEQUATE(n1,&b1);
	}
}

void TVTerrain::nbr2(TriIndex *t, TriIndex *n2)
{
	TriIndex b2;

	bro2(t,&b2);

	if (b2.s && SMALLN2(t)) {
		rightChild(&b2,n2);
	} else {
		TRIEQUATE(n2,&b2);
	}
}

void TVTerrain::nbr3(TriIndex *t, TriIndex *n3)
{
	TriIndex b3;

	bro3(t,&b3);

	if (b3.s && BIGN3(t)) {
		parent(&b3,n3);
	} else {
		TRIEQUATE(n3,&b3);
	}
}


static int hierDepth;

/* initialize ntriabove[] and return size of needed
physical address space */
int TVTerrain::init_ntriabove(int depth)
{
	int d,xm,ym;

	/* initialize special Tri's */
	rootL.x=0; rootL.y=0; rootL.depth=1; rootL.orient=NW; rootL.s=1;
	rootR.x=0; rootR.y=0; rootR.depth=1; rootR.orient=SE; rootR.s=2;

	xm = m_iSize.x-2;	/* count squares (starting at (0,0)) */
	ym = m_iSize.y-2;
	d = hierDepth = depth;
	while (d >= 1) {
		xmax[d] = xm;
		ymax[d] = ym;
		xm >>= 1;
		ym >>= 1;
		d--;
		xmax[d] = xm;
		ymax[d] = ym;
		d--;
	}

	ntriabove[0] = 0;
	ntriabove[1] = 1;
	d = 1;
	while(d <= depth) {

		/* odd depth: NW,SW,NE,SE triangles */
		ntriabove[d+1]=2*(xmax[d]+1)*(ymax[d]+1)+ntriabove[d];
		d++;

		if (d>depth) break;

		/* even depth: N,S,E,W triangles */
		ntriabove[d+1] = 4*(xmax[d]+1)*(ymax[d]+1)+ntriabove[d];
		d++;
	}
	return(ntriabove[depth+1]+1);/* loop went far enough to calc. this */
}


void TVTerrain::getVerts(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3)
{
	Coord2d swt,net;
	int d;

	d = (hierDepth-t->depth+1)>>1;
	swt[0] = t->x << d;
	swt[1] = t->y << d;
	net[0] = (t->x+1) << d;
	net[1] = (t->y+1) << d;

	if ((t->depth&1)==0) {
		midpoint(swt,net,p3);
		switch(t->orient) {
	case NORTH:
		p1[0]=net[0]; p1[1]=net[1];
		p2[0]=swt[0]; p2[1]=net[1];
		break;
	case SOUTH:
		p1[0]=swt[0]; p1[1]=swt[1];
		p2[0]=net[0]; p2[1]=swt[1];
		break;
	case EAST:
		p1[0]=net[0]; p1[1]=swt[1];
		p2[0]=net[0]; p2[1]=net[1];
		break;
	case WEST:
		p1[0]=swt[0]; p1[1]=net[1];
		p2[0]=swt[0]; p2[1]=swt[1];
		break;
	default:
		assert(0);
		break;
		}
	} else {
		switch(t->orient) {
	case NW:
		p1[0]=swt[0]; p1[1]=swt[1];
		p2[0]=net[0]; p2[1]=net[1];
		p3[0]=swt[0]; p3[1]=net[1];
		break;
	case SW:
		p1[0]=net[0]; p1[1]=swt[1];
		p2[0]=swt[0]; p2[1]=net[1];
		p3[0]=swt[0]; p3[1]=swt[1];
		break;
	case SE:
		p1[0]=net[0]; p1[1]=net[1];
		p2[0]=swt[0]; p2[1]=swt[1];
		p3[0]=net[0]; p3[1]=swt[1];
		break;
	case NE:
		p1[0]=swt[0]; p1[1]=net[1];
		p2[0]=net[0]; p2[1]=swt[1];
		p3[0]=net[0]; p3[1]=net[1];
		break;
	default:
		assert(0);
		break;
		}
	}
	/*
	printf("(%d,%d,%d,%d) ((%d,%d),(%d,%d),(%d,%d))\n",t->x,t->y,
	t->depth,t->orient,p1[0],p1[1],p2[0],p2[1],p3[0],p3[1]);
	*/
}


/*  mkscale(em) -- configure scaling  */
/*  sets xmeters, ymeters, xscale, yscale, m2grid */
/*  err2grid  */

void TVTerrain::mkscale(const vtElevationGrid *pGrid)
{
	/* get dimensions in meters */
	DRECT area = pGrid->GetEarthExtents();
	xmeters = area.right - area.left;
	ymeters = area.top - area.bottom;

	/* get meters per elevation model cell */
	xscale = xmeters / (m_iSize.x-1);
	yscale = ymeters / (m_iSize.y-1);

	/* use the average of these to convert meters to EM grid coords */
	m2grid = 1. / ((xscale + yscale) / 2.);

	/* stored error (0...MAXERR) to meters */
	float fMin, fMax;
	pGrid->GetHeightExtents(fMin, fMax);
	err2grid = fMax * m2grid / (double)MAXERR;
}


//
// Initialize the terrain LOD structures
//
// Allocates an array of information for each vertex
// Fills in the array with level and delta values
//
// fZScale converts from height values (meters) to world coordinates
//
DTErr TVTerrain::Init(const vtElevationGrid *pGrid, float fZScale)
{
	DTErr err = BasicInit(pGrid);
	if (err != DTErr_OK)
		return err;

	// determine how many levels deep
	int i, temp = m_iSize.x - 1;
	for (i = 0; temp > 1; i++)
		temp >>= 1;
	m_iLevels = i;

	x_per_y = (double)m_iSize.x / (double)m_iSize.y;
	y_per_x = (double)m_iSize.y / (double)m_iSize.x;

	float fMin, fMax;
	pGrid->GetHeightExtents(fMin, fMax);
	errPerEm = (double)MAXERR / (double)fMax;

	//	Define the size of the hierarchy(iDim)
	int iDim = next2Power(MAX(m_iSize.x, m_iSize.y) - 1) + 1;

	m_depth = 2*ceilingLog2(iDim-1) + 1;
	m_numSurfTri = 0;

	m_size = init_ntriabove(m_depth);
	m_err = (unsigned short *)malloc(m_size * sizeof(short));
	m_info = (uchar *)malloc(m_size * sizeof(char));

	fprintf(stderr, "Hierarchy uses %d bytes.\n",
		(int)(m_size*sizeof(short)+m_size*sizeof(char)));

	//  configure scaling
	mkscale(pGrid);

	// store conversion from world coordinates to array indicies
	m_fXScale = (float) m_fStep.x;
	m_fYScale = (float) m_fStep.y;
	m_fZScale = fZScale;

	// allocate height field
	m_pVertex = (float *)calloc(m_iSize.x * m_iSize.y, sizeof(float));

	int j;
	for (i = 0; i < m_iSize.x; i++)
		for (j = 0; j < m_iSize.y; j++)
		{
			m_pVertex[offset(i, j)] = pGrid->GetFValue(i, j);
		}

		//	Southwest point is origin
		sw[0] = 0;
		sw[1] = 0;

		se[0] = iDim - 1;
		se[1] = 0;

		ne[0] = iDim - 1;
		ne[1] = iDim - 1;

		nw[0] = 0;
		nw[1] = iDim - 1;

		makeDFS(pGrid, &rootL, sw, ne, nw);	// NW corner of square
		makeDFS(pGrid, &rootR, ne, sw, se);	// SE corner of square
		m_err[0] = MAXERR;
		m_info[0] = 0;

		return DTErr_OK;
}

float TVTerrain::GetElevation(int iX, int iZ, bool bTrue) const
{
	if (bTrue)
		return m_pVertex[offset(iX, iZ)] / m_fZScale;
	else
		return m_pVertex[offset(iX, iZ)];
}

void TVTerrain::GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue) const
{
	if (bTrue)
		p.Set(MAKE_XYZ2_TRUE(iX, iZ));
	else
		p.Set(MAKE_XYZ2(iX, iZ));
}


//
// return memory required, in K, for a given grid size
//
int TVTerrain::MemoryRequired(int iDimension)
{
	int k = 0;
	k += (iDimension * iDimension * 12) / 1024;		// Status heirarchy
	k += (iDimension * iDimension * sizeof(float)) / 1024;	// Height field
	return k;
}


void TVTerrain::DoCulling(const vtCamera *pCam)
{
	FPoint3 eyepos_ogl = pCam->GetTrans();
	buildSurface(eyepos_ogl);
}


void TVTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	// Load the appropriate material
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		ApplyMaterial(pMat);
		SetupTexGen(1.0f);
	}

	// Render the triangles
	emitSurface();

	// Clean up
	PostRender();
}


/* Triangle t is split into lt and rt.  Neighbors 1 & 2 of lt and
* rt are known to be the same size as lt and rt.  Neighbor 3 of
* lt is neighbor 2 of t (n3lt == n2t).  If n2t is smaller than
* t then n3lt is same size as lt, o.w. n3lt is bigger than lt (and
* n1n2t == lt is smaller than n2t == n3lt).
* Similar gibberish holds for rt.
*/

void TVTerrain::kidsOnSurf(TriIndex *t)
/* Push the surface down to the children of t.  t must not be a leaf. */
{
	TriIndex lt, rt, n2t, n1t;

	assert(!ISLEAF(t));
	assert(t->s);
	assert(ONSURFACE(t));

	m_info[t->s] &= ~SURFACE;		/* take t from surface */
	m_numSurfTri -= 1;
	leftChild(t,&lt);			/* put t's children on surface */
	if (lt.s) {
		PUTONSURFACE(&lt);
		m_numSurfTri += 1;
	}
	rightChild(t,&rt);
	if (rt.s) {
		PUTONSURFACE(&rt);
		m_numSurfTri += 1;
	}

	/* Let t's neighbors know they should point to t's children */
	nbr2(t,&n2t);
	if (n2t.s && lt.s) {/* n2t.s && !(lt.s) could happen, but then n2t is invalid */
		assert(ONSURFACE(&n2t));
		if (SMALLN2(t)) {		/* n2t is same size as lt */
			m_info[n2t.s] &= ~BIGGER3;
		} else {				/* n2t is bigger than lt */
			m_info[n2t.s] |= SMALLER1;
			m_info[lt.s] |= BIGGER3;
		}
	}

	nbr1(t,&n1t);
	if (n1t.s && rt.s) {/* n1t.s && !(rt.s) could happen, but then n1t is invalid */
		assert(ONSURFACE(&n1t));
		if (SMALLN1(t)) {		/* n1t is same size as rt */
			m_info[n1t.s] &= ~BIGGER3;
		} else {				/* n1t is bigger than lt */
			m_info[n1t.s] |= SMALLER2;
			m_info[rt.s] |= BIGGER3;
		}
	}

	if (m_info[t->s] & ALLINFOV) {
		if (lt.s) m_info[lt.s] |= ALLINFOV;
		if (rt.s) m_info[rt.s] |= ALLINFOV;
	} else if (m_info[t->s] & PARTINFOV) {
		if (lt.s) m_info[lt.s] |= triInFOV(&lt);
		if (rt.s) m_info[rt.s] |= triInFOV(&rt);
	}
}


void TVTerrain::split(TriIndex *t)
{
	TriIndex n3t,tmp;

	assert(t->s);
	assert(ONSURFACE(t));

	nbr3(t,&n3t);
	if (n3t.s == 0) goto N3INVALID;
	if (BIGN3(t)) {

		split( &n3t);	/* propagate split: t's 3-neighbor might change */

		if (IAMLEFTCHILD(t)) {
			rightChild(&n3t,&tmp);
		} else {
			leftChild(&n3t,&tmp);
		}
		if (tmp.s == 0) goto N3INVALID;
		TRIEQUATE(&n3t,&tmp);
	}
	kidsOnSurf(&n3t);		/* t is not LEAF so n3t is not LEAF */

N3INVALID:
	kidsOnSurf(t);
}


#define LEFT_TURN(a, b, c) \
	(((b)[1]-(a)[1])*((c)[0]-(a)[0])-((b)[0]-(a)[0])*((c)[1]-(a)[1]) < 0)

void TVTerrain::rodSplit(TriIndex *t)
{
	TriIndex lt,rt;

	if (t->s && !ISLEAF(t) && BORDER(t))
	{
		if (ONSURFACE(t))
			split(t);

		leftChild(t,&lt);
		rightChild(t,&rt);
		rodSplit(&lt);
		rodSplit(&rt);
	}
}


void TVTerrain::eyeSplit(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3)
{
	Coord2d mid;
	TriIndex lt,rt;

	if (t->s && !ISLEAF(t))
	{
		if (ONSURFACE(t))
			split(t);

		midpoint(p1, p2, mid);
		leftChild(t,&lt);
		rightChild(t,&rt);
		if (LEFT_TURN(mid, p3, eyeP))
		{
			eyeSplit(&lt, p3, p1, mid);
			rodSplit(&rt);
		}
		else
		{
			eyeSplit(&rt, p2, p3, mid);
			rodSplit(&lt);
		}
	}
}

/* baseSurface --
* calculates surface with finest level of detail around eye position
* and no triangles spanning border of rectangle of definition.
*/
void TVTerrain::baseSurface()
{
	eyeP[0] = (int)eyeEmx;
	eyeP[1] = (int)eyeEmy;

	/* SURFACE starts as triangles 1 and 2 */
	PUTONSURFACE(&rootL);
	PUTONSURFACE(&rootR);
	m_numSurfTri = 2;
	m_info[1] |= triInFOV(&rootL);
	m_info[2] |= triInFOV(&rootR);

#if 0	// doesn't seem necessary
	if (LEFT_TURN(sw, ne, eyeP))
	{
		eyeSplit(&rootL, sw, ne, nw);	/* NW corner of square */
		rodSplit(&rootR);
	}
	else
	{
		eyeSplit(&rootR, ne, sw, se);	/* SE corner of square */
		rodSplit(&rootL);
	}
#endif
}


/*-------------------ERROR SURFACE-----------------------------*/

int TVTerrain::tooCoarse(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3)
{
	int h;
	double dx,dy,d;

	assert(t->s);

	if (!INFOV(t))		/* if not in field of view */
		return 0;

	/* Check distance from eye */
	dx = (p3[0] - eyeP[0]);		/* dx and dy in grid units */
	dy = (p3[1] - eyeP[1]);
	d = dx * dx + dy * dy;
#if 0	// doesn't seem necessary
	if (d == 0)				/* if right below eye */
		return 1;
#endif

	d = sqrt(d);		/* d in grid units */
#if 0	// doesn't seem necessary
	if (d  < dtGrid * (m_depth - t->depth))
		return 1;
#endif

	/* Check error threshold */
	h = m_err[t->s];

	if (h > d * etGrid)
		return 1;

	return 0;
}

void TVTerrain::errSplit(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3)
{
	TriIndex lt,rt;
	Coord2d mid;

	if (t->s && !ISLEAF(t) && tooCoarse(t, p1, p2, p3))
	{
		if (ONSURFACE(t))
			split(t);

		midpoint(p1, p2, mid);
		leftChild(t,&lt);
		rightChild(t,&rt);
		errSplit(&lt, p3, p1, mid);
		errSplit(&rt, p2, p3, mid);
	}
}


void TVTerrain::errSurface()
{
	baseSurface();

	// borrow this value from DynTerrain: it appears that 1 pixel error
	// roughly equals .002 TopoVista error
	//	errThresh = m_fPixelError / 500.0;
	errThresh = .01;

	etGrid = errThresh / err2grid;	/* set error parameters */
	dtGrid = distThresh * m2grid;

	errSplit(&rootL, sw, ne, nw);	/* NW corner of square */
	errSplit(&rootR, ne, sw, se);	/* SE corner of square */
}


/*  buildSurface() -- rebuild the surface  */

void TVTerrain::buildSurface(FPoint3 &eyepos_ogl)
{
	// Convert OpenGL coordinates to TV array indicies
	// eyeEm is eye location
	eyeEmx = (eyepos_ogl.x / m_fXScale);
	eyeEmy = (-eyepos_ogl.z / m_fYScale);
	eyeEmz = eyepos_ogl.y;

	/* build the surface */
	errSurface();
}


/*  emitSurface() -- draw surface from triangle hierarchy  */

void TVTerrain::emitSurface()
{
	numDisplayTri = 0;
	emitDFS(&rootL, sw, ne, nw);	/* NW corner of square */
	emitDFS(&rootR, ne, sw, se);	/* SE corner of square */
}


/*  emitDFS(t, p1, p2, p3) -- emit triangle, possibly subdivided  */

void TVTerrain::emitDFS(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3)
{
	Coord2d mid;
	TriIndex lt,rt;

	if (t->s) {
		if (ONSURFACE(t)) {
			emitTri(t, p1, p2, p3);
		} else {

			/*   assert(!ISLEAF(t)); */

			midpoint(p1, p2, mid);
			leftChild(t,&lt);
			rightChild(t,&rt);
			emitDFS(&lt, p3, p1, mid);
			emitDFS(&rt, p2, p3, mid);
		}
	}
}



/*
* emitTri(t, p1, p2, p3) -- emit one triangle
*/
void TVTerrain::emitTri(TriIndex *t, Coord2d p1, Coord2d p2, Coord2d p3)
{
	assert(t->s);

	if (INROD(t))
	{
		numDisplayTri++;

		glBegin(GL_POLYGON);
		emitVertex(p1);
		emitVertex(p2);
		emitVertex(p3);
		glEnd();

		m_iDrawnTriangles++;
	}
}

//
// this function is unused
//
void TVTerrain::getNormal(int x1, int y1, double sz1,
						  int x2, int y2, double sz2,
						  int x3, int y3, double sz3,
						  double *nxp, double *nyp, double *nzp)
{
	double px, py, pz, len;

	double v1x = x2 - x1,  v1y = y2 - y1,  v1z = sz2 - sz1;
	double v2x = x3 - x2,  v2y = y3 - y2,  v2z = sz3 - sz2;

	px = v1y * v2z - v1z * v2y;
	py = v1z * v2x - v1x * v2z;
	pz = v1x * v2y - v1y * v2x;

	/* Normalize */
	len = px * px + py * py + pz * pz;
	len = sqrt(len);
	*nxp = px / len;
	*nyp = py / len;
	*nzp = pz / len;
}

