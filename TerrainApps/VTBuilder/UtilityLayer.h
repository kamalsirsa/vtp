//
// UtilityLayer.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef UTILITYLAYER_H
#define UTILITYLAYER_H

#include "Layer.h"
#include "vtdata/UtilityMap.h"

//////////////////////////////////////////////////////////

class vtUtilityLayer : public vtLayer, public vtUtilityMap
{
public:
	vtUtilityLayer();

	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC *pDC, vtScaledView *pView);
	bool TransformCoords(vtProjection &proj);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	bool AppendDataFrom(vtLayer *pL);
	void Offset(const DPoint2 &delta);

	void DeselectAll();
	void InvertSelection();

protected:
	void DrawPole(wxDC *pDC, vtScaledView *pView, vtPole *pole);

	int m_size;
};

#endif

