//
// Features.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Features.h"
#include "xmlhelper/easyxml.hpp"
#include "vtLog.h"
#include "DLG.h"

//
// Construct / Destruct
//
vtFeatures::vtFeatures()
{
	m_iSHPElems = 0;
	m_iSHPFields = 0;
	m_eGeomType = wkbNone;
}

vtFeatures::~vtFeatures()
{
	Empty();
}

void vtFeatures::Empty()
{
	int count = m_fields.GetSize();
	for (int i = 0; i < count; i++)
	{
		Field *field = m_fields[i];
		delete field;
	}
	m_fields.SetSize(0);
}

//
// File IO
//
bool vtFeatures::LoadFrom(const char *filename)
{
	vtString fname = filename;
	vtString ext = fname.Right(3);
	if (!ext.CompareNoCase("shp"))
		return LoadFromSHP(filename);
	else
		return LoadWithOGR(filename);
}

bool vtFeatures::SaveToSHP(const char *filename) const
{
	int nSHPType = OGRToShapelib(m_eGeomType);
	SHPHandle hSHP = SHPCreate(filename, nSHPType);
	if (!hSHP)
		return false;

	unsigned int i, j, size;
	SHPObject *obj;
	DPoint2 p2;
	DPoint3 p3;
	if (nSHPType == SHPT_POINT)
	{
		size = m_Point2.GetSize();
		for (i = 0; i < size; i++)
		{
			// Save to SHP
			p2 = m_Point2[i];
			obj = SHPCreateSimpleObject(nSHPType, 1, &p2.x, &p2.y, NULL);
			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);
		}
	}
	if (nSHPType == SHPT_POINTZ)
	{
		size = m_Point3.GetSize();
		for (i = 0; i < size; i++)
		{
			// Save to SHP
			p3 = m_Point3[i];
			obj = SHPCreateSimpleObject(nSHPType, 1, &p3.x, &p3.y, &p3.z);
			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);
		}
	}
	if (nSHPType == SHPT_ARC || nSHPType == SHPT_POLYGON)
	{
		size = m_LinePoly.size();
		for (i = 0; i < size; i++)	//for each polyline
		{
			const DLine2 &dl = m_LinePoly[i];
			double* dX = new double[dl.GetSize()];
			double* dY = new double[dl.GetSize()];

			for (j=0; j < dl.GetSize(); j++) //for each vertex
			{
				DPoint2 pt = dl.GetAt(j);
				dX[j] = pt.x;
				dY[j] = pt.y;

			}
			// Save to SHP
			obj = SHPCreateSimpleObject(nSHPType, dl.GetSize(),
				dX, dY, NULL);

			delete dX;
			delete dY;

			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);
		}
	}
	SHPClose(hSHP);

	if (m_fields.GetSize() > 0)
	{
		// Save DBF File also
		vtString dbfname = filename;
		dbfname = dbfname.Left(dbfname.GetLength() - 4);
		dbfname += ".dbf";
		DBFHandle db = DBFCreate(dbfname);
		if (db == NULL)
		{
//			wxMessageBox("Couldn't create DBF file.");
			return false;
		}

		Field *field;
		for (i = 0; i < m_fields.GetSize(); i++)
		{
			field = m_fields[i];

			DBFAddField(db, (const char *) field->m_name, field->m_type,
				field->m_width, field->m_decimals );
		}

		// Write DBF Attributes, one record per entity
		unsigned int entities = GetNumEntities();
		for (i = 0; i < entities; i++)
		{
			for (j = 0; j < m_fields.GetSize(); j++)
			{
				field = m_fields[j];
				switch (field->m_type)
				{
				case FTLogical:
					DBFWriteLogicalAttribute(db, i, j, field->m_bool[i]);
					break;
				case FTInteger:
					DBFWriteIntegerAttribute(db, i, j, field->m_int[i]);
					break;
				case FTDouble:
					DBFWriteDoubleAttribute(db, i, j, field->m_double[i]);
					break;
				case FTString:
					DBFWriteStringAttribute(db, i, j, (const char *) field->m_string[i]);
					break;
				}
			}
		}
		DBFClose(db);
	}

	// Try saving projection to PRJ
	vtString prjname = filename;
	prjname = prjname.Left(prjname.GetLength() - 4);
	prjname += ".prj";
	m_proj.WriteProjFile(prjname);

	return true;
}

