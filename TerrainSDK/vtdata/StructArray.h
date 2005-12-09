//
// StructArray.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTARRAYH
#define STRUCTARRAYH

#include "MathTypes.h"
#include "Projections.h"
#include "Structure.h"
#include "HeightField.h"
#include <stdio.h>


/** This structure stores a description of how an imported file should be
   intepreted as built structure data. */
class StructImportOptions
{
public:
	vtStructureType type;
	DRECT			rect;
	bool			bInsideOnly;
	bool			bFlip;
	bool			bBuildFoundations;
	bool			bUse25DForElevation;

	vtString		m_strLayerName;

	vtString		m_strFieldNameHeight;
	enum HeightType { STORIES, METERS, FEET } m_HeightType;

	vtString		m_strFieldNameElevation;
	enum ElevationType { ETMETERS, ETFEET } m_ElevationType;

	vtString		m_strFieldNameFile;

	vtHeightField	*pHeightField;
};

// Well known import schemas

typedef enum
{
	SCHEMA_OSGB_TOPO_AREA,
	SCHEMA_OSGB_TOPO_POINT,
	SCHEMA_OSGB_TOPO_LINE,
	SCHEMA_UI
} SchemaType;

/**
 * The vtStructureArray class contains a list of Built Structures
 * (vtStructure objects).  It can be loaded and saved to VTST files
 * with the ReadXML and WriteXML methods.
 *
 */
class vtStructureArray : public Array<vtStructure*>
{
public:
	vtStructureArray();
	virtual ~vtStructureArray() { Empty(); free(m_Data); m_Data = NULL; m_MaxSize = 0; }
	virtual void DestructItems(unsigned int first, unsigned int last);

	void SetFilename(const vtString &str) { m_strFilename = str; }
	vtString GetFilename() { return m_strFilename; }

	int GetFirstSelected();
	int GetNextSelected();
	void DeleteSelected();
	virtual void DestroyStructure(int i) {}

	bool ReadSHP(const char *pathname, StructImportOptions &opt,
		bool progress_callback(int) = NULL);
	void AddElementsFromOGR(class OGRDataSource *datasource,
		StructImportOptions &opt, bool progress_callback(int) = NULL);

	void AddElementsFromOGR_SDTS(class OGRDataSource *datasource,
		bool progress_callback(int) = NULL);
	void AddElementsFromOGR_RAW(class OGRDataSource *datasource,
		StructImportOptions &opt, bool progress_callback(int) = NULL);

	void AddBuildingsFromOGR(class OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int) = NULL);
	void AddLinearsFromOGR(class OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int) = NULL);
	void AddInstancesFromOGR(class OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int) = NULL);

	bool ReadBCF(const char *pathname);		// read a .bcf file
	bool ReadBCF_Old(FILE *fp);				// support obsolete format
	bool ReadXML(const char *pathname, bool progress_callback(int) = NULL);

	bool WriteSHP(const char *pathname);
	bool WriteXML(const char *pathname, bool bGZip = false) const;

	bool FindClosestBuildingCorner(const DPoint2 &point, double epsilon,
						   int &building, int &corner, double &distance);
	bool FindClosestBuildingCenter(const DPoint2 &point, double epsilon,
						   int &building, double &distance);
	bool FindClosestLinearCorner(const DPoint2 &point, double epsilon,
						   int &structure, int &corner, double &distance);

	bool FindClosestStructure(const DPoint2 &point, double epsilon,
			int &structure, double &distance, float fMaxInstRadius = 1E9f, bool bSkipBuildings = false);
	bool FindClosestBuilding(const DPoint2 &point, double epsilon,
			int &structure, double &closest);

	bool IsEmpty() { return (GetSize() == 0); }
	void GetExtents(DRECT &ext) const;
	void Offset(const DPoint2 &delta);

	int AddFoundations(vtHeightField *pHF);
	void RemoveFoundations();

	// selection
	int NumSelected();
	void DeselectAll();

	// override these 'Factory' methods so that the vtStructureArray base
	// methods can be capable of handling subclasses of vtBuilding
	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	// convenience methods that create, add, and return a new structure
	vtBuilding *AddNewBuilding();
	vtFence *AddNewFence();
	vtStructInstance *AddNewInstance();

	vtBuilding *AddBuildingFromLineString(class OGRLineString *pLineString);

	// override to catch edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

	vtProjection m_proj;

protected:
	vtString	m_strFilename;

	// used to indicate which edge should be hightlighted during editing
	vtBuilding *m_pEditBuilding;
	int m_iEditLevel;
	int m_iEditEdge;
	int m_iLastSelected;
};

extern vtStructureArray g_DefaultStructures;

// Helpers
int GetSHPType(const char *filename);

bool SetupDefaultStructures(const vtString &fname);
vtBuilding *GetClosestDefault(vtBuilding *pBld);
vtFence *GetClosestDefault(vtFence *pFence);
vtStructInstance *GetClosestDefault(vtStructInstance *pInstance);

#endif	// STRUCTARRAYH

