//
// StructureLayer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTLAYER_H
#define STRUCTLAYER_H

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/StructArray.h"
#include "Layer.h"

//////////////////////////////////////////////////////////

class vtStructureLayer : public vtLayer, public vtStructureArray
{
public:
	vtStructureLayer();

	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	bool AppendDataFrom(vtLayer *pL);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &str);

	void DrawBuildingHighlight(wxDC* pDC, vtScaledView *pView);
	bool AddElementsFromSHP(const char *filename, vtProjection &proj, DRECT rect);
	void AddElementsFromDLG(vtDLGFile *pDlg);
	// Import from SDTS via OGR
	void AddElementsFromOGR(class OGRDataSource *datasource,
		void progress_callback(int) = NULL);

	vtStructure *FindBuilding(DPoint2 &point, double epsilon);
	bool EditBuildingProperties();

	void DrawBuilding(wxDC* pDC, vtScaledView *pView, vtBuilding *bld);

	// inverts selection values on all structures.
	void InvertSelection();
	void DeselectAll();
	int DoBoxSelect(const DRECT &rect, SelectionType st);

	// override to catch edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

protected:
	int m_size;	// size in pixels of the small crosshair at building center
};

// Helper
int GetSHPType(const char *filename);

#endif

