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
#include "vtdata/Array.inl"

#include "Layer.h"

enum FieldType {
  FT_String,
  FT_Integer,
  FT_Double
};

#if 0	// not used yet
struct Field {
	FieldType m_type;
	int m_size;
	int m_offset;
	wxString m_name;
};

class Attributes
{
	Array<Field> m_fields;
	int m_record_size;
	Array<unsigned char *> m_records;
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

	int GetEntityType();
	void SetEntityType(int type);
	void AddPoint(DPoint2 p);

protected:
	// supported values: SHPT_NULL, SHPT_POINT, SHPT_ARC, SHPT_POLYGON
	int			m_nSHPType;
	DLine2		m_Point;
	DPolyArray2	m_LinePoly;

//	Attributes	m_attrib;

	vtProjection	m_proj;
};

#endif
