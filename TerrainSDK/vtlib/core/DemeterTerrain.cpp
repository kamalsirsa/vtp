//
// DemeterTerrain class : an implementation of vtDynTerrainGeom which
// encapsulates support for the 'DemeterTerrain' terrain engine.
//
// In Development.  Not yet functional!  Do not use.
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#if 0	// disabled until its working

#include "DemeterTerrain.h"
#include "Demeter/Terrain.h"
#include "Demeter/DemeterDrawable.h"

using namespace Demeter;
#ifdef _MSC_VER
#pragma comment( lib, "DemeterDebug.lib" )
#endif

//
// Macros used to generate vertex locations from a heightfield index
// (You don't have to use them, especially for height)
//
#define offset(x, y)  ((y) * m_iColumns + (x))
#define MAKE_XYZ1(x,y) m_fXLookup[(x)], m_pData[offset((x),(y))]*m_fZScale, m_fZLookup[(y)]
#define MAKE_XYZ1_TRUE(x,y) m_fXLookup[(x)], m_pData[offset((x),(y))], m_fZLookup[(y)]


//
// Constructor/destructor
//
DemeterTerrain::DemeterTerrain() : vtDynTerrainGeom()
{
	m_pData = NULL;
}

DemeterTerrain::~DemeterTerrain()
{
	if (m_pData)
		delete m_pData;
}


//
// Initialize the terrain data
// fZScale converts from height values (meters) to world coordinates
//
DTErr DemeterTerrain::Init(const vtElevationGrid *pGrid, float fZScale)
{
	// Initializes necessary field of the parent class
	DTErr err = BasicInit(pGrid);
	if (err != DTErr_OK)
		return err;

	Settings::GetInstance()->SetVerbose(true);

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

	try
	{
		// Chosen based on the expected number of triangles that will be
		// visible on-screen at any one time (the terrain mesh will typically
		// have far more triangles than are seen at one time, especially with
		// dynamic tessellation)
		const int MAX_NUM_VISIBLE_TRIANGLES = 50000;
		m_pTerrain = new Terrain;
		const int width = m_iColumns;
		const int height = m_iRows;

		m_pTerrain->SetAllElevations(m_pData,width,height,20.0f);

		m_pDrawable = new DemeterDrawable;
		m_pDrawable->SetTerrain(m_pTerrain);
	}
	catch (DemeterException* pEx)
	{
		cout << pEx->GetErrorMessage() << endl;
		exit(0);
	}
	catch (...)
	{
		// "VIEWER: Unexpected exception while creating terrain"
		return false;
	}

	m_fZScale = fZScale;
	m_iDrawnTriangles = -1;

	return DTErr_OK;
}


//
// This will be called once per frame, during the culling pass.
//
void DemeterTerrain::DoCulling(const vtCamera *pCam)
{
	IPoint2 window_size = vtGetScene()->GetWindowSize();
	Settings::GetInstance()->SetScreenWidth(window_size.x);
	Settings::GetInstance()->SetScreenHeight(window_size.y);

	// Do your visibility testing here.
	// (Compute which detail will actually gets drawn)
}


void DemeterTerrain::DoRender()
{
	float threshold = 6.0f; // The "detail level" of the terrain - higher
	// values will render faster but yield less visual quality

	m_pTerrain->SetMaximumVisibleBlockSize(64);
	m_pTerrain->SetDetailThreshold(threshold);
	m_pTerrain->ModelViewMatrixChanged();

	// Prepare the render state for our OpenGL usage
	PreRender();

	// Render the triangles
	RenderSurface();

	// Clean up
	PostRender();
}


void DemeterTerrain::LoadSingleMaterial()
{
	// single texture for the whole terrain
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		ApplyMaterial(pMat);
		SetupTexGen(1.0f);
	}
}


void DemeterTerrain::RenderSurface()
{
	LoadSingleMaterial();
	RenderPass();

	// here is an example of how to do a second rendering pass
	if (m_bDetailTexture)
	{
		// once again, with the detail texture material
		ApplyMaterial(m_pDetailMat);

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


void DemeterTerrain::RenderPass()
{
	m_pTerrain->Render();
}

//
// GetLocation is called when the framework needs to know the surface
// position of the terrain at a given grid point.  Supply the height
// value from your own data structures.
//
void DemeterTerrain::GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue) const
{
	if (bTrue)
		p.Set(MAKE_XYZ1_TRUE(iX, iZ));
	else
		p.Set(MAKE_XYZ1(iX, iZ));
}

#endif

