//
// ElevLayer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ELEVLAYER_H
#define ELEVLAYER_H

#include "wx/image.h"
#include "vtdata/ElevationGrid.h"
#include "Layer.h"

//////////////////////////////////////////////////////////

class vtElevLayer : public vtLayer 
{
public:
	vtElevLayer();
	vtElevLayer(const DRECT &area, int iColumns, int iRows,
		bool bFloats, vtProjection proj);
	virtual ~vtElevLayer();

	// overrides
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	void SetProjection(vtProjection &proj);
	void GetPropertyText(wxString &str);

	void DrawLayerBitmap(wxDC* pDC, vtScaledView *pView);
	void DrawLayerOutline(wxDC* pDC, vtScaledView *pView);
	bool AppendDataFrom(vtLayer *pL);
	void ReRender() { m_bBitmapRendered = false; }
	void ReImage();

	void SetupDefaults();

	// heightfield operations
//	double GetWidth() { return m_pGrid->GetExtents().Width(); }
//	double GetHeight() { return m_pGrid->GetExtents().Height(); }
	void Resample(int iNewWidth, int iNewHeight);
	void FillGaps();
	void DetermineMeterSpacing();
	void Offset(const DPoint2 &p);
	DRECT GetExtents();
	bool ImportFromFile(wxString &strFileName, void progress_callback(int am));

	// drawing
	void PaintDibFromElevation(vtDIB *dib, bool bShade);
	void GenerateShadedColor(int i, int j, int &r, int &g, int &b);
	void GenerateColorFromGrid1(int i, int j, int &r, int &g, int &b);
	void GenerateColorFromGrid2(int i, int j, int &r, int &g, int &b);
	void ShadePixel(int i, int j, int &r, int &g, int &b, int bias);
	void SetupBitmap(wxDC* pDC);
	void RenderBitmap();

	static bool	m_bShowElevation;
	static bool	m_bShading;
	static bool m_bDoMask;

	vtElevationGrid	*m_pGrid;

protected:
	bool	m_bHasBitmap;
	bool	m_bBitmapRendered;
	bool	m_bHasMask;
	float	m_fSpacing;

	int m_iRows, m_iColumns;

	wxBitmap	*m_pBitmap;
	wxMask		*m_pMask;
	wxImage		*m_pImage;
};

#endif
