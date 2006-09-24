//
// Helper.cpp - various helper functions used by the classes
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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

//
// Display a message to the user, and also send it to the log file.
//
void DisplayAndLog(const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	char ach[2048];
	vsprintf(ach, pFormat, va);

	wxString msg(ach, wxConvUTF8);
	wxMessageBox(msg);

	strcat(ach, "\n");
	VTLOG1(ach);
}

#if SUPPORT_WSTRING
//
// Also wide-character version of the same function.
//
void DisplayAndLog(const wchar_t *pFormat, ...)
{
//#ifdef UNICODE
//	// Try to translate the string
//	wxString trans = wxGetTranslation(pFormat);
//	pFormat = trans.c_str();
//#endif

	va_list va;
	va_start(va, pFormat);

	// Use wide characters
	wchar_t ach[2048];
#ifdef _MSC_VER
	vswprintf(ach, pFormat, va);
#else
	// apparently on non-MSVC platforms this takes 4 arguments (safer)
	vswprintf(ach, 2048, pFormat, va);
#endif

	wxString msg(ach);
	wxMessageBox(msg);

	VTLOG1(ach);
	VTLOG1("\n");
}
#endif // SUPPORT_WSTRING


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

#if USE_OPENGL

#include "GL/gl.h"
#include "GL/glext.h"
#ifdef _MSC_VER
	#pragma message( "Adding link with opengl32.lib" )
	#pragma comment( lib, "opengl32.lib" )
#endif

#if !WIN32
#include <dlfcn.h>
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


void DoTextureCompress(unsigned char *rgb_bytes, MiniDatabuf &output_buf,
					   unsigned int &iTex)
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
	int iParam;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_COMPRESSED_ARB, &iParam);
//	VTLOG("GL_TEXTURE_COMPRESSED_ARB: %d\n", iParam);

	int iInternalFormat;
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, &iInternalFormat);
//	VTLOG("GL_TEXTURE_INTERNAL_FORMAT: %d\n", iInternalFormat);

	int iSize;
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


