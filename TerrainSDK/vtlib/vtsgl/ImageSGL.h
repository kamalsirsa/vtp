
#ifndef VTSGL_IMAGEH
#define VTSGL_IMAGEH

#include "vtdata/vtDIB.h"

class vtImage: public vtImageBase
{
public:
	vtImage(const char *fname, int internalformat = -1);
	vtImage(class vtDIB *pDIB, int internalformat = -1);

	void Release();

	sglTexture2D	*m_pTexture;
};

#endif
