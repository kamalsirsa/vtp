//
// ImageLayer.h
//
// Copyright (c) 2002-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef IMAGELAYER_H
#define IMAGELAYER_H

class vtImage;

#include "Layer.h"

///////////////////////////////////////////////////////////////////////

class vtImageLayer : public vtLayer
{
public:
	vtImageLayer();
	vtImageLayer(const DRECT &area, int xsize, int ysize,
		const vtProjection &proj);
	virtual ~vtImageLayer();

	// overrides for vtLayer methods
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC *pDC, vtScaledView *pView);
	bool TransformCoords(vtProjection &proj);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	void Offset(const DPoint2 &delta);

	// optional overrides
	bool SetExtent(const DRECT &rect);
	void GetPropertyText(wxString &str);

	vtImage *GetImage() const { return m_pImage; }
	DPoint2 GetSpacing() const;
	bool ImportFromFile(const wxString &strFileName, bool progress_callback(int) = NULL);
	bool ImportFromDB(const char *szFileName, bool progress_callback(int) = NULL);
	void ReplaceColor(const RGBi &rgb1, const RGBi &rgb2);

protected:
	vtImage	*m_pImage;
};

// Helper
int GetBitDepthUsingGDAL(const char *fname);
void MakeSampleOffsets(const DPoint2 cellsize, uint N, DLine2 &offsets);

#endif	// IMAGELAYER_H
