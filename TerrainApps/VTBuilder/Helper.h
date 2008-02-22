//
// VTBuilder Helper.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef HELPERH
#define HELPERH

#include "vtdata/Projections.h"
#include "vtdata/MathTypes.h"

// Display a message to the user, and also send it to the log file.
void DisplayAndLog(const char *pFormat, ...);
#if SUPPORT_WSTRING
void DisplayAndLog(const wchar_t *pFormat, ...);
#endif

/////

// Initialize GDAL/OGR.  If there are problems, report them with a message box and VTLOG.
void CheckForGDALAndWarn();

/////////////////
class vtMiniDatabuf;
class ImageGLCanvas;
struct TilingOptions;

void WriteMiniImage(const vtString &fname, const TilingOptions &opts,
					unsigned char *rgb_bytes, vtMiniDatabuf &output_buf,
					int iUncompressedSize, ImageGLCanvas *pCanvas);

#if USE_OPENGL
#include "wx/glcanvas.h"
void DoTextureCompress(unsigned char *rgb_bytes, vtMiniDatabuf &output_buf,
					   GLuint &iTex);

//
// ImageGLCanvas class:
//  We need to open an OpenGL context in order to do the texture compression,
//  so we may as well draw something into it, since it requires little extra
//  work, and provides interesting visual feedback to the user.
//
class ImageGLCanvas : public wxGLCanvas
{
public:
	ImageGLCanvas(wxWindow *parent, const wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
	  const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxT(""),
	  int* gl_attrib = NULL);
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event) {}	// Do nothing, to avoid flashing.

	GLuint m_iTex;
protected:
	DECLARE_EVENT_TABLE()
};

#endif	// USE_OPENGL

#if SUPPORT_SQUISH
void DoTextureSquish(unsigned char *rgb_bytes, vtMiniDatabuf &output_buf, bool bFast);
#endif

#endif	// HELPERH

void autocompress(int isrgbadata,unsigned char *rawdata,unsigned int bytes,
				  unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,
				  void *data);

void InitSquishHook(bool squishFAST=true);
void InitConvHook(bool enableGREYC=false);
