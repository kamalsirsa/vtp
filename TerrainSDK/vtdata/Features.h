//
// Features.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_FEATURES
#define VTDATA_FEATURES

#include "shapelib/shapefil.h"
#include "ogrsf_frmts.h"

#include "MathTypes.h"
#include "vtString.h"
#include "Projections.h"

/**
 * This class is used to store values in memory loaded from DBF files.
 * Alternately, we could use values directly from the DBF file instead,
 * or eventually some combination where a window into the DBF is
 * cached in memory to support very large files.
 */
class Field
{
public:
	Field(const char *name, DBFFieldType ftype);
	~Field();

	int AddRecord();
	void SetValue(int record, const char *string);
	void SetValue(int record, int value);
	void SetValue(int record, double value);
	void GetValue(int record, vtString &string);
	void GetValue(int record, int &value);
	void GetValue(int record, double &value);
	void CopyValue(int FromRecord, int ToRecord);
	void GetValueAsString(int iRecord, vtString &str);
	void SetValueFromString(int iRecord, vtString &str);

	DBFFieldType m_type;
	int m_width, m_decimals;	// these are for remembering SHP limitations
	vtString m_name;

	Array<int> m_int;
	Array<double> m_double;
	Array<vtString*> m_string;
};

// feature flags (bit flags)
#define FF_SELECTED		1
#define FF_PICKED		2
#define FF_DELETE		4

/**
 * vtFeatures contains a collection of features which are just abstract data,
 * without any specific correspondence to any aspect of the physical world.
 * This is the same as a traditional GIS file (e.g. ESRI Shapefile).
 * In fact, SHP/DBF and 'shapelib' are used as the format and basic
 * functionality for this class.
 *
 * Examples: political and property boundaries, geocoded addresses,
 * flight paths, place names.
 */
class vtFeatures
{
public:
	vtFeatures();
	~vtFeatures();

	// File IO
	bool SaveToSHP(const char *filename);
	bool LoadFromSHP(const char *filename);
	bool LoadFromGML(const char *filename);

	bool ReadFeaturesFromWFS(const char *url);

	// feature (entity) operations
	int NumEntities();
	int GetEntityType();
	void SetEntityType(int type);
	int AddPoint(const DPoint2 &p);
	int AddPoint(const DPoint3 &p);
	void GetPoint(int num, DPoint3 &p);
	void GetPoint(int num, DPoint2 &p);
	void CopyEntity(int from, int to);
	int FindClosestPoint(const DPoint2 &p, double epsilon);
	void FindAllPointsAtLocation(const DPoint2 &p, Array<int> &found);
	void SetToDelete(int iFeature);
	void ApplyDeletion();

	// selection
	void Select(int iEnt, bool set = true)
	{
		if (set)
			m_Flags[iEnt] |= FF_SELECTED;
		else
			m_Flags[iEnt] &= ~FF_SELECTED;
	}
	bool IsSelected(int iEnt)
	{
		return ((m_Flags[iEnt] & FF_SELECTED) != 0);
	}
	int NumSelected();
	void DeselectAll();
	void InvertSelection();
	int DoBoxSelect(const DRECT &rect, SelectionType st);
	int SelectByCondition(int iField, int iCondition, const char *szValue);
	void DeleteSelected();

	// picking (alternate form of selection)
	void Pick(int iEnt, bool set = true)
	{
		if (set)
			m_Flags[iEnt] |= FF_PICKED;
		else
			m_Flags[iEnt] &= ~FF_PICKED;
	}
	bool IsPicked(int iEnt)
	{
		return ((m_Flags[iEnt] & FF_PICKED) != 0);
	}
	void DePickAll();

	// attribute (field) operations
	int GetNumFields() { return m_fields.GetSize(); }
	Field *GetField(int i) { return m_fields.GetAt(i); }
	Field *GetField(const char *name);
	int AddField(const char *name, DBFFieldType ftype, int string_length = 40);
	int AddRecord();
	void SetValue(int record, int field, const char *string);
	void SetValue(int record, int field, int value);
	void SetValue(int record, int field, double value);
	void GetValueAsString(int record, int field, vtString &str);
	void SetValueFromString(int iRecord, int iField, vtString &str);

	vtProjection &GetAtProjection() { return m_proj; }

protected:
	void _ShrinkGeomArraySize(int size);
	void _SetupFromOGCType(OGRwkbGeometryType type);

	// supported values for shape type are: SHPT_NULL, SHPT_POINT,
	//	SHPT_POINTZ, SHPT_ARC, SHPT_POLYGON
	int			m_nSHPType;
	DLine2		m_Point2;		// SHPT_POINT
	DLine3		m_Point3;		// SHPT_POINTZ
	DPolyArray2	m_LinePoly;		// SHPT_ARC, SHPT_POLYGON

	Array<unsigned char> m_Flags;

	Array<Field*> m_fields;

	vtProjection	m_proj;
};

#endif // VTDATA_FEATURES

