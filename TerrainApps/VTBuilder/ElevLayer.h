//
// ElevLayer.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ELEVLAYER_H
#define ELEVLAYER_H

#include "wx/image.h"
#include "Layer.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/vtTin.h"
#include "ElevDrawOptions.h"
#include "TilingOptions.h"

#include <set>

#define SHADING_BIAS	200

class vtBitmap;
class vtDIB;
class vtElevationGrid;
class vtFeatureSet;
class vtFeatureSetPoint3D;
class vtFeatureSetPolygon;
class vtHeightField;

struct IntPair
{
	IntPair() {}
	bool operator <(const IntPair &b) const
	{
		if (v0 < b.v0)
			return true;
		else if (v0 > b.v0)
			return false;
		else
		{
			if (v1 < b.v1)
				return true;
			else
				return false;
		}
	}
	bool operator==(const IntPair &b)
	{
		return (v0 == b.v0 && v1 == b.v1);
	}
	IntPair(int i0, int i1) { v0 = i0; v1 = i1; }
	int v0, v1;
};

struct Outline : public std::set<IntPair>
{
	void AddUniqueEdge(const IntPair &b)
	{
		iterator it = find(b);
		if (it != end())
			erase(it);
		else
			insert(b);
	}
};

class vtTin2d : public vtTin
{
public:
	vtTin2d();
	~vtTin2d();

	vtTin2d(vtElevationGrid *grid);
	vtTin2d(vtFeatureSetPoint3D *set);
	vtTin2d(vtFeatureSetPolygon *set, int iFieldNum);

	void DrawTin(wxDC *pDC, vtScaledView *pView);
	void ComputeEdgeLengths();
	void CullLongEdgeTris();
	void FreeEdgeLengths();
	void SetConstraint(bool bConstrain, double fMaxEdge);
	void MakeOutline();
	int GetMemoryUsed() const;

	double *m_fEdgeLen;
	bool m_bConstrain;
	double m_fMaxEdge;

	Outline m_edges;
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
	void DrawLayer(wxDC *pDC, vtScaledView *pView);
	bool TransformCoords(vtProjection &proj);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);

	// optional overrides
	bool SetExtent(const DRECT &rect);
	void GetPropertyText(wxString &str);
	wxString GetFileExtension();
	bool AskForSaveFilename();
	bool GetAreaExtent(DRECT &rect);

	int GetMemoryUsed() const;
	int MemoryNeededToLoad() const;
	void FreeData();
	bool HasData();

	void OnLeftDown(BuilderView *pView, UIContext &ui);
	void OnLeftUp(BuilderView *pView, UIContext &ui);
	void OnMouseMove(BuilderView *pView, UIContext &ui);

	void DrawLayerBitmap(wxDC *pDC, vtScaledView *pView);
	void DrawLayerOutline(wxDC *pDC, vtScaledView *pView);
	bool AppendDataFrom(vtLayer *pL);
	void ReRender();
	void ReImage();
	bool IsGrid() { return m_pGrid != NULL; }
	void SetPreferGZip(bool val) { m_bPreferGZip = val; }

	void SetupDefaults();

	// heightfield operations
	vtHeightField *GetHeightField();
	void Offset(const DPoint2 &p);
	float GetElevation(const DPoint2 &p);
	bool GetHeightExtents(float &fMinHeight, float &fMaxHeight) const;
	bool ImportFromFile(const wxString &strFileName, bool progress_callback(int) = NULL);
	bool CreateFromPoints(vtFeatureSet *set, int iXSize, int iYSize,
		float fDistanceRatio);

	// grid operations
	vtElevationGrid	*GetGrid() { return m_pGrid; }
	int RemoveElevRange(float zmin, float zmax, const DRECT *area = NULL);
	int SetUnknown(float fValue, const DRECT *area = NULL);
	void DetermineMeterSpacing();
	bool WriteGridOfElevTilePyramids(TilingOptions &opts, BuilderView *pView);
	bool ImportFromDB(const char *szFileName, bool progress_callback(int));

	// TIN operations
	void SetTin(vtTin2d *pTin);
	vtTin2d *GetTin() { return m_pTin; }
	void MergeSharedVerts(bool bSilent = false);
	void SetupTinTriangleBins(int target_triangles_per_bin);

	// drawing
	void SetupBitmap(wxDC *pDC);
	void RenderBitmap();
	static void SetupDefaultColors(ColorMap &cmap);

	static ElevDrawOptions m_draw;
	static bool m_bDefaultGZip;

	// only this many elevation files may be loaded, the rest are paged out on an LRU basis
	static int m_iElevMemLimit;

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

// Helpers
FPoint3 LightDirection(float angle, float direction);
bool MatchTilingToResolution(const DRECT &original_area, const DPoint2 &resolution,
							int &iTileSize, bool bGrow, bool bShrink, DRECT &new_area,
							IPoint2 &tiling);
bool ElevCacheOpen(vtElevLayer *pLayer, const char *fname, enum vtElevError *err);
bool ElevCacheLoadData(vtElevLayer *elev);
void ElevCacheRemove(vtElevLayer *elev);

#endif	// ELEVLAYER_H

