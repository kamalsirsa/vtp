//
// LKTerrain class :
// Implementation of the Lindstrom-Koller terrain LOD algorithm
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "LKTerrain.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#define USE_VERTEX_BUFFERS	1
#define VERTEX_BUFFER_SIZE	480
#define DO_VIEW_CULLING		1
#define SHORELINE_WEIGHT	15.0f	// This is done to weight the shoreline
									// heixels - to preserve the coastline.

#if !DO_BOUNDS_CHECK
//#define offset(i,j) ((j)*m_iDim+(i))
// probably faster:
#define offset(x, y)     (((y) << (m_iLevels)) + (y) + (x))
#endif

// how to generate OpenGL vertex locations from a heightfield index
#define LOCX(index) m_fXLookup[(index) % m_iDim]
#define LOCY(index)	(pVertex[(index)].y * m_fHeightScale)
#define LOCZ(index) m_fZLookup[(index) / m_iDim]

#define send_vertex(index) glVertex3f((index) % m_iDim, pVertex[(index)].y, ((index) / m_iDim))

// statistics
int stat_view_rejected, stat_eyepoint_tested, stat_evaluated,
	stat_turn_on_parents;

// vertex buffer
static GLfloat g_vbuf_base[VERTEX_BUFFER_SIZE*3];
static GLfloat *g_vbuf = g_vbuf_base;
static int verts_in_buffer = 0;


//
// Constructor
//
LKTerrain::LKTerrain() : vtDynTerrainGeom()
{
	pVertex = NULL;
	m_iDim = 0;
}

LKTerrain::~LKTerrain()
{
	if (pVertex) free(pVertex);
	pVertex = NULL;

#if DENSE_ARRAY
	if (pFlagSet) free(pFlagSet);
	pFlagSet = NULL;
	if (pFlagEval) free(pFlagEval);
	pFlagEval = NULL;
#else
	if (pFlags) free(pFlags);
	pFlags = NULL;
#endif
}


#define set_level(i, v)	pVertex[(i)].level = v

//
// Initialize the terrain LOD structures
//
// Allocates an array of information for each vertex
// Fills in the array with level and delta values
//
bool LKTerrain::Init(vtLocalGrid *pGrid, float fZScale,
			  float fOceanDepth, int &iError)
{
	int i, j;

	BasicInit(pGrid);

	m_iDim = m_iXPoints;
	m_iLevels = vt_log2(m_iDim);
	m_fHeightScale = fZScale;

	// iXSize and iYSize must be (power of 2) + 1
	if ((1 << m_iLevels) + 1 != m_iXPoints || (1 << m_iLevels) + 1 != m_iYPoints)
	{
		// invalid size
		return false;
	}

	m_iTotalSize = m_iXPoints * m_iYPoints;

	// Allocate the vertex array - this can be BIG
	pVertex = (Vertex *)calloc(m_iTotalSize, sizeof(Vertex));

#if DENSE_ARRAY
	m_statsize = (m_iTotalSize>>4)+1;
	pFlagSet = (unsigned short *)calloc(m_statsize, sizeof(unsigned short));
	pFlagEval = (unsigned short *)calloc(m_statsize, sizeof(unsigned short));
#else
	pFlags = (char *)calloc(m_iTotalSize, sizeof(char));
#endif

	// find indices of corner vertices
	m_sw = offset(0, 0);
	m_se = offset(m_iDim-1, 0);
	m_nw = offset(0, m_iDim-1);
	m_ne = offset(m_iDim-1, m_iDim-1);
	m_center = offset(m_iDim / 2, m_iDim / 2);

	bool shoreline;
	for (i = 0; i < m_iXPoints; i++)
	for (j = 0; j < m_iYPoints; j++)
	{
		int index = offset(i, j);

		pVertex[index].delta2 = 0;
		pVertex[index].y = pGrid->GetFValue(i, j);

		// add weight to shoreline only if the ocean depth is less than zero.
		if (fOceanDepth < 0 && pVertex[index].y == 0)
		{
			pVertex[index].y = fOceanDepth;
			shoreline = false;
			// look at neighboring points.
			// if any neighboring points have a elevation greater than zero,
			// then current point (with height 0) is on the shoreline.
			for (int di = -1; di < 2; di++)
			{
				for (int dj = -1; dj < 2; dj++)
				{
					int newi, newj;
					newi = i + di;
					newj = j + dj;

					if (newi >=0 && newi < m_iXPoints && newj >=0 && newj < m_iYPoints) {
						if (pVertex[offset(newi, newj)].y > 0) {
							shoreline = true;
						}
					}
				}
			}
			if (shoreline) {
				// place greater weight to the shoreline, to preserve the point.
				pVertex[index].delta2 = SHORELINE_WEIGHT;
			}
		}

		pVertex[index].parent0 = -1;
		pVertex[index].parent1 = -1;
	}

	ComputeLevelsAndDeltas();

	return true;
}

