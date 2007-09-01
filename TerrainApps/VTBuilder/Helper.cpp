//
// Helper.cpp - various helper functions used by VTBuilder
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Frame.h"
#include "BuilderView.h"
#include "Helper.h"
#include "vtdata/vtLog.h"
#include "LocalDatabuf.h"

#if SUPPORT_SQUISH
#include "squish.h"
#ifdef _MSC_VER
	#pragma message( "Adding link with squish.lib" )
	#pragma comment( lib, "squish.lib" )
#endif
#endif

//////////////////////////////////

void IncreaseRect(wxRect &rect, int adjust)
{
	rect.y -= adjust;
	rect.height += (adjust<<1);
	rect.x -= adjust;
	rect.width += (adjust<<1);
}

void DrawRectangle(wxDC* pDC, const wxRect &rect)
{
	int left = rect.x;
	int right = rect.x + rect.GetWidth();
	int top = rect.y;
	int bottom = rect.y + rect.GetHeight();
	wxPoint p[5];
	p[0].x = left;
	p[0].y = bottom;

	p[1].x = left;
	p[1].y = top;

	p[2].x = right;
	p[2].y = top;

	p[3].x = right;
	p[3].y = bottom;

	p[4].x = left;
	p[4].y = bottom;
	pDC->DrawLines(5, p);

	pDC->DrawLine(left, bottom, right, top);
	pDC->DrawLine(left, top, right, bottom);
}

//////////////////////////////////////

void AddType(wxString &str, const wxString &filter)
{
	// Chop up the input string.  Expected form is "str1|str2|str3"
	wxString str1 = str.BeforeFirst('|');

	wxString str2 = str.AfterFirst('|');
	str2 = str2.BeforeFirst('|');

	wxString str3 = str.AfterFirst('|');
	str3 = str3.AfterFirst('|');

	// Chop up the filter string.  str4 is the wildcard part.
	wxString str4 = filter.AfterFirst('|');

	// Now rebuild the string, with the filter added
	wxString output = str1 + _T("|");
	output += str2;
	if (str2.Len() > 1)
		output += _T(";");
	output += str4;
	output += _T("|");
	if (str3.Len() > 1)
	{
		output += str3;
		output += _T("|");
	}
	output += filter;

	str = output;
}

//////////////////////////////////////
vtStringArray &GetDataPaths()
{
	return GetMainFrame()->m_datapaths;
}


//////////////////////////////////////

#if WIN32

//
// Win32 allows us to do a real StrectBlt operation, although it still won't
// do a StretchBlt with a mask.
//
void wxDC2::StretchBlit(const wxBitmap &bmp,
						wxCoord x, wxCoord y,
						wxCoord width, wxCoord height,
						wxCoord src_x, wxCoord src_y,
						wxCoord src_width, wxCoord src_height)
{
	wxCHECK_RET( bmp.Ok(), _T("invalid bitmap in wxDC::DrawBitmap") );

	HDC cdc = ((HDC)GetHDC());
	HDC memdc = ::CreateCompatibleDC( cdc );
	HBITMAP hbitmap = (HBITMAP) bmp.GetHBITMAP( );

	COLORREF old_textground = ::GetTextColor(cdc);
	COLORREF old_background = ::GetBkColor(cdc);
	if (m_textForegroundColour.Ok())
	{
		::SetTextColor(cdc, m_textForegroundColour.GetPixel() );
	}
	if (m_textBackgroundColour.Ok())
	{
		::SetBkColor(cdc, m_textBackgroundColour.GetPixel() );
	}

	HGDIOBJ hOldBitmap = ::SelectObject( memdc, hbitmap );

//	int bwidth = bmp.GetWidth(), bheight = bmp.GetHeight();
	::StretchBlt( cdc, x, y, width, height, memdc, src_x, src_y, src_width, src_height, SRCCOPY);

	::SelectObject( memdc, hOldBitmap );
	::DeleteDC( memdc );

	::SetTextColor(cdc, old_textground);
	::SetBkColor(cdc, old_background);
}

#endif // WIN32

/////////////////////////////////////////////////////

void WriteMiniImage(const vtString &fname, const TilingOptions &opts,
					unsigned char *rgb_bytes, vtMiniDatabuf &output_buf,
					int iUncompressedSize, ImageGLCanvas *pCanvas)
{
	if (opts.bUseTextureCompression)
	{
		if (opts.eCompressionType == TC_OPENGL)
		{
#if USE_OPENGL
			DoTextureCompress(rgb_bytes, output_buf, pCanvas->m_iTex);

			output_buf.savedata(fname);
			free(output_buf.data);
			output_buf.data = NULL;

			if (output_buf.xsize == 256)
				pCanvas->Refresh(false);
#endif
		}
		else if (opts.eCompressionType == TC_SQUISH_FAST ||
			opts.eCompressionType == TC_SQUISH_SLOW)
		{
#if SUPPORT_SQUISH
			DoTextureSquish(rgb_bytes, output_buf, opts.eCompressionType == TC_SQUISH_FAST);

			output_buf.savedata(fname);
			free(output_buf.data);
			output_buf.data = NULL;
#endif
		}
	}
	else
	{
		// Uncompressed
		// Output to a plain RGB .db file
		output_buf.type = 3;	// RGB
		output_buf.bytes = iUncompressedSize;
		output_buf.data = rgb_bytes;
		output_buf.savedata(fname);
		output_buf.data = NULL;
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

