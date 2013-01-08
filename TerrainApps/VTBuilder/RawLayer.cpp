//
// RawLayer.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
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

#include "vtui/Helper.h"
#include "xmlhelper/easyxml.hpp"

#include "RawLayer.h"
#include "BuilderView.h"
#include "Builder.h"
#include "Options.h"
// Dialogs
#include "FeatInfoDlg.h"

////////////////////////////////////////////////////////////////////

vtRawLayer::vtRawLayer() : vtLayer(LT_RAW)
{
	m_pSet = NULL;

	wxString name = _("Untitled");
	name += _T(".shp");
	SetLayerFilename(name);

	// default dark red
	m_DrawStyle.m_LineColor.Set(128,0,0);

	m_DrawStyle.m_MarkerShape = 0;
	m_DrawStyle.m_MarkerSize = 2;
	m_bExtentComputed = false;
}

vtRawLayer::~vtRawLayer()
{
	delete m_pSet;
	m_pSet = NULL;
}

void vtRawLayer::SetGeomType(OGRwkbGeometryType type)
{
	if (!m_pSet)
	{
		switch (type)
		{
		case wkbPoint:
			m_pSet = new vtFeatureSetPoint2D;
			break;
		case wkbPoint25D:
			m_pSet = new vtFeatureSetPoint3D;
			break;
		case wkbLineString:
			m_pSet = new vtFeatureSetLineString;
			break;
		case wkbLineString25D:
			m_pSet = new vtFeatureSetLineString3D;
			break;
		case wkbPolygon:
			m_pSet = new vtFeatureSetPolygon;
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

	if (m_bExtentComputed)
		rect = m_Extents;
	else
	{
		if (!m_pSet->ComputeExtent(rect))
			return false;
		m_Extents = rect;
		m_bExtentComputed = true;
	}

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

void vtRawLayer::DrawLayer(wxDC *pDC, vtScaledView *pView)
{
	if (!m_pSet)
		return;

	// To draw simpler and faster, draw only 1/10 of the entities, with a box
	//  around them to make the location and extent of the data clear.
	bool bDrawSimple = g_Options.GetValueBool(TAG_DRAW_RAW_SIMPLE);
	int iIncrement = bDrawSimple ? 10 : 1;
	if (bDrawSimple)
	{
		DRECT rect;
		GetExtent(rect);
		wxRect screenrect = pView->WorldToCanvas(rect);

		// draw a simple box with green lines
		wxPen Pen1(wxColor(0x80, 0x00, 0x00), 1, wxLONG_DASH);
		pDC->SetLogicalFunction(wxCOPY);
		pDC->SetPen(Pen1);
		DrawRectangle(pDC, screenrect);
	}

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
	uint i, j, entities = m_pSet->NumEntities();
	OGRwkbGeometryType type = m_pSet->GetGeomType();
	if (type == wkbPoint)
	{
		vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(m_pSet);
		for (i = 0; i < entities; i += iIncrement)
		{
			if (m_pSet->IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			const DPoint2 &p2 = pSetP2->GetPoint(i);
			pView->screen(p2, p);
			if (bDrawSimple)
			{
				pDC->DrawPoint(p);
			}
			else if (m_DrawStyle.m_MarkerShape == 0)	// dot
			{
				if (m_DrawStyle.m_MarkerSize < 2)
				{
					pDC->DrawPoint(p);
					pDC->DrawPoint(p.x+1, p.y);
					pDC->DrawPoint(p.x, p.y+1);
					pDC->DrawPoint(p.x-1, p.y);
					pDC->DrawPoint(p.x, p.y-1);
				}
				else
				{
					pDC->DrawLine(p.x, p.y-1, p.x+2, p.y-1);
					pDC->DrawLine(p.x-1, p.y, p.x+3, p.y);
					pDC->DrawLine(p.x-1, p.y+1, p.x+3, p.y+1);
					pDC->DrawLine(p.x, p.y+2, p.x+2, p.y+2);
				}
			}
			else if (m_DrawStyle.m_MarkerShape == 1)	// crosshair
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
		for (i = 0; i < entities; i += iIncrement)
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
		for (i = 0; i < entities; i += iIncrement)
		{
			if (m_pSet->IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			DLine2 &dline = pSetLine->GetPolyLine(i);

			bool bClosed = false;
			pView->DrawPolyLine(pDC, dline, bClosed);
		}
	}
	if (type == wkbLineString25D)
	{
		vtFeatureSetLineString3D *pSetLine = dynamic_cast<vtFeatureSetLineString3D *>(m_pSet);
		for (i = 0; i < entities; i += iIncrement)
		{
			if (m_pSet->IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			DLine3 &dline3 = pSetLine->GetPolyLine(i);

			// convert (inefficient)
			DLine2 dline;
			dline.SetSize(dline3.GetSize());
			for (j = 0; j < dline3.GetSize(); j++)
				dline[j].Set(dline3[j].x, dline3[j].y);

			bool bClosed = false;
			pView->DrawPolyLine(pDC, dline, bClosed);
		}
	}
	if (type == wkbPolygon)
	{
		vtFeatureSetPolygon *pSetPoly = dynamic_cast<vtFeatureSetPolygon *>(m_pSet);

		if (bFill)
			pDC->SetPen(NoPen);

		for (i = 0; i < entities; i += iIncrement)
		{
			if (bFill)
				pDC->SetPen(NoPen);
			else
			{
				if (pSetPoly->IsSelected(i)) {
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
				if (pSetPoly->IsSelected(i))
					pDC->SetPen(SelPen);
				else
					pDC->SetPen(DefPen);
				pView->DrawPolygon(pDC, dpoly, false);
			}
		}
	}
}

bool vtRawLayer::TransformCoords(vtProjection &proj)
{
	// Create conversion object
	OCT *trans = CreateCoordTransform(&(m_pSet->GetAtProjection()), &proj);
	if (!trans)
		return false;		// inconvertible projections

	bool success = m_pSet->TransformCoords(trans, progress_callback);
	delete trans;

	if (!success)
		DisplayAndLog("Warning: Some coordinates did not transform correctly.\n");

	m_pSet->SetProjection(proj);
	m_bExtentComputed = false;
	SetModified(true);

	return true;
}

bool vtRawLayer::OnSave(bool progress_callback(int))
{
	if (!m_pSet)
		return false;
	return m_pSet->SaveToSHP(m_pSet->GetFilename(), progress_callback);
}

bool vtRawLayer::OnLoad()
{
	assert(m_pSet == NULL);	// Shouldn't happen!

	vtFeatureLoader loader;

	wxString fname = GetLayerFilename();
	vtString fname_utf8 = (const char *) fname.mb_str(wxConvUTF8);
	vtString ext = fname_utf8.Right(4);
	VTLOG("vtRawLayer::OnLoad, extension is '%s'\n", (const char *) ext);

	OpenProgressDialog(_("Loading"), false);

	if (!ext.CompareNoCase(".gml") ||
		!ext.CompareNoCase(".xml") ||
		!ext.CompareNoCase(".ntf"))
	{
		m_pSet = loader.LoadWithOGR(fname_utf8, progress_callback);
	}
	else if (!ext.CompareNoCase(".shp"))
	{
		m_pSet = loader.LoadFromSHP(fname_utf8, progress_callback);
//		return LoadWithOGR(fname.mb_str(wxConvUTF8));
	}
	else if (!ext.CompareNoCase(".igc"))
	{
		m_pSet = loader.LoadFromIGC(fname_utf8);
	}

	CloseProgressDialog();

	if (m_pSet)
	{
		vtProjection &proj = m_pSet->GetAtProjection();
		if (!g_bld->ConfirmValidCRS(&proj))
		{
			delete m_pSet;
			m_pSet = NULL;
			return false;
		}
	}
	else
		wxMessageBox(wxString(loader.m_strErrorMsg, wxConvUTF8));

	m_bExtentComputed = false;

	return (m_pSet != NULL);
}

bool vtRawLayer::AppendDataFrom(vtLayer *pL)
{
	vtRawLayer *pFrom = (vtRawLayer *)pL;
	vtFeatureSet *m_pFromSet = pFrom->m_pSet;

	// compatibility check
	if (pFrom->m_type != m_type)
		return false;

	// What to do if the geometries are similar, but datafields are different?
	if (!m_pSet->AppendDataFrom(m_pFromSet))
		return false;

	m_bExtentComputed = false;

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
	{
		const vtProjection &current = m_pSet->GetAtProjection();
		if (proj != current)
		{
			SetModified(true);
			m_bExtentComputed = false;
		}
		m_pSet->SetProjection(proj);
	}
}

void vtRawLayer::Offset(const DPoint2 &p)
{
	if (m_pSet)
	{
		if (m_pSet->NumSelected())
			m_pSet->Offset(p, true);
		else
			m_pSet->Offset(p);
	}
	m_bExtentComputed = false;
}

void vtRawLayer::GetPropertyText(wxString &strIn)
{
	if (!m_pSet)
	{
		strIn += _("No Features\n");
		return;
	}

	OGRwkbGeometryType type = m_pSet->GetGeomType();
	wxString str;
	str.Printf(_("Entity type: %hs\n"), OGRGeometryTypeToName(type));
	strIn += str;

	str.Printf(_("Entities: %d\n"), m_pSet->NumEntities());
	strIn += str;

	if (type == wkbPoint25D)
	{
		vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(m_pSet);
		float fmin, fmax;
		if (pSetP3->ComputeHeightRange(fmin, fmax))
		{
			str.Printf(_("Minimum Height: %.2f\n"), fmin);
			strIn += str;

			str.Printf(_("Maximum Height: %.2f\n"), fmax);
			strIn += str;
		}
	}

	if (type == wkbLineString25D)
	{
		vtFeatureSetLineString3D *pLine3 = dynamic_cast<vtFeatureSetLineString3D *>(m_pSet);
		float fmin, fmax;
		if (pLine3->ComputeHeightRange(fmin, fmax))
		{
			str.Printf(_("Minimum Height: %.2f\n"), fmin);
			strIn += str;

			str.Printf(_("Maximum Height: %.2f\n"), fmax);
			strIn += str;
		}
	}

	int num_fields = m_pSet->GetNumFields();
	if (num_fields)
	{
		strIn += _("Fields:\n");
		for (int i = 0; i < num_fields; i++)
		{
			Field *pField = m_pSet->GetField(i);
			str.Printf(_T("  %hs (%hs)\n"), (const char *) pField->m_name,
				DescribeFieldType(pField->m_type));
			strIn += str;
		}
	}
	else
		strIn += _("Fields: None.\n");
}

void vtRawLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	int iEnt;
	double epsilon = pView->odx(6);  // calculate what 6 pixels is as world coord

	if (NULL == m_pSet)		// safety check
		return;

	vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D*>(m_pSet);
	vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D*>(m_pSet);
	vtFeatureSetLineString *pSetL2 = dynamic_cast<vtFeatureSetLineString*>(m_pSet);
	vtFeatureSetLineString3D *pSetL3 = dynamic_cast<vtFeatureSetLineString3D*>(m_pSet);

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
		if (type == wkbPoint)
		{
			iEnt = pSetP2->FindClosestPoint(ui.m_DownLocation, epsilon);
			if (iEnt != -1)
			{
				g_bld->UpdateFeatureDialog(this, pSetP2, iEnt);
			}
		}
		//if (type == wkbPoint25D)
		//{
		//	iEnt = pSetP3->FindClosestPoint(ui.m_DownLocation, epsilon);
		//	if (iEnt != -1)
		//	{
		//		g_bld->UpdateFeatureDialog(this, pSetP3, iEnt);
		//	}
		//}
		if (type == wkbLineString)
		{
			int close_feature;
			DPoint2 close_point;
			if (pSetL2->FindClosest(ui.m_DownLocation, close_feature, close_point))
			{
				//g_bld->UpdateFeatureDialog(this, pSetL3, close_feature);
				VTLOG("Close 2D point: %lf, %lf\n", close_point.x, close_point.y);
				g_bld->UpdateFeatureDialog(this, pSetL2, close_feature);
			}
		}
		if (type == wkbLineString25D)
		{
			int close_feature;
			DPoint3 close_point;
			if (pSetL3->FindClosest(ui.m_DownLocation, close_feature, close_point))
			{
				//g_bld->UpdateFeatureDialog(this, pSetL3, close_feature);
				VTLOG("Close 3D point: %lf, %lf, %lf\n", close_point.x, close_point.y, close_point.z);
				g_bld->UpdateFeatureDialog(this, pSetL3, close_feature);
			}
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
		m_bExtentComputed = false;
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

bool vtRawLayer::LoadWithOGR(const char *filename, bool progress_callback(int))
{
	vtFeatureLoader loader;

	vtFeatureSet *pSet = loader.LoadWithOGR(filename, progress_callback);
	if (pSet)
		m_pSet = pSet;
	return (pSet != NULL);
}

void vtRawLayer::ScaleHorizontally(double factor)
{
	if (!m_pSet)
		return;

	uint i, j, entities = m_pSet->NumEntities();
	OGRwkbGeometryType type = m_pSet->GetGeomType();
	if (type == wkbPoint)
	{
		vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(m_pSet);
		for (i = 0; i < entities; i++)
			pSetP2->SetPoint(i, pSetP2->GetPoint(i) * factor);
	}
	if (type == wkbPoint25D)
	{
		vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			const DPoint3 &p3 = pSetP3->GetPoint(i);
			pSetP3->SetPoint(i, DPoint3(p3.x * factor, p3.y * factor, p3.z));
		}
	}
	if (type == wkbLineString)
	{
		vtFeatureSetLineString *pSetLine = dynamic_cast<vtFeatureSetLineString *>(m_pSet);
		for (i = 0; i < entities; i++)
			pSetLine->GetPolyLine(i).Mult(factor);
	}
	if (type == wkbLineString25D)
	{
		vtFeatureSetLineString3D *pSetLine = dynamic_cast<vtFeatureSetLineString3D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			DLine3 &dline3 = pSetLine->GetPolyLine(i);
			for (j = 0; j < dline3.GetSize(); j++)
			{
				dline3[j].x *= factor;
				dline3[j].y *= factor;
			}
		}
	}
	if (type == wkbPolygon)
	{
		vtFeatureSetPolygon *pSetPoly = dynamic_cast<vtFeatureSetPolygon *>(m_pSet);
		for (i = 0; i < entities; i++)
			pSetPoly->GetPolygon(i).Mult(factor);
	}
	SetModified(true);
	m_bExtentComputed = false;
}

void vtRawLayer::ScaleVertically(double factor)
{
	if (!m_pSet)
		return;

	uint i, j, entities = m_pSet->NumEntities();
	OGRwkbGeometryType type = m_pSet->GetGeomType();
	if (type == wkbPoint25D)
	{
		vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			const DPoint3 &p3 = pSetP3->GetPoint(i);
			pSetP3->SetPoint(i, DPoint3(p3.x, p3.y, p3.z * factor));
		}
	}
	if (type == wkbLineString25D)
	{
		vtFeatureSetLineString3D *pSetLine = dynamic_cast<vtFeatureSetLineString3D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			DLine3 &dline3 = pSetLine->GetPolyLine(i);
			for (j = 0; j < dline3.GetSize(); j++)
				dline3[j].z *= factor;
		}
	}
	SetModified(true);
	m_bExtentComputed = false;
}

void vtRawLayer::OffsetVertically(double amount)
{
	if (!m_pSet)
		return;

	uint i, j, entities = m_pSet->NumEntities();
	OGRwkbGeometryType type = m_pSet->GetGeomType();
	if (type == wkbPoint25D)
	{
		vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			const DPoint3 &p3 = pSetP3->GetPoint(i);
			pSetP3->SetPoint(i, DPoint3(p3.x, p3.y, p3.z + amount));
		}
	}
	if (type == wkbLineString25D)
	{
		vtFeatureSetLineString3D *pSetLine = dynamic_cast<vtFeatureSetLineString3D *>(m_pSet);
		for (i = 0; i < entities; i++)
		{
			DLine3 &dline3 = pSetLine->GetPolyLine(i);
			for (j = 0; j < dline3.GetSize(); j++)
				dline3[j].z += amount;
		}
	}
	SetModified(true);
	m_bExtentComputed = false;
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
	vtFeatureSetPoint2D *pPointSet = new vtFeatureSetPoint2D;
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


////////////////////////////////////////////////////////////////////////
// Visitor class, for parsing of a raw XML file into a Raw Layer.
////////////////////////////////////////////////////////////////////////

class VisitorRawXML : public XMLVisitor
{
public:
	VisitorRawXML(vtFeatureSetPoint2D *fs) : m_state(0), m_rec(-1), m_pSet(fs) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

private:
	string m_data;
	int m_state;
	int m_rec;

	vtFeatureSetPoint2D *m_pSet;
};

void VisitorRawXML::startElement(const char *name, const XMLAttributes &atts)
{
	// clear data at the start of each element
	m_data = "";

	m_state++;
	// State 1 - Container
	// State 2 - Item
	if (m_state == 2)
	{
		// Add record
		m_rec = m_pSet->AddPoint(DPoint2(0,0));
	}
}

void VisitorRawXML::endElement(const char *name)
{
	// State 3 - Field
	if (m_state == 3)
	{
		vtString fieldname = name;	// field name
		double val;
		if (fieldname == "X" || fieldname == "Y")
		{
			// strip commas from number like "152,744.69"
			vtString str = m_data.c_str();
			int j = 0;
			for (int i = 0; i <= str.GetLength(); i++)
			{
				if (i != j) str.SetAt(j, str[i]);
				if (str[i] != ',') j++;
			}
			val = atof(str);
		}
		if (fieldname == "X")
		{
			DPoint2 p;
			m_pSet->GetPoint(m_rec, p);
			p.x = val;
			m_pSet->SetPoint(m_rec, p);
		}
		else if (fieldname == "Y")
		{
			DPoint2 p;
			m_pSet->GetPoint(m_rec, p);
			p.y = val;
			m_pSet->SetPoint(m_rec, p);
		}
		else
		{
			int field = m_pSet->GetFieldIndex(name);
			if (field == -1)
				field = m_pSet->AddField(name, FT_String);

			m_pSet->SetValue(m_rec, field, m_data.c_str());
		}
	}
	m_state--;
}

void VisitorRawXML::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

bool vtRawLayer::ImportFromXML(const char *fname)
{
	vtFeatureSetPoint2D *pPointSet = new vtFeatureSetPoint2D;
//	pPointSet->AddField("..", FT_String, 80);

	VisitorRawXML visitor(pPointSet);
	try
	{
		readXML(fname, visitor);
	}
	catch (xh_exception &)
	{
		delete pPointSet;
		return false;
	}
	m_pSet = pPointSet;
	return true;
}


///////////////////////////////////////////

void vtRawLayer::CreateIndex(int iSize)
{
	vtFeatureSetPolygon *polyset = dynamic_cast<vtFeatureSetPolygon *>(m_pSet);
	if (polyset)
		polyset->CreateIndex(iSize);
}

void vtRawLayer::FreeIndex()
{
	vtFeatureSetPolygon *polyset = dynamic_cast<vtFeatureSetPolygon *>(m_pSet);
	if (polyset)
		polyset->FreeIndex();
}


///////////////////////////////////////////

bool vtRawLayer::CreateFromOGRLayer(OGRLayer *pOGRLayer)
{
	vtFeatureLoader loader;

	vtFeatureSet *pSet = loader.LoadWithOGR(pOGRLayer, progress_callback);
	if (pSet)
		m_pSet = pSet;
	return (pSet != NULL);
}

