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

#if 1	// use this, or use values directly from DBF file instead?
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
#endif

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
	void AddPoint(const DPoint2 &p);
	void AddPoint(const DPoint3 &p);

	int AddField(const char *name, DBFFieldType ftype, int string_length = 40);
	int AddRecord();
	void SetValue(int record, int field, const char *string);
	void SetValue(int record, int field, int value);
	void SetValue(int record, int field, double value);

protected:
	// supported values for shape type are: SHPT_NULL, SHPT_POINT,
	//	SHPT_POINTZ, SHPT_ARC, SHPT_POLYGON
	int			m_nSHPType;
	DLine2		m_Point2;		// SHPT_POINT
	DLine3		m_Point3;		// SHPT_POINTZ
	DPolyArray2	m_LinePoly;		// SHPT_ARC, SHPT_POLYGON

	Array<Field*> m_fields;

	vtProjection	m_proj;
};

#endif
