//
// A raw data layer, suitable for storing and displaying the type of
// generic spatial data contained in a Shapefile.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef RAWLAYERH
#define RAWLAYERH

#include "vtdata/Features.h"
#include "Layer.h"

class vtRawLayer : public vtLayer
{
public:
	vtRawLayer();
	virtual ~vtRawLayer();

	void SetGeomType(OGRwkbGeometryType type);
	OGRwkbGeometryType GetGeomType();
	vtFeatureSet *GetFeatureSet() { return m_pSet; }

	// implementation of vtLayer methods
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, class vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &strIn);
	void OnLeftDown(BuilderView *pView, UIContext &ui);

	void AddPoint(const DPoint2 &p2);
	bool LoadWithOGR(const char *filename, void progress_callback(int) = NULL);
	bool ReadFeaturesFromWFS(const char *szServerURL, const char *layername);

	vtProjection *GetAtProjection()
	{
		if (m_pSet)
			return &(m_pSet->GetAtProjection());
		else
			return NULL;
	}
	wxString2 GetLayerFilename()
	{
		if (m_pSet)
			return wxString2(m_pSet->GetFilename());
		else
			return vtLayer::GetLayerFilename();
	}
	void SetLayerFilename(const wxString2 &fname)
	{
		if (m_pSet)
			m_pSet->SetFilename(fname.mb_str());
		vtLayer::SetLayerFilename(fname);
	}

	void ReadGeoURL();

protected:
	vtFeatureSet	*m_pSet;
	DrawStyle	m_DrawStyle;
};

#endif	// RAWLAYERH

