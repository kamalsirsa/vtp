//
// SRTerrain class : a subclass of vtDynTerrainGeom which exposes
//  Stephan Roettger's CLOD algorithm.
//
// utilizes: Roettger's MINI library implementation, version 1.7
// http://wwwvis.informatik.uni-stuttgart.de/~roettger
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SRTerrain.h"

using namespace mini;

/////////////////////////////////////////////////////////////////////////////

//
// Constructor/destructor
//
SRTerrain::SRTerrain() : vtDynTerrainGeom()
{
	m_fResolution = 1000.0f;
}

SRTerrain::~SRTerrain()
{
}

/////////////////////////////////////////////////////////////////////////////

namespace mini
{

void MiniMod::Initialize(vtLocalGrid *pGrid, float fOceanDepth)
{
	int i, j;
	float elev;
	for (i = 0; i<S; i++)
	{
		for (j = 0; j<S; j++)
		{
			elev = pGrid->GetFValue(i, j);
			if (elev == 0.0f)
				elev = fOceanDepth;
			y[i][j] = elev;
		}
	}

}

} // namespace mini

//
// Initialize the terrain data
// fZScale converts from height values (meters) to world coordinates
//
bool SRTerrain::Init(vtLocalGrid *pGrid, float fZScale, 
					 float fOceanDepth, int &iError)
{
	// Initializes necessary field of the parent class
	BasicInit(pGrid);

	m_fHeightScale = fZScale;

	void GetDimensions(int &nColumns, int &nRows);
	int size = m_iXPoints;
	float dim = m_fXStep;
	float cellaspect = m_fZStep / m_fXStep;

	m_terrain.initmap1(&size, &dim, fZScale, cellaspect);

	//
	// copy data from supplied elevation grid
	//
	m_terrain.Initialize(pGrid, fOceanDepth);

	void *map, *d2map;

	map = m_terrain.initmap3(&d2map, &size, &dim);

	m_iDrawnTriangles = -1;
	return true;
}


//
// This will be called once per frame, during the culling pass.
//
// Visibility testing here.
// (Compute which detail will actually gets drawn)
//
void SRTerrain::DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov)
{
}


void SRTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	// Render the triangles
	RenderSurface();

	// Clean up
	PostRender();
}


void SRTerrain::LoadSingleMaterial()
{
	// single texture for the whole terrain
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		pMat->Apply();
		SetupTexGen(1.0f);
	}
}


void SRTerrain::RenderSurface()
{
	if (m_iTPatchDim == 1)
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

#define ADAPTION_SPEED	.0001f	// speed at which detail converges

void SRTerrain::RenderPass()
{
	// grab necessary values from the VTP Scene framework
	vtScene *pScene = vtGetScene();
	vtCamera *pCamera = pScene->GetCamera();
	float nearp = pCamera->GetHither();
	float farp = pCamera->GetYon();
	float fovy = pCamera->GetFOV();	// in radians
	float fov = fovy * 180 / PIf;

	FPoint3 eyepos_ogl = pCamera->GetTrans();
	IPoint2 window_size = pScene->GetWindowSize();
	float aspect = (float)window_size.x / window_size.y;

	float ex = eyepos_ogl.x;
	float ey = eyepos_ogl.y;
	float ez = eyepos_ogl.z;

	// Focus point
	// Stefan says: "Normally this should be indeed the same as eye point.
	// I added the focus parameter to support the case of a fixed point
	// of interest which should be rendered with highest accuracy
	// independently from the actual eye point.  Up to now I did not need
	// this functionality, but one never knows, so just assume fx/y/z to
	// be equal to ex/y/z for now.
	float fx = ex;
	float fy = ey;
	float fz = ez;

	FMatrix4 mat;
	pCamera->GetTransform1(mat);
	FPoint3 up(0.0f, 1.0f, 0.0f), eye_up;
	mat.TransformVector(up, eye_up);
	float ux = eye_up.x;
	float uy = eye_up.y;
	float uz = eye_up.z;

	FPoint3 forward(0.0f, 0.0f, -1.0f), eye_forward;
	mat.TransformVector(forward, eye_forward);
	float dx = eye_forward.x;
	float dy = eye_forward.y;
	float dz = eye_forward.z;

	m_terrain.drawlandscape(m_fResolution, 
				ex, ey, ez, 
				fx, fy, fz, 
				dx, dy, dz, 
				ux, uy, uz, 
				fov, aspect, 
				nearp, farp);

	// We are drawing fans, so the number of triangles is roughly equal to
	// number of vertices
	m_iDrawnTriangles = m_terrain.vtxcnt;

	// adaptively adjust resolution threshold up or down to attain
	// the desired polygon (vertex) count target
	int diff = m_iDrawnTriangles - m_iPolygonTarget;
	int iRange = m_iPolygonTarget / 20;		// ensure within 5%
	float adjust = 1.0f;
	if (diff < -iRange) adjust = 1.0f + (-diff * ADAPTION_SPEED);
	if (diff > iRange) adjust = 1.0f + (-diff * ADAPTION_SPEED);

	m_fResolution  *=  adjust;

	// keep the error within reasonable bounds
	if (m_fResolution < 1.0f)
		m_fResolution = 1.0f;
	if (m_fResolution > 1E7)
		m_fResolution = 1E7;
}

//
// GetLocation is called when the framework needs to know the surface
// position of the terrain at a given grid point.  Supply the height
// value from our own data structures.
//
void SRTerrain::GetLocation(int i, int j, FPoint3 &p)
{
	p.Set(m_fXLookup[i], m_terrain.y[i][j]*m_fHeightScale, m_fZLookup[j]);
}

