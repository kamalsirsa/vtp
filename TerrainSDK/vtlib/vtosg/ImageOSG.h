//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_IMAGEH
#define VTOSG_IMAGEH

#include "vtdata/vtDIB.h"
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
	vtImage(const char *fname, bool b16bit = false);
	vtImage(class vtDIB *pDIB, bool b16bit = false);
	void Release();

	bool Create(int width, int height, int bitdepth, bool create_palette = false);
	bool Read(const char *fname, bool b16bit = false);
	bool HasData() { return _data != NULL; }

	/// Return the name of the file, if any, from which the image was loaded.
	vtString GetFilename() { return m_strFilename; }

	// Provide vtBitmapBase methods
	void GetPixel24(int x, int y, RGBi &rgb) const;
	void SetPixel24(int x, int y, const RGBi &rgb);

	unsigned char GetPixel8(int x, int y) const;
	void SetPixel8(int x, int y, unsigned char color);

	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetDepth() const;

protected:
	void _CreateFromDIB(vtDIB *pDIB);
	bool _ReadPNG(const char *filename);

protected:
	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	// (Could be private, but that causes an annoying gcc warning.)
	virtual ~vtImage();

protected:
	bool m_b16bit;
	vtString m_strFilename;
	int m_iRowSize;		// in bytes
};

vtImage *vtImageRead(const char *fname, bool b16bit = false);
void vtImageCacheClear();

#endif	// VTOSG_IMAGEH

