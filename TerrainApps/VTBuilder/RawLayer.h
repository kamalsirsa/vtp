//
// A raw data layer, suitable for storing and displaying the type of
// generic spatial data contained in a Shapefile.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef RAWLAYERH
#define RAWLAYERH

#include "vtdata/Features.h"
#include "Layer.h"

class vtRawLayer : public vtLayer, public vtFeatures
{
public:
	vtRawLayer();
	virtual ~vtRawLayer();

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

	wxString2 GetLayerFilename() { return wxString2(GetFilename()); }
	void SetLayerFilename(const wxString2 &fname)
	{
		SetFilename(fname.mb_str());
		vtLayer::SetLayerFilename(fname);
	}

	void ReadGeoURL();
};

#endif	// RAWLAYERH

