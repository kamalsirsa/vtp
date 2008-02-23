//
// Helper.cpp - various helper functions used by VTBuilder
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//#include "Frame.h"
#include "Builder.h"
#include "Helper.h"
#include "vtdata/vtLog.h"
#include "LocalDatabuf.h"
#include "TilingOptions.h"

#if SUPPORT_SQUISH
#include "squish.h"
#ifdef _MSC_VER
	#pragma message( "Adding link with squish.lib" )
	#pragma comment( lib, "squish.lib" )
#endif
#endif

#define USE_LIBMINI_SQUISH 0
#define USE_LIBMINI_CONVHOOK 0

#if (USE_LIBMINI_DATABUF && USE_LIBMINI_SQUISH)
#include "squishbase.h"
#endif

#if (USE_LIBMINI_DATABUF && USE_LIBMINI_CONVHOOK)
#include "convbase.h"
#endif

#if (USE_LIBMINI_DATABUF && (USE_LIBMINI_SQUISH || USE_LIBMINI_CONVHOOK))
#ifdef _MSC_VER
  #if _MSC_VER >= 1400	// vc8
	  #pragma message( "Adding link with libMiniSFX-vc8.lib" )
	  #pragma comment( lib, "libMiniSFX-vc8.lib" )
  #else					// vc71
	  #pragma message( "Adding link with libMiniSFX-vc7.lib" )
	  #pragma comment( lib, "libMiniSFX-vc7.lib" )
  #endif
#endif
#endif


/////////////////////////////////////////////////////

void WriteMiniImage(const vtString &fname, const TilingOptions &opts,
					unsigned char *rgb_bytes, vtMiniDatabuf &output_buf,
					int iUncompressedSize, ImageGLCanvas *pCanvas)
{
	if (opts.bUseTextureCompression)
	{
		// Compressed
		// Output to a compressed RGB .db file

		if (opts.eCompressionType == TC_OPENGL)
		{
#if USE_OPENGL
			DoTextureCompress(rgb_bytes, output_buf, pCanvas->m_iTex);

			output_buf.savedata(fname);
			output_buf.release();

			if (output_buf.xsize == 256)
				pCanvas->Refresh(false);
#endif
		}
		else if (opts.eCompressionType == TC_SQUISH_FAST ||
			opts.eCompressionType == TC_SQUISH_SLOW)
		{
#if SUPPORT_SQUISH

#if (USE_LIBMINI_DATABUF && USE_LIBMINI_SQUISH)
			output_buf.type = 3;	// RGB
			output_buf.bytes = iUncompressedSize;
			output_buf.data = malloc(iUncompressedSize);
			memcpy(output_buf.data, rgb_bytes, iUncompressedSize);

			InitSquishHook(opts.eCompressionType == TC_SQUISH_FAST);
			output_buf.autocompress();
#else
			DoTextureSquish(rgb_bytes, output_buf, opts.eCompressionType == TC_SQUISH_FAST);
#endif

			output_buf.savedata(fname);
			output_buf.release();

#endif
		}
	}
	else
	{
		// Uncompressed
		// Output to a plain RGB .db file

		output_buf.type = 3;	// RGB
		output_buf.bytes = iUncompressedSize;
		output_buf.data = malloc(iUncompressedSize);
		memcpy(output_buf.data, rgb_bytes, iUncompressedSize);

#if USE_LIBMINI_DATABUF
		bool saveasJPEG=false;
		output_buf.savedata(fname,saveasJPEG?1:0); // external format 1=JPEG
#else
		output_buf.savedata(fname);
#endif
		output_buf.release();
	}
}

/////////////////////////////////////////////////////

#if USE_OPENGL

#ifdef __DARWIN_OSX__
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
#else
  #include <GL/gl.h>
  #include <GL/glext.h>
#endif
#ifdef _MSC_VER
	#pragma message( "Adding link with opengl32.lib" )
	#pragma comment( lib, "opengl32.lib" )
#endif

#if !WIN32
#include <dlfcn.h>
#endif
#ifdef __DARWIN_OSX__
# include <mach-o/dyld.h>
#endif