//
// return memory required, in K, for a given grid size
//
int LKTerrain::MemoryRequired(int iDimension)
{
	int k = 0;

	k += (iDimension * iDimension * sizeof(Vertex)) / 1024;	// Vertex array

#if DENSE_ARRAY
	int statsize = ((iDimension*iDimension)>>4)+1;
	k += (statsize * sizeof(unsigned short)) / 1024;
	k += (statsize * sizeof(unsigned short)) / 1024;
#else
	k += (iDimension * iDimension) / 1024;
#endif

	return k;
}

void LKTerrain::ComputeLevelsAndDeltas()
{
	int i, j, a, b, lo, hi, mid, level;

	// compute Delta values
	lo = 0;
	hi = m_iXPoints - 1;
	mid = (lo + hi) >> 1;

	// first set "level" field
	level = 0;
	for (a = 1; a < mid; a <<= 1)
	{
		b = a << 1;

		for (i = lo + a; i <= hi - a; i += b)
		{
			for (j = lo; j <= hi; j += b)
			{
				set_level(j*m_iXPoints + i, level);
				set_level(i*m_iXPoints + j, level);
			}
		}
		level ++;
		for (i = lo + a; i <= hi - a; i += b)
		{
			for (j = lo+a; j <= hi-a; j += b)
			{
				set_level(j*m_iXPoints + i, level);
			}
		}
		level++;
	}

	// midpoints of the sides (block edges) are a special case
	set_level(mid*m_iXPoints + lo, level);
	set_level(lo*m_iXPoints + mid, level);
	set_level(mid*m_iXPoints + hi, level);
	set_level(hi*m_iXPoints + mid, level);

	level++;

	// the center vertex is a special case
	set_level(mid*m_iXPoints + mid, level);

	level++;

	// corners are also a special case
	set_level(lo*m_iXPoints + lo, level);
	set_level(hi*m_iXPoints + lo, level);
	set_level(hi*m_iXPoints + hi, level);
	set_level(lo*m_iXPoints + hi, level);

	///////////////////////////////////////////////////////////////////
	// Now compute delta values
	for (a = 1; a < mid; a <<= 1)
	{
		b = a << 1;

		for (i = lo + a; i <= hi - a; i += b)
		{
			for (j = lo; j <= hi; j += b)
			{
				compute_delta(offset(i-a,j),
							  offset(i, j),
							  offset(i+a, j));
				compute_bbox(i, j);
				compute_delta(offset(j, i-a),
							  offset(j, i),
							  offset(j, i+a));
				compute_bbox(j, i);
			}
		}
		for (i = lo + a; i <= hi - a; i += b)
		{
			for (j = lo+a; j <= hi-a; j += b)
			{
				if ((i ^ j) & b)
				{
					// diagonals are nw-se
					compute_delta(offset(i-a, j+a),
								  offset(i, j),
								  offset(i+a, j-a));
				}
				else
				{
					// diagonals are ne-sw
					compute_delta(offset(i-a, j-a),
								  offset(i, j),
								  offset(i+a, j+a));
				}
				compute_bbox(i, j);
			}
		}
	}

	// midpoints of the sides (block edges) are a special case
	compute_delta(offset(lo, lo),
				  offset(lo, mid),
				  offset(lo, hi));
	compute_bbox(lo, mid);
	compute_delta(offset(lo, lo),
				  offset(mid, lo),
				  offset(hi, lo));
	compute_bbox(mid, lo);
	compute_delta(offset(hi, lo),
				  offset(hi, mid),
				  offset(hi, hi));
	compute_bbox(hi, mid);
	compute_delta(offset(lo, hi),
				  offset(mid, hi),
				  offset(hi, hi));
	compute_bbox(mid, hi);
}


