//
// AbstractLayer.h
//
// Copyright (c) 2006-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ABSTRACTLAYERH
#define ABSTRACTLAYERH

#include "TerrainLayers.h"
#include "vtdata/Features.h"

class vtTerrain;

class Visual
{
public:
	Visual() : m_xform(NULL) {}
	std::vector<vtMesh*> m_meshes;
	vtTransform *m_xform;
};

typedef std::map<vtFeature*,Visual*> VizMap;

/**
 * An abstract layer is a traditional GIS-style set of geometry features with
 * attributes.  It can be shown on the terrain in a variety of ways (styles).
 * Style is described with a set of properties, encoded as a vtTagArray.
 * These are the supported style properties, all of which are optional:
	- "Filename": If the layer should be loaded from disk, this is the filename
		to load from.  It can be a relative path anywhere on the VTP data paths,
		or an absolute path.

	- "ObjectGeometry": true to show geometry for each point of the features.
	 - "ObjectGeomColor": The color of each geometry (R,G,B as float 0..1).
		Default is white.
	 - "ObjectColorFieldIndex": The 0-based index of the field which contains the
		color of each geometry.
	 - "ObjectGeomHeight": The height in meters of each geometry above the ground.
	 - "ObjectGeomSize": The size in meters of each geometry.

	- "LineGeometry": true to show lines running though each point of the features.
	 - "LineGeomColor": The color of each line (R,G,B as float 0..1).
		Default is white.
	 - "LineColorFieldIndex": The 0-based index of the field which contains the
		color of each line.
	 - "LineGeomHeight": The height in meters of each line above the ground.
	 - "LineWidth": Pixel width of the 3D lines to create.  Default it 1.
	 - "Tessellate": true to tesslate the geometry of each feature before draping
		it on the ground.  This can produce a smoother result.

	- "Labels": true to show floating text labels for the features.
	 - "LabelColor": The color of each label (R,G,B as float 0..1)
		Default is white.
	 - "TextColorFieldIndex": The 0-based index of the field which contains the
		color of each label.
	 - "TextFieldIndex": The 0-based index of the field which contains the text you
		want to use for the label.  For example, if you have several fields called
		"ID", "Name", and "Address", and you want to show "Name", use the value 1.
	 - "LabelHeight": Height in meters above the ground.  This is the distance
		from the ground surface to the lower edge of the text label.
		Default is 0.
	 - "LabelSize": Size (vertical extent) of the text labels in meters.
		Default is 18.
	 - "Font": Filename (may include path) of the font to use for text labels.
	    Example: "Arial.ttf"

 When a terrain description (TParams) contains an abstract layer, these same
 style properties are encoded.  On disk, they are stored as XML elements.
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

	void SetFeatureSet(vtFeatureSet *pSet);
	vtFeatureSet *GetFeatureSet() const { return pSet; }
	vtGroup *GetLabelGroup() const { return pLabelGroup; }
	vtGroup *GetContainer() const { return pContainer; }
	void CreateContainer(vtTerrain *pTerr);

	// Create for all features
	void CreateStyledFeatures(vtTerrain *pTerr);
	bool CreateTextureOverlay(vtTerrain *pTerr);

	// Create for a single feature
	void CreateStyledFeature(vtTerrain *pTerr, int iIndex);
	void CreateObjectGeometry(vtTerrain *pTerr, vtTagArray &style, unsigned int iIndex);
	void CreateLineGeometry(vtTerrain *pTerr, vtTagArray &style, unsigned int iIndex);
	void CreateFeatureLabel(vtTerrain *pTerr, vtTagArray &style, unsigned int iIndex);

	void ReleaseGeometry();
	void ReleaseFeatureGeometry(unsigned int iIndex);

	// When the underlying feature changes, we need to rebuild the visual
	void Rebuild(vtTerrain *pTerr);
	void RebuildFeature(vtTerrain *pTerr, unsigned int iIndex);

protected:
	void CreateGeomGroup();
	void CreateLabelGroup();

	/// This is the set of features which the layer contains. Style information
	///  is associated with it, using vtFeatureSet::SetProperties() and 
	///  vtFeatureSet::GetProperties().
	vtFeatureSet *pSet;
	vtGroup *pContainer;
	vtGroup *pGeomGroup;
	vtGroup *pLabelGroup;
	vtMultiTexture *pMultiTexture;

	// Handy pointers to disambiguate pSet
	vtFeatureSetPoint2D *pSetP2;
	vtFeatureSetPoint3D *pSetP3;
	vtFeatureSetLineString   *pSetLS2;
	vtFeatureSetLineString3D *pSetLS3;
	vtFeatureSetPolygon *pSetPoly;

	// Used to create the visual features
	vtFont *pFont;
	vtMaterialArray *pGeomMats;

	int material_index_object;
	int material_index_line;
	vtGeom *pGeomObject;
	vtGeom *pGeomLine;

	VizMap m_Map;
	Visual *GetViz(vtFeature *feat);
};

#endif // ABSTRACTLAYERH