bool vtFeatures::LoadHeaderFromSHP(const char *filename)
{
	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(filename, "rb");
	if (hSHP == NULL)
		return false;

	m_strFilename = filename;

	// Get number of entities (nElem) and type of data (nShapeType)
	int		nShapeType;
	double	adfMinBound[4], adfMaxBound[4];

	SHPGetInfo(hSHP, &m_iSHPElems, &nShapeType, adfMinBound, adfMaxBound);

	//  Check shape type, we only support a few types
	switch (nShapeType)
	{
	case SHPT_POINT:
	case SHPT_POINTZ:
	case SHPT_ARC:
	case SHPT_POLYGON:
		m_eGeomType = ShapelibToOGR(nShapeType);
		break;
	default:
		SHPClose(hSHP);
		return false;
	}
	SHPClose(hSHP);

	// Try loading DBF File as well
	m_dbfname = filename;
	m_dbfname = m_dbfname.Left(m_dbfname.GetLength() - 4);
	m_dbfname += ".dbf";
	DBFFieldType fieldtype;
	DBFHandle db = DBFOpen(m_dbfname, "rb");
	int iField;
	if (db != NULL)
	{
		// Check for field of poly id, current default field in dbf is Id
		m_iSHPFields = DBFGetFieldCount(db);
		int pnWidth, pnDecimals;
		char szFieldName[80];

		for (iField = 0; iField < m_iSHPFields; iField++)
		{
			fieldtype = DBFGetFieldInfo(db, iField, szFieldName,
				&pnWidth, &pnDecimals);
			AddField(szFieldName, fieldtype, pnWidth);
		}
		DBFClose(db);
	}
	return true;
}

bool vtFeatures::LoadFromSHP(const char *filename)
{
	if (!LoadHeaderFromSHP(filename))
		return false;

	SHPHandle hSHP = SHPOpen(filename, "rb");
	DBFHandle db = DBFOpen(m_dbfname, "rb");

	// Initialize arrays
	switch (m_eGeomType)
	{
	case wkbPoint:
		m_Point2.SetSize(m_iSHPElems);
		break;
	case wkbPoint25D:
		m_Point3.SetSize(m_iSHPElems);
		break;
	case wkbLineString:
	case wkbPolygon:
		m_LinePoly.reserve(m_iSHPElems);
		break;
	}

	// Read Data from SHP into memory
	DPoint2 p2;
	DPoint3 p3;
	DLine2 dline;
	for (int i = 0; i < m_iSHPElems; i++)
	{
		// Get the i-th Shape in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		// beware - it is possible for the shape to not actually have any
		// vertices
		if (psShape->nVertices == 0)
		{
			switch (m_eGeomType)
			{
			case wkbPoint:
				p2.Set(0,0);
				m_Point2.SetAt(i, p2);
				break;
			case wkbPoint25D:
				p3.Set(0,0,0);
				m_Point3.SetAt(i, p3);
				break;
			case wkbLineString:
			case wkbPolygon:
				m_LinePoly[i] = dline;
				break;
			}
		}
		else
		{
			switch (m_eGeomType)
			{
			case wkbPoint:
				p2.x = *psShape->padfX;
				p2.y = *psShape->padfY;
				m_Point2.SetAt(i, p2);
				break;
			case wkbPoint25D:
				p3.x = *psShape->padfX;
				p3.y = *psShape->padfY;
				p3.z = *psShape->padfZ;
				m_Point3.SetAt(i, p3);
				break;
			case wkbLineString:
			case wkbPolygon:
				// Store each coordinate
				dline.SetSize(psShape->nVertices);
				for (int j = 0; j < psShape->nVertices; j++)
				{
					p2.x = psShape->padfX[j];
					p2.y = psShape->padfY[j];
					dline.SetAt(j, p2);
				}
				m_LinePoly.push_back(dline);
				break;
			}
		}
		SHPDestroyObject(psShape);

		// Read corresponding attributes (DBF record fields)
		int iField;
		if (db != NULL)
		{
			int rec = AddRecord();
			for (iField = 0; iField < m_iSHPFields; iField++)
			{
				Field *field = m_fields[iField];
				switch (field->m_type)
				{
				case FTString:
					SetValue(rec, iField, DBFReadStringAttribute(db, rec, iField));
					break;
				case FTInteger:
					SetValue(rec, iField, DBFReadIntegerAttribute(db, rec, iField));
					break;
				case FTDouble:
					SetValue(rec, iField, DBFReadDoubleAttribute(db, rec, iField));
					break;
				case FTLogical:
					SetValue(rec, iField, DBFReadLogicalAttribute(db, rec, iField));
					break;
				}
			}
		}
	}

	// Try loading projection from PRJ
	m_proj.ReadProjFile(filename);

	SHPClose(hSHP);
	if (db != NULL)
		DBFClose(db);

	// allocate selection array
	m_Flags.SetSize(m_iSHPElems);
	return true;
}


/////////////////////////////////////////////////////////////////////////////