void LKTerrain::DumpOffsets()
{
	int i, j;

	FILE *fp = fopen("numbers.txt","w");
	fprintf(fp, "\nNumbers:\n");
	for (j = 0; j < m_iDim; j++)
	{
		for (i = 0; i < m_iDim; i++)
		{
			fprintf(fp, " [%3d]", offset(i,j));
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}


void LKTerrain::DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov)
{
	// convert eyepos to the local coordinate scheme
	m_eyepos = eyepos_ogl;

	float lamdad = (float) ((window_size.x / 2.0f) / tan(fov / 2.0f));
	m_kappa2 = m_fPixelError * m_fPixelError / (lamdad * lamdad);

	stat_view_rejected = stat_eyepoint_tested = stat_evaluated = 0;
	stat_turn_on_parents = 0;

	// reset state every frame
#if DENSE_ARRAY
	memset(pFlagSet, 0, m_statsize * sizeof(unsigned short));
	memset(pFlagEval, 0, m_statsize * sizeof(unsigned short));
#else
	memset(pFlags, 0, m_iTotalSize);
#endif

	TURN_ON(m_center);
	TURN_ON(m_sw);
	TURN_ON(m_se);
	TURN_ON(m_nw);
	TURN_ON(m_ne);

	int mid = m_iDim / 2;
	evaluate_vertex(offset(0, mid));
	evaluate_vertex(offset(mid, 0));
	evaluate_vertex(offset(m_iDim-1, mid));
	evaluate_vertex(offset(mid, m_iDim-1));

//	TRACE("view_rejected %d, eyetested %d, eval'd %d, turn_on_parents %d, useless %d, ons = %d\n",
//		stat_view_rejected, stat_eyepoint_tested, stat_evaluated, stat_turn_on_parents);

//	print_culling_results();
}


GLboolean QueryExtension(char *extName)
{
	char *p = (char *) glGetString(GL_EXTENSIONS);
	char *end = p + strlen(p);
	while (p < end)
	{
		unsigned int n = strcspn(p, " ");
		if ((strlen(extName)==n) && (strncmp(extName,p,n)==0))
			return GL_TRUE;
		p += (n + 1);
	}
	return GL_FALSE;
}

void LKTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	// Render the triangles
	if (m_bUseTriStrips)
		terrain_render_tristrips();
	else
		terrain_render_simple();

	// Clean up
	PostRender();
}


//
// Find the index of a child of a vertex
//	child_num is 0,1,2,3
//
int LKTerrain::child_index(int mid_index, int child_num)
{
	if (pVertex[mid_index].level == 0) return -1;

	int step = 1 << ((pVertex[mid_index].level-1) >> 1);
	int i = mid_index % m_iDim;
	int j = mid_index / m_iDim;

	if ((pVertex[mid_index].level & 1) == 0)
	{
		// four children are diagonal
		switch (child_num)
		{
		case 0:
			if (i - step < 0 || j + step >= m_iDim) return -1;
			return offset(i-step, j+step);
		case 1:
			if (i + step >= m_iDim || j + step >= m_iDim) return -1;
			return offset(i+step, j+step);
		case 2:
			if (i + step >= m_iDim || j - step < 0) return -1;
			return offset(i+step, j-step);
		case 3:
			if (i - step < 0 || j - step < 0) return -1;
			return offset(i-step, j-step);
		}
	}
	else
	{
		// four children are lateral
		switch (child_num)
		{
		case 0:
			if (j + step >= m_iDim) return -1;
			return offset(i, j+step);
		case 1:
			if (i + step >= m_iDim) return -1;
			return offset(i+step, j);
		case 2:
			if (j - step < 0) return -1;
			return offset(i, j-step);
		case 3:
			if (i - step < 0) return -1;
			return offset(i-step, j);
		}
	}
	return -1;
}


