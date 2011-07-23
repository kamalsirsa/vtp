//
// Features.h
//
// Copyright (c) 2002-2009 Virtual Terrain Project
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

enum FieldType
{
	FT_Boolean,
	FT_Short,
	FT_Integer,
	FT_Float,
	FT_Double,
	FT_String,
	FT_Unknown
};

/**
 * This class is used to store values in memory for each record.
 *
 * Someday, we could use values directly from a database files instead,
 * or even some interface for accessing very large or remote databases.
 */
class Field
{
public:
	Field(const char *name, FieldType ftype);
	~Field();

	int AddRecord();
	void SetNumRecords(int iNum);

	void SetValue(unsigned int iRecord, const char *string);
	void SetValue(unsigned int iRecord, int value);
	void SetValue(unsigned int iRecord, double value);
	void SetValue(unsigned int iRecord, bool value);

	void GetValue(unsigned int iRecord, vtString &string);
	void GetValue(unsigned int iRecord, short &value);
	void GetValue(unsigned int iRecord, int &value);
	void GetValue(unsigned int iRecord, float &value);
	void GetValue(unsigned int iRecord, double &value);
	void GetValue(unsigned int iRecord, bool &value);

	void CopyValue(unsigned int FromRecord, int ToRecord);
	void GetValueAsString(unsigned int iRecord, vtString &str);
	void SetValueFromString(unsigned int iRecord, const vtString &str);
	void SetValueFromString(unsigned int iRecord, const char *str);

	FieldType m_type;
	int m_width, m_decimals;	// these are for remembering SHP limitations
	vtString m_name;

	vtArray<bool> m_bool;
	vtArray<short> m_short;
	vtArray<int> m_int;
	vtArray<float> m_float;
	vtArray<double> m_double;
	vtStringArray m_string;
};

// Helpers
const char *DescribeFieldType(FieldType type);
const char *DescribeFieldType(DBFFieldType type);
DBFFieldType ConvertFieldType(FieldType type);
FieldType ConvertFieldType(DBFFieldType type);

// feature flags (bit flags)
#define FF_SELECTED		1
#define FF_PICKED		2
#define FF_DELETE		4

struct vtFeature {
	unsigned char flags;
};

/**
 * vtFeatureSet contains a collection of features which are just abstract data,
 * without any specific correspondence to any aspect of the physical world.
 * This is the same as a traditional GIS file (e.g. ESRI Shapefile).
 *
 * Examples: political and property boundaries, geocoded addresses,
 * flight paths, place names.
 */
class vtFeatureSet
{
public:
	vtFeatureSet();
	virtual ~vtFeatureSet();

