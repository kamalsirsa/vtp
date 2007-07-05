//
// AbstractLayer.h
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ABSTRACTLAYERH
#define ABSTRACTLAYERH

#include "TerrainLayers.h"

class vtTerrain;
class vtFeatureSet;

/**
 * An abstract layer is a traditional GIS-style set of geometry features with
 * attributes.  It can be shown on the terrain in a variety of ways (styles).
 * Style is described with a set of properties, encoded as a vtTagArray.
 * These are the supported style properties, all of which are optional:
	- "Filename": If the layer should be loaded from disk, this is the filename
		to load from.  It can be a relative path anywhere on the VTP data paths,
		or an absolute path.
	- "Geometry": true to show geometry (such as 3D lines) for the features.
	 - "GeomColor": The color of each geometry (R,G,B as float 0..1).
		Default is white.
	 - "GeomHeight": The height in meters of each geometry above the ground.
	 - "Tessellate": true to tesslate the geometry of each feature before draping
		it on the ground.  This can produce a smoother result.
	 - "LineWidth": Pixel width of the 3D lines to create.  Default it 1.
	- "Labels": true to show floating text labels for the features.
	 - "LabelColor": The color of each label (R,G,B as float 0..1)
		Default is white.
	 - "TextFieldIndex": The 0-based index of the field which contains the text you
		want to use for the label.  For example, if you have several fields called
		"ID", "Name", and "Address", and you want to show "Name", use the value 1.
	 - "ColorFieldIndex": Similarly, if you want the color of each label to be
		based on a field, this is the index of that field.
	 - "Elevation": Height in meters above the ground.  This is the distance
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

	void CreateStyledFeatures(vtTerrain *pTerr);
	void CreateObjectGeometry(vtTerrain *pTerr);
	void CreateLineGeometry(vtTerrain *pTerr);
	void CreateFeatureLabels(vtTerrain *pTerr);
	bool CreateTextureOverlay(vtTerrain *pTerr);

	void ReleaseGeometry();

	/// This is the set of features which the layer contains. Style information
	///  is associated with it, using vtFeatureSet::SetProperties() and 
	///  vtFeatureSet::GetProperties().
	vtFeatureSet *pSet;
	vtGroup *pContainer;
	vtGroup *pGeomGroup;
	vtGroup *pLabelGroup;
	vtMultiTexture *pMultiTexture;
};

#endif // ABSTRACTLAYERH

