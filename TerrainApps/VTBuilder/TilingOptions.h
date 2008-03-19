//
// TilingOptions.h
//
// Copyright (c) 2006-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TilingOptions_H
#define TilingOptions_H

#include "ElevDrawOptions.h"

enum TextureCompressionType { TC_OPENGL, TC_SQUISH_FAST, TC_SQUISH_SLOW };

/**
 * All the options needed to describe how to create a tileset.
 */
struct TilingOptions
{
	int cols, rows;
	int lod0size;
	int numlods;
	vtString fname;

	// If this is an elevation tileset, then optionally a corresponding
	//  derived image tileset can be created.
	bool bCreateDerivedImages;
	bool bMaskUnknownAreas;
	vtString fname_images;
	ElevDrawOptions draw;

	// If elevation, we can omit flat (sea-level) tiles
	bool bOmitFlatTiles;

	// If imagery (or deriving imagery from elevation) then it can be compressed
	bool bUseTextureCompression;
	TextureCompressionType eCompressionType;

	// after the sampling, will contain the number of NODATA filled in
	int iNoDataFilled;
};

#endif // TilingOptions_H

