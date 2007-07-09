//
// ElevLayer.h
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ELEVLAYER_H
#define ELEVLAYER_H

#include "wx/image.h"
#include "Layer.h"
#include "vtdata/vtTin.h"

#define SHADING_BIAS	200

class vtHeightField;
class vtElevationGrid;
class vtDIB;
class vtBitmap;
class BuilderView;

class vtTin2d : public vtTin
{
public:
	vtTin2d();

	void DrawTin(wxDC* pDC, vtScaledView *pView);
	void ComputeEdgeLengths();
	void CullLongEdgeTris();
	void FreeEdgeLengths();
	void SetConstraint(bool bConstrain, double fMaxEdge);

	double *m_fEdgeLen;
	bool m_bConstrain;
	double m_fMaxEdge;
};

//////////////////////////////////////////////////////////

class vtElevLayer : public vtLayer
{
public:
	vtElevLayer();
	vtElevLayer(const DRECT &area, int iColumns, int iRows,
		bool bFloats, float fScale, const vtProjection &proj);
	virtual ~vtElevLayer();

	// overrides
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool TransformCoords(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);

	// optional overrides
	bool SetExtent(const DRECT &rect);
	void GetPropertyText(wxString &str);
	wxString GetFileExtension();
	bool AskForSaveFilename();
	bool GetAreaExtent(DRECT &rect);

	void OnLeftDown(BuilderView *pView, UIContext &ui);
	void OnLeftUp(BuilderView *pView, UIContext &ui);
	void OnMouseMove(BuilderView *pView, UIContext &ui);

	void DrawLayerBitmap(wxDC* pDC, vtScaledView *pView);
	void DrawLayerOutline(wxDC* pDC, vtScaledView *pView);
	bool AppendDataFrom(vtLayer *pL);
	void ReRender() { m_bBitmapRendered = false; m_bNeedsDraw = true; }
	void ReImage();
	bool IsGrid() { return m_pGrid != NULL; }

	void SetupDefaults();

	// heightfield operations
	vtHeightField *GetHeightField();
	void Offset(const DPoint2 &p);
	float GetElevation(const DPoint2 &p);
	bool GetHeightExtents(float &fMinHeight, float &fMaxHeight) const;
	bool ImportFromFile(const wxString &strFileName, bool progress_callback(int) = NULL);

	// grid operations
	void Resample(int iNewWidth, int iNewHeight);
	bool FillGaps();
	void DetermineMeterSpacing();
	bool WriteGridOfTilePyramids(const TilingOptions &opts, BuilderView *pView);

	// TIN operations
	void SetTin(vtTin2d *pTin);
	vtTin2d *GetTin() { return m_pTin; }
	void MergeSharedVerts(bool bSilent = false);

	// drawing
	void SetupBitmap(wxDC* pDC);
	void RenderBitmap();
	static void SetupDefaultColors(ColorMap &cmap);

	static ElevDrawOptions m_draw;

	vtElevationGrid	*m_pGrid;
	vtTin2d *m_pTin;

	bool NeedsDraw();

protected:
	bool	m_bNeedsDraw;
	bool	m_bBitmapRendered;
	bool	m_bHasMask;
	float	m_fSpacing;
	bool	m_bPreferGZip;	// user wants their elevation treated as a .gz file

	int m_iColumns, m_iRows;
	int m_iImageWidth, m_iImageHeight;

	vtBitmap	*m_pBitmap;
	wxMask		*m_pMask;
};

// Helper
FPoint3 LightDirection(float angle, float direction);

#endif	// ELEVLAYER_H

