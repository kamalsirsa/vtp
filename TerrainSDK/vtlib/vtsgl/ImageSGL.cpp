
#include "vtlib/vtlib.h"

vtImage::vtImage(const char *fname, int internalformat) : vtImageBase(fname)
{
	// TODO
//	m_pTexture = new sglTexture2D((char *)fname);
}

vtImage::vtImage(class vtDIB *pDIB, int internalformat)
{
	// TODO
//	m_pTexture = new sglTexture2D(pDIB);
}
