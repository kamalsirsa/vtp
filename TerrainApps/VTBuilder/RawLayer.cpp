//
// RawLayer.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// A 'raw' layer is just abstract data, without any specific correspondence
// to any aspect of the physical world.  This is the same as a traditional
// GIS file (e.g. ESRI Shapefile).
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RawLayer.h"
#include "BuilderView.h"
#include "Helper.h"
#include "Frame.h"
#include "xmlhelper/easyxml.hpp"
// Dialogs
#include "FeatInfoDlg.h"

////////////////////////////////////////////////////////////////////

vtRawLayer::vtRawLayer() : vtLayer(LT_RAW)
{
	m_pSet = NULL;
	SetLayerFilename("Untitled.shp");

	// default dark red
	m_DrawStyle.m_LineColor.Set(128,0,0);

	m_DrawStyle.m_MarkerShape = 0;
	m_DrawStyle.m_MarkerSize = 1;
}

vtRawLayer::~vtRawLayer()
{
}

void vtRawLayer::SetGeomType(OGRwkbGeometryType type)
{
	if (!m_pSet)
	{
		switch (type)
		{
		case wkbPoint:
			m_pSet = new vtFeatureSetPoint2D();
			break;
		case wkbPoint25D:
			m_pSet = new vtFeatureSetPoint3D();
			break;
		case wkbLineString:
			m_pSet = new vtFeatureSetLineString();
			break;
		case wkbPolygon:
			m_pSet = new vtFeatureSetPolygon();
			break;
		}
	}
}

OGRwkbGeometryType vtRawLayer::GetGeomType()
{
	if (m_pSet)
		return m_pSet->GetGeomType();
	else
		return wkbNone;
}

bool vtRawLayer::GetExtent(DRECT &rect)
{
	if (!m_pSet)
		return false;

	if (!m_pSet->ComputeExtent(rect))
		return false;

	OGRwkbGeometryType type = m_pSet->GetGeomType();
	if (type == wkbPoint || type == wkbPoint25D)
	{
		// to support zoom into a single point, create artificial size
		if (rect.Width() == 0 || rect.Height() == 0)
		{
			if (GetAtProjection()->IsGeographic())
				rect.Grow(0.00002, 0.00002);
			else
				rect.Grow(2, 2);
		}
	}
	return true;
}

void vtRawLayer::DrawLayer(wxDC* pDC, vtScaledView *pView)
{
	if (!m_pSet)
		return;

	wxColor linecolor(m_DrawStyle.m_LineColor.r, m_DrawStyle.m_LineColor.g, m_DrawStyle.m_LineColor.b);
	bool bFill = m_DrawStyle.m_bFill;

	// single pixel solid pens
	wxPen DefPen(linecolor, 1, wxSOLID);
	wxPen SelPen(wxColor(255,255,0), 1, wxSOLID);
	wxPen NoPen(wxColor(0,0,0), 1, wxTRANSPARENT);
	int pen = 0;

	pDC->SetLogicalFunction(wxCOPY);
	pDC->SetPen(DefPen);

	wxPoint p;
	int i, entities = m_pSet->GetNumEntities();
	OGRwkbGeometryType type = m_pSet->GetGeomType();
	if (type == wkbPoint)
	{
		vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			if (m_pSet->IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			const DPoint2 &p2 = pSetP2->GetPoint(i);
			pView->screen(p2, p);
			if (m_DrawStyle.m_MarkerShape == 0)	// dot
			{
				pDC->DrawPoint(p);
				pDC->DrawPoint(p.x+1, p.y);
				pDC->DrawPoint(p.x, p.y+1);
				pDC->DrawPoint(p.x-1, p.y);
				pDC->DrawPoint(p.x, p.y-1);
			}
			if (m_DrawStyle.m_MarkerShape == 1)	// crosshair
			{
				int ms = m_DrawStyle.m_MarkerSize;
				pDC->DrawLine(p.x-ms, p.y, p.x+ms+1, p.y);
				pDC->DrawLine(p.x, p.y-ms, p.x, p.y+ms+1);
			}
		}
	}
	if (type == wkbPoint25D)
	{
		vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			if (m_pSet->IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			const DPoint3 &p3 = pSetP3->GetPoint(i);
			pView->screen(DPoint2(p3.x, p3.y), p);
			pDC->DrawPoint(p);
			pDC->DrawPoint(p.x+1, p.y);
			pDC->DrawPoint(p.x, p.y+1);
			pDC->DrawPoint(p.x-1, p.y);
			pDC->DrawPoint(p.x, p.y-1);
		}
	}
	if (type == wkbLineString)
	{
		vtFeatureSetLineString *pSetLine = dynamic_cast<vtFeatureSetLineString *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			if (m_pSet->IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			DLine2 &dline = pSetLine->GetPolyLine(i);

			bool bClosed = false;
			pView->DrawLine(pDC, dline, bClosed);
		}
	}
	if (type == wkbPolygon)
	{
		vtFeatureSetPolygon *pSetPoly = dynamic_cast<vtFeatureSetPolygon *>(m_pSet);

		if (bFill)
			pDC->SetPen(NoPen);

		for (i = 0; i < entities; i++)
		{
			if (bFill)
				pDC->SetPen(NoPen);
			else
			{
				if (m_pSet->IsSelected(i)) {
					if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
				}
				else {
					if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
				}
			}
			DPolygon2 &dpoly = pSetPoly->GetPolygon(i);
			pView->DrawPolygon(pDC, dpoly, bFill);

			if (bFill)
			{
				if (m_pSet->IsSelected(i))
					pDC->SetPen(SelPen);
				else
					pDC->SetPen(DefPen);
				pView->DrawPolygon(pDC, dpoly, false);
			}
		}
	}
}

