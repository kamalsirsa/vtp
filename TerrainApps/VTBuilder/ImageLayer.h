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
	vtImageLayer(const DRECT &area, int xsize, int ysize,
		const vtProjection &proj);
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
	DPoint2 GetSpacing();

	void GetDimensions(int &xsize, int &ysize)
	{
		xsize = m_iXSize;
		ysize = m_iYSize;
	}
	bool GetFilteredColor(double x, double y, RGBi &rgb);
	bool SaveToFile(const char *fname);
	wxImage *GetImage() { return m_pImage; }

protected:
	bool LoadFromGDAL();
	vtProjection	m_proj;

	bool	m_bInMemory;
	DRECT   m_Extents;
	int		m_iXSize;
	int		m_iYSize;

	wxImage		*m_pImage;
	wxBitmap	*m_pBitmap;
};

#endif