//
// Compute the delta value for a vertex by comparing its height
// against the height of the adjacent vertices whose side it
// bisects
//
void LKTerrain::compute_delta(int left_index, int mid_index, int right_index)
{
	float delta =
		pVertex[mid_index].y -
		((float)pVertex[left_index].y + pVertex[right_index].y) / 2;
	float delta2 = delta * delta;

	// modify delta 2 only if current delta2 isn't the shoreline weight.
	if (pVertex[mid_index].delta2 < SHORELINE_WEIGHT) {
		pVertex[mid_index].delta2 = delta2;
	}

	// bottom-level vertices do not have to consider their children
	if (pVertex[mid_index].level == 0) return;

	// the delta is the maximum of this vertex's delta and the deltas
	// of all its children
	int child;
	for (int child_num = 0; child_num < 4; child_num++)
	{
		child = child_index(mid_index, child_num);
		if (child >= 0)
		{
			delta2 = MAX(delta2, pVertex[child].delta2);

			// Luke, I Am Your Faaa-ther
			if (pVertex[child].parent0 < 0)
				pVertex[child].parent0 = mid_index;
			else
				pVertex[child].parent1 = mid_index;
		}
	}

	// modify delta 2 only if current delta2 isn't the shoreline weight.
	if (pVertex[mid_index].delta2 < SHORELINE_WEIGHT) {
		pVertex[mid_index].delta2 = delta2;
	}
}


//
// Compute the bounding box for a single vertex.
//
// We do this by finding the union of it's 4 child bounding boxes.
//
// Because of the double-binary-tree structure, a high-level vertex can
// have a bounding box that encompasses nearly the entire mesh.
//
void LKTerrain::compute_bbox(int i, int j)
{
	int index = offset(i, j);

	pVertex[index].xmin = pVertex[index].xmax = i;
	pVertex[index].ymin = pVertex[index].ymax = pVertex[index].y;
	pVertex[index].zmin = pVertex[index].zmax = j;

	if (pVertex[index].level == 0)
		return;

	int child;
	for (int child_num = 0; child_num < 4; child_num++)
	{
		child = child_index(index, child_num);
		if (child < 0)
			continue;

		if (pVertex[child].xmin < pVertex[index].xmin) pVertex[index].xmin = pVertex[child].xmin;
		if (pVertex[child].xmax > pVertex[index].xmax) pVertex[index].xmax = pVertex[child].xmax;
		if (pVertex[child].ymin < pVertex[index].ymin) pVertex[index].ymin = pVertex[child].ymin;
		if (pVertex[child].ymax > pVertex[index].ymax) pVertex[index].ymax = pVertex[child].ymax;
		if (pVertex[child].zmin < pVertex[index].zmin) pVertex[index].zmin = pVertex[child].zmin;
		if (pVertex[child].zmax > pVertex[index].zmax) pVertex[index].zmax = pVertex[child].zmax;
	}
}


void LKTerrain::turn_on_parents(int index)
{
	stat_turn_on_parents++;

	int parent0 = pVertex[index].parent0;

	if (parent0 >= 0 && !IS_EVALUATED(parent0) && !IS_ON(parent0))
	{
		TURN_ON(parent0);
		turn_on_parents(parent0);
	}

	int parent1 = pVertex[index].parent1;

	if (parent1 >= 0 && !IS_EVALUATED(parent1) && !IS_ON(parent1))
	{
		TURN_ON(parent1);
		turn_on_parents(parent1);
	}
}


