//
// TowerLayer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TOWERLAYER_H
#define TOWERLAYER_H

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/TowerArray.h"
#include "Layer.h"

//////////////////////////////////////////////////////////

class vtTowerLayer : public vtLayer, public vtTowerArray
{
public:
	vtTowerLayer();

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

	vtTower *FindTower(DPoint2 &point, double epsilon);
	//edit a single tower
	bool EditTowerProperties();
	//edit all selected towers
	bool EditBranchProperties();
	void vtTowerLayer::InvertSelection();

	bool SelectTower(DPoint2 utmCoord, float error, DRECT &bound);

	void DrawTower(wxDC* pDC, vtScaledView *pView, vtTower *twr);
	bool AddNewTower(const DPoint2 &p);

protected:
	int m_size;
	bool m_Branch;
};

#endif