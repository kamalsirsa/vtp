//
// VegLayer.h
//

#ifndef VEGLAYER_H
#define VEGLAYER_H

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/LULC.h"
#include "vtdata/RoadMap.h"
#include "Layer.h"

enum VegLayerType {
	VLT_Unknown,
	VLT_Density,
	VLT_BioMap
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
	void SetProjection(vtProjection &proj);

	// Importing data into veglayer
	VegLayerType m_VLType;
	void AddElementsFromLULC(vtLULCFile *pLULC);
	void AddElementsFromSHP(const char *filename, vtProjection &proj,
		int fieldindex, int datatype);

	// Search functionality
	float FindDensity(const DPoint2 &p);
	int   FindBiotype(const DPoint2 &p);

protected:
	// Array of vegpolys made from poly attrib and array of utm points points
	DPolyArray2		m_Poly;
	Array<float>	m_Density;
	Array<short>	m_Biotype;

	//six control points outline starting at SW clockwise
//	FPoint2 m_VCtrlPts[6];
};

#endif