void LKTerrain::evaluate_vertex(int index)
{
	if (IS_EVALUATED(index))
		return;

	SET_EVALUATED(index);

	stat_evaluated++;

	if (pVertex[index].level == 0)
	{
		// bottom-most level
		FPoint3 diff;
		diff.x = m_eyepos.x - LOCX(index);
		diff.y = m_eyepos.y - LOCY(index);
		diff.z = m_eyepos.z - LOCZ(index);

		diff.x *= diff.x;
		diff.y *= diff.y;
		diff.z *= diff.z;

		if (pVertex[index].delta2 * (diff.x + diff.z) /
			((diff.x + diff.y + diff.z) * (diff.x + diff.y + diff.z))
			> m_kappa2)
		{
			// true!  turn it on
			// projects back larger than what we've determined to be the allowable error

			TURN_ON(index);
			turn_on_parents(index);
		}
	}
	else if (IS_ON(index))
	{
		int child;
		for (int child_num = 0; child_num < 4; child_num++)
		{
			child = child_index(index, child_num);
			if (child < 0)
				continue;
			evaluate_vertex(child);
		}
	}
	else if (m_eyepos.x > m_fXLookup[pVertex[index].xmin] &&
			 m_eyepos.x < m_fXLookup[pVertex[index].xmax] &&
			 m_eyepos.z < m_fZLookup[pVertex[index].zmin] &&
			 m_eyepos.z > m_fZLookup[pVertex[index].zmax] &&
			 m_eyepos.y > pVertex[index].ymin &&
			 m_eyepos.y < pVertex[index].ymax)
	{
		TURN_ON(index);
		turn_on_parents(index);

		int child;
		for (int child_num = 0; child_num < 4; child_num++)
		{
			child = child_index(index, child_num);
			if (child < 0)
				continue;
			evaluate_vertex(child);
		}
	}
	else
	{
#if DO_VIEW_CULLING
		if (pVertex[index].level > 0)
		{
			// experimental culling
			FBox3 bbox;

			bbox.min.x = m_fXLookup[pVertex[index].xmin];
			bbox.min.y = pVertex[index].ymin;
			bbox.min.z = m_fZLookup[pVertex[index].zmin];

			bbox.max.x = m_fXLookup[pVertex[index].xmax];
			bbox.max.y = pVertex[index].ymax;
			bbox.max.z = m_fZLookup[pVertex[index].zmax];

			FSphere sph(bbox);

			int result = IsVisible(sph);
			if (result == 0)
			{
				// not visible - don't split, don't draw
				stat_view_rejected++;
				return;
			}
			else
			{
				// it is visible
			}
		}
#endif
		float h2;

		stat_eyepoint_tested++;

		if (m_eyepos.y > pVertex[index].ymax)
		{
			// eye is above the bounding box
			h2 = (m_eyepos.y - pVertex[index].ymax) *
				 (m_eyepos.y - pVertex[index].ymax);
		}
		else if (m_eyepos.y < pVertex[index].ymin)
		{
			// eye is below the bounding box
			h2 = (m_eyepos.y - pVertex[index].ymin) *
				 (m_eyepos.y - pVertex[index].ymin);
		}
		else
		{
			// eye is in the middle of the z range - height could be as little as 0
			h2 = 0;
		}
		float r_clampx = clamp(m_fXLookup[pVertex[index].xmin], m_eyepos.x, m_fXLookup[pVertex[index].xmax]);
		float r_clampz = clamp(m_fZLookup[pVertex[index].zmax], m_eyepos.z, m_fZLookup[pVertex[index].zmin]);

		float r_min2 = (m_eyepos.x - r_clampx) * (m_eyepos.x - r_clampx) +
					   (m_eyepos.z - r_clampz) * (m_eyepos.z - r_clampz);

		float r_max_x = MAX(fabsf(m_eyepos.x - m_fXLookup[pVertex[index].xmin]),
							fabsf(m_eyepos.x - m_fXLookup[pVertex[index].xmax]));
		float r_max_z = MAX(fabsf(m_eyepos.z - m_fZLookup[pVertex[index].zmin]),
							fabsf(m_eyepos.z - m_fZLookup[pVertex[index].zmax]));

		float r_max2 = r_max_x * r_max_x + r_max_z * r_max_z;

		float r2 = clamp(r_min2, h2, r_max2);

		float f2 = r2 / ((r2 + h2) * (r2 + h2));

		if (f2 * pVertex[index].delta2 > m_kappa2)
		{
			TURN_ON(index);
			turn_on_parents(index);

			int child;
			for (int child_num = 0; child_num < 4; child_num++)
			{
				child = child_index(index, child_num);
				if (child < 0)
					continue;
				evaluate_vertex(child);
			}
		}
	}
}

int g_level;
int g_head, g_tail;

void LKTerrain::render_vertex(int index_l, int index_mid, int index_r, int level)
{
	if (level > 0 && IS_ON(index_mid))
	{
		if (level > 1)
			render_vertex(index_l, (index_l + index_r) >> 1, index_mid, level-1);

		if (g_head != index_mid && g_tail != index_mid)
		{
			if ((level + g_level) & 1)
				g_tail = g_head;
			else
				send_vertex(g_tail);
			send_vertex(index_mid);
			g_head = index_mid;
			g_level = level;
		}

		if (level > 1)
			render_vertex(index_mid, (index_l + index_r) >> 1, index_r, level-1);
	}
	m_iDrawnTriangles++;
}