bool vtRawLayer::ConvertProjection(vtProjection &proj)
{
	// Create conversion object
	OCT *trans = CreateCoordTransform(&(m_pSet->GetAtProjection()), &proj);
	if (!trans)
		return false;		// inconvertible projections

	bool success = m_pSet->TransformCoords(trans);
	delete trans;

	if (!success)
		DisplayAndLog("Warning: Some coordinates did not transform correctly.\n");

	m_pSet->SetProjection(proj);
	return true;
}

bool vtRawLayer::OnSave()
{
	if (!m_pSet)
		return false;
	return m_pSet->SaveToSHP(m_pSet->GetFilename());
}

bool vtRawLayer::OnLoad()
{
	if (m_pSet != NULL)
	{
		// Shouldn't happen!
		int warning = 1;
	}

	vtFeatureLoader loader;

	wxString2 fname = GetLayerFilename();
	if (!fname.Right(4).CmpNoCase(_T(".gml")) ||
		 !fname.Right(4).CmpNoCase(_T(".xml")))
	{
		m_pSet = loader.LoadWithOGR(fname.mb_str(), progress_callback);
	}
	else if (!fname.Right(4).CmpNoCase(_T(".shp")))
	{
		m_pSet = loader.LoadFromSHP(fname.mb_str());
//		return LoadWithOGR(fname.mb_str());
	}
	return (m_pSet != NULL);
}

bool vtRawLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_RAW)
		return false;

	vtRawLayer *pFrom = (vtRawLayer *)pL;
	vtFeatureSet *m_pFromSet = pFrom->m_pSet;

	// compatibility check
	if (pFrom->m_type != m_type)
		return false;

	// What to do if the geometries are similar, but datafields are different?
	if (!m_pSet->AppendDataFrom(m_pFromSet))
		return false;

	return true;
}

void vtRawLayer::GetProjection(vtProjection &proj)
{
	if (m_pSet)
		proj = m_pSet->GetAtProjection();
}

void vtRawLayer::SetProjection(const vtProjection &proj)
{
	if (m_pSet)
		m_pSet->SetProjection(proj);
}

void vtRawLayer::Offset(const DPoint2 &p)
{
	if (m_pSet)
		m_pSet->Offset(p);
}

void vtRawLayer::GetPropertyText(wxString &strIn)
{
	if (!m_pSet)
	{
		strIn += _T("No Features\n");
		return;
	}

	wxString str;

	OGRwkbGeometryType type = m_pSet->GetGeomType();
	str.Printf(_T("Entity type: %hs\n"), OGRGeometryTypeToName(type));
	strIn += str;

	str.Printf(_T("Entities: %d\n"), m_pSet->GetNumEntities());
	strIn += str;

	if (type == wkbPoint25D)
	{
		vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(m_pSet);
		float fmin, fmax;
		if (pSetP3->ComputeHeightRange(fmin, fmax))
		{
			str.Printf(_T("Minimum Height: %.2f\n"), fmin);
			strIn += str;

			str.Printf(_T("Maximum Height: %.2f\n"), fmax);
			strIn += str;
		}
	}

	int num_fields = m_pSet->GetNumFields();
	if (num_fields)
	{
		strIn += _T("Fields:\n");
		for (int i = 0; i < num_fields; i++)
		{
			Field *pField = m_pSet->GetField(i);
			str.Printf(_T("  %s (%s)\n"), (const char *) pField->m_name,
				DescribeFieldType(pField->m_type));
			strIn += str;
		}
	}
	else
		strIn += _T("Fields: None.\n");
}

void vtRawLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	int iEnt;
	double epsilon = pView->odx(6);  // calculate what 6 pixels is as world coord

	vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D*>(m_pSet);
	vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D*>(m_pSet);

	OGRwkbGeometryType type = m_pSet->GetGeomType();
	switch (ui.mode)
	{
	case LB_AddPoints:
		if (type == wkbPoint)
		{
			pSetP2->AddPoint(ui.m_DownLocation);
		}
		else if (type == wkbPoint25D)
		{
			pSetP3->AddPoint(DPoint3(ui.m_DownLocation.x, ui.m_DownLocation.y, 0));
		}
		SetModified(true);
		pView->Refresh();
		break;
	case LB_FeatInfo:
		if (type != wkbPoint)
			return;

		iEnt = pSetP2->FindClosestPoint(ui.m_DownLocation, epsilon);
		if (iEnt != -1)
		{
			DPoint2 &p2 = pSetP2->GetPoint(iEnt);
			Array<int> found;
			pSetP2->FindAllPointsAtLocation(p2, found);

			FeatInfoDlg	*fdlg = GetMainFrame()->ShowFeatInfoDlg();
			fdlg->SetLayer(this);
			fdlg->SetFeatureSet(m_pSet);
			m_pSet->DePickAll();
			for (unsigned int i = 0; i < found.GetSize(); i++)
				m_pSet->Pick(found[i]);
			fdlg->ShowPicked();
		}
		break;
	}
}

void vtRawLayer::AddPoint(const DPoint2 &p2)
{
	vtFeatureSetPoint2D *pPointSet = dynamic_cast<vtFeatureSetPoint2D*>(m_pSet);
	if (pPointSet)
	{
		pPointSet->AddPoint(p2);
		SetModified(true);
	}
}

bool vtRawLayer::ReadFeaturesFromWFS(const char *szServerURL, const char *layername)
{
	vtFeatureLoader loader;

	vtFeatureSet *pSet = loader.ReadFeaturesFromWFS(szServerURL, layername);
	if (pSet)
		m_pSet = pSet;
	return (pSet != NULL);
}

bool vtRawLayer::LoadWithOGR(const char *filename, void progress_callback(int))
{
	vtFeatureLoader loader;

	vtFeatureSet *pSet = loader.LoadWithOGR(filename, progress_callback);
	if (pSet)
		m_pSet = pSet;
	return (pSet != NULL);
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of a GeoURL file.
////////////////////////////////////////////////////////////////////////

class VisitorGU : public XMLVisitor
{
public:
	VisitorGU(vtFeatureSetPoint2D *fs) : m_state(0), m_pSet(fs) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

private:
	string m_data;
	int m_state;
	int m_rec;

	vtFeatureSetPoint2D *m_pSet;
};

void VisitorGU::startElement(const char *name, const XMLAttributes &atts)
{
	// clear data at the start of each element
	m_data = "";
	const char *val;

	if (m_state == 0 && !strcmp(name, "geourl"))
	{
		m_state = 1;
	}
	else if (m_state == 1)
	{
		if (!strcmp(name, "site"))
		{
			DPoint2 p;

			val = atts.getValue("lon");
			if (!val) val = atts.getValue("longitude");
			p.x = atof(val);

			val = atts.getValue("lat");
			if (!val) val = atts.getValue("latitude");
			p.y = atof(val);

			m_rec = m_pSet->AddPoint(p);

			val = atts.getValue("href");
			m_pSet->SetValue(m_rec, 1, val);

			m_state = 2;
		}
	}
}

void VisitorGU::endElement(const char *name)
{
	if (m_state == 2)
	{
		m_pSet->SetValue(m_rec, 0, m_data.c_str());
		m_state = 1;
	}
}

void VisitorGU::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

void vtRawLayer::ReadGeoURL()
{
	vtFeatureSetPoint2D *pPointSet = new vtFeatureSetPoint2D();
	pPointSet->AddField("Name", FT_String, 80);
	pPointSet->AddField("URL", FT_String, 80);

	VisitorGU visitor(pPointSet);
	try
	{
		readXML("D:/Data-World/geourl2.xml", visitor);
	}
	catch (xh_exception &)
	{
		return;
	}
	m_pSet = pPointSet;
}