/*
bool vtFeatures::LoadFromGML(const char *filename)
{
	// try using OGR
	g_GDALWrapper.RequestOGRFormats();

	OGRDataSource *pDatasource = OGRSFDriverRegistrar::Open( filename );
	if (!pDatasource)
		return false;

	int j, feature_count;
	DLine2		*dline;
	DPoint2		p2;

	OGRLayer		*pOGRLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRPoint		*pPoint;
	OGRLineString   *pLineString;
	OGRPolygon		*pPolygon;
	OGRLinearRing	*pRing;

	// Assume that this data source is a "flat feature" GML file
	//
	// Assume there is only 1 layer.
	//
	int num_layers = pDatasource->GetLayerCount();
	pOGRLayer = pDatasource->GetLayer(0);
	if (!pOGRLayer)
		return false;

	feature_count = pOGRLayer->GetFeatureCount();
  	pOGRLayer->ResetReading();
	OGRFeatureDefn *defn = pOGRLayer->GetLayerDefn();
	if (!defn)
		return false;
	const char *layer_name = defn->GetName();

	int iFields = defn->GetFieldCount();
	for (j = 0; j < iFields; j++)
	{
		OGRFieldDefn *field_def1 = defn->GetFieldDefn(j);
		if (field_def1)
		{
			const char *fnameref = field_def1->GetNameRef();
			OGRFieldType ftype = field_def1->GetType();

			switch (ftype)
			{
			case OFTInteger:
				AddField(fnameref, FTInteger);
				break;
			case OFTReal:
				AddField(fnameref, FTDouble);
				break;
			case OFTString:
				AddField(fnameref, FTString);
				break;
			}
		}
	}

	// Get the projection (SpatialReference) from this layer?  We can't,
	// because for current GML the layer doesn't have it; must use the
	// first Geometry instead.
//	OGRSpatialReference *pSpatialRef = pOGRLayer->GetSpatialRef();

	// Look at the first geometry of the first feature in order to know
	// what kind of primitive this file has.
	bool bFirst = true;
	OGRwkbGeometryType eType;
	int num_points;
	int fcount = 0;

	while( (pFeature = pOGRLayer->GetNextFeature()) != NULL )
	{
		pGeom = pFeature->GetGeometryRef();
		if (!pGeom) continue;

		if (bFirst)
		{
			OGRSpatialReference *pSpatialRef = pGeom->getSpatialReference();
			if (pSpatialRef)
				m_proj.SetSpatialReference(pSpatialRef);

			eType = pGeom->getGeometryType();
			_SetupFromOGCType(eType);
			bFirst = false;
		}

		switch (eType)
		{
		case wkbPoint:
			pPoint = (OGRPoint *) pGeom;
			p2.Set(pPoint->getX(), pPoint->getY());
			m_Point2.Append(p2);
			break;
		case wkbLineString:
			pLineString = (OGRLineString *) pGeom;
			num_points = pLineString->getNumPoints();
			dline = new DLine2;
			dline->SetSize(num_points);
			for (j = 0; j < num_points; j++)
				dline->SetAt(j, DPoint2(pLineString->getX(j), pLineString->getY(j)));
			m_LinePoly.Append(dline);
			break;
		case wkbPolygon:
			pPolygon = (OGRPolygon *) pGeom;
			pRing = pPolygon->getExteriorRing();
			int num_points = pRing->getNumPoints();
			dline = new DLine2;
			dline->SetSize(num_points);
			for (j = 0; j < num_points; j++)
				dline->SetAt(j, DPoint2(pRing->getX(j), pRing->getY(j)));
			m_LinePoly.Append(dline);
			break;
		}

		for (j = 0; j < iFields; j++)
		{
			Field *field = GetField(j);
			switch (field->m_type)
			{
			case FTInteger:
				field->m_int.Append(pFeature->GetFieldAsInteger(j));
				break;
			case FTDouble:
				field->m_double.Append(pFeature->GetFieldAsDouble(j));
				break;
			case FTString:
				field->m_string.push_back(vtString(pFeature->GetFieldAsString(j)));
				break;
			}
		}
		fcount++;
	}

	delete pDatasource;

	// allocate selection array
	m_Flags.SetSize(fcount);

	return true;
}
*/

