//
// Name:	 Contours.cpp
// Purpose:  Contour-related code, which interfaces vtlib to the
//	QuikGrid library.
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "QuikGrid/surfgrid.h"
#include "QuikGrid/contour.h"

#include "Contours.h"

static ContourConverter *s_cc = NULL;

//
// This globally-scoped method is found at link time by the QuikGrid
//  library, which uses it as a callback.
//
void DoLineTo( float x, float y, int drawtype )
{
	if (s_cc)
		s_cc->Coord(x, y, drawtype == 0);
}


/////////////////////////////////////////////////////////////////////////////
// class ContourConverter

ContourConverter::ContourConverter()
{
	m_pMesh = NULL;
	m_pGrid = NULL;
}

ContourConverter::~ContourConverter()
{
	delete m_pGrid;
}

/** 
 * Set up the class to do draping on a terrain.
 *
 * \param pTerr The terrain you will generate the contour lines on.
 * \param color The colors of the generated lines.
 * \param fHeight The height above the terrain to drape the lines.  Generally
 *		you will want to use a small offset value here, to keep the lines from
 *		colliding with the terrain itself.
 * \return A transform object which sits above the contours.  This is useful
 *		in case you later want to stretch or scale the contours, such as
 *		keeping them on the surface of an exaggerated terrain.
 */
vtTransform *ContourConverter::Setup(vtTerrain *pTerr, const RGBf &color, float fHeight)
{
	if (!pTerr)
		return NULL;

	// Make a note of this terrain and its attributes
	m_pTerrain = pTerr;
	m_pHF = pTerr->GetHeightFieldGrid3d();
	m_ext = m_pHF->GetEarthExtents();
	m_spacing = m_pHF->GetSpacing();
	m_fHeight = fHeight;

	// Create material and geometry to contain the vector geometry
	vtMaterialArray *pMats = new vtMaterialArray();
	pMats->AddRGBMaterial1(color, false, false, true);

	m_pTrans = new vtTransform;
	m_pGeom = new vtGeom();
	m_pGeom->SetName2("Contour Lines");
	m_pGeom->SetMaterials(pMats);
	pMats->Release();		// pass ownership
	m_pTrans->AddChild(m_pGeom);

	// copy data from our grid to a QuikGrid object
	int nx, ny;
	m_pHF->GetDimensions(nx, ny);
	m_pGrid = new SurfaceGrid(nx,ny);
	int i, j;
	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		{
			// use the true elevation, for true contours
			m_pGrid->zset(i, j, m_pHF->GetElevation(i, j, true));
		}
	}

	// Since we have to interface to a global callback, set a global
	//  pointer to the recipient of the callback.
	s_cc = this;

	return m_pTrans;
}

void ContourConverter::NewMesh()
{
	if (m_pMesh)
	{
		m_pGeom->AddMesh(m_pMesh, 0);
		m_pMesh->Release();		// pass ownership
	}
	m_pMesh = new vtMesh(GL_LINE_STRIP, 0, 2000);
}

/**
 * Generate a contour line to be draped on the terrain.
 *
 * \param fAlt The altitude (elevation) of the line to be generated.
 */
void ContourConverter::GenerateContour(float fAlt)
{
	NewMesh();
	Contour(*m_pGrid, fAlt);
}

/**
 * Generate a set of contour lines to be draped on the terrain.
 *
 * \param fInterval  The vertical spacing between the contours.  For example,
 *		if the elevation range of your data is from 50 to 350 meters, then
 *		an fIterval of 100 will place contour bands at 100,200,300 meters.
 */
void ContourConverter::GenerateContours(float fInterval)
{
	float fMin, fMax;
	m_pHF->GetHeightExtents(fMin, fMax);

	int start = (int) (fMin / fInterval) + 1;
	int stop = (int) (fMax / fInterval);

	for (int i = start; i <= stop; i++)
	{
		NewMesh();
		Contour(*m_pGrid, i * fInterval);
	}
}

void ContourConverter::Coord(float x, float y, bool bStart)
{
	if (bStart)
		Flush();

	DPoint2 p2;
	p2.x = m_ext.left + x * m_spacing.x;
	p2.y = m_ext.bottom + y * m_spacing.y;
	m_line.Append(p2);
}

/**
 * Finishes the contour generation process.  Call once when you are done
 * using the class to generate contours.
 */
void ContourConverter::Finish()
{
	Flush();

	if (m_pMesh)
	{
		m_pGeom->AddMesh(m_pMesh, 0);
		m_pMesh->Release();		// pass ownership
	}

	m_pTerrain->AddNode(m_pTrans);
	float fExag = m_pTerrain->GetVerticalExag();
	m_pTrans->Scale3(1, fExag, 1);

	s_cc = NULL;
}

void ContourConverter::Flush()
{
	if (m_line.GetSize() > 2)
	{
		m_pTerrain->AddSurfaceLineToMesh(m_pMesh, m_line, m_fHeight,
			false, true);	// use true elevation, not scaled
	}
	m_line.Empty();
}


