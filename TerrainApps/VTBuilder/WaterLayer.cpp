//
// WaterLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "WaterLayer.h"
#include "ScaledView.h"
#include "Helper.h"

//////////////////////////////////////////////////////////////////////////

vtWaterLayer::vtWaterLayer() : vtLayer(LT_WATER)
{
	m_strFilename = "Untitled.hyd";
}

vtWaterLayer::~vtWaterLayer()
{
}


bool vtWaterLayer::OnSave()
{
	// unimplemented
	return true;
}

bool vtWaterLayer::OnLoad()
{
	// unimplemented
	return true;
}

bool vtWaterLayer::ConvertProjection(vtProjection &proj_new)
{
	// Create conversion object
	OCT *trans = OGRCreateCoordinateTransformation(&m_proj, &proj_new);
	if (!trans)
		return false;		// inconvertible projections

	int i, c, size, num_lines = m_Lines.GetSize();

	for (i = 0; i < num_lines; i++)
	{
		size = m_Lines[i]->GetSize();
		for (c = 0; c < size; c++)
			trans->Transform(1, &(m_Lines[i]->GetAt(c).x), &(m_Lines[i]->GetAt(c).y));
	}
	delete trans;
	return true;
}

#define MAXPOINTS 20000
static wxPoint roadbuf[MAXPOINTS];

void vtWaterLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	wxPen WaterPen(wxColor(0,40,160), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(WaterPen);

	int num_lines = m_Lines.GetSize();
	for (int i = 0; i < num_lines; i++)
	{
		for (int c = 0; c < m_Lines[i]->GetSize() && c < MAXPOINTS; c++)
			pView->screen(m_Lines[i]->GetAt(c), roadbuf[c]);

		pDC->DrawLines(m_Lines[i]->GetSize(), roadbuf);
	}
}

bool vtWaterLayer::GetExtent(DRECT &rect)
{
	int size = m_Lines.GetSize();
	if (size == 0)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (int i = 0; i < size; i++)
		rect.GrowToContainLine(*m_Lines[i]);
	return true;
}

void vtWaterLayer::AddElementsFromDLG(vtDLGFile *pDlg)
{
	// set projection
	m_proj = pDlg->GetProjection();

	m_Lines.SetSize(pDlg->m_iLines);
	for (int i = 0; i < pDlg->m_iLines; i++)
	{
		DLGLine *pDLine = pDlg->m_lines + i;
		DLine2 *new_line = new DLine2();
		new_line->SetSize(pDLine->m_iCoords);

		for (int j = 0; j < pDLine->m_iCoords; j++)
		{
			new_line->SetAt(j, pDLine->m_p[j]);
		}
		m_Lines.SetAt(i, new_line);
	}
}

void vtWaterLayer::AddElementsFromSHP(const char *filename, vtProjection &proj)
{
	//Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(filename, "rb");
	if (hSHP == NULL)
		return;

	//  Get number of polys (m_iNumPolys) and type of data (nShapeType)
	int		nElem;
	int		nShapeType;
    double	adfMinBound[4], adfMaxBound[4];
	FPoint2 point;
	SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

	//  Check Shape Type, Water Layer should be Poly or Line data
	if (nShapeType != SHPT_ARC && nShapeType != SHPT_POLYGON)
		return;

	m_proj = proj;	// Set projection

	// Initialize arrays
	m_Lines.SetSize(nElem);

	// Read Polys from SHP into Veg Poly
	for (int i = 0; i < nElem; i++)
	{
		// Get the i-th Poly in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		DLine2 *new_poly = new DLine2();
		new_poly->SetSize(psShape->nVertices);

		//Store each SHP Poly Coord in Veg Poly
		for (int j = 0; j < psShape->nVertices; j++)
		{
			new_poly->GetAt(j).x = psShape->padfX[j];
			new_poly->GetAt(j).y = psShape->padfY[j];
		}
		//Store the number of coordinate point in the i-th poly
		m_Lines.SetAt(i, new_poly);

		SHPDestroyObject(psShape);
	}
	SHPClose(hSHP);
}


bool vtWaterLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_WATER)
		return false;

	vtWaterLayer *pFrom = (vtWaterLayer *)pL;

	int from_size = pFrom->m_Lines.GetSize();
	for (int i = 0; i < from_size; i++)
		m_Lines.Append(pFrom->m_Lines[i]);

	pFrom->m_Lines.SetSize(0);
	return true;
}

void vtWaterLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtWaterLayer::Offset(const DPoint2 &p)
{
	int size = m_Lines.GetSize();
	for (int i = 0; i < size; i++)
	{
		for (int c = 0; c < m_Lines[i]->GetSize(); c++)
			m_Lines[i]->GetAt(c) += p;
	}
}
