//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_IMAGEH
#define VTOSG_IMAGEH

#include "vtdata/vtDIB.h"

class vtImage: public vtImageBase
{
public:
	vtImage(const char *fname, int internalformat = -1);
	vtImage(class vtDIB *pDIB, int internalformat = -1);
	~vtImage();

	// Implementation
	osg::Image	*m_pOsgImage;

protected:
	int		m_internalformat;
	unsigned char *m_pPngData;

	void _CreateFromDIB(vtDIB *pDIB);
	bool _ReadPNG(const char *filename);
};

#endif
