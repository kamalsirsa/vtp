//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

void ssgLoadTextureFromDIB(vtDIB *pDib)
{
  int w, h, bpp ;

  w   = pDib->GetWidth();
  h   = pDib->GetHeight();
  bpp = pDib->GetDepth();

  int isMonochrome = FALSE ;
  int isOpaque	 = TRUE ;

  int SizeImage = w * h * (bpp / 8) ;

  GLubyte *data = new GLubyte [ SizeImage ] ;
  memcpy(data, pDib->GetDIBData(), SizeImage);

  GLubyte *image ;
  int z ;

  if ( bpp == 24 )
  {
	z = 3 ;
	image = data ;

	/* BGR --> RGB */

	for ( int i = 0 ; i < w * h ; i++ )
	{
	  GLubyte tmp = image [ 3 * i ] ;
	  image [ 3 * i ] = image [ 3 * i + 2 ];
	  image [ 3 * i + 2 ] = tmp ;
	}
  }
  else
  if ( bpp == 32 )
  {
	z = 4 ;
	image = data ;

	/* BGRA --> RGBA */

	for ( int i = 0 ; i < w * h ; i++ )
	{
	  GLubyte tmp = image [ 4 * i ] ;
	  image [ 4 * i ] = image [ 4 * i + 2 ];
	  image [ 4 * i + 2 ] = tmp ;
	}
  }
  else
  {
	// "Can't load %d bpp BMP textures."
//	loadDummyTexture () ;
	return ;
  }

//  make_mip_maps ( image, w, h, z ) ;
}

ssgTexture2::ssgTexture2(vtDIB *pDib, bool wrapu, bool wrapv, bool mipmap)
{
#ifdef GL_VERSION_1_1
	glGenTextures ( 1, & handle ) ;
	glBindTexture ( GL_TEXTURE_2D, handle ) ;
#else
	/* This is only useful on some ancient SGI hardware */
	glGenTexturesEXT ( 1, & handle ) ;
	glBindTextureEXT ( GL_TEXTURE_2D, handle ) ;
#endif

	filename = NULL ;
//	setFilename ( fname ) ;

	ssgLoadTextureFromDIB(pDib);

	glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ) ;

	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			  mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR ) ;
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapu ? GL_REPEAT : GL_CLAMP ) ;
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapv ? GL_REPEAT : GL_CLAMP ) ;
#ifdef GL_VERSION_1_1
	glBindTexture ( GL_TEXTURE_2D, 0 ) ;
#else
	glBindTextureEXT ( GL_TEXTURE_2D, 0 ) ;
#endif
}


vtImage::vtImage(const char *fname, int internalformat) : vtImageBase(fname)
{
	m_pTexture = new ssgTexture((char *)fname);
}

vtImage::vtImage(class vtDIB *pDIB, int internalformat)
{
	m_pTexture = new ssgTexture2(pDIB);
}
