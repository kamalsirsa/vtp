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
		return LoadFromGML(m_strFilename);
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

void vtRawLayer::AddElementsFromOGR(OGRDataSource *pDatasource,
									 void progress_callback(int))
{
	int i, j, feature_count, count;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;

	// Don't iterate through the layers, there should be only one.
	//
	int num_layers = pDatasource->GetLayerCount();
	if (!num_layers)
		return;

	pLayer = pDatasource->GetLayer(0);
	if (!pLayer)
		return;

	// Get basic information about the layer we're reading
	feature_count = pLayer->GetFeatureCount();
  	pLayer->ResetReading();
	OGRFeatureDefn *defn = pLayer->GetLayerDefn();
	if (!defn)
		return;

	const char *layer_name = defn->GetName();
	int num_fields = defn->GetFieldCount();
	OGRwkbGeometryType geom_type = defn->GetGeomType();

	// Convert from OGR to our geometry type
	m_nSHPType = SHPT_NULL;
	while (m_nSHPType == SHPT_NULL)
	{
		switch (geom_type)
		{
			case wkbPoint:
				m_nSHPType = SHPT_POINT;
				break;
			case wkbLineString:
			case wkbMultiLineString:
				m_nSHPType = SHPT_ARC;
				break;
			case wkbPolygon:
				m_nSHPType = SHPT_POLYGON;
				break;
			case wkbPoint25D:
				m_nSHPType = SHPT_POINTZ;
				break;
			case wkbUnknown:
				// This usually indicates that the file contains a mix of different
				// geometry types.  Look at the first geometry.
				pFeature = pLayer->GetNextFeature();
				pGeom = pFeature->GetGeometryRef();
				geom_type = pGeom->getGeometryType();
				break;
			default:
				return;		// don't know what to do with this geom type
		}
	}

	for (j = 0; j < num_fields; j++)
	{
		OGRFieldDefn *field_def = defn->GetFieldDefn(j);
		const char *field_name = field_def->GetNameRef();
		OGRFieldType field_type = field_def->GetType();
		int width = field_def->GetWidth();

		DBFFieldType ftype;
		switch (field_type)
		{
		case OFTInteger:
			ftype = FTInteger;
			break;
		case OFTReal:
			ftype = FTDouble;
			break;
		case OFTString:
			ftype = FTString;
			break;
		default:
			continue;
		}
		AddField(field_name, ftype, width);
	}

	// Initialize arrays
	switch (m_nSHPType)
	{
	case SHPT_POINT:
		m_Point2.SetMaxSize(feature_count);
		break;
	case SHPT_POINTZ:
		m_Point3.SetMaxSize(feature_count);
		break;
	case SHPT_ARC:
	case SHPT_POLYGON:
		m_LinePoly.SetMaxSize(feature_count);
		break;
	}

	// Read Data from OGR into memory
	DPoint2 p2;
	DPoint3 p3;
	int num_geoms, num_points;
	DLine2 *new_poly;
	OGRPoint		*pPoint;
	OGRPolygon		*pPolygon;
	OGRLinearRing	*pRing;
	OGRLineString   *pLineString;
	OGRMultiLineString   *pMulti;

	pLayer->ResetReading();
	count = 0;
	while( (pFeature = pLayer->GetNextFeature()) != NULL )
	{
		if (progress_callback != NULL)
			progress_callback(count * 100 / feature_count);

		pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;

		// Beware - some OGR-supported formats, such as MapInfo,
		//  will have more than one kind of geometry per layer,
		//  for example, both LineString and MultiLineString
		// Get the geometry type from the Geometry, not the Layer.
		geom_type = pGeom->getGeometryType();
		num_geoms = 1;

		switch (geom_type)
		{
		case wkbPoint:
			pPoint = (OGRPoint *) pGeom;
			p2.x = pPoint->getX();
			p2.y = pPoint->getY();
			m_Point2.Append(p2);
			break;
		case wkbPoint25D:
			pPoint = (OGRPoint *) pGeom;
			p3.x = pPoint->getX();
			p3.y = pPoint->getY();
			p3.z = pPoint->getZ();
			m_Point3.Append(p3);
			break;
		case wkbLineString:
			pLineString = (OGRLineString *) pGeom;
			num_points = pLineString->getNumPoints();
			new_poly = new DLine2();
			new_poly->SetSize(num_points);
			for (j = 0; j < num_points; j++)
			{
				p2.Set(pLineString->getX(j), pLineString->getY(j));
				new_poly->SetAt(j, p2);
			}
			m_LinePoly.Append(new_poly);
			break;
		case wkbMultiLineString:
			pMulti = (OGRMultiLineString *) pGeom;
			num_geoms = pMulti->getNumGeometries();
			for (i = 0; i < num_geoms; i++)
			{
				pLineString = (OGRLineString *) pMulti->getGeometryRef(i);
				num_points = pLineString->getNumPoints();
				new_poly = new DLine2();
				new_poly->SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p2.Set(pLineString->getX(j), pLineString->getY(j));
					new_poly->SetAt(j, p2);
				}
				m_LinePoly.Append(new_poly);
			}
			break;
		case wkbPolygon:
			pPolygon = (OGRPolygon *) pGeom;
			pRing = pPolygon->getExteriorRing();
			num_points = pRing->getNumPoints();
			new_poly = new DLine2();
			new_poly->SetSize(num_points);
			for (j = 0; j < num_points; j++)
			{
				new_poly->SetAt(j, DPoint2(pRing->getX(j), pRing->getY(j)));
			}
			m_LinePoly.Append(new_poly);
			break;
		case wkbMultiPoint:
		case wkbMultiPolygon:
		case wkbGeometryCollection:
			// Hopefully we won't encounter unexpected geometries, but
			// if we do, just skip them for now.
			continue;
			break;
		}

		// In case more than one geometry was encountered, we need to add
		// a record w/ attributes for each one.
		for (i = 0; i < num_geoms; i++)
		{
			AddRecord();

			for (j = 0; j < num_fields; j++)
			{
				Field *pField = GetField(j);
				switch (pField->m_type)
				{
				case FTInteger:
					SetValue(count, j, pFeature->GetFieldAsInteger(j));
					break;
				case FTDouble:
					SetValue(count, j, pFeature->GetFieldAsDouble(j));
					break;
				case FTString:
					SetValue(count, j, pFeature->GetFieldAsString(j));
					break;
				}
			}
			count++;
		}
	}
}