	// File IO
	bool SaveToSHP(const char *filename, bool progress_callback(int)=0) const;
	bool LoadFromOGR(OGRLayer *pLayer, bool progress_callback(int)=0);
	virtual void LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int)=0) = 0;
	bool LoadFromSHP(const char *fname, bool progress_callback(int)=0);
	bool LoadDataFromDBF(const char *filename, bool progress_callback(int)=0);
	bool LoadFieldInfoFromDBF(const char *filename);
	bool LoadDataFromCSV(const char *filename, bool progress_callback(int)=0);
	bool SaveToKML(const char *filename, bool progress_callback(int)=0) const;

	void SetFilename(const vtString &str) { m_strFilename = str; }
	vtString GetFilename() const { return m_strFilename; }

	// feature (entity) operations
	virtual unsigned int GetNumEntities() const = 0;
	void SetNumEntities(int iNum);
	void AllocateFeatures();
	OGRwkbGeometryType GetGeomType() const;
	void SetGeomType(OGRwkbGeometryType eGeomType);
	bool AppendDataFrom(vtFeatureSet *pFromSet);
	/**
	 * If you know how many entities you will be adding to this FeatureSet,
	 * is it more efficient to reserve space for that many.
	 */
	virtual void Reserve(int iNum) = 0;
	virtual bool ComputeExtent(DRECT &rect) const = 0;
	virtual void Offset(const DPoint2 &p, bool bSelectedOnly = false) = 0;
	virtual bool TransformCoords(OCT *pTransform, bool progress_callback(int)=0) = 0;
	virtual bool AppendGeometryFrom(vtFeatureSet *pFromSet) = 0;
	virtual int NumTotalVertices() const { return 0; }

	// deletion
	void SetToDelete(int iFeature);
	void ApplyDeletion();

	// selection
	void Select(unsigned int iEnt, bool set = true)
	{
		if (set)
			m_Features[iEnt]->flags |= FF_SELECTED;
		else
			m_Features[iEnt]->flags &= ~FF_SELECTED;
	}
	bool IsSelected(unsigned int iEnt)
	{
		return ((m_Features[iEnt]->flags & FF_SELECTED) != 0);
	}
	unsigned int NumSelected() const;
	void DeselectAll();
	void InvertSelection();
	int SelectByCondition(int iField, int iCondition, const char *szValue);
	void DeleteSelected();
	bool IsDeleted(unsigned int iEnt)
	{
		return ((m_Features[iEnt]->flags & FF_DELETE) != 0);
	}
	int DoBoxSelect(const DRECT &rect, SelectionType st);

	// picking (alternate form of selection)
	void Pick(unsigned int iEnt, bool set = true)
	{
		if (set)
			m_Features[iEnt]->flags |= FF_PICKED;
		else
			m_Features[iEnt]->flags &= ~FF_PICKED;
	}
	bool IsPicked(unsigned int iEnt)
	{
		return ((m_Features[iEnt]->flags & FF_PICKED) != 0);
	}
	void DePickAll();

	// attribute (field) operations
	unsigned int GetNumFields() const { return m_fields.GetSize(); }
	Field *GetField(int i) { return m_fields.GetAt(i); }
	const Field *GetField(int i) const { return m_fields.GetAt(i); }
	Field *GetField(const char *name);
	int GetFieldIndex(const char *name) const;
	int AddField(const char *name, FieldType ftype, int string_length = 40);
	int AddRecord();
	void DeleteFields();

	void SetValue(unsigned int record, unsigned int field, const char *string);
	void SetValue(unsigned int record, unsigned int field, int value);
	void SetValue(unsigned int record, unsigned int field, double value);
	void SetValue(unsigned int record, unsigned int field, bool value);

	void GetValueAsString(unsigned int record, unsigned int field, vtString &str) const;
	void SetValueFromString(unsigned int iRecord, unsigned int iField, const vtString &str);
	void SetValueFromString(unsigned int iRecord, unsigned int iField, const char *str);

	int GetIntegerValue(unsigned int iRecord, unsigned int iField) const;
	short GetShortValue(unsigned int iRecord, unsigned int iField) const;
	float GetFloatValue(unsigned int iRecord, unsigned int iField) const;
	double GetDoubleValue(unsigned int iRecord, unsigned int iField) const;
	bool GetBoolValue(unsigned int iRecord, unsigned int iField) const;

	void SetProjection(const vtProjection &proj) { m_proj = proj; }
	vtProjection &GetAtProjection() { return m_proj; }

	vtFeature *GetFeature(unsigned int iIndex) const { return m_Features[iIndex]; }

protected:
	// these must be implemented for each type of geometry
	virtual bool IsInsideRect(int iElem, const DRECT &rect) = 0;
	virtual void CopyGeometry(unsigned int from, unsigned int to) = 0;
	virtual void SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)=0) const = 0;
	virtual void SetNumGeometries(int iNum) = 0;

	void CopyEntity(unsigned int from, unsigned int to);
	void ParseDBFFields(DBFHandle db);
	void ParseDBFRecords(DBFHandle db, bool progress_callback(int)=0);

	OGRwkbGeometryType		m_eGeomType;

	// The size of the features array will match the number of elements
	std::vector<vtFeature*> m_Features;

	vtArray<Field*> m_fields;
	vtProjection	m_proj;

	// remember the filename these feature were loaded from or saved to
	vtString	m_strFilename;
};

/**
 * A set of 2D point features.
 */
class vtFeatureSetPoint2D : public vtFeatureSet
{
public:
	vtFeatureSetPoint2D();

	unsigned int GetNumEntities() const;
	void SetNumGeometries(int iNum);
	void Reserve(int iNum);
	bool ComputeExtent(DRECT &rect) const;
	void Offset(const DPoint2 &p, bool bSelectedOnly = false);
	bool TransformCoords(OCT *pTransform, bool progress_callback(int)=0);
	bool AppendGeometryFrom(vtFeatureSet *pFromSet);

	int AddPoint(const DPoint2 &p);
	void SetPoint(unsigned int num, const DPoint2 &p);
	DPoint2 &GetPoint(unsigned int num) { return m_Point2[num]; }
	const DPoint2 &GetPoint(unsigned int num) const { return m_Point2[num]; }