bool vtFeatures::LoadWithOGR(const char *filename,
							 void progress_callback(int))
{
	// try using OGR
	g_GDALWrapper.RequestOGRFormats();

	OGRDataSource *pDatasource = OGRSFDriverRegistrar::Open( filename );
	if (!pDatasource)
		return false;

	int i, j, feature_count, count;
	bool bGotCS = false;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;

	// Don't iterate through the layers, there should be only one.
	//
	int num_layers = pDatasource->GetLayerCount();
	if (!num_layers)
		return false;

	pLayer = pDatasource->GetLayer(0);
	if (!pLayer)
		return false;

	// Get basic information about the layer we're reading
	feature_count = pLayer->GetFeatureCount();
  	pLayer->ResetReading();
	OGRFeatureDefn *defn = pLayer->GetLayerDefn();
	if (!defn)
		return false;

	const char *layer_name = defn->GetName();
	int num_fields = defn->GetFieldCount();
	OGRwkbGeometryType geom_type = defn->GetGeomType();

	// Get the projection (SpatialReference) from this layer, if we can.
	// Sometimes (e.g. for GML) the layer doesn't have it; may have to
	// use the first Geometry instead.
	OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
	if (pSpatialRef)
	{
		m_proj.SetSpatialReference(pSpatialRef);
		bGotCS = true;
	}

	// Convert from OGR to our geometry type
	switch (geom_type)
	{
	case wkbPoint:
	case wkbLineString:
	case wkbPolygon:
	case wkbPoint25D:
		m_eGeomType = geom_type;
		break;
	case wkbUnknown:
		// This usually indicates that the file contains a mix of different
		// geometry types.  Look at the first geometry.
		pFeature = pLayer->GetNextFeature();
		pGeom = pFeature->GetGeometryRef();
		m_eGeomType = pGeom->getGeometryType();
		break;
	default:
		return false;	// don't know what to do with this geom type
	}

	// We're going to read the file now, so take it's name
	m_strFilename = filename;

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
	switch (m_eGeomType)
	{
	case wkbPoint:
		m_Point2.SetMaxSize(feature_count);
		break;
	case wkbPoint25D:
		m_Point3.SetMaxSize(feature_count);
		break;
	case wkbLineString:
	case wkbPolygon:
		m_LinePoly.reserve(feature_count);
		break;
	}

	// Read Data from OGR into memory
	DPoint2 p2;
	DPoint3 p3;
	int num_geoms, num_points;
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

		if (!bGotCS)
		{
			OGRSpatialReference *pSpatialRef = pGeom->getSpatialReference();
			if (pSpatialRef)
			{
				m_proj.SetSpatialReference(pSpatialRef);
				bGotCS = true;
			}
		}
		// Beware - some OGR-supported formats, such as MapInfo,
		//  will have more than one kind of geometry per layer,
		//  for example, both LineString and MultiLineString
		// Get the geometry type from the Geometry, not the Layer.
		geom_type = pGeom->getGeometryType();
		num_geoms = 1;

		DLine2 dline;

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
			dline.SetSize(num_points);
			for (j = 0; j < num_points; j++)
			{
				p2.Set(pLineString->getX(j), pLineString->getY(j));
				dline.SetAt(j, p2);
			}
			m_LinePoly.push_back(dline);
			break;
		case wkbMultiLineString:
			pMulti = (OGRMultiLineString *) pGeom;
			num_geoms = pMulti->getNumGeometries();
			for (i = 0; i < num_geoms; i++)
			{
				pLineString = (OGRLineString *) pMulti->getGeometryRef(i);
				num_points = pLineString->getNumPoints();
				dline.SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p2.Set(pLineString->getX(j), pLineString->getY(j));
					dline.SetAt(j, p2);
				}
				m_LinePoly.push_back(dline);
			}
			break;
		case wkbPolygon:
			pPolygon = (OGRPolygon *) pGeom;
			pRing = pPolygon->getExteriorRing();
			num_points = pRing->getNumPoints();
			dline.SetSize(num_points);
			for (j = 0; j < num_points; j++)
			{
				dline.SetAt(j, DPoint2(pRing->getX(j), pRing->getY(j)));
			}
			m_LinePoly.push_back(dline);
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
		// a record with attributes for each one.
		for (i = 0; i < num_geoms; i++)
		{
			AddRecord();

			for (j = 0; j < num_fields; j++)
			{
				Field *pField = GetField(j);
				switch (pField->m_type)
				{
				case FTLogical:
					SetValue(count, j, pFeature->GetFieldAsInteger(j) != 0);
					break;
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
		// track total features
		feature_count += (num_geoms-1);

		// features returned from OGRLayer::GetNextFeature are our responsibility to delete!
		delete pFeature;
	}
	delete pDatasource;
	return true;
}


bool vtFeatures::AddElementsFromDLG(class vtDLGFile *pDLG)
{
	// A DLG file can be fairly directly interpreted as features, since
	// it consists of nodes, areas, and lines.  However, topology is lost
	// and we must pick which of the three to display.

	int i;
	int nodes = pDLG->m_iNodes, areas = pDLG->m_iAreas, lines = pDLG->m_iLines;
	if (nodes > lines)
	{
		SetGeomType(wkbPoint);
		for (i = 0; i < nodes; i++)
			AddPoint(pDLG->m_nodes[i].m_p);
	}
/*
	else if (areas >= nodes && areas >= lines)
	{
		// "Areas" in a DLG area actually points which indicate an interior
		//  point in a polygon defined by some of the "lines"
		SetEntityType(SHPT_POLYGON);
		for (i = 0; i < areas; i++)
			AddPolyLine(&pDLG->m_areas[i].m_p);
	}
*/
	else
	{
		SetGeomType(wkbLineString);
		for (i = 0; i < areas; i++)
			AddPolyLine(pDLG->m_lines[i].m_p);
	}
	m_proj = pDLG->GetProjection();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// feature (entity) operations
//

int vtFeatures::GetNumEntities() const
{
	if (m_eGeomType == wkbPoint)
		return m_Point2.GetSize();
	else if (m_eGeomType == wkbPoint25D)
		return m_Point3.GetSize();
	else if (m_eGeomType == wkbLineString || m_eGeomType == wkbPolygon)
		return m_LinePoly.size();
	else
		return -1;
}

void vtFeatures::SetNumEntities(int iNum)
{
	if (m_eGeomType == wkbPoint)
		m_Point2.SetSize(iNum);
	else if (m_eGeomType == wkbPoint25D)
		m_Point3.SetSize(iNum);
	else if (m_eGeomType == wkbLineString || m_eGeomType == wkbPolygon)
		m_LinePoly.resize(iNum);
}

/**
 * Returns the type of geometry that each feature has.
 *
 * \return
 *		- wkbPoint for 2D points
 *		- wkbPoint25D fpr 3D points
 *		- wkbLineString for 2D polylines
 *		- wkbPolygon for 2D polygons
 */
OGRwkbGeometryType vtFeatures::GetGeomType() const
{
	return m_eGeomType;
}

/**
 * Set the type of geometry that each feature will have.
 *
 * \param type
 *		- wkbPoint for 2D points
 *		- wkbPoint25D fpr 3D points
 *		- wkbLineString for 2D polylines
 *		- wkbPolygon for 2D polygons
 */
void vtFeatures::SetGeomType(OGRwkbGeometryType eGeomType)
{
	m_eGeomType = eGeomType;
}

int vtFeatures::AddPoint(const DPoint2 &p)
{
	int rec = -1;
	if (m_eGeomType == wkbPoint)
	{
		rec = m_Point2.Append(p);
		AddRecord();
	}
	return rec;
}

int vtFeatures::AddPoint(const DPoint3 &p)
{
	int rec = -1;
	if (m_eGeomType == wkbPoint25D)
	{
		rec = m_Point3.Append(p);
		AddRecord();
	}
	return rec;
}

int vtFeatures::AddPolyLine(const DLine2 &pl)
{
	int rec = -1;
	if (m_eGeomType == wkbLineString || m_eGeomType == wkbPolygon)
	{
		m_LinePoly.push_back(pl);
		rec = m_LinePoly.size()-1;
		AddRecord();
	}
	return rec;
}

void vtFeatures::SetPoint(unsigned int num, const DPoint2 &p)
{
	if (m_eGeomType == wkbPoint)
		m_Point2.SetAt(num, p);
}

void vtFeatures::GetPoint(unsigned int num, DPoint3 &p) const
{
	if (m_eGeomType == wkbPoint)
	{
		DPoint2 p2 = m_Point2.GetAt(num);
		p.x = p2.x;
		p.y = p2.y;
		p.z = 0;
	}
	if (m_eGeomType == wkbPoint25D)
	{
		p = m_Point3.GetAt(num);
	}
}

void vtFeatures::GetPoint(unsigned int num, DPoint2 &p) const
{
	if (m_eGeomType == wkbPoint25D)
	{
		DPoint3 p3 = m_Point3.GetAt(num);
		p.x = p3.x;
		p.y = p3.y;
	}
	if (m_eGeomType == wkbPoint)
	{
		p = m_Point2.GetAt(num);
	}
}

int vtFeatures::FindClosestPoint(const DPoint2 &p, double epsilon)
{
	int entities = GetNumEntities();
	double dist, closest = 1E9;
	int found = -1;
	DPoint2 diff;

	int i;
	for (i = 0; i < entities; i++)
	{
		if (m_eGeomType == wkbPoint)
			diff = p - m_Point2.GetAt(i);
		if (m_eGeomType == wkbPoint25D)
		{
			DPoint3 p3 = m_Point3.GetAt(i);
			diff.x = p.x - p3.x;
			diff.y = p.y - p3.y;
		}
		dist = diff.Length();
		if (dist < closest && dist < epsilon)
		{
			closest = dist;
			found = i;
		}
	}
	return found;
}

void vtFeatures::FindAllPointsAtLocation(const DPoint2 &loc, Array<int> &found)
{
	int entities = GetNumEntities();

	int i;
	for (i = 0; i < entities; i++)
	{
		if (m_eGeomType == wkbPoint)
		{
			if (loc == m_Point2.GetAt(i))
				found.Append(i);
		}
		if (m_eGeomType == wkbPoint25D)
		{
			DPoint3 p3 = m_Point3.GetAt(i);
			if (loc.x == p3.x && loc.y == p3.y)
				found.Append(i);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Selection of Entities

int vtFeatures::NumSelected()
{
	int count = 0;
	int size = m_Flags.GetSize();
	for (int i = 0; i < size; i++)
		if (m_Flags[i] & FF_SELECTED)
			count++;
	return count;
}

void vtFeatures::DeselectAll()
{
	for (unsigned int i = 0; i < m_Flags.GetSize(); i++)
		m_Flags[i] &= ~FF_SELECTED;
}

void vtFeatures::InvertSelection()
{
	for (unsigned int i = 0; i < m_Flags.GetSize(); i++)
		m_Flags[i] ^= FF_SELECTED;
}

int vtFeatures::DoBoxSelect(const DRECT &rect, SelectionType st)
{
	int affected = 0;
	int entities = GetNumEntities();

	bool bIn;
	bool bWas;
	for (int i = 0; i < entities; i++)
	{
		bWas = (m_Flags[i] & FF_SELECTED);
		if (st == ST_NORMAL)
			Select(i, false);

		if (m_eGeomType == wkbPoint)
			bIn = rect.ContainsPoint(m_Point2[i]);

		if (m_eGeomType == wkbPoint25D)
			bIn = rect.ContainsPoint(DPoint2(m_Point3[i].x, m_Point3[i].y));

		if (m_eGeomType == wkbLineString || m_eGeomType == wkbPolygon)
			bIn = rect.ContainsLine(m_LinePoly[i]);

		if (!bIn)
			continue;

		switch (st)
		{
		case ST_NORMAL:
			Select(i, true);
			affected++;
			break;
		case ST_ADD:
			Select(i, true);
			if (!bWas) affected++;
			break;
		case ST_SUBTRACT:
			Select(i, false);
			if (bWas) affected++;
			break;
		case ST_TOGGLE:
			Select(i, !bWas);
			affected++;
			break;
		}
	}
	return affected;
}

int vtFeatures::SelectByCondition(int iField, int iCondition,
								  const char *szValue)
{
	bool bval, btest;
	int i, ival, itest;
	double dval, dtest;
	int entities = GetNumEntities(), selected = 0;
	int con = iCondition;
	bool result;

	if (iField < 0)
	{
		dval = atof(szValue);
		for (i = 0; i < entities; i++)
		{
			// special field numbers are used to refer to the spatial components
			if (m_eGeomType == wkbPoint)
			{
				if (iField == -1) dtest = m_Point2[i].x;
				if (iField == -2) dtest = m_Point2[i].y;
				if (iField == -3) return -1;
			}
			else if (m_eGeomType == wkbPoint25D)
			{
				if (iField == -1) dtest = m_Point3[i].x;
				if (iField == -2) dtest = m_Point3[i].y;
				if (iField == -3) dtest = m_Point3[i].z;
			}
			else
				return -1;	// TODO: support non-point types
			if (con == 0) result = (dtest == dval);
			if (con == 1) result = (dtest > dval);
			if (con == 2) result = (dtest < dval);
			if (con == 3) result = (dtest >= dval);
			if (con == 4) result = (dtest <= dval);
			if (con == 5) result = (dtest != dval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		return selected;
	}
	Field *field = m_fields[iField];
	switch (field->m_type)
	{
	case FTString:
		for (i = 0; i < entities; i++)
		{
			const vtString &sp = field->m_string[i];
			if (con == 0) result = (sp.Compare(szValue) == 0);
			if (con == 1) result = (sp.Compare(szValue) > 0);
			if (con == 2) result = (sp.Compare(szValue) < 0);
			if (con == 3) result = (sp.Compare(szValue) >= 0);
			if (con == 4) result = (sp.Compare(szValue) <= 0);
			if (con == 5) result = (sp.Compare(szValue) != 0);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FTInteger:
		ival = atoi(szValue);
		for (i = 0; i < entities; i++)
		{
			itest = field->m_int[i];
			if (con == 0) result = (itest == ival);
			if (con == 1) result = (itest > ival);
			if (con == 2) result = (itest < ival);
			if (con == 3) result = (itest >= ival);
			if (con == 4) result = (itest <= ival);
			if (con == 5) result = (itest != ival);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FTDouble:
		dval = atof(szValue);
		for (i = 0; i < entities; i++)
		{
			dtest = field->m_double[i];
			if (con == 0) result = (dtest == dval);
			if (con == 1) result = (dtest > dval);
			if (con == 2) result = (dtest < dval);
			if (con == 3) result = (dtest >= dval);
			if (con == 4) result = (dtest <= dval);
			if (con == 5) result = (dtest != dval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FTLogical:
		bval = (atoi(szValue) != 0);
		for (i = 0; i < entities; i++)
		{
			btest = field->m_bool[i];
			if (con == 0) result = (btest == bval);
//			if (con == 1) result = (btest > ival);
//			if (con == 2) result = (btest < ival);
//			if (con == 3) result = (btest >= ival);
//			if (con == 4) result = (btest <= ival);
			if (con > 0 && con < 5)
				continue;
			if (con == 5) result = (btest != bval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	}
	return selected;
}

void vtFeatures::DeleteSelected()
{
	int i, entities = GetNumEntities();
	for (i = 0; i < entities; i++)
	{
		if (IsSelected(i))
		{
			Select(i, false);
			SetToDelete(i);
		}
	}
	ApplyDeletion();
}

void vtFeatures::SetToDelete(int iFeature)
{
	m_Flags[iFeature] |= FF_DELETE;
}

void vtFeatures::ApplyDeletion()
{
	int i, entities = GetNumEntities();

	int target = 0;
	int newtotal = entities;
	for (i = 0; i < entities; i++)
	{
		if (! (m_Flags[i] & FF_DELETE))
		{
			if (target != i)
			{
				CopyEntity(i, target);
				m_Flags[target] = m_Flags[i];
			}
			target++;
		}
		else
			newtotal--;
	}
	_ShrinkGeomArraySize(newtotal);
}

void vtFeatures::_ShrinkGeomArraySize(int size)
{
	if (m_eGeomType == wkbPoint)
		m_Point2.SetSize(size);
	if (m_eGeomType == wkbPoint25D)
		m_Point3.SetSize(size);
	// TODO: check this, it might leak some memory by not freeing
	//  the linepoly which is dropped off the end
	if (m_eGeomType == wkbLineString || m_eGeomType == wkbPolygon)
		m_LinePoly.resize(size);
}

void vtFeatures::CopyEntity(unsigned int from, unsigned int to)
{
	// copy geometry
	if (m_eGeomType == wkbPoint)
	{
		m_Point2[to] = m_Point2[from];
	}
	if (m_eGeomType == wkbPoint25D)
	{
		m_Point3[to] = m_Point3[from];
	}
	if (m_eGeomType == wkbLineString || m_eGeomType == wkbPolygon)
	{
		m_LinePoly[to] = m_LinePoly[from];
	}
	// copy record
	for (unsigned int i = 0; i < m_fields.GetSize(); i++)
	{
		m_fields[i]->CopyValue(from, to);
	}
}

void vtFeatures::DePickAll()
{
	int i, entities = GetNumEntities();
	for (i = 0; i < entities; i++)
		m_Flags[i] &= ~FF_PICKED;
}


/////////////////////////////////////////////////////////////////////////////
// Data Fields

Field *vtFeatures::GetField(const char *name)
{
	int i, num = m_fields.GetSize();
	for (i = 0; i < num; i++)
	{
		if (!m_fields[i]->m_name.CompareNoCase(name))
			return m_fields[i];
	}
	return NULL;
}

/**
 * Return the index of the field with the given name, or -1 if no field
 * with that name was found.
 */
int vtFeatures::GetFieldIndex(const char *name) const
{
	unsigned int i, num = m_fields.GetSize();
	for (i = 0; i < num; i++)
	{
		if (!m_fields[i]->m_name.CompareNoCase(name))
			return i;
	}
	return -1;
}

int vtFeatures::AddField(const char *name, DBFFieldType ftype, int string_length)
{
	Field *f = new Field(name, ftype);
	if (ftype == FTInteger)
	{
		f->m_width = 11;
		f->m_decimals = 0;
	}
	else if (ftype == FTDouble)
	{
		f->m_width = 12;
		f->m_decimals = 12;
	}
	else if (ftype == FTLogical)
	{
		f->m_width = 1;
		f->m_decimals = 0;
	}
	else if (ftype == FTString)
	{
		f->m_width = string_length;
		f->m_decimals = 0;
	}
	else
	{
		VTLOG("Attempting to add field '%s' of type 'invalid', adding an integer field instead.\n", name, ftype);
		f->m_type = FTInteger;
		f->m_width = 1;
		f->m_decimals = 0;
	}
	return m_fields.Append(f);
}

int vtFeatures::AddRecord()
{
	int recs;
	for (unsigned int i = 0; i < m_fields.GetSize(); i++)
	{
		recs = m_fields[i]->AddRecord();
	}
	m_Flags.Append(0);
	return recs;
}

void vtFeatures::SetValue(unsigned int record, unsigned int field, const char *value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatures::SetValue(unsigned int record, unsigned int field, int value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatures::SetValue(unsigned int record, unsigned int field, double value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatures::SetValue(unsigned int record, unsigned int field, bool value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatures::GetValueAsString(unsigned int iRecord, unsigned int iField, vtString &str) const
{
	Field *field = m_fields[iField];
	field->GetValueAsString(iRecord, str);
}

void vtFeatures::SetValueFromString(unsigned int iRecord, unsigned int iField, const vtString &str)
{
	Field *field = m_fields[iField];
	field->SetValueFromString(iRecord, str);
}

void vtFeatures::SetValueFromString(unsigned int iRecord, unsigned int iField, const char *str)
{
	Field *field = m_fields[iField];
	field->SetValueFromString(iRecord, str);
}

int vtFeatures::GetIntegerValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	return field->m_int[iRecord];
}

double vtFeatures::GetDoubleValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	return field->m_double[iRecord];
}

bool vtFeatures::GetBoolValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	return field->m_bool[iRecord];
}

/////////////////////////////////////////////////

//
// Fields
//
Field::Field(const char *name, DBFFieldType ftype)
{
	m_name = name;
	m_type = ftype;
}

Field::~Field()
{
}

int Field::AddRecord()
{
	int index = 0;
	switch (m_type)
	{
	case FTLogical: return	m_bool.Append(false);	break;
	case FTInteger: return	m_int.Append(0);		break;
	case FTDouble:	return	m_double.Append(0.0);	break;
	case FTString:
		index = m_string.size();
		m_string.push_back(vtString(""));
		return index;
	}
	return -1;
}

void Field::SetValue(unsigned int record, const char *value)
{
	if (m_type != FTString)
		return;
	m_string[record] = value;
}

void Field::SetValue(unsigned int record, int value)
{
	if (m_type == FTInteger)
		m_int[record] = value;
	else if (m_type == FTDouble)
		m_double[record] = value;
}

void Field::SetValue(unsigned int record, double value)
{
	if (m_type == FTInteger)
		m_int[record] = (int) value;
	else if (m_type == FTDouble)
		m_double[record] = value;
}

void Field::SetValue(unsigned int record, bool value)
{
	if (m_type == FTInteger)
		m_int[record] = (int) value;
	else if (m_type == FTLogical)
		m_bool[record] = value;
}

void Field::GetValue(unsigned int record, vtString &string)
{
	if (m_type != FTString)
		return;
	string = m_string[record];
}

void Field::GetValue(unsigned int record, int &value)
{
	if (m_type == FTInteger)
		value = m_int[record];
	else if (m_type == FTDouble)
		value = (int) m_double[record];
	else if (m_type == FTLogical)
		value = (int) m_bool[record];
}

void Field::GetValue(unsigned int record, double &value)
{
	if (m_type == FTInteger)
		value = (double) m_int[record];
	else if (m_type == FTDouble)
		value = m_double[record];
}

void Field::GetValue(unsigned int record, bool &value)
{
	if (m_type == FTInteger)
		value = (m_int[record] != 0);
	else if (m_type == FTLogical)
		value = m_bool[record];
}

void Field::CopyValue(unsigned int FromRecord, int ToRecord)
{
	if (m_type == FTInteger)
		m_int[ToRecord] = m_int[FromRecord];
	if (m_type == FTDouble)
		m_double[ToRecord] = m_double[FromRecord];

	// when dealing with strings, copy by value not reference, to
	// avoid memory tracking issues
	if (m_type == FTString)
		m_string[ToRecord] = m_string[FromRecord];
}

void Field::GetValueAsString(unsigned int iRecord, vtString &str)
{
	switch (m_type)
	{
	case FTString:
		str = m_string[iRecord];
		break;
	case FTInteger:
		str.Format("%d", m_int[iRecord]);
		break;
	case FTDouble:
		str.Format("%lf", m_double[iRecord]);
		break;
	}
}

void Field::SetValueFromString(unsigned int iRecord, const vtString &str)
{
	const char *cstr = str;
	SetValueFromString(iRecord, cstr);
}

void Field::SetValueFromString(unsigned int iRecord, const char *str)
{
	int i;
	double d;

	switch (m_type)
	{
	case FTString:
		if (iRecord < (int) m_string.size())
			m_string[iRecord] = str;
		else
			m_string.push_back(vtString(str));
		break;
	case FTInteger:
		i = atoi(str);
		if (iRecord < m_int.GetSize())
			m_int[iRecord] = i;
		else
			m_int.Append(i);
		break;
	case FTDouble:
		d = atof(str);
		if (iRecord < m_double.GetSize())
			m_double[iRecord] = d;
		else
			m_double.Append(d);
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Helpers

/**
 * Convert a Shapelib geometry type to an OGR type.
 */
OGRwkbGeometryType ShapelibToOGR(int nSHPType)
{
	switch (nSHPType)
	{
	case SHPT_NULL: return wkbNone;
	case SHPT_POINT: return wkbPoint;
	case SHPT_ARC: return wkbLineString;
	case SHPT_POLYGON: return wkbPolygon;
	case SHPT_MULTIPOINT: return wkbMultiPoint;
	case SHPT_POINTZ: return wkbPoint25D;
	case SHPT_ARCZ: return wkbLineString25D;
	case SHPT_POLYGONZ: return wkbPolygon25D;

	// the following are guesses
	case SHPT_MULTIPOINTZ: return wkbMultiPoint25D;
	case SHPT_POINTM: return wkbMultiPoint;
	case SHPT_ARCM: return wkbMultiLineString;
	case SHPT_POLYGONM: return wkbMultiPolygon;
	case SHPT_MULTIPOINTM: return wkbUnknown;
	case SHPT_MULTIPATCH: return wkbUnknown;
	}
	return wkbUnknown;
}

/**
 * Convert a OGR geometry type to an Shapelib type.
 */
int OGRToShapelib(OGRwkbGeometryType eGeomType)
{
	switch (eGeomType)
	{
	// some of the following are guesses
	case wkbUnknown: return SHPT_NULL;
	case wkbPoint: return SHPT_POINT;
	case wkbLineString: return SHPT_ARC;
	case wkbPolygon: return SHPT_POLYGON;
	case wkbMultiPoint: return SHPT_MULTIPOINT;
	case wkbMultiLineString: return SHPT_ARCM;
	case wkbMultiPolygon: return SHPT_POLYGONM;

	case wkbGeometryCollection: return SHPT_NULL;
	case wkbNone: return SHPT_NULL;

	case wkbPoint25D: return SHPT_POINTZ;
	case wkbLineString25D: return SHPT_ARCZ;
	case wkbPolygon25D: return SHPT_POLYGONZ;

	case wkbMultiPoint25D: return SHPT_MULTIPOINTZ;
	case wkbMultiLineString25D: return SHPT_NULL;
	case wkbGeometryCollection25D: return SHPT_NULL;
	}
	return SHPT_NULL;
}

