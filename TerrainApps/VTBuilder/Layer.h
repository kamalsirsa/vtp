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

//
// Filename filter strings
//
#define FSTRING_BT		_T("BT Files (*.bt)|*.bt")
#define FSTRING_BTGZ	_T("Gzipped BT Files (*.bt.gz)|*.bt.gz")
#define FSTRING_RMF		_T("RMF Files (*.rmf)|*.rmf")
#define FSTRING_BCF		_T("BCF Files (*.bcf)|*.bcf")
#define FSTRING_TIN		_T("TIN Files (*.itf)|*.itf")

#define FSTRING_ADF		_T("Arc Data Files (*.adf)|*.adf")
#define FSTRING_ASC		_T("ArcInfo ASCII grid (*.asc)|*.asc")
#define FSTRING_BMP		_T("Bitmap (*.bmp)|*.bmp")
#define FSTRING_BIL		_T("ArcInfo BIL grid (*.bil)|*.bil")
#define FSTRING_CDF		_T("CDF Files (*.cdf, *.grd)|*.cdf;*.grd")
#define FSTRING_CHU		_T("ChunkLOD Files (*.chu)|*.chu")
#define FSTRING_DEM		_T("DEM Files (*.dem)|*.dem")
#define FSTRING_DLG		_T("USGS DLG Files (*.dlg, *.opt)|*.dlg;*.opt")
#define FSTRING_DOQ		_T("USGS DOQ Files (*.jpg, *.doq, *.coq)|*.doq;*.coq;*.jpg")
#define FSTRING_DTED	_T("DTED Files (*.dte, *.dt0, *.dt1, *.dt2)|*.dte;*.dt0;*.dt1;*.dt2")
#define FSTRING_DXF		_T("DXF Files (*.dxf)|*.dxf")
#define FSTRING_GML		_T("GML Files (*.gml, *.xml)|*.gml;*.xml")
#define FSTRING_GTOPO	_T("GTOPO30 Files (*.hdr)|*.hdr")	
#define FSTRING_HGT		_T("HGT SRTM Files (*.hgt)|*.hgt")
#define FSTRING_HTML	_T("HTML Files (*.html)|*.html")
#define FSTRING_IGC		_T("IGC Files (*.igc)|*.igc")
#define FSTRING_IMG		_T("IMG Files (*.img)|*.img")
#define FSTRING_JPEG	_T("JPEG Files (*.jpg, *.jpeg)|*.jpg;*.jpeg")
#define FSTRING_LULC	_T("LULC Files (*.gir)|*.gir")
#define FSTRING_MEM		_T("Japanese DEM Files (*.mem)|*.mem")
#define FSTRING_MI		_T("MapInfo Files (*.tab, *.mif)|*.tab;*.mif")
#define FSTRING_NTF		_T("OSGB NTF Files (*.ntf)|*.ntf")
#define FSTRING_Planet	_T("Planet Files (*.txt)|*.txt")
#define FSTRING_PGM		_T("PGM Files (*.pgm)|*.pgm")
#define FSTRING_PNG		_T("PNG Files (*.png)|*.png")
#define FSTRING_RAW		_T("Raw raster files (*.raw)|*.raw")
#define FSTRING_S57		_T("S-57 Files (*.000)|*.000")	
#define FSTRING_SDTS	_T("SDTS Files (*catd.ddf)|*catd.ddf")	
#define FSTRING_SHP		_T("ArcView Shape Files (*.shp)|*.shp")
#define FSTRING_Surfer	_T("Surfer Binary Grids (*.grd)|*.grd")
#define FSTRING_STM		_T("STM Files (*.stm)|*.stm")
#define FSTRING_TER		_T("Terragen terrain (*.ter)|*.ter")
#define FSTRING_TIF		_T("GeoTIFF Files (*.tif)|*.tif")
#define FSTRING_TXT		_T("Text Files (*.txt)|*.txt")
#define FSTRING_UTL		_T("Utility Files (*.utl)|*.utl")
#define FSTRING_VF		_T("Vegetation Files (*.vf)|*.vf")
#define FSTRING_VTST	_T("Structure Files (*.vtst)|*.vtst")
#define FSTRING_VTSTGZ	_T("Structure Files (*.vtst.gz)|*.vtst.gz")
#define FSTRING_WRL		_T("WRL Files (*.wrl)|*.wrl")
#define FSTRING_XML		_T("XML Files (*.xml)|*.xml")
#define FSTRING_XYZ		_T("XYZ Text Files (*.xyz)|*.xyz")

#define FSTRING_COMP	_T("Compressed Files (*.tar, *.gz, *.tgz, *.zip)|*.tar;*.gz;*.tgz;*.zip")

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


///
// Name: LayerArray
// An array of layer objects.
//
class LayerArray : public Array<vtLayerPtr>
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

////////////////////
// Helpers

void AddType(wxString &str, const wxString &filter);
wxString2 GetLayerTypeName(const LayerType &lype);

#endif

