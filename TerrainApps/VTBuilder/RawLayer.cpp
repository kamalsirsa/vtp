//
// RawLayer.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
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
#include "BuilderView.h"
#include "Helper.h"
#include "Frame.h"
#include "xmlhelper/easyxml.hpp"
// Dialogs
#include "FeatInfoDlg.h"

////////////////////////////////////////////////////////////////////

vtRawLayer::vtRawLayer() : vtLayer(LT_RAW)
{
	SetFilename("Untitled.shp");
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

		// to support zoom into a single point, create artificial size
		if (rect.Width() == 0 || rect.Height() == 0)
		{
			if (m_proj.IsGeographic())
				rect.Grow(0.00002, 0.00002);
			else
				rect.Grow(2, 2);
		}
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
			rect.GrowToContainLine(m_LinePoly[i]);
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
		size = m_LinePoly.size();
		for (i = 0; i < entities; i++)
		{
			if (IsSelected(i)) {
				if (pen == 0) { pDC->SetPen(SelPen); pen = 1; }
			}
			else {
				if (pen == 1) { pDC->SetPen(DefPen); pen = 0; }
			}
			DLine2 &dline = m_LinePoly[i];
			size2 = dline.GetSize();

			for (j = 0; j < size2 && j < SCREENBUF_SIZE-1; j++)
				pView->screen(dline.GetAt(j), g_screenbuf[j]);
			if (m_nSHPType == SHPT_POLYGON)
				pView->screen(dline.GetAt(0), g_screenbuf[j++]);

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

	unsigned int i, j, pts, success, good = 0, bad = 0;
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
		for (i = 0; i < m_LinePoly.size(); i++)
		{
			DLine2 &dline = m_LinePoly[i];
			pts = dline.GetSize();
			for (j = 0; j < pts; j++)
			{
				DPoint2 &p = dline.GetAt(j);
				success = trans->Transform(1, &p.x, &p.y);
				if (success == 1)
					good++;
				else
					bad++;
			}
		}
	}
	delete trans;

	if (bad)
		DisplayAndLog("Warning: %d of %d coordinates did not project correctly.\n",
			bad, bad+good);

	m_proj = proj;
	return true;
}

bool vtRawLayer::OnSave()
{
	return SaveToSHP(GetFilename());
}

bool vtRawLayer::OnLoad()
{
	wxString2 fname = GetLayerFilename();
	if (!fname.Right(4).CmpNoCase(_T(".gml")) ||
		 !fname.Right(4).CmpNoCase(_T(".xml")))
	{
		return LoadWithOGR(fname.mb_str(), progress_callback);
	}
	else if (!fname.Right(4).CmpNoCase(_T(".shp")))
	{
		return LoadFromSHP(fname.mb_str());
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
			m_LinePoly.push_back(pFrom->m_LinePoly[i]);
			result = m_LinePoly.size()-1;
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
	case SHPT_POLYGON: pFrom->m_LinePoly.resize(0); break;
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
			m_LinePoly[i].Add(p);
	}
}

void vtRawLayer::GetPropertyText(wxString &strIn)
{
	wxString str;

	str.Printf(_T("Entity type: %hs\n"), SHPTypeName(m_nSHPType));
	strIn += str;

	str.Printf(_T("Entities: %d\n"), NumEntities());
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

		str.Printf(_T("Minimum Height: %.2f\n"), fmin);
		strIn += str;

		str.Printf(_T("Maximum Height: %.2f\n"), fmax);
		strIn += str;
	}
}

void vtRawLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	int etype, iEnt;
	double epsilon = pView->odx(6);  // calculate what 6 pixels is as world coord

	switch (ui.mode)
	{
	case LB_AddPoints:
		AddPoint(ui.m_DownLocation);
		SetModified(true);
		pView->Refresh();
		break;
	case LB_FeatInfo:
		etype = GetEntityType();
		if (etype != SHPT_POINT && etype != SHPT_POINTZ)
			return;

		iEnt = FindClosestPoint(ui.m_DownLocation, epsilon);
		if (iEnt != -1)
		{
			DPoint2 loc;
			GetPoint(iEnt, loc);
			Array<int> found;
			FindAllPointsAtLocation(loc, found);

			FeatInfoDlg	*fdlg = GetMainFrame()->ShowFeatInfoDlg();
			fdlg->SetFeatureSet(this);
			DePickAll();
			for (unsigned int i = 0; i < found.GetSize(); i++)
				Pick(found[i]);
			fdlg->ShowPicked();
		}
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of a GeoURL file.
////////////////////////////////////////////////////////////////////////

class VisitorGU : public XMLVisitor
{
public:
	VisitorGU(vtRawLayer *rl) : m_state(0), m_pLayer(rl) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

private:
	string m_data;
	int m_state;
	int m_rec;

	vtRawLayer *m_pLayer;
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

			m_rec = m_pLayer->AddPoint(p);

			val = atts.getValue("href");
			m_pLayer->SetValue(m_rec, 1, val);

			m_state = 2;
		}
	}
}

void VisitorGU::endElement(const char *name)
{
	if (m_state == 2)
	{
		m_pLayer->SetValue(m_rec, 0, m_data.c_str());
		m_state = 1;
	}
}

void VisitorGU::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

void vtRawLayer::ReadGeoURL()
{
	SetEntityType(SHPT_POINT);
	AddField("Name", FTString, 80);
	AddField("URL", FTString, 80);

	VisitorGU visitor(this);
	try
	{
		readXML("D:/Data-World/geourl2.xml", visitor);
	}
	catch (xh_exception &)
	{
		return;
	}
}

