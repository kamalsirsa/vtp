//
// BryanTerrain class : an generic implementation of
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "BryanTerrain.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef unsigned long UINT_32;
typedef long INT_32;

//
// Macro used to determine the index of a vertex (element of the height
// field) given it's (x,y) location in the grid
//
#define offset(x, y)  ((y) * m_iColumns + (x))

//
// Macro used to generate vertex locations from a heightfield index
// (You don't have to use them, especially for height)
//
#define LOCX(index) m_fXLookup[index % m_iDim]
#define LOCY(index)	m_pData[index]*m_fZScale
#define LOCZ(index) m_fZLookup[index / m_iDim]
#define MAKE_XYZ(index) LOCX(index), LOCY(index), LOCZ(index)
#define MAKE_XYZ2(x,y) m_fXLookup[x], m_pData[offset(x,y)]*m_fZScale, m_fZLookup[y]
#define MAKE_XYZ2_TRUE(x,y) m_fXLookup[x], m_pData[offset(x,y)], m_fZLookup[y]


//
// Construct/destruct
//
BryanTerrain::BryanTerrain() : vtDynTerrainGeom()
{
	m_pData = NULL;
}

BryanTerrain::~BryanTerrain()
{
	if (m_pData)
		delete m_pData;
}


//
// Initialize the terrain data
// fZScale converts from height values (meters) to world coordinates
//
DTErr BryanTerrain::Init(const vtElevationGrid *pGrid, float fZScale)
{
	// Initializes necessary field of the parent class
	DTErr err = BasicInit(pGrid);
	if (err != DTErr_OK)
		return err;

	//
	// Allocate array, copy data from supplied elevation grid
	//
	// (replace this with your own storage representation)
	//
	UINT_32 nPoints = m_iColumns, nWidth = 0;
	if (m_iRows < m_iColumns)		// Pick the smallest size..
		nPoints = m_iRows;

	// Munge into a 2^n+1 size.
	while ((nWidth+1) > nPoints)
		nWidth <<= 1;					// Find 2^n size which is >= to nPoints
	nWidth++;							// Make it 2^n + 1

	if ( nWidth > nPoints )				// If it's too big
		nPoints = (nWidth>>1) + 1;		// Use the next smaller 2^n + 1 size

	m_pData = new float[nPoints * nPoints];
	int i, j;
	for (i = 0; i < m_iColumns; i++)
	{
		for (j = 0; j < m_iRows; j++)
			m_pData[offset(i,j)] = pGrid->GetFValue(i, j);
	}

	m_fZScale = fZScale;
	m_iDrawnTriangles = -1;

	// --------------------------------------------------
	// Quad Tree Initialization
	// --------------------------------------------------
	{
		UINT_32 i;
		// Initialize Frame Variance
		m_fFrameVariance = 0.32f;
		m_nPoints = nPoints;

		m_aQuadPool = new QuadNodeStruc[POOL_SIZE];

		memset( m_aQuadPool, 0, sizeof(QuadNodeStruc) * POOL_SIZE );

		m_aQuadPool[0].nNorth = EDGE_NODE;	// Set up the Root Node's Neighbors.
		m_aQuadPool[0].nSouth = EDGE_NODE;
		m_aQuadPool[0].nEast  = EDGE_NODE;
		m_aQuadPool[0].nWest  = EDGE_NODE;

		// Create Free Block Links
		i = 5;								// Node Zero is not used, so node 1-4 are first block, thus start with node 5.
		while ( i < (POOL_SIZE-3) )			// POOL_SIZE-3 ensures that the last node is never used.
		{
			m_aQuadPool[i-4].nNorth = i;	// Use North pointer to link free blocks.
			i += 4;
		}

		m_nFirstFree = 1;
	}
	return DTErr_OK;
}


void BryanTerrain::DoCulling(const vtCamera *pCam)
{
	m_eyepos_ogl = pCam->GetTrans();

	// Tessellate the Mesh
	RecursTessellate( 0, 0, 0, m_nPoints, 4 );
}


void BryanTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	glFrontFace(GL_CW);

	// Render the triangles
	RenderSurface();

	glFrontFace(GL_CCW);

	// Clean up
	PostRender();
}


void BryanTerrain::LoadSingleMaterial()
{
	// single texture for the whole terrain
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		pMat->Apply();
		SetupTexGen(1.0f);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}


