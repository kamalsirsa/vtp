//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPLIB_IMAGEH
#define VTPLIB_IMAGEH

#include "vtdata/vtDIB.h"

class ssgTexture2 : public ssgTexture
{
public:
	ssgTexture2(vtDIB *pDib, bool wrapu = true, bool wrapv = true, bool mipmap = true);
};

class vtImage: public vtImageBase
{
public:
	vtImage(const char *fname, int internalformat = -1);
	vtImage(class vtDIB *pDIB, int internalformat = -1);

	// Implementation
	ssgTexture	*m_pTexture;
};

#endif
