//
// UtilityLayer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef UTILITYLAYER_H
#define UTILITYLAYER_H

#include "Layer.h"
#include "vtdata/shapelib/shapefil.h"

//////////////////////////////////////////////////////////

class vtUtilityLayer : public vtLayer
{
public:
	vtUtilityLayer();

	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	bool AppendDataFrom(vtLayer *pL);
	void Offset(DPoint2 p);

	void AddElementsFromSHP(const char *filename, vtProjection &proj);
	void AddElementsFromSHPPoints(SHPHandle hSHP, int nElem);
	void AddElementsFromSHPPolygons(const char *filename,
									SHPHandle hSHP, int nElem);

	void DeselectAll();
	void InvertSelection();

protected:
	vtProjection m_proj;
};

#endif