//
// ImageLayer.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef IMAGELAYER_H
#define IMAGELAYER_H

#include "wx/image.h"
#include "Layer.h"

//////////////////////////////////////////////////////////

class vtImageLayer : public vtLayer 
{
public:
	vtImageLayer();
	virtual ~vtImageLayer();

	// overrides for vtLayer methods
	bool GetExtent(DRECT &rect);
	bool GetAreaExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, class vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);

private:
	bool LoadFromGDAL();
	vtProjection	m_Proj;
	wxImage		*m_pImage;
	wxBitmap	*m_pBitmap;
	DRECT   m_Extents;
};

#endif
