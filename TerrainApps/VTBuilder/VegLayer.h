//
// VegLayer.h
//
// Copyright (c) 2001-2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VEGLAYER_H
#define VEGLAYER_H

#include "vtdata/LULC.h"
#include "vtdata/RoadMap.h"
#include "vtdata/Plants.h"
#include "Layer.h"
#include "VegPointOptions.h"

enum VegLayerType {
	VLT_None,
	VLT_Density,
	VLT_BioMap,
	VLT_Instances
};

class vtVegLayer : public vtLayer
{
public:
	vtVegLayer();
	~vtVegLayer();

	// Projection
	vtProjection m_proj;

	// Basics to overwrite vtLayer 
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj_new);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &str);
	bool CanBeSaved();

	// Importing data into veglayer
	VegLayerType m_VLType;
	void AddElementsFromLULC(vtLULCFile *pLULC);
	void AddElementsFromSHP_Polys(const wxString2 &filename, const vtProjection &proj,
		int fieldindex, int datatype);
	bool AddElementsFromSHP_Points(const wxString2 &filename, const vtProjection &proj,
		VegPointOptions &opt);

	// Search functionality
	float FindDensity(const DPoint2 &p);
	int   FindBiotype(const DPoint2 &p);

	// Exporting data
	bool ExportToSHP(const char *fname);

protected:
	void DrawInstances(wxDC* pDC, vtScaledView *pView);
	void DrawPolys(wxDC* pDC, vtScaledView *pView);

	// Array of vegpolys made from poly attrib and array of utm points points
	DPolyArray2		m_Poly;
	Array<float>	m_Density;
	Array<short>	m_Biotype;
	vtPlantInstanceArray m_Pia;
};

#endif


