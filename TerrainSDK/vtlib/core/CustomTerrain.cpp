//
// CustomTerrain class : an generic implementation of vtDynTerrainGeom
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "CustomTerrain.h"

//
// Macro used to determine the index of a vertex (element of the height
// field) given it's (x,y) location in the grid
//
#define offset(x, y)  ((y) * m_iColumns + (x))

//
// Macros used to generate vertex locations from a heightfield index
// (You don't have to use them, especially for height)
//
#define MAKE_XYZ1(x,y) m_fXLookup[(x)], m_pData[offset((x),(y))]*m_fZScale, m_fZLookup[(y)]
#define MAKE_XYZ2(x,y) (float)(x), m_pData[offset((x),(y))], (float)(y)


//
// Constructor/destructor
//
CustomTerrain::CustomTerrain() : vtDynTerrainGeom()
{
	m_pData = NULL;
}

CustomTerrain::~CustomTerrain()
{
	if (m_pData)
		delete m_pData;
}


//
// Initialize the terrain data
// fZScale converts from height values (meters) to world coordinates
//
DTErr CustomTerrain::Init(vtElevationGrid *pGrid, float fZScale,
					 float fOceanDepth)
{
	// Initializes necessary field of the parent class
	BasicInit(pGrid);

	//
	// allocate array, copy data from supplied elevation grid
	//
	// (replace this with your own storage representation)
	//
	m_pData = new float[m_iColumns * m_iRows];
	int i, j;
	for (i = 0; i < m_iColumns; i++)
	{
		for (j = 0; j < m_iRows; j++)
			m_pData[offset(i,j)] = pGrid->GetFValue(i, j);
	}

	m_fZScale = fZScale;
	m_iDrawnTriangles = -1;

	return DTErr_OK;
}


//
// This will be called once per frame, during the culling pass.
//
void CustomTerrain::DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov)
{
	// Do your visibility testing here.
	// (Compute which detail will actually gets drawn)

	// Here are some handy methods to test against the view frustum:
#if 0
	// Tests a sphere or triangle, and return one of:
	//	0				- not in view
	//  VT_Visible		- partly in view
	//  VT_AllVisible	- entirely in view
	//
	int IsVisible(const FSphere &sphere) const;
	int IsVisible(const FPoint3 &point0,
				  const FPoint3 &point1,
				  const FPoint3 &point2,
				  const float fTolerance = 0.0f) const;
	int IsVisible(FPoint3 point, float radius);

	// Tests a single point, returns true if in view
	bool IsVisible(const FPoint3 &point) const;
#endif
}


void CustomTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	// Render the triangles
	RenderSurface();

	// Clean up
	PostRender();
}


void CustomTerrain::LoadSingleMaterial()
{
	// single texture for the whole terrain
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		pMat->Apply();
		SetupTexGen(1.0f);
	}
}


void CustomTerrain::RenderSurface()
{
	LoadSingleMaterial();
	RenderPass();

	// here is an example of how to do a second rendering pass
	if (m_bDetailTexture)
	{
		// once again, with the detail texture material
		m_pDetailMat->Apply();

		// the uv tiling is different (usually highly repetitive)
		SetupTexGen(m_fDetailTiling);

		// draw the second pass
		glPolygonOffset(-1.0f, -1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		RenderPass();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	DisableTexGen();
}


void CustomTerrain::RenderPass()
{
	//
	// Very naive code which draws the grid as immediate-mode
	// triangle strips.  (Replace with your own algorithm.)
	//
	int i, j;
	for (i = 0; i < m_iColumns-3; i+=2)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < m_iRows; j+=2)
		{
			glVertex3f(MAKE_XYZ2(i, j));
			glVertex3f(MAKE_XYZ2(i+2, j));
			m_iDrawnTriangles += 2;
		}
		glEnd();
	}
}

//
// These methods are called when the framework needs to know the surface
// position of the terrain at a given grid point.  Supply the height
// value from your own data structures.
//
float CustomTerrain::GetElevation(int iX, int iZ) const
{
	return m_pData[offset(iX,iZ)];
}

void CustomTerrain::GetWorldLocation(int iX, int iZ, FPoint3 &p) const
{
	p.Set(MAKE_XYZ1(iX, iZ));
}


