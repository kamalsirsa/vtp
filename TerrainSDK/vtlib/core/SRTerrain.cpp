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
#include "LocalGrid.h"

#if ENABLE_SRTERRAIN
  using namespace mini;
  #ifdef _MSC_VER
    #pragma comment( lib, "libMini.lib" )
  #endif
#endif

/////////////////////////////////////////////////////////////////////////////

//
// Constructor/destructor
//
SRTerrain::SRTerrain() : vtDynTerrainGeom()
{
	m_fResolution = 10000.0f;
}

SRTerrain::~SRTerrain()
{
}

/////////////////////////////////////////////////////////////////////////////

//
// Unfortunately the following statics are required because libMini only
// supports some functionality by callback, and the callback is only a
// simple C function which has no context to tell us which terrain.
//
static vtLocalGrid *s_pGrid;
static SRTerrain *s_pSRTerrain;
static int myfancnt, myvtxcnt;

void beginfan_vtp()
{
	if (myfancnt++>0)
		glEnd();
	glBegin(GL_TRIANGLE_FAN);
	myvtxcnt-=2;	// 2 vertices are needed to start each fan
}

void fanvertex_vtp(float x, float y, float z)
{
   glVertex3f(x,y,z);
   myvtxcnt++;
}

void notify_vtp(int i, int j, int size)
{
	// check to see if we need to switch texture
	if (s_pSRTerrain->m_iTPatchDim > 1 && size == s_pSRTerrain->m_iBlockSize)
	{
		int a = i / size;
		int b = j / size;
		s_pSRTerrain->LoadBlockMaterial(a, b);
	}
}

short int getelevation_vtp1(int i, int j, int size)
{
	return s_pGrid->GetValue(i, j);
}

float getelevation_vtp2(int i, int j, int size)
{
	return s_pGrid->GetFValue(i, j);
}

//
// Initialize the terrain data
// fZScale converts from height values (meters) to world coordinates
//
bool SRTerrain::Init(vtLocalGrid *pGrid, float fZScale, 
					 float fOceanDepth, int &iError)
{
#if ENABLE_SRTERRAIN
	// Initializes necessary field of the parent class
	BasicInit(pGrid);

	m_fHeightScale = fZScale;

	int size = m_iXPoints;
	float dim = m_fXStep;
	float cellaspect = m_fZStep / m_fXStep;

	s_pGrid = pGrid;

	if (pGrid->IsFloatMode())
	{
		float *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, fZScale, cellaspect,
				beginfan_vtp, fanvertex_vtp, notify_vtp,
				getelevation_vtp2);
	}
	else
	{
		short *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, fZScale, cellaspect,
				beginfan_vtp, fanvertex_vtp, notify_vtp,
				getelevation_vtp1);
	}

	m_iDrawnTriangles = -1;
	m_iBlockSize = m_iXPoints / 4;
#endif
	return true;
}


//
// This will be called once per frame, during the culling pass.
//
// However, libMini does not allow you to call the culling pass
// independently of the rendering pass, so we cannot do it here.
//
void SRTerrain::DoCulling(FPoint3 &eyepos_ogl, IPoint2 window_size, float fov)
{
	// store for later
	m_eyepos_ogl = eyepos_ogl;
	m_window_size = window_size;
	m_fFOVY = fov * window_size.y / window_size.x * 180 / PIf;
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


void SRTerrain::LoadBlockMaterial(int a, int b)
{
	// we can't change the texture between glBegin/glEnd
	if (myfancnt++>0)
		glEnd();

	// each block has it's own texture map
	int appidx = a*m_iTPatchDim + (m_iTPatchDim-1-b);
	vtMaterial *pMat = GetMaterial(appidx);
	if (pMat)
	{
		pMat->Apply();
		SetupBlockTexGen(a, b);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBegin(GL_TRIANGLE_FAN);
}


void SRTerrain::RenderSurface()
{
	s_pSRTerrain = this;

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
#if ENABLE_SRTERRAIN
	// grab necessary values from the VTP Scene framework
	vtScene *pScene = vtGetScene();
	vtCamera *pCamera = pScene->GetCamera();
	float nearp = pCamera->GetHither();
	float farp = pCamera->GetYon();

	float aspect = (float)m_window_size.x / m_window_size.y;

	float ex = m_eyepos_ogl.x;
	float ey = m_eyepos_ogl.y;
	float ez = m_eyepos_ogl.z;

	// Get up vector and direction vector from camera matrix
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

	myfancnt = myvtxcnt = 0;
	int size = m_iXPoints;

	// Convert to the strange origin-centered coordinate scheme of libMini.
	// This is what works best so far:
	ex-=(m_iXPoints/2)*m_fXStep;
	ez+=(m_iYPoints/2)*m_fZStep;

	m_pMini->draw(m_fResolution, 
				ex, ey, ez, 
				dx, dy, dz, 
				ux, uy, uz, 
				m_fFOVY * 1.05, aspect,		// exaggerate by 5%
				nearp, farp);

	if (myfancnt>0) glEnd();

	// We are drawing fans, so the number of triangles is roughly equal to
	// number of vertices
	m_iDrawnTriangles = myvtxcnt;

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
	if (m_fResolution > 4E7)
		m_fResolution = 4E7;
#endif
}

//
// GetLocation is called when the framework needs to know the surface
// position of the terrain at a given grid point.  Supply the height
// value from our own data structures.
//
void SRTerrain::GetLocation(int i, int j, FPoint3 &p)
{
#if ENABLE_SRTERRAIN
	p.Set(m_fXLookup[i],
		  m_pMini->getheight(i, j),
		  m_fZLookup[j]);
#endif
}

