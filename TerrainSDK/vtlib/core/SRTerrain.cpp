//
// SRTerrain class : a subclass of vtDynTerrainGeom which encapsulates
//  Stefan Roettger's CLOD algorithm.
//
// utilizes: Roettger's MINI library implementation
// http://wwwvis.informatik.uni-stuttgart.de/~roettger
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SRTerrain.h"
#include "vtdata/vtLog.h"

#if VTLIB_NI
// stub
SRTerrain::SRTerrain() {}
SRTerrain::~SRTerrain() {}
float SRTerrain::GetElevation(int iX, int iZ, bool bTrue) const { return 0; }
void SRTerrain::GetWorldLocation(int i, int j, FPoint3 &p, bool bTrue) const {}
void SRTerrain::DoCulling(const vtCamera *pCam) {}
void SRTerrain::DoRender() {}
void SRTerrain::SetVerticalExag(float fExag) {}
DTErr SRTerrain::Init(const vtElevationGrid *pGrid, float fZScale) { return DTErr_NOMEM; }
#else

#include "mini.h"
#include "ministub.hpp"

using namespace mini;
#ifdef _MSC_VER
#pragma comment( lib, "libMini.lib" )
#endif

/////////////////////////////////////////////////////////////////////////////

//
// Constructor/destructor
//
SRTerrain::SRTerrain() : vtDynTerrainGeom()
{
	m_fResolution = 10000.0f;
	m_pMini = NULL;
	m_iLastDiff = 0;
	m_fDamping = 0;
}

SRTerrain::~SRTerrain()
{
	if (m_pMini)
	{
		delete m_pMini;
		m_pMini = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////

//
// Unfortunately the following statics are required because libMini only
// supports some functionality by callback, and the callback is only a
// simple C function which has no context to tell us which terrain.
//
static const vtElevationGrid *s_pGrid;
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
DTErr SRTerrain::Init(const vtElevationGrid *pGrid, float fZScale)
{
	// Initializes necessary field of the parent class
	DTErr err = BasicInit(pGrid);
	if (err != DTErr_OK)
		return err;

	if (m_iColumns != m_iRows)
		return DTErr_NOTSQUARE;

	// compute n (log2 of grid size)
	// ensure that the grid is size (1 << n) + 1
	int n = vt_log2(m_iColumns - 1);
	int required_size = (1<<n) + 1;
	if (m_iColumns != required_size || m_iRows != required_size)
		return DTErr_NOTPOWER2;

	int size = m_iColumns;
	float dim = m_fXStep;
	float cellaspect = m_fZStep / m_fXStep;

	s_pGrid = pGrid;

#if 0
	// NOTE: the following workaround is no longer needed with libMini 5.02
	//
	// Totally strange but repeatable behavior: runtime exit in Release-mode
	//  libMini with values over a certain level.  Workaround here!
	// Exit happens if (scale * maximum_height / dim > 967)
	//
	// 	scale * maximum_height / dim > 967
	//	scale > dim * 967 / maximum_height
	//
	float fMin, fMax;
	pGrid->GetHeightExtents(fMin, fMax);
	// Avoid trouble with fMax small or zero
	if (fMax < 10) fMax = 10;
	// compute the largest supported value for MaxScale
	float fMaxMax = dim * 960 / fMax;
	// values greater than 10 are unnecessarily large
	if (fMaxMax > 10)
		fMaxMax = 10;
	m_fMaximumScale = fMaxMax;
#else
	// This maxiumum scale is a reasonable tradeoff between the exaggeration
	//  that the user is likely to need, and numerical precision issues.
	m_fMaximumScale = 10;
#endif

	m_fHeightScale = fZScale;
	m_fDrawScale = m_fHeightScale / m_fMaximumScale;

	if (pGrid->IsFloatMode())
	{
		float *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, m_fMaximumScale, cellaspect,
				beginfan_vtp, fanvertex_vtp, notify_vtp,
				getelevation_vtp2);
	}
	else
	{
		short *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, m_fMaximumScale, cellaspect,
				beginfan_vtp, fanvertex_vtp, notify_vtp,
				getelevation_vtp1);
	}

	m_iDrawnTriangles = -1;
	m_iBlockSize = m_iColumns / 4;

	return DTErr_OK;
}

void SRTerrain::SetVerticalExag(float fExag)
{
	m_fHeightScale = fExag;

	// safety check
	if (m_fHeightScale > m_fMaximumScale)
		m_fHeightScale = m_fMaximumScale;

	m_fDrawScale = m_fHeightScale / m_fMaximumScale;
}


