//
// VegLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"
#include "ScaledView.h"
#include "VegLayer.h"
#include "Helper.h"

//////////////////////////////////////////////////////////////////////////

vtVegLayer::vtVegLayer() : vtLayer(LT_VEG)
{
//	m_strFilename = "Untitled.vf";
	m_VLType = VLT_Unknown;
}

vtVegLayer::~vtVegLayer()
{
}

bool vtVegLayer::GetExtent(DRECT &rect)
{
	if (!m_Poly.GetSize())
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	int size = m_Poly.GetSize();
	for (int i = 0; i < size; i++)
		rect.GrowToContainLine(*m_Poly.GetAt(i));

	return true;
}

#define MAXPOINTS 8000
static wxPoint vegbuf[MAXPOINTS];

void vtVegLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	int i, j, k;

	//set the pen options
	wxPen VegPen(wxColor(0,100,0), 1, PS_SOLID);  //single pixel solid green pen
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(VegPen);

#if 0
	for (i = 0; i < m_Poly.GetSize(); i++)
	{
		int vbuflength = 0;
		for (int c = 0; c < m_Poly[i].GetSize() && c < MAXPOINTS; c++)
		{
			pView->screen(m_Poly[i].GetAt(c), vegbuf[vbuflength]);
			vbuflength += 1;

			if (m_Poly[i].m_pbNoLine[c] == true)
			{
				if (vbuflength > 1) pDC->DrawLines(vbuflength, vegbuf);
				vbuflength = 0;
			}
		}
#else
	// draw each polygon in m_Poly
	bool pbNoLine[30000];

	int num_polys = m_Poly.GetSize();
	for (i = 0; i < num_polys; i++)
	{
		int vbuflength = 0;
		int num_points = m_Poly[i]->GetSize();
		for (j = 0; j < num_points; j++)
			pbNoLine[j] = false;

		bool cont = true;
		int a = num_points - 1, b = 0;

		while (cont)
		{
			pbNoLine[a] = true;
			for (k = b+1; k; k++)
			{
				if (a == k)
				{
					cont=false;
					break;
				}
				if (m_Poly[i]->GetAt(a) == m_Poly[i]->GetAt(k))
				{
					pbNoLine[k] = true;
					a -= 1;
					b = k+1;
					break;
				}
			}
		}
		for (int c = 0; c < num_points && c < MAXPOINTS; c++)
		{
			pView->screen(m_Poly[i]->GetAt(c), vegbuf[vbuflength]);
			vbuflength += 1;

			if (pbNoLine[c] == true || c == num_points-1)
			{
				if (vbuflength > 1)
					pDC->DrawLines(vbuflength, vegbuf);
				vbuflength = 0;
			}
		}
#endif
	}
}

void vtVegLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

bool vtVegLayer::OnSave()
{
	// unimplemented
	return true;
}

bool vtVegLayer::OnLoad()
{
	// unimplemented
	return true;
}

bool vtVegLayer::ConvertProjection(vtProjection &proj_new)
{
	// Create conversion object
	OCT *trans = OGRCreateCoordinateTransformation(&m_proj, &proj_new);
	if (!trans)
		return false;		// inconvertible projections

	int i, j;
	for (i = 0; i < m_Poly.GetSize(); i++)
	{
		for (j = 0; j < m_Poly[i]->GetSize(); j++)
		{
			trans->Transform(1, &(m_Poly[i]->GetAt(j).x), &(m_Poly[i]->GetAt(j).y));
		}
	}
	delete trans;
	return true;
}

void vtVegLayer::AppendDataFrom(vtLayer *pL)
{
//	vtVegLayer *pVL = dynamic_cast<vtVegLayer *>(pL);
	vtVegLayer *pVL = (vtVegLayer *)(pL);
	if (!pVL)
		return;

	// Must be of compatible types
	if (m_VLType != pVL->m_VLType)
		return;

	int size1 = m_Poly.GetSize();
	int size2 = pVL->m_Poly.GetSize();
//	m_Poly.SetSize(size1 + size2);

	int i;
	for (i = 0; i < size2; i++)
	{
		m_Poly.Append(pVL->m_Poly.GetAt(i));
	}

	// We've stolen all the polygons from the old layer, so empty it
	pVL->m_Poly.SetSize(0);
}



