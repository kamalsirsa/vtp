//
// A raw data layer, suitable for storing and displaying the type of
// generic spatial data contained in a Shapefile.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef RAWLAYERH
#define RAWLAYERH

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/Array.h"
#include "vtdata/vtString.h"

#include "Layer.h"

//
// This class is used to store values in memory loaded from DBF files.
// Alternately, we could use values directly from DBF file instead,
//  or eventually some combination where a window into the DBF is
//  cached in memory to support very large files.
//
class Field
{
public:
	Field(const char *name, DBFFieldType ftype) { m_name = name; m_type = ftype; }

	DBFFieldType m_type;
	int m_width, m_decimals;	// these are for remembering SHP limitations
	vtString m_name;

	Array<int> m_int;
	Array<double> m_double;
	Array<vtString*> m_string;

	int AddRecord();
	void SetValue(int record, const char *string);
	void SetValue(int record, int value);
	void SetValue(int record, double value);
};


class vtRawLayer : public vtLayer
{
public:
	vtRawLayer();
	virtual ~vtRawLayer();

	// overrides for vtLayer methods
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, class vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(vtProjection &proj);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &strIn);

	int NumEntities();
	int GetEntityType();
	void SetEntityType(int type);
	int AddPoint(const DPoint2 &p);
	int AddPoint(const DPoint3 &p);
	void GetPoint(int num, DPoint3 &p);

	// selection
	void Select(int iEnt, bool set = true);
	bool IsSelected(int iEnt);
	void DeselectAll();
	int DoBoxSelect(const DRECT &rect);

	int GetNumFields() { return m_fields.GetSize(); }
	Field *GetField(int i) { return m_fields.GetAt(i); }
	int AddField(const char *name, DBFFieldType ftype, int string_length = 40);
	int AddRecord();
	void SetValue(int record, int field, const char *string);
	void SetValue(int record, int field, int value);
	void SetValue(int record, int field, double value);
	void GetValueAsString(int record, int field, vtString &str);
	int SelectByCondition(int iField, int iCondition, const char *szValue);

protected:
	// supported values for shape type are: SHPT_NULL, SHPT_POINT,
	//	SHPT_POINTZ, SHPT_ARC, SHPT_POLYGON
	int			m_nSHPType;
	DLine2		m_Point2;		// SHPT_POINT
	DLine3		m_Point3;		// SHPT_POINTZ
	DPolyArray2	m_LinePoly;		// SHPT_ARC, SHPT_POLYGON

	Array<bool>	m_Selected;

	Array<Field*> m_fields;

	vtProjection	m_proj;
};

#endif