void LKTerrain::terrain_render_tristrips2()
{
	glBegin(GL_TRIANGLE_STRIP);

	send_vertex(m_sw);
	send_vertex(m_sw);
	g_level = 1;
	g_head = g_tail = 0;

	render_vertex(m_sw, m_center, m_se, 2*m_iLevels);

	if (!(g_level&1))
		g_tail = g_head;
	else
		send_vertex(g_tail);

	send_vertex(m_se);
	g_head = m_se;
	g_level = 1;

	render_vertex(m_se, m_center, m_ne, 2*m_iLevels);

	if (!(g_level&1))
		g_tail = g_head;
	else
		send_vertex(g_tail);

	send_vertex(m_ne);
	g_head = m_ne;
	g_level = 1;

	render_vertex(m_ne, m_center, m_nw, 2*m_iLevels);

	if (!(g_level&1))
		g_tail = g_head;
	else
		send_vertex(g_tail);

	send_vertex(m_nw);
	g_head = m_nw;
	g_level = 1;

	render_vertex(m_nw, m_center, m_sw, 2*m_iLevels);

	if (!(g_level&1))
		g_tail = g_head;
	else
		send_vertex(g_tail);

	send_vertex(m_sw);

	glEnd();
}

//
// Render the terrain using a sophisticated triangle stripping algorithm
// which draws the entire mesh as a single strip (with some degenerate
// triangles along the way, for turning).
//
void LKTerrain::terrain_render_tristrips()
{
	vtMaterial *pMat = GetMaterial(0);
	if (!pMat) return;
	pMat->Apply();

	SetupTexGen(1.0f);

	// render once
	terrain_render_tristrips2();

	if (m_bDetailTexture)
	{
		m_pDetailMat->Apply();

		SetupTexGen(m_fDetailTiling);
		// render twice
		terrain_render_tristrips2();
	}
	DisableTexGen();
}


void LKTerrain::determine_bucket(int index_l, int index_r, int index_mid,
								 int &a, int &b)
{
	float x = 0.0f, z = 0.0f;
	x += m_fXLookup[index_l % m_iDim];
	z += m_fZLookup[index_l / m_iDim];
	x += m_fXLookup[index_r % m_iDim];
	z += m_fZLookup[index_r / m_iDim];
	x += m_fXLookup[index_mid % m_iDim];
	z += m_fZLookup[index_mid / m_iDim];
	x /= 3.0f;
	z /= 3.0f;

	a = (int) (((float)BUCKETS) * (x - m_WorldExtents.left) / m_WorldSize.x);
	b = (int) (((float)BUCKETS) * (z - m_WorldExtents.top) / m_WorldSize.y);

	if (!(a >= 0 && a < BUCKETS && b >= 0 && b < BUCKETS))
		int foo = 1;
}

void LKTerrain::flush_buffer(bool bStrips)
{
//	glVertexPointer(3, GL_FLOAT, 0, VBUF_SIZE, g_vbuf_base);
	glVertexPointer(3, GL_FLOAT, 0, g_vbuf_base);
	if (bStrips)
		glDrawArrays(GL_TRIANGLE_STRIP, 0, verts_in_buffer);
	else
		glDrawArrays(GL_TRIANGLES, 0, verts_in_buffer);

	g_vbuf = g_vbuf_base;
	verts_in_buffer = 0;
}

void LKTerrain::render_triangle(int index_l, int index_mid, int index_r, int level)
{
	int splitter = (index_l + index_r) >> 1;

	if ((level > 1 && IS_ON(splitter)) || level > ((m_iLevels << 1) - 4))
	{
		render_triangle(index_mid, (index_l + index_r) >> 1, index_l, level-1);
		render_triangle(index_r, (index_l + index_r) >> 1, index_mid, level-1);
		return;
	}
	if (m_iTPatchDim != 1)
	{
		int a, b;
		determine_bucket(index_l, index_r, index_mid, a, b);

		m_Bucket[a][b].Append(index_l);
		m_Bucket[a][b].Append(index_r);
		m_Bucket[a][b].Append(index_mid);
	}
	else
	{
#if USE_VERTEX_BUFFERS
		*g_vbuf++ = m_fXLookup[index_l % m_iDim];
		*g_vbuf++ = pVertex[index_l].y;
		*g_vbuf++ = m_fZLookup[index_l / m_iDim];

		*g_vbuf++ = m_fXLookup[index_r % m_iDim];
		*g_vbuf++ = pVertex[index_r].y;
		*g_vbuf++ = m_fZLookup[index_r / m_iDim];

		*g_vbuf++ = m_fXLookup[index_mid % m_iDim];
		*g_vbuf++ = pVertex[index_mid].y;
		*g_vbuf++ = m_fZLookup[index_mid / m_iDim];

		verts_in_buffer += 3;
		if (verts_in_buffer == VERTEX_BUFFER_SIZE)
			flush_buffer(false);
#else
		glBegin(GL_TRIANGLES);

		send_vertex(index_l);
		send_vertex(index_r);
		send_vertex(index_mid);

		glEnd();
#endif
	}
	m_iDrawnTriangles++;
}


