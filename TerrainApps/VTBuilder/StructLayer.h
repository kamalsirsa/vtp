//
// StructureLayer.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTLAYER_H
#define STRUCTLAYER_H

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/Features.h"
#include "vtdata/StructArray.h"
#include "vtdata/Features.h"
#include "Layer.h"

class vtDLGFile;
class vtElevLayer;

//////////////////////////////////////////////////////////

class vtStructureLayer : public vtLayer, public vtStructureArray
{
public:
	vtStructureLayer();

	// Implement vtLayer methods
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	bool AppendDataFrom(vtLayer *pL);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &str);

	wxString2 GetLayerFilename() { return wxString2(GetFilename()); }
	void SetLayerFilename(const wxString2 &fname)
	{
		SetFilename(fname.mb_str());
		vtLayer::SetLayerFilename(fname);
	}

	// UI event handlers
	void OnLeftDown(BuilderView *pView, UIContext &ui);
	void OnLeftUp(BuilderView *pView, UIContext &ui);
	void OnRightDown(BuilderView *pView, UIContext &ui);
	void OnMouseMove(BuilderView *pView, UIContext &ui);

	void OnLeftDownEditBuilding(BuilderView *pView, UIContext &ui);
	void OnLeftDownBldAddPoints(BuilderView *pView, UIContext &ui);
	void OnLeftDownBldDeletePoints(BuilderView *pView, UIContext &ui);
	void OnLeftDownEditLinear(BuilderView *pView, UIContext &ui);
	void UpdateMove(UIContext &ui);
	void UpdateRotate(UIContext &ui);
	void UpdateResizeScale(BuilderView *pView, UIContext &ui);

	void DrawBuildingHighlight(wxDC* pDC, vtScaledView *pView);
	bool AddElementsFromSHP(const wxString2 &filename, const vtProjection &proj, DRECT rect);
	void AddElementsFromDLG(vtDLGFile *pDlg);

	bool EditBuildingProperties();
	void AddFoundations(vtElevLayer *pEL);

	void DrawBuilding(wxDC* pDC, vtScaledView *pView, vtBuilding *bld);
	void DrawLinear(wxDC* pDC, vtScaledView *pView, vtFence *fen);

	// inverts selection values on all structures.
	void InvertSelection();
	void DeselectAll();
	int DoBoxSelect(const DRECT &rect, SelectionType st);

	// override to catch edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

protected:
	int m_size;	// size in pixels of the small crosshair at building center
};

#endif	// STRUCTLAYER_H

