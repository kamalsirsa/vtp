//
// RawLayer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// A 'raw' layer is just abstract data, without any specific correspondence
// to any aspect of the physical world.  This is the same as a traditional
// GIS file (e.g. ESRI Shapefile).  In fact, SHP/DBF and 'shapelib' are used
// as the format and basic functionality for this layer.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RawLayer.h"
#include "ScaledView.h"
#include "Helper.h"

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
	int i, entities = NumEntities();

	if (!entities)
		return false;

	if (m_nSHPType == SHPT_POINT)
	{
		rect.SetRect(1E9, -1E9, -1E9, 1E9);
		for (i = 0; i < entities; i++)
			rect.GrowToContainPoint(m_Point2[i]);
	}
	if (m_nSHPType == SHPT_POINTZ)
	{
		rect.SetRect(1E9, -1E9, -1E9, 1E9);
		DPoint2 p;
		for (i = 0; i < entities; i++)
		{
			p.Set(m_Point3[i].x, m_Point3[i].y);
			rect.GrowToContainPoint(p);
		}
	}
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
	{
		rect.SetRect(1E9, -1E9, -1E9, 1E9);
		for (i = 0; i < entities; i++)
			rect.GrowToContainLine(*m_LinePoly[i]);
	}
	return true;
}

void vtRawLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	// single pixel solid pen
	wxPen DefPen(wxColor(128,0,0), 1, wxSOLID);
	wxPen SelPen(wxColor(255,255,0), 1, wxSOLID);
	int pen = 0;

	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(DefPen);
	int i, j, size, size2;

	wxPoint p;
	int entities = NumEntities();
	if (m_nSHPType == SHPT_POINT)
	{
		for (i = 0; i < entities; i++)
		{
			if (IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			pView->screen(m_Point2[i], p);
			pDC->DrawPoint(p);
			pDC->DrawPoint(p.x+1, p.y);
			pDC->DrawPoint(p.x, p.y+1);
			pDC->DrawPoint(p.x-1, p.y);
			pDC->DrawPoint(p.x, p.y-1);
		}
	}
	if (m_nSHPType == SHPT_POINTZ)
	{
		size = m_Point3.GetSize();
		for (i = 0; i < entities; i++)
		{
			if (IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			pView->screen(DPoint2(m_Point3[i].x, m_Point3[i].y), p);
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
		for (i = 0; i < entities; i++)
		{
			if (IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			DLine2 *dl = m_LinePoly.GetAt(i);
			size2 = dl->GetSize();

			for (j = 0; j < size2 && j < SCREENBUF_SIZE-1; j++)
				pView->screen(dl->GetAt(j), g_screenbuf[j]);
			if (m_nSHPType == SHPT_POLYGON)
				pView->screen(dl->GetAt(0), g_screenbuf[j++]);

			pDC->DrawLines(j, g_screenbuf);
		}
	}
}

bool vtRawLayer::ConvertProjection(vtProjection &proj)
{
	// Create conversion object
	OCT *trans = OGRCreateCoordinateTransformation(&m_proj, &proj);
	if (!trans)
		return false;		// inconvertible projections

	int i, j, pts;
	if (m_nSHPType == SHPT_POINT)
	{
		for (i = 0; i < m_Point2.GetSize(); i++)
			trans->Transform(1, &m_Point2[i].x, &m_Point2[i].y);
	}
	if (m_nSHPType == SHPT_POINTZ)
	{
		for (i = 0; i < m_Point3.GetSize(); i++)
			trans->Transform(1, &m_Point3[i].x, &m_Point3[i].y);
	}
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
	{
		for (i = 0; i < m_LinePoly.GetSize(); i++)
		{
			DLine2 *dline = m_LinePoly.GetAt(i);
			pts = dline->GetSize();
			for (j = 0; j < pts; j++)
			{
				trans->Transform(1, &((*dline)[i]).x, &((*dline)[i]).y);
			}
		}
	}
	delete trans;

	m_proj = proj;
	return true;
}

bool vtRawLayer::OnSave()
{
	return SaveToSHP(m_strFilename);
}

bool vtRawLayer::OnLoad()
{
	if (!m_strFilename.Right(4).CmpNoCase(".gml") ||
		 !m_strFilename.Right(4).CmpNoCase(".xml"))
	{
		return LoadWithOGR(m_strFilename, progress_callback);
	}
	else if (!m_strFilename.Right(4).CmpNoCase(".shp"))
	{
		return LoadFromSHP(m_strFilename);
	}
	return false;
}

bool vtRawLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_RAW)
		return false;

	vtRawLayer *pFrom = (vtRawLayer *)pL;

	// compatibility check
	if (pFrom->m_type != m_type)
		return false;
	if (pFrom->GetEntityType() != GetEntityType())
		return false;

	// copy entities
	vtString str;
	int i, f, result;
	int num = pFrom->NumEntities();
	for (i = 0; i < num; i++)
	{
		// copy geometry
		switch (m_nSHPType)
		{
		case SHPT_POINT:
			result = m_Point2.Append(pFrom->m_Point2[i]);
			break;
		case SHPT_POINTZ:
			result = m_Point3.Append(pFrom->m_Point3[i]);
			break;
		case SHPT_ARC:
		case SHPT_POLYGON:
			result = m_LinePoly.Append(pFrom->m_LinePoly[i]);	// steal pointer
			break;
		}
		// copy record data for all field names which match
		for (f = 0; f < pFrom->GetNumFields(); f++)
		{
			Field *field1 = pFrom->GetField(f);
			Field *field2 = GetField((const char *) field1->m_name);
			if (!field2)
				continue;
			field1->GetValueAsString(i, str);
			field2->SetValueFromString(result, str);
		}
		m_Flags.Append(pFrom->m_Flags[i]);
	}
	// empty the source layer
	switch (m_type)
	{
	case SHPT_POINT:  pFrom->m_Point2.SetSize(0); break;
	case SHPT_POINTZ: pFrom->m_Point3.SetSize(0); break;
	case SHPT_ARC:
	case SHPT_POLYGON: pFrom->m_LinePoly.SetSize(0); break;
	}
	return true;
}

void vtRawLayer::GetProjection(vtProjection &proj)
{
	proj = m_proj;
}

void vtRawLayer::SetProjection(const vtProjection &proj)
{
	m_proj = proj;
}

void vtRawLayer::Offset(const DPoint2 &p)
{
	int i, entities = NumEntities();
	if (m_nSHPType == SHPT_POINT)
	{
		for (i = 0; i < entities; i++)
			m_Point2[i] += p;
	}
	if (m_nSHPType == SHPT_POINTZ)
	{
		for (i = 0; i < entities; i++)
			m_Point3[i] += DPoint3(p.x, p.y, 0);
	}
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
	{
		for (i = 0; i < entities; i++)
			m_LinePoly[i]->Add(p);
	}
}

void vtRawLayer::GetPropertyText(wxString &strIn)
{
	wxString str;

	str.Printf("Entity type: %s\n", SHPTypeName(m_nSHPType));
	strIn += str;

	str.Printf("Entities: %d\n", NumEntities());
	strIn += str;

	int entities = NumEntities();
	if (m_nSHPType == SHPT_POINTZ && entities > 0)
	{
		float fmin = 1E9, fmax = -1E9;

		for (int i = 0; i < entities; i++)
		{
			if (m_Point3[i].z > fmax) fmax = m_Point3[i].z;
			if (m_Point3[i].z < fmin) fmin = m_Point3[i].z;
		}

		str.Printf("Minimum Height: %.2f\n", fmin);
		strIn += str;

		str.Printf("Maximum Height: %.2f\n", fmax);
		strIn += str;
	}
}


