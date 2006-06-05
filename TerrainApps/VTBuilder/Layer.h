//
// Layer.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LAYER_H
#define LAYER_H

#include "vtdata/MathTypes.h"
#include "vtdata/Projections.h"
#include "vtui/wxString2.h"

enum LayerType
{
	LT_UNKNOWN = -1,
	LT_RAW,
	LT_ELEVATION,
	LT_IMAGE,
	LT_ROAD,
	LT_STRUCTURE,
	LT_WATER,
	LT_VEG,
	LT_TRANSIT,
	LT_UTILITY,
	LAYER_TYPES
};

class BuilderView;
class vtScaledView;
struct UIContext;

class vtLayer
{
public:
	vtLayer(LayerType type);
	virtual ~vtLayer();

	// attributes
	LayerType GetType() { return m_type; }
	bool SetVisible(bool bVisible);
	bool GetVisible() { return m_bVisible; }
	void SetModified(bool bModified);
	bool GetModified() { return m_bModified; }
	bool IsNative() { return m_bNative; }

	wxString2 GetImportedFrom() { return m_wsImportedFrom; }
	void SetImportedFrom(const wxString2 &fname) { m_wsImportedFrom = fname; }

	// operations
	static vtLayer *CreateNewLayer(LayerType ltype);
	bool Save(const wxString &filename = _T(""));
	bool Load(const wxString &filename = _T(""));

	// these must be implemented
	virtual bool GetExtent(DRECT &rect) = 0;
	virtual void DrawLayer(wxDC* pDC, vtScaledView *pView) = 0;
	virtual bool TransformCoords(vtProjection &proj) = 0;
	virtual bool OnSave() = 0;
	virtual bool OnLoad() = 0;
	virtual bool AppendDataFrom(vtLayer *pL) = 0;
	virtual void GetProjection(vtProjection &proj) = 0;
	virtual void SetProjection(const vtProjection &proj) = 0;
	virtual void Offset(const DPoint2 &p);

	// these may be optionally implemented
	virtual bool SetExtent(const DRECT &rect) { return false; }
	virtual void GetPropertyText(wxString &str) {}
	virtual wxString GetFileExtension();
	virtual bool CanBeSaved() { return true; }
	virtual wxString2 GetLayerFilename() { return m_wsFilename; }
	virtual void SetLayerFilename(const wxString2 &fname);
	virtual bool AskForSaveFilename();
	vtString GetExportFilename(const wxString &format_filter);
	bool GetAreaExtent(DRECT &rect) { return GetExtent(rect); }

	// UI event handlers which can be implemented if desired
	virtual void OnLeftDown(BuilderView *pView, UIContext &ui) {}
	virtual void OnLeftUp(BuilderView *pView, UIContext &ui) {}
	virtual void OnRightDown(BuilderView *pView, UIContext &ui) {}
	virtual void OnRightUp(BuilderView *pView, UIContext &ui) {}
	virtual void OnLeftDoubleClick(BuilderView *pView, UIContext &ui) {}
	virtual void OnMouseMove(BuilderView *pView, UIContext &ui) {}

	static wxArrayString LayerTypeNames;
	static wxChar *LayerFileExtension[];

protected:
	wxString GetSaveFileDialogFilter();
	void SetMessageText(const wxString &msg);

	// this filename is only used if the layer subclass doesn't have its own
	wxString2	m_wsFilename;

	// remember what file this layer was imported from
	wxString2	m_wsImportedFrom;

	LayerType	m_type;
	bool		m_bVisible;
	bool		m_bModified;
	bool		m_bNative;
};

typedef vtLayer *vtLayerPtr;


//
// Name: LayerArray
// An array of layer objects.
//
class LayerArray : public vtArray<vtLayerPtr>
{
public:
	// don't need explicit destructor here because Empty() is always called
	virtual void DestructItems(unsigned int first, unsigned int last);
};

class DrawStyle
{
public:
	DrawStyle();

	RGBi m_LineColor;
	RGBi m_FillColor;
	bool m_bFill;
	int m_MarkerShape;	// 0 = circle, 1 = crosshair, this should be an enum
	int m_MarkerSize;	// in pixels
};

class ElevDrawOptions
{
public:
	ElevDrawOptions()
	{
		m_bShowElevation = true;
		m_bShadingQuick = true;
		m_bShadingDot = false;
		m_bCastShadows = false;
		m_bDoMask = true;
		m_iCastAngle = 30;
		m_iCastDirection = 90;
		m_strColorMapFile = "";
	}
	bool operator != (const ElevDrawOptions &val)
	{
		return (m_bShowElevation != val.m_bShowElevation ||
			m_bShadingQuick != val.m_bShadingQuick ||
			m_bShadingDot != val.m_bShadingDot ||
			m_bCastShadows != val.m_bCastShadows ||
			m_bDoMask != val.m_bDoMask ||
			m_iCastAngle != val.m_iCastAngle ||
			m_iCastDirection != val.m_iCastDirection ||
			m_strColorMapFile != val.m_strColorMapFile);
	}
	bool m_bShowElevation;
	bool m_bShadingQuick;
	bool m_bShadingDot;
	bool m_bCastShadows;
	bool m_bDoMask;
	int m_iCastAngle;
	int m_iCastDirection;
	vtString m_strColorMapFile;
};

struct TilingOptions
{
	int cols, rows;
	int lod0size;
	int numlods;
	vtString fname;

	// If this is an elevation tileset, then optionally a corresponding
	//  derived image tileset can be created.
	bool bCreateDerivedImages;
	vtString fname_images;
	ElevDrawOptions draw;
};

////////////////////
// Helpers

wxString2 GetLayerTypeName(const LayerType &lype);

#endif