void BryanTerrain::RenderSurface()
{
	LoadSingleMaterial();
//	RenderPass();
	RecursRender( 0, 0, 0, m_nPoints );

/*	if (m_bDetailTexture)
	{
		// once again, with the detail texture
		m_pDetailMat->Apply();
		SetupTexGen(m_fDetailTiling);

		glPolygonOffset(-1.0f, -1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		RenderSurface();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
*/
	DisableTexGen();
}

void BryanTerrain::RenderPass()
{
	//
	// Very naive code which draws the grid as immediate-mode
	// triangle strips.  (Replace with your own algorithm.)
	//
/*	int i, j;
	for (i = 0; i < m_iColumns-5; i+=4)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < m_iRows; j+=4)
		{
			glVertex3f(MAKE_XYZ2(i, j));
			glVertex3f(MAKE_XYZ2(i+4, j));
			m_iDrawnTriangles += 2;
		}
		glEnd();
	}
*/
}

float BryanTerrain::GetElevation(int iX, int iZ, bool bTrue) const
{
	float f = m_pData[offset(iX,iZ)];
	if (bTrue)
		f /= m_fZScale;
	return f;
}

void BryanTerrain::GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue) const
{
	if (bTrue)
		p.Set(MAKE_XYZ2_TRUE(iX, iZ));
	else
		p.Set(MAKE_XYZ2(iX, iZ));
}

// -------------------------------------------------------------
//
// -------------------------------------------------------------
inline float BryanTerrain::HeightField( UINT_32 y, UINT_32 x )
{
	return m_pData[offset(x,y)]*m_fZScale;
}

// -------------------------------------------------------------
//
// -------------------------------------------------------------
inline void BryanTerrain::MakeVertex( UINT_32 x, UINT_32 y, float height, UINT_32 NW_X, UINT_32 NW_Y )
{
	glVertex3f(	m_fXLookup[NW_X + (x)],	// Send the vertex to the graphics card
				height,
				m_fZLookup[NW_Y + (y)] );
}

// -------------------------------------------------------------
//
// -------------------------------------------------------------
inline void BryanTerrain::MakeBlendedVertex(UINT_32 x, UINT_32 y,
	float heightOne, float heightTwo, float fBlend,	UINT_32 NW_X, UINT_32 NW_Y)
{
	float height = (fBlend * heightOne) + ((1.0f-fBlend) * heightTwo);
	MakeVertex( x, y, height, NW_X, NW_Y );
}

