//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTPSM_IMAGEH
#define VTPSM_IMAGEH

#include "vtdata/vtDIB.h"

class vtImage : public PSImage
{
public:
	vtImage(const char *fname, int internalformat = -1);
	vtImage(class vtDIB *pDIB, int internalformat = -1);
	const char* GetFilename() const	{ return GetFileName(); }
};

#ifdef VTLIB_PSMDIB
extern	bool	_psm_ReadDIB(const char* filename, PSStream* stream, PSLoadEvent* e);
#endif

#endif
