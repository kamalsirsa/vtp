//
// Name:	 Contours.cpp
// Purpose:  Contour-related code, which interfaces vtlib to the
//	QuikGrid library.
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "surfgrid.h"
#include "contour.h"

#include "Contours.h"

#ifdef _MSC_VER
#pragma comment( lib, "QuikGrid.lib" )
#endif

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

void ContourConverter::Setup(vtTerrain *pTerr, const RGBf &color)
{
	// Make a note of this terrain and its attributes
	m_pTerrain = pTerr;
	m_pHF = pTerr->GetHeightFieldGrid3d();
	m_ext = m_pHF->GetEarthExtents();
	m_spacing = m_pHF->GetSpacing();

	// Create material and geometry to contain the vector geometry
	vtMaterialArray *pMats = new vtMaterialArray();
	pMats->AddRGBMaterial1(color, false, false, true);
	m_pGeom = new vtGeom();
	m_pGeom->SetName2("Contour Lines");
	m_pGeom->SetMaterials(pMats);
	pMats->Release();		// pass ownership

	// copy data from our grid to a QuikGrid object
	int nx, ny;
	m_pHF->GetDimensions(nx, ny);
	m_pGrid = new SurfaceGrid(nx,ny);
	int i, j;
	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		{
			m_pGrid->zset(i, j, m_pHF->GetElevation(i, j));
		}
	}

	// Since we have to interface to a global callback, set a global
	//  pointer to the recipient of the callback.
	s_cc = this;
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

void ContourConverter::GenerateContour(float fAlt)
{
	NewMesh();
	Contour(*m_pGrid, fAlt);
}

void ContourConverter::GenerateContours(float fInterval)
{
	float fMin, fMax;
	m_pHF->GetHeightExtents(fMin, fMax);

	int start = (int) (fMin / fInterval) + 1;
	int stop = (int) (fMax / fInterval);

	for (int i = start; i < stop; i++)
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

void ContourConverter::Finish()
{
	Flush();

	if (m_pMesh)
	{
		m_pGeom->AddMesh(m_pMesh, 0);
		m_pMesh->Release();		// pass ownership
	}

	m_pTerrain->AddNode(m_pGeom);

	s_cc = NULL;
}

void ContourConverter::Flush()
{
	if (m_line.GetSize() > 2)
		m_pTerrain->AddSurfaceLineToMesh(m_pMesh, m_line, 10, false);
	m_line.Empty();
}