// --------------------------------------------------------
//
// --------------------------------------------------------
void BryanTerrain::Split( UINT16 nNode )
{
	QuadNodeStruc *pNode = &(m_aQuadPool[nNode]);

	if ( pNode->nChildren == 0 &&				// IF there are no children currently
		 pNode->nNorth && pNode->nSouth &&		// And all the neighbors at this level exist
		 pNode->nEast && pNode->nWest &&
		 m_nFirstFree							// And there are free nodes... THEN
		 )
	{
//		Allocate 4 nodes
		UINT16 nChild = pNode->nChildren = m_nFirstFree;	// Take the first free node block
		m_nFirstFree = m_aQuadPool[nChild].nNorth;			// Point at the next free node block

//		Set their info to NULL => memset()
		memset( &(m_aQuadPool[nChild]), 0, 4 * sizeof(QuadNodeStruc) );

//		Attach children to each other in mesh, and to neighbor's children, be sure to pass on 0xFFFF where needed.
		m_aQuadPool[nChild + CHILD_NW].nEast  = nChild + CHILD_NE;			// Link the children to each other.
		m_aQuadPool[nChild + CHILD_NW].nSouth = nChild + CHILD_SW;

		m_aQuadPool[nChild + CHILD_NE].nWest  = nChild + CHILD_NW;
		m_aQuadPool[nChild + CHILD_NE].nSouth = nChild + CHILD_SE;

		m_aQuadPool[nChild + CHILD_SW].nEast  = nChild + CHILD_SE;
		m_aQuadPool[nChild + CHILD_SW].nNorth = nChild + CHILD_NW;

		m_aQuadPool[nChild + CHILD_SE].nWest  = nChild + CHILD_SW;
		m_aQuadPool[nChild + CHILD_SE].nNorth = nChild + CHILD_NE;

		// NW = CHILD_NW, NE = CHILD_NE, SW = CHILD_SW, SE = CHILD_SE
		if ( pNode->nNorth )	// Link to children of my neighbors (NORTH)
		{
			if ( m_aQuadPool[pNode->nNorth].nChildren )
			{
				m_aQuadPool[nChild + CHILD_NW].nNorth = m_aQuadPool[pNode->nNorth].nChildren + CHILD_SW;
				m_aQuadPool[nChild + CHILD_NE].nNorth = m_aQuadPool[pNode->nNorth].nChildren + CHILD_SE;

				m_aQuadPool[m_aQuadPool[pNode->nNorth].nChildren + CHILD_SW].nSouth = nChild + CHILD_NW;
				m_aQuadPool[m_aQuadPool[pNode->nNorth].nChildren + CHILD_SE].nSouth = nChild + CHILD_NE;

			}
			else if ( pNode->nNorth == EDGE_NODE )
			{
				m_aQuadPool[nChild + CHILD_NW].nNorth = EDGE_NODE;
				m_aQuadPool[nChild + CHILD_NE].nNorth = EDGE_NODE;
			}
		}

		if ( pNode->nSouth )
		{
			if ( m_aQuadPool[pNode->nSouth].nChildren )		// (SOUTH)
			{
				m_aQuadPool[nChild + CHILD_SW].nSouth = m_aQuadPool[pNode->nSouth].nChildren + CHILD_NW;
				m_aQuadPool[nChild + CHILD_SE].nSouth = m_aQuadPool[pNode->nSouth].nChildren + CHILD_NE;

				m_aQuadPool[m_aQuadPool[pNode->nSouth].nChildren + CHILD_NW].nNorth = nChild + CHILD_SW;
				m_aQuadPool[m_aQuadPool[pNode->nSouth].nChildren + CHILD_NE].nNorth = nChild + CHILD_SE;
			}
			else if ( pNode->nSouth == EDGE_NODE )
			{
				m_aQuadPool[nChild + CHILD_SW].nSouth = EDGE_NODE;
				m_aQuadPool[nChild + CHILD_SE].nSouth = EDGE_NODE;
			}
		}

		if ( pNode->nEast )
		{
			if ( m_aQuadPool[pNode->nEast].nChildren )		// (EAST)
			{
				m_aQuadPool[nChild + CHILD_NE].nEast = m_aQuadPool[pNode->nEast].nChildren + CHILD_NW;
				m_aQuadPool[nChild + CHILD_SE].nEast = m_aQuadPool[pNode->nEast].nChildren + CHILD_SW;

				m_aQuadPool[m_aQuadPool[pNode->nEast].nChildren + CHILD_NW].nWest = nChild + CHILD_NE;
				m_aQuadPool[m_aQuadPool[pNode->nEast].nChildren + CHILD_SW].nWest = nChild + CHILD_SE;
			}
			else if ( pNode->nEast == EDGE_NODE )
			{
				m_aQuadPool[nChild + CHILD_NE].nEast = EDGE_NODE;
				m_aQuadPool[nChild + CHILD_SE].nEast = EDGE_NODE;
			}
		}

		if ( pNode->nWest )
		{
			if ( m_aQuadPool[pNode->nWest].nChildren )		// (WEST)
			{
				m_aQuadPool[nChild + CHILD_NW].nWest = m_aQuadPool[pNode->nWest].nChildren + CHILD_NE;
				m_aQuadPool[nChild + CHILD_SW].nWest = m_aQuadPool[pNode->nWest].nChildren + CHILD_SE;

				m_aQuadPool[m_aQuadPool[pNode->nWest].nChildren + CHILD_NE].nEast = nChild + CHILD_NW;
				m_aQuadPool[m_aQuadPool[pNode->nWest].nChildren + CHILD_SE].nEast = nChild + CHILD_SW;
			}
			else if ( pNode->nWest == EDGE_NODE )
			{
				m_aQuadPool[nChild + CHILD_NW].nWest = EDGE_NODE;
				m_aQuadPool[nChild + CHILD_SW].nWest = EDGE_NODE;
			}
		}
	}
//	else
//		;
	// Children not allowed for this node unless other nodes are split first.
	//  - Should it split them?
	//	  * probably not for breadth-first algorithm.  This ensures no more depth is added until the breadth is filled out.
}

