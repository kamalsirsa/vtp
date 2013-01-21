//
// SurfaceTexture.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_SURFACE_TEXTURE_H
#define VTLIB_SURFACE_TEXTURE_H

#include "TParams.h"
#include "vtdata/HeightField.h"

class SurfaceTexture
{
public:
	SurfaceTexture()
	{
		m_pMaterials = new vtMaterialArray;
	}

	void LoadTexture(const TParams &options, const vtHeightFieldGrid3d *pHFGrid,
		bool progress_callback(int) = NULL);

	void ShadeTexture(const TParams &options, const vtHeightFieldGrid3d *pHFGrid,
		const FPoint3 &light_dir, bool progress_callback(int) = NULL);

	void MakeColorMap(const vtTagArray &options);
	void CopyFromUnshaded(const TParams &options);

	ImagePtr		m_pUnshadedImage;
	ImagePtr		m_pTextureImage;
	vtMaterialArrayPtr m_pMaterials;
	auto_ptr<ColorMap>	m_pColorMap;

protected:
	void LoadSingleTexture(const TParams &options);
	void MakeDerivedTexture(const TParams &options, const vtHeightFieldGrid3d *pHFGrid,
		bool progress_callback(int));

	/** Color from elevation. */
	void PaintDib(const vtHeightFieldGrid3d *pHFGrid, bool progress_callback(int) = NULL);
};

#endif  // VTLIB_SURFACE_TEXTURE_H
