//
// Layer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LAYER_H
#define LAYER_H

#include "vtdata/MathTypes.h"
#include "vtdata/Projections.h"

//
// Filename filter strings
//
#define FSTRING_BT		"BT Files (*.bt)|*.bt|"
#define FSTRING_RMF		"RMF Files (*.rmf)|*.rmf|"
#define FSTRING_BCF		"BCF Files (*.bcf)|*.bcf|"

#define FSTRING_ADF		"Arc Data Files (*.adf)|*.adf|"
#define FSTRING_ASC		"ArcInfo ASCII grid (*.asc)|*.asc|"
#define FSTRING_BIL		"ArcInfo BIL grid (*.bil)|*.bil|"
#define FSTRING_CDF		"CDF Files (*.cdf, *.grd)|*.cdf;*.grd|"
#define FSTRING_DEM		"DEM Files (*.dem)|*.dem|"
#define FSTRING_DLG		"USGS DLG Files (*.dlg, *.opt)|*.dlg;*.opt|"
#define FSTRING_DOQ		"USGS DOQ Files (*.jpg, *.doq, *.coq)|*.doq;*.coq;*.jpg|"
#define FSTRING_DTED	"DTED Files (*.dte, *.dt0)|*.dte;*.dt0|"
#define FSTRING_GTOPO	"GTOPO30 Files (*.hdr)|*.hdr|"	
#define FSTRING_LULC	"LULC Files (*.gir)|*.gir|"
#define FSTRING_PGM		"PGM Files (*.pgm)|*.pgm|"
#define FSTRING_PNG		"PNG Files (*.png)|*.png|"
#define FSTRING_RAW		"Raw raster files (*.raw)|*.raw|"
#define FSTRING_SDTS	"SDTS Files (*catd.ddf)|*catd.ddf|"	
#define FSTRING_SHP		"ArcView Shape Files (*.shp)|*.shp|"
#define FSTRING_Surfer	"Surfer Binary Grids (*.grd)|*.grd|"
#define FSTRING_TER		"Terragen terrain (*.ter)|*.ter|"
#define FSTRING_TIF		"GeoTIFF Files (*.tif)|*.tif|"
#define FSTRING_UTL		"Utility Files (*.utl)|*.utl|"
#define FSTRING_VTST	"Structure Files (*.vtst)|*.vtst|"
#define FSTRING_XML		"XML Files (*.xml)|*.xml|"

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

class vtLayer
{
public:
	vtLayer(LayerType type);
	virtual ~vtLayer();

	// attributes
	wxString GetFilename() { return m_strFilename; }
	void SetFilename(wxString str);
	LayerType GetType() { return m_type; }
	bool SetVisible(bool bVisible);
	bool GetVisible() { return m_bVisible; }
	void SetModified(bool bModified);
	bool GetModified() { return m_bModified; }
	bool IsNative() { return m_bNative; }

	// operations
	static vtLayer *CreateNewLayer(LayerType ltype);
	bool Save(const char *filename = NULL);
	bool Load(const char *filename = NULL);

	// these must be overriden
	virtual bool GetExtent(DRECT &rect) = 0;
	virtual void DrawLayer(wxDC* pDC, class vtScaledView *pView) = 0;
	virtual bool ConvertProjection(vtProjection &proj) = 0;
	virtual bool OnSave() = 0;
	virtual bool OnLoad() = 0;
	virtual bool AppendDataFrom(vtLayer *pL) = 0;
	virtual void GetProjection(vtProjection &proj) = 0;
	virtual void SetProjection(vtProjection &proj) = 0;
	virtual void Offset(const DPoint2 &p);
	virtual void GetPropertyText(wxString &str) {}

	static char *LayerTypeName[];
	static char *LayerFileExtension[];
	bool AskForSaveFilename();

protected:
	char *GetFileDialogFilter();
	void SetMessageText(const char *msg);

	wxString	m_strFilename;
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
	virtual void DestructItems(int first, int last);
};

////////////////////
// helpers

void AddType(wxString &str, wxString filter);

#endif

