//
// Features.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Features.h"
#include "xmlhelper/easyxml.hpp"
#include "vtLog.h"
#include "DLG.h"

//
// Construct / Destruct
//
vtFeatureSet::vtFeatureSet()
{
	m_eGeomType = wkbNone;
}

vtFeatureSet::~vtFeatureSet()
{
	DeleteFields();
}

void vtFeatureSet::DeleteFields()
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
bool vtFeatureSet::SaveToSHP(const char *filename) const
{
	int nSHPType = OGRToShapelib(m_eGeomType);
	SHPHandle hSHP = SHPCreate(filename, nSHPType);
	if (!hSHP)
		return false;

	unsigned int i, j;

	SaveGeomToSHP(hSHP);
	SHPClose(hSHP);

	if (m_fields.GetSize() > 0)
	{
		// Save DBF File also
		vtString dbfname = filename;
		dbfname = dbfname.Left(dbfname.GetLength() - 4);
		dbfname += ".dbf";
		DBFHandle db = DBFCreate(dbfname);
		if (db == NULL)
			return false;

		Field *field;
		for (i = 0; i < m_fields.GetSize(); i++)
		{
			field = m_fields[i];

			DBFFieldType dbtype = ConvertFieldType(field->m_type);
			DBFAddField(db, (const char *) field->m_name, dbtype,
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
				case FT_Boolean:
					DBFWriteLogicalAttribute(db, i, j, field->m_bool[i]);
					break;
				case FT_Integer:
					DBFWriteIntegerAttribute(db, i, j, field->m_int[i]);
					break;
				case FT_Short:
					DBFWriteIntegerAttribute(db, i, j, field->m_short[i]);
					break;
				case FT_Float:
					// SHP does do floats, only doubles
					DBFWriteDoubleAttribute(db, i, j, field->m_float[i]);
					break;
				case FT_Double:
					DBFWriteDoubleAttribute(db, i, j, field->m_double[i]);
					break;
				case FT_String:
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

bool vtFeatureSet::LoadFromSHP(const char *fname)
{
	VTLOG(" LoadFromSHP\n");

	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(fname, "rb");
	if (hSHP == NULL)
		return false;

	LoadGeomFromSHP(hSHP);
	SHPClose(hSHP);

	SetFilename(fname);

	// Read corresponding attributes (DBF fields and records)
	LoadDataFromDBF(fname);

	return true;
}


vtFeatureSet *vtFeatureLoader::LoadFrom(const char *filename)
{
	vtString fname = filename;
	vtString ext = fname.Right(3);
	if (!ext.CompareNoCase("shp"))
		return LoadFromSHP(filename);
	else
		return LoadWithOGR(filename);
}

vtFeatureSet *vtFeatureLoader::LoadFromSHP(const char *filename)
{
	VTLOG(" FeatureLoader LoadFromSHP\n");

	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(filename, "rb");
	if (hSHP == NULL)
		return NULL;

	// Get number of entities (nElem) and type of data (nShapeType)
	int		nElems, nShapeType;
	SHPGetInfo(hSHP, &nElems, &nShapeType, NULL, NULL);

	//  Check shape type, we only support a few types
	vtFeatureSet *pSet = NULL;

	switch (nShapeType)
	{
	case SHPT_POINT:
		pSet = new vtFeatureSetPoint2D();
		break;
	case SHPT_POINTZ:
		pSet = new vtFeatureSetPoint3D();
		break;
	case SHPT_ARC:
		pSet = new vtFeatureSetLineString();
		break;
	case SHPT_ARCZ:
		pSet = new vtFeatureSetLineString3D();
		break;
	case SHPT_POLYGON:
		pSet = new vtFeatureSetPolygon();
		break;
	default:
		SHPClose(hSHP);
		return NULL;
	}
	SHPClose(hSHP);

	// Read SHP header and geometry from SHP into memory
	pSet->LoadFromSHP(filename);

	return pSet;
}


/////////////////////////////////////////////////////////////////////////////

/*
bool vtFeatureSet::LoadFromGML(const char *filename)
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
			case OFT_Integer:
				AddField(fnameref, FT_Integer);
				break;
			case OFTReal:
				AddField(fnameref, FT_Double);
				break;
			case OFT_String:
				AddField(fnameref, FT_String);
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
			case FT_Integer:
				field->m_int.Append(pFeature->GetFieldAsInteger(j));
				break;
			case FT_Double:
				field->m_double.Append(pFeature->GetFieldAsDouble(j));
				break;
			case FT_String:
				field->m_string.push_back(vtString(pFeature->GetFieldAsString(j)));
				break;
			}
		}
		fcount++;
	}

	delete pDatasource;

	// allocate selection array
	m_Flags.resize(fcount, 0);

	return true;
}
*/

vtFeatureSet *vtFeatureLoader::LoadWithOGR(const char *filename,
							 void progress_callback(int))
{
	// try using OGR
	g_GDALWrapper.RequestOGRFormats();

	OGRDataSource *pDatasource = OGRSFDriverRegistrar::Open( filename );
	if (!pDatasource)
		return NULL;

	// Don't iterate through the layers, there should be only one.
	//
	OGRLayer *pLayer = pDatasource->GetLayer(0);
	if (!pLayer)
		return NULL;

	// Get basic information about the layer we're reading
  	pLayer->ResetReading();
	OGRFeatureDefn *defn = pLayer->GetLayerDefn();
	if (!defn)
		return NULL;
	OGRwkbGeometryType geom_type = defn->GetGeomType();

	vtFeatureSet *pSet = NULL;
	if (geom_type == wkbUnknown)
	{
		// This usually indicates that the file contains a mix of different
		// geometry types.  Look at the first geometry.
		OGRFeature *pFeature = pLayer->GetNextFeature();
		OGRGeometry *pGeom = pFeature->GetGeometryRef();
		geom_type = pGeom->getGeometryType();
	}
	switch (geom_type)
	{
	case wkbPoint:
		pSet = new vtFeatureSetPoint2D();
		break;
	case wkbPoint25D:
		pSet = new vtFeatureSetPoint3D();
		break;
	case wkbLineString:
		pSet = new vtFeatureSetLineString();
		break;
	case wkbLineString25D:
		pSet = new vtFeatureSetLineString3D();
		break;
	case wkbPolygon:
		pSet = new vtFeatureSetPolygon();
		break;
	default:
		return NULL;
	}

	// We're going to read the file now, so take it's name
	pSet->SetFilename(filename);

	if (!pSet->LoadFromOGR(pDatasource, progress_callback))
	{
		delete pSet;
		return NULL;
	}
	return pSet;
}

bool vtFeatureSet::LoadFromOGR(OGRDataSource *pDatasource,
							 void progress_callback(int))
{
	VTLOG(" LoadFromOGR\n");

	// get informnation from the datasource
	OGRLayer *pLayer = pDatasource->GetLayer(0);
	OGRFeatureDefn *defn = pLayer->GetLayerDefn();
	int feature_count = pLayer->GetFeatureCount();
	const char *layer_name = defn->GetName();
	int num_fields = defn->GetFieldCount();

	// Get the projection (SpatialReference) from this layer, if we can.
	// Sometimes (e.g. for GML) the layer doesn't have it; may have to
	// use the first Geometry instead.
	bool bGotCS = false;
	OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
	if (pSpatialRef)
	{
		m_proj.SetSpatialReference(pSpatialRef);
		bGotCS = true;
	}

	int i, j, count;

	for (j = 0; j < num_fields; j++)
	{
		OGRFieldDefn *field_def = defn->GetFieldDefn(j);
		const char *field_name = field_def->GetNameRef();
		OGRFieldType field_type = field_def->GetType();
		int width = field_def->GetWidth();

		FieldType ftype;
		switch (field_type)
		{
		case OFTInteger:
			ftype = FT_Integer;
			break;
		case OFTReal:
			ftype = FT_Double;
			break;
		case OFTString:
			ftype = FT_String;
			break;
		default:
			continue;
		}
		AddField(field_name, ftype, width);
	}

	// For efficiency, pre-allocate room for the number of features
	//  we expect, although there may be a few more.
	Reserve(feature_count);

	// Read Data from OGR into memory
	DPoint2 p2, first_p2;
	DPoint3 p3;
	int num_geoms, num_points;
	OGRPoint		*pPoint;
	OGRPolygon		*pPolygon;
	OGRLinearRing	*pRing;
	OGRLineString   *pLineString;
	OGRMultiLineString   *pMulti;

	vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(this);
	vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(this);
	vtFeatureSetLineString *pSetLine = dynamic_cast<vtFeatureSetLineString *>(this);
	vtFeatureSetLineString3D *pSetLine3 = dynamic_cast<vtFeatureSetLineString3D *>(this);
	vtFeatureSetPolygon *pSetPoly = dynamic_cast<vtFeatureSetPolygon *>(this);

	pLayer->ResetReading();
	count = 0;
	OGRFeature *pFeature;
	while( (pFeature = pLayer->GetNextFeature()) != NULL )
	{
		if (progress_callback != NULL)
			progress_callback(count * 100 / feature_count);

		OGRGeometry	*pGeom = pFeature->GetGeometryRef();
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
		OGRwkbGeometryType geom_type = pGeom->getGeometryType();
		num_geoms = 1;

		DLine2 dline2;
		DLine3 dline3;
		DPolygon2 dpoly;

		switch (geom_type)
		{
		case wkbPoint:
			pPoint = (OGRPoint *) pGeom;
			if (pSetP2)
				pSetP2->AddPoint(DPoint2(pPoint->getX(), pPoint->getY()));
			break;

		case wkbPoint25D:
			pPoint = (OGRPoint *) pGeom;
			if (pSetP3)
				pSetP3->AddPoint(DPoint3(pPoint->getX(), pPoint->getY(), pPoint->getZ()));
			break;

		case wkbLineString:
			pLineString = (OGRLineString *) pGeom;
			if (pSetLine)
			{
				num_points = pLineString->getNumPoints();
				dline2.SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p2.Set(pLineString->getX(j), pLineString->getY(j));
					dline2.SetAt(j, p2);
				}
				pSetLine->AddPolyLine(dline2);
			}
			break;

		case wkbLineString25D:
			pLineString = (OGRLineString *) pGeom;
			if (pSetLine3)
			{
				num_points = pLineString->getNumPoints();
				dline3.SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p3.Set(pLineString->getX(j), pLineString->getY(j), pLineString->getZ(j));
					dline3.SetAt(j, p3);
				}
				pSetLine3->AddPolyLine(dline3);
			}
			break;

		case wkbMultiLineString:
			pMulti = (OGRMultiLineString *) pGeom;
			if (pSetLine)
			{
				num_geoms = pMulti->getNumGeometries();
				for (i = 0; i < num_geoms; i++)
				{
					pLineString = (OGRLineString *) pMulti->getGeometryRef(i);
					num_points = pLineString->getNumPoints();
					dline2.SetSize(num_points);
					for (j = 0; j < num_points; j++)
					{
						p2.Set(pLineString->getX(j), pLineString->getY(j));
						dline2.SetAt(j, p2);
					}
					pSetLine->AddPolyLine(dline2);
				}
			}
			break;

		case wkbPolygon:
			pPolygon = (OGRPolygon *) pGeom;
			pRing = pPolygon->getExteriorRing();
			num_points = pRing->getNumPoints();

			dpoly.resize(0);

			// do exterior ring
			dline2.SetSize(0);
			dline2.SetMaxSize(num_points);
			for (j = 0; j < num_points; j++)
			{
				p2.Set(pRing->getX(j), pRing->getY(j));

				// ignore last point if it's the same as the first
				if (j == 0)
					first_p2 = p2;
				if (j == num_points-1 && p2 == first_p2)
					continue;

				dline2.Append(p2);
			}
			dpoly.push_back(dline2);

			// do interior ring(s)
			for (i = 0; i < pPolygon->getNumInteriorRings(); i++)
			{
				pRing = pPolygon->getInteriorRing(i);
				num_points = pRing->getNumPoints();
				dline2.SetSize(0);
				dline2.SetMaxSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p2.Set(pRing->getX(j), pRing->getY(j));

					// ignore last point if it's the same as the first
					if (j == 0)
						first_p2 = p2;
					if (j == num_points-1 && p2 == first_p2)
						continue;

					dline2.Append(p2);
				}
				dpoly.push_back(dline2);
			}
			if (pSetPoly)
				pSetPoly->AddPolygon(dpoly);
			break;

		case wkbMultiPolygon:
			// possible TODO
			break;

		case wkbMultiPoint:
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
				case FT_Boolean:
					SetValue(count, j, pFeature->GetFieldAsInteger(j) != 0);
					break;
				case FT_Integer:
					SetValue(count, j, pFeature->GetFieldAsInteger(j));
					break;
				case FT_Double:
					SetValue(count, j, pFeature->GetFieldAsDouble(j));
					break;
				case FT_String:
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


/*
vtFeatureSet *vtFeatureLoader::CreateFromDLG(class vtDLGFile *pDLG)
{
	// A DLG file can be fairly directly interpreted as features, since
	// it consists of nodes, areas, and lines.  However, topology is lost
	// and we must pick which of the three to display.

	int i;
	int nodes = pDLG->m_iNodes, areas = pDLG->m_iAreas, lines = pDLG->m_iLines;
	if (nodes > lines)
	{
		pSet->SetGeomType(wkbPoint);
		for (i = 0; i < nodes; i++)
			AddPoint(pDLG->m_nodes[i].m_p);
	}
#if 0
	else if (areas >= nodes && areas >= lines)
	{
		// "Areas" in a DLG area actually points which indicate an interior
		//  point in a polygon defined by some of the "lines"
		SetEntityType(SHPT_POLYGON);
		for (i = 0; i < areas; i++)
			AddPolyLine(&pDLG->m_areas[i].m_p);
	}
#endif
	else
	{
		SetGeomType(wkbLineString);
		for (i = 0; i < areas; i++)
			AddPolyLine(pDLG->m_lines[i].m_p);
	}
	m_proj = pDLG->GetProjection();
	return true;
}*/

bool vtFeatureSet::LoadDataFromDBF(const char *filename)
{
	VTLOG(" LoadDataFromDBF\n");
	// Try loading DBF File as well
	vtString dbfname = MakeDBFName(filename);
	DBFHandle db = DBFOpen(dbfname, "rb");
	if (db == NULL)
		return false;

	ParseDBFFields(db);
	ParseDBFRecords(db);
	DBFClose(db);

	return true;
}

/**
 * A lightweight alternative to LoadDataFromDBF, which simply reads the
 * field descriptions from the DBF file.
 */
bool vtFeatureSet::LoadFieldInfoFromDBF(const char *filename)
{
	VTLOG(" LoadFieldInfoFromDBF\n");
	vtString dbfname = MakeDBFName(filename);
	DBFHandle db = DBFOpen(dbfname, "rb");
	if (db == NULL)
		return false;

	ParseDBFFields(db);
	DBFClose(db);
	return true;
}

void vtFeatureSet::ParseDBFFields(DBFHandle db)
{
	// Check for field of poly id, current default field in dbf is Id
	int iSHPFields = DBFGetFieldCount(db);
	int pnWidth, pnDecimals;
	DBFFieldType fieldtype;
	char szFieldName[80];
	int iField;

	for (iField = 0; iField < iSHPFields; iField++)
	{
		fieldtype = DBFGetFieldInfo(db, iField, szFieldName,
			&pnWidth, &pnDecimals);

		FieldType ftype = ConvertFieldType(fieldtype);

		AddField(szFieldName, ftype, pnWidth);
	}
}

void vtFeatureSet::ParseDBFRecords(DBFHandle db)
{
	int iRecords = DBFGetRecordCount(db);
	for (int i = 0; i < iRecords; i++)
	{
		unsigned int iField;
		int rec = AddRecord();
		for (iField = 0; iField < GetNumFields(); iField++)
		{
			Field *field = m_fields[iField];
			switch (field->m_type)
			{
			case FT_String:
				SetValue(rec, iField, DBFReadStringAttribute(db, rec, iField));
				break;
			case FT_Integer:
				SetValue(rec, iField, DBFReadIntegerAttribute(db, rec, iField));
				break;
			case FT_Double:
				SetValue(rec, iField, DBFReadDoubleAttribute(db, rec, iField));
				break;
			case FT_Boolean:
				SetValue(rec, iField, DBFReadLogicalAttribute(db, rec, iField));
				break;
			}
		}
	}
}

void vtFeatureSet::SetNumEntities(int iNum)
{
	// First set the number of geometries
	SetNumGeometries(iNum);

	// Then set the number of records for each field
	for (unsigned int iField = 0; iField < GetNumFields(); iField++)
		m_fields[iField]->SetNumRecords(iNum);

	// Also keep size of flag array in synch
	m_Flags.resize(iNum, 0);
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
OGRwkbGeometryType vtFeatureSet::GetGeomType() const
{
	return m_eGeomType;
}

/**
 * Set the type of geometry that each feature will have.
 *
 * \param eGeomType
 *		- wkbPoint for 2D points
 *		- wkbPoint25D fpr 3D points
 *		- wkbLineString for 2D polylines
 *		- wkbPolygon for 2D polygons
 */
void vtFeatureSet::SetGeomType(OGRwkbGeometryType eGeomType)
{
	m_eGeomType = eGeomType;
}

bool vtFeatureSet::AppendDataFrom(vtFeatureSet *pFromSet)
{
	// Must be the same geometry type
	if (pFromSet->GetGeomType() != GetGeomType())
		return false;

	int first_appended_ent = GetNumEntities();

	// copy geometry
	if (!AppendGeometryFrom(pFromSet))
		return false;

	// copy entities
	vtString str;
	unsigned int i, num = pFromSet->GetNumEntities();
	for (i = 0; i < num; i++)
	{
		// copy record data for all field names which match
		for (unsigned int f = 0; f < pFromSet->GetNumFields(); f++)
		{
			Field *field1 = pFromSet->GetField(f);
			Field *field2 = GetField((const char *) field1->m_name);
			if (!field2)
				continue;
			field1->GetValueAsString(i, str);
			field2->SetValueFromString(first_appended_ent+i, str);
		}
		m_Flags.push_back(pFromSet->m_Flags[i]);
	}

	// empty the source layer
	pFromSet->SetNumEntities(0);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// Selection of Entities

unsigned int vtFeatureSet::NumSelected() const
{
	unsigned int count = 0;
	unsigned int size = m_Flags.size();
	for (unsigned int i = 0; i < size; i++)
		if (m_Flags[i] & FF_SELECTED)
			count++;
	return count;
}

void vtFeatureSet::DeselectAll()
{
	for (unsigned int i = 0; i < m_Flags.size(); i++)
		m_Flags[i] &= ~FF_SELECTED;
}

void vtFeatureSet::InvertSelection()
{
	for (unsigned int i = 0; i < m_Flags.size(); i++)
		m_Flags[i] ^= FF_SELECTED;
}

int vtFeatureSet::DoBoxSelect(const DRECT &rect, SelectionType st)
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

		bIn = IsInsideRect(i, rect);
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

int vtFeatureSet::SelectByCondition(int iField, int iCondition,
								  const char *szValue)
{
	bool bval, btest;
	int i, ival, itest;
	short sval;
	double dval, dtest;
	int entities = GetNumEntities(), selected = 0;
	int con = iCondition;
	bool result;
	DPoint2 p2;
	DPoint3 p3;

	if (iField < 0)
	{
		dval = atof(szValue);
		for (i = 0; i < entities; i++)
		{
			// special field numbers are used to refer to the spatial components
			if (m_eGeomType == wkbPoint)
			{
				vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(this);
				pSetP2->GetPoint(i, p2);
				if (iField == -1) dtest = p2.x;
				if (iField == -2) dtest = p2.y;
				if (iField == -3) return -1;
			}
			else if (m_eGeomType == wkbPoint25D)
			{
				vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(this);
				pSetP3->GetPoint(i, p3);
				if (iField == -1) dtest = p3.x;
				if (iField == -2) dtest = p3.y;
				if (iField == -3) dtest = p3.z;
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
	case FT_String:
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
	case FT_Integer:
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
	case FT_Short:
		sval = (short) atoi(szValue);
		for (i = 0; i < entities; i++)
		{
			itest = field->m_short[i];
			if (con == 0) result = (itest == sval);
			if (con == 1) result = (itest > sval);
			if (con == 2) result = (itest < sval);
			if (con == 3) result = (itest >= sval);
			if (con == 4) result = (itest <= sval);
			if (con == 5) result = (itest != sval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FT_Double:
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
	case FT_Boolean:
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

void vtFeatureSet::DeleteSelected()
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

void vtFeatureSet::SetToDelete(int iFeature)
{
	m_Flags[iFeature] |= FF_DELETE;
}

void vtFeatureSet::ApplyDeletion()
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
	SetNumEntities(newtotal);
}

void vtFeatureSet::CopyEntity(unsigned int from, unsigned int to)
{
	// copy geometry
	CopyGeometry(from, to);

	// copy record fields
	for (unsigned int i = 0; i < m_fields.GetSize(); i++)
		m_fields[i]->CopyValue(from, to);
}

void vtFeatureSet::DePickAll()
{
	int i, entities = GetNumEntities();
	for (i = 0; i < entities; i++)
		m_Flags[i] &= ~FF_PICKED;
}


/////////////////////////////////////////////////////////////////////////////
// Data Fields

Field *vtFeatureSet::GetField(const char *name)
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
int vtFeatureSet::GetFieldIndex(const char *name) const
{
	unsigned int i, num = m_fields.GetSize();
	for (i = 0; i < num; i++)
	{
		if (!m_fields[i]->m_name.CompareNoCase(name))
			return i;
	}
	return -1;
}

int vtFeatureSet::AddField(const char *name, FieldType ftype, int string_length)
{
	Field *f = new Field(name, ftype);
	if (ftype == FT_Integer)
	{
		f->m_width = 11;
		f->m_decimals = 0;
	}
	else if (ftype == FT_Short)
	{
		f->m_width = 6;
		f->m_decimals = 0;
	}
	else if (ftype == FT_Float)
	{
		f->m_width = 8;
		f->m_decimals = 6;
	}
	else if (ftype == FT_Double)
	{
		f->m_width = 12;
		f->m_decimals = 12;
	}
	else if (ftype == FT_Boolean)
	{
		f->m_width = 1;
		f->m_decimals = 0;
	}
	else if (ftype == FT_String)
	{
		f->m_width = string_length;
		f->m_decimals = 0;
	}
	else
	{
		VTLOG("Attempting to add field '%s' of type 'invalid', adding an integer field instead.\n", name, ftype);
		f->m_type = FT_Integer;
		f->m_width = 1;
		f->m_decimals = 0;
	}
	return m_fields.Append(f);
}

int vtFeatureSet::AddRecord()
{
	int recs;
	for (unsigned int i = 0; i < m_fields.GetSize(); i++)
	{
		recs = m_fields[i]->AddRecord();
	}
	m_Flags.push_back(0);
	return recs;
}

void vtFeatureSet::SetValue(unsigned int record, unsigned int field, const char *value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::SetValue(unsigned int record, unsigned int field, int value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::SetValue(unsigned int record, unsigned int field, double value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::SetValue(unsigned int record, unsigned int field, bool value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::GetValueAsString(unsigned int iRecord, unsigned int iField, vtString &str) const
{
	Field *field = m_fields[iField];
	field->GetValueAsString(iRecord, str);
}

void vtFeatureSet::SetValueFromString(unsigned int iRecord, unsigned int iField, const vtString &str)
{
	Field *field = m_fields[iField];
	field->SetValueFromString(iRecord, str);
}

void vtFeatureSet::SetValueFromString(unsigned int iRecord, unsigned int iField, const char *str)
{
	Field *field = m_fields[iField];
	field->SetValueFromString(iRecord, str);
}

int vtFeatureSet::GetIntegerValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	return field->m_int[iRecord];
}

short vtFeatureSet::GetShortValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	short val;
	field->GetValue(iRecord, val);
	return val;
}

float vtFeatureSet::GetFloatValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	float val;
	field->GetValue(iRecord, val);
	return val;
}

double vtFeatureSet::GetDoubleValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	double val;
	field->GetValue(iRecord, val);
	return val;
}

bool vtFeatureSet::GetBoolValue(unsigned int iRecord, unsigned int iField) const
{
	Field *field = m_fields[iField];
	return field->m_bool[iRecord];
}

/////////////////////////////////////////////////

//
// Fields
//
Field::Field(const char *name, FieldType ftype)
{
	m_name = name;
	m_type = ftype;
}

Field::~Field()
{
}

void Field::SetNumRecords(int iNum)
{
	switch (m_type)
	{
	case FT_Boolean: m_bool.SetSize(iNum);	break;
	case FT_Short: m_short.SetSize(iNum);	break;
	case FT_Integer: m_int.SetSize(iNum);	break;
	case FT_Float:	m_float.SetSize(iNum);	break;
	case FT_Double:	m_double.SetSize(iNum);	break;
	case FT_String: m_string.resize(iNum);	break;
	}
}

int Field::AddRecord()
{
	int index = 0;
	switch (m_type)
	{
	case FT_Boolean: return	m_bool.Append(false);	break;
	case FT_Short:	return	m_short.Append(0);		break;
	case FT_Integer: return	m_int.Append(0);		break;
	case FT_Float:	return	m_float.Append(0.0f);	break;
	case FT_Double:	return	m_double.Append(0.0);	break;
	case FT_String:
		index = m_string.size();
		m_string.push_back(vtString(""));
		return index;
	}
	return -1;
}

void Field::SetValue(unsigned int record, const char *value)
{
	if (m_type != FT_String)
		return;
	m_string[record] = value;
}

void Field::SetValue(unsigned int record, int value)
{
	if (m_type == FT_Integer)
		m_int[record] = value;
	else if (m_type == FT_Short)
		m_short[record] = value;
	else if (m_type == FT_Double)
		m_double[record] = value;
	else if (m_type == FT_Float)
		m_float[record] = (float) value;
}

void Field::SetValue(unsigned int record, double value)
{
	if (m_type == FT_Double)
		m_double[record] = value;
	else if (m_type == FT_Float)
		m_float[record] = (float) value;
	else if (m_type == FT_Integer)
		m_int[record] = (int) value;
	else if (m_type == FT_Short)
		m_short[record] = (int) value;
}

void Field::SetValue(unsigned int record, bool value)
{
	if (m_type == FT_Boolean)
		m_bool[record] = value;
	else if (m_type == FT_Integer)
		m_int[record] = (int) value;
	else if (m_type == FT_Short)
		m_short[record] = (int) value;
}

void Field::GetValue(unsigned int record, vtString &string)
{
	if (m_type != FT_String)
		return;
	string = m_string[record];
}

void Field::GetValue(unsigned int record, short &value)
{
	if (m_type == FT_Short)
		value = m_short[record];
	else if (m_type == FT_Integer)
		value = m_int[record];
	else if (m_type == FT_Double)
		value = (int) m_double[record];
	else if (m_type == FT_Boolean)
		value = (int) m_bool[record];
}

void Field::GetValue(unsigned int record, int &value)
{
	if (m_type == FT_Integer)
		value = m_int[record];
	else if (m_type == FT_Short)
		value = m_short[record];
	else if (m_type == FT_Double)
		value = (int) m_double[record];
	else if (m_type == FT_Boolean)
		value = (int) m_bool[record];
}

void Field::GetValue(unsigned int record, float &value)
{
	if (m_type == FT_Float)
		value = m_float[record];
	else if (m_type == FT_Double)
		value = (float) m_double[record];
	else if (m_type == FT_Integer)
		value = (float) m_int[record];
	else if (m_type == FT_Short)
		value = (float) m_short[record];
}

void Field::GetValue(unsigned int record, double &value)
{
	if (m_type == FT_Double)
		value = m_double[record];
	else if (m_type == FT_Float)
		value = m_float[record];
	else if (m_type == FT_Integer)
		value = (double) m_int[record];
	else if (m_type == FT_Short)
		value = (double) m_short[record];
}

void Field::GetValue(unsigned int record, bool &value)
{
	if (m_type == FT_Boolean)
		value = m_bool[record];
	else if (m_type == FT_Integer)
		value = (m_int[record] != 0);
	else if (m_type == FT_Short)
		value = (m_short[record] != 0);
}

void Field::CopyValue(unsigned int FromRecord, int ToRecord)
{
	if (m_type == FT_Integer)
		m_int[ToRecord] = m_int[FromRecord];
	else if (m_type == FT_Short)
		m_short[ToRecord] = m_short[FromRecord];
	else if (m_type == FT_Double)
		m_double[ToRecord] = m_double[FromRecord];

	// when dealing with strings, copy by value not reference, to
	// avoid memory tracking issues
	else if (m_type == FT_String)
		m_string[ToRecord] = m_string[FromRecord];

	else if (m_type == FT_Boolean)
		m_bool[ToRecord] = m_bool[FromRecord];
}

void Field::GetValueAsString(unsigned int iRecord, vtString &str)
{
	switch (m_type)
	{
	case FT_String:
		str = m_string[iRecord];
		break;
	case FT_Integer:
		str.Format("%d", m_int[iRecord]);
		break;
	case FT_Short:
		str.Format("%d", m_short[iRecord]);
		break;
	case FT_Double:
		str.Format("%lf", m_double[iRecord]);
		break;
	case FT_Boolean:
		str = m_bool[iRecord] ? "true" : "false";
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
	case FT_String:
		if (iRecord < (int) m_string.size())
			m_string[iRecord] = str;
		else
			m_string.push_back(vtString(str));
		break;
	case FT_Integer:
		i = atoi(str);
		if (iRecord < m_int.GetSize())
			m_int[iRecord] = i;
		else
			m_int.Append(i);
		break;
	case FT_Short:
		i = atoi(str);
		if (iRecord < m_short.GetSize())
			m_short[iRecord] = (short) i;
		else
			m_short.Append((short) i);
		break;
	case FT_Double:
		d = atof(str);
		if (iRecord < m_double.GetSize())
			m_double[iRecord] = d;
		else
			m_double.Append(d);
		break;
	case FT_Boolean:
		if (!strcmp(str, "true"))
			m_bool[iRecord] = true;
		else
			m_bool[iRecord] = false;
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Helpers

const char *DescribeFieldType(FieldType type)
{
	switch (type)
	{
	case FT_Boolean: return "Boolean";
	case FT_Integer: return "Integer";
	case FT_Short: return "Short";
	case FT_Float: return "Float";
	case FT_Double: return "Double";
	case FT_String: return "String";
	case FT_Unknown:
	default:
		return "Unknown";
	}
}

const char *DescribeFieldType(DBFFieldType type)
{
	switch (type)
	{
	case FTString: return "String";
	case FTInteger: return "Integer";
	case FTDouble: return "Double";
	case FTLogical: return "Logical";
	case FTInvalid:
	default:
		return "Unknown";
	}
}

DBFFieldType ConvertFieldType(FieldType type)
{
	switch (type)
	{
	case FT_Boolean: return FTLogical;
	case FT_Integer: return FTInteger;
	case FT_Short: return FTInteger;
	case FT_Float: return FTDouble;
	case FT_Double: return FTDouble;
	case FT_String: return FTString;
	case FT_Unknown:
	default:
		return FTInvalid;
	}
}

FieldType ConvertFieldType(DBFFieldType type)
{
	switch (type)
	{
	case FTLogical: return FT_Boolean;
	case FTInteger: return FT_Integer;
	case FTDouble:	return FT_Double;
	case FTString:  return FT_String;
	case FTInvalid:
	default:
		return FT_Unknown;
	}
}

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
	case SHPT_MULTIPOINTZ: return wkbMultiPoint25D;

	// the following are unknown
	case SHPT_MULTIPATCH: return wkbUnknown;
	case SHPT_POINTM: return wkbUnknown;
	case SHPT_ARCM: return wkbUnknown;
	case SHPT_POLYGONM: return wkbUnknown;
	case SHPT_MULTIPOINTM: return wkbUnknown;
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

vtString MakeDBFName(const char *filename)
{
	vtString fname = filename;
	fname = fname.Left(fname.GetLength() - 4);
	fname += ".dbf";
	return fname;
}