// --------------------------------------------------------
//
// --------------------------------------------------------
void BryanTerrain::FreeChildren( UINT16 nNode )
{
	QuadNodeStruc *pNode = &(m_aQuadPool[nNode]);
	UINT16 nChildren = pNode->nChildren;

	// Deletes the children of this node, and all other nodes which may be invalidated by it.
	if ( (pNode->nChildren == 0) ||
		 (nNode < 5) ||
		 (nNode == EDGE_NODE) )
		return;

	// Mark my children as deleted
	pNode->nChildren = 0;

	// Recursively Free my children's children
	FreeChildren( nChildren + CHILD_NW );
	FreeChildren( nChildren + CHILD_NE );
	FreeChildren( nChildren + CHILD_SW );
	FreeChildren( nChildren + CHILD_SE );

	// Unlink the child from the Mesh
	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_NW].nNorth ].nSouth = 0;
	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_NW].nWest  ].nEast  = 0;

	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_NE].nNorth ].nSouth = 0;
	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_NE].nEast  ].nWest  = 0;

	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_SW].nSouth ].nNorth = 0;
	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_SW].nWest  ].nEast  = 0;

	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_SE].nSouth ].nNorth = 0;
	m_aQuadPool[ m_aQuadPool[nChildren + CHILD_SE].nEast  ].nWest  = 0;

	// Link my children into the free list
	m_aQuadPool[ nChildren ].nNorth = m_nFirstFree;
	m_nFirstFree = nChildren;

	// Free my neighbor's children also, to keep the mesh legal
	FreeChildren( m_aQuadPool[pNode->nNorth].nChildren + CHILD_SW );
	FreeChildren( m_aQuadPool[pNode->nNorth].nChildren + CHILD_SE );

	FreeChildren( m_aQuadPool[pNode->nSouth].nChildren + CHILD_NW );
	FreeChildren( m_aQuadPool[pNode->nSouth].nChildren + CHILD_NE );

	FreeChildren( m_aQuadPool[pNode->nEast].nChildren + CHILD_NW );
	FreeChildren( m_aQuadPool[pNode->nEast].nChildren + CHILD_SW );

	FreeChildren( m_aQuadPool[pNode->nWest].nChildren + CHILD_NE );
	FreeChildren( m_aQuadPool[pNode->nWest].nChildren + CHILD_SE );
}

// -------------------------------------------------------------
// Decide if this Quad Node needs to be split.
// -------------------------------------------------------------
UINT8 BryanTerrain::SplitMetric( UINT16 nNode, UINT_32 NW_X, UINT_32 NW_Y, UINT_32 nQuadWidth )
{
#define MIN_QUAD_WIDTH (2)

	QuadNodeStruc *pNode = &(m_aQuadPool[nNode]);
	int radius = nQuadWidth >> 1;

	// ---------------------------------------------------------------------
	// Distance Metric
	//
	float A = (float)fabs(m_fXLookup[NW_X + radius] - m_eyepos_ogl.x);
	float B = (float)fabs(m_fZLookup[NW_Y + radius] - m_eyepos_ogl.z);
	float C = (float)fabs(HeightField( NW_Y + radius, NW_X + radius ) -  m_eyepos_ogl.y) ;

	// The L-1 Norm: Max distance (X, Y, Z) to Center of Quad
	float fDistance = MAX( A, MAX( B, C ) );

	// L-2 Norm Calculation : Pythagorian Theorem in 3D
//	m_fDistance = (float)sqrtf( SQR(A) + SQR(B) + SQR(C) );

	// A given Quad will have some error associated with it.
	// Using this error, we adjust the 'size' of the quad to make it look larger (thus more error = bigger triangle = split sooner)
	float fSplit = ((nQuadWidth							// Take the quad's size
					//+ m_pError[nErrNode]				// Add in the error for this quad to make the quad larger
					) / fDistance);						// Divide by distance to scale the need to split by it's proximity.

	pNode->nBlend = 0xFF;	// Blend always == 1.0f

	if ( nQuadWidth > MIN_QUAD_WIDTH ) 					// Minimum Quad size.
		return ( fSplit > m_fFrameVariance );
	else
		return 0;
}

