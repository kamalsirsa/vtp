//
// RawLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// A 'raw' layer is just data, without any specific meaning.  This is the
// same as a traditional GIS file (e.g. ESRI Shapefile) without any
// attributes.  In fact, SHP and 'shapelib' are used as the format and
// basic functionality for this layer.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RawLayer.h"
#include "ScaledView.h"

////////////////////////////////////////////////////////////////////

vtRawLayer::vtRawLayer() : vtLayer(LT_RAW)
{
	m_strFilename = "Untitled.shp";
	m_nSHPType = SHPT_NULL;
}

vtRawLayer::~vtRawLayer()
{
}

bool vtRawLayer::GetExtent(DRECT &rect)
{
	int i;
	if (m_nSHPType == SHPT_POINT)
	{
		if (m_Point.IsEmpty())
			return false;
		rect.SetRect(1E9, -1E9, -1E9, 1E9);
		for (i = 0; i < m_Point.GetSize(); i++)
			rect.GrowToContainPoint(m_Point[i]);
	}
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
	{
		if (m_LinePoly.IsEmpty())
			return false;
		rect.SetRect(1E9, -1E9, -1E9, 1E9);
		for (i = 0; i < m_LinePoly.GetSize(); i++)
			rect.GrowToContainLine(*m_LinePoly[i]);
	}
	return true;
}

#define MAXPOINTS 32000
static wxPoint pbuf[MAXPOINTS];

void vtRawLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	//set the pen options
	//single pixel solid white pen
	wxPen DefPen(wxColor(255,255,0), 1, wxSOLID);
	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(DefPen);
	int i, j, size, size2;

	if (m_nSHPType == SHPT_POINT)
	{
		size = m_Point.GetSize();
		wxPoint p;
		for (i = 0; i < size; i++)
		{
			pView->screen(m_Point[i], p);
			pDC->DrawPoint(p);
			pDC->DrawPoint(p.x+1, p.y);
			pDC->DrawPoint(p.x, p.y+1);
			pDC->DrawPoint(p.x-1, p.y);
			pDC->DrawPoint(p.x, p.y-1);
		}
	}
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
	{
		size = m_LinePoly.GetSize();
		for (i = 0; i < size; i++)
		{
			DLine2 *dl = m_LinePoly.GetAt(i);
			size2 = dl->GetSize();

			for (j = 0; j < size2 && j < MAXPOINTS-1; j++)
				pView->screen(dl->GetAt(j), pbuf[j]);
			if (m_nSHPType == SHPT_POLYGON)
				pView->screen(dl->GetAt(0), pbuf[j++]);

			pDC->DrawLines(j, pbuf);
		}
	}
}

bool vtRawLayer::ConvertProjection(vtProjection &proj)
{
	// TODO - unimplemented
	return false;
}

bool vtRawLayer::OnSave()
{
	SHPHandle hSHP = SHPCreate(m_strFilename, m_nSHPType);
	if (!hSHP)
		return false;

	int i, size;
	if (m_nSHPType == SHPT_POINT)
	{
		size = m_Point.GetSize();
		wxPoint p;
		for (i = 0; i < size; i++)
		{
			// Save to SHP
			SHPObject *obj = SHPCreateSimpleObject(m_nSHPType, 1,
				&m_Point[i].x, &m_Point[i].y, NULL); 
			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);
		}
	}
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
	{
		size = m_LinePoly.GetSize();
		for (i = 0; i < size; i++)
		{
			DLine2 *dl = m_LinePoly.GetAt(i);
			// Save to SHP
			SHPObject *obj = SHPCreateSimpleObject(m_nSHPType, dl->GetSize(),
				&m_Point[i].x, &m_Point[i].y, NULL);
			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);
		}
	}
	SHPClose(hSHP);
	return true;
}

bool vtRawLayer::OnLoad()
{
	//Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(m_strFilename, "rb");
	if (hSHP == NULL)
		return false;

	//  Get number of polys (m_iNumPolys) and type of data (nShapeType)
	int		nElem;
	int		nShapeType;
    double	adfMinBound[4], adfMaxBound[4];
	FPoint2 point;
	SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

	//  Check Shape Type, Veg Layer should be Poly data
	switch (nShapeType)
	{
	case SHPT_POINT:
	case SHPT_ARC:
	case SHPT_POLYGON:
		m_nSHPType = nShapeType;
		break;
	default:
		SHPClose(hSHP);
		return false;
	}

#if 0
	//Open DBF File & Get DBF Info:
	DBFHandle db = DBFOpen(filename, "rb");
	if (db == NULL)
		return false;

	//  Check for field of poly id, current default field in dbf is Id
	int iField = 0, *pnWidth = 0, *pnDecimals = 0;
	char *pszFieldName = NULL;

	DBFFieldType fieldtype = DBFGetFieldInfo(db, iField,
		pszFieldName, pnWidth, pnDecimals );
	if (fieldtype != FTInteger)
		return false;
#endif
//	if (pszFieldName != "Id")
//		return;

	//Initialize arrays
	switch (m_nSHPType)
	{
	case SHPT_POINT:
		m_Point.SetSize(nElem);
		break;
	case SHPT_ARC:
	case SHPT_POLYGON:
		m_LinePoly.SetSize(nElem);
		break;
	}

	//Read Polys from SHP into Veg Poly
	for (int i = 0; i < nElem; i++)
	{
		//Read DBF Attributes per poly
//		m_pAttrib[i] = DBFReadIntegerAttribute(db, i, iField);

		//Get the i-th Poly in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		DPoint2 p;
		DLine2 *new_poly;
		switch (m_nSHPType)
		{
		case SHPT_POINT:
			p.x = *psShape->padfX;
			p.y = *psShape->padfY;
			m_Point.SetAt(i, p);
			break;
		case SHPT_ARC:
		case SHPT_POLYGON:
			new_poly = new DLine2();
			new_poly->SetSize(psShape->nVertices);

			//Store the number of coordinate point in the i-th poly
			m_LinePoly.SetAt(i, new_poly);

			//Store each SHP Poly Coord in Veg Poly
			for (int j = 0; j < psShape->nVertices; j++)
			{
				p.x = psShape->padfX[j];
				p.y = psShape->padfY[j];
				new_poly->SetAt(j, p);
			}
			break;
		}
		SHPDestroyObject(psShape);
	}

//	DBFClose(db);
	SHPClose(hSHP);
	return true;
}

void vtRawLayer::AppendDataFrom(vtLayer *pL)
{
	// unimplemented
}

void vtRawLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtRawLayer::SetProjection(vtProjection &proj)
{
	m_proj = proj;
}

void vtRawLayer::Offset(const DPoint2 &p)
{
	// TODO
}

void vtRawLayer::GetPropertyText(wxString &strIn)
{
	wxString str;

	str.Printf("Entity type: %s\n", SHPTypeName(m_nSHPType));
	strIn += str;

	str = "";
	if (m_nSHPType == SHPT_POINT)
		str.Printf("%d entities\n", m_Point.GetSize());
	else if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
		str.Printf("%d entities\n", m_LinePoly.GetSize());
	strIn += str;
}

int vtRawLayer::GetEntityType()
{
	return m_nSHPType;
}

void vtRawLayer::SetEntityType(int type)
{
	m_nSHPType = type;
}

void vtRawLayer::AddPoint(DPoint2 p)
{
	if (m_nSHPType == SHPT_POINT)
	{
		m_Point.Append(p);
		SetModified(true);
	}
}


