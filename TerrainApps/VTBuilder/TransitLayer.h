//
// TransitLayer.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TRANSITLAYER_H
#define TRANSITLAYER_H

#include "Layer.h"

//////////////////////////////////////////////////////////

class vtTransitLayer : public vtLayer
{
public:
	vtTransitLayer();
	~vtTransitLayer();

#if SUPPORT_TRANSIT
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC *pDC, vtScaledView *pView);
	bool TransformCoords(vtProjection &proj);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void Offset(const DPoint2 &p);

protected:
	// data for rivers and water bodies
	// eventually, should have vector+width data for rivers, area data for bodies
	// for now, just use plain vectors for everything
	vtProjection	m_proj;

	void KludgeRefPosApply(bool bForward);

	void drawSeg(wxDC *pDC, vtScaledView *pView, double x1, double y1, double x2, double y2);
#endif
};

#endif