	int FindClosestPoint(const DPoint2 &p, double epsilon);
	void FindAllPointsAtLocation(const DPoint2 &p, vtArray<int> &found);
	void GetPoint(unsigned int num, DPoint2 &p) const;

	// implement necessary virtual methods
	virtual bool IsInsideRect(int iElem, const DRECT &rect);
	virtual void CopyGeometry(unsigned int from, unsigned int to);
	virtual void SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)=0) const;
	virtual void LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int)=0);

protected:
	DLine2	m_Point2;	// wkbPoint
};

/**
 * A set of 3D point features.
 */
class vtFeatureSetPoint3D : public vtFeatureSet
{
public:
	vtFeatureSetPoint3D();

	unsigned int GetNumEntities() const;
	void SetNumGeometries(int iNum);
	void Reserve(int iNum);
	bool ComputeExtent(DRECT &rect) const;
	void Offset(const DPoint2 &p, bool bSelectedOnly = false);
	bool TransformCoords(OCT *pTransform, bool progress_callback(int)=0);
	bool AppendGeometryFrom(vtFeatureSet *pFromSet);

	int AddPoint(const DPoint3 &p);
	void SetPoint(unsigned int num, const DPoint3 &p);
	void GetPoint(unsigned int num, DPoint3 &p) const;
	DPoint3 &GetPoint(unsigned int num) { return m_Point3[num]; }
	const DPoint3 &GetPoint(unsigned int num) const { return m_Point3[num]; }
	const DLine3 &GetAllPoints() const { return m_Point3; }
	bool ComputeHeightRange(float &fmin, float &fmax);

	// implement necessary virtual methods
	virtual bool IsInsideRect(int iElem, const DRECT &rect);
	virtual void CopyGeometry(unsigned int from, unsigned int to);
	virtual void SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)=0) const;
	virtual void LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int)=0);

protected:
	DLine3	m_Point3;	// wkbPoint25D
};

/**
 * A set of 2D linestring features, also known as "polylines."  Each
 *  linestring is basically a simple set of 2D points.
 */
class vtFeatureSetLineString : public vtFeatureSet
{
public:
	vtFeatureSetLineString();

	unsigned int GetNumEntities() const;
	void SetNumGeometries(int iNum);
	void Reserve(int iNum);
	bool ComputeExtent(DRECT &rect) const;
	void Offset(const DPoint2 &p, bool bSelectedOnly = false);
	bool TransformCoords(OCT *pTransform, bool progress_callback(int)=0);
	bool AppendGeometryFrom(vtFeatureSet *pFromSet);

	int AddPolyLine(const DLine2 &pl);
	const DLine2 &GetPolyLine(unsigned int num) const { return m_Line[num]; }
	DLine2 &GetPolyLine(unsigned int num) { return m_Line[num]; }
	int NumTotalVertices() const;

	// implement necessary virtual methods
	virtual bool IsInsideRect(int iElem, const DRECT &rect);
	virtual void CopyGeometry(unsigned int from, unsigned int to);
	virtual void SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)=0) const;
	virtual void LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int)=0);

protected:
	DLine2Array	m_Line;		// wkbLineString
};

/**
 * A set of 3D linestring features, also known as "polylines."  Each
 *  linestring is basically a simple set of 3D points.
 */
class vtFeatureSetLineString3D : public vtFeatureSet
{
public:
	vtFeatureSetLineString3D();

	unsigned int GetNumEntities() const;
	void SetNumGeometries(int iNum);
	void Reserve(int iNum);
	bool ComputeExtent(DRECT &rect) const;
	void Offset(const DPoint2 &p, bool bSelectedOnly = false);
	bool TransformCoords(OCT *pTransform, bool progress_callback(int)=0);
	bool AppendGeometryFrom(vtFeatureSet *pFromSet);

	int AddPolyLine(const DLine3 &pl);
	const DLine3 &GetPolyLine(unsigned int num) const { return m_Line[num]; }
	DLine3 &GetPolyLine(unsigned int num) { return m_Line[num]; }
	bool ComputeHeightRange(float &fmin, float &fmax);
	int NumTotalVertices() const;
	bool FindClosest(const DPoint2 &p, int &close_feature, DPoint3 &close_point);

