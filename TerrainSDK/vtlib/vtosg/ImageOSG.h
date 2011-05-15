//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_IMAGEH
#define VTOSG_IMAGEH

#include "vtdata/vtDIB.h"
#include "vtdata/Projections.h"
#include <osg/Image>

/**
 * This class represents an Image.  It can be loaded from a number of
 * file formats, and used as a texture map for a textured material, by
 * passing it to vtMaterial::SetTexture()
 */
class vtImage: public vtBitmapBase, public osg::Image
{
public:
	vtImage();
//	vtImage(const char *fname, bool bAllowCache = true);
	vtImage(class vtDIB *pDIB);
	vtImage(vtImage *copyfrom);

	bool Create(int width, int height, int bitdepth, bool create_palette = false);
	bool WritePNG(const char *fname, bool progress_callback(int) = NULL);
	bool WriteJPEG(const char *fname, int quality = 99, bool progress_callback(int) = NULL);
	bool HasData() { return valid() && data() != NULL; }
	void Scale(int w, int h);

	/// Return the name of the file, if any, from which the image was loaded.
	std::string GetFilename() const { return getFileName(); }

	// Provide vtBitmapBase methods
	unsigned char GetPixel8(int x, int y) const;
	void GetPixel24(int x, int y, RGBi &rgb) const;
	void GetPixel32(int x, int y, RGBAi &rgba) const;

	void SetPixel8(int x, int y, unsigned char color);
	void SetPixel24(int x, int y, const RGBi &rgb);
	void SetPixel32(int x, int y, const RGBAi &rgba);

	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetDepth() const;

	unsigned char *GetData() { return data(); }
	unsigned char *GetRowData(int row) { return data(0, row); }
	void Set16Bit(bool bFlag);

protected:
//	bool _Read(const char *fname, bool bAllowCache = true, bool progress_callback(int) = NULL);
	void _BasicInit();
	void _CreateFromDIB(vtDIB *pDIB, bool b16bit = false);
	bool _ReadPNG(const char *filename);
};
typedef osg::ref_ptr<vtImage> vtImagePtr;

class vtImageGeo : public vtImage
{
public:
	vtImageGeo();
	vtImageGeo(const vtImageGeo *copyfrom);

	bool ReadTIF(const char *filename, bool progress_callback(int) = NULL);
	void ReadExtents(const char *filename);

	// In case the image was loaded from a georeferenced format (such as
	//  GeoTIFF), provide access to the georef
	vtProjection &GetProjection() { return m_proj; }
	DRECT &GetExtents() { return m_extents; }

protected:
	// These two fields are rarely used
	vtProjection m_proj;
	DRECT m_extents;
};
typedef osg::ref_ptr<vtImageGeo> vtImageGeoPtr;

class vtOverlappedTiledImage : public vtOverlappedTiledBitmap
{
public:
	vtOverlappedTiledImage();
	bool Create(int iTileSize, int iBitDepth);
	bool Load(const char *filename, bool progress_callback(int) = NULL);

	vtBitmapBase *GetTile(int i, int j) { return m_Tiles[i][j]; }
	const vtBitmapBase *GetTile(int i, int j) const { return m_Tiles[i][j]; }

	vtImagePtr m_Tiles[4][4];
};

bool vtImageInfo(const char *filename, int &width, int &height, int &depth);
vtImagePtr vtImageRead(const char *fname);

#endif	// VTOSG_IMAGEH