void* getGLExtensionFuncPtr(const char *funcName)
{
#if defined(WIN32)
	return (void*)wglGetProcAddress(funcName);

#elif defined(__APPLE__)
	std::string temp( "_" );
	temp += funcName;    // Mac OS X prepends an underscore on function names
	if ( NSIsSymbolNameDefined( temp.c_str() ) )
	{
		NSSymbol symbol = NSLookupAndBindSymbol( temp.c_str() );
		return NSAddressOfSymbol( symbol );
	} else
		return NULL;

#elif defined (__sun)
	static void *handle = dlopen((const char *)0L, RTLD_LAZY);
	return dlsym(handle, funcName);

#elif defined (__sgi)
	static void *handle = dlopen((const char *)0L, RTLD_LAZY);
	return dlsym(handle, funcName);

#elif defined (__FreeBSD__)
	return dlsym( RTLD_DEFAULT, funcName );

#elif defined (__linux__)
	typedef void (*__GLXextFuncPtr)(void);
	typedef __GLXextFuncPtr (*GetProcAddressARBProc)(const char*);
	static GetProcAddressARBProc s_glXGetProcAddressARB = (GetProcAddressARBProc)dlsym(0, "glXGetProcAddressARB");
	if (s_glXGetProcAddressARB)
	{
		return (void*) (s_glXGetProcAddressARB)(funcName);
	}
	else
	{
		return dlsym(0, funcName);
	}

#else // all other unixes
	return dlsym(0, funcName);

#endif
}


void DoTextureCompress(unsigned char *rgb_bytes, vtMiniDatabuf &output_buf,
					   GLuint &iTex)
{
	// Next, compress them to a DXT1 output file
	GLenum target = GL_TEXTURE_2D;
	int level = 0;
	GLint internalformat = GL_COMPRESSED_RGB_ARB;
	int border = 0;
	GLenum format = GL_RGB;
	GLenum type = GL_UNSIGNED_BYTE;
	GLvoid *pixels = rgb_bytes;

	if (iTex == 9999)	// not yet assigned
		glGenTextures(1, &iTex);
	glBindTexture(GL_TEXTURE_2D, iTex);

	glTexImage2D(target, level, internalformat,
		output_buf.xsize, output_buf.ysize, border, format, type, pixels);

	// Check to see if the compression operation succeeded
	GLint iParam;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_COMPRESSED_ARB, &iParam);
//	VTLOG("GL_TEXTURE_COMPRESSED_ARB: %d\n", iParam);

	GLint iInternalFormat;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, &iInternalFormat);
//	VTLOG("GL_TEXTURE_INTERNAL_FORMAT: %d\n", iInternalFormat);

	GLint iSize;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &iSize);
//	VTLOG("GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB: %d\n", iSize);

	output_buf.type = 5;	// compressed RGB
	output_buf.bytes = iSize;
	output_buf.data = malloc(iSize);

	PFNGLGETCOMPRESSEDTEXIMAGEARBPROC gctia = (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)
		getGLExtensionFuncPtr("glGetCompressedTexImageARB");
	gctia(target, level, output_buf.data);
}


//
// ImageGLCanvas class:
//  We need to open an OpenGL context in order to do the texture compression,
//  so we may as well draw something into it, since it requires little extra
//  work, and provides interesting visual feedback to the user.
//
BEGIN_EVENT_TABLE(ImageGLCanvas, wxGLCanvas)
EVT_PAINT(ImageGLCanvas::OnPaint)
EVT_SIZE(ImageGLCanvas::OnSize)
END_EVENT_TABLE()

ImageGLCanvas::ImageGLCanvas(wxWindow *parent, const wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString &name,
	int* gl_attrib) : wxGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
	m_iTex = 9999;

	// These two lines are needed for wxGTK (and possibly other platforms, but not wxMSW)
	parent->Show(TRUE);
	SetCurrent();
}

