//
// AbstractLayer.h
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ABSTRACTLAYERH
#define ABSTRACTLAYERH

class vtTerrain;
class vtFeatureSet;

class vtLayer
{
public:
	virtual ~vtLayer() {}

	virtual void SetLayerName(const vtString &fname) = 0;
	virtual vtString GetLayerName() = 0;
	virtual void SetVisible(bool vis) = 0;
	virtual bool GetVisible() = 0;
};

/**
 * An abstract layer is a traditional GIS-style set of geometry features with
 * attributes.  It can be shown on the terrain in a variety of ways.  It is
 * described with a set of XML-style properties (see TParams).
 */
class vtAbstractLayer : public vtLayer
{
public:
	vtAbstractLayer();
	~vtAbstractLayer();

	void SetLayerName(const vtString &fname);
	vtString GetLayerName();
	void SetVisible(bool vis);
	bool GetVisible();

	void CreateStyledFeatures(vtTerrain *pTerr);
	void CreateFeatureLabels(vtTerrain *pTerr);
	void CreateFeatureGeometry(vtTerrain *pTerr);

	void ReleaseGeometry();

	/// This is the set of features which the layer contains. Style information
	///  is associated with it, using vtFeatureSet::SetProperties() and 
	///  vtFeatureSet::GetProperties().
	vtFeatureSet *pSet;
	vtGroup *pContainer;
	vtGroup *pGeomGroup;
	vtGroup *pLabelGroup;
};

#endif // ABSTRACTLAYERH

