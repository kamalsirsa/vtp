//
// VegLayer.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VEGLAYER_H
#define VEGLAYER_H

#include "vtdata/LULC.h"
#include "vtdata/RoadMap.h"
#include "vtdata/Plants.h"
#include "RawLayer.h"
#include "VegPointOptions.h"

enum VegLayerType
{
	VLT_None,
	VLT_Density,
	VLT_BioMap,
	VLT_Instances
};

enum VegImportFieldType
{
	VIFT_Density,
	VIFT_BiotypeName,
	VIFT_BiotypeID
};

class vtVegLayer : public vtRawLayer
{
public:
	vtVegLayer();
	~vtVegLayer();

	// Projection
	vtProjection m_proj;

	// Basics to overwrite vtLayer / vtRawLayer
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetPropertyText(wxString &str);
	bool CanBeSaved();
	wxString GetFileExtension();

	void SetVegType(VegLayerType type);
	VegLayerType GetVegType() { return m_VLType; }

	// Importing data into veglayer
	void AddElementsFromLULC(vtLULCFile *pLULC);
	bool AddElementsFromSHP_Polys(const wxString2 &filename, const vtProjection &proj,
		int fieldindex, VegImportFieldType datatype);
	bool AddElementsFromSHP_Points(const wxString2 &filename, const vtProjection &proj,
		VegPointOptions &opt);

	// Search functionality
	float FindDensity(const DPoint2 &p);
	int   FindBiotype(const DPoint2 &p);

	// Exporting data
	bool ExportToSHP(const char *fname);

protected:
	vtPlantInstanceArray *GetPIA() { return (vtPlantInstanceArray *) m_pSet; }
	vtFeatureSetPolygon *GetPS() { return (vtFeatureSetPolygon *) m_pSet; }
	VegLayerType m_VLType;

	int	m_field_density;
	int m_field_biotype;
};

#endif	// VEGLAYER_H

