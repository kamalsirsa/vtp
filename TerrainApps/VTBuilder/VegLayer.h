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

	// Basics to overwrite vtLayer 
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetPropertyText(wxString &str);
	bool CanBeSaved();

	// Importing data into veglayer
	VegLayerType m_VLType;
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
	void DrawInstances(wxDC* pDC, vtScaledView *pView);

	int	m_field_density;
	int m_field_biotype;

	vtPlantInstanceArray m_Pia;
};

#endif	// VEGLAYER_H