void LKTerrain::terrain_render_simple()
{
	if (m_iTPatchDim != 1)
		empty_buckets();
	else
	{
		vtMaterial *pMat = GetMaterial(0);
		if (!pMat) return;
		pMat->Apply();
	}
	SetupTexGen(1.0f);

	// render once
	// if using buckets, fills the buckets
	terrain_render_simple2();

	if (m_bDetailTexture && m_iTPatchDim == 1)
	{
		m_pDetailMat->Apply();

		SetupTexGen(m_fDetailTiling);
		// render twice
		terrain_render_simple2();
	}
	if (m_iTPatchDim != 1)
		terrain_render_buckets();

	DisableTexGen();
}

void LKTerrain::terrain_render_simple2()
{
	render_triangle(m_sw, m_center, m_se, m_iLevels*2);	// bottom
	render_triangle(m_se, m_center, m_ne, m_iLevels*2);	// right
	render_triangle(m_ne, m_center, m_nw, m_iLevels*2);	// top
	render_triangle(m_nw, m_center, m_sw, m_iLevels*2);	// left
#if USE_VERTEX_BUFFERS
	if (verts_in_buffer)
		flush_buffer(false);
#endif
}


void LKTerrain::empty_buckets()
{
	int a, b;
	for (a = 0; a < BUCKETS; a++)
		for (b = 0; b < BUCKETS; b++)
			m_Bucket[a][b].Empty();
}


void LKTerrain::terrain_render_buckets()
{
	int a, b;

	for (a = 0; a < BUCKETS; a++)
	{
		for (b = 0; b < BUCKETS; b++)
		{
			int appidx = a*BUCKETS + b;

			vtMaterial *pMat = GetMaterial(appidx);
			if (!pMat) continue;
			pMat->Apply();
			SetupBlockTexGen(a, b);

			int tris = m_Bucket[a][b].GetSize() / 3;
			int index, i, j;
			for (int t = 0; t < tris; t++)
			{
				glBegin(GL_TRIANGLES);

				index = m_Bucket[a][b].GetAt(t * 3 + 0);
				i = (index % m_iDim) - m_Bucket[a][b].base_i;
				j = (index / m_iDim) - m_Bucket[a][b].base_j;
				send_vertex(index);

				index = m_Bucket[a][b].GetAt(t * 3 + 1);
				i = (index % m_iDim) - m_Bucket[a][b].base_i;
				j = (index / m_iDim) - m_Bucket[a][b].base_j;
				send_vertex(index);

				index = m_Bucket[a][b].GetAt(t * 3 + 2);
				i = (index % m_iDim) - m_Bucket[a][b].base_i;
				j = (index / m_iDim) - m_Bucket[a][b].base_j;
				send_vertex(index);

				glEnd();
			}
		}
	}
}


void LKTerrain::GetLocation(int i, int j, FPoint3 &p) const
{
	p.Set(m_fXLookup[i], LOCY(offset(i,j)), m_fZLookup[j]);
}

///////////////////////////////////////////////////////
// Old unused code

#define RENDER_WIREFRAME	1

void LKTerrain::print_culling_results()
{
	int i, j;
	printf("\nState:\n");
	for (j = 0; j < m_iDim; j++)
	{
		for (i = 0; i < m_iDim; i++)
		{
			printf(" %s%s", IS_ON(offset(i,j)) ? "O" : "x",
				IS_EVALUATED(offset(i,j)) ? "e" : ".");
		}
		printf("\n");
	}
}
