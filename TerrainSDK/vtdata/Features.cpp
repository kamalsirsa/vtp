//
// Features.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Features.h"

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
	// string memory must be freed manually
	if (m_type == FTString)
	{
		int size = m_string.GetSize();
		for (int i = 0; i < size; i++)
		{
			vtString *string = m_string.GetAt(i);
			delete string;
		}
	}
}


//
// Construct / Destruct
//

vtFeatures::vtFeatures()
{
}

vtFeatures::~vtFeatures()
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
bool vtFeatures::SaveToSHP(const char *filename)
{
	SHPHandle hSHP = SHPCreate(filename, m_nSHPType);
	if (!hSHP)
		return false;

	int i, j, size;
	SHPObject *obj;
	if (m_nSHPType == SHPT_POINT)
	{
		size = m_Point2.GetSize();
		for (i = 0; i < size; i++)
		{
			// Save to SHP
			obj = SHPCreateSimpleObject(m_nSHPType, 1,
				&m_Point2[i].x, &m_Point2[i].y, NULL); 
			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);
		}
	}
	if (m_nSHPType == SHPT_POINTZ)
	{
		size = m_Point3.GetSize();
		for (i = 0; i < size; i++)
		{
			// Save to SHP
			obj = SHPCreateSimpleObject(m_nSHPType, 1,
				&m_Point3[i].x, &m_Point3[i].y, &m_Point3[i].z); 
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
			obj = SHPCreateSimpleObject(m_nSHPType, dl->GetSize(),
				&m_Point2[i].x, &m_Point2[i].y, NULL);
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
		int entities = NumEntities();
		for (i = 0; i < entities; i++)
		{
			for (j = 0; j < m_fields.GetSize(); j++)
			{
				field = m_fields[j];
				switch (field->m_type)
				{
				case FTInteger:
					DBFWriteIntegerAttribute(db, i, j, field->m_int[i]);
					break;
				case FTDouble:
					DBFWriteDoubleAttribute(db, i, j, field->m_double[i]);
					break;
				case FTString:
					DBFWriteStringAttribute(db, i, j, (const char *) *(field->m_string[i]));
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

bool vtFeatures::LoadFromSHP(const char *filename)
{
	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(filename, "rb");
	if (hSHP == NULL)
		return false;

	// Get number of entities (nElem) and type of data (nShapeType)
	int		nElem;
	int		nShapeType;
    double	adfMinBound[4], adfMaxBound[4];
	FPoint2 point;
	SHPGetInfo(hSHP, &nElem, &nShapeType, adfMinBound, adfMaxBound);

	//  Check shape type, we only support a few types
	switch (nShapeType)
	{
	case SHPT_POINT:
	case SHPT_POINTZ:
	case SHPT_ARC:
	case SHPT_POLYGON:
		m_nSHPType = nShapeType;
		break;
	default:
		SHPClose(hSHP);
		return false;
	}

	// Try loading DBF File as well
	vtString dbfname = filename;
	dbfname = dbfname.Left(dbfname.GetLength() - 4);
	dbfname += ".dbf";
	DBFFieldType fieldtype;
	DBFHandle db = DBFOpen(dbfname, "rb");
	int iField, iFields;
	if (db != NULL)
	{
		// Check for field of poly id, current default field in dbf is Id
		iFields = DBFGetFieldCount(db);
		int pnWidth, pnDecimals;
		char szFieldName[80];

		for (iField = 0; iField < iFields; iField++)
		{
			fieldtype = DBFGetFieldInfo(db, iField, szFieldName,
				&pnWidth, &pnDecimals);
			AddField(szFieldName, fieldtype, pnWidth);
		}
	}

	// Initialize arrays
	switch (m_nSHPType)
	{
	case SHPT_POINT:
		m_Point2.SetSize(nElem);
		break;
	case SHPT_POINTZ:
		m_Point3.SetSize(nElem);
		break;
	case SHPT_ARC:
	case SHPT_POLYGON:
		m_LinePoly.SetSize(nElem);
		break;
	}

	// Read Data from SHP into memory
	for (int i = 0; i < nElem; i++)
	{
		// Get the i-th Shape in the SHP file
		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		DPoint2 p2;
		DPoint3 p3;
		DLine2 *new_poly;
		switch (m_nSHPType)
		{
		case SHPT_POINT:
			p2.x = *psShape->padfX;
			p2.y = *psShape->padfY;
			m_Point2.SetAt(i, p2);
			break;
		case SHPT_POINTZ:
			p3.x = *psShape->padfX;
			p3.y = *psShape->padfY;
			p3.z = *psShape->padfZ;
			m_Point3.SetAt(i, p3);
			break;
		case SHPT_ARC:
		case SHPT_POLYGON:
			new_poly = new DLine2();
			new_poly->SetSize(psShape->nVertices);
			m_LinePoly.SetAt(i, new_poly);

			// Store each coordinate
			for (int j = 0; j < psShape->nVertices; j++)
			{
				p2.x = psShape->padfX[j];
				p2.y = psShape->padfY[j];
				new_poly->SetAt(j, p2);
			}
			break;
		}
		SHPDestroyObject(psShape);

		// Read corresponding attributes (DBF record fields)
		if (db != NULL)
		{
			int rec = AddRecord();
			for (iField = 0; iField < iFields; iField++)
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
	m_Selected.SetSize(nElem);
	return true;
}


//
// feature (entity) operations
//

int vtFeatures::NumEntities()
{
	if (m_nSHPType == SHPT_POINT)
		return m_Point2.GetSize();
	else if (m_nSHPType == SHPT_POINTZ)
		return m_Point3.GetSize();
	else if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
		return m_LinePoly.GetSize();
	else
		return -1;
}

int vtFeatures::GetEntityType()
{
	return m_nSHPType;
}

void vtFeatures::SetEntityType(int type)
{
	m_nSHPType = type;
}

int vtFeatures::AddPoint(const DPoint2 &p)
{
	int rec = -1;
	if (m_nSHPType == SHPT_POINT)
	{
		m_Point2.Append(p);
		AddRecord();
	}
	return rec;
}

int vtFeatures::AddPoint(const DPoint3 &p)
{
	int rec = -1;
	if (m_nSHPType == SHPT_POINTZ)
	{
		rec = m_Point3.Append(p);
		AddRecord();
	}
	return rec;
}

void vtFeatures::GetPoint(int num, DPoint3 &p)
{
	if (m_nSHPType == SHPT_POINTZ)
	{
		p = m_Point3.GetAt(num);
	}
}

void vtFeatures::GetPoint(int num, DPoint2 &p)
{
	if (m_nSHPType == SHPT_POINTZ)
	{
		DPoint3 p3 = m_Point3.GetAt(num);
		p.x = p3.x;
		p.y = p3.y;
	}
	if (m_nSHPType == SHPT_POINT)
	{
		p = m_Point2.GetAt(num);
	}
}

int vtFeatures::FindClosestPoint(const DPoint2 &p, double epsilon)
{
	int entities = NumEntities();
	double dist, closest = 1E9;
	int found = -1;
	DPoint2 diff;

	int i;
	for (i = 0; i < entities; i++)
	{
		if (m_nSHPType == SHPT_POINT)
			diff = p - m_Point2.GetAt(i);
		if (m_nSHPType == SHPT_POINTZ)
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
	int entities = NumEntities();

	int i;
	for (i = 0; i < entities; i++)
	{
		if (m_nSHPType == SHPT_POINT)
		{
			if (loc == m_Point2.GetAt(i))
				found.Append(i);
		}
		if (m_nSHPType == SHPT_POINTZ)
		{
			DPoint3 p3 = m_Point3.GetAt(i);
			if (loc.x == p3.x && loc.y == p3.y)
				found.Append(i);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Selection of Entities

void vtFeatures::Select(int iEnt, bool set)
{
	m_Selected[iEnt] = set;
}

bool vtFeatures::IsSelected(int iEnt)
{
	return m_Selected[iEnt];
}

int vtFeatures::NumSelected()
{
	int count = 0;
	int size = m_Selected.GetSize();
	for (int i = 0; i < size; i++)
		if (m_Selected[i])
			count++;
	return count;
}

void vtFeatures::DeselectAll()
{
	for (int i = 0; i < m_Selected.GetSize(); i++)
		m_Selected[i] = false;
}

void vtFeatures::InvertSelection()
{
	for (int i = 0; i < m_Selected.GetSize(); i++)
		m_Selected[i] = !m_Selected[i];
}

int vtFeatures::DoBoxSelect(const DRECT &rect, SelectionType st)
{
	int affected = 0;
	int entities = NumEntities();

	bool bIn;
	bool bWas;
	for (int i = 0; i < entities; i++)
	{
		bWas = m_Selected[i];
		if (st == ST_NORMAL)
			Select(i, false);

		if (m_nSHPType == SHPT_POINT)
			bIn = rect.ContainsPoint(m_Point2[i]);

		if (m_nSHPType == SHPT_POINTZ)
			bIn = rect.ContainsPoint(DPoint2(m_Point3[i].x, m_Point3[i].y));

		if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
			bIn = rect.ContainsLine(*m_LinePoly[i]);

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
	int i, ival, itest;
	double dval, dtest;
	int entities = NumEntities(), selected = 0;
	int con = iCondition;
	vtString *sp;
	bool result;

	if (iField < 0)
	{
		dval = atof(szValue);
		for (i = 0; i < entities; i++)
		{
			// special field numbers are used to refer to the spatial components
			if (m_nSHPType == SHPT_POINT)
			{
				if (iField == -1) dtest = m_Point2[i].x;
				if (iField == -2) dtest = m_Point2[i].y;
				if (iField == -3) return -1;
			}
			else if (m_nSHPType == SHPT_POINTZ)
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
			sp = field->m_string[i];
			if (con == 0) result = (sp->Compare(szValue) == 0);
			if (con == 1) result = (sp->Compare(szValue) > 0);
			if (con == 2) result = (sp->Compare(szValue) < 0);
			if (con == 3) result = (sp->Compare(szValue) >= 0);
			if (con == 4) result = (sp->Compare(szValue) <= 0);
			if (con == 5) result = (sp->Compare(szValue) != 0);
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
	}
	return selected;
}

void vtFeatures::DeleteSelected()
{
	int i, entities = NumEntities();

	int target = 0;
	int newtotal = entities;
	for (i = 0; i < entities; i++)
	{
		if (!m_Selected[i])
		{
			if (target != i)
			{
				CopyEntity(i, target);
				m_Selected[target] = false;
			}
			target++;
		}
		else
			newtotal--;
	}
	if (m_nSHPType == SHPT_POINT)
		m_Point2.SetSize(newtotal);
	if (m_nSHPType == SHPT_POINTZ)
		m_Point3.SetSize(newtotal);
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
		m_LinePoly.SetSize(newtotal);
}

void vtFeatures::CopyEntity(int from, int to)
{
	// copy geometry
	if (m_nSHPType == SHPT_POINT)
	{
		m_Point2[to] = m_Point2[from];
	}
	if (m_nSHPType == SHPT_POINTZ)
	{
		m_Point3[to] = m_Point3[from];
	}
	if (m_nSHPType == SHPT_ARC || m_nSHPType == SHPT_POLYGON)
	{
		m_LinePoly[to] = m_LinePoly[from];
	}
	// copy record
	for (int i = 0; i < m_fields.GetSize(); i++)
	{
		m_fields[i]->CopyValue(from, to);
	}
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

int vtFeatures::AddField(const char *name, DBFFieldType ftype, int string_length)
{
	Field *f = new Field(name, ftype);
	if (ftype == FTInteger)
	{
		f->m_width = 11;
		f->m_decimals = 0;
	}
	if (ftype == FTDouble)
	{
		f->m_width = 12;
		f->m_decimals = 12;
	}
	if (ftype == FTString)
	{
		f->m_width = string_length;
		f->m_decimals = 0;
	}
	return m_fields.Append(f);
}

int vtFeatures::AddRecord()
{
	int recs;
	for (int i = 0; i < m_fields.GetSize(); i++)
	{
		recs = m_fields[i]->AddRecord();
	}
	m_Selected.Append(false);
	return recs;
}

void vtFeatures::SetValue(int record, int field, const char *value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatures::SetValue(int record, int field, int value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatures::SetValue(int record, int field, double value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatures::GetValueAsString(int iRecord, int iField, vtString &str)
{
	Field *field = m_fields[iField];
	field->GetValueAsString(iRecord, str);
}

void vtFeatures::SetValueFromString(int iRecord, int iField, vtString &str)
{
	Field *field = m_fields[iField];
	field->SetValueFromString(iRecord, str);
}

/////////////////////////////////////////////////

int Field::AddRecord()
{
	switch (m_type)
	{
	case FTInteger: return	m_int.Append(0); break;
	case FTDouble: return m_double.Append(0.0); break;
	case FTString: return m_string.Append(new vtString); break;
	}
	return -1;
}

void Field::SetValue(int record, const char *value)
{
	if (m_type != FTString)
		return;
	*(m_string[record]) = value;
}

void Field::SetValue(int record, int value)
{
	if (m_type == FTInteger)
		m_int[record] = value;
	else if (m_type == FTDouble)
		m_double[record] = value;
}

void Field::SetValue(int record, double value)
{
	if (m_type == FTInteger)
		m_int[record] = (int) value;
	else if (m_type == FTDouble)
		m_double[record] = value;
}

void Field::GetValue(int record, vtString &string)
{
	if (m_type != FTString)
		return;
	string = *(m_string[record]);
}

void Field::GetValue(int record, int &value)
{
	if (m_type == FTInteger)
		value = m_int[record];
	else if (m_type == FTDouble)
		value = (int) m_double[record];
}

void Field::GetValue(int record, double &value)
{
	if (m_type == FTInteger)
		value = (double) m_int[record];
	else if (m_type == FTDouble)
		value = m_double[record];
}

void Field::CopyValue(int FromRecord, int ToRecord)
{
	if (m_type == FTInteger)
		m_int[ToRecord] = m_int[FromRecord];
	if (m_type == FTDouble)
		m_double[ToRecord] = m_double[FromRecord];

	// when dealing with strings, copy by value not reference, to
	// avoid memory tracking issues
	if (m_type == FTString)
		*m_string[ToRecord] = *m_string[FromRecord];
}

void Field::GetValueAsString(int iRecord, vtString &str)
{
	switch (m_type)
	{
	case FTString:
		str = *(m_string[iRecord]);
		break;
	case FTInteger:
		str.Format("%d", m_int[iRecord]);
		break;
	case FTDouble:
		str.Format("%lf", m_double[iRecord]);
		break;
	}
}

void Field::SetValueFromString(int iRecord, vtString &str)
{
	int i;
	double d;

	switch (m_type)
	{
	case FTString:
		if (iRecord < m_string.GetSize())
			*(m_string[iRecord]) = str;
		else
			m_string.Append(new vtString(str));
		break;
	case FTInteger:
		i = atoi((const char *) str);
		if (iRecord < m_int.GetSize())
			m_int[iRecord] = i;
		else
			m_int.Append(i);
		break;
	case FTDouble:
		d = atof((const char *) str);
		if (iRecord < m_double.GetSize())
			m_double[iRecord] = d;
		else
			m_double.Append(d);
		break;
	}
}