// -------------------------------------------------------------------------------------------------------------
// Recursively walk the tree, culling nodes which no longer need to be split, and selecting nodes for splitting.
// -------------------------------------------------------------------------------------------------------------
void BryanTerrain::RecursTessellate( UINT16 nNode, UINT_32 NW_X, UINT_32 NW_Y, UINT_32 nQuadWidth, UINT8 nSortOrder )
{
	QuadNodeStruc *pNode = &(m_aQuadPool[nNode]);
	UINT_32 radius = nQuadWidth >> 1;

	UINT8 bSplit = SplitMetric( nNode, NW_X, NW_Y, nQuadWidth );

	if ( bSplit && !pNode->nChildren)			// IF this node needs splitting...
		Split( nNode );							// Add this node to the queue
	else if ( pNode->nChildren && !bSplit )		// IF this node has children AND it does not need to be split anymore
		FreeChildren( nNode );

	if ( pNode->nChildren )						// Always traverse the entire tree...
	{
		// ---------------------------------------------------------------------
		// Select the node closest to the eye for processing first.
		// ---------------------------------------------------------------------

//	BT: Need to compress this code into a table lookup or two.  It's ugly and inefficient.

		if ( nSortOrder == 4 )
		{
//			nSortOrder = 0;
			//
			// Find which Quad the Eye is in..
			//
/*			if ( gViewPosition[0] > (NW_X + radius) )
				nSortOrder |= 0x01;

			if ( gViewPosition[2] > (NW_Y + radius) )
				nSortOrder |= 0x02;
*/
			nSortOrder =	((m_eyepos_ogl.z > (m_fZLookup[NW_Y + radius])) << 1 ) +	// Sets bit 0x02 = North or South of eye
							(m_eyepos_ogl.x > (m_fXLookup[NW_X + radius]));			// Sets bit 0x01 = East or West of eye
		}

		// Visit the nodes in (roughly) closest-to-eye order.
		switch (nSortOrder)
		{
		case 2:		// Eye is in NW quad
			RecursTessellate( pNode->nChildren + CHILD_NW, NW_X,			NW_Y,			radius, 4		 );
			RecursTessellate( pNode->nChildren + CHILD_NE, NW_X + radius,	NW_Y,			radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_SW, NW_X,			NW_Y + radius,	radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_SE, NW_X + radius,	NW_Y + radius,	radius, nSortOrder);
			break;

		case 3:		// Eye is in NE quad
			RecursTessellate( pNode->nChildren + CHILD_NE, NW_X + radius,	NW_Y,			radius, 4		);
			RecursTessellate( pNode->nChildren + CHILD_NW, NW_X,			NW_Y,			radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_SE, NW_X + radius,	NW_Y + radius,	radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_SW, NW_X,			NW_Y + radius,	radius, nSortOrder);
			break;

		case 0:		// Eye is in SW quad
			RecursTessellate( pNode->nChildren + CHILD_SW, NW_X,			NW_Y + radius,	radius, 4		);
			RecursTessellate( pNode->nChildren + CHILD_NW, NW_X,			NW_Y,			radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_SE, NW_X + radius,	NW_Y + radius,	radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_NE, NW_X + radius,	NW_Y,			radius, nSortOrder);
			break;

		case 1:		// Eye is in SE quad
			RecursTessellate( pNode->nChildren + CHILD_SE, NW_X + radius,	NW_Y + radius,	radius, 4		);
			RecursTessellate( pNode->nChildren + CHILD_NE, NW_X + radius,	NW_Y,			radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_SW, NW_X,			NW_Y + radius,	radius, nSortOrder);
			RecursTessellate( pNode->nChildren + CHILD_NW, NW_X,			NW_Y,			radius, nSortOrder);
			break;
		}
	}
}

