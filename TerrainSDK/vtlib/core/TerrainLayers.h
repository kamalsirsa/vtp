//
// TerrainLayers.h
//
// Copyright (c) 2006-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINLAYERSH
#define TERRAINLAYERSH

#include "vtdata/LayerBase.h"
#include "Structure3d.h"	// for vtStructureArray3d
#include "Plants3d.h"		// for vtPlantInstanceArray3d

/** \addtogroup terrain */
/*@{*/

/**
 * Simple abstraction class to describe all vtlib terrain layers.
 */
class vtLayer : public osg::Referenced, public vtLayerBase
{
public:
	vtLayer(LayerType type) : vtLayerBase(type) {}

	virtual void SetLayerName(const vtString &fname) = 0;
	virtual vtString GetLayerName() = 0;
};
typedef osg::ref_ptr<vtLayer> vtLayerPtr;

/**
 * This class encapsulates vtStructureArray3d as a terrain layer.
 */
class vtStructureLayer : public vtStructureArray3d, public vtLayer
{
public:
	vtStructureLayer(): vtLayer(LT_STRUCTURE) {}

	void SetLayerName(const vtString &fname) { SetFilename(fname); }
	vtString GetLayerName() { return GetFilename(); }
	void SetVisible(bool vis)
	{
		SetEnabled(vis);
		vtLayerBase::SetVisible(vis);
	}
};

/**
 * This class encapsulates vtPlantInstanceArray3d as a terrain layer.
 */
class vtVegLayer : public vtPlantInstanceArray3d, public vtLayer
{
public:
	vtVegLayer(): vtLayer(LT_VEG) {}

	void SetLayerName(const vtString &fname) { SetFilename(fname); }
	vtString GetLayerName() { return GetFilename(); }
	void SetVisible(bool vis)
	{
		SetEnabled(vis);
		vtLayerBase::SetVisible(vis);
	}
};

/**
 * A vtlib image layer is a bitmap draped on the terrain surface.
 */
class vtImageLayer : public vtLayer
{
public:
	vtImageLayer();
	~vtImageLayer();

	void SetLayerName(const vtString &fname) { /* not applicable */ }
	vtString GetLayerName() { return m_pImage->GetFilename().c_str(); }
	void SetVisible(bool vis);
	bool GetVisible();

	vtImageGeoPtr m_pImage;
	vtMultiTexture *m_pMultiTexture;
};

/** The set of all structure arrays which are on a terrain. */
class LayerSet : public std::vector<vtLayerPtr>
{
public:
	void Remove(vtLayer *lay);
	vtLayer *FindByName(const vtString &name);
	vtStructureLayer *FindStructureFromNode(osg::Node *pNode, int &iOffset);
};

/*@}*/	// Group terrain

#endif // TERRAINLAYERSH