	// implement necessary virtual methods
	virtual bool IsInsideRect(int iElem, const DRECT &rect);
	virtual void CopyGeometry(unsigned int from, unsigned int to);
	virtual void SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)=0) const;
	virtual void LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int)=0);

protected:
	std::vector<DLine3>	m_Line;		// wkbLineString25D
};


typedef std::vector<int> IntVector;
typedef IntVector *IntVectorPtr;

//
// A utility class to speed up polygon testing operations
class SpatialIndex
{
public:
	SpatialIndex(int iSize);
	~SpatialIndex();

	void GenerateIndices(const class vtFeatureSetPolygon *feat);
	const IntVector *GetIndexForPoint(const DPoint2 &p) const;
	int m_iLastFound;

protected:
	int m_iSize;
	IntVectorPtr *m_pArray;
	DPoint2 m_base, m_step;
	DRECT m_Extent;
};

/**
 * A set of polygon features.  Each polygon is a DPolygon2 object,
 *  which consists of a number of rings: one external ring, and any
 *  number of internal rings (holes).
 */
class vtFeatureSetPolygon : public vtFeatureSet
{
public:
	vtFeatureSetPolygon();

	unsigned int GetNumEntities() const;
	void SetNumGeometries(int iNum);
	void Reserve(int iNum);
	bool ComputeExtent(DRECT &rect) const;
	void Offset(const DPoint2 &p, bool bSelectedOnly = false);
	bool TransformCoords(OCT *pTransform, bool progress_callback(int)=0);
	bool AppendGeometryFrom(vtFeatureSet *pFromSet);

	int AddPolygon(const DPolygon2 &poly);
	void SetPolygon(unsigned int num, const DPolygon2 &poly) { m_Poly[num] = poly; }
	const DPolygon2 &GetPolygon(unsigned int num) const { return m_Poly[num]; }
	DPolygon2 &GetPolygon(unsigned int num) { return m_Poly[num]; }
	int FindSimplePolygon(const DPoint2 &p) const;
	int FindPolygon(const DPoint2 &p) const;

	// Try to address some kinds of degenerate geometry that can occur in polygons
	int FixGeometry(double dEpsilon);
	int SelectBadFeatures(double dEpsilon);

	// speed optimization
	void CreateIndex(int iSize);
	void FreeIndex();

	// implement necessary virtual methods
	virtual bool IsInsideRect(int iElem, const DRECT &rect);
	virtual void CopyGeometry(unsigned int from, unsigned int to);
	virtual void SaveGeomToSHP(SHPHandle hSHP, bool progress_callback(int)=0) const;
	virtual void LoadGeomFromSHP(SHPHandle hSHP, bool progress_callback(int)=0);

protected:
	DPolyArray	m_Poly;		// wkbPolygon

	// speed optimization
	SpatialIndex *m_pIndex;
};

class vtFeatureLoader
{
public:
	virtual vtFeatureSet *LoadFrom(const char *filename);
	vtFeatureSet *LoadFromSHP(const char *filename, bool progress_callback(int) = NULL);
	vtFeatureSet *LoadHeaderFromSHP(const char *filename);
	vtFeatureSet *LoadWithOGR(const char *filename, bool progress_callback(int) = NULL);
	vtFeatureSet *LoadWithOGR(OGRLayer *pLayer, bool progress_callback(int) = NULL);
	vtFeatureSet *LoadFromIGC(const char *filename);
	vtFeatureSet *LoadFromDXF(const char *filename, bool progress_callback(int) = NULL);

	vtFeatureSet *ReadFeaturesFromWFS(const char *szServerURL, const char *layername);

	vtString m_strErrorMsg;
};

// Helpers
OGRwkbGeometryType GetFeatureGeomType(const char *filename);
OGRwkbGeometryType ShapelibToOGR(int nSHPType);
int OGRToShapelib(OGRwkbGeometryType eGeomType);
vtString MakeDBFName(const char *filename);
int GetIntFromString(const char *buf, int len);
bool GeometryTypeIs3D(OGRwkbGeometryType type);
vtString GetShapeTypeName(int nShapeType);

bool SHPToDPolygon2(SHPObject *pObj, DPolygon2 &dpoly);
void DPolygon2ToOGR(const DPolygon2 &dp, OGRPolygon &op);
void OGRToDPolygon2(const OGRPolygon &op, DPolygon2 &dp);

#endif // VTDATA_FEATURES