// -------------------------------------------------------------
// Recursively render the QuadTree, outputting leaf nodes as FANs
// -------------------------------------------------------------
void BryanTerrain::RecursRender(	UINT16 nNode, UINT_32 NW_X, UINT_32 NW_Y, UINT_32 nQuadWidth )
{
	QuadNodeStruc *pNode = &(m_aQuadPool[nNode]);

	UINT_32 radius = nQuadWidth >> 1;

	if ( pNode->nChildren )
	{
		// ---------------------------------------------------------
		// RENDER CHILD NODES
		// ---------------------------------------------------------
		RecursRender( pNode->nChildren + CHILD_NW, NW_X,			NW_Y,			radius);
		RecursRender( pNode->nChildren + CHILD_NE, NW_X + radius,	NW_Y,			radius);
		RecursRender( pNode->nChildren + CHILD_SW, NW_X,			NW_Y + radius,	radius);
		RecursRender( pNode->nChildren + CHILD_SE, NW_X + radius,	NW_Y + radius,	radius);
	}
	else
	{
		// ---------------------------------------------------------
		// LOAD THE LOCAL HEIGHTS
		// ---------------------------------------------------------
		float nNorthWest = HeightField(((NW_Y + (0))			), (NW_X + (0)) );
		float nNorthEast = HeightField(((NW_Y + (0))			), (NW_X + (nQuadWidth)) );
		float nSouthWest = HeightField(((NW_Y + (nQuadWidth))	), (NW_X + (0)) );
		float nSouthEast = HeightField(((NW_Y + (nQuadWidth))	), (NW_X + (nQuadWidth)) );

		float nCenter	= HeightField(((NW_Y + (radius))		), (NW_X + (radius)) );

		// ---------------------------------------------------------
		// DRAW ONE TRIANGLE FAN : from Four to Eight Triangles
		// ---------------------------------------------------------
		glBegin(GL_TRIANGLE_FAN);

		// ---------------------------------------------------------------------------------------
#define MAKE_BLEND_FLOAT(a) ((float)(a) / 255.0f)

		float fBlend = MAKE_BLEND_FLOAT(pNode->nBlend);		// Convert blend from byte back into float.

		// Center Point
		MakeBlendedVertex(	radius, radius, nCenter,
							(nNorthEast + nSouthWest + nNorthWest + nSouthEast)/4, fBlend, NW_X, NW_Y );

		// Other Points (counter-clockwise)
		m_iDrawnTriangles++;
		MakeVertex( 0, 0, nNorthWest, NW_X, NW_Y );

		if ( pNode->nWest )
		{
			m_iDrawnTriangles++;
			if ( m_aQuadPool[pNode->nWest].nChildren == 0 )		// Any child will do...
				MakeBlendedVertex(	0, radius, HeightField(((NW_Y + (radius))),(NW_X + (0)) ),
								(nNorthWest + nSouthWest)/2, (fBlend + MAKE_BLEND_FLOAT(m_aQuadPool[pNode->nWest].nBlend)) / 2, NW_X, NW_Y );
			else
				MakeVertex( 0, radius, HeightField(((NW_Y + (radius))),(NW_X + (0)) ) , NW_X, NW_Y );
		}

		m_iDrawnTriangles++;
		MakeVertex( 0, nQuadWidth, nSouthWest, NW_X, NW_Y );

		if ( pNode->nSouth )
		{
			m_iDrawnTriangles++;
			if ( m_aQuadPool[pNode->nSouth].nChildren == 0 )		// Any child will do...
				MakeBlendedVertex(	radius, nQuadWidth, HeightField(((NW_Y + (nQuadWidth))),(NW_X + (radius)) ),
								(nSouthWest + nSouthEast)/2, (fBlend + MAKE_BLEND_FLOAT(m_aQuadPool[pNode->nSouth].nBlend)) / 2, NW_X, NW_Y );
			else
				MakeVertex( radius, nQuadWidth, HeightField(((NW_Y + (nQuadWidth))),(NW_X + (radius)) ) , NW_X, NW_Y );
		}

		m_iDrawnTriangles++;
		MakeVertex( nQuadWidth, nQuadWidth, nSouthEast, NW_X, NW_Y );

		if ( pNode->nEast )
		{
			m_iDrawnTriangles++;
			if ( m_aQuadPool[pNode->nEast].nChildren == 0 )		// Any child will do...
				MakeBlendedVertex(	nQuadWidth, radius, HeightField(((NW_Y + (radius))),(NW_X + (nQuadWidth)) ),
								(nNorthEast + nSouthEast)/2, (fBlend + MAKE_BLEND_FLOAT(m_aQuadPool[pNode->nEast].nBlend)) / 2, NW_X, NW_Y );
			else
				MakeVertex( nQuadWidth, radius, HeightField(((NW_Y + (radius))),(NW_X + (nQuadWidth)) ) , NW_X, NW_Y );
		}

		m_iDrawnTriangles++;
		MakeVertex( nQuadWidth, 0, nNorthEast, NW_X, NW_Y );

		if ( pNode->nNorth )
		{
			m_iDrawnTriangles++;
			if ( m_aQuadPool[pNode->nNorth].nChildren == 0 )		// Any child will do...
				MakeBlendedVertex(	radius, 0, HeightField(((NW_Y + (0))),(NW_X + (radius)) ),
								(nNorthEast + nNorthWest)/2, (fBlend + MAKE_BLEND_FLOAT(m_aQuadPool[pNode->nNorth].nBlend)) / 2, NW_X, NW_Y );
			else
				MakeVertex( radius, 0, HeightField(((NW_Y + (0))),(NW_X + (radius)) ) , NW_X, NW_Y );
		}
		// ---------------------------------------------------------------------------------------

		MakeVertex( 0, 0, nNorthWest, NW_X, NW_Y );
		glEnd();
	}
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

