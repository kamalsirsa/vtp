//
// WaterLayer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef WATERLAYER_H
#define WATERLAYER_H

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/RoadMap.h"
#include "Layer.h"

class vtDIB;

//////////////////////////////////////////////////////////

class vtWaterFeature : public DLine2
{
public:
	vtWaterFeature() : DLine2() { m_bIsBody = false; }
	bool m_bIsBody;
};

class vtWaterLayer : public vtLayer
{
public:
	vtWaterLayer();
	~vtWaterLayer();

	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	void Offset(const DPoint2 &p);

	void AddFeature(vtWaterFeature *pFeat);
	vtWaterFeature *GetFeature(int i) { return (vtWaterFeature *) m_Lines[i]; }

	void PaintDibWithWater(vtDIB *dib);

	void AddElementsFromDLG(vtDLGFile *pDlg);
	void AddElementsFromSHP(const char *filename, vtProjection &proj);

	// Import from SDTS via OGR
	void AddElementsFromOGR(class OGRDataSource *datasource,
		void progress_callback(int) = NULL);

protected:
	// data for rivers and water bodies
	// eventually, should have vector+width data for rivers, area data for bodies
	// for now, just use plain vectors for everything
	DPolyArray2		m_Lines;
	vtProjection	m_proj;
};

#endif

