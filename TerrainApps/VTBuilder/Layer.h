//
// Layer.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LAYER_H
#define LAYER_H

#include "vtdata/MathTypes.h"
#include "vtdata/vtString.h"

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
	LT_UTILITY,
#if SUPPORT_TRANSIT
	LT_TRANSIT,
#endif
	LAYER_TYPES
};

class BuilderView;
class vtScaledView;
class vtProjection;
struct UIContext;

/**
 * A layer is a single blob of data, generally stored in memory and drawn in
 * the main view.  Each layer has a type (Raw, Elevation, Image, etc.) and a CRS.
 * vtLayer is an abstract base class for all the layer classes.
 * It defines a set of operations which each layer class may implement.
 */
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
	void SetSticky(bool bSticky) { m_bSticky = bSticky; }
	bool GetSticky() { return m_bSticky; }
	bool IsNative() { return m_bNative; }

	wxString GetImportedFrom() { return m_wsImportedFrom; }
	void SetImportedFrom(const wxString &fname) { m_wsImportedFrom = fname; }

	// operations
	static vtLayer *CreateNewLayer(LayerType ltype);
	bool Save(bool progress_callback(int) = NULL);
	bool SaveAs(const wxString &filename = _T(""), bool progress_callback(int) = NULL);
	bool Load(const wxString &filename = _T(""));

	// these must be implemented:
	/// Get the extents
	virtual bool GetExtent(DRECT &rect) = 0;
	virtual void DrawLayer(wxDC *pDC, vtScaledView *pView) = 0;
	/// Transform the coordinates into another CRS
	virtual bool TransformCoords(vtProjection &proj) = 0;
	virtual bool OnSave(bool progress_callback(int) = NULL) = 0;
	virtual bool OnLoad() = 0;
	/// Merge the contents of another layer (of the same type)
	virtual bool AppendDataFrom(vtLayer *pL) = 0;
	/// Get the CRS
	virtual void GetProjection(vtProjection &proj) = 0;
	/// Set the CRS, which does not reproject
	virtual void SetProjection(const vtProjection &proj) = 0;
	/// Shift all the coordinates by a horizontal offset
	virtual void Offset(const DPoint2 &p);

	// these may be optionally implemented:
	virtual bool SetExtent(const DRECT &rect) { return false; }
	virtual void GetPropertyText(wxString &str) {}
	virtual wxString GetFileExtension();
	virtual bool CanBeSaved() { return true; }
	virtual wxString GetLayerFilename() { return m_wsFilename; }
	virtual void SetLayerFilename(const wxString &fname);
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
	static const wxChar *LayerFileExtension[];

protected:
	wxString GetSaveFileDialogFilter();
	void SetMessageText(const wxString &msg);

	// this filename is only used if the layer subclass doesn't have its own
	wxString	m_wsFilename;

	// remember what file this layer was imported from
	wxString	m_wsImportedFrom;

	LayerType	m_type;
	bool		m_bVisible;
	bool		m_bModified;
	bool		m_bNative;
	bool		m_bSticky;		// If sticky, don't page out the layer
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
	vtLayer *FindByFilename(const wxString &name);
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

////////////////////
// Helpers

wxString GetLayerTypeName(const LayerType &lype);

#endif