//
// This will be called once per frame, during the culling pass.
//
// However, libMini does not allow you to call the culling pass
// independently of the rendering pass, so we cannot do culling here.
// Instead, just store the values for later use.
//
void SRTerrain::DoCulling(const vtCamera *pCam)
{
	// Grab necessary values from the VTP Scene framework, store for later
	m_eyepos_ogl = pCam->GetTrans();
	m_window_size = vtGetScene()->GetWindowSize();
	m_fAspect = (float)m_window_size.x / m_window_size.y;
	m_fNear = pCam->GetHither();
	m_fFar = pCam->GetYon();

	// Get up vector and direction vector from camera matrix
	FMatrix4 mat;
	pCam->GetTransform1(mat);
	FPoint3 up(0.0f, 1.0f, 0.0f);
	mat.TransformVector(up, eye_up);

	FPoint3 forward(0.0f, 0.0f, -1.0f);
	mat.TransformVector(forward, eye_forward);

	if (pCam->IsOrtho())
	{
		// libMini supports orthographic viewing as of libMini 5.0.
		// A negative FOV value indicates to the library that the FOV is
		//  actually the orthographic height of the camera.
		m_fFOVY = pCam->GetWidth() / m_fAspect;
		m_fFOVY = -m_fFOVY;
	}
	else
	{
		float fov = pCam->GetFOV();
		float fov_y2 = atan(tan (fov/2) / m_fAspect);
		m_fFOVY = fov_y2 * 2.0f * 180 / PIf;
	}
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
	int matidx = a*m_iTPatchDim + (m_iTPatchDim-1-b);
	vtMaterial *pMat = GetMaterial(matidx);
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

	// how to do a second rendering pass
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


#define ADAPTION_SPEED	.0002f		// speed at which detail converges

void SRTerrain::RenderPass()
{
	float ex = m_eyepos_ogl.x;
	float ey = m_eyepos_ogl.y;
	float ez = m_eyepos_ogl.z;

	float fov = m_fFOVY;

	float ux = eye_up.x;
	float uy = eye_up.y;
	float uz = eye_up.z;

	float dx = eye_forward.x;
	float dy = eye_forward.y;
	float dz = eye_forward.z;

	myfancnt = myvtxcnt = 0;

	// Convert the eye location to the unusual coordinate scheme of libMini.
	ex -= (m_iColumns/2)*m_fXStep;
	ez += (m_iRows/2)*m_fZStep;

	m_pMini->draw(m_fResolution,
				ex, ey, ez,
				dx, dy, dz,
				ux, uy, uz,
				fov, m_fAspect,
				m_fNear, m_fFar,
				m_fDrawScale);

	if (myfancnt>0) glEnd();

	// We are drawing fans, so the number of triangles is roughly equal to
	// number of vertices
	m_iDrawnTriangles = myvtxcnt;

	// adaptively adjust resolution threshold up or down to attain
	// the desired polygon (vertex) count target
	int diff = m_iDrawnTriangles - m_iPolygonTarget;
	int iRange = m_iPolygonTarget / 10;		// ensure within 10%
	float adjust = 1.0f;

	if (diff < -iRange || diff > iRange)
	{
		// the amount we change resolution is proportional to how much we
		//  missed our triangle count target
		float change_by = -diff * ADAPTION_SPEED;

		// avoid hysteresis by using additional damping on direction change
		if ((m_iLastDiff > 0 && diff < 0) ||
			(m_iLastDiff < 0 && diff > 0))
		{
//			VTLOG("Changing dir, diff %d, previous %d\n", diff, previous_diff);
			m_fDamping = -0.9;	// 90% damping
		}
		change_by *= (1.0 + m_fDamping);

		// exponential response factor
		adjust = pow(2.0, (double)change_by);

		// clamp, to avoid extreme cases
		if (adjust > 100.0f) adjust = 100.0f;
		if (adjust < 0.01f) adjust = 0.01f;
	}
	m_iLastDiff = diff;	// remember for next frame
	m_fDamping /= 2;	// damping falls off rapidly

	m_fResolution *= adjust;

	// keep the error within reasonable bounds
	if (m_fResolution < 1.0f)
		m_fResolution = 1.0f;
	if (m_fResolution > 4E7)
		m_fResolution = 4E7;
}

//
// These methods are called when the framework needs to know the surface
// position of the terrain at a given grid point.  Supply the height
// value from our own data structures.
//
float SRTerrain::GetElevation(int iX, int iZ, bool bTrue) const
{
	float height = m_pMini->getheight(iX, iZ);

	if (bTrue)
		// convert stored value to true value
		return height / m_fMaximumScale;
	else
		// convert stored value to drawn value
		return height * m_fDrawScale;
}

void SRTerrain::GetWorldLocation(int i, int j, FPoint3 &p, bool bTrue) const
{
	float height = m_pMini->getheight(i, j);

	if (bTrue)
		// convert stored value to true value
		height /= m_fMaximumScale;
	else
		// convert stored value to drawn value
		height *= m_fDrawScale;

	p.Set(m_fXLookup[i],
		  height,
		  m_fZLookup[j]);
}

#endif	// VTLIB_NI
