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
class vtImage: public vtImageBase, public osg::Referenced
{
public:
	vtImage(const char *fname, bool b16bit = false);
	vtImage(class vtDIB *pDIB, bool b16bit = false);
	void Release();

	int GetWidth() const;
	int GetHeight() const;
	int GetDepth() const;

	// Implementation
	osg::ref_ptr<osg::Image>	m_pOsgImage;

protected:
	bool m_b16bit;
	unsigned char *m_pPngData;

	void _CreateFromDIB(vtDIB *pDIB);
	bool _ReadPNG(const char *filename);

protected:
	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	// (Could be private, but that causes an annoying gcc warning.)
	virtual ~vtImage();
};

#endif	// VTOSG_IMAGEH

