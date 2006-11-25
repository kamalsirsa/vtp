//
// TerrainLayers.h
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINLAYERSH
#define TERRAINLAYERSH

#include "Structure3d.h"	// for vtStructureArray3d

/** \addtogroup terrain */
/*@{*/

class vtLayer
{
public:
	virtual ~vtLayer() {}

	virtual void SetLayerName(const vtString &fname) = 0;
	virtual vtString GetLayerName() = 0;
	virtual void SetVisible(bool vis) = 0;
	virtual bool GetVisible() = 0;
};

class vtStructureLayer : public vtStructureArray3d, public vtLayer
{
public:
	void SetLayerName(const vtString &fname) { SetFilename(fname); }
	vtString GetLayerName() { return GetFilename(); }
	void SetVisible(bool vis)
	{
		SetEnabled(vis);
	}
	bool GetVisible()
	{
		return GetEnabled();
	}
};

class vtImageLayer : public vtLayer
{
public:
	vtImageLayer();
	~vtImageLayer();

	void SetLayerName(const vtString &fname) { /* not applicable */ }
	vtString GetLayerName() { return m_pImage->GetFilename(); }
	void SetVisible(bool vis);
	bool GetVisible();

	vtImage *m_pImage;
	vtMultiTexture *m_pMultiTexture;
};

/** The set of all structure arrays which are on a terrain. */
class LayerSet : public vtArray<vtLayer *>
{
public:
	void Remove(vtLayer *lay);
	vtStructureLayer *FindStructureFromNode(vtNode* pNode, int &iOffset);
};

/*@}*/	// Group terrain

#endif // TERRAINLAYERSH

