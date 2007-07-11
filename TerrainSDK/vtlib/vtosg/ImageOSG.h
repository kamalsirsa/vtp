//
// Copyright (c) 2001-2007 Virtual Terrain Project
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
class vtImage: public osg::Image, public vtBitmapBase
{
public:
	vtImage();
	vtImage(const char *fname, bool bAllowCache = true);
	vtImage(class vtDIB *pDIB);
	vtImage(vtImage *copyfrom);
	void Release();

	bool Create(int width, int height, int bitdepth, bool create_palette = false);
	bool Read(const char *fname, bool bAllowCache = true, bool progress_callback(int) = NULL);
	bool WritePNG(const char *fname, bool progress_callback(int) = NULL);
	bool WriteJPEG(const char *fname, int quality = 99, bool progress_callback(int) = NULL);
	bool HasData() { return _data != NULL; }
	void Scale(int w, int h);

	/// Return the name of the file, if any, from which the image was loaded.
	vtString GetFilename() { return m_strFilename; }

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
	void SetLoadWithAlpha(bool bFlag) { m_bLoadWithAlpha = bFlag; }

	// In case the image was loaded from a georeferenced format (such as
	//  GeoTIFF), provide access to the georef
	vtProjection &GetProjection() { return m_proj; }
	DRECT &GetExtents() { return m_extents; }

protected:
	void _BasicInit();
	void _CreateFromDIB(vtDIB *pDIB);
	bool _ReadPNG(const char *filename);
	bool _ReadTIF(const char *filename, bool progress_callback(int) = NULL);

protected:
	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	// (Could be private, but that causes an annoying gcc warning.)
	virtual ~vtImage();

protected:
	bool m_b16bit;
	bool m_bLoadWithAlpha;
	vtString m_strFilename;
	int m_iRowSize;		// in bytes

	// These two fields are rarely used, and increase size of this object
	//  from 168 to 256 bytes.
	vtProjection m_proj;
	DRECT m_extents;
};

class vtOverlappedTiledImage : public vtOverlappedTiledBitmap
{
public:
	vtOverlappedTiledImage();
	bool Create(int iTileSize, int iBitDepth);
	void Release();
	bool Load(const char *filename, bool progress_callback(int) = NULL);

	vtBitmapBase *GetTile(int i, int j) { return m_Tiles[i][j]; }
	const vtBitmapBase *GetTile(int i, int j) const { return m_Tiles[i][j]; }

	vtImage *m_Tiles[4][4];
};

bool vtImageInfo(const char *filename, int &width, int &height, int &depth);
vtImage *vtImageRead(const char *fname, bool bAllowCache = true, bool progress_callback(int) = NULL);
void vtImageCacheClear();

#endif	// VTOSG_IMAGEH