void vtVegLayer::AddElementsFromLULC(vtLULCFile *pLULC)
{
	LULCSection *section;
	LULCPoly *poly;

	//set projections
	m_proj.SetProjectionSimple(0, -1, WGS_84);

	m_VLType = VLT_Density;

	// figure out the number of polygons in file 
	int size = 0;
	for (int sec = 0; sec < pLULC->NumSections(); sec++)
	{
		section = pLULC->GetSection(sec);
		size = size + section->m_iNumPolys;
	}
	m_Poly.SetSize(size);
	m_pAttrib = new int[size];

// Not storing control points any more, used for testing.
/*	// get the 6 control points and convert them from latlon to utm
	for (int j = 0; j < 6; j++)
	{
		if (bConvertToUTM)
		{
			float cplon, cplat;
			cplon = pLULC->m_Corners[j].x;
			cplat = pLULC->m_Corners[j].y;
			(convert)
			m_VCtrlPts[j].x = (float) cpx;
			m_VCtrlPts[j].y = (float) cpy;
		}
		else
		{
			m_VCtrlPts[j].x = pLULC->m_Corners[j].x;
			m_VCtrlPts[j].y = pLULC->m_Corners[j].y;
		}
	}
*/

	// get each poly from LULC file
	int i, s, p, count = 0;
	for (s = 0; s < pLULC->NumSections(); s++)
	{
		section = pLULC->GetSection(s);
		for (p = 0; p < section->m_iNumPolys; p++)
		{
			poly = section->m_pPoly + p;
			m_pAttrib[count] = poly->Attribute;

			DLine2 *new_poly = new DLine2();
			new_poly->SetSize(poly->m_iCoords);

			// get Coords of LULCpoly and store as latlon, then save in VPoly
			for (i = 0; i < new_poly->GetSize(); i++)
				new_poly->SetAt(i, poly->m_p[i]);

			m_Poly.SetAt(count, new_poly);
			count++;
		}
	}
}


void vtVegLayer::AddElementsFromSHP(const char *filename, vtProjection &proj)
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

	//  Check Shape Type, Veg Layer should be Poly data
	if (nShapeType != SHPT_POLYGON)
		return;

	// Open DBF File & Get DBF Info:
	DBFHandle db = DBFOpen(filename, "rb");
	if (db == NULL)
		return;

	// Check for field of poly id, current default field in dbf is Id
	int iField = 0, *pnWidth = 0, *pnDecimals = 0;
	char *pszFieldName = NULL;

	DBFFieldType fieldtype = DBFGetFieldInfo(db, iField,
		pszFieldName, pnWidth, pnDecimals );
	if (fieldtype != FTInteger)
		return;
//	if (pszFieldName != "Id")
//		return;

	m_proj = proj;	// Set projection
	m_VLType = VLT_BioMap;

	// Initialize arrays
	m_Poly.SetSize(nElem);
	m_pAttrib = new int [nElem];

	// Read Polys from SHP into Veg Poly
	for (int i = 0; i < nElem; i++)
	{
		// Read DBF Attributes per poly
		m_pAttrib[i] = DBFReadIntegerAttribute(db, i, iField);

		// Get the i-th Poly in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		DLine2 *new_poly = new DLine2();
		new_poly->SetSize(psShape->nVertices);

		//Store the number of coordinate point in the i-th poly
		m_Poly.SetAt(i, new_poly);

		//Store each SHP Poly Coord in Veg Poly
		for (int j = 0; j < m_Poly[i]->GetSize(); j++)
		{
			new_poly->GetAt(j).x = psShape->padfX[j];
			new_poly->GetAt(j).y = psShape->padfY[j];
		}
		SHPDestroyObject(psShape);
	}

	DBFClose(db);
	SHPClose(hSHP);
}

int vtVegLayer::FindAttribute(DPoint2 p)
{
	int poly = m_Poly.FindPoly(p);
	if (poly != -1)
		return m_pAttrib[poly];
	else
		return -1;
}