void ImageGLCanvas::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	if (m_iTex == 9999)
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	IPoint2 size;
	GetClientSize(&size.x, &size.y);

	// Direct pixel coordinates with origin in center of window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-size.x/2,size.x/2,-size.y/2,size.y/2,0,1);

	// Draw a quad with the current texture
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, m_iTex);
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);	glVertex3f(-128, -128, 0);
	glTexCoord2f(1, 1);	glVertex3f( 128, -128, 0);
	glTexCoord2f(1, 0);	glVertex3f( 128,  128, 0);
	glTexCoord2f(0, 0);	glVertex3f(-128,  128, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	SwapBuffers();
}

void ImageGLCanvas::OnSize(wxSizeEvent& event)
{
	glViewport(0, 0, event.m_size.x, event.m_size.y);
}
#endif	// USE_OPENGL


///////////////////////////////////////////////////////////////////////
// As an alternative to using OpenGL for texture compression, use the
//  Squish library if available.
//
#if SUPPORT_SQUISH
using namespace squish;

void DoTextureSquish(unsigned char *rgb_bytes, vtMiniDatabuf &output_buf, bool bFast)
{
	int flags = kDxt1;

	if (bFast)
	{
		//! Use a fast but low quality colour compressor.
		flags |= kColourRangeFit;
	}
	else
	{
		//! Use a slow but very high quality colour compressor.
		flags |= kColourClusterFit;
	}

	int bytesPerBlock = ( ( flags & kDxt1 ) != 0 ) ? 8 : 16;
	int targetDataSize = bytesPerBlock*output_buf.xsize*output_buf.ysize/16;
	int stride = output_buf.xsize * 3;

	output_buf.type = 5;	// compressed RGB
	output_buf.bytes = targetDataSize;
	output_buf.data = malloc(targetDataSize);

	// loop over blocks and compress them
	u8* targetBlock = (u8 *) output_buf.data;
	for( unsigned int y = 0; y < output_buf.ysize; y += 4 )
	{
		// process a row of blocks
		for( unsigned int x = 0; x < output_buf.xsize; x += 4 )
		{
			// get the block data
			u8 sourceRgba[16*4];

			for( int py = 0, i = 0; py < 4; ++py )
			{
				u8 const *row = rgb_bytes + (y + py)*stride + (x*3);
				for( int px = 0; px < 4; ++px, ++i )
				{
					// get the pixel colour
					for( int j = 0; j < 3; ++j )
						sourceRgba[4*i + j] = *row++;

					// skip alpha for now
					sourceRgba[4*i + 3] = 255;
				}
			}

			// compress this block
			Compress( sourceRgba, targetBlock, flags );

			// advance
			targetBlock += bytesPerBlock;
		}
	}
}
#endif	// SUPPORT_SQUISH

// S3TC auto-compression hook
void autocompress(int isrgbadata,unsigned char *rawdata,unsigned int bytes,
				  unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,
				  void *data)
   {
#if (USE_LIBMINI_DATABUF && USE_LIBMINI_SQUISH)

   int mode=*((int *)data);

   squishbase::compressS3TC(isrgbadata,rawdata,bytes,
							s3tcdata,s3tcbytes,width,height,mode);

#endif
   }

void InitSquishHook(bool squishFAST)
   {
#if (USE_LIBMINI_DATABUF && USE_LIBMINI_SQUISH)

   static int mode=squishbase::SQUISHMODE_GOOD;

   if (squishFAST) mode=squishbase::SQUISHMODE_FAST;
   else mode=squishbase::SQUISHMODE_GOOD;

   // register auto-compression hook
   databuf::setautocompress(autocompress,&mode);

#endif
   }

void InitConvHook(bool enableGREYC)
   {
#if (USE_LIBMINI_DATABUF && USE_LIBMINI_CONVHOOK)

   // specify conversion parameters
   static convbase::MINI_CONVERSION_PARAMS conversion_params;
   conversion_params.jpeg_quality=75.0f; // jpeg quality in percent
   conversion_params.usegreycstoration=enableGREYC; // use greycstoration for image denoising
   conversion_params.greyc_p=0.8f; // greycstoration sharpness, useful range=[0.7-0.9]
   conversion_params.greyc_a=0.4f; // greycstoration anisotropy, useful range=[0.1-0.5]

   // register libMini conversion hook (JPEG/PNG)
   databuf::setconversion(convbase::conversionhook,&conversion_params);

#endif
   }
