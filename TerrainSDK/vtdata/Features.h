//
// Features.h
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_FEATURES
#define VTDATA_FEATURES

#include "shapelib/shapefil.h"
#include "ogrsf_frmts.h"

#include "MathTypes.h"
#include "vtString.h"
#include "Projections.h"
#include "Content.h"

enum SelectionType
{
	ST_NORMAL,
	ST_ADD,
	ST_SUBTRACT,
	ST_TOGGLE
};

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

	void SetValue(unsigned int iRecord, const char *string);
	void SetValue(unsigned int iRecord, int value);
	void SetValue(unsigned int iRecord, double value);
	void SetValue(unsigned int iRecord, bool value);

	void GetValue(unsigned int iRecord, vtString &string);
	void GetValue(unsigned int iRecord, int &value);
	void GetValue(unsigned int iRecord, double &value);
	void GetValue(unsigned int iRecord, bool &value);

	void CopyValue(unsigned int FromRecord, int ToRecord);
	void GetValueAsString(unsigned int iRecord, vtString &str);
	void SetValueFromString(unsigned int iRecord, const vtString &str);
	void SetValueFromString(unsigned int iRecord, const char *str);

	DBFFieldType m_type;
	int m_width, m_decimals;	// these are for remembering SHP limitations
	vtString m_name;

	Array<int> m_int;
	Array<double> m_double;
	vtStringArray m_string;
	Array<bool> m_bool;
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
	bool LoadFrom(const char *filename);
	bool SaveToSHP(const char *filename) const;
	bool LoadFromSHP(const char *filename);
	bool LoadHeaderFromSHP(const char *filename);
	bool LoadWithOGR(const char *filename, void progress_callback(int) = NULL);

	bool ReadFeaturesFromWFS(const char *szServerURL, const char *layername);
	bool AddElementsFromDLG(class vtDLGFile *pDLG);

	void SetFilename(const vtString &str) { m_strFilename = str; }
	vtString GetFilename() { return m_strFilename; }

	// feature (entity) operations
	int NumEntities() const;
	int GetEntityType() const;
	void SetEntityType(int type);

	// geometric primitives
	int AddPoint(const DPoint2 &p);
	int AddPoint(const DPoint3 &p);
	int AddPolyLine(const DLine2 &pl);
	void GetPoint(unsigned int num, DPoint3 &p) const;
	void GetPoint(unsigned int num, DPoint2 &p) const;
	const DLine2 &GetLine(unsigned int num) { return m_LinePoly[num]; }

	void CopyEntity(unsigned int from, unsigned int to);
	int FindClosestPoint(const DPoint2 &p, double epsilon);
	void FindAllPointsAtLocation(const DPoint2 &p, Array<int> &found);

	void SetToDelete(int iFeature);
	void ApplyDeletion();

	// selection
	void Select(unsigned int iEnt, bool set = true)
	{
		if (set)
			m_Flags[iEnt] |= FF_SELECTED;
		else
			m_Flags[iEnt] &= ~FF_SELECTED;
	}
	bool IsSelected(unsigned int iEnt)
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
	void Pick(unsigned int iEnt, bool set = true)
	{
		if (set)
			m_Flags[iEnt] |= FF_PICKED;
		else
			m_Flags[iEnt] &= ~FF_PICKED;
	}
	bool IsPicked(unsigned int iEnt)
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

	void SetValue(unsigned int record, unsigned int field, const char *string);
	void SetValue(unsigned int record, unsigned int field, int value);
	void SetValue(unsigned int record, unsigned int field, double value);
	void SetValue(unsigned int record, unsigned int field, bool value);

	void GetValueAsString(unsigned int record, unsigned int field, vtString &str) const;
	void SetValueFromString(unsigned int iRecord, unsigned int iField, const vtString &str);
	void SetValueFromString(unsigned int iRecord, unsigned int iField, const char *str);

	int GetIntegerValue(unsigned int iRecord, unsigned int iField) const;
	double GetDoubleValue(unsigned int iRecord, unsigned int iField) const;
	bool GetBoolValue(unsigned int iRecord, unsigned int iField) const;

	vtProjection &GetAtProjection() { return m_proj; }

protected:
	void _ShrinkGeomArraySize(int size);
	vtString	m_dbfname;
	int			m_iSHPElems, m_iSHPFields;

	// supported values for shape type are: SHPT_NULL, SHPT_POINT,
	//	SHPT_POINTZ, SHPT_ARC, SHPT_POLYGON
	int			m_nSHPType;
	DLine2		m_Point2;		// SHPT_POINT
	DLine3		m_Point3;		// SHPT_POINTZ
	DPolyArray2	m_LinePoly;		// SHPT_ARC, SHPT_POLYGON

	Array<unsigned char> m_Flags;

	Array<Field*> m_fields;

	vtProjection	m_proj;

	// remember the filename these feature were loaded from or saved to
	vtString	m_strFilename;
};

#endif // VTDATA_FEATURES

